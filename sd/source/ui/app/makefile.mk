#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: ka $ $Date: 2001-03-08 11:04:21 $
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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=app

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Imagelisten --------------------------------------------------

IMGLST_SRS=$(SRS)$/app.srs
BMP_IN=$(PRJ)$/res/imagelst

# --- Update-Version -----------------------------------------------

RSCUPDVER=$(RSCREVISION)SV$(UPD)$(UPDMINOR)

# --- Files --------------------------------------------------------

SRCFILES =	\
        app.src 			\
        toolbox.src			\
        strings.src 		\
        res_bmp.src 		\
        tbx_ww.src			\
        popup.src			\
        sdstring.src		\
        pseudo.src

OBJFILES = \
        $(OBJ)$/sdlib.obj	\
        $(OBJ)$/sdresid.obj	\

SLOFILES =	\
        $(SLO)$/sdmod.obj		\
        $(SLO)$/sdmod1.obj		\
        $(SLO)$/sdmod2.obj		\
        $(SLO)$/sddll.obj		\
        $(SLO)$/sddll1.obj      \
        $(SLO)$/sddll2.obj      \
        $(SLO)$/tbxww.obj		\
        $(SLO)$/optsitem.obj	\
        $(SLO)$/sdresid.obj		\
        $(SLO)$/sdpopup.obj		\
        $(SLO)$/sdxfer.obj		

.IF "$(GUI)" == "WNT"

NOOPTFILES=\
    $(SLO)$/optsitem.obj

.ENDIF

# --- Tagets -------------------------------------------------------

all: \
    $(INCCOM)$/sddll0.hxx   \
    ALLTAR

$(INCCOM)$/sddll0.hxx: makefile.mk
    +@echo $@
.IF "$(GUI)"=="OS2"
    echo #define DLL_NAME "sd$(UPD)$(DLLPOSTFIX)" >$@
.ELSE
.IF "$(GUI)"=="MAC"
    echo "$(HASHMARK)define DLL_NAME ¶"sd$(UPD)$(DLLPOSTFIX).dll¶"" > $@
.ELSE
.IF "$(GUI)"=="UNX"
    echo #define DLL_NAME \"libsd$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo #define DLL_NAME "sd$(UPD)$(DLLPOSTFIX).DLL" >$@
.ENDIF
.ENDIF
.ENDIF

.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

