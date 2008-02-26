#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 14:32:53 $
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

PRJ=..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=csssax
PACKAGE=com$/sun$/star$/xml$/sax

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    InputSource.idl\
    SAXException.idl\
    SAXInvalidCharacterException.idl\
    SAXParseException.idl\
    XAttributeList.idl\
    XDocumentHandler.idl\
    XDTDHandler.idl\
    XEntityResolver.idl\
    XErrorHandler.idl\
    XExtendedDocumentHandler.idl\
    XLocator.idl\
    XParser.idl\
    XSAXSerializable.idl\
    XFastParser.idl\
    XFastDocumentHandler.idl\
    XFastContextHandler.idl\
    XFastShapeContextHandler.idl\
    XFastTokenHandler.idl\
    XFastAttributeList.idl\
    FastToken.idl \
    FastShapeContextHandler.idl \
    FastTokenHandler.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
