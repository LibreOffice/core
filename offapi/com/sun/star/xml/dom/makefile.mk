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
# $Revision: 1.5 $
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

PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=cssdom
PACKAGE=com$/sun$/star$/xml$/dom

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    DOMException.idl \
    DOMExceptionType.idl \
    NodeType.idl \
    XNode.idl \
    XAttr.idl \
    XCharacterData.idl \
    XText.idl \
    XCDATASection.idl \
    XComment.idl \
    XDOMImplementation.idl \
    XDocument.idl \
    XDocumentBuilder.idl \
    XDocumentFragment.idl \
    XDocumentType.idl \
    XElement.idl \
    XEntity.idl \
    XEntityReference.idl \
    XNamedNodeMap.idl \
    XNodeList.idl \
    XNotation.idl \
    XProcessingInstruction.idl \
    SAXDocumentBuilder.idl \
    XSAXDocumentBuilder.idl \
    SAXDocumentBuilderState.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk






