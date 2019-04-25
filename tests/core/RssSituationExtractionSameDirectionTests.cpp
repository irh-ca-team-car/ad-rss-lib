// ----------------- BEGIN LICENSE BLOCK ---------------------------------
//
// Copyright (c) 2018-2019 Intel Corporation
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
//    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
//    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//    POSSIBILITY OF SUCH DAMAGE.
//
// ----------------- END LICENSE BLOCK -----------------------------------

#include "TestSupport.hpp"
#include "ad_rss/core/RssSituationExtraction.hpp"

namespace ad_rss {

namespace core {

class RssSituationExtractionSameDirectionTests : public testing::Test
{
protected:
  virtual void SetUp()
  {
    worldModel.egoVehicleRssDynamics = getEgoRssDynamics();
    scene.situationType = ad_rss::situation::SituationType::SameDirection;
    scene.objectRssDynamics = getObjectRssDynamics();
    leadingObject = createObject(36., 0.);
    leadingObject.objectId = 0;

    {
      world::OccupiedRegion occupiedRegion;
      occupiedRegion.lonRange.minimum = ParametricValue(0.8);
      occupiedRegion.lonRange.maximum = ParametricValue(1.0);
      occupiedRegion.segmentId = 1.;
      occupiedRegion.latRange.minimum = ParametricValue(0.2);
      occupiedRegion.latRange.maximum = ParametricValue(0.4);

      leadingObject.occupiedRegions.push_back(occupiedRegion);
    }

    followingObject = createObject(36., 0.);
    followingObject.objectId = 1;
    {
      world::OccupiedRegion occupiedRegion;
      occupiedRegion.lonRange.minimum = ParametricValue(0.1);
      occupiedRegion.lonRange.maximum = ParametricValue(0.2);
      occupiedRegion.segmentId = 1.;
      occupiedRegion.latRange.minimum = ParametricValue(0.6);
      occupiedRegion.latRange.maximum = ParametricValue(0.8);
      followingObject.occupiedRegions.push_back(occupiedRegion);
    }
  }

  virtual void TearDown()
  {
    followingObject.occupiedRegions.clear();
    leadingObject.occupiedRegions.clear();
    scene.egoVehicleRoad.clear();
  }

  world::RoadSegment longitudinalNoDifferenceRoadSegment()
  {
    world::RoadSegment roadSegment;
    world::LaneSegment laneSegment;

    laneSegment.id = 1;
    laneSegment.length.minimum = Distance(10);
    laneSegment.length.maximum = Distance(10);

    laneSegment.width.minimum = Distance(5);
    laneSegment.width.maximum = Distance(5);

    roadSegment.push_back(laneSegment);
    return roadSegment;
  }

  world::RoadSegment longitudinalDifferenceRoadSegment()
  {
    world::RoadSegment roadSegment;
    world::LaneSegment laneSegment;

    laneSegment.id = 1;
    laneSegment.length.minimum = Distance(5);
    laneSegment.length.maximum = Distance(10);

    laneSegment.width.minimum = Distance(5);
    laneSegment.width.maximum = Distance(5);
    roadSegment.push_back(laneSegment);
    return roadSegment;
  }

  world::Object followingObject;
  world::Object leadingObject;
  world::WorldModel worldModel;
  world::Scene scene;
  RssSituationExtraction situationExtraction;
};

TEST_F(RssSituationExtractionSameDirectionTests, noLongitudinalDifference)
{
  situation::SituationVector situationVector;

  scene.egoVehicle = objectAsEgo(leadingObject);
  scene.object = followingObject;

  scene.egoVehicleRoad.push_back(longitudinalNoDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  worldModel.timeIndex = 1;

  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);

  ASSERT_EQ(situationVector[0].relativePosition.longitudinalDistance, Distance(6));
  ASSERT_EQ(situationVector[0].egoVehicleState.velocity.speedLon, Speed(10));
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.accelMax,
            worldModel.egoVehicleRssDynamics.alphaLon.accelMax);

  ASSERT_EQ(situationVector[0].relativePosition.lateralPosition, situation::LateralRelativePosition::AtLeft);
  ASSERT_EQ(situationVector[0].relativePosition.lateralDistance, Distance(1));
}

TEST_F(RssSituationExtractionSameDirectionTests, longitudinalDifferenceEgoLeading)
{
  situation::SituationVector situationVector;

  scene.egoVehicle = objectAsEgo(leadingObject);
  scene.object = followingObject;
  scene.object.objectType = world::ObjectType::ArtificialObject;
  scene.egoVehicleRoad.push_back(longitudinalDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  worldModel.timeIndex = 1;

  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);

  ASSERT_EQ(situationVector[0].relativePosition.longitudinalDistance, Distance(2));
  ASSERT_EQ(situationVector[0].egoVehicleState.velocity.speedLon, Speed(10));
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.accelMax,
            worldModel.egoVehicleRssDynamics.alphaLon.accelMax);

  ASSERT_EQ(situationVector[0].relativePosition.lateralPosition, situation::LateralRelativePosition::AtLeft);
  ASSERT_EQ(situationVector[0].relativePosition.lateralDistance, Distance(1));
}

