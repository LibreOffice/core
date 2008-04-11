#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.25 $
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
TARGET=debian

# !!! FIXME !!!
# debian-menus file.

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

DEBFILES=$(foreach,i,{$(PRODUCTLIST)} $(PKGDIR)$/$i-$(TARGET)-menus_$(PKGVERSION.$i)-$(PKGREV)_all.deb)

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(PKGFORMAT)"!="$(PKGFORMAT:s/deb//)"

ALLTAR : $(DEBFILES) 


%/DEBIAN/control : $$(@:f)
    @$(MKDIRHIER) $(@:d) $*$/etc $*$/usr/share/applnk/Office $*$/usr/lib/menu
    ln -sf /opt/$(UNIXFILENAME.$(*:f:s/-/ /:1)) $*$/etc$/
    /bin/sh -c -x "cd $(COMMONMISC)$/$(*:f:s/-/ /:1) && DESTDIR=$(shell @cd $*; pwd) ICON_PREFIX=$(ICONPREFIX) KDEMAINDIR=/usr GNOMEDIR=/usr create_tree.sh"
        @cat openoffice.org-debian-menus | sed -e 's/%PRODUCTNAME/$(PRODUCTNAME.$(*:f:s/-/ /:1)) $(PRODUCTVERSION.$(*:f:s/-/ /:1))/' -e 's/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1))/' -e 's/%ICONPREFIX/$(ICONPREFIX.$(*:f:s/-/ /:1))/' > $*$/usr/lib/menu/$(*:f:s/_/ /:1)
    echo "Package: $(*:f:s/_/ /:1)" > $@
    cat $(@:f) | tr -d "\015" | sed "s/%productname/$(PRODUCTNAME.$(*:f:s/-/ /:1))/" >> $@
    echo "Version: $(PKGVERSION.$(*:f:s/-/ /:1))-$(PKGREV)" >> $@
    @du -k -s $* | awk -F ' ' '{ printf "Installed-Size: %s\n", $$1 ; }' >> $@

%/DEBIAN/postinst : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1))/g" > $@

%/DEBIAN/postrm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1))/g" > $@

%/DEBIAN/prerm : $$(@:f)
     @cat $< | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(*:f:s/-/ /:1))/g" > $@

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
