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
TARGET=debian

# !!! FIXME !!!
# debian-menus file.

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

DEBFILES=$(foreach,i,{$(PRODUCTLIST)} $(PKGDIR)$/$i$(PRODUCTVERSION)-$(TARGET)-menus_$(PKGVERSION.$i)-$(PKGREV)_all.deb)

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

ALLTAR : $(DEBFILES) 


%/DEBIAN/control : $$(@:f)
    @$(MKDIRHIER) $(@:d) $*$/etc $*$/usr/share/applnk/Office $*$/usr/lib/menu
    ln -sf /opt/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//)) $*$/etc$/
    /bin/sh -c -x "cd $(COMMONMISC)$/$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//) && DESTDIR=$(shell @cd $*; pwd) ICON_PREFIX=$(ICONPREFIX) KDEMAINDIR=/usr GNOMEDIR=/usr create_tree.sh"
        @cat openoffice.org-debian-menus | sed -e 's/%PRODUCTNAME/$(PRODUCTNAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//)) $(PRODUCTVERSION.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/' -e 's/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/' -e 's/%ICONPREFIX/$(ICONPREFIX.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/' > $*$/usr/lib/menu/$(*:f:s/_/ /:1:s/$(PRODUCTVERSION)//)
    echo "Package: $(*:f:s/_/ /:1:s/$(PRODUCTVERSION)//)" > $@
    cat $(@:f) | tr -d "\015" | sed "s/%productname/$(PRODUCTNAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/" >> $@
    echo "Version: $(PKGVERSION.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))-$(PKGREV)" >> $@
    @du -k -s $* | awk -F ' ' '{ printf "Installed-Size: %s\n", $$1 ; }' >> $@

%/DEBIAN/postinst : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" > $@

%/DEBIAN/postrm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" > $@

%/DEBIAN/prerm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1:s/$(PRODUCTVERSION)//))/g" > $@

# --- packaging ---------------------------------------------------

# getuid.so fakes the user/group for us	
$(DEBFILES) : $(COMMONMISC)$/{$(PRODUCTLIST)}$/build.flag
$(DEBFILES) : makefile.mk control postinst postrm prerm
    -$(RM) $(@:d)$(@:f:s/_/ /:1)_*
    $(RM) -r $(MISC)$/$(@:b)
    dmake $(MISC)$/$(@:b)$/DEBIAN$/{control postinst postrm prerm} 
    @chmod -R g-w $(MISC)$/$(@:b)
    @chmod a+rx $(MISC)$/$(@:b)$/DEBIAN $(MISC)/$(@:b)/DEBIAN/post* $(MISC)/$(@:b)/DEBIAN/pre*
    @chmod g-s $(MISC)/$(@:b)/DEBIAN
    @mkdir -p $(PKGDIR)
    /bin/bash -c "LD_PRELOAD=$(SOLARBINDIR)/getuid.so dpkg-deb --build $(MISC)/$(@:b) $@" 
    $(RM) -r $(MISC)$/$(@:b)
#	@chmod -R g+w $(MISC)/$(TARGET)/$(DEBFILE:f)

.ENDIF
