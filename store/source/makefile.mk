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

PRJNAME=store
TARGET=store

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

SLOFILES=	\
    $(SLO)$/object.obj	\
    $(SLO)$/lockbyte.obj	\
    $(SLO)$/storbase.obj	\
    $(SLO)$/storbios.obj	\
    $(SLO)$/storcach.obj	\
    $(SLO)$/stordata.obj	\
    $(SLO)$/stordir.obj	\
    $(SLO)$/storlckb.obj	\
    $(SLO)$/stortree.obj	\
    $(SLO)$/storpage.obj	\
    $(SLO)$/store.obj

.IF "$(debug)" != ""
OBJFILES=	\
    $(OBJ)$/object.obj	\
    $(OBJ)$/lockbyte.obj	\
    $(OBJ)$/storbase.obj	\
    $(OBJ)$/storbios.obj	\
    $(OBJ)$/storcach.obj	\
    $(OBJ)$/stordata.obj	\
    $(OBJ)$/stordir.obj	\
    $(OBJ)$/storlckb.obj	\
    $(OBJ)$/stortree.obj	\
    $(OBJ)$/storpage.obj	\
    $(OBJ)$/store.obj

.ENDIF # debug

# --- Targets ---

.INCLUDE : target.mk

