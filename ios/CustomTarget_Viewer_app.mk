# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,ios/Viewer_app))

# We distinguish between builds for the simulator and device by
# looking for the "iarmv7" or "i386" in the -arch option that is part
# of $(CC)

ifneq ($(filter i386,$(CC)),)
xcode_sdk=iphonesimulator
xcode_arch=i386
else
xcode_sdk=iphoneos
xcode_arch=armv7
endif

ifeq ($(debug),)
xcode_config := Release
else
xcode_config := Debug
endif

# If run from Xcode, check that its configuration (device or
# simulator) matches that of gbuild. We detect being run from Xcode by
# looking for $(XCODE_VERSION_ACTUAL)

ifneq ($(XCODE_VERSION_ACTUAL),)

export CCACHE_CPP2=y

ifneq ($(CURRENT_ARCH),$(xcode_arch))
$(error Xcode platform (device or simulator) does not match that of this build tree)
endif

ifneq ($(CONFIGURATION),$(xcode_config))
$(error Xcode configuration (release or debug) does not match that of this build tree)
endif

endif

APP := Viewer

ios_Viewer_app_DIR := $(SRC_ROOT)/ios/experimental/Viewer/DerivedData/$(APP)/Build/Products/$(xcode_config)-$(xcode_sdk)/$(APP).app

ios_Viewer_app_EXE := $(ios_Viewer_app_DIR)/Viewer

$(call gb_CustomTarget_get_target,ios/Viewer_app) : $(ios_Viewer_app_EXE)

$(ios_Viewer_app_EXE): $(call gb_Executable_get_target,Viewer)
	mkdir -p $(ios_Viewer_app_DIR)
	cp $(call gb_Executable_get_target,Viewer) $(ios_Viewer_app_DIR)

# vim: set noet sw=4 ts=4:
