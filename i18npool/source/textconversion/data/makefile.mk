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

PRJNAME=i18npool
TARGET=textconv_dict
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# Fix heap limit problem on MSC
.IF "$(OS)" == "WNT"
.IF  "$(COMEX)" != "8"
CDEFS+=-Zm300
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUI)" == "OS2"
SHL1STDLIBS+= $(SALLIB) $(CPPULIB)
.ENDIF

# collator data library
SHL1TARGET=$(TARGET)
SHL1IMPLIB=i$(SHL1TARGET)

SHL1VERSIONMAP=$(SHL1TARGET).map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

DATA_FILES:=$(shell @ls ./*.dic)
SHL1OBJS=$(subst,./,$(SLO)$/ $(DATA_FILES:s/.dic/.obj/))

LIB1TARGET=	$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

APP1TARGET = genconv_dict
$(MISC)$/%.cxx : %.dic
    $(AUGMENT_LIBRARY_PATH) $(BIN)$/genconv_dict $* $< $@

# ugly - is this dependency really required here?
.IF "$(GUI)" != "OS2"
$(foreach,i,$(shell @$(FIND) . -name "*.dic") $(MISC)$/dict_$(i:b).cxx) : $(BIN)$/genconv_dict$(EXECPOST)
.ENDIF

