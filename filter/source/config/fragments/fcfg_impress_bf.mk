all_fragments+=impress_bf


# -----------------------------------------------
# count = 9
T4_IMPRESS_BF = \
    draw_StarDraw_30 \
    draw_StarDraw_30_Vorlage \
    draw_StarDraw_50 \
    draw_StarDraw_50_Vorlage \
    impress_StarImpress_40 \
    impress_StarImpress_40_Vorlage \
    impress_StarImpress_50 \
    impress_StarImpress_50_Vorlage \
    impress_StarImpress_50_packed

# -----------------------------------------------
# count = 9
F4_IMPRESS_BF = \
    StarDraw_3_0_Vorlage__StarImpress_ \
    StarDraw_3_0__StarImpress_ \
    StarDraw_5_0_Vorlage__StarImpress_ \
    StarDraw_5_0__StarImpress_ \
    StarImpress_4_0 \
    StarImpress_4_0_Vorlage \
    StarImpress_5_0 \
    StarImpress_5_0_Vorlage \
    StarImpress_5_0__packed_

# -----------------------------------------------
# count = 5
F4_UI_IMPRESS_BF = \
    StarDraw_3_0_Vorlage__StarImpress__ui \
    StarDraw_5_0_Vorlage__StarImpress__ui \
    StarImpress_4_0_Vorlage_ui \
    StarImpress_5_0_Vorlage_ui \
    StarImpress_5_0__packed__ui \

# -----------------------------------------------
# count = 0
L4_IMPRESS_BF =

# -----------------------------------------------
# count = 0
C4_IMPRESS_BF =

# -----------------------------------------------
TYPES_4fcfg_impress_bf           = $(foreach,i,$(T4_IMPRESS_BF)    types$/$i.xcu                   )
FILTERS_4fcfg_impress_bf         = $(foreach,i,$(F4_IMPRESS_BF)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_impress_bf      = $(foreach,i,$(F4_UI_IMPRESS_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_impress_bf    = $(foreach,i,$(L4_IMPRESS_BF)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_impress_bf = $(foreach,i,$(C4_IMPRESS_BF)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_impress_bf = \
    $(TYPES_4fcfg_impress_bf) \
    $(FILTERS_4fcfg_impress_bf) \
    $(UI_FILTERS_4fcfg_impress_bf) \
    $(FRAMELOADERS_4fcfg_impress_bf) \
    $(CONTENTHANDLERS_4fcfg_impress_bf)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_impress_bf)

ALL_PACKAGES+=impress_bf

