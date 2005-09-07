#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.39 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:43:35 $
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

PRJNAME=sfx2
TARGET=appl
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
CFLAGS+=-Od
.ENDIF

.IF "$(GUI)"=="UNX"
        CDEFS+=-DDLL_NAME=libsfx$(UPD)$(DLLPOSTFIX)$(DLLPOST)
.ELSE
        CDEFS+=-DDLL_NAME=sfx$(UPD)$(DLLPOSTFIX)$(DLLPOST)
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=appl
SRC1FILES =  \
        app.src image.src newhelp.src dde.src

SRS2NAME=sfx
SRC2FILES =  \
        sfx.src

SLOFILES =  \
    $(SLO)$/imagemgr.obj\
    $(SLO)$/appuno.obj \
    $(SLO)$/appmail.obj \
    $(SLO)$/appmain.obj \
    $(SLO)$/appopen.obj \
    $(SLO)$/appinit.obj \
    $(SLO)$/appmisc.obj \
    $(SLO)$/appdemo.obj \
    $(SLO)$/appreg.obj \
    $(SLO)$/appcfg.obj \
    $(SLO)$/appquit.obj \
    $(SLO)$/appchild.obj \
    $(SLO)$/appserv.obj \
    $(SLO)$/appdata.obj \
    $(SLO)$/app.obj \
    $(SLO)$/appbas.obj \
    $(SLO)$/appdde.obj \
    $(SLO)$/workwin.obj \
    $(SLO)$/sfxhelp.obj \
    $(SLO)$/childwin.obj \
    $(SLO)$/sfxdll.obj \
    $(SLO)$/module.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/dlgcont.obj \
    $(SLO)$/namecont.obj \
    $(SLO)$/scriptcont.obj \
    $(SLO)$/newhelp.obj \
    $(SLO)$/helpinterceptor.obj \
    $(SLO)$/shutdownicon.obj \
    $(SLO)$/shutdowniconw32.obj \
    $(SLO)$/sfxpicklist.obj \
    $(SLO)$/helpdispatch.obj \
    $(SLO)$/imestatuswindow.obj \
    $(SLO)$/xpackcreator.obj\
    $(SLO)$/linksrc.obj\
    $(SLO)$/linkmgr2.obj\
    $(SLO)$/lnkbase2.obj\
    $(SLO)$/impldde.obj

EXCEPTIONSFILES=\
    $(SLO)$/imagemgr.obj		\
    $(SLO)$/appopen.obj \
    $(SLO)$/appmain.obj			\
    $(SLO)$/appmisc.obj			\
    $(SLO)$/frstinit.obj		\
    $(SLO)$/appinit.obj			\
    $(SLO)$/appcfg.obj			\
    $(SLO)$/helpinterceptor.obj	\
    $(SLO)$/newhelp.obj			\
    $(SLO)$/sfxhelp.obj			\
    $(SLO)$/shutdownicon.obj	\
    $(SLO)$/shutdowniconw32.obj \
    $(SLO)$/sfxpicklist.obj		\
    $(SLO)$/helpdispatch.obj	\
    $(SLO)$/xpackcreator.obj

.IF "$(GUI)" == "MAC"
SLOFILES +=\
        $(SLO)$/appmac.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

