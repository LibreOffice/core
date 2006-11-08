#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2006-11-08 11:55:35 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sysui
TARGET=slackware

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

# GNOME does not like icon names with more than one '.'
ICONPREFIX = $(UNIXFILENAME:s/.//g)

LAUNCHERLIST = writer calc draw impress math base printeradmin

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

KDEMIMEDEPN = ../mimetypes/{$(MIMELIST)}.desktop

KDEMIMEFLAGFILE = \
    $(MISC)/$(TARGET)/opt/kde/share/mimelnk/application.flag

KDEICONLIST = \
    hicolor/{16x16 32x32 48x48}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    hicolor/{16x16 32x32 48x48}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png \
    locolor/{16x16 32x32}/apps/$(ICONPREFIX)-{$(LAUNCHERLIST)}.png \
    locolor/{16x16 32x32}/mimetypes/$(ICONPREFIX)-{$(MIMEICONLIST)}.png


PKGNAME=openoffice.org-$(TARGET)-menus
MENUFILE=$(PKGDIR)/$(PKGNAME)-$(PKGVERSION)-noarch-$(PKGREV).tgz
MENUDEPN = \
    $(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME) \
    $(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME)-printeradmin \
    $(MISC)/$(TARGET)/usr/share/applications/ \
    $(MISC)/$(TARGET)/usr/share/mime/packages/openoffice.org.xml \
    $(MISC)/$(TARGET)/opt/kde/share/mimelnk/application.flag \
    $(MISC)/$(TARGET)/opt/kde/share/icons/{$(KDEICONLIST)} \
    $(MISC)/$(TARGET)/install/doinst.sh \
    $(MISC)/$(TARGET)/install/slack-desc 
        
ULFDIR = $(COMMONMISC)$/desktopshare

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)" == "LINUX"

ALLTAR : $(MENUFILE) 

$(MISC)/$(TARGET)/usr/share/applications/ :
    @$(MKDIRHIER) $(@)

# --- icons --------------------------------------------------------

$(MISC)/$(TARGET)/opt/kde/share/icons/{$(KDEICONLIST)} : ../icons/$$(@:d:d:d:d:d:d:f)/$$(@:d:d:d:d:f)/$$(@:d:d:f)/$$(@:f:s/$(ICONPREFIX)-//)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
    
# --- mime types ---------------------------------------------------

$(KDEMIMEFLAGFILE) : $(KDEMIMEDEPN) ../productversion.mk ../share/brand.pl ../share/translate.pl $(ULFDIR)/documents.ulf
    @$(MKDIRHIER) $(@:db)
    @echo Creating KDE mimelnk entries ..
    @echo ---------------------------------
    @$(PERL) ../share/brand.pl -p "$(PRODUCTNAME)" -u $(UNIXFILENAME) --prefix "$(UNIXFILENAME)-" --iconprefix "$(ICONPREFIX)-" $(KDEMIMEDEPN) $(@:db)
    @$(PERL) ../share/translate.pl -p "$(PRODUCTNAME)" -d $(@:db) --prefix "$(UNIXFILENAME)-" --ext "desktop" --key "Comment" $(ULFDIR)/documents.ulf
    @touch $@    

$(MISC)/$(TARGET)/usr/share/mime/packages/openoffice.org.xml : $(COMMONMISC)$/desktopshare/openoffice.org.xml
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

# --- script ------------------------------------------------------

$(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME) : ../share/openoffice.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@
    @chmod 0755 $@

$(MISC)/$(TARGET)/usr/bin/$(UNIXFILENAME)-printeradmin : ../share/printeradmin.sh
    @$(MKDIRHIER) $(@:d)
    @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME)/g" > $@
    @chmod 0755 $@

# --- slackware-specific stuff ------------------------------------
# symlinks shall not be included in the tarball, but created with the doinst-script

$(MISC)/$(TARGET)/install/doinst.sh : update-script
    @$(MKDIRHIER) $(@:d)
    @echo "( cd etc ; rm -rf $(UNIXFILENAME) )" > $@
    @echo "( cd etc ; ln -snf /opt/$(UNIXFILENAME:s/-//) $(UNIXFILENAME) )" >> $@
    @echo "( cd usr/bin ; rm -rf soffice )" >> $@
    @echo "( cd usr/bin ; ln -sf /etc/$(UNIXFILENAME)/program/soffice soffice )" >> $@
    @echo $(foreach,i,$(LAUNCHERLIST) "\n( cd usr/share/applications ; rm -rf $(UNIXFILENAME)-$i.desktop )\n( cd usr/share/applications ; ln -sf /etc/$(UNIXFILENAME)/share/xdg/$i.desktop $(UNIXFILENAME)-$i.desktop )") >> $@
    @cat $< >> $@

$(MISC)/$(TARGET)/install/slack-desc : slack-desc
    @$(MKDIRHIER) $(@:d)
    @sed -e "s/PKGNAME/$(PKGNAME)/g" -e "s/PKGVERSION/$(PKGVERSION)/g" \
         -e "s/LONGPRODUCTNAME/$(LONGPRODUCTNAME)/g" -e "s/PRODUCTNAME/$(PRODUCTNAME)/g" \
         -e "s/UNIXFILENAME/$(UNIXFILENAME)/g" $< > $@

# needed to satisfy the slackware pagckage tools - they need
# the entries like this
#       ./              <- very important
#       directory/file
# the follwoing does not work (no description found then):
#       ./
#       ./directory/file
# and this doesn't work either (broken file-list, package cannot be removed)
#       directory/file

$(MISC)/$(TARGET)/empty.tar :
    @$(MKDIRHIER) $(@:d)/empty
    @tar -C $(MISC)/$(TARGET)/empty -cf $@ .
    
# --- packaging ---------------------------------------------------
    
$(MENUFILE) : $(MENUDEPN) $(MISC)/$(TARGET)/empty.tar
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$/$(PKGNAME)-*.tgz 
    @$(COPY) $(MISC)/$(TARGET)/empty.tar $@.tmp
    @tar -C $(MISC)/$(TARGET) --owner=root --group=root --same-owner --exclude application.flag -rf $@.tmp install usr opt
    @gzip < $@.tmp > $@
    @$(RM) $@.tmp

.ENDIF
