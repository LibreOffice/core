#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2007-10-09 15:26:12 $
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

PRJ=..$/..$/..
PRJNAME=ucb
# Version
UCPTDOC_MAJOR=1
TARGET=ucptdoc$(UCPTDOC_MAJOR).uno
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------

# no "lib" prefix
DLLPRE =

SLOFILES=\
    $(SLO)$/tdoc_provider.obj	\
    $(SLO)$/tdoc_services.obj	\
    $(SLO)$/tdoc_uri.obj		\
    $(SLO)$/tdoc_content.obj	\
    $(SLO)$/tdoc_contentcaps.obj \
    $(SLO)$/tdoc_storage.obj	\
    $(SLO)$/tdoc_docmgr.obj		\
    $(SLO)$/tdoc_datasupplier.obj \
    $(SLO)$/tdoc_resultset.obj  \
    $(SLO)$/tdoc_documentcontentfactory.obj \
    $(SLO)$/tdoc_passwordrequest.obj \
    $(SLO)$/tdoc_stgelems.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=	exports.map

SHL1STDLIBS=\
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=$(TARGET)
.ENDIF

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

