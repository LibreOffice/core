#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 12:47:42 $
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
PRJNAME=filter
TARGET=fl_javafilter_binaries

.INCLUDE: settings.mk

all: \
    $(BIN)$/docbooktosoffheadings.xsl   \
    $(BIN)$/sofftodocbookheadings.xsl   \
    $(BIN)$/DocBookTemplate.stw
    


$(OUT)$/bin$/docbooktosoffheadings.xsl: docbooktosoffheadings.xsl
         $(COPY) docbooktosoffheadings.xsl $(OUT)$/bin$/docbooktosoffheadings.xsl

$(OUT)$/bin$/sofftodocbookheadings.xsl: sofftodocbookheadings.xsl
         $(COPY) sofftodocbookheadings.xsl $(OUT)$/bin$/sofftodocbookheadings.xsl

$(OUT)$/bin$/DocBookTemplate.stw: DocBookTemplate.stw
         $(COPY) DocBookTemplate.stw $(OUT)$/bin$/DocBookTemplate.stw

.INCLUDE: target.mk

