#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: hdu $ $Date: 2001-02-19 15:36:27 $
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

PRJNAME=VCL
TARGET=vcl
VERSION=$(UPD)
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk
.INCLUDE :	makefile.pmk

LDUMP=ldump2.exe

# --- Allgemein ----------------------------------------------------------

.IF "$(depend)" == ""

HXXDEPNLST= $(INC)$/accel.hxx		\
            $(INC)$/animate.hxx 	\
            $(INC)$/apptypes.hxx	\
            $(INC)$/bitmap.hxx		\
            $(INC)$/bitmapex.hxx	\
            $(INC)$/bmpacc.hxx		\
            $(INC)$/btndlg.hxx		\
            $(INC)$/button.hxx		\
            $(INC)$/ctrl.hxx		\
            $(INC)$/color.hxx		\
            $(INC)$/config.hxx		\
            $(INC)$/cursor.hxx		\
            $(INC)$/clip.hxx		\
            $(INC)$/cmdevt.hxx		\
            $(INC)$/drag.hxx		\
            $(INC)$/decoview.hxx	\
            $(INC)$/dialog.hxx		\
            $(INC)$/dockwin.hxx 	\
            $(INC)$/edit.hxx		\
            $(INC)$/event.hxx		\
            $(INC)$/exchange.hxx	\
            $(INC)$/field.hxx		\
            $(INC)$/fixed.hxx		\
            $(INC)$/floatwin.hxx	\
            $(INC)$/font.hxx		\
            $(INC)$/floatwin.hxx	\
            $(INC)$/graph.hxx		\
            $(INC)$/group.hxx		\
            $(INC)$/help.hxx		\
            $(INC)$/jobset.hxx		\
            $(INC)$/keycodes.hxx	\
            $(INC)$/keycod.hxx		\
            $(INC)$/image.hxx		\
            $(INC)$/line.hxx		\
            $(INC)$/lstbox.h		\
            $(INC)$/lstbox.hxx		\
            $(INC)$/mapmod.hxx		\
            $(INC)$/metaact.hxx 	\
            $(INC)$/menu.hxx		\
            $(INC)$/menubtn.hxx 	\
            $(INC)$/metric.hxx		\
            $(INC)$/morebtn.hxx 	\
            $(INC)$/msgbox.hxx		\
            $(INC)$/octree.hxx		\
            $(INC)$/outdev.hxx		\
            $(INC)$/outdev3d.hxx	\
            $(INC)$/pointr.hxx		\
            $(INC)$/poly.hxx		\
            $(INC)$/ptrstyle.hxx	\
            $(INC)$/prntypes.hxx	\
            $(INC)$/print.hxx		\
            $(INC)$/prndlg.hxx		\
            $(INC)$/region.hxx		\
            $(INC)$/rc.hxx			\
            $(INC)$/resid.hxx		\
            $(INC)$/resary.hxx		\
            $(INC)$/salbtype.hxx	\
            $(INC)$/scrbar.hxx		\
            $(INC)$/slider.hxx		\
            $(INC)$/seleng.hxx		\
            $(INC)$/settings.hxx	\
            $(INC)$/sound.hxx		\
            $(INC)$/sndstyle.hxx	\
            $(INC)$/split.hxx		\
            $(INC)$/splitwin.hxx	\
            $(INC)$/spin.hxx		\
            $(INC)$/spinfld.hxx 	\
            $(INC)$/status.hxx		\
            $(INC)$/stdtext.hxx 	\
            $(INC)$/sv.h			\
            $(INC)$/svapp.hxx		\
            $(INC)$/syschild.hxx	\
            $(INC)$/sysdata.hxx 	\
            $(INC)$/system.hxx		\
            $(INC)$/syswin.hxx		\
            $(INC)$/tabctrl.hxx 	\
            $(INC)$/tabdlg.hxx		\
            $(INC)$/tabpage.hxx 	\
            $(INC)$/toolbox.hxx 	\
            $(INC)$/timer.hxx		\
            $(INC)$/virdev.hxx		\
            $(INC)$/wall.hxx		\
            $(INC)$/waitobj.hxx 	\
            $(INC)$/wintypes.hxx	\
            $(INC)$/window.hxx		\
            $(INC)$/wrkwin.hxx

