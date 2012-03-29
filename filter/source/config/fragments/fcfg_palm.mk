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
all_fragments+=palm

# -----------------------------------------------
# count = 1
T4_PALM = \
    writer_AportisDoc_PalmDB_File

# -----------------------------------------------
# count = 1
F4_PALM = \
    AportisDoc_Palm_DB

# -----------------------------------------------
# count = 0
F4_UI_PALM =
    
# -----------------------------------------------
# count = 0
L4_PALM =

# -----------------------------------------------
# count = 0
C4_PALM =

# -----------------------------------------------
TYPES_4fcfg_palm           = $(foreach,i,$(T4_PALM)    types$/$i.xcu                     )
FILTERS_4fcfg_palm         = $(foreach,i,$(F4_PALM)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_palm      = $(foreach,i,$(F4_UI_PALM) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_palm    = $(foreach,i,$(L4_PALM)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_palm = $(foreach,i,$(C4_PALM)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_palm = \
    $(TYPES_4fcfg_palm) \
    $(FILTERS_4fcfg_palm) \
    $(UI_FILTERS_4fcfg_palm) \
    $(FRAMELOADERS_4fcfg_palm) \
    $(CONTENTHANDLERS_4fcfg_palm)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_palm)    
    
ALL_PACKAGES+=palm
    
