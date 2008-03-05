all_fragments+=impress


.IF "$(WITH_BINFILTER)" != "NO"
T4_IMPRESS_BINFILTER = \
    draw_StarDraw_30 \
    draw_StarDraw_30_Vorlage \
    draw_StarDraw_50 \
    draw_StarDraw_50_Vorlage \
    impress_StarImpress_40 \
    impress_StarImpress_40_Vorlage \
    impress_StarImpress_50 \
    impress_StarImpress_50_Vorlage \
    impress_StarImpress_50_packed
F4_IMPRESS_BINFILTER = \
    StarDraw_3_0_Vorlage__StarImpress_ \
    StarDraw_3_0__StarImpress_ \
    StarDraw_5_0_Vorlage__StarImpress_ \
    StarDraw_5_0__StarImpress_ \
    StarImpress_4_0 \
    StarImpress_4_0_Vorlage \
    StarImpress_5_0 \
    StarImpress_5_0_Vorlage \
    StarImpress_5_0__packed_
.ELSE
T4_IMPRESS_BINFILTER =
F4_IMPRESS_BINFILTER =
.ENDIF

# -----------------------------------------------
# count = 19
T4_IMPRESS = \
    draw_StarOffice_XML_Draw \
    impress_MS_PowerPoint_97 \
    impress_MS_PowerPoint_97_Vorlage \
    $(T4_IMPRESS_BINFILTER) \
    impress_StarOffice_XML_Impress \
    impress_StarOffice_XML_Impress_Template \
    pdf_Portable_Document_Format \
    pwp_PlaceWare\
    impress8\
    impress8_template\
    draw8\
    MS_PowerPoint_2007_XML\
    MS_PowerPoint_2007_XML_Template

# -----------------------------------------------
# count = 20
F4_IMPRESS = \
    MS_PowerPoint_97 \
    MS_PowerPoint_97_Vorlage \
    $(F4_IMPRESS_BINFILTER) \
    impress_StarOffice_XML_Draw \
    StarOffice_XML__Impress_ \
    impress_StarOffice_XML_Impress_Template \
    impress_pdf_Export \
    placeware_Export\
    impress8\
    impress8_template\
    impress8_draw\
    impress_MS_PowerPoint_2007_XML\
    impress_MS_PowerPoint_2007_XML_Template

# -----------------------------------------------
# count = 12
F4_UI_IMPRESS = \
    MS_PowerPoint_97_Vorlage_ui \
    StarDraw_3_0_Vorlage__StarImpress__ui \
    StarDraw_5_0_Vorlage__StarImpress__ui \
    impress_StarOffice_XML_Draw_ui \
    StarImpress_4_0_Vorlage_ui \
    StarImpress_5_0_Vorlage_ui \
    StarImpress_5_0__packed__ui \
    StarOffice_XML__Impress__ui \
    impress_StarOffice_XML_Impress_Template_ui \
    impress8_ui \
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

