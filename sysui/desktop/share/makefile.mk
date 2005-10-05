#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2005-10-05 13:00:54 $
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
TARGET=desktopshare

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk
.INCLUDE :  packtools.mk

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

.IF "$(RPM)"!=""
SPECFILES = \
    $(MISC)/redhat-menus.spec \
    $(MISC)/suse-menus.spec \
    $(MISC)/freedesktop-menus.spec \
    $(MISC)/mandriva-menus.spec
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(GUI)"=="UNX"
ALLTAR : $(LAUNCHERFLAGFILE) $(MIMEINFO) $(SPECFILES)
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
    @+echo x$(foreach,i,$(LAUNCHERLIST) $(shell +echo "StartupNotify=true" >> $(@:db).$(INPATH)/$i.desktop))x $(noout)
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

.IF "$(RPM)"!=""
$(SPECFILES) : add_specfile_triggers.sed symlink_triggers
$(SPECFILES) : ../$$(@:b:s/-menus//)/$$(@:f)
    @sed -f ../share/add_specfile_triggers.sed $< | tr -d "\015" >$@
.ENDIF

#
# Install section
#

%.xml : $(COMMONMISC)/$(TARGET)/$$(@:f)
    @$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
#	@chmod 0544 $@
