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
# $Revision: 1.34 $
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

PRJNAME=testcppu
TARGET=	testcppu
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=$(OUT)$/inc$/test
INCPRE+=$(OUT)$/inc$/test

OBJFILES= \
        $(OBJ)$/testcppu.obj	\
        $(OBJ)$/test_di.obj	\
        $(OBJ)$/test_Cincludes.obj
#		$(OBJ)$/test_cuno.obj	\
#		$(OBJ)$/test_sec.obj	\

APP1TARGET=	testcppu
APP1OBJS=	\
        $(OBJ)$/testcppu.obj	\
        $(OBJ)$/test_di.obj
#		$(OBJ)$/test_cuno.obj
#		$(OBJ)$/test_sec.obj

APP1STDLIBS+=	\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)

APP1DEF=$(MISC)$/$(APP1TARGET).def

ALLIDLFILES:= \
        cpputest.idl 		\
        language_binding.idl	\
        alignment.idl


APP2TARGET  := Mapping.test
APP2OBJS    := $(OBJ)$/Mapping.test.obj
APP2STDLIBS := $(CPPULIB) $(SALLIB) 

APP3TARGET  := Environment.test
APP3OBJS    := $(OBJ)$/Environment.test.obj
APP3STDLIBS := $(CPPULIB) $(SALLIB) 

APP4TARGET  := IdentityMapping.test
APP4OBJS    := $(OBJ)$/IdentityMapping.test.obj
APP4STDLIBS := $(CPPULIB) $(SALLIB) 



# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : $(BIN)$/testcppu.rdb unoheader ALLTAR
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

CPPUMAKERFLAGS = -L

TYPES:=		-Ttest.XLanguageBindingTest \
        -Ttest.XSimpleInterface \
        -Ttest.Test1 \
        -Ttest.Test2 \
        -Ttest.TdTest1 \
        -Ttest.Test3 \
        -Ttest.Base \
        -Ttest.Base1 \
        -Ttest.Base2 \
        -Tcom.sun.star.lang.XMultiServiceFactory \
        -Tcom.sun.star.lang.XSingleServiceFactory \
        -Tcom.sun.star.lang.XInitialization \
        -Tcom.sun.star.lang.XServiceInfo \
        -Tcom.sun.star.lang.XEventListener \
        -Tcom.sun.star.lang.XTypeProvider \
        -Tcom.sun.star.lang.DisposedException \
        -Tcom.sun.star.registry.XSimpleRegistry \
        -Tcom.sun.star.registry.XRegistryKey \
        -Tcom.sun.star.loader.XImplementationLoader \
        -Tcom.sun.star.registry.XImplementationRegistration \
        -Tcom.sun.star.lang.XComponent \
        -Tcom.sun.star.uno.XComponentContext \
        -Tcom.sun.star.container.XSet \
        -Tcom.sun.star.container.XNameContainer \
        -Tcom.sun.star.uno.TypeClass \
        -Tcom.sun.star.uno.XReference \
        -Tcom.sun.star.uno.XAdapter \
        -Tcom.sun.star.uno.XAggregation \
        -Tcom.sun.star.uno.XWeak \
        -Tcom.sun.star.beans.XPropertySet \
        -Tcom.sun.star.reflection.XIdlClassProvider \
        -Tcom.sun.star.container.XHierarchicalNameAccess \
        -Tcom.sun.star.uno.XCurrentContext

$(BIN)$/testcppu.rdb: $(ALLIDLFILES)
    idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    regmerge $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    regmerge $@ / $(UNOUCRRDB)
    touch $@

# 	regcomp -register -r $@ -c javaloader.dll
# 	regcomp -register -r $@ -c jen.dll

unoheader: $(BIN)$/testcppu.rdb
    cppumaker $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/testcppu.rdb
#	cunomaker -BUCR -O$(UNOUCROUT) $(TYPES) $(BIN)$/testcppu.rdb

