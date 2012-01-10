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

PRJNAME=cosv
TARGET=cosv_storage

ENABLE_EXCEPTIONS=true



# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.INCLUDE : $(PRJ)$/source$/fullcpp.mk




# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/dirchain.obj 	\
    $(OBJ)$/file.obj 		\
    $(OBJ)$/mbstream.obj    \
    $(OBJ)$/persist.obj	    \
    $(OBJ)$/ploc.obj 	    \
    $(OBJ)$/ploc_dir.obj    \
    $(OBJ)$/plocroot.obj

#SLOFILES= \
#	$(SLO)$/file.obj 		\
#	$(SLO)$/csfileim.obj 	\
#	$(SLO)$/memstorg.obj



# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



