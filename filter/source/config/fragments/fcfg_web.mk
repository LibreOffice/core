# -----------------------------------------------
# count = 12
T4_WEB = \
    writer_web_HTML \
    writer_StarWriter_30 \
    writer_StarWriter_40 \
    writer_StarWriter_50 \
    writer_web_StarWriterWeb_40_VorlageTemplate \
    writer_web_StarWriterWeb_50_VorlageTemplate \
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
    StarWriter_3_0__StarWriter_Web_ \
    StarWriter_4_0__StarWriter_Web_ \
    StarWriter_5_0__StarWriter_Web_ \
    StarWriter_Web_4_0_Vorlage_Template \
    StarWriter_Web_5_0_Vorlage_Template \
    Text__StarWriter_Web_ \
    Text__encoded___StarWriter_Web_ \
    writer_web_HTML_help \
    writer_web_StarOffice_XML_Writer \
    writer_web_StarOffice_XML_Writer_Web_Template \
    writer_web_pdf_Export\
    writerweb8_writer_template\
    writerweb8_writer

# -----------------------------------------------
# count = 0
L4_WEB =

# -----------------------------------------------
# count = 0
C4_WEB =

# -----------------------------------------------
TYPES_4fcfg_web           = $(foreach,i,$(T4_WEB) types$/$i.xcu          )
FILTERS_4fcfg_web         = $(foreach,i,$(F4_WEB) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_web    = $(foreach,i,$(L4_WEB) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_web = $(foreach,i,$(C4_WEB) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_web = \
    $(TYPES_4fcfg_web) \
    $(FILTERS_4fcfg_web) \
    $(FRAMELOADERS_4fcfg_web) \
    $(CONTENTHANDLERS_4fcfg_web)
