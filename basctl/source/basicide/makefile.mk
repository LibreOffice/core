#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:06:46 $
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

PRJNAME=basctl
TARGET=basicide

PROJECTPCH4DLL=TRUE
PROJECTPCH=ide_pch
PROJECTPCHSOURCE=ide_pch

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(basicdebug)" != "" || "$(BASICDEBUG)" != ""
CDEFS+=-DBASICDEBUG
.ENDIF


# --- Allgemein ----------------------------------------------------------

SLOFILES =  \
            $(SLO)$/basdoc.obj	\
            $(SLO)$/basicbox.obj	\
            $(SLO)$/basidesh.obj	\
            $(SLO)$/basides1.obj	\
            $(SLO)$/basides2.obj	\
            $(SLO)$/basides3.obj	\
            $(SLO)$/baside2.obj	\
            $(SLO)$/baside2b.obj	\
            $(SLO)$/baside3.obj	\
            $(SLO)$/basobj.obj	\
            $(SLO)$/basobj2.obj	\
            $(SLO)$/basobj3.obj	\
            $(SLO)$/bastypes.obj	\
            $(SLO)$/bastype2.obj	\
            $(SLO)$/bastype3.obj	\
            $(SLO)$/brkdlg.obj	\
            $(SLO)$/iderdll.obj	\
            $(SLO)$/macrodlg.obj	\
            $(SLO)$/moptions.obj	\
            $(SLO)$/moduldlg.obj	\
            $(SLO)$/moduldl2.obj	\
            $(SLO)$/objdlg.obj	\
                $(SLO)$/unomodel.obj	\
                $(SLO)$/register.obj	\
            $(SLO)$/tbxctl.obj	\
            $(SLO)$/basidectrlr.obj

EXCEPTIONSFILES=$(SLO)$/basidesh.obj	\
                $(SLO)$/basides1.obj	\
                $(SLO)$/basides2.obj	\
                $(SLO)$/basides3.obj	\
                $(SLO)$/baside2.obj		\
                $(SLO)$/baside3.obj		\
                $(SLO)$/basobj2.obj		\
                $(SLO)$/basobj3.obj		\
                $(SLO)$/bastypes.obj	\
                $(SLO)$/bastype2.obj	\
                $(SLO)$/bastype3.obj	\
                $(SLO)$/iderdll.obj		\
                $(SLO)$/macrodlg.obj	\
                $(SLO)$/moduldlg.obj	\
                $(SLO)$/moduldl2.obj	\
                $(SLO)$/unomodel.obj	\
                $(SLO)$/register.obj	\
                $(SLO)$/tbxctl.obj		\
                $(SLO)$/basidectrlr.obj

SRS1NAME=$(TARGET)
SRC1FILES=	basidesh.src macrodlg.src moptions.src moduldlg.src objdlg.src brkdlg.src tbxctl.src

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
        echo \#define DLL_NAME \"libbasctl$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
.IF "$(USE_SHELL)"!="4nt"
        echo \#define DLL_NAME \"basctl$(UPD)$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE          # "$(USE_SHELL)"!="4nt"
        echo #define DLL_NAME "basctl$(UPD)$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF          # "$(USE_SHELL)"!="4nt"
.ENDIF

$(SLO)$/basiclib.obj : $(INCCOM)$/dllname.hxx

$(INCCOM)$/basicide.hrc: basidesh.hrc
    @-+$(COPY) basidesh.hrc $@

