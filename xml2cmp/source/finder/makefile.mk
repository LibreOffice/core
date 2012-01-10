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



PRJ=..$/..

PRJNAME=xml2cmp
TARGET=srvdepy
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk



# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/dependy.obj	\
    $(OBJ)$/dep_main.obj


# --- Targets ------------------------------------------------------

APP1TARGET=	$(TARGET)
.IF "$(GUI)"!="OS2"
APP1STACK=	1000000
.ENDIF

LIBSALCPPRT=$(0)

.IF "$(GUI)"=="WNT"
UWINAPILIB=$(0)
.ENDIF


APP1LIBS=	$(LB)$/$(TARGET).lib $(LB)$/x2c_xcdl.lib $(LB)$/x2c_support.lib
APP1DEPN=	$(LB)$/$(TARGET).lib $(LB)$/x2c_xcdl.lib $(LB)$/x2c_support.lib


.INCLUDE :  target.mk


