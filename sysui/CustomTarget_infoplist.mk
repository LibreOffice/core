# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

info_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/infoplist)
info_SRCDIR := $(SRCDIR)/sysui/desktop/macosx
info_BUILDDIR := $(BUILDDIR)/sysui/desktop/macosx


$(eval $(call gb_CustomTarget_CustomTarget,sysui/infoplist))

$(eval $(call gb_CustomTarget_register_targets,sysui/infoplist,\
	PkgInfo \
	Info.plist \
	$(foreach lang,en-US $(gb_WITH_LANG),\
	InfoPlist_$(lang).zip InfoPlist_$(lang)/InfoPlist.strings) \
))

$(info_WORKDIR)/PkgInfo:
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo "APPLLIBO" > $@

$(info_WORKDIR)/Info.plist: $(info_BUILDDIR)/Info.plist
	cp $< $@

$(info_WORKDIR)/InfoPlist_%.zip: $(info_WORKDIR)/InfoPlist_%/InfoPlist.strings
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ZIP,1)
	zip -j $@ $<

$(info_WORKDIR)/InfoPlist_%/InfoPlist.strings: \
		$(info_WORKDIR)/Info.plist $(info_WORKDIR)/documents.ulf
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(PERL) -w $(info_SRCDIR)/gen_strings.pl -l $* -p $^ | \
	iconv -f UTF-8 -t UTF-16 >$@

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(info_WORKDIR)/documents.ulf,\
	$(SRCDIR)/sysui/desktop/share/documents.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/sysui/desktop/share.po)))

# vim: set noet sw=4 ts=4:
