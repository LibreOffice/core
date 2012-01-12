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

PRJNAME=i18npool
TARGET=collator

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

txtlist:=$(shell @cd data >& $(NULLDEV) && ls *.txt)
LOCAL_RULE_LANGS:=$(uniq $(foreach,i,$(txtlist) $(i:s/-/_/:s/_/ /:1)))
rules_dependencies:=$(foreach,i,$(txtlist) data$/$i) $(INCCOM)$/lrl_include.hxx

rules_obj = $(SLO)$/collator_unicode.obj

SLOFILES=   \
        $(SLO)$/collatorImpl.obj \
        $(SLO)$/chaptercollator.obj \
        $(rules_obj)

APP1TARGET = gencoll_rule
APP1RPATH = NONE

APP1OBJS   = $(OBJ)$/gencoll_rule.obj

DEPOBJFILES = $(APP1OBJS)

APP1STDLIBS = $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB) \
        $(ICUDATALIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(rules_obj) : $(rules_dependencies)

$(INCCOM)$/lrl_include.hxx: $(foreach,i,$(txtlist) data$/$i)
    @@$(RM) $@
    @echo $(EMQ)#define LOCAL_RULE_LANGS $(EMQ)"$(LOCAL_RULE_LANGS)$(EMQ)" >& $@

