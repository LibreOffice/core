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
all_fragments+=writer_bf

# -----------------------------------------------
# count = 9
T4_WRITER_BF = \
    writer_StarWriter_10 \
    writer_StarWriter_20 \
    writer_StarWriter_30 \
    writer_StarWriter_30_VorlageTemplate \
    writer_StarWriter_40 \
    writer_StarWriter_40_VorlageTemplate \
    writer_StarWriter_50 \
    writer_StarWriter_50_VorlageTemplate \
    writer_StarWriter_DOS

# -----------------------------------------------
# count = 9
F4_WRITER_BF = \
    StarWriter_1_0 \
    StarWriter_2_0 \
    StarWriter_3_0 \
    StarWriter_3_0_Vorlage_Template \
    StarWriter_4_0 \
    StarWriter_4_0_Vorlage_Template \
    StarWriter_5_0 \
    StarWriter_5_0_Vorlage_Template \
    StarWriter_DOS

# -----------------------------------------------
# count = 3
F4_UI_WRITER_BF = \
    StarWriter_3_0_Vorlage_Template_ui \
    StarWriter_4_0_Vorlage_Template_ui \
    StarWriter_5_0_Vorlage_Template_ui \

# -----------------------------------------------
# count = 0
L4_WRITER_BF =

# -----------------------------------------------
# count = 0
C4_WRITER_BF =

# -----------------------------------------------
TYPES_4fcfg_writer_bf           = $(foreach,i,$(T4_WRITER_BF)    types$/$i.xcu                     )
FILTERS_4fcfg_writer_bf         = $(foreach,i,$(F4_WRITER_BF)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_writer_bf      = $(foreach,i,$(F4_UI_WRITER_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_writer_bf    = $(foreach,i,$(L4_WRITER_BF)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_writer_bf = $(foreach,i,$(C4_WRITER_BF)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_writer_bf = \
    $(TYPES_4fcfg_writer_bf) \
    $(FILTERS_4fcfg_writer_bf) \
    $(UI_FILTERS_4fcfg_writer_bf) \
    $(FRAMELOADERS_4fcfg_writer_bf) \
    $(CONTENTHANDLERS_4fcfg_writer_bf)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_writer_bf)

ALL_PACKAGES+=writer_bf

