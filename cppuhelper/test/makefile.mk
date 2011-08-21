#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJNAME=cppuhelper
TARGET=testhelper
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/udkapi.rdb
UNOUCROUT=	$(OUT)$/inc$/test
INCPRE+=	$(OUT)$/inc$/test

OBJFILES=	\
        $(OBJ)$/testhelper.obj		\
        $(OBJ)$/testpropshlp.obj 	\
        $(OBJ)$/testidlclass.obj 	\
        $(OBJ)$/testproptyphlp.obj	\
        $(OBJ)$/testimplhelper.obj \
        $(OBJ)$/testcontainer.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES) 

APP1STDLIBS+=	\
        $(CPPULIB)	\
        $(CPPUHELPERLIB) \
        $(SALLIB)

APP1DEF=	$(MISC)$/$(APP1TARGET).def

#############################################

APP2OBJS = $(OBJ)$/testdefaultbootstrapping.obj
APP2STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
APP2TARGET = testdefaultbootstrapping

#############################################

SLOFILES= \
        $(SLO)$/cfg_test.obj
LIB1TARGET=$(SLB)$/cfg_test.lib
LIB1OBJFILES=$(SLOFILES)


APP3OBJS = $(OBJ)$/cfg_test.obj
APP3STDLIBS += $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
APP3TARGET = test_cfg

#############################################

ALLIDLFILES:=	helpertest.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL: $(BIN)$/cpputest.rdb				\
     unoheader 					\
     $(BIN)$/testrc 				\
     $(BIN)$/testdefaultbootstrapping.pl		\
    ALLTAR 						\
     $(BIN)$/$(APP2TARGET).bin  			\
     $(BIN)$/$(APP2TARGET).Bin  			\
     $(BIN)$/$(APP2TARGET).Exe

.IF "$(GUI)"=="UNX"
ALL:  $(BIN)$/$(APP2TARGET).exe

$(BIN)$/$(APP2TARGET).exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

.ENDIF


$(BIN)$/$(APP2TARGET).bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Bin : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

$(BIN)$/$(APP2TARGET).Exe : $(APP2TARGETN)
    cp $(APP2TARGETN) $@

.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

CPPUMAKERFLAGS =
.IF "$(COM)" == "MSC"
CPPUMAKERFLAGS = -L
.ENDIF

TYPES:=test.A;test.BA;test.CA;test.DBA;test.E;test.FE;test.G;test.H;test.I
HELPERTYPES:=com.sun.star.uno.XReference;com.sun.star.uno.XAdapter;com.sun.star.uno.XAggregation;com.sun.star.uno.XWeak;com.sun.star.lang.XComponent;com.sun.star.lang.XTypeProvider;com.sun.star.lang.XEventListener;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XMultiServiceFactory;com.sun.star.registry.XRegistryKey;com.sun.star.lang.XInitialization;com.sun.star.lang.XServiceInfo;com.sun.star.loader.XImplementationLoader;com.sun.star.lang.IllegalAccessException;com.sun.star.beans.XVetoableChangeListener;com.sun.star.beans.XPropertySet;com.sun.star.uno.XComponentContext
FACTORYTYPES:=com.sun.star.lang.XComponent;com.sun.star.registry.XSimpleRegistry;com.sun.star.lang.XInitialization;com.sun.star.lang.XMultiServiceFactory;com.sun.star.loader.XImplementationLoader;com.sun.star.registry.XImplementationRegistration;com.sun.star.container.XSet;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XSingleComponentFactory;com.sun.star.lang.XMultiComponentFactory

.IF "$(OS)" == "WNT"
UNO_PATH := $(SOLARBINDIR)
UNO_OUT := $(OUT)$/bin
EXT_SO  := .dll
.ELSE
UNO_PATH := $(SOLARLIBDIR)
UNO_OUT  := $(OUT)$/lib
EXT_SO   := .so
.ENDIF

$(UNO_OUT)$/%$(EXT_SO): $(UNO_PATH)$/%$(EXT_SO)
    $(COPY) $^ $@

$(BIN)$/cpputest.rdb: $(ALLIDLFILES) $(UNO_OUT)$/invocadapt.uno$(EXT_SO) $(UNO_OUT)$/bootstrap.uno$(EXT_SO)
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $(ALLIDLFILES)
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    $(REGMERGE) $@ / $(UNOUCRRDB)
    $(REGCOMP) -register -r $@ -c reflection.uno$(DLLPOST)
    $(TOUCH) $@

unoheader: $(BIN)$/cpputest.rdb
    $(CPPUMAKER) $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(TYPES);$(HELPERTYPES)" $(BIN)$/cpputest.rdb
    $(CPPUMAKER) $(CPPUMAKERFLAGS) -BUCR -O$(UNOUCROUT) -T"$(FACTORYTYPES)" $(BIN)$/cpputest.rdb


$(BIN)$/testdefaultbootstrapping.pl: testdefaultbootstrapping.pl
    cp testdefaultbootstrapping.pl $@

$(BIN)$/testrc: makefile.mk
.IF "$(GUI)"=="WNT"
    echo [Tests] > $@
    echo TestKey1=com.sun.star.uno.NamingService.rdb >> $@
.ELSE
    echo '[Tests]' > $@
    echo 'TestKey1=file:///'$(PWD)/$(BIN)'/com.sun.star.uno.NamingService.rdb' >> $@
.ENDIF



