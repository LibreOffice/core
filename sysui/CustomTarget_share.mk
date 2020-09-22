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
else ifeq ($(ENABLE_QT5),TRUE)
	brand_URIPARAM := --urls
else
	brand_URIPARAM :=
endif

share_WORKDIR := $(call gb_CustomTarget_get_workdir,sysui/share)
share_SRCDIR := $(SRCDIR)/sysui/desktop

share_TRANSLATE := $(SRCDIR)/solenv/bin/desktop-translate.py

LAUNCHERLIST_APPS := writer calc draw impress math base startcenter
LAUNCHERLIST := $(LAUNCHERLIST_APPS) xsltfilter
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
    oasis-master-document-template \
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
    oasis-master-document-template \
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
		$(product)/create_tree.sh \
		$(if $(INTROSPECTION_SCANNER),\
			$(product)/LOKDocView-0.1.gir \
			$(product)/LOKDocView-0.1.typelib) \
		$(product)/launcherlist) \
))

$(share_WORKDIR)/%/openoffice.org.xml: $(share_WORKDIR)/documents.ulf $(MIMEDESKTOPS) $(share_SRCDIR)/share/create_mime_xml.pl
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(share_SRCDIR)/share/create_mime_xml.pl $< > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(share_WORKDIR)/%/openoffice.keys:  \
	$(share_SRCDIR)/mimetypes/openoffice.mime $(MIMEKEYS) $(share_SRCDIR)/share/brand.pl \
	$(share_TRANSLATE)  $(share_WORKDIR)/documents.ulf \
	$(call gb_ExternalExecutable_get_dependencies,python)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(share_SRCDIR)/share/brand.pl -p $* -u $(UNIXFILENAME.$*) \
		--iconprefix $(UNIXFILENAME.$*)- $(MIMEKEYS) $(share_WORKDIR)/$*
	$(call gb_ExternalExecutable_get_command,python) $(share_TRANSLATE) \
		-p $* -d $(share_WORKDIR)/$* \
		--ext "keys" --key "description" $(share_WORKDIR)/documents.ulf
	cat $(MIMEKEYS) > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(share_WORKDIR)/%/openoffice.mime: $(share_SRCDIR)/mimetypes/openoffice.mime
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CAT)
	cat $< | tr -d "\015" > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CAT)

$(share_WORKDIR)/%/openoffice.sh: $(share_SRCDIR)/share/openoffice.sh
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CAT)
	cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$*)/g" > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CAT)

$(share_WORKDIR)/%/create_tree.sh: $(share_SRCDIR)/share/create_tree.sh \
	$(share_WORKDIR)/%/openoffice.org.xml $(share_WORKDIR)/%/openoffice.applications $(share_WORKDIR)/%/openoffice.mime \
	$(share_WORKDIR)/%/openoffice.keys $(share_WORKDIR)/%/launcherlist
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CAT)
	echo "#!/bin/sh" > $@
	echo "PREFIX=$(UNIXFILENAME.$*)" >> $@
	echo "ICON_PREFIX=$(UNIXFILENAME.$*)" >> $@
	echo "ICON_SOURCE_DIR=$(SRCDIR)/sysui/desktop/icons" >> $@
	echo "APPDATA_SOURCE_DIR=$(SRCDIR)/sysui/desktop/appstream-appdata" >> $@
	echo "PRODUCTVERSION=$(PRODUCTVERSION)" >> $@
	cat $< >> $@
	chmod 774 $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CAT)

# Generate gobject-introspection files
# These are *not* packaged in rpms because there's no good place to put them
# where the system will actually find them and where it won't conflict with a
# distro packaged LO; on Fedora 30 at least there's no /opt path in
# $XDG_DATA_DIRS
ifneq ($(INTROSPECTION_SCANNER),)

