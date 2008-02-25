#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.53 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:28:12 $
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
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
CFLAGS+=-Od
CFLAGS+=-DENABLE_QUICKSTART_APPLET
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
        CDEFS+=-DDLL_NAME=libsfx$(DLLPOSTFIX)$(DLLPOST)
.IF "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
        PKGCONFIG_MODULES=gtk+-2.0
        .INCLUDE: pkg_config.mk
        CFLAGS+=$(PKGCONFIG_CFLAGS)
        CFLAGS+=-DENABLE_QUICKSTART_APPLET
        CDEFS+=-DPLUGIN_NAME=libqstart_gtk$(DLLPOSTFIX)$(DLLPOST)
.ENDIF # "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
.ELSE
        CDEFS+=-DDLL_NAME=sfx$(DLLPOSTFIX)$(DLLPOST)
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=appl
SRC1FILES =  \
        app.src newhelp.src dde.src

SRS2NAME=sfx
SRC2FILES =  \
        sfx.src

SFX_OBJECTS = \
    $(SLO)$/app.obj \
    $(SLO)$/appbas.obj \
    $(SLO)$/appcfg.obj \
    $(SLO)$/appchild.obj \
    $(SLO)$/appdata.obj \
    $(SLO)$/appdde.obj \
    $(SLO)$/appinit.obj \
        $(SLO)$/appmain.obj \
    $(SLO)$/appmisc.obj \
    $(SLO)$/appopen.obj \
    $(SLO)$/appquit.obj \
    $(SLO)$/appreg.obj \
    $(SLO)$/appserv.obj \
    $(SLO)$/appuno.obj \
    $(SLO)$/appbaslib.obj \
    $(SLO)$/childwin.obj \
    $(SLO)$/helpdispatch.obj \
    $(SLO)$/helpinterceptor.obj \
    $(SLO)$/imagemgr.obj\
        $(SLO)$/imestatuswindow.obj \
        $(SLO)$/impldde.obj \
        $(SLO)$/linkmgr2.obj \
        $(SLO)$/linksrc.obj \
        $(SLO)$/lnkbase2.obj \
    $(SLO)$/module.obj \
    $(SLO)$/newhelp.obj \
    $(SLO)$/sfxdll.obj \
    $(SLO)$/sfxhelp.obj \
    $(SLO)$/sfxpicklist.obj \
    $(SLO)$/shutdownicon.obj \
    $(SLO)$/shutdowniconw32.obj \
    $(SLO)$/workwin.obj \
        $(SLO)$/xpackcreator.obj \
    $(SLO)$/fwkhelper.obj

.IF "$(GUI)"=="OS2"
SFX_OBJECTS +=  $(SLO)$/shutdowniconOs2.obj
.ENDIF

SLOFILES = $(SFX_OBJECTS)
LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES= $(SFX_OBJECTS)

.IF "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
QUICKSTART_OBJECTS = $(SLO)$/shutdowniconunx.obj
SLOFILES += $(QUICKSTART_OBJECTS)

LIB2TARGET= $(SLB)$/quickstart.lib
LIB2OBJFILES= $(QUICKSTART_OBJECTS)
.ENDIF

EXCEPTIONSFILES=\
    $(SLO)$/imagemgr.obj		\
    $(SLO)$/appopen.obj \
    $(SLO)$/appmain.obj			\
    $(SLO)$/appmisc.obj			\
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


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

