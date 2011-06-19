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
    writer_WordPerfect_Document \
    writer_MS_Works_Document \
    writer_T602_Document \
    writer_LotusWordPro_Document \
    writer_Text \
    writer_Text_encoded \
    writer_MIZI_Hwp_97 \
    writer_StarOffice_XML_Writer_Template \
    pdf_Portable_Document_Format\
    writer8_template\
    writer8 \
    writer_MS_Word_2003_XML \
    writer_MS_Word_2007_XML \
    writer_MS_Word_2007_XML_Template \
    writer_OOXML \
    writer_OOXML_Template \
    writer_layout_dump_xml

# -----------------------------------------------
# count = 39
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
    WordPerfect \
    MS_Works \
    T602Document \
    LotusWordPro \
    Text \
    Text__encoded_ \
    writer_MIZI_Hwp_97 \
    writer_StarOffice_XML_Writer_Template \
    writer_pdf_Export\
    writer8\
    writer8_template \
    MS_Word_2003_XML \
    MS_Word_2007_XML \
    MS_Word_2007_XML_Template \
    OOXML_Text \
    OOXML_Text_Template \
    writer_layout_dump

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
    writer8_template_ui \
    MS_Word_2003_XML_ui \
    MS_Word_2007_XML_ui \
    MS_Word_2007_XML_Template_ui \
    OOXML_Text_ui \
    OOXML_Text_Template_ui

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

