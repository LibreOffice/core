all_fragments+=draw

# -----------------------------------------------
# count = 9
T4_DRAW = \
    draw_StarOffice_XML_Draw \
    draw_StarOffice_XML_Draw_Template \
    pdf_Portable_Document_Format\
    draw8 \
    draw8_template \
    draw_WordPerfect_Graphics \
    draw_Visio_Document

# -----------------------------------------------
# count = 9
F4_DRAW = \
    StarOffice_XML__Draw_ \
    draw_StarOffice_XML_Draw_Template \
    draw_pdf_Export \
    draw8 \
    draw8_template \
    WordPerfectGraphics \
    VisioDocument


# -----------------------------------------------
# count = 4
F4_UI_DRAW = \
    StarOffice_XML__Draw__ui \
    draw_StarOffice_XML_Draw_Template_ui \
    draw8_ui \
    draw8_template_ui
    
# -----------------------------------------------
# count = 0
L4_DRAW =

# -----------------------------------------------
# count = 0
C4_DRAW =

# -----------------------------------------------
TYPES_4fcfg_draw           = $(foreach,i,$(T4_DRAW)    types$/$i.xcu                    )
FILTERS_4fcfg_draw         = $(foreach,i,$(F4_DRAW)    filters$/$i.xcu                  )
UI_FILTERS_4fcfg_draw      = $(foreach,i,$(F4_UI_DRAW) $(DIR_LOCFRAG)$/filters$/$i.xcu  )
FRAMELOADERS_4fcfg_draw    = $(foreach,i,$(L4_DRAW)    frameloaders$/$i.xcu             )
CONTENTHANDLERS_4fcfg_draw = $(foreach,i,$(C4_DRAW)    contenthandlers$/$i.xcu          )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_draw = \
    $(TYPES_4fcfg_draw) \
    $(FILTERS_4fcfg_draw) \
    $(UI_FILTERS_4fcfg_draw) \
    $(FRAMELOADERS_4fcfg_draw) \
    $(CONTENTHANDLERS_4fcfg_draw)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_draw)

ALL_PACKAGES+=draw

