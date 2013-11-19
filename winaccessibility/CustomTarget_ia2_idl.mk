# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,winaccessibility/ia2/idl))

wina11y_COMIDLDIR := $(call gb_CustomTarget_get_workdir,winaccessibility/ia2/idl)
wina11y_SOURCE := $(SRCDIR)/winaccessibility/source/UAccCOMIDL

# We cannot depend on *.tlb because they only produced by IDL compiler
# if idl contains 'library' statement.
$(call gb_CustomTarget_get_target,winaccessibility/ia2/idl) : \
	$(wina11y_COMIDLDIR)/AccessibleKeyBinding.h \
	$(wina11y_COMIDLDIR)/AccessibleKeyStroke.h \
	$(wina11y_COMIDLDIR)/Charset.h \
	$(wina11y_COMIDLDIR)/UAccCOM.h \
	$(wina11y_COMIDLDIR)/defines.h \
	$(wina11y_COMIDLDIR)/ia2_api_all.h

$(wina11y_COMIDLDIR)/%.h : $(wina11y_SOURCE)/%.idl \
		| $(wina11y_COMIDLDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),IDL,1)
	$(call gb_Helper_abbreviate_dirs, \
	midl.exe \
		-tlb $(wina11y_COMIDLDIR)/$*.tlb \
		-h $(wina11y_COMIDLDIR)/$*.h \
		-iid $(wina11y_COMIDLDIR)/$*_i.c \
		-dlldata $(wina11y_COMIDLDIR)/dlldata.c \
		-proxy $(wina11y_COMIDLDIR)/$*_p.c \
		-Oicf \
		$(SOLARINC) \
		-I $(wina11y_SOURCE) \
		$<)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
