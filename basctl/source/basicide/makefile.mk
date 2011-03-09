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

PRJNAME=basctl
TARGET=basicide
VISIBILITY_HIDDEN=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=ide_pch
PROJECTPCHSOURCE=ide_pch

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(basicdebug)" != "" || "$(BASICDEBUG)" != ""
CDEFS+=-DBASICDEBUG
.ENDIF

# --- Allgemein ----------------------------------------------------------

EXCEPTIONSFILES= \
                $(SLO)$/basdoc.obj		\
                $(SLO)$/basicbox.obj		\
                $(SLO)$/basicrenderable.obj	\
                $(SLO)$/baside2.obj		\
                $(SLO)$/baside2b.obj		\
                $(SLO)$/baside3.obj		\
                $(SLO)$/basidectrlr.obj		\
                $(SLO)$/basides1.obj		\
                $(SLO)$/basides2.obj		\
                $(SLO)$/basides3.obj		\
                $(SLO)$/basidesh.obj		\
                $(SLO)$/basobj2.obj		\
                $(SLO)$/basobj3.obj		\
                $(SLO)$/bastype2.obj		\
                $(SLO)$/bastype3.obj		\
                $(SLO)$/bastypes.obj		\
                $(SLO)$/brkdlg.obj		\
                $(SLO)$/doceventnotifier.obj	\
                $(SLO)$/docsignature.obj	\
                $(SLO)$/documentenumeration.obj	\
                $(SLO)$/iderdll.obj		\
                $(SLO)$/localizationmgr.obj	\
                $(SLO)$/macrodlg.obj		\
                $(SLO)$/moduldl2.obj		\
                $(SLO)$/moduldlg.obj		\
                $(SLO)$/objdlg.obj		\
                $(SLO)$/register.obj		\
                $(SLO)$/scriptdocument.obj	\
                $(SLO)$/tbxctl.obj		\
                $(SLO)$/unomodel.obj

SLOFILES =  $(EXCEPTIONSFILES)

SRS1NAME=$(TARGET)
SRC1FILES=	basidesh.src macrodlg.src moptions.src moduldlg.src objdlg.src brkdlg.src basicprint.src

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
        echo \#define DLL_NAME \"libbasctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
        echo \#define DLL_NAME \"basctl$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ENDIF

$(SLO)$/basiclib.obj : $(INCCOM)$/dllname.hxx

$(INCCOM)$/basicide.hrc: basidesh.hrc
    @-$(COPY) basidesh.hrc $@

