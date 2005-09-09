#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 05:04:59 $
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

PRJNAME=codemaker
TARGET=rdbmaker
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------
UNOUCRDEP=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=	$(OUT)$/inc
INCPRE+=	$(OUT)$/inc

UNOTYPES= \
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.container.XHierarchicalNameAccess	\
        com.sun.star.beans.XPropertySet	\
        com.sun.star.uno.XComponentContext	\
        com.sun.star.reflection.XInterfaceTypeDescription	\
        com.sun.star.reflection.XModuleTypeDescription	\
        com.sun.star.reflection.XConstantTypeDescription	\
        com.sun.star.reflection.XConstantsTypeDescription	\
        com.sun.star.reflection.XInterfaceMemberTypeDescription	\
        com.sun.star.reflection.XInterfaceMethodTypeDescription	\
        com.sun.star.reflection.XInterfaceAttributeTypeDescription	\
        com.sun.star.reflection.XCompoundTypeDescription	\
        com.sun.star.reflection.XIndirectTypeDescription	\
        com.sun.star.reflection.XEnumTypeDescription	\
        com.sun.star.registry.XRegistryKey	\

CXXFILES=	rdbmaker.cxx	\
            rdboptions.cxx	\
            typeblop.cxx	\
            specialtypemanager.cxx	\
            rdbtype.cxx

APP1TARGET= $(TARGET)

APP1OBJS=   $(OBJ)$/rdbmaker.obj	\
            $(OBJ)$/rdboptions.obj	\
            $(OBJ)$/typeblop.obj	\
            $(OBJ)$/specialtypemanager.obj	\
            $(OBJ)$/rdbtype.obj

APP1STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB)

APP1LIBS= \
        $(LB)$/codemaker.lib

.INCLUDE :  target.mk
