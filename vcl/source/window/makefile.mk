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

PRJNAME=vcl
TARGET=win
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
            $(SLO)$/arrange.obj		\
            $(SLO)$/abstdlg.obj		\
            $(SLO)$/accel.obj		\
            $(SLO)$/accmgr.obj		\
            $(SLO)$/brdwin.obj		\
            $(SLO)$/btndlg.obj		\
            $(SLO)$/cmdevt.obj		\
            $(SLO)$/cursor.obj		\
            $(SLO)$/decoview.obj	\
            $(SLO)$/dialog.obj		\
            $(SLO)$/dlgctrl.obj 	\
            $(SLO)$/dndevdis.obj	\
            $(SLO)$/dndlcon.obj     \
            $(SLO)$/dockingarea.obj \
            $(SLO)$/dockmgr.obj 	\
            $(SLO)$/dockwin.obj 	\
            $(SLO)$/floatwin.obj	\
            $(SLO)$/introwin.obj	\
            $(SLO)$/keycod.obj		\
            $(SLO)$/keyevent.obj	\
            $(SLO)$/mouseevent.obj	\
            $(SLO)$/menu.obj		\
            $(SLO)$/mnemonic.obj	\
            $(SLO)$/mnemonicengine.obj  \
            $(SLO)$/msgbox.obj		\
            $(SLO)$/popupmenuwindow.obj		\
            $(SLO)$/scrwnd.obj		\
            $(SLO)$/printdlg.obj    \
            $(SLO)$/seleng.obj		\
            $(SLO)$/split.obj		\
            $(SLO)$/splitwin.obj	\
            $(SLO)$/status.obj		\
            $(SLO)$/syschild.obj	\
            $(SLO)$/javachild.obj	\
            $(SLO)$/syswin.obj		\
            $(SLO)$/tabdlg.obj		\
            $(SLO)$/tabpage.obj 	\
            $(SLO)$/taskpanelist.obj	\
            $(SLO)$/toolbox.obj 	\
            $(SLO)$/toolbox2.obj	\
            $(SLO)$/window.obj		\
            $(SLO)$/winproc.obj 	\
            $(SLO)$/window2.obj 	\
            $(SLO)$/window3.obj		\
            $(SLO)$/window4.obj		\
            $(SLO)$/wpropset.obj	\
            $(SLO)$/wrkwin.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    @$(RM) $@
    @echo \#define DLL_NAME \"libcui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    @echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/abstdlg.obj : $(INCCOM)$/cuilib.hxx
