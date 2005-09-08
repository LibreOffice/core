#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 21:45:34 $
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
PRJNAME=filter
TARGET=PptImporter

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	svpre.mk
.INCLUDE :  	settings.mk
.INCLUDE :  	sv.mk

# --- Types -------------------------------------

UNOTYPES=	com.sun.star.uno.XWeak						\
            com.sun.star.uno.XNamingService				\
            com.sun.star.uno.XComponentContext			\
            com.sun.star.uno.XAggregation				\
            com.sun.star.lang.XServiceInfo				\
            com.sun.star.lang.XSingleServiceFactory		\
            com.sun.star.lang.XMultiServiceFactory		\
            com.sun.star.lang.XSingleComponentFactory	\
            com.sun.star.lang.XTypeProvider				\
            com.sun.star.registry.XSimpleRegistry		\
            com.sun.star.document.XFilter				\
            com.sun.star.document.XImporter				\
            com.sun.star.lang.XInitialization			\
            com.sun.star.io.XInputStream				\
            com.sun.star.xml.sax.XDocumentHandler		\
            com.sun.star.xml.sax.InputSource			\
            com.sun.star.xml.sax.XParser

# --- Files -------------------------------------

CXXFILES=	pptimporter.cxx								\
            pptimporteruno.cxx							\
            ppttoxml.cxx


SLOFILES=	$(SLO)$/pptimporter.obj						\
            $(SLO)$/pptimporteruno.obj					\
            $(SLO)$/ppttoxml.obj

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1STDLIBS=\
            $(SOTLIB)			\
            $(TOOLSLIB)			\
            $(CPPUHELPERLIB)	\
            $(CPPULIB)			\
            $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
