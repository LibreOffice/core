# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,imath))

#$(eval $(call gb_Library_set_precompiled_header,imath,imath/inc/pch/precompiled_imath))

#$(eval $(call gb_Library_use_custom_headers,imath,imath/generated))

$(eval $(call gb_Library_set_include,imath,\
    -I$(SRCDIR)/imath/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_cxxflags,imath,\
    -fvisibility=default \
))

$(eval $(call gb_Library_add_defs,imath,\
	-DINSIDE_SM \
	-DOO_IS_AOO=0 \
        -DOO_MAJOR_VERSION=$(LIBO_VERSION_MAJOR) \
        -DOO_MINOR_VERSION=$(LIBO_VERSION_MINOR) \
))

$(eval $(call gb_Library_use_externals,imath,\
	cln \
	ginac \
))

$(eval $(call gb_Library_use_libraries,imath,\
        comphelper \
        cppu \
        cppuhelper \
        imath \
        sal \
))

$(eval $(call gb_Library_use_sdk_api,imath))

$(eval $(call gb_Library_add_exception_objects,imath,\
    imath/source/alignblock \
    imath/source/differential \
    imath/source/eqc \
    imath/source/equation \
    imath/source/exderivative \
    imath/source/expression \
    imath/source/extintegral \
    imath/source/extsymbol \
    imath/source/func \
    imath/source/imathutils \
    imath/source/iFormulaLine \
    imath/source/iIterator \
    imath/source/msgdriver \
    imath/source/operands \
    imath/source/option \
    imath/source/printing \
    imath/source/settingsmanager \
    imath/source/stringex \
    imath/source/unit \
    imath/source/unitmgr \
    imath/source/utils \
))

# vim: set noet sw=4 ts=4:
