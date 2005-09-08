#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 20:28:55 $
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
PRJNAME=svx
TARGET=customshapes
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Types -------------------------------------

# Disable optimization for SunCC SPARC (funny loops
# when parsing e.g. "x+width/2")
.IF "$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC"
NOOPTFILES= $(SLO)$/EnhancedCustomShapeFunctionParser.obj
.ENDIF
# same issue for MACOSX
.IF "$(OS)"=="MACOSX"
NOOPTFILES= $(SLO)$/EnhancedCustomShapeFunctionParser.obj
.ENDIF

UNOTYPES=	com.sun.star.uno.XWeak									\
            com.sun.star.uno.XNamingService							\
            com.sun.star.uno.XComponentContext						\
            com.sun.star.uno.XAggregation							\
            com.sun.star.lang.XServiceInfo							\
            com.sun.star.lang.XSingleServiceFactory					\
            com.sun.star.lang.XMultiServiceFactory					\
            com.sun.star.lang.XSingleComponentFactory				\
            com.sun.star.lang.XTypeProvider							\
            com.sun.star.lang.XInitialization						\
            com.sun.star.registry.XSimpleRegistry					\
            com.sun.star.util.logging.XLogger						\
            com.sun.star.util.logging.LogLevel						\
            com.sun.star.util.XTextSearch							\
            com.sun.star.util.SearchResult							\
            com.sun.star.xml.sax.XDocumentHandler					\
            com.sun.star.drawing.XCustomShapeEngine			\
            com.sun.star.drawing.XCustomShapeHandle


# --- Files -------------------------------------

SLOFILES=	$(SLO)$/EnhancedCustomShapeEngine.obj			\
            $(SLO)$/EnhancedCustomShapeTypeNames.obj		\
            $(SLO)$/EnhancedCustomShapeGeometry.obj			\
            $(SLO)$/EnhancedCustomShape2d.obj				\
            $(SLO)$/EnhancedCustomShape3d.obj				\
            $(SLO)$/EnhancedCustomShapeFontWork.obj			\
            $(SLO)$/EnhancedCustomShapeHandle.obj			\
            $(SLO)$/EnhancedCustomShapeFunctionParser.obj	\
            $(SLO)$/tbxcustomshapes.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
