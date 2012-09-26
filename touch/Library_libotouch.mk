# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright 2012 LibreOffice contributors.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Library_Library,libotouch))

$(eval $(call gb_Library_set_include,libotouch,\
    -I$(SRCDIR)/libotouch/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,libotouch))

$(eval $(call gb_Library_use_internal_comprehensive_api,libotouch,\
	touch \
))

$(eval $(call gb_Library_use_libraries,libotouch,\
))

$(eval $(call gb_Library_add_standard_system_libs,libotouch))

$(eval $(call gb_Library_add_exception_objects,libotouch,\
	touch/source/uno/Document \
	touch/source/generic/libotouch \
))

ifeq ($(OS),ANDROID)

$(eval $(call gb_Library_use_libraries,libotouch,\
	lo-bootstrap \
))

$(eval $(call gb_Library_add_exception_objects,libotouch,\
	touch/source/android/android \
))

endif

ifeq ($(OS),IOS)

$(eval $(call gb_Library_add_objcxx_objects,libotouch,\
	touch/source/ios/ios \
))

endif

# vim: set noet sw=4 ts=4:
