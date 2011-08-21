all_fragments+=xslt

# -----------------------------------------------
# count = 5
T4_XSLT = \
    calc_ODS_FlatXML \
    draw_ODG_FlatXML \
    impress_ODP_FlatXML \
    writer_DocBook_File \
    writer_ODT_FlatXML \
    XHTML_File \
    Unified_Office_Format_text \
    Unified_Office_Format_spreadsheet \
    Unified_Office_Format_presentation

# -----------------------------------------------
# count = 8
F4_XSLT = \
    DocBook_File \
    ODG_FlatXML \
    ODP_FlatXML \
    ODS_FlatXML \
    ODT_FlatXML \
    XHTML_Calc_File \
    XHTML_Draw_File \
    XHTML_Impress_File \
    XHTML_Writer_File \
    UOF_text \
    UOF_spreadsheet \
    UOF_presentation

# -----------------------------------------------
# count = 0
F4_UI_XSLT =

# -----------------------------------------------
# count = 0
L4_XSLT =

# -----------------------------------------------
# count = 0
C4_XSLT =

# -----------------------------------------------
TYPES_4fcfg_xslt           = $(foreach,i,$(T4_XSLT)    types$/$i.xcu                     )
FILTERS_4fcfg_xslt         = $(foreach,i,$(F4_XSLT)    filters$/$i.xcu                   )
UI_FILTERS_4fcfg_xslt      = $(foreach,i,$(F4_UI_XSLT) $(DIR_LOCFRAG)$/filters$/$i.xcu   )
FRAMELOADERS_4fcfg_xslt    = $(foreach,i,$(L4_XSLT)    frameloaders$/$i.xcu              )
CONTENTHANDLERS_4fcfg_xslt = $(foreach,i,$(C4_XSLT)    contenthandlers$/$i.xcu           )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_xslt = \
    $(TYPES_4fcfg_xslt) \
    $(FILTERS_4fcfg_xslt) \
    $(UI_FILTERS_4fcfg_xslt) \
    $(FRAMELOADERS_4fcfg_xslt) \
    $(CONTENTHANDLERS_4fcfg_xslt)

ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_xslt)

ALL_PACKAGES+=xslt

