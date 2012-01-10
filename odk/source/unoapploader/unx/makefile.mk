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

PRJNAME=odk
TARGET=unoapploader
LIBTARGET=NO

LIBSALCPPRT=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
LINK=$(CC)
.ENDIF

.IF "$(OS)"!="FREEBSD"
STDLIB= -ldl
.ENDIF


APP1NOSAL=TRUE
APP1RPATH=NONE

APP1TARGET= $(TARGET)

APP1OBJS=   $(OBJ)$/unoapploader.obj $(SOLARLIBDIR)$/findsofficepath.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
