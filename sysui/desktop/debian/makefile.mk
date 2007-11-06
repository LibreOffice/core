#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:57:20 $
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
TARGET=debian

# !!! FIXME !!!
# debian-menus file.

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  packtools.mk

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
