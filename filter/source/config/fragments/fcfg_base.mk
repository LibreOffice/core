# -----------------------------------------------
# count = 5
T4_BASE = \
    writer_web_HTML_help \
    wav_Wave_Audio_File \
    component_Bibliography \
    StarBase \
    component_DB

# -----------------------------------------------
# count = 2
F4_BASE = \
    writer_web_HTML_help \
    StarOffice_XML__Base_

# -----------------------------------------------
# count = 3
L4_BASE = \
    com_sun_star_frame_Bibliography \
    org_openoffice_comp_dbflt_DBContentLoader2 \
    com_sun_star_sdb_ContentLoader

# -----------------------------------------------
# count = 1
C4_BASE = \
    com_sun_star_comp_framework_SoundHandler

# -----------------------------------------------
TYPES_4fcfg_base           = $(foreach,i,$(T4_BASE) types$/$i.xcu          )
FILTERS_4fcfg_base         = $(foreach,i,$(F4_BASE) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_base    = $(foreach,i,$(L4_BASE) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_base = $(foreach,i,$(C4_BASE) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_base = \
    $(TYPES_4fcfg_base) \
    $(FILTERS_4fcfg_base) \
    $(FRAMELOADERS_4fcfg_base) \
    $(CONTENTHANDLERS_4fcfg_base)
