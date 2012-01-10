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
TARGET=dtobjfact
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

#-DUNICODE -D_UNICODE
.IF "$(COM)"!="GCC"
CFLAGS+=-GR -Ob0
.ENDIF

SLOFILES=$(SLO)$/DtObjFactory.obj\
         $(SLO)$/APNDataObject.obj\
         $(SLO)$/DOTransferable.obj\
         $(SLO)$/DTransHelper.obj\
         $(SLO)$/XTDataObject.obj\
         $(SLO)$/TxtCnvtHlp.obj\
         $(SLO)$/DataFmtTransl.obj\
         $(SLO)$/FmtFilter.obj\
         $(SLO)$/FetcList.obj\
         $(SLO)$/Fetc.obj\
         $(SLO)$/XNotifyingDataObject.obj

LIB1TARGET=$(SLB)$/$(TARGET).lib
.IF "$(COM)"!="GCC"
LIB1OBJFILES=$(SLOFILES)
.ELSE
LIB1OBJFILES=$(SLO)$/DtObjFactory.obj\
         $(SLO)$/APNDataObject.obj\
         $(SLO)$/DOTransferable.obj\
         $(SLO)$/DTransHelper.obj\
         $(SLO)$/XTDataObject.obj\
         $(SLO)$/TxtCnvtHlp.obj\
         $(SLO)$/DataFmtTransl.obj\
         $(SLO)$/FmtFilter.obj\
         $(SLO)$/FetcList.obj\
         $(SLO)$/Fetc.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
