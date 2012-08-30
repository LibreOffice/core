# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# convert old po to new po
#

$(eval $(call gb_Executable_Executable,renewpo))

$(eval $(call gb_Executable_set_include,renewpo,\
    -I$(SRCDIR)/l10ntools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_libraries,renewpo,\
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,renewpo,\
    transex \
))

$(eval $(call gb_Executable_add_exception_objects,renewpo,\
    l10ntools/source/renewpo \
))

# vim: set noet sw=4 ts=4:
