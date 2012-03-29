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
all_fragments+=pocketexcel

# -----------------------------------------------
# count = 1
T4_POCKETEXCEL = \
    calc_Pocket_Excel_File

# -----------------------------------------------
# count = 1
F4_POCKETEXCEL = \
    Pocket_Excel

# -----------------------------------------------
# count = 0
F4_UI_POCKETEXCEL =
    
# -----------------------------------------------
# count = 0
L4_POCKETEXCEL =

# -----------------------------------------------
# count = 0
C4_POCKETEXCEL =

# -----------------------------------------------
TYPES_4fcfg_pocketexcel           = $(foreach,i,$(T4_POCKETEXCEL)    types$/$i.xcu                   )
FILTERS_4fcfg_pocketexcel         = $(foreach,i,$(F4_POCKETEXCEL)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_pocketexcel      = $(foreach,i,$(F4_UI_POCKETEXCEL) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_pocketexcel    = $(foreach,i,$(L4_POCKETEXCEL)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_pocketexcel = $(foreach,i,$(C4_POCKETEXCEL)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_pocketexcel = \
    $(TYPES_4fcfg_pocketexcel) \
    $(FILTERS_4fcfg_pocketexcel) \
    $(UI_FILTERS_4fcfg_pocketexcel) \
    $(FRAMELOADERS_4fcfg_pocketexcel) \
    $(CONTENTHANDLERS_4fcfg_pocketexcel)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_pocketexcel)    
    
ALL_PACKAGES+=pocketexcel
    