$(share_WORKDIR)/%/LOKDocView-0.1.gir: \
		$(call gb_Library_get_target,libreofficekitgtk)
	mkdir -p $(dir $@)
	PYTHONWARNINGS=default g-ir-scanner "${SRCDIR}/include/LibreOfficeKit/LibreOfficeKitGtk.h" \
				 "${SRCDIR}/libreofficekit/source/gtk/lokdocview.cxx" \
                 `${PKG_CONFIG} --cflags gobject-introspection-1.0 gtk+-3.0` \
				 -I"${SRCDIR}/include/" \
                 --include=GLib-2.0 --include=GObject-2.0 --include=Gio-2.0 \
                 --library=libreofficekitgtk --library-path="${INSTDIR}/program" \
                 --include=Gdk-3.0 --include=GdkPixbuf-2.0 --include=Gtk-3.0 \
                 --namespace=LOKDocView --nsversion=0.1 --identifier-prefix=LOKDoc --symbol-prefix=lok_doc \
                 --c-include="LibreOfficeKit/LibreOfficeKitGtk.h" \
				 --output="$@" --warn-all --no-libtool

$(share_WORKDIR)/%/LOKDocView-0.1.typelib: $(share_WORKDIR)/%/LOKDocView-0.1.gir
	g-ir-compiler "$<" --output="$@"

endif

$(share_WORKDIR)/%/launcherlist: $(LAUNCHERS)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),ECH)
	echo "$(addsuffix .desktop,$(LAUNCHERLIST))" > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),ECH)


$(share_WORKDIR)/%/openoffice.applications: $(share_SRCDIR)/mimetypes/openoffice.applications
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CAT)
	cat $< | tr -d "\015" | sed -e "s/OFFICENAME/$(UNIXFILENAME.$*)/" -e "s/%PRODUCTNAME/$(PRODUCTNAME.$*) $(PRODUCTVERSION.$*)/" > $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CAT)

# these .desktop files are written by brand.pl below
# need to have a rule for these because they are targets in Package_share
define sysui_Desktop_rule
$(share_WORKDIR)/%/$(1).desktop : $(share_WORKDIR)/%/build.flag
	touch $$@

endef

$(foreach launcher,$(LAUNCHERLIST),$(eval $(call sysui_Desktop_rule,$(launcher))))

$(share_WORKDIR)/%/build.flag: $(share_SRCDIR)/share/brand.pl $(LAUNCHERS) \
		$(share_TRANSLATE) $(addprefix $(share_WORKDIR)/,$(ULFS)) \
		$(call gb_ExternalExecutable_get_dependencies,python)
	mkdir -p $(dir $@)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),PRL,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),PRL)
	$(PERL) $(share_SRCDIR)/share/brand.pl -p '$${PRODUCTNAME} $${PRODUCTVERSION}' -u $(UNIXFILENAME.$*) \
		$(brand_URIPARAM) \
		--iconprefix '$${UNIXBASISROOTNAME}-' $(LAUNCHERS) $(share_WORKDIR)/$*
	$(call gb_ExternalExecutable_get_command,python) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "Comment" $(share_WORKDIR)/launcher_comment.ulf
	$(call gb_ExternalExecutable_get_command,python) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "GenericName" $(share_WORKDIR)/launcher_genericname.ulf
	$(call gb_ExternalExecutable_get_command,python) $(share_TRANSLATE) -p $(PRODUCTNAME.$*)$(PRODUCTVERSION) -d $(share_WORKDIR)/$* \
		--ext "desktop" --key "UnityQuickList" $(share_WORKDIR)/launcher_unityquicklist.ulf
	touch $@
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),PRL)

$(eval $(call gb_CustomTarget_ulfex_rule,\
	$(share_WORKDIR)/%.ulf,\
	$(share_SRCDIR)/share/%.ulf,\
	$(foreach lang,$(gb_TRANS_LANGS),\
		$(gb_POLOCATION)/$(lang)/sysui/desktop/share.po)))

# vim: set noet sw=4 ts=4:
