#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..

PRJNAME=sal
TARGET=qa_osl_socket

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP1OBJS=  \
    $(SLO)$/osl_Socket.obj

APP1TARGET=osl_SocketOld
APP1STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP1STDLIBS+= $(WS2_32LIB)
.ENDIF

.IF ""$(OS)"=="SOLARIS"
APP1STDLIBS+= -ldl -lnsl
.ENDIF

APP1RPATH = NONE
APP1TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
APP2OBJS=  \
    $(SLO)$/osl_Socket_tests.obj
APP2TARGET= osl_Socket_tests
APP2STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP2RPATH = NONE
APP2TEST = enabled
# END --------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP3OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_StreamSocket.obj

APP3TARGET= osl_StreamSocket
APP3STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP3STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP3STDLIBS += -ldl -lnsl
.ENDIF

APP3RPATH = NONE
APP3TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP4OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_DatagramSocket.obj

APP4TARGET= osl_DatagramSocket
APP4STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP4STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP4STDLIBS += -ldl -lnsl
.ENDIF

APP4RPATH = NONE
APP4TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP5OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_SocketAddr.obj

APP5TARGET= osl_SocketAddr
APP5STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP5STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP5STDLIBS += -ldl -lnsl
.ENDIF

APP5RPATH = NONE
APP5TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP6OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_Socket2.obj

APP6TARGET= osl_Socket2
APP6STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP6STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP6STDLIBS += -ldl -lnsl
.ENDIF

APP6RPATH = NONE
APP6TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP7OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_ConnectorSocket.obj

APP7TARGET= osl_ConnectorSocket
APP7STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP7STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP7STDLIBS += -ldl -lnsl
.ENDIF

APP7RPATH = NONE
APP7TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:Socket by codegen.pl
APP8OBJS=  \
    $(SLO)$/sockethelper.obj \
    $(SLO)$/osl_AcceptorSocket.obj

APP8TARGET= osl_AcceptorSocket
APP8STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
APP8STDLIBS += $(WS2_32LIB)
.ENDIF

.IF "$(OS)"=="SOLARIS"
APP8STDLIBS += -ldl -lnsl
.ENDIF

APP8PATH = NONE
APP8TEST = enabled
# auto generated Target:Socket
# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"