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
TARGET=solaris

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------
 
.IF "$(OS)"=="SOLARIS"

PKGFILES = $(BIN)$/pkg$/{$(PRODUCTLIST)}-desktop-integration.tar.gz
   
PKGDATESTRING = $(shell @date -u +%Y.%m.%d)
PKGARCH=all

FASPAC=`test -f $(SOLARBINDIR)/faspac-so.sh && echo "/sbin/sh" || echo "echo"`

.ENDIF
    
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(OS)"=="SOLARIS"

ALLTAR : $(PKGFILES) 

# --- pkginfo ----------------------------------------------------

# Copy the pkginfo file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/{pkginfo depend} : $$(@:f) ../productversion.mk makefile.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PRODUCTNAME/$(PRODUCTNAME.$(@:d:d:f)) $(PRODUCTVERSION.$(@:d:d:f))/g" -e "s/%pkgprefix/$(@:d:d:f:s/.//)/" > $@

# --- mailcap ---------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}/mailcap : $$(@:f) ../productversion.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(MISC)/$(@:d:d:f)_g" > $@

# --- copyright--------------------------------

# Copy the copyright file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/copyright : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- postinstall--------------------------------


# Copy the postinstall file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/postinstall : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- postremove--------------------------------

# Copy the postremove file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/postremove : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- openoffice.xml---------------------------

.IF "$(common_build)"=="TRUE"
# Copy the openoffice.org.xml file to $(MISC) 
#$(MISC)/$(foreach,i,$(PRODUCTLIST) $i/$i.xml) :  $(COMMONMISC)/$$(@:b)/openoffice.org.xml
$(MISC)/$(foreach,i,$(PRODUCTLIST) $(MISC)/$i/openoffice.org.xml) :  $(COMMONMISC)/$$(@:b)/openoffice.org.xml
    $(MKDIRHIER) $(@:d)
    echo hier hier $@
    cat $< | tr -d "\015" > $@
.ENDIF			# "$(COMMON_BUILD)"==""

# --- prototype ---------------------------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}$/prototype : $$(@:f) ../productversion.mk makefile.mk
    @$(MKDIRHIER) $(@:d)
    cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(COMMONMISC)/$(@:d:d:f)_g" -e "s/%ICONPREFIX/$(ICONPREFIX.$(@:d:d:f))/g" > $@

# --- packaging ---------------------------------------------------

#$(PKGFILES) : $(MISC)/{$(PRODUCTLIST)}/{copyright pkginfo depend mailcap postinstall postremove} makefile.mk $(MISC)/$$(@:b:b:s/-desktop-integration//)/$$(@:b:b:s/-desktop-integration//).xml
$(PKGFILES) : $(MISC)/{$(PRODUCTLIST)}/{copyright pkginfo depend mailcap postinstall postremove} makefile.mk $(MISC)/$$(@:b:b:s/-desktop-integration//)/openoffice.org.xml
$(PKGFILES) : $(MISC)$/{$(PRODUCTLIST)}$/prototype
    @-$(RM) $(BIN)$/$(@:f)
    @$(MKDIRHIER) $(@:d)
    pkgmk -l 1073741824 -r . -f $(MISC)$/$(@:b:b:s/-/ /:1)$/prototype -o -d $(PKGDIR) ARCH=$(PKGARCH) VERSION=$(PKGVERSION.$(@:b:s/-/ /:1)),REV=$(PKGREV).$(PKGDATESTRING) 
.IF "$(DONTCOMPRESS)"==""
    $(FASPAC) $(SOLARBINDIR)/faspac-so.sh -a -d $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int
.ENDIF # "$(DONTCOMPRESS)"==""
    @$(GNUTAR) -cf - -C $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int | gzip > $@
    @rm -rf $(PKGDIR)/$(@:b:b:s/-/ /:1:s/.//)$(PRODUCTVERSIONSHORT)-desktop-int

.ENDIF
