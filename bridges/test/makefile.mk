#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 14:40:38 $
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

