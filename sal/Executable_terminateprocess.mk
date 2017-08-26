# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,terminateprocess))

$(eval $(call gb_Executable_set_include,terminateprocess,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sal/inc \
))

$(eval $(call gb_Library_add_defs,terminateprocess,\
    -DSAL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Executable_use_libraries,terminateprocess,\
    sal \
))

$(eval $(call gb_Executable_add_exception_objects,terminateprocess,\
    sal/workben/osl/process/terminateprocess \
))

$(call gb_Executable_get_clean_target,terminateprocess) :
	rm -f $(WORKDIR)/LinkTarget/Executable/terminateprocess
# vim: set ts=4 sw=4 et:
