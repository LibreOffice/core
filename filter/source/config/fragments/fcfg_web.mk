all_fragments+=web

.IF "$(WITH_BINFILTER)" != "NO"
T4_WEB_BINFILTER = \
    writer_StarWriter_30 \
    writer_StarWriter_40 \
    writer_StarWriter_50 \
    writer_web_StarWriterWeb_40_VorlageTemplate \
    writer_web_StarWriterWeb_50_VorlageTemplate
F4_WEB_BINFILTER = \
    StarWriter_3_0__StarWriter_Web_ \
    StarWriter_4_0__StarWriter_Web_ \
    StarWriter_5_0__StarWriter_Web_ \
    StarWriter_Web_4_0_Vorlage_Template \
    StarWriter_Web_5_0_Vorlage_Template
.ELSE
T4_WEB_BINFILTER =
F4_WEB_BINFILTER =
.ENDIF

# -----------------------------------------------
# count = 12
T4_WEB = \
    writer_web_HTML \
    $(T4_WEB_BINFILTER) \
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
    $(F4_WEB_BINFILTER) \
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
    StarWriter_Web_4_0_Vorlage_Template_ui \
    StarWriter_Web_5_0_Vorlage_Template_ui \
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
    
