#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2003-10-16 15:23:11 $
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

PRJ = ..$/..
PRJNAME = cli_ure

TARGET = climaker
TARGETTYPE = CUI
LIBTARGET = NO
ENABLE_EXCEPTIONS = TRUE

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(BUILD_FOR_CLI)" != ""

UNOUCRDEP = $(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB = $(SOLARBINDIR)$/udkapi.rdb
CPPUMAKERFLAGS =

UNOTYPES = \
    com.sun.star.uno.TypeClass					\
    com.sun.star.uno.XAggregation					\
    com.sun.star.uno.XWeak						\
    com.sun.star.uno.XComponentContext				\
    com.sun.star.lang.XTypeProvider					\
    com.sun.star.lang.XInitialization				\
    com.sun.star.lang.XComponent					\
    com.sun.star.lang.XMultiComponentFactory			\
    com.sun.star.lang.XMultiServiceFactory			\
    com.sun.star.container.XSet					\
    com.sun.star.container.XHierarchicalNameAccess			\
    com.sun.star.loader.XImplementationLoader			\
    com.sun.star.registry.XSimpleRegistry				\
    com.sun.star.registry.XRegistryKey				\
    com.sun.star.reflection.XTypeDescriptionEnumerationAccess	\
    com.sun.star.reflection.XConstantTypeDescription		\
    com.sun.star.reflection.XConstantsTypeDescription		\
    com.sun.star.reflection.XIndirectTypeDescription		\
    com.sun.star.reflection.XEnumTypeDescription			\
    com.sun.star.reflection.XInterfaceTypeDescription		\
    com.sun.star.reflection.XInterfaceMethodTypeDescription		\
    com.sun.star.reflection.XInterfaceAttributeTypeDescription	\
    com.sun.star.reflection.XCompoundTypeDescription		\
    com.sun.star.reflection.XEnumTypeDescription

.IF "$(COMEX)"=="10"
CFLAGS += /clr:noAssembly,initialAppDomain
.ELSE
CFLAGS += /clr:noAssembly
.ENDIF

OBJFILES = \
    $(OBJ)$/climaker_emit.obj	\
    $(OBJ)$/climaker_app.obj

APP1TARGET = $(TARGET)
APP1OBJS = $(OBJFILES)

APP1STDLIBS = \
    $(CPPUHELPERLIB)		\
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

ALL : ALLTAR $(BIN)$/climaker.exe.config

.ENDIF

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE :  target.mk

$(BIN)$/climaker.exe.config : climaker.exe.config
    $(GNUCOPY) -f $? $@
