all_fragments+=global_bf


# -----------------------------------------------
# count = 5
T4_GLOBAL_BF = \
    writer_StarWriter_30 \
    writer_StarWriter_40 \
    writer_globaldocument_StarWriter_40GlobalDocument \
    writer_StarWriter_50 \
    writer_globaldocument_StarWriter_50GlobalDocument

# -----------------------------------------------
# count = 5
F4_GLOBAL_BF = \
    StarWriter_3_0__StarWriter_GlobalDocument_ \
    StarWriter_4_0__StarWriter_GlobalDocument_ \
    StarWriter_4_0_GlobalDocument \
    StarWriter_5_0__StarWriter_GlobalDocument_ \
    StarWriter_5_0_GlobalDocument

# -----------------------------------------------
# count = 2
F4_UI_GLOBAL_BF = \
    StarWriter_4_0_GlobalDocument_ui \
    StarWriter_5_0_GlobalDocument_ui

# -----------------------------------------------
# count = 0
L4_GLOBAL_BF =

# -----------------------------------------------
# count = 0
C4_GLOBAL_BF =

# -----------------------------------------------
TYPES_4fcfg_global_bf           = $(foreach,i,$(T4_GLOBAL_BF)    types$/$i.xcu                     )
FILTERS_4fcfg_global_bf         = $(foreach,i,$(F4_GLOBAL_BF)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_global_bf      = $(foreach,i,$(F4_UI_GLOBAL_BF) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_global_bf    = $(foreach,i,$(L4_GLOBAL_BF)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_global_bf = $(foreach,i,$(C4_GLOBAL_BF)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_global_bf = \
    $(TYPES_4fcfg_global_bf) \
    $(FILTERS_4fcfg_global_bf) \
    $(UI_FILTERS_4fcfg_global_bf) \
    $(FRAMELOADERS_4fcfg_global_bf) \
    $(CONTENTHANDLERS_4fcfg_global_bf)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_global_bf)
    
ALL_PACKAGES+=global_bf

