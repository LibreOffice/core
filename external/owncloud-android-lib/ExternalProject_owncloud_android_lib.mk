# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,owncloud_android_lib))

$(eval $(call gb_ExternalProject_register_targets,owncloud_android_lib,\
	build \
))

$(call gb_ExternalProject_get_state_target,owncloud_android_lib,build) :
	$(call gb_ExternalProject_run,build,\
	$(ICECREAM_RUN) "$(ANT)" \
		$(if $(verbose),-v,-q) \
		-f build.xml \
		-Dsdk.dir=$(ANDROID_SDK_HOME) -Dtarget=android-22 \
		release \
	)

# vim: set noet sw=4 ts=4:
