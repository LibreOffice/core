#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2005-10-05 12:57:58 $
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
        
RPMDIR  = $(shell cd $(BIN); pwd)
    
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(RPM)"!=""

ALLTAR : PACKAGE_RPM 

# --- packaging ---------------------------------------------------
    
.PHONY PACKAGE_RPM : $(MISC)/$(TARGET)-menus.spec
    @$(MKDIRHIER) $(MISC)$/$(TARGET)
    @$(MKDIRHIER) $(MISC)$/$(TARGET)$/BUILD
    @$(RPM) --define "basedir $(shell pwd)" --define "unixfilename $(UNIXFILENAME)" \
        --define "version $(PKGVERSION)" --define "release $(PKGREV)" \
        --define "_rpmdir $(RPMDIR)" -bb $< \
        --define "source $(shell cd $(MISC)$/redhat; pwd)" \
        --define "unique $(shell echo $$$$)" \
        --define "_builddir $(shell cd $(MISC)$/$(TARGET)/BUILD; pwd)" 
    
.ENDIF
