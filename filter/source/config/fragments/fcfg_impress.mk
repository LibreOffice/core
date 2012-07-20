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
all_fragments+=impress

# -----------------------------------------------
# count = 19
T4_IMPRESS = \
    draw_StarOffice_XML_Draw \
    impress_MS_PowerPoint_97 \
    impress_MS_PowerPoint_97_Vorlage \
    impress_StarOffice_XML_Impress \
    impress_StarOffice_XML_Impress_Template \
    pdf_Portable_Document_Format \
    pwp_PlaceWare\
    impress8\
    impress_NSO_UOF2 \
    impress8_template\
    draw8\
    MS_PowerPoint_2007_XML\
    MS_PowerPoint_2007_XML_Template

# -----------------------------------------------
# count = 20
F4_IMPRESS = \
    MS_PowerPoint_97 \
    MS_PowerPoint_97_Vorlage \
    impress_StarOffice_XML_Draw \
    StarOffice_XML__Impress_ \
    impress_StarOffice_XML_Impress_Template \
    impress_pdf_Export \
    placeware_Export\
    impress8\
    NSO_Impress_UOF2 \
    impress8_template\
    impress8_draw\
    impress_MS_PowerPoint_2007_XML\
    impress_MS_PowerPoint_2007_XML_Template

# -----------------------------------------------
# count = 12
F4_UI_IMPRESS = \
    MS_PowerPoint_97_Vorlage_ui \
    impress_StarOffice_XML_Draw_ui \
    StarOffice_XML__Impress__ui \
    impress_StarOffice_XML_Impress_Template_ui \
    impress8_ui \
    NSO_Impress_UOF2_ui \
    impress8_template_ui \
    impress8_draw_ui \
    impress_MS_PowerPoint_2007_XML_ui \
    impress_MS_PowerPoint_2007_XML_Template_ui

# -----------------------------------------------
# count = 0
L4_GLOBAL =

# -----------------------------------------------
# count = 0
C4_GLOBAL =

# -----------------------------------------------
TYPES_4fcfg_impress           = $(foreach,i,$(T4_IMPRESS)    types$/$i.xcu                   )
FILTERS_4fcfg_impress         = $(foreach,i,$(F4_IMPRESS)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_impress      = $(foreach,i,$(F4_UI_IMPRESS) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_impress    = $(foreach,i,$(L4_IMPRESS)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_impress = $(foreach,i,$(C4_IMPRESS)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_impress = \
    $(TYPES_4fcfg_impress) \
    $(FILTERS_4fcfg_impress) \
    $(UI_FILTERS_4fcfg_impress) \
    $(FRAMELOADERS_4fcfg_impress) \
    $(CONTENTHANDLERS_4fcfg_impress)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_impress)

ALL_PACKAGES+=impress

