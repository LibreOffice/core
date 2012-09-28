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

PRJNAME=bridges
TARGET=test
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
ALLIDLFILES = test_bridge.idl
CPPUMAKERFLAGS += -C


UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb  $(BIN)$/test.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb  $(BIN)$/test.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

# adding to includepath
INCPRE+=$(UNOUCROUT)
CFLAGS += -I..$/source$/remote$/urp

UNOTYPES = \
        com.sun.star.uno.XWeak\
        com.sun.star.uno.XNamingService\
        com.sun.star.uno.XAggregation \
        com.sun.star.uno.TypeClass\
        com.sun.star.io.XInputStream\
        com.sun.star.io.XOutputStream\
        com.sun.star.lang.XInitialization \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.registry.XSimpleRegistry \
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.corba.giop.TargetAddress \
        com.sun.star.corba.giop.TargetAddressGroup \
        com.sun.star.lang.XComponent \
        com.sun.star.bridge.XBridgeFactory\
        com.sun.star.connection.XAcceptor\
        com.sun.star.connection.XConnector\
        com.sun.star.beans.Property\
        com.sun.star.corba.giop.RequestHeader_1_2\
             com.sun.star.container.XSet\
        com.sun.star.lang.XServiceInfo\
        test.XTestFactory \
        com.sun.star.test.performance.XPerformanceTest \
        com.sun.star.lang.XMain \
        com.sun.star.lang.XMultiComponentFactory \
        com.sun.star.lang.XSingleComponentFactory

JARFILES = jurt.jar unoil.jar

OBJFILES=	\
            $(OBJ)$/testserver.obj \
            $(OBJ)$/testclient.obj \
            $(OBJ)$/testcomp.obj  \
            $(OBJ)$/testsameprocess.obj 


APP2TARGET=	testserver
APP2OBJS=	$(OBJ)$/testserver.obj \
        $(OBJ)$/testcomp.obj 

.IF "$(OS)" == "LINUX"
APP2STDLIBS+= -lstdc++
.ENDIF

APP2STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)

APP3TARGET=	testclient
APP3OBJS=	$(OBJ)$/testclient.obj \
        $(OBJ)$/testcomp.obj

.IF "$(OS)" == "LINUX"
APP3STDLIBS+= -lstdc++
.ENDIF

APP3STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)

#----------------------------------

APP4TARGET=	testsameprocess
APP4OBJS=	$(OBJ)$/testsameprocess.obj \
        $(OBJ)$/testcomp.obj

.IF "$(OS)" == "LINUX"
APP4STDLIBS+= -lstdc++
.ENDIF

APP4STDLIBS+=	\
            $(CPPULIB)	\
            $(CPPUHELPERLIB)	\
            $(SALLIB)

APP4DEF=	$(MISC)$/$(APP4TARGET).def

#----------------------------------

# APP5TARGET=	testoffice
# APP5OBJS=	$(OBJ)$/testoffice.obj \
# 		$(OBJ)$/testcomp.obj

# .IF "$(OS)" == "LINUX"
# APP5STDLIBS+= -lstdc++
# .ENDIF

# APP5STDLIBS+=	\
# 			$(CPPULIB)	\
# 			$(CPPUHELPERLIB)\
# 			$(SALLIB)

# APP5DEF=	$(MISC)$/$(APP5TARGET).def


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : 	$(BIN)$/test.rdb	\
    $(BIN)$/server.rdb	\
    $(BIN)$/client.rdb	

$(BIN)$/test.rdb: $(ALLIDLFILES)
    $(IDLC) -I$(PRJ) -I$(SOLARIDLDIR) -O$(BIN) $?
    $(REGMERGE) $@ /UCR $(BIN)$/{$(?:f:s/.idl/.urd/)}
    touch $@

$(BIN)$/client.rdb: $(BIN)$/test.rdb
    rm -f $(BIN)$/client.rdb
    $(REGMERGE) $@ / $(BIN)$/test.rdb $(SOLARBINDIR)$/udkapi.rdb

$(BIN)$/server.rdb: $(BIN)$/test.rdb
    rm -f $(BIN)$/client.rdb
    $(REGMERGE) $@ / $(BIN)$/test.rdb $(SOLARBINDIR)$/udkapi.rdb

