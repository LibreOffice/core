#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

PRJ=..$/..

PRJNAME=rsc
TARGETTYPE=CUI
TARGET=rsc
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------------

.IF "$(GUI)"!="WIN"
CXXFILES=   gui.cxx                 \
            start.cxx

OBJFILES=   $(OBJ)$/gui.obj          \
            $(OBJ)$/start.obj

.IF "$(GUI)$(COM)$(COMEX)" != "DOSSTCX"
APP1TARGET= rsc
APP1STDLIBS=$(TOOLSLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
APP1LIBS=   $(LIBPRE) $(LB)$/rsctoo.lib
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
APP2STDLIBS=$(TOOLSLIB) $(VOSLIB) $(SALLIB) #RTLLIB)
.ENDIF
.ENDIF
APP2LIBS=   $(LIBPRE) $(LB)$/rsctoo.lib \
            $(LIBPRE) $(LB)$/rscres.lib \
            $(LIBPRE) $(LB)$/rscpar.lib \
            $(LIBPRE) $(LB)$/rscrsc.lib \
            $(LIBPRE) $(LB)$/rscmis.lib
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
