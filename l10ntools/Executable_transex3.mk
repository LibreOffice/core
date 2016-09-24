# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,transex3))

$(eval $(call gb_Executable_set_include,transex3,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,transex3,\
    sal \
))

$(eval $(call gb_Executable_add_scanners,transex3,\
    l10ntools/source/srclex \
))

$(call gb_LexTarget_get_scanner_target,l10ntools/source/srclex) : T_LEXFLAGS := -w

$(eval $(call gb_Executable_use_static_libraries,transex3,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,transex3,\
    l10ntools/source/export \
))

$(eval $(call gb_Executable_use_externals,transex3,\
    boost_headers \
    libxml2 \
))

# vim:set noet sw=4 ts=4:
