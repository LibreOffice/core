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
TARGET=redhat

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  packtools.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

LAUNCHERLIST = writer calc draw impress math printeradmin
LAUNCHERDEPN = ../menus/{$(LAUNCHERLIST)}.desktop
LAUNCHERDIR  = $(shell cd $(MISC)$/$(TARGET); pwd)

LAUNCHERFLAGFILES = \
    $(MISC)/$(TARGET)/usr/share/applications.flag \
    $(MISC)/$(TARGET)/usr/share/applnk-$(TARGET)/Office.flag

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
    master-document

GNOMEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.keys
KDEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.desktop

KDEMIMEFLAGFILE = \
    $(MISC)/$(TARGET)/usr/share/mimelnk/application.flag
        
GNOMEICONLIST = \
    {16x16 22x22 32x32 48x48}/apps/$(UNIXFILENAME)-{$(LAUNCHERLIST)}.png \
    {16x16 22x22 32x32 48x48}/mimetypes/$(UNIXFILENAME)-{$(MIMELIST)}.png
    
KDEICONLIST = \
    hicolor/{16x16 22x22 32x32 48x48}/apps/$(UNIXFILENAME)-{$(LAUNCHERLIST)}.png \
    hicolor/{16x16 22x22 32x32 48x48}/mimetypes/$(UNIXFILENAME)-{$(MIMELIST)}.png \
    locolor/{16x16 22x22 32x32}/apps/$(UNIXFILENAME)-{$(LAUNCHERLIST)}.png \
    locolor/{16x16 22x22 32x32}/mimetypes/$(UNIXFILENAME)-{$(MIMELIST)}.png

.IF "$(RPM)"!=""

RPMFLAGFILE = $(MISC)$/$(TARGET).flag 
RPMDEPN = \
    $(MISC)/$(TARGET)/usr/share/applications.flag \
    $(MISC)/$(TARGET)/usr/share/applnk-$(TARGET)/Office.flag \
    $(MISC)/$(TARGET)/usr/share/application-registry/$(UNIXFILENAME).applications \
    $(MISC)/$(TARGET)/usr/share/mime-info/$(UNIXFILENAME).keys \
    $(MISC)/$(TARGET)/usr/share/mimelnk/application.flag \
    $(MISC)/$(TARGET)/usr/share/icons/gnome/{$(GNOMEICONLIST)} \
    $(MISC)/$(TARGET)/usr/share/icons/{$(KDEICONLIST)} 
        
RPMDIR  = $(shell cd $(BIN); pwd)
ULFDIR = $(COMMONMISC)$/desktopshare
    
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(RPM)"!=""

ALLTAR : $(RPMFLAGFILE) 

# --- launcher ------------------------------------------------------

#
# Copy/patch the .desktop files to the output tree and 
# merge-in the translations. 
#
$(LAUNCHERFLAGFILES) : $(LAUNCHERDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/launcher_name.ulf $(ULFDIR)/launcher_comment.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating desktop entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(LONGPRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(UNIXFILENAME)-" --category "X-Red-Hat-Base" $(LAUNCHERDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Name" $(ULFDIR)/launcher_name.ulf
    @$(PERL) ../share/translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" $(ULFDIR)/launcher_comment.ulf
.IF "$(WITH_LIBSN)"=="YES"
    @$(foreach,i,$(LAUNCHERLIST) $(shell echo "StartupNotify=true" >> $(@:db)/$(UNIXFILENAME)-$i.desktop))
.ENDIF
    @touch $@

# --- icons --------------------------------------------------------

#
# This target is responsible for copying the GNOME icons to their package specific target
# e.g. $(LAUNCHERDIR)/usr/share/icons/gnome/16x16/apps/openoffice-writer.png
#
$(MISC)/$(TARGET)/usr/share/icons/gnome/{$(GNOMEICONLIST)} : ../icons/hicolor/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(UNIXFILENAME)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(MISC)/$(TARGET)/usr/share/icons/{$(KDEICONLIST)} : ../icons/$$(@:d:d:d:d:d:d:f)/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(UNIXFILENAME)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
    
# --- mime types ---------------------------------------------------

$(MISC)/$(TARGET)/usr/share/mime-info/$(UNIXFILENAME).keys : $(GNOMEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p $(PRODUCTNAME) -u $(UNIXFILENAME) --iconprefix "$(UNIXFILENAME)-" $(GNOMEMIMEDEPN) $(MISC)/$(TARGET)
    @$(PERL) ../share/translate.pl -p $(PRODUCTNAME) -d $(MISC)/$(TARGET) --ext "keys" --key "description"  $(ULFDIR)/documents.ulf
    @cat $(MISC)/$(TARGET)/{$(MIMELIST)}.keys > $@

$(KDEMIMEFLAGFILE) : $(KDEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating KDE mimelnk entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(PRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(UNIXFILENAME)-" $(KDEMIMEDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" $(ULFDIR)/documents.ulf
    @touch $@    

$(MISC)/$(TARGET)/usr/share/application-registry/$(UNIXFILENAME).applications : ../productversion.mk ../mimetypes/openoffice.applications
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" | sed -e "s/openoffice/$(UNIXFILENAME)/" -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/" > $@

# --- packaging ---------------------------------------------------
    
$(RPMFLAGFILE) : $(RPMDEPN)
    @cat $(@:b)-menus.spec | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/" -e "s/Version: .*/Version: $(PKGVERSION)/" -e "s/Release: .*/Release: $(PKGREV)/" > $(@:db)-menus.spec
    @echo "%define _rpmdir $(RPMDIR)" >> $(@:db)-menus.spec
    @$(RPM) -bb $(@:db)-menus.spec --buildroot $(LAUNCHERDIR) --target noarch
    @touch $@
    
.ENDIF
