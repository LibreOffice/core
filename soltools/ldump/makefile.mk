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



PRJ=..

PRJNAME=ldump
TARGET=ldump
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE : $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

# --- Files --------------------------------------------------------

# ldump only supports windows environment
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
.IF "$(COM)"!="GCC"
#ldump4 reimplements feature set of ldump2 and ldump3
APP1TARGET=	ldump4
.IF "$(GUI)"=="WNT"
APP1STACK=	16000
.ENDIF
APP1OBJS=   $(OBJ)$/ldump.obj $(OBJ)$/hashtbl.obj

.ENDIF #"$(COM)"!="GCC"
.ENDIF #"$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
