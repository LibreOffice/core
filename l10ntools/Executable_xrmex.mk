# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,xrmex))

$(eval $(call gb_Executable_set_include,xrmex,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,xrmex,\
    sal \
))

$(eval $(call gb_Executable_add_scanners,xrmex,\
    l10ntools/source/xrmlex \
))

$(eval $(call gb_Executable_use_static_libraries,xrmex,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,xrmex,\
    l10ntools/source/xrmmerge \
))

$(eval $(call gb_Executable_use_externals,xrmex,\
    boost_headers \
    libxml2 \
    icudata \
    icuuc \
    icui18n \
    icu_headers \
))

# vim:set noet sw=4 ts=4:
