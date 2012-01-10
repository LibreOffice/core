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
PRJNAME=extensions
TARGET=msidl

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT" && "$(DISABLE_ACTIVEX)"==""

# --- Targets ----------------------------------

ALL : $(MISC)$/activex_component

MTL=midl.exe
MTL_PROJ=

SOURCE=so_activex.idl

MTL_SWITCHES=-tlb $(MISC)$/so_activex.tlb -h $(MISC)$/so_activex.h -iid $(MISC)$/so_activex_i.c -dlldata $(MISC)$/so_activex_dll.c -proxy $(MISC)$/so_activex_p.c -Oicf

.IF "$(COM)"=="GCC"
MTL_SWITCHES+=-cpp_cmd gcc -cpp_opt "-x c -E -D__stdcall= -D_stdcall= -nostdinc -I $(shell @cygpath -w $(PSDK_HOME)/include)"

$(MISC)$/activex_component : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE) 
.ELSE
$(MISC)$/activex_component : $(SOURCE) 
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) && $(TOUCH) $@
.ENDIF

.ENDIF

.INCLUDE : target.mk

