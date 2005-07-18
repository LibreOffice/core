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
TARGET=debian

# !!! FIXME !!!
# debian-menus file.

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
    oasis-web-template

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

GNOMEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.keys ../mimetypes/openoffice.mime 
KDEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.desktop

KDEMIMEFLAGFILE = \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mimelnk/application.flag
        
GNOMEICONLIST = \
    {16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    {16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png

HCICONLIST = \
    HighContrast/{16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    HighContrast/{16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(HCMIMEICONLIST)}.png
    
KDEICONLIST = \
    hicolor/{16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    hicolor/{16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png \
    locolor/{16x16 32x32}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    locolor/{16x16 32x32}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png

.IF "$(DPKG)"!=""

PKGNAME=openofficeorg-$(TARGET)-menus
DEBFILE=$(BIN)/$(PKGNAME)_$(PKGVERSION)-$(PKGREV)_all.deb
DEBDEPN = \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/etc/$(UNIXFILENAME) \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/soffice \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/$(UNIXFILENAME) \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/$(UNIXFILENAME)-printeradmin \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/lib/menu/$(PKGNAME) \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/applications/{$(LAUNCHERDEPN)} \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/applnk/Office \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/application-registry/$(UNIXFILENAME).applications \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime/packages/openoffice.org.xml \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime-info/$(UNIXFILENAME).keys \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime-info/$(UNIXFILENAME).mime \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mimelnk/application.flag \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/icons/gnome/{$(GNOMEICONLIST)} \
    $(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/icons/{$(KDEICONLIST)} 
        
DEBDIR  = $(shell cd $(BIN); pwd)
ULFDIR = $(COMMONMISC)$/desktopshare
    
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(DPKG)"!=""

ALLTAR : $(DEBFILE) 

# --- launcher ------------------------------------------------------

%.desktop :
    @$(MKDIRHIER) $(@:d)
    @ln -s -f $(subst,$(UNIXFILENAME)-, /etc/$(UNIXFILENAME)/share/xdg/$(@:f)) $@

%/Office :
    @$(MKDIRHIER) $@

%/menu/$(PKGNAME) : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $< | sed -e 's/%PRODUCTNAME/$(LONGPRODUCTNAME)/' -e 's/%PREFIX/%unixfilename/' \
        -e 's/%ICONPREFIX/%unixfilename/' > $@

# --- icons --------------------------------------------------------

#
# This target is responsible for copying the GNOME icons to their package specific target
# e.g. $(LAUNCHERDIR)/usr/share/icons/gnome/16x16/apps/openoffice-writer.png
#
$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/icons/gnome/{$(GNOMEICONLIST)} : ../icons/hicolor/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(ICONPREFIX)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/icons/{$(KDEICONLIST)} : ../icons/$$(@:d:d:d:d:d:d:f)/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(ICONPREFIX)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
    
# --- mime types ---------------------------------------------------

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime-info/$(UNIXFILENAME).keys : $(GNOMEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p $(PRODUCTNAME) -u $(UNIXFILENAME) --iconprefix "$(ICONPREFIX)-" $(GNOMEMIMEDEPN) $(MISC)/$(TARGET)
    @$(PERL) ../share/translate.pl -p $(PRODUCTNAME) -d $(MISC)/$(TARGET) --ext "keys" --key "description"  $(ULFDIR)/documents.ulf
    @cat $(MISC)/$(TARGET)/{$(MIMELIST)}.keys > $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime-info/$(UNIXFILENAME).mime : ../mimetypes/openoffice.mime
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .mime file ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" > $@

$(KDEMIMEFLAGFILE) : $(KDEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating KDE mimelnk entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(PRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(ICONPREFIX)-" $(KDEMIMEDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" $(ULFDIR)/documents.ulf
    @touch $@    

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/application-registry/$(UNIXFILENAME).applications : ../productversion.mk ../mimetypes/openoffice.applications
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file ..
    @echo ---------------------------------
    @cat ../mimetypes/openoffice.applications | tr -d "\015" | sed -e "s/openoffice/$(UNIXFILENAME)/" -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/" > $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/share/mime/packages/openoffice.org.xml : $(COMMONMISC)$/desktopshare/openoffice.org.xml
    @$(MKDIRHIER) $(@:d)
    @cp $< $@

# --- script ------------------------------------------------------

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/$(UNIXFILENAME) : ../share/openoffice.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@
    @chmod a+x $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/$(UNIXFILENAME)-printeradmin : ../share/printeradmin.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@
    @chmod a+x $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/usr/bin/soffice : 
    @$(MKDIRHIER) $(@:d)
    @ln -sf /etc/$(UNIXFILENAME)/program/soffice $@

$(MISC)/$(TARGET)/$(DEBFILE:f)/etc/$(UNIXFILENAME) :
    @$(MKDIRHIER) $(@:d)
    @ln -sf /opt/openoffice.org$(PKGVERSION) $@



# --- packaging ---------------------------------------------------

# getuid.so fakes the user/group for us	
$(DEBFILE) : $(DEBDEPN)
    @$(MKDIRHIER) $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN
    @cat control | tr -d "\015" > $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/control
    @echo "Version: $(PKGVERSION)" >> $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/control
    @cat postinst | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/postinst
    @cat postrm | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/postrm
    @cat prerm | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/prerm
    @chmod -R g-w $(MISC)/$(TARGET)/$(DEBFILE:f)
    @chmod a+rx $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/post* $(MISC)/$(TARGET)/$(DEBFILE:f)/DEBIAN/pre*
    /bin/bash -c "LD_PRELOAD=$(SOLARBINDIR)/getuid.so dpkg-deb --build $(MISC)/$(TARGET)/$(@:f) $@"
    @chmod -R g+w $(MISC)/$(TARGET)/$(@:f)

.ENDIF
