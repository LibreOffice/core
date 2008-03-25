#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.35 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:14:39 $
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

LIB1TARGET=$(SLB)$/dbaccess.lib
LIB1FILES=\
        $(SLB)$/api.lib	\
        $(SLB)$/dataaccess.lib	\
        $(SLB)$/misc.lib	\
        $(SLB)$/core_resource.lib	\
        $(SLB)$/dbashared.lib	

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
        $(VOSLIB) \
        $(SALLIB) \
        $(SFXLIB) \
        $(BASICLIB) \
        $(SALHELPERLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(TARGET).map

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
        $(SVXLIB)				\
        $(SFXLIB)				\
        $(BASICLIB)				\
        $(SVTOOLLIB)			\
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

SHL2DEPN=
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
.ENDIF


