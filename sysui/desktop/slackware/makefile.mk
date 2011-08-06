#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

MENUFILES=$(PKGDIR)$/{$(PRODUCTLIST)}3.4-$(TARGET)-menus-$(PKGVERSION)-noarch-$(PKGREV).tgz

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
    /bin/sh -c "cd $(COMMONMISC)$/$(*:b:s/-/ /:1:s/3.4//); DESTDIR=$(shell @cd $*; pwd) GNOMEDIR="" ICON_PREFIX=$(ICONPREFIX.$(*:b:s/-/ /:1:s/3.4//)) KDEMAINDIR=/opt/kde .$/create_tree.sh"
    $(RM) $*$/opt$/kde$/share$/icons$/*$/*$/*$/*-extension.png	
    $(RM) $*$/opt$/kde$/share$/mimelnk$/application$/*-extension.desktop 
    $(RM) $*$/usr$/share$/applications$/*.desktop
    $(RM) $*$/usr$/bin$/soffice

%$/install$/doinst.sh : update-script
    @echo "( cd etc ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//)) )" > $@
    @echo "( cd etc ; ln -snf /opt/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//):s/-//) $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//)) )" >> $@
    @echo "( cd usr/bin ; rm -rf soffice )" >> $@
    @echo "( cd usr/bin ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//))/program/soffice soffice )" >> $@
    @echo -e $(foreach,i,$(shell @cat $(COMMONMISC)$/$(*:b:s/-/ /:1:s/3.4//)/launcherlist) "\n( cd usr/share/applications ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//))-$i )\n( cd usr/share/applications ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//))/share/xdg/$i $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//))-$i )") >> $@
    @cat $< >> $@

%$/install$/slack-desc : slack-desc
    @$(MKDIRHIER) $(@:d)
    @sed -e "s/PKGNAME/$(*:b:s/-/ /:1:s/3.4//)-$(TARGET)-menus/g" -e "s/PKGVERSION/$(PKGVERSION.$(*:b:s/-/ /:1:s/3.4//))/g" \
        -e "s/LONGPRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/3.4//)) $(PRODUCTVERSION.$(*:b:s/-/ /:1:s/3.4//))/g" \
        -e "s/PRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/3.4//))/g" \
        -e "s/UNIXFILENAME/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.4//))/g" $< > $@

# needed to satisfy the slackware package tools - they need
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
    @$(GNUTAR) -C $(MISC)/$(TARGET)/empty --owner=root --group=root --same-owner -cf $@ .


# --- packaging ---------------------------------------------------

$(MENUFILES) : makefile.mk slack-desc update-script $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag
$(MENUFILES) : $(MISC)/$(TARGET)/empty.tar
    @-$(MKDIRHIER) $(@:d)
    -$(RM) $(@:d)$/$(@:b:s/-/ /:1)-$(TARGET)-menus-*.tgz 
    -$(RM) -r $(MISC)$/$(@:b)
    dmake $(MISC)$/$(@:b)$/usr/share/applications $(MISC)$/$(@:b)$/install$/slack-desc $(MISC)$/$(@:b)$/install$/doinst.sh
    @$(COPY) $(MISC)/$(TARGET)$/empty.tar $@.tmp
    @$(GNUTAR) -C $(MISC)/$(@:b) --owner=root --group=root --same-owner --exclude application.flag -rf $@.tmp install usr opt
    @gzip < $@.tmp > $@
    @$(RM) $@.tmp
    $(RM) -r $(MISC)$/$(@:b)

.ENDIF
