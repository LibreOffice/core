#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.24 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 13:04:57 $
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

PRJNAME=vcl
TARGET=win
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
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
            $(SLO)$/scrwnd.obj		\
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
            $(SLO)$/wrkwin.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

$(INCCOM)$/cuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"libcui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"cui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/abstdlg.obj : $(INCCOM)$/cuilib.hxx
