all_fragments+=chart

.IF "$(WITH_BINFILTER)" != "NO"
T4_CHART_BINFILTER = \
    chart_StarChart_30 \
    chart_StarChart_40 \
    chart_StarChart_50
F4_CHART_BINFILTER = \
    StarChart_3_0 \
    StarChart_4_0 \
    StarChart_5_0
.ELSE
T4_CHART_BINFILTER =
F4_CHART_BINFILTER =
.ENDIF

# -----------------------------------------------
# count = 5
T4_CHART = \
    $(T4_CHART_BINFILTER) \
    chart_StarOffice_XML_Chart \
    chart2_StarOffice_XML_Chart\
    chart8

# -----------------------------------------------
# count = 5
F4_CHART = \
    $(F4_CHART_BINFILTER) \
    StarOffice_XML__Chart_\
    chart8
    
# -----------------------------------------------
# count = 2
F4_UI_CHART = \
    StarOffice_XML__Chart__ui \
    chart8_ui

# -----------------------------------------------
# count = 0
L4_CHART = \
    com_sun_star_comp_chart2_ChartFrameLoader

# -----------------------------------------------
# count = 0
C4_CHART =

# -----------------------------------------------
TYPES_4fcfg_chart           = $(foreach,i,$(T4_CHART)    types$/$i.xcu                   )
FILTERS_4fcfg_chart         = $(foreach,i,$(F4_CHART)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_chart      = $(foreach,i,$(F4_UI_CHART) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_chart    = $(foreach,i,$(L4_CHART)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_chart = $(foreach,i,$(C4_CHART)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_chart = \
    $(TYPES_4fcfg_chart) \
    $(FILTERS_4fcfg_chart) \
    $(UI_FILTERS_4fcfg_chart) \
    $(FRAMELOADERS_4fcfg_chart) \
    $(CONTENTHANDLERS_4fcfg_chart)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_chart)

ALL_PACKAGES+=chart
