#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
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

.IF "$(OS)"!="WNT"
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
.IF "$(OS)"=="WNT"
    echo [Tests] > $@
    echo TestKey1=com.sun.star.uno.NamingService.rdb >> $@
.ELSE
    echo '[Tests]' > $@
    echo 'TestKey1=file:///'$(PWD)/$(BIN)'/com.sun.star.uno.NamingService.rdb' >> $@
.ENDIF