.IF "$(linkinc)" != ""
SHL11FILE=	$(MISC)$/app.slo
SHL12FILE=	$(MISC)$/gdi.slo
SHL13FILE=	$(MISC)$/win.slo
SHL14FILE=	$(MISC)$/ctrl.slo
#SHL15FILE=  $(MISC)$/ex.slo
SHL16FILE=	$(MISC)$/salapp.slo
SHL17FILE=	$(MISC)$/salwin.slo
SHL18FILE=	$(MISC)$/salgdi.slo
.ENDIF

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/app.lib 	\
            $(SLB)$/gdi.lib 	\
            $(SLB)$/win.lib 	\
            $(SLB)$/ctrl.lib	\
            $(SLB)$/helper.lib

.IF "$(TF_SVDATA)"==""
LIB1FILES+= $(SLB)$/ex.lib
.ENDIF

.IF "$(remote)" != ""
    LIB1FILES+= $(SLB)$/remote.lib
.IF "$(COM)"=="GCC"
LIB1OBJFILES=$(SLO)$/salmain.obj
.ENDIF
.ELSE
LIB1FILES+= \
            $(SLB)$/salwin.lib	\
            $(SLB)$/salgdi.lib	\
            $(SLB)$/salapp.lib
.ENDIF

.IF "$(GUI)" == "UNX"
.IF "$(PSPRINT)" != ""
    SHL1STDLIBS=-lpsp$(VERSION)$(DLLPOSTFIX)
.ENDIF
.ENDIF

.IF "$(USE_BUILTIN_RASTERIZER)"!=""
    LIB1FILES +=	$(SLB)$/glyphs.lib
    SHL1STDLIBS+=	$(FREETYPELIB)
.ENDIF

SHL1TARGET= vcl$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= ivcl
SHL1STDLIBS+=\
            $(TOOLSLIB) 		\
            $(SOTLIB)			\
            $(VOSLIB)			\
            $(SALLIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(CPPULIB)			\
            $(UNOTOOLSLIB)		\
            $(COMPHELPERLIB)

.IF "$(remote)" != ""
SHL1STDLIBS+=	$(UNOLIB)
.ENDIF

.IF "$(GUI)"!="MAC"
SHL1DEPN=	$(L)$/itools.lib $(L)$/sot.lib
.ENDIF

SHL1LIBS=	$(LIB1TARGET)
.IF "$(GUI)"!="UNX"
SHL1OBJS=	$(SLO)$/salshl.obj
.ENDIF

.IF "$(GUI)" != "MAC"
.IF "$(GUI)" != "UNX"
SHL1RES=	$(RES)$/salsrc.res
.ENDIF
.ENDIF
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

ALLTAR+=	$(LIB1TARGET)

# --- VCL-Light ----------------------------------------------------------

.IF "$(VCL_LIGHT)" != ""
LIB2TARGET= $(SLB)$/vll.lib
LIB2FILES=	$(SLO)$/access.obj		\
            $(SLO)$/config.obj		\
            $(SLO)$/dbggui.obj		\
            $(SLO)$/system.obj		\
            $(SLO)$/oldsv.obj		\
            $(SLO)$/help.obj		\
            $(SLO)$/idlemgr.obj 	\
            $(SLO)$/resmgr.obj		\
            $(SLO)$/settings.obj	\
            $(SLO)$/sound.obj		\
            $(SLO)$/stdtext.obj 	\
            $(SLO)$/svapp.obj		\
            $(SLO)$/svdata.obj		\
            $(SLO)$/svmain.obj		\
            $(SLO)$/timer.obj		\
            $(SLO)$/animate.obj 	\
            $(SLO)$/salmisc.obj 	\
            $(SLO)$/impanmvw.obj	\
            $(SLO)$/bitmap.obj		\
            $(SLO)$/bitmap2.obj 	\
            $(SLO)$/bitmap3.obj 	\
            $(SLO)$/bitmap4.obj 	\
            $(SLO)$/alpha.obj		\
            $(SLO)$/bitmapex.obj	\
            $(SLO)$/imgcons.obj 	\
            $(SLO)$/bmpacc.obj		\
            $(SLO)$/bmpacc2.obj 	\
            $(SLO)$/bmpacc3.obj 	\
            $(SLO)$/color.obj		\
            $(SLO)$/cvtsvm.obj		\
            $(SLO)$/cvtgrf.obj		\
            $(SLO)$/font.obj		\
            $(SLO)$/gdimtf.obj		\
            $(SLO)$/gfxlink.obj 	\
            $(SLO)$/graph.obj		\
            $(SLO)$/impgraph.obj	\
            $(SLO)$/gradient.obj	\
            $(SLO)$/hatch.obj		\
            $(SLO)$/image.obj		\
            $(SLO)$/impbmp.obj		\
            $(SLO)$/impimage.obj	\
            $(SLO)$/impprn.obj		\
            $(SLO)$/impvect.obj 	\
            $(SLO)$/implncvt.obj	\
            $(SLO)$/jobset.obj		\
            $(SLO)$/line.obj		\
            $(SLO)$/lineinfo.obj	\
            $(SLO)$/mapmod.obj		\
            $(SLO)$/metaact.obj 	\
            $(SLO)$/metric.obj		\
            $(SLO)$/octree.obj		\
            $(SLO)$/outmap.obj		\
            $(SLO)$/outdev.obj		\
            $(SLO)$/outdev2.obj 	\
            $(SLO)$/outdev3.obj 	\
            $(SLO)$/outdev4.obj 	\
            $(SLO)$/outdev5.obj 	\
            $(SLO)$/outdev6.obj 	\
            $(SLO)$/opengl.obj		\
            $(SLO)$/poly.obj		\
            $(SLO)$/poly2.obj		\
            $(SLO)$/print.obj		\
            $(SLO)$/print2.obj		\
            $(SLO)$/regband.obj 	\
            $(SLO)$/region.obj		\
            $(SLO)$/virdev.obj		\
            $(SLO)$/wall.obj		\
            $(SLO)$/accel.obj		\
            $(SLO)$/accmgr.obj		\
            $(SLO)$/brdwin.obj		\
            $(SLO)$/btndlg.obj		\
            $(SLO)$/cursor.obj		\
            $(SLO)$/dockwin.obj 	\
            $(SLO)$/decoview.obj	\
            $(SLO)$/dialog.obj		\
            $(SLO)$/dlgctrl.obj 	\
            $(SLO)$/floatwin.obj	\
            $(SLO)$/keycod.obj		\
            $(SLO)$/menu.obj		\
            $(SLO)$/mnemonic.obj	\
            $(SLO)$/msgbox.obj		\
            $(SLO)$/syschild.obj	\
            $(SLO)$/syswin.obj		\
            $(SLO)$/toolbox.obj 	\
            $(SLO)$/toolbox2.obj	\
            $(SLO)$/window.obj		\
            $(SLO)$/window2.obj 	\
            $(SLO)$/winproc.obj 	\
            $(SLO)$/wrkwin.obj		\
            $(SLO)$/scrwnd.obj		\
            $(SLO)$/button.obj		\
            $(SLO)$/ctrl.obj		\
            $(SLO)$/combobox.obj	\
            $(SLO)$/edit.obj		\
            $(SLO)$/field.obj		\
            $(SLO)$/fixed.obj		\
            $(SLO)$/group.obj		\
            $(SLO)$/ilstbox.obj 	\
            $(SLO)$/lstbox.obj		\
            $(SLO)$/morebtn.obj 	\
            $(SLO)$/spinfld.obj 	\
            $(SLO)$/scrbar.obj		\
            $(SLO)$/tabctrl.obj 	\
            $(SLB)$/ex.lib			\
            $(SLB)$/helper.lib		\
            $(SLB)$/salwin.lib		\
            $(SLB)$/salgdi.lib		\
            $(SLB)$/salapp.lib

#			 $(SLO)$/resary.obj 	 \
#			 $(SLO)$/filedlg.obj	 \
#			 $(SLO)$/seleng.obj 	 \
#			 $(SLO)$/split.obj		 \
#			 $(SLO)$/splitwin.obj	 \
#			 $(SLO)$/status.obj 	 \
#			 $(SLO)$/tabdlg.obj 	 \
#			 $(SLO)$/tabpage.obj	 \
#			 $(SLO)$/field2.obj 	 \
#			 $(SLO)$/imgctrl.obj	 \
#			 $(SLO)$/longcurr.obj	 \
#			 $(SLO)$/fixbrd.obj 	 \
#			 $(SLO)$/menubtn.obj	 \
#			 $(SLO)$/slider.obj 	 \
#			 $(SLO)$/spinbtn.obj	 \

SHL2TARGET= vll$(VERSION)$(DLLPOSTFIX)
SHL2IMPLIB= ivll
SHL2STDLIBS=$(TOOLSLIB) 		\
            $(SOTLIB)			\
            $(VOSLIB)			\
            $(SALLIB)			\
            $(CPPUHELPERLIB)	\
            $(CPPULIB)		\
            $(UNOTOOLSLIB)
.IF "$(GUI)"!="MAC"
SHL2DEPN=	$(L)$/itools.lib $(L)$/sot.lib
.ENDIF
SHL2LIBS=	$(LIB2TARGET)

.IF "$(GUI)"!="UNX"
SHL2OBJS=	$(SLO)$/salshl.obj
.ENDIF

.IF "$(GUI)" != "MAC"
.IF "$(GUI)" != "UNX"
SHL2RES=	$(RES)$/salsrc.res
.ENDIF
.ENDIF
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

ALLTAR+=	$(LIB2TARGET)

.ENDIF

# --- All ----------------------------------------------------------------

ALL:		ALLTAR

# ---MAC----------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
        delete -i $@
        $(LINK)  $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL$(TNR)OBJS) $(SHL$(TNR)LIBS) -f $@.exp · dev:null
        duplicate -y $@.exp $@
        delete -i $@.exp
.IF "$(RUECKWAERTS)" != "" || "$(rueckwaerts)" != ""
        filter_import "$(SOLARINCDIR)"$/GLOBAL.IMP $@
.ENDIF

.IF "$(VCL_LIGHT)" != ""
$(MISC)$/$(SHL2TARGET).def:  makefile.mk
        delete -i $@
        $(LINK)  $(LINKFLAGS) $(LINKFLAGSSHL) $(SHL$(TNR)OBJS) $(SHL$(TNR)LIBS) -f $@.exp · dev:null
        duplicate -y $@.exp $@
        delete -i $@.exp
.IF "$(RUECKWAERTS)" != "" || "$(rueckwaerts)" != ""
        filter_import "$(SOLARINCDIR)"$/GLOBAL.IMP $@
.ENDIF
.ENDIF

.ENDIF

# --- W32 ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

SHL1STDLIBS += gdi32.lib		\
               winspool.lib 	\
               ole32.lib		\
               shell32.lib		\
               advapi32.lib 	\
               imm32.lib

#.IF ("$(COM)" == "MSC") && ("$(GUI)" == "WNT") && (("$(CPU)" == "A") || ("$(CPU)"=="P") || ("$(CPU)" == "M"))
.IF "$(COM)$(GUI)" == "MSCWNT"
.IF $(CPU)" == "A" || "$(CPU)"=="P" || "$(CPU)" == "M"
LINKFLAGSSHL +=  /ENTRY:LibMain
.ENDIF
.ENDIF

#.IF ("$(COM)" == "MSC") && ("$(GUI)" == "WNT") && ("$(CPU)" == "I")
.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
LINKFLAGSSHL += /ENTRY:LibMain@12
.ENDIF

# --- Def-File ---

$(MISC)$/$(SHL1TARGET).def: $(MISC)$/$(SHL1TARGET).flt				\
                            $(HXXDEPNLST)							\
                            makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @+-attrib -r defs
    @echo LIBRARY	  $(SHL1TARGET) 								 >$@
    @echo DESCRIPTION 'VCL'                                         >>$@
    @echo DATA		  READWRITE NONSHARED							>>$@
    @echo EXPORTS													>>$@
    @echo GetVersionInfo											>>$@
    $(LIBMGR) -EXTRACT:/ /OUT:$(TARGET).exp $(LIB1TARGET)
    @$(LDUMP) -E20 -F$(MISC)$/$(SHL1TARGET).flt $(TARGET).exp		>>$@
    +-del $(TARGET).exp

# --- VCL-Light ---

.IF "$(VCL_LIGHT)" != ""

SHL2STDLIBS += gdi32.lib		\
               winspool.lib 	\
               ole32.lib		\
               shell32.lib		\
               advapi32.lib 	\
               imm32.lib

#.IF ("$(COM)" == "MSC") && ("$(GUI)" == "WNT") && (("$(CPU)" == "A") || ("$(CPU)"=="P") || ("$(CPU)" == "M"))
.IF "$(COM)$(GUI)" == "MSCWNT"
.IF $(CPU)" == "A" || "$(CPU)"=="P" || "$(CPU)" == "M"
LINKFLAGSSHL +=  /ENTRY:LibMain
.ENDIF
.ENDIF

#.IF ("$(COM)" == "MSC") && ("$(GUI)" == "WNT") && ("$(CPU)" == "I")
.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
LINKFLAGSSHL += /ENTRY:LibMain@12
.ENDIF

# --- Def-File ---

$(MISC)$/$(SHL2TARGET).def: $(MISC)$/$(SHL1TARGET).flt				\
                            $(HXXDEPNLST)							\
                            makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY	  $(SHL2TARGET) 								 >$@
    @echo DESCRIPTION 'VCL'                                         >>$@
    @echo DATA		  READWRITE NONSHARED							>>$@
    @echo EXPORTS													>>$@
    @echo GetVersionInfo											>>$@
    $(LIBMGR) -EXTRACT:/ /OUT:vll.exp $(LIB2TARGET)
    @$(LDUMP) -E20 -F$(MISC)$/$(SHL1TARGET).flt vll.exp 			>>$@
    +-del vll.exp

.ENDIF

.ENDIF

# --- OS2 ----------------------------------------------------------------

.IF "$(GUI)" == "OS2"

# --- Def-File ---

$(MISC)$/$(SHL1TARGET).def: $(MISC)$/$(SHL1TARGET).flt				\
                            $(HXXDEPNLST)							\
                            makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY	  $(SHL1TARGET) INITINSTANCE TERMINSTANCE		 >$@
    @echo DESCRIPTION 'VCL'                                        >>$@
    @echo PROTMODE													>>$@
    @echo CODE		  LOADONCALL									>>$@
    @echo DATA		  PRELOAD MULTIPLE NONSHARED					>>$@
    @echo EXPORTS													>>$@
    @$(LDUMP) -E1 -A -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET) 	>>$@

# --- VCL-Light ---

.IF "$(VCL_LIGHT)" != ""

$(MISC)$/$(SHL2TARGET).def: $(MISC)$/$(SHL1TARGET).flt				\
                            $(HXXDEPNLST)							\
                            makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY	  $(SHL2TARGET) INITINSTANCE TERMINSTANCE		 >$@
    @echo DESCRIPTION 'VCL'                                        >>$@
    @echo PROTMODE													>>$@
    @echo CODE		  LOADONCALL									>>$@
    @echo DATA		  PRELOAD MULTIPLE NONSHARED					>>$@
    @echo EXPORTS													>>$@
    @$(LDUMP) -E1 -A -F$(MISC)$/$(SHL1TARGET).flt $(LIB2TARGET) 	>>$@

.ENDIF

.ENDIF

# --- UNX ----------------------------------------------------------------

.IF "$(GUI)"=="UNX"

.IF "$(USE_XPRINT)"!="TRUE"
.ELSE
CFLAGS+=-D_USE_PRINT_EXTENSION_=1
.ENDIF

# Solaris
.IF "$(OS)"=="SOLARIS"

.IF "$(USE_XPRINT)" == "TRUE"
SHL1STDLIBS += -lXp -lXm -lXt -lX11
.ELIF "$(PSPRINT)"!="" 
SHL1STDLIBS += -lXm -lXt -lX11
.ELSE
SHL1STDLIBS += -lxp$(UPD)$(DLLPOSTFIX) -lXm -lXt -lX11
.ENDIF

# MacOSX
.ELIF "$(OS)"=="MACOSX"
SHL1STDLIBS +=

# Others
.ELSE

.IF "$(USE_XPRINT)" == "TRUE"
SHL1STDLIBS += -lXp -lXaw -lXt -lX11
.ELIF "$(PSPRINT)"!=""
SHL1STDLIBS += -lXaw -lXt -lX11
.ELSE
SHL1STDLIBS += -lxp$(UPD)$(DLLPOSTFIX) -lXaw -lXt -lX11
.ENDIF

.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="SOLARIS"
SHL1STDLIBS += -laudio
.ENDIF

# --- VCL-Light ---

.IF "$(VCL_LIGHT)" != ""

.IF "$(OS)"=="SOLARIS"
SHL2STDLIBS += -lxp$(UPD)$(DLLPOSTFIX) -lXm -lXt -lX11
.ELSE
SHL2STDLIBS += -lxp$(UPD)$(DLLPOSTFIX) -lXaw -lXt -lX11
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="SOLARIS"
SHL2STDLIBS += -laudio
.ENDIF

.ENDIF

.ENDIF

# --- Allgemein ----------------------------------------------------------

# --- VCL-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Impl > $@
    @echo Sal>> $@
    @echo Dbg>> $@
    @echo HelpTextWindow>> $@
    @echo MenuBarWindow>> $@
    @echo MenuFloatingWindow>> $@
    @echo MenuItemList>> $@
    @echo LibMain>> $@
    @echo LIBMAIN>> $@
    @echo Wep>> $@
    @echo WEP>> $@
    @echo RmEvent>> $@
    @echo RmFrameWindow>> $@
    @echo RmPrinter>> $@
    @echo RmBitmap>> $@
    @echo RmSound>> $@
    @echo __CT>> $@

# --- Targets ------------------------------------------------------------

.ENDIF

.INCLUDE :	target.mk
