# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

info_WORKDIR := $(gb_CustomTarget_workdir)/sysui/infoplist
info_SRCDIR := $(SRCDIR)/sysui/desktop/macosx
info_BUILDDIR := $(BUILDDIR)/sysui/desktop/macosx


$(eval $(call gb_CustomTarget_CustomTarget,sysui/infoplist))

$(eval $(call gb_CustomTarget_register_targets,sysui/infoplist,\
	PkgInfo \
	Info.plist \
	$(foreach lang,$(filter ca cs da de el en es fi fr hr hu id it ja ko ms nl no pl pt pt_PT ro ru sk sv th tr uk vi zh_CN zh_TW,$(gb_WITH_LANG)),\
	InfoPlist_$(lang)/InfoPlist.strings) \
))

$(info_WORKDIR)/PkgInfo:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	echo "APPLLIBO" > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)

$(info_WORKDIR)/Info.plist: $(info_BUILDDIR)/Info.plist
	cp $< $@

$(info_WORKDIR)/InfoPlist_%/InfoPlist.strings: \
		$(info_WORKDIR)/Info.plist $(info_WORKDIR)/documents.ulf
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) -w $(info_SRCDIR)/gen_strings.pl -l $* -p $^ | \
	iconv -f UTF-8 -t UTF-16 >$@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(info_WORKDIR)/documents.ulf,\
	$(SRCDIR)/sysui/desktop/share/documents.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/sysui/desktop/share.po)))

# vim: set noet sw=4 ts=4:
