#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:33:28 $
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

PRJNAME=			filter
TARGET=				fl_xmlfilteradaptor

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  		settings.mk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=framework

# --- Targets ------------------------------------------------------

SHL1TARGET=		xmlfa$(DLLPOSTFIX)
SHL1IMPLIB=		i$(SHL1TARGET)

SHL1VERSIONMAP= xmlfa.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

SHL1OBJS=		$(SLO)$/genericfilter.obj		\
                $(SLO)$/XmlFilterAdaptor.obj	\
                $(SLO)$/streamwrap.obj

SHL1STDLIBS=	$(COMPHELPERLIB) \
                $(CPPUHELPERLIB) \
                $(CPPULIB) \
                $(TOOLSLIB) \
                $(SALLIB)	

.INCLUDE :			target.mk

