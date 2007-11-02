#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 12:41:23 $
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

PRJ = ..
PRJNAME = jvmaccess
TARGET = $(PRJNAME)

ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT" && "$(OS)" != "OS2"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(TARGET)$(UDK_MAJOR)$(COMID)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(TARGET)$(COMID)
.ENDIF # UNIXVERSIONNAMES

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPULIB) $(SALLIB) $(SALHELPERLIB)
.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT
SHL1RPATH = URELIB

.IF "$(COMNAME)" == "msci"
SHL1VERSIONMAP = msvc_win32_intel.map
.ELIF "$(COMNAME)" == "sunpro5"
SHL1VERSIONMAP = cc5_solaris_sparc.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL1VERSIONMAP = gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "FREEBSDIgcc3"
SHL1VERSIONMAP = gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "LINUXSgcc3"
SHL1VERSIONMAP = gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "OS2Igcc3"
SHL1VERSIONMAP = gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)"=="MACOSXIgcc3"
SHL1VERSIONMAP = gcc3_linux_intel.map
.ELIF "$(GUI)$(COM)" == "WNTGCC"
SHL1VERSIONMAP = mingw.map
.ENDIF

DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
