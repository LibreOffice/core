# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cppunit))

$(eval $(call gb_UnpackedTarball_set_tarball,cppunit,$(CPPUNIT_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	cppunit/windows.patch \
	cppunit/unix.patch \
))
ifeq ($(OS),ANDROID)
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	cppunit/android.patch \
))
else ifeq ($(OS),IOS)
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	cppunit/ios.patch \
))
endif
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cppunit,\
	cppunit/disable-dynloading.patch \
))
endif
$(eval $(call gb_UnpackedTarball_add_file,cppunit,src/cppunit/cppunit_dll.vcproj,cppunit/src/cppunit_dll.vcproj))
$(eval $(call gb_UnpackedTarball_add_file,cppunit,src/cppunit/cppunit_dll.vcxproj,cppunit/src/cppunit_dll.vcxproj))
$(eval $(call gb_UnpackedTarball_add_file,cppunit,src/DllPlugInTester/DllPlugInTester.vcproj,cppunit/src/DllPlugInTester.vcproj))
$(eval $(call gb_UnpackedTarball_add_file,cppunit,src/DllPlugInTester/DllPlugInTester.vcxproj,cppunit/src/DllPlugInTester.vcxproj))

# vim: set noet sw=4 ts=4:
