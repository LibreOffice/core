#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.19 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

NO_HID_FILES=app.src
.EXPORT : NO_HID_FILES

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        app.src 			\
        toolbox.src			\
        strings.src 		\
        res_bmp.src 		\
        tbx_ww.src			\
        popup.src			\
        sdstring.src

#OBJFILES = \
#		$(OBJ)$/sdlib.obj	\
#		$(OBJ)$/sdresid.obj	\

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
        $(SLO)$/sdxfer.obj		\
        $(SLO)$/tmplctrl.obj

EXCEPTIONSFILES= \
        $(SLO)$/sdxfer.obj		\
        $(SLO)$/sdmod1.obj		\
        $(SLO)$/tmplctrl.obj

.IF "$(GUI)" == "WNT"

NOOPTFILES=\
    $(SLO)$/optsitem.obj

.ENDIF

# --- Tagets -------------------------------------------------------

all: \
    $(INCCOM)$/sddll0.hxx   \
    ALLTAR

$(INCCOM)$/sddll0.hxx: makefile.mk
    @echo $@
.IF "$(GUI)"=="UNX"
    echo \#define DLL_NAME \"libsd$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE			# "$(GUI)"=="UNX"
.IF "$(USE_SHELL)"!="4nt"
    echo \#define DLL_NAME \"sd$(DLLPOSTFIX).DLL\" >$@
.ELSE			# "$(USE_SHELL)"!="4nt"
    echo #define DLL_NAME "sd$(DLLPOSTFIX).DLL" >$@
.ENDIF			#  "$(USE_SHELL)"!="4nt"
.ENDIF			# "$(GUI)"=="UNX"

LOCALIZE_ME =  tbxids_tmpl.src menuids2_tmpl.src menu_tmpl.src menuids_tmpl.src menuids4_tmpl.src popup2_tmpl.src toolbox2_tmpl.src menuportal_tmpl.src menuids3_tmpl.src


.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


$(SRS)$/$(SRS1NAME).srs  : $(LOCALIZE_ME_DEST)

