# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,shell/source/win32/spsupp/idl))

spsupp_IDLDIR := $(gb_CustomTarget_workdir)/shell/source/win32/spsupp/idl

$(call gb_CustomTarget_get_target,shell/source/win32/spsupp/idl) : \
	$(spsupp_IDLDIR)/spsupp.tlb

$(spsupp_IDLDIR)/spsupp.tlb : \
		$(SRCDIR)/shell/source/win32/spsupp/spsupp.idl \
		| $(spsupp_IDLDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IDL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),IDL)
	$(call gb_Helper_abbreviate_dirs, \
	midl.exe \
		-tlb $@ \
		-h $(spsupp_IDLDIR)/spsupp_h.h \
		-iid $(spsupp_IDLDIR)/spsupp_i.c \
		$(INCLUDE) \
		$(SOLARINC) \
		-Oicf \
		$<)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),IDL)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
