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

PRJNAME=dtrans
TARGET=dtutils
LIBTARGET=NO
USE_BOUNDCHK=
ENABLE_EXCEPTIONS=YES

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------

.INCLUDE :  settings.mk

#-------------------------------------------
# files to compile to obj's

SLOFILES= $(SLO)$/ImplHelper.obj

#-------------------------------------------
# build a static library

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)


# --- Targets ------------------------------

.INCLUDE :	target.mk

