#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: ganaya $ $Date: 2000-10-09 17:17:42 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
UNOUCRDEP=	$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/applicat.rdb

UNOUCROUT=	$(OUT)$/inc
INCPRE+=	$(OUT)$/inc

UNOTYPES= \
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.container.XHierarchicalNameAccess	\
        com.sun.star.reflection.XInterfaceTypeDescription	\
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

# NETBSD: somewhere we have to instantiate the static data members.
# NETBSD-1.2.1 doesn't know about weak symbols so the default mechanism for GCC won't work.
# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="NETBSD" || "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"==
"MACOSX"
CXXFILES+=staticmbrdbmaker.cxx
.ENDIF


APP1TARGET= $(TARGET)

APP1OBJS=   $(OBJ)$/rdbmaker.obj	\
            $(OBJ)$/rdboptions.obj	\
            $(OBJ)$/typeblop.obj	\
            $(OBJ)$/specialtypemanager.obj	\
            $(OBJ)$/rdbtype.obj

# NETBSD: somewhere we have to instantiate the static data members.
# NETBSD-1.2.1 doesn't know about weak symbols so the default mechanism for GCCwon't work.
# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="NETBSD" || "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
APP1OBJS+=$(OBJ)$/staticmbrdbmaker.obj
.ENDIF


APP1STDLIBS=\
            $(SALLIB) \
            $(VOSLIB) \
            $(REGLIB) \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(STDLIBCPP)

.IF "$(GUI)"=="WNT"
APP1STDLIBS+= \
            $(LIBCIMT) $(LIBCMT)
.ENDIF

APP1LIBS= \
        $(LB)$/codemaker.lib

.INCLUDE :  target.mk
