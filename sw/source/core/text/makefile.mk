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
PRJ=..$/..$/..

PRJNAME=sw
TARGET=text

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

.IF "$(ENABLE_GRAPHITE)" == "TRUE"
CFLAGS+=-DENABLE_GRAPHITE
.ENDIF
# --- Files --------------------------------------------------------

SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/txtcache.obj \
        $(SLO)$/txtinit.obj

.IF "$(DBG_LEVEL)">="2"
SLOFILES +=  \
        $(SLO)$/txtio.obj
.ENDIF

EXCEPTIONSFILES = \
        $(SLO)$/EnhancedPDFExportHelper.obj \
        $(SLO)$/SwGrammarMarkUp.obj \
        $(SLO)$/atrstck.obj \
        $(SLO)$/blink.obj \
        $(SLO)$/frmcrsr.obj \
        $(SLO)$/frmform.obj \
        $(SLO)$/frminf.obj \
        $(SLO)$/frmpaint.obj \
        $(SLO)$/guess.obj \
        $(SLO)$/inftxt.obj \
        $(SLO)$/itradj.obj \
        $(SLO)$/itratr.obj \
        $(SLO)$/itrcrsr.obj \
        $(SLO)$/itrform2.obj \
        $(SLO)$/itrpaint.obj \
        $(SLO)$/itrtxt.obj \
        $(SLO)$/noteurl.obj \
        $(SLO)$/porexp.obj \
        $(SLO)$/porfld.obj \
        $(SLO)$/porfly.obj \
        $(SLO)$/porglue.obj \
        $(SLO)$/porlay.obj \
        $(SLO)$/porlin.obj \
        $(SLO)$/pormulti.obj \
        $(SLO)$/porref.obj \
        $(SLO)$/porrst.obj \
        $(SLO)$/portox.obj \
        $(SLO)$/portxt.obj \
        $(SLO)$/redlnitr.obj \
        $(SLO)$/txtdrop.obj \
        $(SLO)$/txtfld.obj \
        $(SLO)$/txtfly.obj \
        $(SLO)$/txtfrm.obj \
        $(SLO)$/txtftn.obj \
        $(SLO)$/txthyph.obj \
        $(SLO)$/txtpaint.obj \
        $(SLO)$/txttab.obj \
        $(SLO)$/widorp.obj \
        $(SLO)$/wrong.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

