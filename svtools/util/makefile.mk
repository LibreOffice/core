#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.45 $
#*
#*  last change: $Author: hjs $ $Date: 2003-08-18 14:55:11 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#*************************************************************************

PRJ=..

PRJNAME=svtools
TARGET=svtool
RESTARGET=svt
RESTARGETSIMPLE=svs
VERSION=$(UPD)
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

USE_LDUMP2=TRUE

# --- general section ----------------------------------------------------

LIB1TARGET= $(LB)$/svmem.lib
LIB1ARCHIV=  $(LB)$/libsvarray.a
LIB1FILES=	$(LB)$/svarray.lib

LIB2TARGET=   $(LB)$/filearch.lib
LIB2OBJFILES= $(OBJ)$/filearch.obj

.IF "$(GUI)"!="UNX"
LIB3TARGET= $(LB)$/svtool.lib
LIB3FILES=	$(LB)$/_svt.lib
LIB3OBJFILES=\
    $(OBJ)$/rtfkey2.obj
.ENDIF

.IF "$(GUI)"!="UNX"
LIB4TARGET= $(LB)$/isvl.lib
LIB4FILES=	$(LB)$/_isvl.lib
LIB4OBJFILES=\
    $(OBJ)$/htmlkey2.obj
.ENDIF

LIB7TARGET= $(SLB)$/svt.lib
LIB7FILES=	\
        $(SLB)$/misc2.lib		\
        $(SLB)$/items2.lib		\
        $(SLB)$/accessibility.lib	\
        $(SLB)$/browse.lib		\
        $(SLB)$/ctrl.lib		\
        $(SLB)$/dialogs.lib 	\
        $(SLB)$/edit.lib		\
        $(SLB)$/unoiface.lib	\
        $(SLB)$/filter.lib		\
        $(SLB)$/igif.lib		\
        $(SLB)$/ipng.lib		\
        $(SLB)$/jpeg.lib		\
        $(SLB)$/ixpm.lib		\
        $(SLB)$/ixbm.lib		\
        $(SLB)$/sbx.lib 		\
        $(SLB)$/numbers.lib 	\
        $(SLB)$/wmf.lib 		\
        $(SLB)$/undo.lib		\
        $(SLB)$/urlobj.lib		\
        $(SLB)$/plugapp.lib 	\
        $(SLB)$/svcontnr.lib	\
        $(SLB)$/syslocale.lib   \
        $(SLB)$/svhtml2.lib     \
        $(SLB)$/filepicker.lib  \
        $(SLB)$/heavyconfig.lib

.IF "$(GUI)" == "UNX"
LIB7FILES+= 	$(SLB)$/eaimp.lib
.ENDIF

LIB7FILES+= \
            $(SLB)$/svdde.lib

LIB8TARGET= $(SLB)$/svl.lib
LIB8OBJFILES= \
            $(SLO)$/svtdata.obj \
            $(SLO)$/registerlight.obj

LIB8FILES=	\
        $(SLB)$/config.lib	\
        $(SLB)$/filerec.lib \
        $(SLB)$/items1.lib	\
        $(SLB)$/misc1.lib	\
        $(SLB)$/notify.lib	\
        $(SLB)$/passwordcontainer.lib	\
        $(SLB)$/svarray.lib \
        $(SLB)$/svhtml1.lib \
        $(SLB)$/svrtf.lib	\
        $(SLB)$/svsql.lib	\
        $(SLB)$/cmdparse.lib

# generation of resourcen-lib ----------------------------------------

RESLIB1NAME=	$(RESTARGET)
RESLIB1SRSFILES=$(SRS)$/filter.srs  \
        $(SRS)$/misc.srs        \
        $(SRS)$/sbx.srs         \
        $(SRS)$/ctrl.srs        \
        $(SRS)$/items.srs       \
        $(SRS)$/dialogs.srs     \
        $(SRS)$/plugapp.srs     \
        $(SRS)$/svcontnr.srs    \
        $(SRS)$/filepicker.srs  \
        $(SRS)$/uno.srs         \
        $(SRS)$/browse.srs		\
        $(SRS)$/accessibility.srs

RESLIB2NAME=	$(RESTARGETSIMPLE)
RESLIB2SRSFILES=\
    $(SRS)$/mediatyp.srs

# build the shared library  --------------------------------------------------

SHL1TARGET= svt$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= _svt

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
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB)			\
        $(ICUUCLIB)

.IF "$(OS)"=="MACOSX"
# static libraries go at end
SHL1STDLIBS+= $(JPEG3RDLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        uwinapi.lib \
        $(LIBPRE) advapi32.lib	\
        $(LIBPRE) gdi32.lib
.ENDIF # WNT

SHL1LIBS= \
        $(SLB)$/svt.lib 	\
        $(LB)$/svmem.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1DEPN=$(SHL2TARGETN)

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =svt
DEF1DES 	=SvTools
DEF1EXPORTFILE=	svt.dxp


# --- svtools lite --------------------------------------------------

SHL2TARGET= svl$(VERSION)$(DLLPOSTFIX)
SHL2IMPLIB= _isvl
SHL1OBJS=$(SLO)$/svtdata.obj

SHL2STDLIBS= \
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB) 		\
        $(UCBHELPERLIB)		\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(VOSLIB)			\
        $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= \
        uwinapi.lib \
        $(LIBPRE) advapi32.lib	\
        $(LIBPRE) gdi32.lib
.ENDIF # WNT

SHL2LIBS=	$(SLB)$/svl.lib

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt $(SLB)$/svl.lib
DEFLIB2NAME=svl
DEF2DES =SvTools lite

# --- bmpgui application --------------------------------------------------

APP1TARGET	=	bmpgui
APP1BASE	=	0x10000000
APP1DEPN	=   $(SHL1TARGETN) $(SHL2TARGETN)
APP1OBJS	=   $(OBJ)$/bmpgui.obj	\
                $(OBJ)$/bmpcore.obj

.IF "$(GUI)"!="UNX"
APP1STDLIBS+= svtool.lib
.ELSE
APP1STDLIBS+= -lsvt$(UPD)$(DLLSUFFIX)
APP1STDLIBS+= -lsvl$(UPD)$(DLLSUFFIX)
.ENDIF # UNX

APP1STDLIBS+=	\
                $(VCLLIB)		\
                $(TOOLSLIB)		\
                $(VOSLIB) 		\
                $(SALLIB)

# --- g2g application --------------------------------------------------

APP2TARGET	=	g2g
APP2BASE	=	0x10000000
APP2DEPN	=   $(SHL1TARGETN) $(SHL2TARGETN)

APP2OBJS	=   $(OBJ)$/g2g.obj

.IF "$(GUI)"!="UNX"
APP2STDLIBS+= svtool.lib
.ELSE
APP2STDLIBS+= -lsvt$(UPD)$(DLLSUFFIX)
APP2STDLIBS+= -lsvl$(UPD)$(DLLSUFFIX)
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
    +$(TYPE) svt.flt >$@

$(MISC)$/$(SHL2TARGET).flt: svl.flt
    @echo ------------------------------
    @echo Making: $@
    +$(TYPE) svl.flt >$@

