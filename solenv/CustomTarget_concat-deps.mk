# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,solenv/concat-deps))

$(eval $(call gb_CustomTarget_register_targets,solenv/concat-deps,\
	concat-deps$(gb_Executable_EXT_for_build) \
))

$(call gb_CustomTarget_get_workdir,solenv/concat-deps)/concat-deps$(gb_Executable_EXT_for_build) : \
		$(SRCDIR)/solenv/bin/concat-deps.c
	$(call gb_Output_announce,solenv/concat-deps,$(true),C,1)
ifeq ($(COM_FOR_BUILD),MSC)
	LIB="$(ILIB)" $(CC_FOR_BUILD) -nologo $(SOLARINC) -O2 $< -Fo$(dir $@) -Fe$(dir $@)
else
	$(CC_FOR_BUILD) -O2 $< -o $@
endif

# vim: set noet sw=4 ts=4:
