# -----------------------------------------------
# count = 18
T4_IMPRESS = \
    draw_StarDraw_30 \
    draw_StarDraw_30_Vorlage \
    draw_StarDraw_50 \
    draw_StarDraw_50_Vorlage \
    draw_StarOffice_XML_Draw \
    impress_MS_PowerPoint_97 \
    impress_MS_PowerPoint_97_Vorlage \
    impress_StarImpress_40 \
    impress_StarImpress_40_Vorlage \
    impress_StarImpress_50 \
    impress_StarImpress_50_Vorlage \
    impress_StarImpress_50_packed \
    impress_StarOffice_XML_Impress \
    impress_StarOffice_XML_Impress_Template \
    pdf_Portable_Document_Format \
    pwp_PlaceWare\
    impress8\
    impress8_template

# -----------------------------------------------
# count = 19
F4_IMPRESS = \
    MS_PowerPoint_97 \
    MS_PowerPoint_97_Vorlage \
    StarDraw_3_0_Vorlage__StarImpress_ \
    StarDraw_3_0__StarImpress_ \
    StarDraw_5_0_Vorlage__StarImpress_ \
    StarDraw_5_0__StarImpress_ \
    StarImpress_4_0 \
    impress_StarOffice_XML_Draw \
    StarImpress_4_0_Vorlage \
    StarImpress_5_0 \
    StarImpress_5_0_Vorlage \
    StarImpress_5_0__packed_ \
    StarOffice_XML__Impress_ \
    impress_StarOffice_XML_Impress_Template \
    impress_pdf_Export \
    placeware_Export\
    impress8\
    impress8_template\
    impress8_draw

# -----------------------------------------------
# count = 0
L4_GLOBAL =

# -----------------------------------------------
# count = 0
C4_GLOBAL =

# -----------------------------------------------
TYPES_4fcfg_impress           = $(foreach,i,$(T4_IMPRESS) types$/$i.xcu          )
FILTERS_4fcfg_impress         = $(foreach,i,$(F4_IMPRESS) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_impress    = $(foreach,i,$(L4_IMPRESS) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_impress = $(foreach,i,$(C4_IMPRESS) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_impress = \
    $(TYPES_4fcfg_impress) \
    $(FILTERS_4fcfg_impress) \
    $(FRAMELOADERS_4fcfg_impress) \
    $(CONTENTHANDLERS_4fcfg_impress)
