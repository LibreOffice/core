all_fragments+=web_bf

# -----------------------------------------------
# count = 5
T4_WEB_BF = \
    writer_StarWriter_30 \
    writer_StarWriter_40 \
    writer_StarWriter_50 \
    writer_web_StarWriterWeb_40_VorlageTemplate \
    writer_web_StarWriterWeb_50_VorlageTemplate

# -----------------------------------------------
# count = 5
F4_WEB_BF = \
    StarWriter_3_0__StarWriter_Web_ \
    StarWriter_4_0__StarWriter_Web_ \
    StarWriter_5_0__StarWriter_Web_ \
    StarWriter_Web_4_0_Vorlage_Template \
    StarWriter_Web_5_0_Vorlage_Template

# -----------------------------------------------
# count = 2
F4_UI_WEB_BF = \
    StarWriter_Web_4_0_Vorlage_Template_ui \
    StarWriter_Web_5_0_Vorlage_Template_ui
    
# -----------------------------------------------
# count = 0
L4_WEB_BF =

# -----------------------------------------------
# count = 0
C4_WEB_BF =

# -----------------------------------------------
TYPES_4fcfg_web_bf           = $(foreach,i,$(T4_WEB_BF)    types$/$i.xcu                   )
FILTERS_4fcfg_web_bf         = $(foreach,i,$(F4_WEB_BF)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_web_bf      = $(foreach,i,$(F4_UI_WEB_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_web_bf    = $(foreach,i,$(L4_WEB_BF)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_web_bf = $(foreach,i,$(C4_WEB_BF)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_web_bf = \
    $(TYPES_4fcfg_web_bf) \
    $(FILTERS_4fcfg_web_bf) \
    $(UI_FILTERS_4fcfg_web_bf) \
    $(FRAMELOADERS_4fcfg_web_bf) \
    $(CONTENTHANDLERS_4fcfg_web_bf)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_web_bf)
    
ALL_PACKAGES+=web_bf
    
