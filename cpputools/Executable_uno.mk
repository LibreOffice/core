# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,uno))

$(eval $(call gb_Executable_use_internal_comprehensive_api,uno,\
    udkapi \
))

$(eval $(call gb_Executable_use_libraries,uno,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_externals,uno,\
    libxml2 \
))

$(eval $(call gb_Executable_add_exception_objects,uno,\
    cpputools/source/unoexe/unoexe \
))

# vim:set noet sw=4 ts=4:
