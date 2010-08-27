all_fragments+=base

# -----------------------------------------------
# count = 6
T4_BASE = \
    writer_web_HTML_help \
    oxt_OpenOffice_Extension \
    wav_Wave_Audio_File \
    component_Bibliography \
    component_DB

# -----------------------------------------------
# count = 2
F4_BASE = \
    writer_web_HTML_help
    
# -----------------------------------------------
# count = 1
F4_UI_BASE =

# -----------------------------------------------
# count = 3
L4_BASE = \
    com_sun_star_frame_Bibliography \
    com_sun_star_sdb_ContentLoader

# -----------------------------------------------
# count = 2
C4_BASE = \
    com_sun_star_comp_framework_SoundHandler \
    com_sun_star_comp_framework_oxt_handler

# -----------------------------------------------
TYPES_4fcfg_base           = $(foreach,i,$(T4_BASE)    types$/$i.xcu                     )
FILTERS_4fcfg_base         = $(foreach,i,$(F4_BASE)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_base      = $(foreach,i,$(F4_UI_BASE) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_base    = $(foreach,i,$(L4_BASE)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_base = $(foreach,i,$(C4_BASE)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_base = \
    $(TYPES_4fcfg_base) \
    $(FILTERS_4fcfg_base) \
    $(UI_FILTERS_4fcfg_base) \
    $(FRAMELOADERS_4fcfg_base) \
    $(CONTENTHANDLERS_4fcfg_base)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_base)

ALL_PACKAGES+=base
