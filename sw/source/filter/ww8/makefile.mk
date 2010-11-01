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
TARGET=ww8

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
MAKING_LIBMSWORD=TRUE
.INCLUDE :	$(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = \
        $(SLO)$/wrtww8gr.obj \
        $(SLO)$/ww8par2.obj \
        $(SLO)$/ww8par3.obj \
        $(SLO)$/ww8par5.obj \
        $(SLO)$/tracer.obj \
        $(SLO)$/fields.obj \
        $(SLO)$/styles.obj \
        $(SLO)$/ww8graf.obj \
        $(SLO)$/ww8graf2.obj \
        $(SLO)$/wrtw8esh.obj \
        $(SLO)$/wrtw8nds.obj \
        $(SLO)$/wrtw8num.obj \
        $(SLO)$/wrtw8sty.obj \
        $(SLO)$/wrtww8.obj \
        $(SLO)$/docxattributeoutput.obj \
        $(SLO)$/docxexportfilter.obj \
        $(SLO)$/docxexport.obj \
        $(SLO)$/ww8atr.obj \
        $(SLO)$/ww8par.obj \
        $(SLO)$/ww8par6.obj \
        $(SLO)$/writerhelper.obj \
        $(SLO)$/writerwordglue.obj \
        $(SLO)$/ww8scan.obj \
        $(SLO)$/WW8TableInfo.obj \
        $(SLO)$/WW8FFData.obj \
        $(SLO)$/WW8Sttbf.obj \
        $(SLO)$/ww8toolbar.obj \
        $(SLO)$/WW8FibData.obj \
        $(SLO)$/rtfexportfilter.obj \
        $(SLO)$/rtfimportfilter.obj \
        $(SLO)$/rtfattributeoutput.obj \
        $(SLO)$/rtfsdrexport.obj \
        $(SLO)$/rtfexport.obj


SLOFILES =	\
        $(SLO)$/wrtw8esh.obj \
        $(SLO)$/wrtw8nds.obj \
        $(SLO)$/wrtw8num.obj \
        $(SLO)$/wrtw8sty.obj \
        $(SLO)$/wrtww8.obj \
        $(SLO)$/wrtww8gr.obj \
        $(SLO)$/docxattributeoutput.obj \
        $(SLO)$/docxexportfilter.obj \
        $(SLO)$/docxexport.obj \
        $(SLO)$/ww8atr.obj \
        $(SLO)$/ww8graf.obj \
        $(SLO)$/ww8graf2.obj \
        $(SLO)$/ww8par.obj \
        $(SLO)$/ww8par2.obj \
        $(SLO)$/ww8par3.obj \
        $(SLO)$/ww8par4.obj \
        $(SLO)$/ww8par5.obj \
        $(SLO)$/ww8par6.obj \
        $(SLO)$/ww8glsy.obj \
        $(SLO)$/tracer.obj \
        $(SLO)$/fields.obj \
        $(SLO)$/styles.obj \
        $(SLO)$/ww8scan.obj \
        $(SLO)$/writerhelper.obj \
        $(SLO)$/writerwordglue.obj \
        $(SLO)$/WW8TableInfo.obj \
        $(SLO)$/WW8FFData.obj \
        $(SLO)$/WW8Sttbf.obj \
        $(SLO)$/ww8toolbar.obj \
        $(SLO)$/WW8FibData.obj \
        $(SLO)$/rtfexportfilter.obj \
        $(SLO)$/rtfimportfilter.obj \
        $(SLO)$/rtfattributeoutput.obj \
        $(SLO)$/rtfsdrexport.obj \
        $(SLO)$/rtfexport.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk

