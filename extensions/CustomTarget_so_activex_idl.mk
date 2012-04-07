# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,extensions/source/activex/idl))

EXAI := $(call gb_CustomTarget_get_workdir,extensions/source/activex/idl)

$(call gb_CustomTarget_get_target,extensions/source/activex/idl) : \
	$(EXAI)/so_activex.tlb

# XXX: I presume that the "$(COM)"=="GCC" case in the original
# extensions/source/activex/msidl/makefile.mk was for the
# use-mingw-on-windows case and thus is not interesting for us.
$(EXAI)/so_activex.tlb : \
		$(SRCDIR)/extensions/source/activex/so_activex.idl | $(EXAI)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IDL,1)
	$(call gb_Helper_abbreviate_dirs_native, \
	midl.exe \
		-tlb $@ \
		-h $(EXAI)/so_activex.h \
		-iid $(EXAI)/so_activex_i.c \
		-dlldata $(EXAI)/so_activex_dll.c \
		-proxy $(EXAI)/so_activex_p.c \
		-Oicf \
		$(INCLUDE) \
		$<)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
