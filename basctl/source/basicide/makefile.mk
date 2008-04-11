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
# $Revision: 1.28 $
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

EXCEPTIONSFILES=$(SLO)$/scriptdocument.obj  \
                $(SLO)$/basidesh.obj	\
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
                $(SLO)$/basidectrlr.obj	\
                $(SLO)$/localizationmgr.obj \
                $(SLO)$/doceventnotifier.obj \
                $(SLO)$/docsignature.obj \
                $(SLO)$/documentenumeration.obj

SLOFILES =  $(EXCEPTIONSFILES) \
            $(SLO)$/basdoc.obj	\
            $(SLO)$/basicbox.obj	\
            $(SLO)$/baside2b.obj	\
            $(SLO)$/basobj.obj	\
            $(SLO)$/brkdlg.obj	\
            $(SLO)$/moptions.obj	\
            $(SLO)$/objdlg.obj	\

SRS1NAME=$(TARGET)
SRC1FILES=	basidesh.src macrodlg.src moptions.src moduldlg.src objdlg.src brkdlg.src tbxctl.src

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
        echo \#define DLL_NAME \"libbasctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
.IF "$(USE_SHELL)"!="4nt"
        echo \#define DLL_NAME \"basctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE          # "$(USE_SHELL)"!="4nt"
        echo #define DLL_NAME "basctl$(DLLPOSTFIX)$(DLLPOST)" >$@
.ENDIF          # "$(USE_SHELL)"!="4nt"
.ENDIF

$(SLO)$/basiclib.obj : $(INCCOM)$/dllname.hxx

$(INCCOM)$/basicide.hrc: basidesh.hrc
    @-$(COPY) basidesh.hrc $@

