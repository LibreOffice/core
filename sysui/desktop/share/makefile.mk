#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2005-07-18 14:00:13 $
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
TARGET=desktopshare

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

# gnome-vfs treats everything behind the last '.' as an icon extension, 
# even though the "icon_filename" in '.keys' is specified as filename 
# without extension. Since it alos does not know how to handle "9-writer"
# type icons :-), we are stripping all '.' for now.
ICONPREFIX = $(UNIXFILENAME:s/.//g)

ULFFILES= \
    documents.ulf \
    launcher_comment.ulf \
    launcher_name.ulf

LAUNCHERLIST = writer calc draw impress math base printeradmin
LAUNCHERDEPN = ../menus/{$(LAUNCHERLIST)}.desktop

LAUNCHERFLAGFILE = $(COMMONMISC)/$(TARGET)/xdg.flag
MIMEINFO = $(COMMONMISC)/$(TARGET)/openoffice.org.xml

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(GUI)"=="UNX"
ALLTAR : $(LAUNCHERFLAGFILE) $(MIMEINFO)
.ENDIF          # "$(GUI)"=="UNIX"

#
# Copy/patch the .desktop files to the output tree and 
# merge-in the translations. 
#
$(LAUNCHERFLAGFILE) : $(LAUNCHERDEPN) ../productversion.mk brand.pl translate.pl $(COMMONMISC)$/$(TARGET)$/launcher_name.ulf $(COMMONMISC)$/$(TARGET)$/launcher_comment.ulf
    @$(MKDIRHIER) $(@:db).$(INPATH)
    @echo Creating desktop entries ..
    @echo ---------------------------------
    @$(PERL) brand.pl -p "$(LONGPRODUCTNAME)" -u $(UNIXFILENAME) --iconprefix "$(ICONPREFIX)-" $(LAUNCHERDEPN) $(@:db).$(INPATH)
    @$(PERL) translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db).$(INPATH) --ext "desktop" --key "Name" $(COMMONMISC)$/$(TARGET)$/launcher_name.ulf
    @$(PERL) translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db).$(INPATH) --ext "desktop" --key "Comment" $(COMMONMISC)$/$(TARGET)$/launcher_comment.ulf
.IF "$(WITH_LIBSN)"=="YES"
    @$(foreach,i,$(LAUNCHERLIST) $(shell echo "StartupNotify=true" >> $(@:db).$(INPATH)/$i.desktop))
.ENDIF
    @mv -f $(@:db).$(INPATH)/* $(@:d)
    @touch $@

#
# Create shared mime info xml file
#
$(MIMEINFO) : $(shell ls ../mimetypes/*.desktop) create_mime_xml.pl
$(MIMEINFO) : $(COMMONMISC)$/$(TARGET)$/documents.ulf
    @echo Create shared mime info xml file ..
    @echo ---------------------------------
    @$(PERL) create_mime_xml.pl $< > $(@).$(INPATH)
    @mv -f $(@).$(INPATH) $@


#
# Install section
#

%.xml : $(COMMONMISC)/$(TARGET)/$$(@:f)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
#	@chmod 0544 $@
