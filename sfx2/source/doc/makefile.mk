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

PRJNAME=sfx2
TARGET=doc
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
        doc.src new.src doctdlg.src docvor.src doctempl.src graphhelp.src

SLOFILES =	\
        $(SLO)$/printhelper.obj \
        $(SLO)$/docinf.obj \
        $(SLO)$/oleprops.obj \
        $(SLO)$/iframe.obj \
        $(SLO)$/plugin.obj \
        $(SLO)$/docfile.obj \
        $(SLO)$/objuno.obj \
        $(SLO)$/frmdescr.obj \
        $(SLO)$/objxtor.obj \
        $(SLO)$/objmisc.obj \
        $(SLO)$/objstor.obj \
        $(SLO)$/objcont.obj \
        $(SLO)$/objserv.obj \
        $(SLO)$/objitem.obj \
        $(SLO)$/ownsubfilterservice.obj \
        $(SLO)$/docfac.obj \
        $(SLO)$/docfilt.obj \
        $(SLO)$/doctempl.obj \
        $(SLO)$/doctemplates.obj \
        $(SLO)$/doctemplateslocal.obj \
        $(SLO)$/docvor.obj \
        $(SLO)$/new.obj \
        $(SLO)$/doctdlg.obj \
        $(SLO)$/sfxbasemodel.obj \
        $(SLO)$/guisaveas.obj\
        $(SLO)$/objembed.obj\
        $(SLO)$/graphhelp.obj \
        $(SLO)$/QuerySaveDocument.obj \
        $(SLO)$/docinsert.obj \
        $(SLO)$/docmacromode.obj \
        $(SLO)$/SfxDocumentMetaData.obj \
        $(SLO)$/DocumentMetadataAccess.obj \
        $(SLO)$/Metadatable.obj \
        $(SLO)$/sfxmodelfactory.obj \
        $(SLO)$/sfxacldetect.obj \
        $(SLO)$/docstoragemodifylistener.obj \
        $(SLO)$/querytemplate.obj \
        $(SLO)$/syspath.obj \
        $(SLO)$/syspathw32.obj \
        $(SLO)$/docundomanager.obj

#                $(SLO)$/applet.obj \

.IF "$(GUI)" == "WNT"

#HACK TO DISABLE PCH
NOOPTFILES= \
        $(SLO)$/sfxacldetect.obj \
        $(SLO)$/syspathw32.obj
.ENDIF        


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


