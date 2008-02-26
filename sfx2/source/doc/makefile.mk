#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.30 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 15:08:50 $
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
TARGET=doc
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
        doc.src new.src doctdlg.src docvor.src doctempl.src sfxbasemodel.src graphhelp.src

SLOFILES =	\
        $(SLO)$/printhelper.obj \
        $(SLO)$/docinf.obj \
        $(SLO)$/oleprops.obj \
        $(SLO)$/iframe.obj \
        $(SLO)$/applet.obj \
        $(SLO)$/commitlistener.obj \
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
        $(SLO)$/attributelist.obj \
        $(SLO)$/docvor.obj \
        $(SLO)$/new.obj \
        $(SLO)$/doctdlg.obj \
        $(SLO)$/sfxbasemodel.obj \
        $(SLO)$/stormodifylistener.obj \
        $(SLO)$/guisaveas.obj\
        $(SLO)$/objembed.obj\
        $(SLO)$/graphhelp.obj \
        $(SLO)$/QuerySaveDocument.obj \
        $(SLO)$/opostponedtruncationstream.obj \
        $(SLO)$/docinsert.obj \
        $(SLO)$/docmacromode.obj \
        $(SLO)$/SfxDocumentMetaData.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


