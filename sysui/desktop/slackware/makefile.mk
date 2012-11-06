#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..

PRJNAME=sysui
TARGET=slackware

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

MENUFILES=$(PKGDIR)$/{$(PRODUCTLIST)}$(PRODUCTVERSION)-$(TARGET)-menus-$(PKGVERSION)-noarch-$(PKGREV).tgz

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
    /bin/sh -c "cd $(COMMONMISC)$/$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//); DESTDIR=$(shell @cd $*; pwd) GNOMEDIR="" ICON_PREFIX=$(ICONPREFIX.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)) KDEMAINDIR=/opt/kde .$/create_tree.sh"
    $(RM) $*$/opt$/kde$/share$/icons$/*$/*$/*$/*-extension.png	
    $(RM) $*$/opt$/kde$/share$/mimelnk$/application$/*-extension.desktop 
    $(RM) $*$/usr$/share$/applications$/*.desktop
    $(RM) $*$/usr$/bin$/soffice

%$/install$/doinst.sh : update-script
    @echo "( cd etc ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)) )" > $@
    @echo "( cd etc ; ln -snf /opt/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//):s/-//) $(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)) )" >> $@
    @echo "( cd usr/bin ; rm -rf soffice )" >> $@
    @echo "( cd usr/bin ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/program/soffice soffice )" >> $@
    @echo -e $(foreach,i,$(shell @cat $(COMMONMISC)$/$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)/launcherlist) "\n( cd usr/share/applications ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))-$i )\n( cd usr/share/applications ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/share/xdg/$i $(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))-$i )") >> $@
    @cat $< >> $@

%$/install$/slack-desc : slack-desc
    @$(MKDIRHIER) $(@:d)
    @sed -e "s/PKGNAME/$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)-$(TARGET)-menus/g" -e "s/PKGVERSION/$(PKGVERSION.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" \
        -e "s/LONGPRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//)) $(PRODUCTVERSION.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" \
        -e "s/PRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" \
        -e "s/UNIXFILENAME/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" $< > $@

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
