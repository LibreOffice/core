all_fragments+=calc_bf

# -----------------------------------------------
# count = 7
T4_CALC_BF = \
    calc_StarCalc_10 \
    calc_StarCalc_30 \
    calc_StarCalc_30_VorlageTemplate \
    calc_StarCalc_40 \
    calc_StarCalc_40_VorlageTemplate \
    calc_StarCalc_50 \
    calc_StarCalc_50_VorlageTemplate

# -----------------------------------------------
# count = 7
F4_CALC_BF = \
    StarCalc_1_0 \
    StarCalc_3_0 \
    StarCalc_3_0_Vorlage_Template \
    StarCalc_4_0 \
    StarCalc_4_0_Vorlage_Template \
    StarCalc_5_0 \
    StarCalc_5_0_Vorlage_Template

# -----------------------------------------------
# count = 3
F4_UI_CALC_BF = \
    StarCalc_3_0_Vorlage_Template_ui \
    StarCalc_4_0_Vorlage_Template_ui \
    StarCalc_5_0_Vorlage_Template_ui
    
# -----------------------------------------------
# count = 0
L4_CALC_BF =

# -----------------------------------------------
# count = 0
C4_CALC_BF =

# -----------------------------------------------
TYPES_4fcfg_calc_bf           = $(foreach,i,$(T4_CALC_BF)    types$/$i.xcu                     )
FILTERS_4fcfg_calc_bf         = $(foreach,i,$(F4_CALC_BF)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_calc_bf      = $(foreach,i,$(F4_UI_CALC_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_calc_bf    = $(foreach,i,$(L4_CALC_BF)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_calc_bf = $(foreach,i,$(C4_CALC_BF)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_calc_bf = \
    $(TYPES_4fcfg_calc) \
    $(FILTERS_4fcfg_calc) \
    $(UI_FILTERS_4fcfg_calc) \
    $(FRAMELOADERS_4fcfg_calc) \
    $(CONTENTHANDLERS_4fcfg_calc)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_calc_bf)

ALL_PACKAGES+=calc_bf

