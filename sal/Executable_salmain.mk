# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,salmain))

$(eval $(call gb_Executable_set_include,salmain,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,salmain,\
    -DSAL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Executable_use_libraries,salmain,\
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,salmain,\
    sal/workben/salmain \
))

$(eval $(call gb_Executable_Executable,salmainargs))

$(eval $(call gb_Executable_set_include,salmainargs,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,salmainargs,\
    -DSAL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Executable_use_libraries,salmainargs,\
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,salmainargs,\
    sal/workben/salmainargs \
))
# vim: set ts=4 sw=4 et:
