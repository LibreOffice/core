#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 17:15:01 $
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

.IF "$(OS)"=="SOLARIS"
.IF "$(CPU)"=="S"
LIB1OBJFILES += $(OUT)$/obj$/autorec.obj
.ENDIF
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
    $(VOSLIB) \
    $(TKLIB) \
    $(SDLIB) \
    $(SCLIB) \
    $(UCBHELPERLIB) \
    $(XMLOFFLIB) \
    $(BASCTLLIB)


.IF "$(GUI)" ==	"UNX"
SHL2STDLIBS += \
    $(SIMLIB) \
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


SHL2DEF=    $(MISC)$/$(SHL2TARGET).def
SHL2BASE=	0x1e000000


# rem ===== hier wird die Mega Dll gebaut ==========================
.IF "$(make_xl)" != ""

.IF "$(GUI)"=="WNT"
LINKFLAGS=$(LINKFLAGS) /FORCE:MULTIPLE /ENTRY:DLLEntryPoint@12
.ENDIF

.IF "$(GUI)"=="OS2"
LIBFLAGS=$(LIBFLAGS) /P4096
.ENDIF

LIB5TARGET= $(SLB)$/xl.lib
LIB5FILES=	$(SLB)$/$(TARGET).lib		\
            $(SOLARLIBDIR)$/xtools.lib	\
            $(SOLARLIBDIR)$/xsv.lib		\
            $(SOLARLIBDIR)$/xsvtool.lib	\
            $(SOLARLIBDIR)$/xsb.lib		\
            $(SOLARLIBDIR)$/xso2.lib		\
            $(SOLARLIBDIR)$/xgo.lib		\
            $(SOLARLIBDIR)$/xsj.lib		\
            $(SOLARLIBDIR)$/xsfx.lib		\
            $(SOLARLIBDIR)$/xdg.lib		\
            $(SOLARLIBDIR)$/xsvx.lib

LIB6TARGET=	$(LB)$/xapp.lib
LIB6FILES=	$(SOLARLIBDIR)$/xsfxapp.obj	\
            $(SOLARLIBDIR)$/xsvapp.obj	\
            $(SOLARLIBDIR)$/xsvmain.obj	\
            $(SOLARLIBDIR)$/xword2.obj	\
            $(SOLARLIBDIR)$/xplugapp.obj	\
            $(OBJ)$/appctor.obj

SHL3TARGET= xl$(UPD)$(DLLPOSTFIX)
SHL3IMPLIB= _xl
SHL3LIBS=   $(SLB)$/xl.lib

.IF "$(GUI)"=="WNT"
SHL3STDLIBS=svmem.lib\
            comdlg32.lib advapi32.lib shell32.lib gdi32.lib \
            ole32.lib uuid.lib oleaut32.lib comctl32.lib winspool.lib
.ELSE
SHL3STDLIBS=svmem.lib\
            $(L)$/mail.lib
.ENDIF

SHL3STDLIBS+= \
            go.lib \
            docmgr.lib sj.lib thread.lib inetdll.lib ipc.lib

SHL3DEPN=   \
    $(SLB)$/core1.lib\
    $(SLB)$/core2.lib\
    $(SLB)$/filter.lib\
    $(SLB)$/ui1.lib\
    $(SLB)$/ui2.lib

.IF "$(GUI)"!="WNT"
SHL3DEPN+=   \
    $(L)$/mail.lib
.ENDIF

SHL3OBJS=\
            $(SOLARLIBDIR)$/xsvdll.obj $(SLO)$/app.obj

SHL3RES=    $(SOLARRESDIR)$/svsrc.res
SHL3DEF=    $(MISC)$/$(SHL3TARGET).def
SHL3BASE=	0x1c000000

DEF3NAME=   $(SHL3TARGET)
DEF3DEPN=   $(MISC)$/$(SHL3TARGET).flt
DEFLIB3NAME =xl
DEF3DES     =offmgr app-interface

.ENDIF
# rem ===== hier wird die Mega Dll gebaut  (ENDE) ===================

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
# Windows
# ------------------------------------------------------------------




# ------------------------------------------------------------------
# OS/2
# ------------------------------------------------------------------


.IF "$(GUI)" == "OS2"

$(MISC)$/$(SHL2TARGET).def:  makefile.mk
    @echo ================================================================
    @echo building $@
    @echo ----------------------------------------------------------------
.IF "$(COM)"!="WTC"
    echo  LIBRARY		INITINSTANCE TERMINSTANCE			>$@
    echo  DESCRIPTION   'SwDLL'                            >>$@
    echo  PROTMODE										   >>$@
        @echo CODE        LOADONCALL 			              >>$@
    @echo DATA		  PRELOAD MULTIPLE NONSHARED					  >>$@
        @echo EXPORTS                                              >>$@
.IF "$(COM)"!="ICC"
    @echo _CreateSdDrawDocShellDll @2                              >>$@
    @echo _CreateSdGraphicDocShellDll @3                           >>$@
    @echo _CreateObjSdDrawDocShellDll @4                           >>$@
    @echo _CreateObjSdGraphicDocShellDll @5                        >>$@
    @echo _InitSdDll @6                                            >>$@
    @echo _DeInitSdDll @7                                          >>$@
.ELSE
    @echo   CreateSwDocShellDll @2 	                           >>$@
    @echo   CreateSwWebDocShellDll @3                            >>$@
    @echo   CreateSwGlobalDocShellDll @4                            >>$@
    @echo   CreateObjSwDocShellDll @5                         >>$@
    @echo   CreateObjSwWebDocShellDll @6                         >>$@
    @echo   CreateObjSwGlobalDocShellDll @7                         >>$@
    @echo   InitSwDll @8                                          >>$@
    @echo   DeInitSwDll @9                                        >>$@
.ENDIF
.ELSE
        @echo option DESCRIPTION 'SwDLL'                            >$@
        @echo name $(BIN)$/$(SHL2TARGET).dll                         >>$@
    @echo CreateSwDocShellDll_ @2      >>temp.def
    @echo CreateSwGlobalDocShellDll_ @2      >>temp.def
    @echo CreateSwWebDocShellDll_ @3   >>temp.def
    @echo CreateObjSwDocShellDll_ @4   >>temp.def
    @echo CreateObjSwGlobalDocShellDll_ @4   >>temp.def
    @echo CreateObjSwWebDocShellDll_ @5   >>temp.def
    @echo InitSwDll_ @6                    >>temp.def
    @echo DeInitSwDll_ @7                  >>temp.def
    @gawk -f s:\util\exp.awk temp.def				>>$@
    del temp.def
.ENDIF
.ENDIF

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

.ENDIF

.IF "$(GUI)" == "MAC"

$(MISC)$/$(SHL2TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo   CreateSwDocShellDll                             > $@
    @echo   CreateSwWebDocShellDll                            >> $@
    @echo   CreateSwGlobalDocShellDll                             >> $@
    @echo   CreateObjSwDocShellDll                          >> $@
    @echo   CreateObjSwWebDocShellDll                          >> $@
    @echo   CreateObjSwGlobalDocShellDll                        >> $@
    @echo   InitSwDll                                         >> $@
    @echo   DeInitSwDll                                       >> $@


.ENDIF

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)" =="WNT"
    @+echo
    @+echo 	NO HIDS!
    @+echo
    +copy ..\inc\helpid.h .
    +mhids hidother.src ..$/$(INPATH)$/srs sw hidother
    +del helpid.h
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

