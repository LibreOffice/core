# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,transex))

$(eval $(call gb_StaticLibrary_set_include,transex,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_use_externals,transex,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,transex,\
    l10ntools/source/helper \
    l10ntools/source/common \
    l10ntools/source/merge \
    l10ntools/source/po \
))

# vim: set noet sw=4 ts=4:
