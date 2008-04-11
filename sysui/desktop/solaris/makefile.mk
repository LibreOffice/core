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
# $Revision: 1.20 $
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
PKGARCH=sparc,i386

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

# --- space, postinstall & mailcap ---------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}/{space postinstall checkinstall mailcap} : $$(@:f) ../productversion.mk
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(MISC)/$(@:d:d:f)_g" > $@

# --- checkinstall & copyright--------------------------------

# Copy the checkinstall and copyright file to $(MISC) 
$(MISC)/{$(PRODUCTLIST)}/{copyright} : $$(@:f)
    @$(MKDIRHIER) $(@:d)
    @cat $(@:f) | tr -d "\015" > $@

# --- prototype ---------------------------------------------------

# Copy the prototype file to $(MISC)
$(MISC)/{$(PRODUCTLIST)}$/prototype : $$(@:f) ../productversion.mk $(COMMONMISC)$/$$(@:d:d:f)$/cdelauncherlist makefile.mk
    @$(MKDIRHIER) $(@:d)
    cat $(@:f) | tr -d "\015" | sed -e "s/%PREFIX/$(UNIXFILENAME.$(@:d:d:f))/g" -e "s_%SOURCE_$(COMMONMISC)/$(@:d:d:f)_g" -e "s/%ICONPREFIX/$(ICONPREFIX.$(@:d:d:f))/g" > $@
    pkgproto $(COMMONMISC)$/$(@:d:d:f)/types=usr/dt/appconfig/types | awk '{ printf "%s %s %s 0%d%d root bin\n",$$1, $$2, $$3, $$4/100, $$4%10*11 }' >> $@

# --- packaging ---------------------------------------------------

$(PKGFILES) : $(MISC)/{$(PRODUCTLIST)}/{checkinstall copyright space pkginfo depend postinstall mailcap} makefile.mk
$(PKGFILES) : $(MISC)$/{$(PRODUCTLIST)}$/prototype
    @-$(RM) $(BIN)$/$(@:f)
    @$(MKDIRHIER) $(@:d)
    pkgmk -r . -f $(MISC)$/$(@:b:b:s/-/ /:1)$/prototype -o -d $(PKGDIR) ARCH=$(PKGARCH) VERSION=$(PKGVERSION.$(@:b:s/-/ /:1)),REV=$(PKGREV).$(PKGDATESTRING)
    $(FASPAC) $(SOLARBINDIR)/faspac-so.sh -a -d $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)-desktop-integratn
    @tar -cf - -C $(PKGDIR) $(@:b:b:s/-/ /:1:s/.//)-desktop-integratn | gzip > $@
    @rm -rf $(PKGDIR)/$(@:b:b:s/-/ /:1:s/.//)-desktop-integratn

.ENDIF
