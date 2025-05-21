# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_accessible_relation_set))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_accessible_relation_set))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_accessible_relation_set, \
    sw/qa/extras/accessibility/accessible_relation_set \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_accessible_relation_set, \
    comphelper \
    cppu \
	cppuhelper \
	i18nlangtag \
    sal \
    subsequenttest \
    sw \
    test \
    tk \
	tl \
    unotest \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_accessible_relation_set,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_set_include,sw_accessible_relation_set,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_accessible_relation_set,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_accessible_relation_set))
$(eval $(call gb_CppunitTest_use_vcl,sw_accessible_relation_set))

$(eval $(call gb_CppunitTest_use_rdb,sw_accessible_relation_set,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_accessible_relation_set))

# we need to explicitly depend on Library_acc because it's dynamically loaded
$(call gb_CppunitTest_get_target,sw_accessible_relation_set) : $(call gb_Library_get_target,acc)

# vim: set noet sw=4 ts=4:
