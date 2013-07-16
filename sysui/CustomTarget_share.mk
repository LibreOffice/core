# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sysui/productlist.mk

ifeq ($(ENABLE_GIO),TRUE)
	brand_URIPARAM := --urls
else
ifeq ($(ENABLE_GNOMEVFS),TRUE)
	brand_URIPARAM := --urls
else
	brand_URIPARAM :=
endif
endif

share_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/share)
share_SRCDIR := $(SRCDIR)/sysui/desktop

share_TRANSLATE := $(SOLARENV)/bin/desktop-translate.pl

LAUNCHERLIST_APPS := writer calc draw impress math base startcenter
LAUNCHERLIST := $(LAUNCHERLIST_APPS) qstart xsltfilter
LAUNCHERS := $(foreach launcher,$(LAUNCHERLIST),$(share_SRCDIR)/menus/$(launcher).desktop)

MIMELIST := \
    text \
    text-template \
    spreadsheet \
    spreadsheet-template \
    drawing \
    drawing-template \
    presentation \
    presentation-template \
    formula \
    master-document \
    oasis-text \
    oasis-text-template \
    oasis-text-flat-xml \
    oasis-spreadsheet \
    oasis-spreadsheet-template \
    oasis-spreadsheet-flat-xml \
    oasis-drawing \
    oasis-drawing-template \
    oasis-drawing-flat-xml \
    oasis-presentation \
    oasis-presentation-template \
    oasis-presentation-flat-xml \
    oasis-formula \
    oasis-master-document \
    oasis-database \
    oasis-web-template \
    ms-excel-sheet-12 \
    ms-excel-sheet \
    ms-excel-template-12 \
    ms-powerpoint-presentation-12 \
    ms-powerpoint-presentation \
    ms-powerpoint-template-12 \
    ms-word-document-12 \
    ms-word-document \
    ms-word-document2 \
    ms-word-template-12 \
    openxmlformats-officedocument-presentationml-presentation \
    openxmlformats-officedocument-presentationml-template \
    openxmlformats-officedocument-spreadsheetml-sheet \
    openxmlformats-officedocument-spreadsheetml-template \
    openxmlformats-officedocument-wordprocessingml-document \
    openxmlformats-officedocument-wordprocessingml-template \
    ms-excel-sheet-binary-12	\
    extension

MIMEICONLIST := \
    oasis-text \
    oasis-text-template \
    oasis-spreadsheet \
    oasis-spreadsheet-template \
    oasis-drawing \
    oasis-drawing-template \
    oasis-presentation \
    oasis-presentation-template \
    oasis-formula \
    oasis-master-document \
    oasis-database \
    oasis-web-template \
    text \
    text-template \
    spreadsheet \
    spreadsheet-template \
    drawing \
    drawing-template \
    presentation \
    presentation-template \
    formula \
    master-document \
    database \
    extension

share_ICONS := $(foreach size,16x16 32x32 48x48,\
	$(foreach contrast,hicolor locolor,\
	$(foreach app,$(LAUNCHERLIST_APPS),\
	$(share_SRCDIR)/icons/$(contrast)/$(size)/apps/$(app).png) \
	$(foreach mime,$(MIMEICONLIST),\
	$(share_SRCDIR)/icons/$(contrast)/$(size)/mimetypes/$(mime).png)))

MIMEKEYS := $(foreach mime,$(MIMELIST),$(share_SRCDIR)/mimetypes/$(mime).keys)
MIMEDESKTOPS := $(foreach mime,$(MIMELIST),$(share_SRCDIR)/mimetypes/$(mime).desktop)
ULFS := documents.ulf \
	launcher_comment.ulf \
	launcher_genericname.ulf \
	launcher_unityquicklist.ulf

$(eval $(call gb_CustomTarget_CustomTarget,sysui/share))

$(eval $(call gb_CustomTarget_register_targets,sysui/share,\
	$(ULFS) \
	$(foreach product,$(PRODUCTLIST),\
	$(product)/build.flag \
	$(product)/openoffice.org.xml \
	$(product)/openoffice.mime \
	$(product)/openoffice.applications \
	$(product)/openoffice.keys \
	$(product)/openoffice.sh \
	$(product)/printeradmin.sh \
	$(product)/create_tree.sh \
	$(product)/mimelnklist \
	$(product)/launcherlist) \
))

$(share_WORKDIR)/%/openoffice.org.xml: $(share_WORKDIR)/documents.ulf $(MIMEDESKTOPS) $(share_SRCDIR)/share/create_mime_xml.pl
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(PERL) $(share_SRCDIR)/share/create_mime_xml.pl $< > $@


$(share_WORKDIR)/%/openoffice.keys:  \
	$(share_SRCDIR)/mimetypes/openoffice.mime $(MIMEKEYS) $(share_SRCDIR)/share/brand.pl \
	$(share_TRANSLATE)  $(share_WORKDIR)/documents.ulf
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(PERL) $(share_SRCDIR)/share/brand.pl -p $* -u $(UNIXFILENAME.$*) \
		--iconprefix $(UNIXFILENAME.$*)- $^ $(share_WORKDIR)/$*
	$(PERL) $(share_TRANSLATE) -p $* -d $(share_WORKDIR)/$* \
		--ext "keys" --key "description" $(share_WORKDIR)/documents.ulf
	cat $(MIMEKEYS) > $@

$(share_WORKDIR)/%/mimelnklist: $(MIMEDESKTOPS) $(share_SRCDIR)/share/brand.pl \
	$(share_TRANSLATE) $(share_WORKDIR)/documents.ulf
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(PERL) $(share_SRCDIR)/share/brand.pl -p $* -u $(UNIXFILENAME.$*) \
		--iconprefix $(UNIXFILENAME.$*)- $^ $(share_WORKDIR)/$*
	$(PERL) $(share_TRANSLATE) -p $* -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "Comment" $(share_WORKDIR)/documents.ulf
	echo "$(MIMEDESKTOPS)" > $@

$(share_WORKDIR)/%/openoffice.mime: $(share_SRCDIR)/mimetypes/openoffice.mime
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	cat $< | tr -d "\015" > $@

$(share_WORKDIR)/%/openoffice.sh: $(share_SRCDIR)/share/openoffice.sh
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$*)/g" > $@

$(share_WORKDIR)/%/printeradmin.sh: $(share_SRCDIR)/share/printeradmin.sh
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$*)/g" > $@

$(share_WORKDIR)/%/create_tree.sh: $(share_SRCDIR)/share/create_tree.sh $(share_WORKDIR)/%/mimelnklist $(share_WORKDIR)/%/printeradmin.sh \
	$(share_WORKDIR)/%/openoffice.org.xml $(share_WORKDIR)/%/openoffice.applications $(share_WORKDIR)/%/openoffice.mime \
	$(share_WORKDIR)/%/openoffice.keys $(share_WORKDIR)/%/launcherlist
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	echo "#!/bin/sh" > $@
	echo "PREFIX=$(UNIXFILENAME.$*)" >> $@
	echo "ICON_PREFIX=$(UNIXFILENAME.$*)" >> $@
	echo "ICON_SOURCE_DIR=$(SRCDIR)/sysui/desktop/icons" >> $@
	echo "PRODUCTVERSION=$(PRODUCTVERSION)" >> $@
	cat $< >> $@
	chmod 774 $@


$(share_WORKDIR)/%/launcherlist: $(LAUNCHERS)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo "$(addsuffix .desktop,$(filter-out qstart,$(LAUNCHERLIST)))" > $@


$(share_WORKDIR)/%/openoffice.applications: $(share_SRCDIR)/mimetypes/openoffice.applications
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	cat $< | tr -d "\015" | sed -e "s/OFFICENAME/$(UNIXFILENAME.$*)/" -e "s/%PRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION.$*)/" > $@

$(share_WORKDIR)/%/build.flag: $(share_SRCDIR)/share/brand.pl $(LAUNCHERS) \
	$(share_TRANSLATE)  $(addprefix $(share_WORKDIR)/,$(ULFS))
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(PERL) $(share_SRCDIR)/share/brand.pl -p '$${PRODUCTNAME} $${PRODUCTVERSION}' -u $(UNIXFILENAME.$*) \
		$(brand_URIPARAM) \
		--iconprefix '$${UNIXBASISROOTNAME}-' $^ $(share_WORKDIR)/$*
	$(PERL) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "Comment" $(share_WORKDIR)/launcher_comment.ulf
	$(PERL) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "GenericName" $(share_WORKDIR)/launcher_genericname.ulf
	$(PERL) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "UnityQuickList" $(share_WORKDIR)/launcher_unityquicklist.ulf
	touch $@

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(share_WORKDIR)/%.ulf,\
	$(share_SRCDIR)/share/%.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/sysui/desktop/share.po)))

# vim: set noet sw=4 ts=4:
