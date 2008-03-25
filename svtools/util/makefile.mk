#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.65 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:19:34 $
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
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(I18NISOLANGLIB)   \
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

# --- bmpgui application --------------------------------------------------

APP1TARGET	=	bmpgui
APP1BASE	=	0x10000000
APP1DEPN	=   $(SHL1TARGETN) $(SHL2TARGETN)
APP1OBJS	=   $(OBJ)$/bmpgui.obj	\
                $(OBJ)$/bmpcore.obj

.IF "$(GUI)"!="UNX"
APP1STDLIBS+= $(SVTOOLLIB)
.ELSE
APP1STDLIBS+= -lsvt$(DLLPOSTFIX)
APP1STDLIBS+= -lsvl$(DLLPOSTFIX)
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

