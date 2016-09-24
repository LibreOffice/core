# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,cfgex))

$(eval $(call gb_Executable_set_include,cfgex,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,cfgex,\
    sal \
))

$(eval $(call gb_Executable_add_scanners,cfgex,\
    l10ntools/source/cfglex \
))

$(eval $(call gb_Executable_use_static_libraries,cfgex,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,cfgex,\
    l10ntools/source/cfgmerge \
))

$(eval $(call gb_Executable_use_externals,cfgex,\
    boost_headers \
    libxml2 \
))

# vim:set noet sw=4 ts=4:
