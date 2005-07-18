#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sysui
TARGET=suse

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  packtools.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

# GNOME does not like icon names with more than one '.'
ICONPREFIX = $(UNIXFILENAME:s/.//g)

LAUNCHERLIST = writer calc draw impress math base printeradmin
LAUNCHERDEPN = $(foreach,i,$(LAUNCHERLIST) $(UNIXFILENAME)-$i.desktop)
LAUNCHERDIR  = $(shell cd $(MISC)$/$(TARGET); pwd)

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
    database

GNOMEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.keys 
KDEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.desktop

KDEMIMEFLAGFILE = \
    $(MISC)/$(TARGET)/opt/kde3/share/mimelnk/application.flag
        
GNOMEICONLIST = \
    {16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    {16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png

KDEICONLIST = \
    hicolor/{16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    hicolor/{16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png \
    locolor/{16x16 32x32}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    locolor/{16x16 32x32}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png

.IF "$(RPM)"!=""

PKGNAME=$(shell sed -n -e 's/^Name: //p' $(TARGET)-menus.spec)
RPMFILE=$(BIN)/noarch/$(PKGNAME)-$(PKGVERSION)-$(PKGREV).noarch.rpm
RPMDEPN = \
    $(MISC)/$(TARGET)/etc/$(UNIXFILENAME) \
    $(MISC)/$(TARGET)/usr/bin/soffice \
    $(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME) \
    $(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME)-printeradmin \
    $(MISC)/$(TARGET)/usr/share/applications/{$(LAUNCHERDEPN)} \
    $(MISC)/$(TARGET)/usr/share/mime/packages/openoffice.org.xml \
    $(MISC)/$(TARGET)/opt/gnome/share/application-registry/$(UNIXFILENAME).applications \
    $(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).keys \
    $(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).mime \
    $(MISC)/$(TARGET)/opt/kde3/share/mimelnk/application.flag \
    $(MISC)/$(TARGET)/opt/gnome/share/icons/gnome/{$(GNOMEICONLIST)} \
    $(MISC)/$(TARGET)/opt/kde3/share/icons/{$(KDEICONLIST)} 
        
RPMDIR  = $(shell cd $(BIN); pwd)
ULFDIR = $(COMMONMISC)$/desktopshare
SPECFILE = $(MISC)/$(TARGET)-menus.spec

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(RPM)"!=""

ALLTAR : $(RPMFILE) 

# --- launcher ------------------------------------------------------

%.desktop :
    @$(MKDIRHIER) $(@:d)
    @ln -s -f $(subst,$(UNIXFILENAME)-, /etc/$(UNIXFILENAME)/share/xdg/$(@:f)) $@

# --- icons --------------------------------------------------------

#
# This target is responsible for copying the GNOME icons to their package specific target
# e.g. $(LAUNCHERDIR)/opt/gnome/share/icons/gnome/16x16/apps/openoffice-writer.png
#
$(MISC)/$(TARGET)/opt/gnome/share/icons/gnome/{$(GNOMEICONLIST)} : ../icons/hicolor/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(ICONPREFIX)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(MISC)/$(TARGET)/opt/kde3/share/icons/{$(KDEICONLIST)} : ../icons/$$(@:d:d:d:d:d:d:f)/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(ICONPREFIX)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
    
# --- mime types ---------------------------------------------------

$(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).keys : $(GNOMEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p $(PRODUCTNAME) -u $(UNIXFILENAME) --iconprefix "$(ICONPREFIX)-" $(GNOMEMIMEDEPN) $(MISC)/$(TARGET)
    @$(PERL) ../share/translate.pl -p $(PRODUCTNAME) -d $(MISC)/$(TARGET) --ext "keys" --key "description"  $(ULFDIR)/documents.ulf
    @cat $(MISC)/$(TARGET)/{$(MIMELIST)}.keys > $@
        
$(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).mime : ../mimetypes/openoffice.mime
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .mime file ..
    @echo ---------------------------------
    @cat ../mimetypes/openoffice.mime | tr -d "\015" > $@

$(KDEMIMEFLAGFILE) : $(KDEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating KDE mimelnk entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(PRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(ICONPREFIX)-" $(KDEMIMEDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" $(ULFDIR)/documents.ulf
    @touch $@    

$(MISC)/$(TARGET)/opt/gnome/share/application-registry/$(UNIXFILENAME).applications : ../productversion.mk ../mimetypes/openoffice.applications
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file ..
    @echo ---------------------------------
    @cat ../mimetypes/openoffice.applications | tr -d "\015" | sed -e "s/openoffice/$(UNIXFILENAME)/" -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/" > $@

$(MISC)/$(TARGET)/usr/share/mime/packages/openoffice.org.xml : $(COMMONMISC)$/desktopshare/openoffice.org.xml
    @$(MKDIRHIER) $(@:d)
    @cp $< $@

# --- script ------------------------------------------------------

$(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME) : ../share/openoffice.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@

$(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME)-printeradmin : ../share/printeradmin.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@

$(MISC)/$(TARGET)/usr/bin/soffice :
    @$(MKDIRHIER) $(@:d)
    @ln -sf /etc/$(UNIXFILENAME)/program/soffice $@

$(MISC)/$(TARGET)/etc/$(UNIXFILENAME) :
    @$(MKDIRHIER) $(@:d)
    @touch $@

# --- specfile ----------------------------------------------------

$(SPECFILE) : $$(@:f)
    @cat $< | tr -d "\015" >$@

# --- packaging ---------------------------------------------------
    
.PHONY $(RPMFILE) : $(RPMDEPN) $(SPECFILE)
    -$(RM) $(@:d)/$(PKGNAME)-*
    @$(RPM) -bb $(MISC)/$(TARGET)-menus.spec --buildroot $(LAUNCHERDIR) \
        --define "_builddir $(shell cd ../share; pwd)" \
        --define "_rpmdir $(RPMDIR)" \
        --define "unixfilename $(UNIXFILENAME)" \
        --define "version $(PKGVERSION)" --define "release $(PKGREV)"

.ENDIF
