#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: vg $ $Date: 2001-02-27 18:19:43 $
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

PRJNAME=sw
TARGET=sw
GEN_HID=TRUE
.IF "$(CPU)"=="i386"
USE_LDUMP2=TRUE
.ENDIF


# --- Settings ------------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

#	nmake		 	-	swdll

DESK=T

.IF "$(prjpch)" != ""
CDEFS=$(CDEFS) -DPRECOMPILED
.ENDIF

.IF "$(GUI)" == "WIN"
RESLIBSPLIT1NAME=sw
.ELSE
RESLIB1NAME=sw
.ENDIF
MYRESLIBNAME=sw

#RSCLOCINC=$(RSCLOCINC);$(PRJ)$/RES

# --- Allgemein -----------------------------------------------------------
.IF "$(GUI)"=="WIN"
LIBFLAGS=/PAGE:128 /NOE /NOI
#OPTLINKS=YES
MAPSYM=tmapsym
.IF "$(debug)" != ""
LINKFLAGS= /F /PACKCODE:65520 /PACKDATA /NOD /NOE /MAP /COD /NOCV
.ELSE
LINKFLAGS= /F /PACKCODE:65520 /PACKDATA /NOD /NOE /MAP
.ENDIF
.ENDIF

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:1024 /PACKD:32768
.ENDIF

.IF "$(header)" == ""

sw_res_files= \
    $(SRS)$/app.srs          \
    $(SRS)$/dialog.srs       \
    $(SRS)$/chrdlg.srs       \
    $(SRS)$/config.srs       \
    $(SRS)$/dbui.srs	    \
    $(SRS)$/dochdl.srs       \
    $(SRS)$/docvw.srs        \
    $(SRS)$/envelp.srs       \
    $(SRS)$/fldui.srs        \
    $(SRS)$/fmtui.srs        \
    $(SRS)$/frmdlg.srs       \
    $(SRS)$/globdoc.srs      \
    $(SRS)$/index.srs        \
    $(SRS)$/lingu.srs        \
    $(SRS)$/misc.srs         \
    $(SRS)$/ribbar.srs       \
    $(SRS)$/shells.srs       \
    $(SRS)$/swslots.srs     \
    $(SRS)$/table.srs        \
    $(SRS)$/uiview.srs       \
    $(SRS)$/utlui.srs        \
    $(SRS)$/web.srs          \
    $(SRS)$/wizard.srs       \
    $(SRS)$/wrtsh.srs        \
    $(SOLARRESDIR)$/sfx.srs

.IF "$(GUI)" == "WIN"
RESLIBSPLIT1SRSFILES= \
    $(sw_res_files)
.ELSE
RESLIB1SRSFILES= \
    $(sw_res_files)
.ENDIF

LIB1TARGET=$(LB)$/swlib.lib
LIB1ARCHIV=$(LB)$/libswlib.a
LIB1OBJFILES= \
        $(OUT)$/obj$/swlib.obj \
        $(OUT)$/obj$/swcomlib.obj \
        $(OUT)$/obj$/w4wflt.obj

.IF "$(OS)$(CPU)"=="SOLARISS"
LIB1OBJFILES += $(SOLARLIBDIR)$/autorec.o
.ENDIF



SHL2TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL2VERSIONMAP= $(TARGET).map
SHL2IMPLIB= _$(TARGET)
SHL2LIBS= \
    $(SLB)$/core1.lib\
    $(SLB)$/core2.lib\
    $(SLB)$/filter.lib\
    $(SLB)$/ui1.lib\
    $(SLB)$/ui2.lib


SHL2STDLIBS= \
    $(SALLIB) \
    $(SFXLIB) \
    $(OFALIB) \
    $(SFXDEBUGLIB) \
    $(BASICLIB) \
    $(SVXLIB) \
    $(GOODIESLIB) \
    $(CHANNELLIB) \
    $(INETLIBSH) \
    $(SVMEMLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(SVLLIB)	\
    $(SVLIB) \
    $(SOTLIB) \
    $(TOOLSLIB) \
    $(UNOLIB) \
    $(ONELIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(DBTOOLSLIB) \
    $(VOSLIB) \
    $(TKLIB) \
    $(SDLIB) \
    $(SCLIB) \
    $(UCBHELPERLIB) \
    $(XMLOFFLIB) \
    $(BASCTLLIB) \
    $(COMPHELPERLIB)


.IF "$(GUI)" ==	"UNX"
SHL2STDLIBS += \
    $(SCHLIB) \
    $(SMLIB)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL2STDLIBS+= \
        $(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= \
            advapi32.lib
.ELSE
#SHL2STDLIBS+= \
#			$(MAILLIB)
.ENDIF

#			uno.lib usr.lib sj.lib aofa.lib
#			ysch.lib  ysim.lib ysm.lib basic.lib ich.lib

SHL2DEPN=   \
    $(SLB)$/core1.lib\
    $(SLB)$/core2.lib\
    $(SLB)$/filter.lib\
    $(SLB)$/ui1.lib\
    $(SLB)$/ui2.lib


SHL2OBJS= \
    $(OUT)$/slo$/swmodule.obj \
    $(OUT)$/slo$/swdll.obj 
#	$(SLO)$/.obj		  ^ \ nicht vergessen!

.IF "$(OS)"!="LINUX"
SHL2OBJS+= \
            $(SLO)$/atrfrm.obj      \
            $(SLO)$/fmtatr2.obj
.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISS"
SHL2OBJS +=  $(SOLARLIBDIR)$/autorec.o
.ENDIF

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def
SHL2BASE=	0x1e000000


.IF "$(GUI)"=="WNT"
do_build+= \
    $(MISC)$/linkinc.ls
.ENDIF

do_build+= \
    $(SHL2TARGETN)

.IF "$(depend)"==""
ALL:\
    $(do_build) \
    $(SRS)$/hidother.hid\
    $(INC)$/sw.lst	\
    ALLTAR
.ENDIF
.ENDIF

.INCLUDE :  target.mk

$(MISCX)$/$(SHL2TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@

# ------------------------------------------------------------------
# Windows NT
# ------------------------------------------------------------------

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL2TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL2TARGET)                                  >$@
    @echo DESCRIPTION 'SWriter4 DLL'                                 >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
    @echo EXPORTS                                                   >>$@
    @echo   CreateSwDocShellDll @20                            >>$@
    @echo   CreateSwWebDocShellDll @30                            >>$@
    @echo   CreateSwGlobalDocShellDll @40                            >>$@
    @echo   CreateObjSwDocShellDll @21                         >>$@
    @echo   CreateObjSwWebDocShellDll @22                         >>$@
    @echo   CreateObjSwGlobalDocShellDll @23                         >>$@
    @echo   InitSwDll @24                                          >>$@
    @echo   DeInitSwDll @25                                        >>$@
    @echo   component_getImplementationEnvironment @50				>>$@
    @echo   component_writeInfo @51									>>$@
    @echo   component_getFactory @52								>>$@

.ENDIF

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)" =="WNT"
.IF "$(BUILD_SOSL)"==""
    @+echo
    @+echo 	NO HIDS!
    @+echo
    +copy ..\inc\helpid.h .
    +mhids hidother.src ..$/$(INPATH)$/srs sw hidother
    +del helpid.h
.ENDIF
.ELSE
    @echo wnt only
.ENDIF

$(INC)$/sw.lst:
.IF "$(GUI)" =="WNT"
    +-@echo clook missed!!!!
#clook -o $@ -p 1 -i ..\inc;..\source\ui\inc;..\source\core\inc;..\source\filter\inc;. dummy.cxx
.ELSE
    @echo wnt only
.ENDIF

