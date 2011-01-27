all_fragments+=math_bf

# -----------------------------------------------
# count = 4
T4_MATH_BF = \
    math_StarMath_20 \
    math_StarMath_30 \
    math_StarMath_40 \
    math_StarMath_50

# -----------------------------------------------
# count = 4
F4_MATH_BF = \
    StarMath_2_0 \
    StarMath_3_0 \
    StarMath_4_0 \
    StarMath_5_0

# -----------------------------------------------
# count = 0
F4_UI_MATH_BF =
    
# -----------------------------------------------
# count = 0
L4_MATH_BF =

# -----------------------------------------------
# count = 0
C4_MATH_BF =

# -----------------------------------------------
TYPES_4fcfg_math_bf           = $(foreach,i,$(T4_MATH_BF)    types$/$i.xcu                      )
FILTERS_4fcfg_math_bf         = $(foreach,i,$(F4_MATH_BF)    filters$/$i.xcu                    )
UI_FILTERS_4fcfg_math_bf      = $(foreach,i,$(F4_UI_MATH_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu    )
FRAMELOADERS_4fcfg_math_bf    = $(foreach,i,$(L4_MATH_BF)    frameloaders$/$i.xcu               )
CONTENTHANDLERS_4fcfg_math_bf = $(foreach,i,$(C4_MATH_BF)    contenthandlers$/$i.xcu            )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_math_bf = \
    $(TYPES_4fcfg_math_bf) \
    $(FILTERS_4fcfg_math_bf) \
    $(UI_FILTERS_4fcfg_math_bf) \
    $(FRAMELOADERS_4fcfg_math_bf) \
    $(CONTENTHANDLERS_4fcfg_math_bf)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_math_bf)
    
ALL_PACKAGES+=math_bf
    
