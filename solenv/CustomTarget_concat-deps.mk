# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,solenv/concat-deps))

$(call gb_CustomTarget_get_target,solenv/concat-deps) : \
	$(call gb_CustomTarget_get_workdir,solenv/concat-deps)/concat-deps

$(call gb_CustomTarget_get_workdir,solenv/concat-deps)/concat-deps : \
		$(SRCDIR)/solenv/bin/concat-deps.c \
		| $(call gb_CustomTarget_get_workdir,solenv/concat-deps)/.dir
	$(call gb_Output_announce,solenv/concat-deps,$(true),GCC,1)
ifeq ($(COM_FOR_BUILD),MSC)
	# on cygwin force the use of gcc
	gcc -O2 $< -o $@
else
	$(CC_FOR_BUILD) -O2 $< -o $@
endif

# vim: set noet sw=4 ts=4:
