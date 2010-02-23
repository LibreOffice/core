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

# adding to inludeoath
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

