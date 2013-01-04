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
# looking for $(SCRIPT_OUTPUT_FILE_0). The Run Script build phase in
# our project has as its (single) output file the location of the app
# executable in its app bundle.

ifneq ($(SCRIPT_OUTPUT_FILE_0),)

export CCACHE_CPP2=y

ifneq ($(CURRENT_ARCH),$(xcode_arch))
$(error Xcode platform (device or simulator) does not match that of this build tree)
endif

ifneq ($(CONFIGURATION),$(xcode_config))
$(error Xcode configuration (release or debug) does not match that of this build tree)
endif

endif

ifneq ($(SCRIPT_OUTPUT_FILE_0),)
# When run from Xcode, we move the Viewer executable from solver into
# the Viewer.app directory that Xcode uses.
$(call gb_CustomTarget_get_target,ios/Viewer_app) : $(SCRIPT_OUTPUT_FILE_0)

$(SCRIPT_OUTPUT_FILE_0) : $(call gb_Executable_get_target,Viewer)
	$(call gb_Output_announce,$@,fii,APP,2)
	mkdir -p `dirname $(SCRIPT_OUTPUT_FILE_0)`
	mv $(call gb_Executable_get_target,Viewer) $(SCRIPT_OUTPUT_FILE_0)

else
# When run just from the command line, we don't have any app bundle to
# copy or move the executable to. So do nothing.
$(call gb_CustomTarget_get_target,ios/Viewer_app) : $(call gb_Executable_get_target,Viewer)

$(call gb_CustomTarget_get_clean_target,ios/Viewer_app) :
# Here we just assume that Xcode's settings are default, or something
	rm -rf experimental/Viewer/build

endif

# vim: set noet sw=4 ts=4:
