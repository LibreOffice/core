#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: obo $ $Date: 2004-08-12 09:32:20 $
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

PRJNAME=sc
TARGET=scalc3
#LIBTARGET=NO
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  connectivity/version.mk
.INCLUDE :  settings.mk

IENV!:=$(IENV);..$/res

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=30
.ENDIF

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/miscdlgs.srs	\
    $(SRS)$/docshell.srs	\
    $(SRS)$/ui.srs		\
    $(SRS)$/dbgui.srs	\
    $(SRS)$/drawfunc.srs \
    $(SRS)$/core.srs 	\
    $(SRS)$/styleui.srs	\
    $(SRS)$/scslots.srs	\
    $(SRS)$/formdlgs.srs \
    $(SRS)$/pagedlg.srs	\
    $(SRS)$/navipi.srs	\
    $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=sc
RESLIB1IMAGES=\
    $(PRJ)$/res					\
    $(PRJ)$/res/imglst/apptbx	\
    $(PRJ)$/res/imglst/dbgui	\
    $(PRJ)$/res/imglst/navipi	
    
    
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- StarClac DLL

SHL1TARGET= sc$(UPD)$(DLLPOSTFIX)
#SHL1VERSIONMAP= sc.map
SHL1IMPLIB= sci

.IF "$(OS)"!="MACOSX"
# static libraries
SHL1STDLIBS+= $(SCHLIB)
.ENDIF

# dynamic libraries
SHL1STDLIBS+=       \
    $(BASICLIB)	\
    $(SFXLIB)		\
    $(SO2LIB)		\
    $(SVMEMLIB)	\
    $(SVTOOLLIB)	\
    $(SVLLIB)		\
    $(SVXLIB)		\
    $(GOODIESLIB)	\
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(TKLIB)		\
    $(VOSLIB)		\
    $(SALLIB)		\
    $(TOOLSLIB)	\
    $(UNOTOOLSLIB) \
    $(SOTLIB)		\
    $(XMLOFFLIB)	\
    $(DBTOOLSLIB)	\
    $(AVMEDIALIB)	

.IF "$(OS)"=="MACOSX"
# static libraries at end for OS X
SHL1STDLIBS+= $(SCHLIB)
.ENDIF

SHL1DEPN=   $(L)$/itools.lib
#SHL1LIBS=   $(LIB3TARGET) $(LIB4TARGET)
SHL1LIBS= $(LIB1TARGET)
LIB1TARGET=$(SLB)$/scall.lib
LIB1FILES = $(LIB3TARGET) $(LIB4TARGET)

.IF "$(GUI)"!="UNX"
SHL1OBJS=   $(SLO)$/scdll.obj
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1DEPN=$(MISC)$/$(SHL1TARGET).flt
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME= scall

.IF "$(GUI)" == "WNT"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB3TARGET=$(SLB)$/scalc3.lib
#LIB3FILES=	\
#	$(SLB)$/app.lib \
#	$(SLB)$/docshell.lib \
#	$(SLB)$/view.lib \
#	$(SLB)$/undo.lib \
#	$(SLB)$/attrdlg.lib \
#	$(SLB)$/namedlg.lib \
#	$(SLB)$/styleui.lib \
#	$(SLB)$/miscdlgs.lib \
#	$(SLB)$/formdlgs.lib \
#	$(SLB)$/cctrl.lib \
#	$(SLB)$/dbgui.lib \
#	$(SLB)$/optdlg.lib \
#	$(SLB)$/pagedlg.lib \
#	$(SLB)$/drawfunc.lib \
#	$(SLB)$/navipi.lib

LIB3FILES=	\
    $(SLB)$/app.lib \
    $(SLB)$/docshell.lib \
    $(SLB)$/view.lib \
    $(SLB)$/undo.lib \
    $(SLB)$/attrdlg.lib \
    $(SLB)$/namedlg.lib \
    $(SLB)$/miscdlgs.lib \
    $(SLB)$/formdlgs.lib \
    $(SLB)$/cctrl.lib \
    $(SLB)$/dbgui.lib \
    $(SLB)$/pagedlg.lib \
    $(SLB)$/drawfunc.lib \
    $(SLB)$/navipi.lib
    
LIB3FILES+= \
            $(SLB)$/unoobj.lib


LIB4TARGET=$(SLB)$/scalc3c.lib
LIB4FILES=	\
    $(SLB)$/data.lib \
    $(SLB)$/tool.lib \
    $(SLB)$/dif.lib \
    $(SLB)$/excel.lib \
    $(SLB)$/xcl97.lib \
    $(SLB)$/html.lib \
    $(SLB)$/lotus.lib \
    $(SLB)$/rtf.lib \
    $(SLB)$/xml.lib \
    $(SLB)$/accessibility.lib \
    $(SLB)$/ftools.lib \
    $(SLB)$/scflt.lib

#LIB5TARGET=$(LB)$/sclib.lib
#LIB5ARCHIV=$(LB)$/libsclib.a

