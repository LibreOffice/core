# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,helpex))

$(eval $(call gb_Executable_set_include,helpex,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,helpex,\
    sal \
))

$(eval $(call gb_Executable_use_externals,helpex,\
	expat \
	boost_headers \
	icu_headers \
    libxml2 \
	icudata \
    icuuc \
    icui18n \
))

$(eval $(call gb_Executable_use_static_libraries,helpex,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,helpex,\
    l10ntools/source/helpex \
    l10ntools/source/xmlparse \
    l10ntools/source/helpmerge \
))

# vim:set noet sw=4 ts=4:
