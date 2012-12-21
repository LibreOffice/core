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
TARGET=debian

# !!! FIXME !!!
# debian-menus file.

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

DEBFILES=$(foreach,i,{$(PRODUCTLIST)} $(PKGDIR)$/$i3.5-$(TARGET)-menus_$(PKGVERSION.$i)-$(PKGREV)_all.deb)

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

ALLTAR : $(DEBFILES) 


%/DEBIAN/control : $$(@:f)
    @$(MKDIRHIER) $(@:d) $*$/etc $*$/usr/share/applnk/Office $*$/usr/lib/menu
    ln -sf /opt/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/3.5//)) $*$/etc$/
    /bin/sh -c -x "cd $(COMMONMISC)$/$(*:f:s/-/ /:1:s/3.5//) && DESTDIR=$(shell @cd $*; pwd) ICON_PREFIX=$(ICONPREFIX) KDEMAINDIR=/usr GNOMEDIR=/usr create_tree.sh"
        @cat openoffice.org-debian-menus | sed -e 's/%PRODUCTNAME/$(PRODUCTNAME.$(*:f:s/-/ /:1:s/3.5//)) $(PRODUCTVERSION.$(*:f:s/-/ /:1:s/3.5//))/' -e 's/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/3.5//))/' -e 's/%ICONPREFIX/$(ICONPREFIX.$(*:f:s/-/ /:1:s/3.5//))/' > $*$/usr/lib/menu/$(*:f:s/_/ /:1:s/3.5//)
    echo "Package: $(*:f:s/_/ /:1:s/3.5//)" > $@
    cat $(@:f) | tr -d "\015" | sed "s/%productname/$(PRODUCTNAME.$(*:f:s/-/ /:1:s/3.5//))/" >> $@
    echo "Version: $(PKGVERSION.$(*:f:s/-/ /:1:s/3.5//))-$(PKGREV)" >> $@
    @du -k -s $* | awk -F ' ' '{ printf "Installed-Size: %s\n", $$1 ; }' >> $@

%/DEBIAN/postinst : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/3.5//))/g" > $@

%/DEBIAN/postrm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/3.5//))/g" > $@

%/DEBIAN/prerm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/3.5//))/g" > $@

# --- packaging ---------------------------------------------------

# getuid.so fakes the user/group for us	
$(DEBFILES) : $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag
$(DEBFILES) : makefile.mk control postinst postrm prerm
    -$(RM) $(@:d)$(@:f:s/_/ /:1)_*
    $(RM) -r $(MISC)$/$(@:b)
    dmake $(MISC)$/$(@:b)$/DEBIAN$/{control postinst postrm prerm} 
    @chmod -R o-w $(MISC)$/$(@:b)
    @chmod a+rx $(MISC)$/$(@:b)$/DEBIAN $(MISC)/$(@:b)/DEBIAN/post* $(MISC)/$(@:b)/DEBIAN/pre*
    @chmod g-s $(MISC)/$(@:b)/DEBIAN
    @mkdir -p $(PKGDIR)
    /bin/bash -c "LD_PRELOAD=$(SOLARBINDIR)/getuid.so dpkg-deb --build $(MISC)/$(@:b) $@" 
    $(RM) -r $(MISC)$/$(@:b)
#	@chmod -R g+w $(MISC)/$(TARGET)/$(DEBFILE:f)

.ENDIF
