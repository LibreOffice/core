#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..

PRJNAME=sysui
TARGET=slackware

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

MENUFILES=$(PKGDIR)$/{$(PRODUCTLIST)}3.5-$(TARGET)-menus-$(PKGVERSION)-noarch-$(PKGREV).tgz

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
    /bin/sh -c "cd $(COMMONMISC)$/$(*:b:s/-/ /:1:s/3.5//); DESTDIR=$(shell @cd $*; pwd) GNOMEDIR="" ICON_PREFIX=$(ICONPREFIX.$(*:b:s/-/ /:1:s/3.5//)) KDEMAINDIR=/opt/kde .$/create_tree.sh"
    $(RM) $*$/opt$/kde$/share$/icons$/*$/*$/*$/*-extension.png	
    $(RM) $*$/opt$/kde$/share$/mimelnk$/application$/*-extension.desktop 
    $(RM) $*$/usr$/share$/applications$/*.desktop
    $(RM) $*$/usr$/bin$/soffice

%$/install$/doinst.sh : update-script
    @echo "( cd etc ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//)) )" > $@
    @echo "( cd etc ; ln -snf /opt/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//):s/-//) $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//)) )" >> $@
    @echo "( cd usr/bin ; rm -rf soffice )" >> $@
    @echo "( cd usr/bin ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//))/program/soffice soffice )" >> $@
    @echo -e $(foreach,i,$(shell @cat $(COMMONMISC)$/$(*:b:s/-/ /:1:s/3.5//)/launcherlist) "\n( cd usr/share/applications ; rm -rf $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//))-$i )\n( cd usr/share/applications ; ln -sf /etc/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//))/share/xdg/$i $(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//))-$i )") >> $@
    @cat $< >> $@

%$/install$/slack-desc : slack-desc
    @$(MKDIRHIER) $(@:d)
    @sed -e "s/PKGNAME/$(*:b:s/-/ /:1:s/3.5//)-$(TARGET)-menus/g" -e "s/PKGVERSION/$(PKGVERSION.$(*:b:s/-/ /:1:s/3.5//))/g" \
        -e "s/LONGPRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/3.5//)) $(PRODUCTVERSION.$(*:b:s/-/ /:1:s/3.5//))/g" \
        -e "s/PRODUCTNAME/$(PRODUCTNAME.$(*:b:s/-/ /:1:s/3.5//))/g" \
        -e "s/UNIXFILENAME/$(UNIXFILENAME.$(*:b:s/-/ /:1:s/3.5//))/g" $< > $@

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
