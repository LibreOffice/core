#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2006-11-08 11:55:36 $
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

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

ULFFILES= \
    documents.ulf \
    launcher_comment.ulf \
    launcher_name.ulf

LAUNCHERLIST = writer calc draw impress math base printeradmin qstart
LAUNCHERDEPN = ../menus/{$(LAUNCHERLIST)}.desktop

LAUNCHERFLAGFILE = $(COMMONMISC)/$(TARGET)/xdg.flag
MIMEINFO = $(COMMONMISC)/$(TARGET)/openoffice.org.xml

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"
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
$(LAUNCHERFLAGFILE) : $(LAUNCHERDEPN) ../productversion.mk brand.pl translate.pl $(ULFDIR)$/launcher_name.ulf $(ULFDIR)$/launcher_comment.ulf
    @$(MKDIRHIER) $(@:db).$(INPATH)
    @echo Creating desktop entries ..
    @echo ---------------------------------
    @$(PERL) brand.pl -p "$(LONGPRODUCTNAME)" -u $(UNIXFILENAME) --iconprefix "$(ICONPREFIX)-" $(LAUNCHERDEPN) $(@:db).$(INPATH)
    @$(PERL) translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db).$(INPATH) --ext "desktop" --key "Name" $(ULFDIR)$/launcher_name.ulf
    @$(PERL) translate.pl -p "$(LONGPRODUCTNAME)" -d $(@:db).$(INPATH) --ext "desktop" --key "Comment" $(ULFDIR)$/launcher_comment.ulf
.IF "$(WITH_LIBSN)"=="YES"
    @noop x$(foreach,i,$(LAUNCHERLIST) $(shell +echo "StartupNotify=true" >> $(@:db).$(INPATH)/$i.desktop))x
.ENDIF
    @mv -f $(@:db).$(INPATH)/* $(@:d)
    @touch $@

#
# Create shared mime info xml file
#
$(MIMEINFO) : $(shell ls ../mimetypes/*.desktop) create_mime_xml.pl
$(MIMEINFO) : $(ULFDIR)$/documents.ulf
    @echo Create shared mime info xml file ..
    @echo ---------------------------------
    @$(PERL) create_mime_xml.pl $< > $(@).$(INPATH)
    @mv -f $(@).$(INPATH) $@

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/rpm//)"
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
