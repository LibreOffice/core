#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2006-01-13 14:57:10 $
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
TARGET=freedesktop

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  packtools.mk

# --- Product Version Information ----------------------------------

.INCLUDE :  ../productversion.mk

# --- Files --------------------------------------------------------

.IF "$(RPM)"!=""

PACKAGE_RPM = $(MISC)$/$(TARGET).rpmflag

.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(RPM)"!=""

ALLTAR : $(PACKAGE_RPM)

# --- packaging ---------------------------------------------------
    
$(PACKAGE_RPM) : $(MISC)$/redhat.rpmflag

$(PACKAGE_RPM) : $(MISC)/$(TARGET)-menus.spec
    -@$(RM) $(PKGDIR)$/openoffice.org-$(TARGET)-*.noarch.rpm $(BIN)/noarch/openoffice.org-$(TARGET)-*.noarch.rpm
    @$(MKDIRHIER) $(MISC)$/$(TARGET)
    @$(MKDIRHIER) $(MISC)$/$(TARGET)$/BUILD
    @$(RPM) $(RPMMACROS) -bb $< \
        --define "basedir $(PWD)" --define "unixfilename $(UNIXFILENAME)" \
        --define "version $(PKGVERSION)" --define "release $(PKGREV)" \
        --define "source $(ABSLOCALOUT)$/misc$/redhat" \
        --define "unique $(shell echo $$$$)" \
        --define "_builddir $(ABSLOCALOUT)$/misc$/$(TARGET)/BUILD" 
    @touch $@
    
.ENDIF
