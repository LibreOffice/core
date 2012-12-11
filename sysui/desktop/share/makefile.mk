#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..

PRJNAME=sysui
TARGET=desktopshare

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

ULFFILES= \
    documents.ulf \
    launcher_comment.ulf \
    launcher_genericname.ulf \
    launcher_unityquicklist.ulf \
    launcher_name.ulf

LAUNCHERLIST_APPS = writer calc draw impress math base printeradmin startcenter
LAUNCHERLIST = $(LAUNCHERLIST_APPS) qstart javafilter xsltfilter
LAUNCHERDEPN = ../menus/{$(LAUNCHERLIST)}.desktop

LAUNCHERFLAGFILE = $(COMMONMISC)/$(TARGET)/xdg.flag

MIMELIST = \
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

MIMEICONLIST = \
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

ICONDEPN = \
	   ../icons/hicolor/{16x16 32x32 48x48}/apps/{$(LAUNCHERLIST_APPS)}.png \
    ../icons/hicolor/{16x16 32x32 48x48}/mimetypes/{$(MIMEICONLIST)}.png \
    ../icons/locolor/{16x16 32x32}/apps/{$(LAUNCHERLIST_APPS)}.png \
    ../icons/locolor/{16x16 32x32}/mimetypes/{$(MIMEICONLIST)}.png

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)"!="WNT"
ALLTAR : $(LAUNCHERFLAGFILE) $(SPECFILES) $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag

.ENDIF          # "$(OS)"!="WNT"

#
# Copy/patch the .desktop files to the output tree and 
# merge-in the translations. 
#

$(LAUNCHERFLAGFILE) : ../productversion.mk brand.pl translate.pl $(ULFDIR)$/launcher_name.ulf $(ULFDIR)$/launcher_comment.ulf $(ULFDIR)/launcher_genericname.ulf $(ULFDIR)/launcher_unityquicklist.ulf
$(LAUNCHERFLAGFILE) : $(LAUNCHERDEPN) 
    @@-$(MKDIRHIER) $(@:db).$(INPATH).$(@:f)
    @echo Creating desktop entries for $(@:f) ..
    @echo ---------------------------------
    @$(PERL) brand.pl -p '$${{PRODUCTNAME}} $${{PRODUCTVERSION}}' -u $(UNIXWRAPPERNAME) --iconprefix '$${{UNIXBASISROOTNAME}}-' $< $(@:db).$(INPATH).$(@:f)
    @$(PERL) translate.pl -p '$${{PRODUCTNAME}} $${{PRODUCTVERSION}}' -d $(@:db).$(INPATH).$(@:f) --ext "desktop" --key "Name" $(ULFDIR)$/launcher_name.ulf
    @$(PERL) translate.pl -p '$${{PRODUCTNAME}} $${{PRODUCTVERSION}}' -d $(@:db).$(INPATH).$(@:f) --ext "desktop" --key "Comment" $(ULFDIR)$/launcher_comment.ulf
    @$(PERL) translate.pl -p '$${{PRODUCTNAME}} $${{PRODUCTVERSION}}' -d $(@:db).$(INPATH).$(@:f) --ext "desktop" --key "GenericName" $(ULFDIR)$/launcher_genericname.ulf
    @$(PERL) translate.pl -p '$${{PRODUCTNAME}} $${{PRODUCTVERSION}}' -d $(@:db).$(INPATH).$(@:f) --ext "desktop" --key "UnityQuicklist" $(ULFDIR)$/launcher_unityquicklist.ulf
.IF "$(WITH_LIBSN)"=="YES"
    @noop x$(foreach,i,$(LAUNCHERLIST) $(shell @echo "StartupNotify=true" >> $(@:db).$(INPATH).$(@:f)/$i.desktop))x
