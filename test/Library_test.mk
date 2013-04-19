# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,test))

$(eval $(call gb_Library_use_package,test,test_inc))

$(eval $(call gb_Library_add_defs,test,\
    -DOOO_DLLIMPLEMENTATION_TEST \
))

$(eval $(call gb_Library_use_sdk_api,test))

$(eval $(call gb_Library_use_externals,test,\
	boost_headers \
	cppunit \
	libxml2 \
))

$(eval $(call gb_Library_use_libraries,test,\
    comphelper \
    cppu \
    cppuhelper \
	i18nlangtag \
    sal \
    svt \
	tl \
	utl \
	unotest \
	vcl \
	$(gb_UWINAPI) \
))

ifeq ($(GUIBASE),unx)
$(call gb_Library_get_target,test) : \
    $(call gb_Library_get_target,desktop_detector) \
    $(call gb_Library_get_target,vclplug_svp) \

endif

$(eval $(call gb_Library_add_exception_objects,test,\
    test/source/bootstrapfixture \
    test/source/diff/diff \
))

# vim: set noet sw=4 ts=4:
