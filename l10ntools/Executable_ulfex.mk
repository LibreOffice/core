# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,ulfex))

$(eval $(call gb_Executable_set_include,ulfex,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,ulfex,\
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,ulfex,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,ulfex,\
    l10ntools/source/lngmerge \
    l10ntools/source/lngex \
))

$(eval $(call gb_Executable_use_externals,ulfex,\
    boost_headers \
    libxml2 \
))

# vim:set noet sw=4 ts=4:
