# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,odk_checkapi))

$(eval $(call gb_CppunitTest_add_exception_objects,odk_checkapi,\
    odk/qa/checkapi/checkapi \
))

$(eval $(call gb_CppunitTest_add_cxxflags,odk_checkapi,\
	$(gb_CXX03FLAGS) \
	$(if $(ENABLE_GTK3),$(GTK3_CFLAGS)) \
))

$(eval $(call gb_CppunitTest_set_external_code,odk_checkapi))

# The remaining lines must be in the given order, to set up a set of include paths that only
# contains the instdir SDK include directory, plus SOLARINC as needed for platform-specific
# includes, plus the workdir sub-directory containing the generated allheaders.hxx and the workdir
# sub-directory containing the generated UNOIDL include files (which are not bundled in the SDK but
# would rather get created on demand by the SDK's makefiles), plus whatever is needed for CppUnit:

$(eval $(call gb_CppunitTest_set_include,odk_checkapi, \
    -I$(INSTDIR)/$(SDKDIRNAME)/include $(SOLARINC)))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,odk_checkapi,\
	-ldl \
))
endif

$(eval $(call gb_CppunitTest_use_custom_headers,odk_checkapi,\
	odk/allheaders \
))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,odk_checkapi,\
	udkapi \
))

$(eval $(call gb_CppunitTest_use_external,odk_checkapi,cppunit))

# vim: set noet sw=4 ts=4:
