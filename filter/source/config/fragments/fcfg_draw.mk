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
all_fragments+=draw

# -----------------------------------------------
# count = 9
T4_DRAW = \
    draw_StarOffice_XML_Draw \
    draw_StarOffice_XML_Draw_Template \
    pdf_Portable_Document_Format\
    draw8 \
    draw8_template

# -----------------------------------------------
# count = 9
F4_DRAW = \
    StarOffice_XML__Draw_ \
    draw_StarOffice_XML_Draw_Template \
    draw_pdf_Export \
    draw8 \
    draw8_template

# -----------------------------------------------
# count = 4
F4_UI_DRAW = \
    StarOffice_XML__Draw__ui \
    draw_StarOffice_XML_Draw_Template_ui \
    draw8_ui \
    draw8_template_ui
    
# -----------------------------------------------
# count = 0
L4_DRAW =

# -----------------------------------------------
# count = 0
C4_DRAW =

# -----------------------------------------------
TYPES_4fcfg_draw           = $(foreach,i,$(T4_DRAW)    types$/$i.xcu                    )
FILTERS_4fcfg_draw         = $(foreach,i,$(F4_DRAW)    filters$/$i.xcu                  )
UI_FILTERS_4fcfg_draw      = $(foreach,i,$(F4_UI_DRAW) $(DIR_LOCFRAG)$/filters$/$i.xcu  )
FRAMELOADERS_4fcfg_draw    = $(foreach,i,$(L4_DRAW)    frameloaders$/$i.xcu             )
CONTENTHANDLERS_4fcfg_draw = $(foreach,i,$(C4_DRAW)    contenthandlers$/$i.xcu          )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_draw = \
    $(TYPES_4fcfg_draw) \
    $(FILTERS_4fcfg_draw) \
    $(UI_FILTERS_4fcfg_draw) \
    $(FRAMELOADERS_4fcfg_draw) \
    $(CONTENTHANDLERS_4fcfg_draw)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_draw)

ALL_PACKAGES+=draw

