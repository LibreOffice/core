# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,java_websocket))

$(eval $(call gb_ExternalProject_register_targets,java_websocket,\
	build \
))

$(call gb_ExternalProject_get_state_target,java_websocket,build) :
	$(call gb_Trace_StartRange,java_websocket,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		unset MSYS_NO_PATHCONV && JAVA_HOME=$(JAVA_HOME_FOR_BUILD) \
		$(ICECREAM_RUN) "$(ANT)" \
			$(if $(verbose),-v,-q) \
			-f build.xml \
			-Dbuild.label="build-$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)" \
			-Dant.build.javac.source=$(JAVA_SOURCE_VER) \
			-Dant.build.javac.target=$(JAVA_TARGET_VER) \
			$(if $(debug),-Dbuild.debug="on") \
			jar \
	)
	$(call gb_Trace_EndRange,java_websocket,EXTERNAL)

# vim: set noet sw=4 ts=4:
