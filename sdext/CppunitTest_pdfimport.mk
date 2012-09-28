# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CppunitTest_CppunitTest,sdext_pdfimport))

$(eval $(call gb_CppunitTest_add_defs,sdext_pdfimport,\
    -DPDFIMPORT_EXECUTABLE_LOCATION=\"$(call gb_Helper_make_url,\
        $(dir $(call gb_Executable_get_target,xpdfimport)))\" \
))

$(eval $(call gb_CppunitTest_use_api,sdext_pdfimport,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_set_include,sdext_pdfimport,\
    -I$(SRCDIR)/sdext/source/pdfimport/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_externals,sdext_pdfimport,\
    zlib \
))

$(eval $(call gb_CppunitTest_use_libraries,sdext_pdfimport,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_static_libraries,sdext_pdfimport,\
    pdfimport_s \
))

$(eval $(call gb_CppunitTest_use_library_objects,sdext_pdfimport,basegfx))

$(eval $(call gb_CppunitTest_add_exception_objects,sdext_pdfimport,\
    sdext/source/pdfimport/test/tests \
))

$(eval $(call gb_CppunitTest_use_executable,sdext_pdfimport,xpdfimport))

$(eval $(call gb_CppunitTest_use_ure,sdext_pdfimport))

$(eval $(call gb_CppunitTest_use_components,sdext_pdfimport,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,sdext_pdfimport))

# vim:set shiftwidth=4 softtabstop=4 expandtab:
