#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:36:34 $
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
PRJNAME = jvmfwk
FRAMEWORKLIB=jvmfwk
TARGET = $(FRAMEWORKLIB)
ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

UNOUCROUT = $(OUT)$/inc

SLOFILES = \
    $(SLO)$/framework.obj \
    $(SLO)$/libxmlutil.obj \
    $(SLO)$/fwkutil.obj \
    $(SLO)$/elements.obj \
    $(SLO)$/fwkbase.obj


#LIB1TARGET=$(SLB)$/$(FRAMEWORKLIB).lib

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(FRAMEWORKLIB)$(UDK_MAJOR)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(FRAMEWORKLIB)
.ENDIF # UNIXVERSIONNAMES

#SHL1TARGET=$(FRAMEWORKLIB)
SHL1DEPN=
SHL1IMPLIB = i$(FRAMEWORKLIB)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(SALHELPERLIB) $(XML2LIB)

.IF "$(OS)" == "WNT"
SHL1STDLIBS += advapi32.lib
.ENDIF # WNT

SHL1VERSIONMAP = framework.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.IF "$(GUI)"=="UNX"
RCFILE=$(BIN)$/jvmfwk3rc
.ELIF "$(GUI)"=="WNT"
RCFILE=$(BIN)$/jvmfwk3.ini
.END


.INCLUDE: target.mk
$(RCFILE): jvmfwk3rc
    +-$(COPY) $< $@

ALLTAR: \
    $(RCFILE)

