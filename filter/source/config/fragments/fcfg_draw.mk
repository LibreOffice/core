# -----------------------------------------------
# count = 9
T4_DRAW = \
    draw_StarDraw_30 \
    draw_StarDraw_30_Vorlage \
    draw_StarDraw_50 \
    draw_StarDraw_50_Vorlage \
    draw_StarOffice_XML_Draw \
    draw_StarOffice_XML_Draw_Template \
    pdf_Portable_Document_Format\
    draw8 \
    draw8_template

# -----------------------------------------------
# count = 9
F4_DRAW = \
    StarDraw_3_0 \
    StarDraw_3_0_Vorlage \
    StarDraw_5_0 \
    StarDraw_5_0_Vorlage \
    StarOffice_XML__Draw_ \
    draw_StarOffice_XML_Draw_Template \
    draw_pdf_Export\
    draw8\
    draw8_template

# -----------------------------------------------
# count = 0
L4_DRAW =

# -----------------------------------------------
# count = 0
C4_DRAW =

# -----------------------------------------------
TYPES_4fcfg_draw           = $(foreach,i,$(T4_DRAW) types$/$i.xcu          )
FILTERS_4fcfg_draw         = $(foreach,i,$(F4_DRAW) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_draw    = $(foreach,i,$(L4_DRAW) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_draw = $(foreach,i,$(C4_DRAW) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_draw = \
    $(TYPES_4fcfg_draw) \
    $(FILTERS_4fcfg_draw) \
    $(FRAMELOADERS_4fcfg_draw) \
    $(CONTENTHANDLERS_4fcfg_draw)