TEST_F(RssSituationExtractionSameDirectionTests, longitudinalDifferenceEgoFollowing)
{
  situation::SituationVector situationVector;

  scene.egoVehicle = objectAsEgo(followingObject);
  scene.object = leadingObject;
  scene.egoVehicleRoad.push_back(longitudinalDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  worldModel.timeIndex = 1;

  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);

  ASSERT_EQ(situationVector[0].relativePosition.longitudinalDistance, Distance(2));
  ASSERT_EQ(situationVector[0].egoVehicleState.velocity.speedLon, Speed(10));
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.accelMax, scene.objectRssDynamics.alphaLon.accelMax);
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.brakeMin, scene.objectRssDynamics.alphaLon.brakeMin);

  ASSERT_EQ(situationVector[0].relativePosition.lateralPosition, situation::LateralRelativePosition::AtRight);
  ASSERT_EQ(situationVector[0].relativePosition.lateralDistance, Distance(1));
}

TEST_F(RssSituationExtractionSameDirectionTests, mergeWorstCase)
{
  situation::SituationVector situationVector;

  scene.egoVehicle = objectAsEgo(followingObject);
  scene.object = leadingObject;

  scene.egoVehicleRoad.push_back(longitudinalDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  scene.egoVehicleRoad.clear();
  scene.egoVehicleRoad.push_back(longitudinalNoDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  worldModel.timeIndex = 1;

  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);

  ASSERT_EQ(situationVector[0].relativePosition.longitudinalDistance, Distance(2));
  ASSERT_EQ(situationVector[0].egoVehicleState.velocity.speedLon, Speed(10));
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.accelMax, scene.objectRssDynamics.alphaLon.accelMax);
  ASSERT_EQ(situationVector[0].egoVehicleState.dynamics.alphaLon.brakeMin, scene.objectRssDynamics.alphaLon.brakeMin);

  ASSERT_EQ(situationVector[0].relativePosition.lateralPosition, situation::LateralRelativePosition::AtRight);
  ASSERT_EQ(situationVector[0].relativePosition.lateralDistance, Distance(1));
}

TEST_F(RssSituationExtractionSameDirectionTests, mergeFails)
{
  situation::SituationVector situationVector;

  scene.egoVehicle = objectAsEgo(followingObject);
  scene.object = leadingObject;

  scene.egoVehicleRoad.push_back(longitudinalDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  scene.egoVehicleRoad.clear();
  scene.egoVehicleRoad.push_back(longitudinalNoDifferenceRoadSegment());
  worldModel.scenes.push_back(scene);
  worldModel.timeIndex = 1;

  // validate setup
  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);

  // adapt velocities
  auto originalObject = worldModel.scenes[0].egoVehicle;

  worldModel.scenes[0].egoVehicle.velocity.speedLat = Speed(2.2);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].egoVehicle = originalObject;

  worldModel.scenes[0].egoVehicle.velocity.speedLon = Speed(2.2);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].egoVehicle = originalObject;

  originalObject = worldModel.scenes[0].object;
  worldModel.scenes[0].object.velocity.speedLat = Speed(2.2);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].object = originalObject;

  worldModel.scenes[0].object.velocity.speedLon = Speed(2.2);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].object = originalObject;

  // adapt dynamics
  auto originalRssDynamics = worldModel.scenes[0].objectRssDynamics;
  worldModel.scenes[0].objectRssDynamics.alphaLat.accelMax = Acceleration(3.33);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.alphaLat.brakeMin = Acceleration(3.33);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.alphaLon.accelMax = Acceleration(3.33);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMax
    = worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMax + Acceleration(1.);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMin
    = worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMin - Acceleration(1.);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMinCorrect
    = worldModel.scenes[0].objectRssDynamics.alphaLon.brakeMinCorrect + Acceleration(.5);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.lateralFluctuationMargin = Distance(1.);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  worldModel.scenes[0].objectRssDynamics.responseTime = Duration(5.);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].objectRssDynamics = originalRssDynamics;

  // adapt lane correctness
  worldModel.scenes[0].egoVehicleRoad.front().front().drivingDirection = world::LaneDrivingDirection::Negative;
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].egoVehicleRoad.front().front().drivingDirection = world::LaneDrivingDirection::Positive;

  // influence relative position
  auto const originalOccupiedRegion = worldModel.scenes[0].egoVehicle.occupiedRegions;
  worldModel.scenes[0].egoVehicle.occupiedRegions.front().latRange.minimum = ParametricValue(0.);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].egoVehicle.occupiedRegions = originalOccupiedRegion;

  worldModel.scenes[0].egoVehicle.occupiedRegions.front().lonRange.maximum = ParametricValue(0.5);
  ASSERT_FALSE(situationExtraction.extractSituations(worldModel, situationVector));
  worldModel.scenes[0].egoVehicle.occupiedRegions = originalOccupiedRegion;

  // validate resetting of error setup in the above test code
  ASSERT_TRUE(situationExtraction.extractSituations(worldModel, situationVector));
  ASSERT_EQ(situationVector.size(), 1);
}

} // namespace core
} // namespace ad_rss
