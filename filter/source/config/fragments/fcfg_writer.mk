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
all_fragments+=writer

# -----------------------------------------------
# count = 39
T4_WRITER = \
    writer_web_HTML \
    writer_Lotus_1_2_3_10_DOS_StarWriter \
    writer_Lotus_1_2_3_10_WIN_StarWriter \
    calc_MS_Excel_40 \
    calc_MS_Excel_5095 \
    calc_MS_Excel_95 \
    writer_MS_WinWord_5 \
    writer_MS_WinWord_60 \
    writer_MS_Word_95 \
    writer_MS_Word_95_Vorlage \
    writer_MS_Word_97 \
    writer_MS_Word_97_Vorlage \
    writer_Rich_Text_Format \
    writer_StarOffice_XML_Writer \
    writer_T602_Document \
    writer_Text \
    writer_Text_encoded \
    writer_StarOffice_XML_Writer_Template \
    pdf_Portable_Document_Format\
    writer8_template\
    writer8 \
    writer_NSO_UOF2 \
    writer_MS_Word_2003_XML \
    writer_MS_Word_2007_XML \
    writer_MS_Word_2007_XML_Template

.IF "$(SYSTEM_LIBWPD)" == "YES"
T4_WRITER += writer_WordPerfect_Document
.ENDIF

# -----------------------------------------------
# count = 38
F4_WRITER = \
    HTML__StarWriter_ \
    Lotus_1_2_3_1_0__DOS___StarWriter_ \
    Lotus_1_2_3_1_0__WIN___StarWriter_ \
    MS_Excel_4_0__StarWriter_ \
    MS_Excel_5_0__StarWriter_ \
    MS_Excel_95__StarWriter_ \
    MS_WinWord_5 \
    MS_WinWord_6_0 \
    MS_Word_95 \
    MS_Word_95_Vorlage \
    MS_Word_97 \
    MS_Word_97_Vorlage \
    Rich_Text_Format \
    StarOffice_XML__Writer_ \
    T602Document \
    Text \
    Text__encoded_ \
    writer_StarOffice_XML_Writer_Template \
    writer_pdf_Export\
    writer8\
    NSO_Writer_UOF2 \
    writer8_template \
    MS_Word_2003_XML \
    MS_Word_2007_XML \
    MS_Word_2007_XML_Template

.IF "$(SYSTEM_LIBWPD)" == "YES"
F4_WRITER += WordPerfect
.ENDIF


# -----------------------------------------------
# count = 14
F4_UI_WRITER = \
    HTML__StarWriter__ui \
    MS_Word_95_Vorlage_ui \
    MS_Word_97_Vorlage_ui \
    StarOffice_XML__Writer__ui \
    Text_ui \
    Text__encoded__ui \
    writer_StarOffice_XML_Writer_Template_ui \
    writer8_ui \
    NSO_Writer_UOF2_ui \
    writer8_template_ui \
    MS_Word_2003_XML_ui \
    MS_Word_2007_XML_ui \
    MS_Word_2007_XML_Template_ui

# -----------------------------------------------
# count = 0
L4_WRITER =

# -----------------------------------------------
# count = 0
C4_WRITER =

# -----------------------------------------------
TYPES_4fcfg_writer           = $(foreach,i,$(T4_WRITER)    types$/$i.xcu                     )
FILTERS_4fcfg_writer         = $(foreach,i,$(F4_WRITER)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_writer      = $(foreach,i,$(F4_UI_WRITER) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_writer    = $(foreach,i,$(L4_WRITER)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_writer = $(foreach,i,$(C4_WRITER)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_writer = \
    $(TYPES_4fcfg_writer) \
    $(FILTERS_4fcfg_writer) \
    $(UI_FILTERS_4fcfg_writer) \
    $(FRAMELOADERS_4fcfg_writer) \
    $(CONTENTHANDLERS_4fcfg_writer)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_writer)

ALL_PACKAGES+=writer

