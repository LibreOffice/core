#*************************************************************************
#*
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

PRJ=..

PRJNAME=svtools
TARGET=svtool
RESTARGET=svt
RESTARGETPATCH=svp
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

USE_LDUMP2=TRUE

# --- general section ----------------------------------------------------

.IF "$(GUI)"!="UNX"
LIB2TARGET= $(LB)$/svtool.lib
LIB2FILES=	$(LB)$/_svt.lib
.ENDIF

LIB1TARGET= $(SLB)/svt.lib
LIB1FILES=	\
        $(SLB)/browse.lib		\
        $(SLB)/config.lib	\
        $(SLB)/svcontnr.lib	\
        $(SLB)/ctrl.lib		\
        $(SLB)/dialogs.lib 	\
        $(SLB)/edit.lib		\
        $(SLB)/filter.lib		\
        $(SLB)/filter.uno.lib	\
        $(SLB)$/graphic.lib	\
        $(SLB)/igif.lib		\
        $(SLB)/jpeg.lib		\
        $(SLB)/ixpm.lib		\
        $(SLB)/ixbm.lib		\
        $(SLB)/wmf.lib 		\
        $(SLB)/java.lib		\
        $(SLB)/misc.lib		\
        $(SLB)/plugapp.lib 	\
        $(SLB)/svhtml.lib   \
        $(SLB)/svrtf.lib	\
        $(SLB)/table.lib 	\
        $(SLB)/unoiface.lib	\
        $(SLB)/unowiz.lib	\
        $(SLB)/urlobj.lib	\
        $(SLB)/toolpanel.lib

# generation of resourcen-lib ----------------------------------------

RESLIB1NAME=	$(RESTARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
        $(SRS)$/filter.srs  \
        $(SRS)$/misc.srs        \
        $(SRS)$/ctrl.srs        \
        $(SRS)$/dialogs.srs     \
        $(SRS)$/plugapp.srs     \
        $(SRS)$/svcontnr.srs    \
        $(SRS)$/browse.srs		\
        $(SRS)$/toolpanel.srs		\
        $(SRS)$/javaerror.srs

RESLIB3NAME= $(RESTARGETPATCH)
RESLIB3SRSFILES= \
        $(SRS)$/patchjavaerror.srs

# build the shared library  --------------------------------------------------

SHL1TARGET= svt$(DLLPOSTFIX)
SHL1IMPLIB= _svt
SHL1USE_EXPORTS=name

.IF "$(OS)"!="MACOSX"
# static libraries
SHL1STDLIBS+= $(JPEG3RDLIB)
.ENDIF

# dynamic libraries
SHL1STDLIBS+= \
        $(TKLIB)			\
        $(VCLLIB)			\
        $(SVLLIB)			\
        $(SOTLIB)			\
        $(BASEGFXLIB)		\
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(I18NISOLANGLIB)   \
        $(I18NUTILLIB)		\
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB)			\
        $(ICUUCLIB)		\
        $(JVMFWKLIB)

.IF "$(OS)"=="MACOSX"
# static libraries go at end
SHL1STDLIBS+= $(JPEG3RDLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(UWINAPILIB) \
        $(ADVAPI32LIB)	\
        $(GDI32LIB) \
        $(OLE32LIB) 	\
        $(UUIDLIB) 	\
        $(ADVAPI32LIB)	\
        $(OLEAUT32LIB)
.ENDIF # WNT

SHL1LIBS= \
    $(SLB)$/svt.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME =svt
DEF1DES 	=SvTools


# --- g2g application --------------------------------------------------

APP2TARGET	=	g2g
APP2BASE	=	0x10000000
APP2DEPN	=   $(SHL1TARGETN) $(SHL2TARGETN)

APP2OBJS	=   $(OBJ)$/g2g.obj

.IF "$(GUI)"!="UNX"
APP2STDLIBS+= $(SVTOOLLIB)
.ELSE
APP2STDLIBS+= -lsvt$(DLLPOSTFIX)
APP2STDLIBS+= -lsvl$(DLLPOSTFIX)
.ENDIF

APP2STDLIBS+=	$(VCLLIB)		\
                $(TOOLSLIB)		\
                $(VOSLIB) 		\
                $(SALLIB)

# --- Targets ------------------------------------------------------

.IF "$(GUI)"=="UNX"
SVTTARGETS= $(LB)$/lib$(SHL1TARGET)$(DLLPOST)
.ELSE
SVTTARGETS= $(BIN)$/$(SHL1TARGET)$(DLLPOST)
.ENDIF

# just a quick fix - has to be cleaned up some day...
.IF "$(L10N-framework)"==""
ALL: $(SLB)$/svt.lib \
    $(MISC)$/$(SHL1TARGET).def \
    $(SVTTARGETS) \
    ALLTAR
.ENDIF          # "$(L10N-framework)"==""

.INCLUDE :	target.mk