.IF "$(depend)" == ""
ALL:	\
    $(MISC)$/linkinc.ls \
    ALLTAR
.ENDIF

SHL2TARGET= scd$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= scdimp
SHL2VERSIONMAP= scd.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB)	\
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/scdetect.obj \
        $(SLO)$/detreg.obj
SHL2DEPN+=	makefile.mk


# add for scui
SHL8TARGET= scui$(UPD)$(DLLPOSTFIX)
SHL8IMPLIB= scuiimp
SHL8VERSIONMAP= scui.map
SHL8DEF=$(MISC)$/$(SHL8TARGET).def
DEF8NAME=       $(SHL8TARGET)

SHL8STDLIBS= \
        $(ISCLIB)	\
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
            $(ICUUCLIB)	\
            $(BASICLIB)
            
SHL8LIBS=   $(SLB)$/scui.lib
LIB8TARGET = $(SLB)$/scui.lib

LIB8FILES=$(SLB)$/styleui.lib	\
        $(SLB)$/optdlg.lib

LIB8OBJFILES = \
        $(SLO)$/scuiexp.obj     \
            $(SLO)$/scdlgfact.obj	\
             $(SLO)$/tpsubt.obj		\
            $(SLO)$/tptable.obj	\
             $(SLO)$/tpstat.obj	\
            $(SLO)$/tabpages.obj	\
            $(SLO)$/tpsort.obj		\
            $(SLO)$/sortdlg.obj		\
            $(SLO)$/validate.obj	\
            $(SLO)$/textdlgs.obj		\
            $(SLO)$/subtdlg.obj		\
             $(SLO)$/tphf.obj		\
            $(SLO)$/scuitphfedit.obj	\
            $(SLO)$/hfedtdlg.obj	\
            $(SLO)$/attrdlg.obj	\
             $(SLO)$/scuiimoptdlg.obj	\
            $(SLO)$/strindlg.obj		\
            $(SLO)$/shtabdlg.obj		\
             $(SLO)$/scendlg.obj		\
             $(SLO)$/pvfundlg.obj	\
            $(SLO)$/pfiltdlg.obj	\
            $(SLO)$/namepast.obj		\
            $(SLO)$/namecrea.obj		\
            $(SLO)$/mvtabdlg.obj		\
            $(SLO)$/mtrindlg.obj		\
            $(SLO)$/linkarea.obj		\
            $(SLO)$/lbseldlg.obj		\
            $(SLO)$/instbdlg.obj		\
            $(SLO)$/inscodlg.obj		\
            $(SLO)$/inscldlg.obj		\
            $(SLO)$/groupdlg.obj		\
            $(SLO)$/filldlg.obj			\
            $(SLO)$/delcodlg.obj		\
            $(SLO)$/delcldlg.obj		\
             $(SLO)$/dapitype.obj	\
            $(SLO)$/dapidata.obj	\
            $(SLO)$/crdlg.obj			\
            $(SLO)$/scuiasciiopt.obj	\
            $(SLO)$/scuiautofmt.obj
# --- Targets -------------------------------------------------------------

.IF "$(depend)" == ""

#APP1HEAP=	8192
#.IF "$(GUI)"!="WNT"
#.IF "$(GUI)"=="WIN"
# DGROUP:  DATA<0x5000  HEAP==0x2000  => STACK:0x9000 (36k)
# wenn das runtergesetz wird, muss evtl. in data\cell.cxx die
# MAXRECURSION Tiefe angepasst werden !!! (leider ein PI mal Daumen Wert)
#APP1STACK=36864
#.ELSE
# os2: ca. 2,3*win
#APP1STACK=81920
#.ENDIF
#.ENDIF
.ENDIF


.INCLUDE :  target.mk
   
    
.IF "$(depend)" == ""


.IF "$(GUI)" == "WNT"

#$(MISC)$/$(SHL1TARGET).def:  makefile.mk
#    @echo ------------------------------
#    @echo Making: $@
#    @echo LIBRARY     $(SHL1TARGET)                                  >$@
#    @echo DESCRIPTION 'SCALC3 DLL'                                 >>$@
#    @echo DATA        READ WRITE NONSHARED                          >>$@
#    @echo EXPORTS                                                   >>$@
#    @echo   component_getImplementationEnvironment @24             >>$@
#    @echo   component_writeInfo @25                                >>$@
#    @echo   component_getFactory @26                               >>$@
.ENDIF

.IF "$(OPTLINKS)" == "YES"
    echo  RC $(RCFLAGS) $(RES)$/scappi.res                    >>$@
.ENDIF

.ENDIF

#$(MISCX)$/$(SHL1TARGET).flt:
#    @echo ------------------------------
#    @echo Making: $@
#    @echo WEP>$@
#    @echo LIBMAIN>>$@
#    @echo LibMain>>$@


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @+$(TYPE) sc.flt > $@