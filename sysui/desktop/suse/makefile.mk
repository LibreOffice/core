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

LAUNCHERLIST = writer calc draw impress math printeradmin
LAUNCHERDEPN = ../menus/{$(LAUNCHERLIST)}.desktop
LAUNCHERDIR  = $(shell cd $(MISC)$/$(TARGET); pwd)

LAUNCHERFLAGFILES = \
    $(MISC)/$(TARGET)/opt/gnome/share/applications.flag \
    $(MISC)/$(TARGET)/opt/kde3/share/applnk/Office.flag

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
    $(MISC)/$(TARGET)/opt/kde3/share/mimelnk/application.flag
        
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
    $(MISC)/$(TARGET)/opt/gnome/share/applications.flag \
    $(MISC)/$(TARGET)/opt/kde3/share/applnk/Office.flag \
    $(MISC)/$(TARGET)/opt/gnome/share/application-registry/$(UNIXFILENAME).applications \
    $(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).keys \
    $(MISC)/$(TARGET)/opt/kde3/share/mimelnk/application.flag \
    $(MISC)/$(TARGET)/opt/gnome/share/icons/gnome/{$(GNOMEICONLIST)} \
    $(MISC)/$(TARGET)/opt/kde3/share/icons/{$(KDEICONLIST)} 
        
RPMDIR  = $(shell cd $(BIN); pwd)
    
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
$(LAUNCHERFLAGFILES) : $(LAUNCHERDEPN) ../share/brand.pl ../share/translate.pl ../share/launcher_name.ulf ../share/launcher_comment.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating desktop entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(LONGPRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(UNIXFILENAME)-" --category "X-Red-Hat-Base" $(LAUNCHERDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Name" ../share/launcher_name.ulf
.IF "$(WITH_LIBSN)"=="YES"
    @$(foreach,i,$(LAUNCHERLIST) $(shell echo "StartupNotify=true" >> $(@:db)/$(UNIXFILENAME)-$i.desktop))
.ENDIF
    @touch $@

#
# FIXME: disabled comments for now due to missing string review
#	@$(PERL) translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" ../share/launcher_comment.ulf
#

# --- icons --------------------------------------------------------

#
# This target is responsible for copying the GNOME icons to their package specific target
# e.g. $(LAUNCHERDIR)/opt/gnome/share/icons/gnome/16x16/apps/openoffice-writer.png
#
$(MISC)/$(TARGET)/opt/gnome/share/icons/gnome/{$(GNOMEICONLIST)} : ../icons/hicolor/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(UNIXFILENAME)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(MISC)/$(TARGET)/opt/kde3/share/icons/{$(KDEICONLIST)} : ../icons/$$(@:d:d:d:d:d:d:f)/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(UNIXFILENAME)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
    
# --- mime types ---------------------------------------------------

$(MISC)/$(TARGET)/opt/gnome/share/mime-info/$(UNIXFILENAME).keys : $(GNOMEMIMEDEPN) ../share/brand.pl ../share/translate.pl ../share/documents.ulf
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .keys file ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p $(PRODUCTNAME) -u $(UNIXFILENAME) --iconprefix "$(UNIXFILENAME)-" $(GNOMEMIMEDEPN) $(MISC)/$(TARGET)
    @$(PERL) ../share/translate.pl -p $(PRODUCTNAME) -d $(MISC)/$(TARGET) --ext "keys" --key "description"  ../share/documents.ulf
    @cat $(MISC)/$(TARGET)/{$(MIMELIST)}.keys > $@

$(KDEMIMEFLAGFILE) : $(KDEMIMEDEPN) ../share/brand.pl ../share/translate.pl ../share/documents.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating KDE mimelnk entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(PRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(UNIXFILENAME)-" $(KDEMIMEDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" ../share/documents.ulf
    @touch $@    

$(MISC)/$(TARGET)/opt/gnome/share/application-registry/$(UNIXFILENAME).applications : ../mimetypes/openoffice.applications
    @$(MKDIRHIER) $(@:d)
    @echo Creating GNOME .applications file ..
    @echo ---------------------------------
    @cat $< | tr -d "\015" | sed -e "s/openoffice/$(UNIXFILENAME)/" -e "s/%PRODUCTNAME/$(LONGPRODUCTNAME)/" > $@

# --- packaging ---------------------------------------------------
    
$(RPMFLAGFILE) : $(RPMDEPN)
    @cat $(@:b)-menus.spec | tr -d "\015" | sed "s/%PREFIX/$(UNIXFILENAME)/" > $(@:db)-menus.spec
    @echo "%define _rpmdir $(RPMDIR)" >> $(@:db)-menus.spec
    @$(RPM) -bb $(@:db)-menus.spec --buildroot $(LAUNCHERDIR) --target noarch
    @touch $@

.ENDIF
