# *************************************************************
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
# *************************************************************
all_fragments+=pocketword

# -----------------------------------------------
# count = 1
T4_POCKETWORD = \
    writer_PocketWord_File

# -----------------------------------------------
# count = 1
F4_POCKETWORD = \
    PocketWord_File

# -----------------------------------------------
# count = 0
F4_UI_POCKETWORD =
    
# -----------------------------------------------
# count = 0
L4_POCKETWORD =

# -----------------------------------------------
# count = 0
C4_POCKETWORD =

# -----------------------------------------------
TYPES_4fcfg_pocketword           = $(foreach,i,$(T4_POCKETWORD)    types$/$i.xcu                        )
FILTERS_4fcfg_pocketword         = $(foreach,i,$(F4_POCKETWORD)    filters$/$i.xcu                      )
UI_FILTERS_4fcfg_pocketword      = $(foreach,i,$(F4_UI_POCKETWORD) $(DIR_LOCFRAG)$/filters$/$i.xcu      )
FRAMELOADERS_4fcfg_pocketword    = $(foreach,i,$(L4_POCKETWORD)    frameloaders$/$i.xcu                 )
CONTENTHANDLERS_4fcfg_pocketword = $(foreach,i,$(C4_POCKETWORD)    contenthandlers$/$i.xcu              )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_pocketword = \
    $(TYPES_4fcfg_pocketword) \
    $(FILTERS_4fcfg_pocketword) \
    $(UI_FILTERS_4fcfg_pocketword) \
    $(FRAMELOADERS_4fcfg_pocketword) \
    $(CONTENTHANDLERS_4fcfg_pocketword)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_pocketword)
    
ALL_PACKAGES+=pocketword

