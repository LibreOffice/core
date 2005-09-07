#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 18:42:43 $
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

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building javaunohelper because Java is disabled"
.ENDIF

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
        $(SLO)$/preload.obj \
        $(SLO)$/vm.obj

# ------------------------------------------------------------------

LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=\
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj \
        $(SLO)$/vm.obj

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

