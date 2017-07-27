# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,alloca))

$(eval $(call gb_Executable_set_include,alloca,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,alloca,\
    -DSAL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Executable_use_libraries,alloca,\
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,alloca,\
    sal/workben/alloca \
))

$(call gb_Executable_get_clean_target,alloca) :
	rm -f $(WORKDIR)/LinkTarget/Executable/alloca
# vim: set ts=4 sw=4 et:
