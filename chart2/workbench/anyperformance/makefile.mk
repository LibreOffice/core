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
# $Revision: 1.4 $
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
PRJ=..

PRJNAME=anyperformance
TARGET=	anyperformance
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=$(OUT)$/inc$/test
INCPRE+=$(OUT)$/inc$/test

OBJFILES= \
        $(OBJ)$/anyperformance.obj

APP1TARGET=	anyperformance
APP1OBJS=	\
        $(OBJ)$/anyperformance.obj

APP1STDLIBS+=	\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)

APP1DEF=$(MISC)$/$(APP1TARGET).def

#ALLIDLFILES:= \
#		cpputest.idl 		\
#		language_binding.idl	\
#		alignment.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
#ALL : $(BIN)$/anyperformance.rdb unoheader ALLTAR
ALL : ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.IF "$(COM)" == "MSC"
.IF "$(debug)" != ""
CFLAGS += /Ob0
.ENDIF
.ENDIF

.IF "$(extra_mapping)" != ""
CFLAGS += -DEXTRA_MAPPING
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS = -C
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

#TYPES:=		-Ttest.XLanguageBindingTest \
#		-Ttest.XSimpleInterface \
#		-Ttest.Test1 \
#		-Ttest.Test2 \
#		-Ttest.TdTest1 \
#		-Ttest.Test3 \
#		-Ttest.Base \
#		-Ttest.Base1 \
#		-Ttest.Base2 \
# 		-Tcom.sun.star.lang.XMultiServiceFactory \
# 		-Tcom.sun.star.lang.XSingleServiceFactory \
# 		-Tcom.sun.star.lang.XInitialization \
# 		-Tcom.sun.star.lang.XServiceInfo \
# 		-Tcom.sun.star.lang.XEventListener \
# 		-Tcom.sun.star.lang.XTypeProvider \
# 		-Tcom.sun.star.registry.XSimpleRegistry \
# 		-Tcom.sun.star.registry.XRegistryKey \
# 		-Tcom.sun.star.loader.XImplementationLoader \
# 		-Tcom.sun.star.registry.XImplementationRegistration \
# 		-Tcom.sun.star.lang.XComponent \
# 		-Tcom.sun.star.uno.XComponentContext \
# 		-Tcom.sun.star.container.XSet \
# 		-Tcom.sun.star.container.XNameContainer \
# 		-Tcom.sun.star.uno.TypeClass \
# 		-Tcom.sun.star.uno.XReference \
# 		-Tcom.sun.star.uno.XAdapter \
# 		-Tcom.sun.star.uno.XAggregation \
# 		-Tcom.sun.star.uno.XWeak \
# 		-Tcom.sun.star.beans.XPropertySet \
# 		-Tcom.sun.star.reflection.XIdlClassProvider \
# 		-Tcom.sun.star.container.XHierarchicalNameAccess \
# 		-Tcom.sun.star.uno.XCurrentContext

$(BIN)$/anyperformance.rdb: $(ALLIDLFILES)
    idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    regmerge $@ / $(UNOUCRRDB)
    touch $@

# 	regcomp -register -r $@ -c javaloader.dll
# 	regcomp -register -r $@ -c jen.dll

unoheader: $(BIN)$/anyperformance.rdb
    cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/anyperformance.rdb
#	cunomaker -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/anyperformance.rdb

