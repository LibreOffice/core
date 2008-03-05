all_fragments+=database

# -----------------------------------------------
# count = 3
T4_DATABASE = \
    StarBase \
    StarBase_Report \
    StarBaseReportChart

# -----------------------------------------------
# count = 3
F4_DATABASE = \
    StarOffice_XML__Base_ \
    StarOffice_XML__Report_ \
    StarOffice_XML__DB_Report_Chart_
    
# -----------------------------------------------
# count = 3
F4_UI_DATABASE = \
    StarOffice_XML__Base__ui \
    StarOffice_XML__Report__ui \
    StarOffice_XML__DB_Report_Chart__ui

# -----------------------------------------------
# count = 3
L4_DATABASE = \
    org_openoffice_comp_dbflt_DBContentLoader2

# -----------------------------------------------
# count = 1
C4_DATABASE =

# -----------------------------------------------
TYPES_4fcfg_database           = $(foreach,i,$(T4_DATABASE)    types$/$i.xcu                     )
FILTERS_4fcfg_database         = $(foreach,i,$(F4_DATABASE)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_database      = $(foreach,i,$(F4_UI_DATABASE) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_database    = $(foreach,i,$(L4_DATABASE)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_database = $(foreach,i,$(C4_DATABASE)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_database = \
    $(TYPES_4fcfg_database) \
    $(FILTERS_4fcfg_database) \
    $(UI_FILTERS_4fcfg_database) \
    $(FRAMELOADERS_4fcfg_database) \
    $(CONTENTHANDLERS_4fcfg_database)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_database)

ALL_PACKAGES+=database
