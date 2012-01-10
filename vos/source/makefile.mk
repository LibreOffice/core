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

PRJNAME=vos
TARGET=cppvos
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=       $(SLO)$/conditn.obj     \
                $(SLO)$/mutex.obj       \
                $(SLO)$/object.obj      \
                $(SLO)$/refernce.obj    \
                $(SLO)$/socket.obj      \
                $(SLO)$/thread.obj      \
                $(SLO)$/stream.obj      \
                $(SLO)$/module.obj      \
                $(SLO)$/timer.obj       \
                $(SLO)$/process.obj     \
                $(SLO)$/security.obj    \
                $(SLO)$/signal.obj      \
                $(SLO)$/pipe.obj        \
                $(SLO)$/xception.obj


.IF "$(UPDATER)"=="YES"
OBJFILES=       $(OBJ)$/conditn.obj     \
                $(OBJ)$/mutex.obj       \
                $(OBJ)$/object.obj      \
                $(OBJ)$/refernce.obj    \
                $(OBJ)$/socket.obj      \
                $(OBJ)$/thread.obj      \
                $(OBJ)$/stream.obj      \
                $(OBJ)$/module.obj      \
                $(OBJ)$/timer.obj       \
                $(OBJ)$/process.obj     \
                $(OBJ)$/security.obj    \
                $(OBJ)$/signal.obj      \
                $(OBJ)$/pipe.obj        \
                $(OBJ)$/xception.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



