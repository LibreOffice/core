#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: nn $ $Date: 2001-04-03 17:41:34 $
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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=app

PROJECTPCH4DLL=TRUE
PROJECTPCH=ui_pch
PDBTARGET=ui_pch
PROJECTPCHSOURCE=..$/pch$/ui_pch

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

OBJFILES = \
    $(OBJ)$/sclib.obj  \

SLOFILES =  \
    $(SLO)$/scmod.obj  \
    $(SLO)$/scmod2.obj  \
    $(SLO)$/scdll.obj  \
    $(SLO)$/typemap.obj  \
    $(SLO)$/dataobj.obj \
    $(SLO)$/dataobj2.obj \
    $(SLO)$/dataobj3.obj \
    $(SLO)$/transobj.obj \
    $(SLO)$/drwtrans.obj \
    $(SLO)$/lnktrans.obj \
    $(SLO)$/inputhdl.obj \
    $(SLO)$/inputwin.obj \
    $(SLO)$/rfindlst.obj \
    $(SLO)$/uiitems.obj  \
    $(SLO)$/msgpool.obj \
    $(SLO)$/client.obj

EXCEPTIONSFILES= \
    $(SLO)$/scmod2.obj

LIB3TARGET=$(SLB)$/ysclib.lib
LIB3OBJFILES=$(SLO)$/sclib.obj

# --- Targets -------------------------------------------------------

.IF "$(depend)" == ""


ALL:    \
        $(INCCOM)$/scdll0.hxx\
        ALLTAR

$(INCCOM)$/scdll0.hxx: makefile.mk
.IF "$(GUI)"=="OS2"
    echo #define DLL_NAME "sc$(UPD)$(DLLPOSTFIX)" >$@
.ELSE
.IF "$(GUI)"=="MAC"
    echo "$(HASHMARK)define DLL_NAME ¶"sc$(UPD)$(DLLPOSTFIX).dll¶"" > $@
.ELSE
.IF "$(GUI)"=="UNX"
    echo #define DLL_NAME \"libsc$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo #define DLL_NAME "sc$(UPD)$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF
.ENDIF
.ENDIF
.ENDIF

.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

