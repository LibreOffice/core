#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: vg $ $Date: 2003-04-15 13:40:17 $
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
PRJ=..

PRJNAME=javaunohelper
TARGET=juh
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc$/comprehensive
INCPRE+=$(OUT)$/inc$/comprehensive
CPPUMAKERFLAGS+=-C

UNOTYPES= \
        com.sun.star.container.XHierarchicalNameAccess		\
        com.sun.star.loader.XImplementationLoader		\
        com.sun.star.registry.XRegistryKey			\
        com.sun.star.registry.XSimpleRegistry			\
        com.sun.star.beans.XPropertySet				\
        com.sun.star.lang.IllegalArgumentException		\
        com.sun.star.lang.XTypeProvider				\
        com.sun.star.lang.XServiceInfo				\
        com.sun.star.lang.XMultiServiceFactory			\
        com.sun.star.lang.XMultiComponentFactory		\
        com.sun.star.lang.XSingleServiceFactory			\
        com.sun.star.lang.XSingleComponentFactory   		\
        com.sun.star.uno.TypeClass				\
        com.sun.star.uno.XWeak					\
        com.sun.star.uno.XAggregation				\
            com.sun.star.uno.XComponentContext          		\
        com.sun.star.lang.XInitialization           		\
        com.sun.star.lang.XComponent

SLOFILES= \
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj					\
        $(SLO)$/preload.obj

# ------------------------------------------------------------------

LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=\
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj

SHL1TARGET=juhx

SHL1STDLIBS= \
        $(JVMACCESSLIB)		\
        $(SALHELPERLIB)		\
        $(SALLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)

SHL1VERSIONMAP = javaunohelper.map

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# ------------------------------------------------------------------

LIB2TARGET=$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=\
        $(SLO)$/preload.obj

SHL2TARGET=juh

SHL2STDLIBS= \
        $(SALLIB)

SHL2VERSIONMAP = javaunohelper.map

SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
