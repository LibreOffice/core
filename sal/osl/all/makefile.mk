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

PRJNAME=sal
TARGET=oslall
ENABLE_EXCEPTIONS=TRUE
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

MULTITHREAD_OBJ=TRUE

.IF "$(GUI)" == "OS2"
STL_OS2_BUILDING=1
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES=	\
            $(SLO)$/utility.obj\
            $(SLO)$/filepath.obj\
            $(SLO)$/debugbase.obj\
            $(SLO)$/loadmodulerelative.obj \
            $(SLO)/printtrace.obj

#			$(SLO)$/readline.obj\

#.IF "$(UPDATER)"=="YES"
OBJFILES=	\
            $(OBJ)$/utility.obj\
            $(OBJ)$/filepath.obj\
            $(OBJ)$/debugbase.obj\
            $(OBJ)$/loadmodulerelative.obj \
            $(OBJ)/printtrace.obj

#			$(OBJ)$/readline.obj\
#.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


