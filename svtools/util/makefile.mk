#*************************************************************************
#*
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.67 $
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
RESTARGETSIMPLE=svs
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

USE_LDUMP2=TRUE

# --- general section ----------------------------------------------------

.IF "$(GUI)"!="UNX"
LIB3TARGET= $(LB)$/svtool.lib
LIB3FILES=	$(LB)$/_svt.lib
.ENDIF

.IF "$(GUI)"!="UNX"
LIB4TARGET= $(LB)$/isvl.lib
LIB4FILES=	$(LB)$/_isvl.lib
.ENDIF

LIB7TARGET= $(SLB)$/svt.lib
LIB7FILES=	\
        $(SLB)$/misc.lib		\
        $(SLB)$/items.lib		\
        $(SLB)$/browse.lib		\
        $(SLB)$/ctrl.lib		\
        $(SLB)$/dialogs.lib 	\
        $(SLB)$/edit.lib		\
        $(SLB)$/unoiface.lib	\
        $(SLB)$/filter.lib		\
        $(SLB)$/igif.lib		\
        $(SLB)$/jpeg.lib		\
        $(SLB)$/ixpm.lib		\
        $(SLB)$/ixbm.lib		\
        $(SLB)$/numbers.lib 	\
        $(SLB)$/numbers.uno.lib 	\
        $(SLB)$/wmf.lib 		\
        $(SLB)$/undo.lib		\
        $(SLB)$/urlobj.lib		\
        $(SLB)$/plugapp.lib 	\
        $(SLB)$/svcontnr.lib	\
        $(SLB)$/syslocale.lib   \
        $(SLB)$/svdde.lib \
        $(SLB)$/svhtml.lib     \
        $(SLB)$/svrtf.lib	\
        $(SLB)$/heavyconfig.lib 	\
        $(SLB)$/table.lib 	\
        $(SLB)$/java.lib	

LIB8TARGET= $(SLB)$/svl.lib
LIB8FILES=	\
        $(SLB)$/config.lib	\
        $(SLB)$/filerec.lib \
        $(SLB)$/filepicker.lib \
        $(SLB)$/items1.lib	\
        $(SLB)$/misc1.lib	\
        $(SLB)$/notify.lib	\
        $(SLB)$/svarray.lib \
        $(SLB)$/svsql.lib

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
        $(SRS)$/uno.srs         \
        $(SRS)$/browse.srs		\
        $(SRS)$/javaerror.srs

RESLIB2NAME=	$(RESTARGETSIMPLE)
RESLIB2SRSFILES=\
        $(SRS)$/items1.srs \
        $(SRS)$/misc1.srs


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

SHL1OBJS= \
    $(SLO)$/svtdata.obj

SHL1LIBS= \
    $(SLB)$/svt.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1DEPN=$(SHL2TARGETN)

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =svt
DEF1DES 	=SvTools


# --- svtools lite --------------------------------------------------

SHL2TARGET= svl$(DLLPOSTFIX)
SHL2IMPLIB= _isvl
SHL2USE_EXPORTS=name
#Do not link with VCL or any other library that links with VCL
SHL2STDLIBS= \
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB) 		\
        $(I18NISOLANGLIB)   \
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(VCLLIB)			\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= \
        $(UWINAPILIB) \
        $(ADVAPI32LIB)	\
        $(GDI32LIB)
.ENDIF # WNT

SHL2LIBS=	$(SLB)$/svl.lib

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt $(SLB)$/svl.lib
DEFLIB2NAME=svl
DEF2DES =SvTools lite

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
SVTTARGETS= $(LB)$/lib$(SHL2TARGET)$(DLLPOST) $(LB)$/lib$(SHL1TARGET)$(DLLPOST)
.ELSE
SVTTARGETS= $(LB)$/isvl.lib \
            $(BIN)$/$(SHL2TARGET)$(DLLPOST) $(BIN)$/$(SHL1TARGET)$(DLLPOST)
.ENDIF

# just a quick fix - has to be cleaned up some day...
.IF "$(L10N-framework)"==""
ALL: $(SLB)$/svl.lib \
    $(SLB)$/svt.lib \
    $(MISC)$/$(SHL2TARGET).flt \
    $(MISC)$/$(SHL1TARGET).flt \
    $(MISC)$/$(SHL2TARGET).def \
    $(MISC)$/$(SHL1TARGET).def \
    $(SVTTARGETS) \
    ALLTAR
.ENDIF          # "$(L10N-framework)"==""

.INCLUDE :	target.mk

# --- Svtools-Control-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: svt.flt
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) svt.flt >$@

$(MISC)$/$(SHL2TARGET).flt: svl.flt
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) svl.flt >$@

