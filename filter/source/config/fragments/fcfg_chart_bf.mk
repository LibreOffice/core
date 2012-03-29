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
all_fragments+=chart_bf

# -----------------------------------------------
# count = 3
T4_CHART_BF = \
    chart_StarChart_30 \
    chart_StarChart_40 \
    chart_StarChart_50

# -----------------------------------------------
# count = 3
F4_CHART_BF = \
    StarChart_3_0 \
    StarChart_4_0 \
    StarChart_5_0

# -----------------------------------------------
# count = 0
F4_UI_CHART_BF =

# -----------------------------------------------
# count = 0
L4_CHART_BF =

# -----------------------------------------------
# count = 0
C4_CHART_BF =

# -----------------------------------------------
TYPES_4fcfg_chart_bf           = $(foreach,i,$(T4_CHART_BF)    types$/$i.xcu                   )
FILTERS_4fcfg_chart_bf         = $(foreach,i,$(F4_CHART_BF)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_chart_bf      = $(foreach,i,$(F4_UI_CHART_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_chart_bf    = $(foreach,i,$(L4_CHART_BF)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_chart_bf = $(foreach,i,$(C4_CHART_BF)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_chart_bf = \
    $(TYPES_4fcfg_chart_bf) \
    $(FILTERS_4fcfg_chart_bf) \
    $(UI_FILTERS_4fcfg_chart_bf) \
    $(FRAMELOADERS_4fcfg_chart_bf) \
    $(CONTENTHANDLERS_4fcfg_chart_bf)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_chart_bf)

ALL_PACKAGES+=chart_bf
