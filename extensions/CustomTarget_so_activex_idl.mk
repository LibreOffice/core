# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extensions/source/activex/idl))

extensions_AXIDLDIR := $(gb_CustomTarget_workdir)/extensions/source/activex/idl

$(call gb_CustomTarget_get_target,extensions/source/activex/idl) : \
	$(extensions_AXIDLDIR)/so_activex.tlb

# XXX: I presume that the "$(COM)"=="GCC" case in the original
# extensions/source/activex/msidl/makefile.mk was for the
# use-mingw-on-windows case and thus is not interesting for us.
$(extensions_AXIDLDIR)/so_activex.tlb : \
		$(SRCDIR)/extensions/source/activex/so_activex.idl \
		| $(extensions_AXIDLDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IDL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),IDL)
	$(call gb_Helper_abbreviate_dirs, \
	midl.exe \
		-tlb $@ \
		-h $(extensions_AXIDLDIR)/so_activex.h \
		-iid $(extensions_AXIDLDIR)/so_activex_i.c \
		-dlldata $(extensions_AXIDLDIR)/so_activex_dll.c \
		-proxy $(extensions_AXIDLDIR)/so_activex_p.c \
		-Oicf \
		$(INCLUDE) \
		$<)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),IDL)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
