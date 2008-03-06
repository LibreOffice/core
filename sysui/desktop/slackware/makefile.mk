#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:00:42 $
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

MENUFILES=$(PKGDIR)$/{$(PRODUCTLIST)}-$(TARGET)-menus-$(PKGVERSION)-noarch-$(PKGREV).tgz

.IF "$(USE_SHELL)"=="bash"
ECHOPARAM=-e
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)" == "LINUX"

ALLTAR : $(MENUFILES) 

$(MISC)/$(TARGET)/usr/share/applications/ :
    @$(MKDIRHIER) $(@)
    
# --- slackware-specific stuff ------------------------------------
# symlinks shall not be included in the tarball, but created with the doinst-script

# FIXME: removal of *-extension.* only to create identical packages to OOF680
%/usr/share/applications : 
    @$(MKDIRHIER) $@
    /bin/sh -c "cd $(COMMONMISC)$/$(*:b:s/-/ /:1); DESTDIR=$(shell @cd $*; pwd) GNOMEDIR="" ICON_PREFIX=$(ICONPREFIX.$(*:b:s/-/ /:1)) KDEMAINDIR=/opt/kde .$/create_tree.sh"
    $(RM) $*$/opt$/kde$/share$/icons$/*$/*$/*$/*-extension.png	
    $(RM) $*$/opt$/kde$/share$/mimelnk$/application$/*-extension.desktop 
    $(RM) $*$/usr$/share$/applications$/*.desktop
    $(RM) $*$/usr$/bin$/soffice
    $(RM) $*$/usr$/bin$/unopkg

%$/install$/doinst.sh : update-script
    @echo "( cd etc ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1)) )" > $@
    @echo "( cd etc ; ln -snf /opt/$(UNIXFILENAME.$(*:b:s/-/ /:1):s/-//) $(UNIXFILENAME.$(*:b:s/-/ /:1)) )" >> $@
    @echo "( cd usr/bin ; rm -rf soffice )" >> $@
    @echo "( cd usr/bin ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1))/program/soffice soffice )" >> $@
    @echo $(ECHOPARAM) $(foreach,i,$(shell @sed  's/extensionmgr.desktop//' $(COMMONMISC)$/$(*:b:s/-/ /:1)/launcherlist) "\n( cd usr/share/applications ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1))-$i )\n( cd usr/share/applications ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1))/share/xdg/$i $(UNIXFILENAME.$(*:b:s/-/ /:1))-$i )") >> $@
    @cat $< >> $@

%$/install$/slack-desc : slack-desc
    @$(MKDIRHIER) $(@:d)
    @sed -e "s/PKGNAME/$(*:b:s/-/ /:1)-$(TARGET)-menus/g" -e "s/PKGVERSION/$(PKGVERSION.$(*:b:s/-/ /:1))/g" \
        -e "s/LONGPRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1)) $(PRODUCTVERSION.$(*:b:s/-/ /:1))/g" \
        -e "s/PRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1))/g" \
        -e "s/UNIXFILENAME/$(UNIXFILENAME.$(*:b:s/-/ /:1))/g" $< > $@

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
    @tar -C $(MISC)/$(TARGET)/empty --owner=root --group=root --same-owner -cf $@ .


# --- packaging ---------------------------------------------------

$(MENUFILES) : makefile.mk slack-desc update-script $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag
$(MENUFILES) : $(MISC)/$(TARGET)/empty.tar
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$/$(@:b:s/-/ /:1)-$(TARGET)-menus-*.tgz 
    -$(RM) -r $(MISC)$/$(@:b)
    dmake $(MISC)$/$(@:b)$/usr/share/applications $(MISC)$/$(@:b)$/install$/slack-desc $(MISC)$/$(@:b)$/install$/doinst.sh
    @$(COPY) $(MISC)/$(TARGET)$/empty.tar $@.tmp
    @tar -C $(MISC)/$(@:b) --owner=root --group=root --same-owner --exclude application.flag -rf $@.tmp install usr opt
    @gzip < $@.tmp > $@
    @$(RM) $@.tmp
    $(RM) -r $(MISC)$/$(@:b)

.ENDIF
