all_fragments+=pocketexcel

# -----------------------------------------------
# count = 1
T4_POCKETEXCEL = \
    calc_Pocket_Excel_File

# -----------------------------------------------
# count = 1
F4_POCKETEXCEL = \
    Pocket_Excel

# -----------------------------------------------
# count = 0
F4_UI_POCKETEXCEL =
    
# -----------------------------------------------
# count = 0
L4_POCKETEXCEL =

# -----------------------------------------------
# count = 0
C4_POCKETEXCEL =

# -----------------------------------------------
TYPES_4fcfg_pocketexcel           = $(foreach,i,$(T4_POCKETEXCEL)    types$/$i.xcu                   )
FILTERS_4fcfg_pocketexcel         = $(foreach,i,$(F4_POCKETEXCEL)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_pocketexcel      = $(foreach,i,$(F4_UI_POCKETEXCEL) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_pocketexcel    = $(foreach,i,$(L4_POCKETEXCEL)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_pocketexcel = $(foreach,i,$(C4_POCKETEXCEL)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_pocketexcel = \
    $(TYPES_4fcfg_pocketexcel) \
    $(FILTERS_4fcfg_pocketexcel) \
    $(UI_FILTERS_4fcfg_pocketexcel) \
    $(FRAMELOADERS_4fcfg_pocketexcel) \
    $(CONTENTHANDLERS_4fcfg_pocketexcel)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_pocketexcel)    
    
ALL_PACKAGES+=pocketexcel
    
