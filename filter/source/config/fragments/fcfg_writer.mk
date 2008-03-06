all_fragments+=writer


.IF "$(WITH_BINFILTER)" != "NO"
T4_WRITER_BINFILTER = \
    writer_StarWriter_10 \
    writer_StarWriter_20 \
    writer_StarWriter_30 \
    writer_StarWriter_30_VorlageTemplate \
    writer_StarWriter_40 \
    writer_StarWriter_40_VorlageTemplate \
    writer_StarWriter_50 \
    writer_StarWriter_50_VorlageTemplate \
    writer_StarWriter_DOS
F4_WRITER_BINFILTER = \
    StarWriter_1_0 \
    StarWriter_2_0 \
    StarWriter_3_0 \
    StarWriter_3_0_Vorlage_Template \
    StarWriter_4_0 \
    StarWriter_4_0_Vorlage_Template \
    StarWriter_5_0 \
    StarWriter_5_0_Vorlage_Template \
    StarWriter_DOS
.ELSE
T4_WRITER_BINFILTER =
F4_WRITER_BINFILTER =
.ENDIF

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
    $(T4_WRITER_BINFILTER) \
    writer_WordPerfect_Document \
    writer_T602_Document \
    writer_Text \
    writer_Text_encoded \
    writer_JustSystem_Ichitaro_10 \
    writer_JustSystem_Ichitaro_10_template \
    writer_MIZI_Hwp_97 \
    writer_StarOffice_XML_Writer_Template \
    writer_WPSSystem_WPS2000_10 \
    pdf_Portable_Document_Format\
    writer8_template\
    writer8 \
    writer_MS_Word_2003_XML \
    writer_MS_Word_2007_XML \
    MediaWiki_File \
    LaTeX \
    BibTeX

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
    $(F4_WRITER_BINFILTER) \
    WordPerfect \
    T602Document \
    Text \
    Text__encoded_ \
    writer_JustSystem_Ichitaro_10 \
    writer_JustSystem_Ichitaro_10_template \
    writer_MIZI_Hwp_97 \
    writer_StarOffice_XML_Writer_Template \
    writer_WPSSystem_WPS2000_10 \
    writer_pdf_Export\
    writer8\
    writer8_template \
    writer_MediaWiki_File \
    writer_web_MediaWiki_File \
    MS_Word_2003_XML \
    MS_Word_2007_XML \
    LaTeX_Writer \
    BibTeX_Writer

# -----------------------------------------------
# count = 14
F4_UI_WRITER = \
    HTML__StarWriter__ui \
    MS_Word_95_Vorlage_ui \
    MS_Word_97_Vorlage_ui \
    StarOffice_XML__Writer__ui \
    StarWriter_3_0_Vorlage_Template_ui \
    StarWriter_4_0_Vorlage_Template_ui \
    StarWriter_5_0_Vorlage_Template_ui \
    Text_ui \
    Text__encoded__ui \
    writer_JustSystem_Ichitaro_10_template_ui \
    writer_StarOffice_XML_Writer_Template_ui \
    writer8_ui \
    writer8_template_ui \
    MS_Word_2003_XML_ui \
    MS_Word_2007_XML_ui

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

