#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.38 $
#
#   last change: $Author: obo $ $Date: 2004-08-12 10:19:45 $
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
GEN_HID_OTHER=TRUE
LIBTARGET=NO

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein -----------------------------------------------------------

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=40
.ENDIF

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
    $(SRS)$/undo.srs        \
    $(SRS)$/utlui.srs        \
    $(SRS)$/web.srs          \
    $(SRS)$/wizard.srs       \
    $(SRS)$/wrtsh.srs        \
    $(SOLARCOMMONRESDIR)$/sfx.srs

RESLIB1NAME=sw
RESLIB1IMAGES=$(PRJ)$/imglst $(PRJ)$/res
RESLIB1SRSFILES= \
    $(sw_res_files)

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
#SHL1VERSIONMAP= $(TARGET).map
SHL1IMPLIB= _$(TARGET)
SHL1LIBS= $(SLB)$/swall.lib

LIB1TARGET      =$(SLB)$/swall.lib

LIB1OBJFILES= $(OUT)$/slo$/swmodule.obj \
    $(OUT)$/slo$/swdll.obj

LIB1FILES       = \
                $(LIBPRE) $(SLB)$/core1.lib	\
        $(LIBPRE) $(SLB)$/core2.lib	\
        $(LIBPRE) $(SLB)$/filter.lib	\
        $(LIBPRE) $(SLB)$/ui1.lib	\
        $(LIBPRE) $(SLB)$/ui2.lib
        
#SHL1LIBS= \
#	$(SLB)$/core1.lib\
#	$(SLB)$/core2.lib\
#	$(SLB)$/filter.lib\
#	$(SLB)$/ui1.lib\
#	$(SLB)$/ui2.lib

.IF "$(OS)"!="MACOSX"
# static libraries
SHL1STDLIBS+= $(SCHLIB)
.ENDIF

# dynamic libraries
SHL1STDLIBS+= \
    $(SVXLIB) \
    $(SFXLIB) \
    $(XMLOFFLIB) \
    $(BASICLIB) \
    $(GOODIESLIB) \
    $(SO2LIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB)	\
    $(SOTLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(VOSLIB) \
    $(SALLIB) \
        $(ICUUCLIB) \
    $(I18NUTILLIB)	\
    $(AVMEDIALIB)	

.IF "$(OS)"=="MACOSX"
# static libraries at end for OS X
SHL1STDLIBS+= $(SCHLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= advapi32.lib
.ENDIF # WNT

SHL1DEPN=   \
    $(SLB)$/core1.lib\
    $(SLB)$/core2.lib\
    $(SLB)$/filter.lib\
    $(SLB)$/ui1.lib\
    $(SLB)$/ui2.lib 


#SHL1OBJS= \
#	$(OUT)$/slo$/swmodule.obj \
#	$(OUT)$/slo$/swdll.obj
#	$(SLO)$/.obj		  ^ \ nicht vergessen!


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1BASE=	0x1e000000
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME=swall

SHL2TARGET= swd$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= swdimp
SHL2VERSIONMAP= swd.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=       $(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(TOOLSLIB) \
        $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2OBJS=   $(SLO)$/swdetect.obj \
        $(SLO)$/swdet2.obj \
        $(SLO)$/detreg.obj  \
            $(OUT)$/obj$/w4wflt.obj

.IF "$(product)"==""
SHL2OBJS+=  \
        $(SLO)$/errhdl.obj
.ENDIF

SHL2DEPN+=  makefile.mk

# add for swui
SHL3TARGET= swui$(UPD)$(DLLPOSTFIX)
SHL3IMPLIB= swuiimp
SHL3VERSIONMAP= swui.map
SHL3DEF=$(MISC)$/$(SHL3TARGET).def
DEF3NAME=       $(SHL3TARGET)

SHL3STDLIBS= \
        $(ISWLIB) \
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
            $(BASICLIB)	\
            $(AVMEDIALIB)	
            
SHL3LIBS=   $(SLB)$/swui.lib
LIB3TARGET = $(SLB)$/swui.lib

LIB3OBJFILES = \
        $(SLO)$/swuiexp.obj     \
        $(SLO)$/swdlgfact.obj	\
        $(SLO)$/addrdlg.obj \
        $(SLO)$/ascfldlg.obj \
        $(SLO)$/break.obj \
        $(SLO)$/bookmark.obj \
        $(SLO)$/changedb.obj \
        $(SLO)$/chardlg.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/cption.obj \
        $(SLO)$/dbinsdlg.obj \
        $(SLO)$/docfnote.obj	\
        $(SLO)$/docstdlg.obj \
         $(SLO)$/envlop1.obj \
         $(SLO)$/envfmt.obj  \
         $(SLO)$/envprt.obj  \
         $(SLO)$/label1.obj  \
          $(SLO)$/labfmt.obj  \
          $(SLO)$/labelexp.obj \
           $(SLO)$/labprt.obj  \
           $(SLO)$/drpcps.obj \
           $(SLO)$/pardlg.obj \
           $(SLO)$/pattern.obj \
            $(SLO)$/pggrid.obj \
            $(SLO)$/pgfnote.obj \
            $(SLO)$/rowht.obj \
            $(SLO)$/selglos.obj \
            $(SLO)$/split.obj \
            $(SLO)$/splittbl.obj \
             $(SLO)$/srtdlg.obj	\
             $(SLO)$/tautofmt.obj	\
             $(SLO)$/tblnumfm.obj \
             $(SLO)$/uiborder.obj \
             $(SLO)$/wrap.obj	\
             $(SLO)$/colwd.obj	\
             $(SLO)$/tabledlg.obj \
             $(SLO)$/fldtdlg.obj \
             $(SLO)$/fldedt.obj \
             $(SLO)$/fldpage.obj \
             $(SLO)$/flddb.obj \
             $(SLO)$/flddinf.obj \
             $(SLO)$/flddok.obj \
             $(SLO)$/fldfunc.obj \
             $(SLO)$/fldref.obj \
             $(SLO)$/fldvar.obj \
             $(SLO)$/swrenamexnameddlg.obj	\
             $(SLO)$/swmodalredlineacceptdlg.obj	\
        $(SLO)$/abstract.obj \
    $(SLO)$/frmdlg.obj \
    $(SLO)$/tmpdlg.obj \
    $(SLO)$/frmpage.obj \
    $(SLO)$/glosbib.obj \
    $(SLO)$/glossary.obj \
    $(SLO)$/inpdlg.obj \
    $(SLO)$/insfnote.obj \
    $(SLO)$/insrc.obj \
    $(SLO)$/instable.obj \
    $(SLO)$/insrule.obj \
    $(SLO)$/javaedit.obj \
    $(SLO)$/linenum.obj \
    $(SLO)$/mailmrge.obj \
    $(SLO)$/multmrk.obj \
    $(SLO)$/mergetbl.obj \
    $(SLO)$/outline.obj \
    $(SLO)$/num.obj \
    $(SLO)$/column.obj \
    $(SLO)$/cnttab.obj \
    $(SLO)$/cntex.obj \
    $(SLO)$/uiregionsw.obj \
    $(SLO)$/optload.obj \
        $(SLO)$/optcomp.obj \
        $(SLO)$/optpage.obj \
        $(SLO)$/swuiccoll.obj \
        $(SLO)$/numpara.obj \
        $(SLO)$/swdialmgr.obj \
        $(SLO)$/swuiidxmrk.obj \
                $(SLO)$/wordcountdialog.obj

#SHL3OBJS= \
#        $(SLO)$/swuiexp.obj     \
#        $(SLO)$/swabstdlg.obj	\
#        $(SLO)$/swdlgfact.obj



.INCLUDE :  target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @+$(TYPE) sw.flt > $@

