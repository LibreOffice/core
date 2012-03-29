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
all_fragments+=web

# -----------------------------------------------
# count = 12
T4_WEB = \
    writer_web_HTML \
    writer_Text \
    writer_web_HTML_help \
    writer_StarOffice_XML_Writer \
    writer_web_StarOffice_XML_Writer_Web_Template \
    pdf_Portable_Document_Format\
    writerweb8_writer_template

# -----------------------------------------------
# count = 14
F4_WEB = \
    HTML \
    Text__StarWriter_Web_ \
    Text__encoded___StarWriter_Web_ \
    writer_web_HTML_help \
    writer_web_StarOffice_XML_Writer \
    writer_web_StarOffice_XML_Writer_Web_Template \
    writer_web_pdf_Export\
    writerweb8_writer_template\
    writerweb8_writer

# -----------------------------------------------
# count = 9
F4_UI_WEB = \
    HTML_ui \
    Text__StarWriter_Web__ui \
    Text__encoded___StarWriter_Web__ui \
    writer_web_StarOffice_XML_Writer_ui \
    writer_web_StarOffice_XML_Writer_Web_Template_ui \
    writerweb8_writer_template_ui \
    writerweb8_writer_ui
    
# -----------------------------------------------
# count = 0
L4_WEB =

# -----------------------------------------------
# count = 0
C4_WEB =

# -----------------------------------------------
TYPES_4fcfg_web           = $(foreach,i,$(T4_WEB)    types$/$i.xcu                   )
FILTERS_4fcfg_web         = $(foreach,i,$(F4_WEB)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_web      = $(foreach,i,$(F4_UI_WEB) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_web    = $(foreach,i,$(L4_WEB)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_web = $(foreach,i,$(C4_WEB)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_web = \
    $(TYPES_4fcfg_web) \
    $(FILTERS_4fcfg_web) \
    $(UI_FILTERS_4fcfg_web) \
    $(FRAMELOADERS_4fcfg_web) \
    $(CONTENTHANDLERS_4fcfg_web)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_web)
    
ALL_PACKAGES+=web
    