.ENDIF
    @$(MV) -f $(@:db).$(INPATH).$(@:f)/* $(@:d)
    @rmdir $(@:db).$(INPATH).$(@:f)
    @touch $@

#
# Create shared mime info xml file
#
$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.org.xml : ../mimetypes/{$(MIMELIST)}.desktop create_mime_xml.pl
$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.org.xml : $(ULFDIR)$/documents.ulf
    @echo Create shared mime info xml file ..
    @echo ---------------------------------
    @$(PERL) create_mime_xml.pl $< > $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

#
# Create GNOME mime-info files
#

$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.keys : ../mimetypes/openoffice.mime brand.pl translate.pl ../productversion.mk $(ULFDIR)$/documents.ulf
$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.keys : ../mimetypes/{$(MIMELIST)}.keys  
    @@-$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file for $(@:d:d:f) ..
    @echo ---------------------------------
    @$(PERL) brand.pl -p $(PRODUCTNAME.$(@:d:d:f)) -u $(UNIXFILENAME.$(@:d:d:f)) --iconprefix '$(ICONPREFIX.$(@:d:d:f))-' $< $(MISC)/$(@:d:d:f)
    @$(PERL) translate.pl -p $(PRODUCTNAME.$(@:d:d:f)) -d $(MISC)/$(@:d:d:f) --ext "keys" --key "description" $(ULFDIR)$/documents.ulf
    @cat $(MISC)/$(@:d:d:f)/{$(MIMELIST)}.keys > $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.mime : ../mimetypes/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .mime file for $(@:d:d:f) ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" > $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.applications : ../productversion.mk 
$(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.applications : ../mimetypes/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file for $(@:d:d:f) ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" | sed -e "s/OFFICENAME/$(UNIXFILENAME.$(@:d:d:f))/" -e "s/%PRODUCTNAME/$(PRODUCTNAME.$(@:d:d:f)) $(PRODUCTVERSION.$(@:d:d:f))/" > $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

#
# Create KDE mimelnk files
#
$(COMMONMISC)$/{$(PRODUCTLIST)}$/mimelnklist : brand.pl translate.pl ../productversion.mk $(ULFDIR)$/documents.ulf
$(COMMONMISC)$/{$(PRODUCTLIST)}$/mimelnklist : ../mimetypes/{$(MIMELIST)}.desktop
    @@-$(MKDIRHIER) $(@:db).$(INPATH)
    @echo Creating KDE mimelnk entries for $(@:d:d:f) ..
    @echo ---------------------------------
    @$(PERL) brand.pl -p "$(PRODUCTNAME.$(@:d:d:f))" -u $(UNIXFILENAME.$(@:d:d:f)) --iconprefix '$(ICONPREFIX.$(@:d:d:f))-' $< $(@:db).$(INPATH)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME.$(@:d:d:f))" -d $(@:db).$(INPATH) --ext "desktop" --key "Comment" $(ULFDIR)$/documents.ulf
    @mv -f $(@:db).$(INPATH)/* $(@:d)
    @rmdir $(@:db).$(INPATH)
    @echo  "{$(MIMELIST)}.desktop" > $@

#
# Generate customized install scripts
#
$(COMMONMISC)$/{$(PRODUCTLIST)}$/create_tree.sh : makefile.mk
$(COMMONMISC)$/{$(PRODUCTLIST)}$/create_tree.sh : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    @echo "#!/bin/sh" > $@.$(INPATH)
    @echo "PREFIX=$(UNIXFILENAME.$(@:d:d:f))" >> $@.$(INPATH)
    @echo "ICON_PREFIX=$(ICONPREFIX.$(@:d:d:f))" >> $@.$(INPATH)
    @echo "ICON_SOURCE_DIR=../../../desktop/icons" >> $@.$(INPATH)
    @echo "PRODUCTVERSION=$(PRODUCTVERSION.$(@:d:d:f))" >> $@.$(INPATH)
    @cat $< >> $@.$(INPATH)
    @chmod 774 $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

#
# Create skript files
#

$(COMMONMISC)$/{$(PRODUCTLIST)}$/{openoffice printeradmin}.sh : ../productversion.mk
$(COMMONMISC)$/{$(PRODUCTLIST)}$/{openoffice printeradmin}.sh : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" > $@.$(INPATH)
    @mv -f $@.$(INPATH) $@

#
# Launcher list
#

$(COMMONMISC)$/{$(PRODUCTLIST)}$/launcherlist : $(LAUNCHERDEPN)
    @@-$(MKDIRHIER) $(@:d)
    @echo "{$(LAUNCHERLIST:s/qstart//)}.desktop" >$@.$(INPATH)
    @mv -f $@.$(INPATH) $@

#
# Flagfile
#
$(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag : $(ICONDEPN) \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/launcherlist \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/mimelnklist \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/create_tree.sh \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.keys \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.mime \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.org.xml	\
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/openoffice.applications \
    $(COMMONMISC)$/{$(PRODUCTLIST)}$/{openoffice printeradmin}.sh
    @touch $@

