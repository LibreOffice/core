# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,cppunit,cppunit))
$(eval $(call gb_ExternalPackage_use_external_project,cppunit,cppunit))

ifeq ($(OS),WNT)
ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/cppunit_dll.dll,src/cppunit/ReleaseDll/cppunit_dll.dll))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/icppunit_dll.lib,src/cppunit/ReleaseDll/cppunit_dll.lib))
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/DllPlugInTester_dll.exe,src/DllPlugInTester/ReleaseDll/DllPlugInTester_dll.exe))
else
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/libcppunit-1-13-0.dll,src/cppunit/.libs/libcppunit-1-13-0.dll))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.dll.a,src/cppunit/.libs/libcppunit.dll.a))
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/DllPlugInTester.exe,src/DllPlugInTester/.libs/DllPlugInTester.exe))
endif
else
ifneq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/DllPlugInTester,src/DllPlugInTester/.libs/DllPlugInTester))
else
ifneq ($(filter DESKTOP,$(BUILD_TYPE)),)
$(eval $(call gb_ExternalPackage_add_file,cppunit,bin/DllPlugInTester,src/DllPlugInTester/DllPlugInTester))
endif
endif
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit-1.13.0.dylib,src/cppunit/.libs/libcppunit-1.13.0.dylib))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.dylib,src/cppunit/.libs/libcppunit.dylib))
else ifeq ($(OS),AIX)
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit-1.13.a,src/cppunit/.libs/libcppunit-1.13.a))
else ifeq ($(OS),OPENBSD)
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit-1.13.so.0.0,src/cppunit/.libs/libcppunit-1.13.so.0.0))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.so,src/cppunit/.libs/libcppunit-1.13.so.0.0))
else ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.a,src/cppunit/.libs/libcppunit.a))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit-1.13.so,src/cppunit/.libs/libcppunit-1.13.so))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.so,src/cppunit/.libs/libcppunit-1.13.so))
else
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit-1.13.so.0,src/cppunit/.libs/libcppunit-1.13.so.0.0.1))
$(eval $(call gb_ExternalPackage_add_file,cppunit,lib/libcppunit.so,src/cppunit/.libs/libcppunit-1.13.so.0.0.1))
endif
endif

# vim: set noet sw=4 ts=4:
