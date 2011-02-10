#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
PRJ=..$/..
PRJNAME=svx
TARGET=dialogs
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

#
.IF "$(GUI)"=="WNT"
CFLAGS+= -DUNICODE -D_UNICODE
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=dialogs
SRC1FILES =  \
        bmpmask.src \
        contdlg.src \
        ctredlin.src \
        dlgctrl.src \
        docrecovery.src \
        fontwork.src \
        frmsel.src \
        hdft.src \
        imapdlg.src \
        langbox.src \
        language.src \
        linkwarn.src \
        passwd.src \
        prtqry.src \
        rubydialog.src\
        ruler.src \
        srchdlg.src \
        svxbmpnumvalueset.src \
        swframeposstrings.src \
        txenctab.src \
        ucsubset.src \
        optgrid.src     

SRS2NAME=drawdlgs
SRC2FILES =  \
        sdstring.src

LIB1TARGET=$(SLB)$/$(TARGET)-core.lib

LIB1OBJFILES= \
        $(SLO)$/checklbx.obj \
        $(SLO)$/dialmgr.obj\
        $(SLO)$/dlgutil.obj \
        $(SLO)$/framelink.obj\
        $(SLO)$/langbox.obj \
        $(SLO)$/simptabl.obj \
        $(SLO)$/svxdlg.obj \
        $(SLO)$/stddlg.obj \

LIB2TARGET=$(SLB)$/$(TARGET).lib

LIB2OBJFILES= \
        $(SLO)$/charmap.obj \
        $(SLO)$/connctrl.obj \
        $(SLO)$/contwnd.obj \
        $(SLO)$/ctredlin.obj \
        $(SLO)$/databaseregistrationui.obj \
        $(SLO)$/dialcontrol.obj \
        $(SLO)$/dlgctl3d.obj \
        $(SLO)$/dlgctrl.obj \
        $(SLO)$/docrecovery.obj \
        $(SLO)$/fntctrl.obj \
        $(SLO)$/fontlb.obj \
        $(SLO)$/fontwork.obj \
        $(SLO)$/framelinkarray.obj \
        $(SLO)$/frmdirlbox.obj \
        $(SLO)$/frmsel.obj \
        $(SLO)$/graphctl.obj \
        $(SLO)$/grfflt.obj \
        $(SLO)$/hdft.obj \
        $(SLO)$/hyperdlg.obj \
        $(SLO)$/imapdlg.obj \
        $(SLO)$/imapwnd.obj \
        $(SLO)$/linkwarn.obj \
        $(SLO)$/measctrl.obj \
        $(SLO)$/orienthelper.obj \
        $(SLO)$/pagectrl.obj \
        $(SLO)$/paraprev.obj \
        $(SLO)$/passwd.obj \
        $(SLO)$/pfiledlg.obj \
        $(SLO)$/prtqry.obj \
        $(SLO)$/relfld.obj \
        $(SLO)$/rlrcitem.obj \
        $(SLO)$/rubydialog.obj \
        $(SLO)$/rulritem.obj \
        $(SLO)$/SpellDialogChildWindow.obj \
        $(SLO)$/srchctrl.obj \
        $(SLO)$/srchdlg.obj \
        $(SLO)$/strarray.obj \
        $(SLO)$/svxbmpnumvalueset.obj\
        $(SLO)$/svxgrahicitem.obj \
        $(SLO)$/svxruler.obj \
        $(SLO)$/swframeexample.obj \
        $(SLO)$/swframeposstrings.obj \
        $(SLO)$/txencbox.obj \
        $(SLO)$/txenctab.obj \
        $(SLO)$/wrapfield.obj \
        $(SLO)$/_bmpmask.obj \
        $(SLO)$/_contdlg.obj \
        $(SLO)$/optgrid.obj
        

.IF "$(GUI)"=="UNX"
LIB2OBJFILES +=    $(SLO)$/sendreportunx.obj
.ELSE
.IF "$(GUI)"=="WNT"
LIB2OBJFILES +=	$(SLO)$/sendreportw32.obj
.ELSE
LIB2OBJFILES +=	$(SLO)$/sendreportgen.obj
.ENDIF
.ENDIF

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

