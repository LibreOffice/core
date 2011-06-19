all_fragments+=calc

# -----------------------------------------------
# count = 27
T4_CALC = \
    calc_DIF \
    writer_web_HTML \
    calc_Lotus \
    calc_QPro \
    calc_MS_Excel_40 \
    calc_MS_Excel_40_VorlageTemplate \
    calc_MS_Excel_5095 \
    calc_MS_Excel_5095_VorlageTemplate \
    calc_MS_Excel_95 \
    calc_MS_Excel_95_VorlageTemplate \
    calc_MS_Excel_97 \
    calc_MS_Excel_97_VorlageTemplate \
    writer_Rich_Text_Format \
    calc_SYLK \
    calc_StarOffice_XML_Calc \
    calc_Text_txt_csv_StarCalc \
    calc_StarOffice_XML_Calc_Template \
    pdf_Portable_Document_Format \
    calc_dBase\
    calc8\
    calc8_template \
    calc_MS_Excel_2003_XML \
    MS_Excel_2007_XML \
    MS_Excel_2007_XML_Template \
    MS_Excel_2007_Binary \
    calc_OOXML \
    calc_OOXML_Template

# -----------------------------------------------
# count = 28
F4_CALC = \
    DIF \
    HTML__StarCalc_ \
    Lotus \
    QPro \
    MS_Excel_4_0 \
    MS_Excel_4_0_Vorlage_Template \
    MS_Excel_5_0_95 \
    MS_Excel_5_0_95_Vorlage_Template \
    MS_Excel_95 \
    MS_Excel_95_Vorlage_Template \
    MS_Excel_97 \
    MS_Excel_97_Vorlage_Template \
    Rich_Text_Format__StarCalc_ \
    SYLK \
    StarOffice_XML__Calc_ \
    Text___txt___csv__StarCalc_ \
    calc_HTML_WebQuery \
    calc_StarOffice_XML_Calc_Template \
    calc_pdf_Export \
    dBase \
    calc8 \
    calc8_template \
    MS_Excel_2003_XML \
    calc_MS_Excel_2007_XML \
    calc_MS_Excel_2007_XML_Template \
    calc_MS_Excel_2007_Binary \
    calc_OOXML \
    calc_OOXML_Template

# -----------------------------------------------
# count = 12
F4_UI_CALC = \
    HTML__StarCalc__ui \
    MS_Excel_4_0_Vorlage_Template_ui \
    MS_Excel_5_0_95_Vorlage_Template_ui \
    MS_Excel_95_Vorlage_Template_ui \
    MS_Excel_97_Vorlage_Template_ui \
    StarOffice_XML__Calc__ui \
    Text___txt___csv__StarCalc__ui \
    calc_HTML_WebQuery_ui \
    calc_StarOffice_XML_Calc_Template_ui \
    calc8_ui \
    calc8_template_ui \
    MS_Excel_2003_XML_ui \
    calc_MS_Excel_2007_XML_ui \
    calc_MS_Excel_2007_XML_Template_ui \
    calc_MS_Excel_2007_Binary_ui \
    calc_OOXML_ui \
    calc_OOXML_Template_ui

# -----------------------------------------------
# count = 0
L4_CALC =

# -----------------------------------------------
# count = 0
C4_CALC =

# -----------------------------------------------
TYPES_4fcfg_calc           = $(foreach,i,$(T4_CALC)    types$/$i.xcu                     )
FILTERS_4fcfg_calc         = $(foreach,i,$(F4_CALC)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_calc      = $(foreach,i,$(F4_UI_CALC) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_calc    = $(foreach,i,$(L4_CALC)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_calc = $(foreach,i,$(C4_CALC)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_calc = \
    $(TYPES_4fcfg_calc) \
    $(FILTERS_4fcfg_calc) \
    $(UI_FILTERS_4fcfg_calc) \
    $(FRAMELOADERS_4fcfg_calc) \
    $(CONTENTHANDLERS_4fcfg_calc)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_calc)

ALL_PACKAGES+=calc

