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
PRJNAME=winaccessibility
TARGET=UAccComIDL

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(GUI)" == "WNT"

# --- Targets ----------------------------------

ALL : $(MISC)$/ia2_api_all \
      $(MISC)$/AccessibleKeyBinding \
      $(MISC)$/AccessibleKeyStroke \
      $(MISC)$/Charset \
      $(MISC)$/defines \
      $(MISC)$/UAccCOM

MTL=midl.exe
MTL_PROJ=

SOURCE=AccessibleKeyBinding.idl
SOURCE1=AccessibleKeyStroke.idl
SOURCE2=Charset.idl
SOURCE3=defines.idl
SOURCE4=UAccCOM.idl
SOURCE5=ia2_api_all.idl

MTL_SWITCHES=-tlb $(MISC)$/AccessibleKeyBinding.tlb -h $(MISC)$/AccessibleKeyBinding.h -iid $(MISC)$/AccessibleKeyBinding_i.c -proxy $(MISC)$/AccessibleKeyBinding_p.c -Oicf
MTL_SWITCHES1=-tlb $(MISC)$/AccessibleKeyStroke.tlb -h $(MISC)$/AccessibleKeyStroke.h -iid $(MISC)$/AccessibleKeyStroke_i.c -proxy $(MISC)$/AccessibleKeyStroke_p.c -Oicf
MTL_SWITCHES2=-tlb $(MISC)$/Charset.tlb -h $(MISC)$/Charset.h -iid $(MISC)$/Charset_i.c -proxy $(MISC)$/Charset_p.c -Oicf
MTL_SWITCHES3=-tlb $(MISC)$/defines.tlb -h $(MISC)$/defines.h -iid $(MISC)$/defines_i.c -proxy $(MISC)$/defines_p.c -Oicf
MTL_SWITCHES4=-tlb $(MISC)$/UAccCOM.tlb -h $(MISC)$/UAccCOM.h -iid $(MISC)$/UAccCOM_i.c -proxy $(MISC)$/UAccCOM_p.c -Oicf
MTL_SWITCHES5=-tlb $(MISC)$/ia2_api_all.tlb -h $(MISC)$/ia2_api_all.h -iid $(MISC)$/ia2_api_all_i.c -proxy $(MISC)$/ia2_api_all_p.c -Oicf

.IF "$(COM)"=="GCC"

MTL_SWITCHESCOMMON=-cpp_cmd gcc -cpp_opt "-x c -E -D__stdcall= -D_stdcall= -nostdinc -I $(shell @cygpath -w $(PSDK_HOME)/include)"
MTL_SWITCHES+=MTL_SWITCHESCOMMON
MTL_SWITCHES1+=MTL_SWITCHESCOMMON
MTL_SWITCHES2+=MTL_SWITCHESCOMMON
MTL_SWITCHES3+=MTL_SWITCHESCOMMON
MTL_SWITCHES4+=MTL_SWITCHESCOMMON
MTL_SWITCHES5+=MTL_SWITCHESCOMMON

$(MISC)$/AccessibleKeyBinding : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE) 
$(MISC)$/AccessibleKeyStroke : $(SOURCE1) 
    $(MTL) $(MTL_SWITCHES1) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE1) 
$(MISC)$/Charset : $(SOURCE2) 
    $(MTL) $(MTL_SWITCHES2) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE2)
$(MISC)$/defines : $(SOURCE3) 
    $(MTL) $(MTL_SWITCHES3) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE3)
$(MISC)$/UAccCOM : $(SOURCE4) 
    $(MTL) $(MTL_SWITCHES4) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE4)
$(MISC)$/ia2_api_all : $(SOURCE5) 
    $(MTL) $(MTL_SWITCHES5) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE5)

.ELSE


$(MISC)$/AccessibleKeyBinding : $(SOURCE)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) && $(TOUCH) $@
$(MISC)$/AccessibleKeyStroke : $(SOURCE1)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES1) $(INCLUDE) $(SOURCE1) && $(TOUCH) $@
$(MISC)$/Charset : $(SOURCE2)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES2) $(INCLUDE) $(SOURCE2) && $(TOUCH) $@
$(MISC)$/defines : $(SOURCE3)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES3) $(INCLUDE) $(SOURCE3) && $(TOUCH) $@
$(MISC)$/UAccCOM : $(SOURCE4)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES4) $(INCLUDE) $(SOURCE4) && $(TOUCH) $@
$(MISC)$/ia2_api_all : $(SOURCE5)
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES5) $(INCLUDE) $(SOURCE5) && $(TOUCH) $@
.ENDIF

.ENDIF

.INCLUDE : target.mk

