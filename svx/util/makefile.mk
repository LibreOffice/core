#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.34 $
#
#   last change: $Author: hr $ $Date: 2004-08-06 12:03:21 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=svx
TARGET=svx
#svx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=30
.ENDIF
RSCLOCINC+=-I$(PRJ)$/source$/svdraw

SHL4TARGET= cui$(UPD)$(DLLPOSTFIX)
SHL4VERSIONMAP= cui.map
SHL4IMPLIB=icui
DEF4NAME= $(SHL4TARGET)
SHL4DEF=$(MISC)$/$(SHL4TARGET).def
SHL4LIBS=   $(SLB)$/cui.lib
SHL4OBJS= \
        $(SLO)$/cuiexp.obj     \
        $(SLO)$/dlgfact.obj

SHL4STDLIBS= \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(GOODIESLIB) \
            $(SO2LIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB)	\
            $(CPPULIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(JVMFWKLIB) \
            $(ICUUCLIB)

# --- Svx - DLL ----------

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= svx$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= svx
SVXLOKAL=	$(LB)$/svx.lib
SHL1BASE  = 0x1d800000

SHL1STDLIBS= \
            $(SFX2LIB) \
            $(XMLOFFLIB) \
            $(GOODIESLIB) \
            $(BASEGFXLIB) \
            $(BASICLIB) \
            $(SO2LIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB)	\
            $(CPPULIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(ICUUCLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
            $(SHELLLIB)
.ENDIF # WNT

.IF "$(BIG_SVX)"==""
SHL1STDLIBS+=\
            $(LB)$/dl.lib
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1DEPN=       $(SLB)$/svx.lib $(LB)$/dl.lib
.ENDIF # WNT

SHL1LIBS=       $(SLB)$/svx.lib

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME=svx
DEF1DES		= Rtf, Edt, Outliner, SvDraw, Form, Fmcomp, Engine3D, MSFilter
# THB: exports list svx checked for 6.0 Final 6.12.2001
DEF1EXPORTFILE	= svx.dxp

.IF "$(BIG_SVX)"==""

SHL2TARGET= dl$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= dl
SVXLOKAL+=	$(LB)$/dl.lib
SHL2BASE  = 0x1db00000
SHL2STDLIBS= \
            $(SVXLIB) \
            $(SALLIB) \
            $(VOSLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(SVLIB) \
            $(SO2LIB) \
            $(SOTLIB) \
            $(XMLOFFLIB) \
            $(SFX2LIB) \
            $(GOODIESLIB) \
            $(BASEGFXLIB) \
            $(BASICLIB) \
            $(SVMEMLIB) \
            $(TKLIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(UCBHELPERLIB) \
            $(COMPHELPERLIB)

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+=\
            $(SHELLLIB)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL2STDLIBS+=\
        $(SJLIB)
.ENDIF


SHL2DEPN=       $(SLB)$/dl.lib $(LB)$/svx.lib
SHL2LIBS=       $(SLB)$/dl.lib
SHL2OBJS+=      $(SLO)$/svxempty.obj

SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
DEF2NAME        =$(SHL2TARGET)
DEF2DEPN        =$(MISC)$/$(SHL2TARGET).flt
DEFLIB2NAME=dl
DEF2DES     =SvDraw, Form, Fmcomp, Engine3D, XOutDev, MSFilter

.ENDIF

LIBEXTRAFILES=\
        $(LIBPRE) $(SLB)$/properties.lib \
        $(LIBPRE) $(SLB)$/contact.lib \
        $(LIBPRE) $(SLB)$/mixer.lib \
        $(LIBPRE) $(SLB)$/event.lib \
        $(LIBPRE) $(SLB)$/animation.lib \
        $(LIBPRE) $(SLB)$/svdraw.lib \
        $(LIBPRE) $(SLB)$/form.lib \
        $(LIBPRE) $(SLB)$/fmcomp.lib \
        $(LIBPRE) $(SLB)$/engine3d.lib \
        $(LIBPRE) $(SLB)$/msfilter.lib \
        $(LIBPRE) $(SLB)$/xout.lib \
        $(LIBPRE) $(SLB)$/xml.lib

LIB1TARGET      =$(SLB)$/svx.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/items.lib     \
            $(LIBPRE) $(SLB)$/svxlink.lib   \
            $(LIBPRE) $(SLB)$/svxrtf.lib    \
            $(LIBPRE) $(SLB)$/editeng.lib   \
            $(LIBPRE) $(SLB)$/outliner.lib \
            $(LIBPRE) $(SLB)$/dialogs.lib\
            $(LIBPRE) $(SLB)$/mnuctrls.lib  \
            $(LIBPRE) $(SLB)$/options.lib   \
            $(LIBPRE) $(SLB)$/stbctrls.lib  \
            $(LIBPRE) $(SLB)$/tbxctrls.lib  \
            $(LIBPRE) $(SLB)$/unoedit.lib   \
            $(LIBPRE) $(SLB)$/unodraw.lib	\
            $(LIBPRE) $(SLB)$/gal.lib		\
            $(LIBPRE) $(SLB)$/accessibility.lib	\
            $(LIBPRE) $(SLB)$/customshapes.lib\
            $(LIBPRE) $(SLB)$/toolbars.lib

.IF "$(BIG_SVX)"==""
LIB2TARGET      =$(SLB)$/dl.lib
LIB2FILES       = $(LIBEXTRAFILES)
.ELSE
LIB1FILES+=$(LIBEXTRAFILES)
.ENDIF

.IF "$(GUI)" == "OS2" || "(GUIBASE)" == "WIN"
LIB1FILES  += \
            $(LIBPRE) $(SLB)$/ibrwimp.lib
.ENDIF


SRSFILELIST=\
                $(SRS)$/svdstr.srs      \
                $(SRS)$/editeng.srs     \
                $(SRS)$/outliner.srs \
                $(SRS)$/dialogs.srs     \
                $(SRS)$/drawdlgs.srs \
                $(SRS)$/mnuctrls.srs \
                $(SRS)$/stbctrls.srs \
                $(SRS)$/tbxctrls.srs \
                $(SRS)$/options.srs     \
                $(SRS)$/svxitems.srs \
                $(SRS)$/form.srs \
                $(SRS)$/fmcomp.srs \
                $(SRS)$/engine3d.srs \
                $(SRS)$/unodraw.srs \
                $(SRS)$/svxlink.srs \
                $(SRS)$/accessibility.srs \
                $(SRS)$/toolbars.srs \
                $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=svx
RESLIB1IMAGES=$(PRJ)$/res $(PRJ)$/source/svdraw
RESLIB1SRSFILES= $(SRSFILELIST)

.IF "$(depend)" != ""

ALL:
    @echo nothing to depend on

.ELSE
.IF "$(GUI)"=="WNT"


.IF "$(BIG_SVX)"==""
ALL:      \
        $(MAKELANGDIR)  \
            $(SLB)$/dl.lib  $(SLB)$/svx.lib \
            $(LB)$/dl.lib   $(LB)$/svx.lib \
          $(MISC)$/linkinc.ls                   \
          ALLTAR
.ELSE
ALL:      \
        $(MAKELANGDIR)  \
            $(SLB)$/svx.lib \
            $(LB)$/svx.lib \
          $(MISC)$/linkinc.ls                   \
          ALLTAR
.ENDIF

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
ALL: \
        $(MAKELANGDIR)  \
        ALLTAR
.ENDIF
.ENDIF			#F "$(depend)" != ""

# --- Targets -------------------------------------------------------


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    +$(TYPE) svx.flt >$@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    +$(TYPE) dl.flt >$@

$(MISC)$/$(SHL4TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    +$(TYPE) cui.flt >$@

.INCLUDE :  target.mk


implib1: $(MISC)\svx1.def
    implib /noi $(LB)\svx1.lib $(MISC)\svx1.def

implib2: $(MISC)\svx2.def
    implib /noi $(LB)\svx2.lib $(MISC)\svx2.def

implib3: $(MISC)\dl1.def
    implib /noi $(LB)\dl1.lib $(MISC)\dl1.def

implib4: $(MISC)\dl2.def
    implib /noi $(LB)\dl2.lib $(MISC)\dl2.def

implib_defs: $(SHL1DEF) $(SHL2DEF)
    +-$(RM) $(MISC)$/svx1.def
    +-$(RM) $(MISC)$/svx2.def
    +-$(RM) $(MISC)$/dl1.def
    +-$(RM) $(MISC)$/dl2.def
    splitdef $(SHL1DEF) $(MISC)$/svx1.def $(MISC)$/svx2.def
    splitdef $(SHL2DEF) $(MISC)$/dl1.def $(MISC)$/dl2.def
