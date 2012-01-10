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



PRJ = ..$/..

PRJNAME	= lingucomponent
TARGET	= ulingu
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk


.IF "$(SYSTEM_DICTS)" == "YES"
CXXFLAGS += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
CFLAGSCXX += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
CFLAGSCC += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
.ENDIF


SLOFILES = $(SLO)$/lingutil.obj

LIB1TARGET= $(SLB)$/lib$(TARGET).lib
LIB1ARCHIV= $(LB)/lib$(TARGET).a
LIB1OBJFILES= $(SLOFILES)


# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

