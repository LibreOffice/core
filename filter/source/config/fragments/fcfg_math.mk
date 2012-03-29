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
all_fragments+=math


# -----------------------------------------------
# count = 5
T4_MATH = \
    math_MathML_XML_Math \
    math_MathType_3x \
    math_StarOffice_XML_Math \
    pdf_Portable_Document_Format \
    math8

# -----------------------------------------------
# count = 5
F4_MATH = \
    MathML_XML__Math_ \
    MathType_3_x \
    StarOffice_XML__Math_ \
    math_pdf_Export \
    math8

# -----------------------------------------------
# count = 2
F4_UI_MATH = \
    StarOffice_XML__Math__ui \
    math8_ui
    
# -----------------------------------------------
# count = 0
L4_MATH =

# -----------------------------------------------
# count = 0
C4_MATH =

# -----------------------------------------------
TYPES_4fcfg_math           = $(foreach,i,$(T4_MATH)    types$/$i.xcu                      )
FILTERS_4fcfg_math         = $(foreach,i,$(F4_MATH)    filters$/$i.xcu                    )
UI_FILTERS_4fcfg_math      = $(foreach,i,$(F4_UI_MATH) $(DIR_LOCFRAG)$/filters$/$i.xcu    )
FRAMELOADERS_4fcfg_math    = $(foreach,i,$(L4_MATH)    frameloaders$/$i.xcu               )
CONTENTHANDLERS_4fcfg_math = $(foreach,i,$(C4_MATH)    contenthandlers$/$i.xcu            )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_math = \
    $(TYPES_4fcfg_math) \
    $(FILTERS_4fcfg_math) \
    $(UI_FILTERS_4fcfg_math) \
    $(FRAMELOADERS_4fcfg_math) \
    $(CONTENTHANDLERS_4fcfg_math)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_math)
    
ALL_PACKAGES+=math
    
