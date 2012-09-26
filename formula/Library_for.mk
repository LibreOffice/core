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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Library_Library,for))

$(eval $(call gb_Library_use_package,for,\
	formula_inc \
))

$(eval $(call gb_Library_set_include,for,\
    $$(INCLUDE) \
    -I$(SRCDIR)/formula/source/core/inc \
))

$(eval $(call gb_Library_add_defs,for,\
    -DFORMULA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,for))

$(eval $(call gb_Library_use_libraries,for,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svl \
    svt \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,for))

$(eval $(call gb_Library_set_componentfile,for,formula/util/for))

$(eval $(call gb_Library_add_exception_objects,for,\
    formula/source/core/api/FormulaCompiler \
    formula/source/core/api/FormulaOpCodeMapperObj \
    formula/source/core/api/services \
    formula/source/core/api/token \
    formula/source/core/resource/core_resource \
))

# Runtime dependency for unit-tests
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,for)) :| \
	$(call gb_AllLangResTarget_get_target,for)

# vim: set noet sw=4 ts=4:
