all_fragments+=impress

# -----------------------------------------------
# count = 19
T4_IMPRESS = \
    draw_StarOffice_XML_Draw \
    impress_MS_PowerPoint_97 \
    impress_MS_PowerPoint_97_AutoPlay \
    impress_MS_PowerPoint_97_Vorlage \
    impress_StarOffice_XML_Impress \
    impress_StarOffice_XML_Impress_Template \
    pdf_Portable_Document_Format \
    pwp_PlaceWare\
    impress8\
    impress8_template\
    draw8\
    MS_PowerPoint_2007_XML\
    MS_PowerPoint_2007_XML_AutoPlay\
    MS_PowerPoint_2007_XML_Template\
    impress_OOXML_Presentation\
    impress_OOXML_Presentation_Template

# -----------------------------------------------
# count = 20
F4_IMPRESS = \
    MS_PowerPoint_97 \
    MS_PowerPoint_97_AutoPlay \
    MS_PowerPoint_97_Vorlage \
    impress_StarOffice_XML_Draw \
    StarOffice_XML__Impress_ \
    impress_StarOffice_XML_Impress_Template \
    impress_pdf_Export \
    placeware_Export\
    impress8\
    impress8_template\
    impress8_draw\
    impress_MS_PowerPoint_2007_XML\
    impress_MS_PowerPoint_2007_XML_AutoPlay\
    impress_MS_PowerPoint_2007_XML_Template\
    impress_OOXML\
    impress_OOXML_Template

# -----------------------------------------------
# count = 12
F4_UI_IMPRESS = \
    MS_PowerPoint_97_Vorlage_ui \
    impress_StarOffice_XML_Draw_ui \
    StarOffice_XML__Impress__ui \
    impress_StarOffice_XML_Impress_Template_ui \
    impress8_ui \
    impress8_template_ui \
    impress8_draw_ui \
    impress_MS_PowerPoint_2007_XML_ui \
    impress_OOXML_ui\
    impress_OOXML_Template_ui

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

