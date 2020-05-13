# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,box2d))

$(eval $(call gb_StaticLibrary_use_unpacked,box2d,box2d))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,box2d))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,box2d,cpp))

$(eval $(call gb_StaticLibrary_set_include,box2d,\
    -I$(call gb_UnpackedTarball_get_dir,box2d/Box2D/)\
    $$(INCLUDE)\
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,box2d,\
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2Timer \
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2StackAllocator \
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2Draw \
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2Math \
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2BlockAllocator \
	UnpackedTarball/box2d/Box2D/Box2D/Common/b2Settings \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2Body \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2PolygonAndCircleContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2CircleContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2ContactSolver \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2PolygonContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2ChainAndPolygonContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2ChainAndCircleContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2Contact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Contacts/b2EdgeAndCircleContact \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2RopeJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2WheelJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2FrictionJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2PrismaticJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2WeldJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2PulleyJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2MouseJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2MotorJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2DistanceJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2GearJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2Joint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/Joints/b2RevoluteJoint \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2WorldCallbacks \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2Fixture \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2ContactManager \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2Island \
	UnpackedTarball/box2d/Box2D/Box2D/Dynamics/b2World \
	UnpackedTarball/box2d/Box2D/Box2D/Rope/b2Rope \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2TimeOfImpact \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2CollidePolygon \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2Distance \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2Collision \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2CollideEdge \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2CollideCircle \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2BroadPhase \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/Shapes/b2EdgeShape \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/Shapes/b2CircleShape \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/Shapes/b2ChainShape \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/Shapes/b2PolygonShape \
	UnpackedTarball/box2d/Box2D/Box2D/Collision/b2DynamicTree \
))

# vim: set noet sw=4 ts=4:
