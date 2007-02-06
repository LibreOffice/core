#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 14:31:59 $
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

PRJ=..$/..

PRJNAME=rsc
TARGETTYPE=CUI
TARGET=rsc
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

.IF "$(GUI)"!="WIN"
CXXFILES=   gui.cxx                 \
            start.cxx

OBJFILES=   $(OBJ)$/gui.obj          \
            $(OBJ)$/start.obj

.IF "$(GUI)$(COM)$(COMEX)" != "DOSSTCX"
APP1TARGET= rsc
APP1STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
APP1LIBS=   $(LB)$/rsctoo.lib
APP1OBJS=   $(OBJ)$/start.obj
APP1STACK=64000
.IF "$(GUI)" != "OS2"
APP1STACK=32768
.ENDIF
.ENDIF

.IF "$(GUI)$(COM)$(COMEX)" != "DOSSTC"
APP2TARGET= rsc2
.IF "$(GUI)" == "MAC"
APP2STDLIBS=$(PRJ)$/..$/TOOLS$/$(INPATH)$/SLB$/tools.lib \
            $(PRJ)$/..$/SAL$/$(INPATH)$/SLB$/cpposl.lib \
            $(PRJ)$/..$/VOS$/$(INPATH)$/SLB$/cppvos.lib \
            $(PRJ)$/..$/RTL$/$(INPATH)$/SLB$/cpprtl.lib \
            $(MWPPCLibraries)PPCToolLibs.o  \
            $(STDSLOGUI)
MACRES= $(SV_RES)MPWToolCfrg.r -d SVTOOLNAME="¶"SV TOOL¶""
.ELSE
.IF "$(OS)"=="SCO"
# SCO hat Probleme mit fork/exec und einigen shared libraries. 
# rsc2 muss daher statisch gelinkt werden
APP2STDLIBS=$(STATIC) -latools $(BPICONVLIB) $(VOSLIB) $(OSLLIB) $(RTLLIB) $(DYNAMIC)
.ELSE
APP2STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) #RTLLIB)
.ENDIF
.ENDIF
APP2LIBS=   $(LB)$/rsctoo.lib \
            $(LB)$/rscres.lib \
            $(LB)$/rscpar.lib \
            $(LB)$/rscrsc.lib \
            $(LB)$/rscmis.lib
APP2OBJS=   $(OBJ)$/gui.obj
APP2STACK=64000
.IF "$(GUI)" != "OS2"
APP2STACK=32768
.ENDIF
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

# -------------------------------------------------------------------
# PM2
# -------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(APP1TARGET).def : makefile
    echo  NAME          RSC WINDOWCOMPAT                    >$@
    echo  DESCRIPTION   'RSC-Compiler'                     >>$@
.IF "$(COM)" != "BLC"
    echo  STUB          'os2STUB.EXE'                      >>$@
.ENDIF
    echo  DATA          MULTIPLE                           >>$@
    echo  EXETYPE       OS2                                >>$@
    echo  PROTMODE                                         >>$@
    echo  HEAPSIZE      16000                              >>$@
    echo  STACKSIZE     48000                              >>$@

$(MISC)$/$(APP2TARGET).def : makefile
    echo  NAME          RSC2 WINDOWCOMPAT                   >$@
    echo  DESCRIPTION   'RSC2-Compiler'                    >>$@
.IF "$(COM)" != "BLC"
    echo  STUB          'os2STUB.EXE'                      >>$@
.ENDIF
    echo  DATA          MULTIPLE                           >>$@
    echo  EXETYPE       OS2                                >>$@
    echo  PROTMODE                                         >>$@
    echo  HEAPSIZE      16000                              >>$@
    echo  STACKSIZE     48000                              >>$@
.ENDIF
