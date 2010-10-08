all_fragments+=draw_bf

# -----------------------------------------------
# count = 4
T4_DRAW_BF = \
    draw_StarDraw_30 \
    draw_StarDraw_30_Vorlage \
    draw_StarDraw_50 \
    draw_StarDraw_50_Vorlage

# -----------------------------------------------
# count = 4
F4_DRAW_BF = \
    StarDraw_3_0 \
    StarDraw_3_0_Vorlage \
    StarDraw_5_0 \
    StarDraw_5_0_Vorlage

# -----------------------------------------------
# count = 2
F4_UI_DRAW_BF = \
    StarDraw_3_0_Vorlage_ui \
    StarDraw_5_0_Vorlage_ui
    
# -----------------------------------------------
# count = 0
L4_DRAW_BF =

# -----------------------------------------------
# count = 0
C4_DRAW_BF =

# -----------------------------------------------
TYPES_4fcfg_draw_bf           = $(foreach,i,$(T4_DRAW_BF)    types$/$i.xcu                    )
FILTERS_4fcfg_draw_bf         = $(foreach,i,$(F4_DRAW_BF)    filters$/$i.xcu                  )
UI_FILTERS_4fcfg_draw_bf      = $(foreach,i,$(F4_UI_DRAW_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu  )
FRAMELOADERS_4fcfg_draw_bf    = $(foreach,i,$(L4_DRAW_BF)    frameloaders$/$i.xcu             )
CONTENTHANDLERS_4fcfg_draw_bf = $(foreach,i,$(C4_DRAW_BF)    contenthandlers$/$i.xcu          )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_draw_bf = \
    $(TYPES_4fcfg_draw_bf) \
    $(FILTERS_4fcfg_draw_bf) \
    $(UI_FILTERS_4fcfg_draw_bf) \
    $(FRAMELOADERS_4fcfg_draw_bf) \
    $(CONTENTHANDLERS_4fcfg_draw_bf)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_draw_bf)

ALL_PACKAGES+=draw_bf
