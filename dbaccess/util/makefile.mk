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
# $Revision: 1.38.68.2 $
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
PRJNAME=dbaccess
TARGET=dba
TARGET2=dbu
TARGET3=sdbt
USE_DEFFILE=TRUE
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

LDUMP=ldump2.exe

# --- database core (dba) -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
        $(SLB)$/api.lib	\
        $(SLB)$/dataaccess.lib	\
        $(SLB)$/misc.lib	\
        $(SLB)$/core_resource.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(SVTOOLLIB) \
        $(VCLLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(SVLLIB)	\
        $(CPPULIB) \
        $(SALLIB) \
        $(SFXLIB) \
        $(BASICLIB) \
        $(FWELIB) \
        $(SALHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
# SHL1VERSIONMAP=$(TARGET).map
SHL1USE_EXPORTS=name

# --- .res file ----------------------------------------------------------

RES1FILELIST=\
    $(SRS)$/core_strings.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)

# --- database ui (dbu) -----------------------------------

LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=\
        $(SLB)$/relationdesign.lib	\
        $(SLB)$/tabledesign.lib		\
        $(SLB)$/querydesign.lib		\
        $(SLB)$/uimisc.lib			\
        $(SLB)$/uidlg.lib			\
        $(SLB)$/dbushared.lib		\
        $(SLB)$/browser.lib			\
        $(SLB)$/uiuno.lib			\
        $(SLB)$/app.lib				\
        $(SLB)$/uicontrols.lib

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)

SHL2STDLIBS= \
        $(SVXCORELIB)				\
        $(SVXLIB)				\
        $(SFXLIB)				\
        $(SVTOOLLIB)			\
        $(FWELIB)				\
        $(TKLIB)				\
        $(VCLLIB)				\
        $(GOODIESLIB)			\
        $(SVLLIB)				\
        $(SOTLIB)				\
        $(UNOTOOLSLIB)			\
        $(TOOLSLIB)				\
        $(UCBHELPERLIB) 		\
        $(DBTOOLSLIB)			\
        $(COMPHELPERLIB)		\
        $(CPPUHELPERLIB)		\
        $(CPPULIB)				\
        $(SO2LIB)				\
        $(VOSLIB)				\
        $(SALLIB)
        
.IF "$(GUI)"=="OS2"
SHL2STDLIBS+=	$(LB)$/i$(TARGET).lib
SHL2DEPN=$(LB)$/i$(TARGET).lib
.ELIF "$(GUI)"!="WNT" || "$(COM)"=="GCC"
SHL2STDLIBS+= \
        -l$(TARGET)$(DLLPOSTFIX)
SHL2DEPN=$(SHL1TARGETN)
.ELSE
SHL2STDLIBS+= \
        $(LB)$/i$(TARGET).lib
SHL2DEPN=$(LB)$/i$(TARGET).lib
.ENDIF

SHL2IMPLIB=i$(TARGET2)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
DEFLIB2NAME=$(TARGET2)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt \
            $(SLB)$/$(TARGET2).lib
# SHL2VERSIONMAP=$(TARGET2).map
SHL2USE_EXPORTS=name

ALL: \
    $(LIB1TARGET)	\
    $(LIB2TARGET)	\
    ALLTAR

# --- .res file ----------------------------------------------------------

RES2FILELIST=\
    $(SRS)$/uidlg.srs				\
    $(SRS)$/app.srs					\
    $(SRS)$/uicontrols.srs			\
    $(SRS)$/browser.srs				\
    $(SRS)$/uiuno.srs				\
    $(SRS)$/querydesign.srs			\
    $(SRS)$/tabledesign.srs			\
    $(SRS)$/relationdesign.srs		\
    $(SRS)$/uiimglst.srs			\
    $(SRS)$/uimisc.srs

.IF "$(GUI)"=="WNT"
#needed by 		$(SLO)$/adodatalinks.obj
SHL2STDLIBS+=		$(OLE32LIB) \
                    $(OLEAUT32LIB) \
                    $(UUIDLIB)
.ENDIF

RESLIB2NAME=$(TARGET2)
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(RES2FILELIST)

# --- database tools (sdbt) -----------------------------------

LIB3TARGET=$(SLB)$/$(TARGET3).lib
LIB3FILES=\
        $(SLB)$/conntools.lib \
        $(SLB)$/sdbtmisc.lib \
        $(SLB)$/sdbtshared.lib

SHL3TARGET=$(TARGET3)$(DLLPOSTFIX)

SHL3STDLIBS= \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB) \
        $(DBTOOLSLIB) \
        $(SALLIB)

SHL3LIBS=$(LIB3TARGET)
SHL3DEF=$(MISC)$/$(SHL3TARGET).def
DEF3NAME=$(SHL3TARGET)
SHL3VERSIONMAP=$(TARGET3).map

# --- .res file ----------------------------------------------------------

RES3FILELIST=\
    $(SRS)$/sdbt_strings.srs

RESLIB3NAME=$(TARGET3)
RESLIB3IMAGES=$(PRJ)$/res
RESLIB3SRSFILES=$(RES3FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk
.IF "$(depend)"==""

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@
.ENDIF

