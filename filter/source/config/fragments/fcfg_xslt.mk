# -----------------------------------------------
# count = 4
T4_XSLT = \
    writer_DocBook_File \
    writer_MS_Word_2003_XML \
    calc_MS_Excel_2003_XML \
    XHTML_File

# -----------------------------------------------
# count = 7
F4_XSLT = \
    DocBook_File \
    MS_Excel_2003_XML \
    MS_Word_2003_XML \
    XHTML_Calc_File \
    XHTML_Draw_File \
    XHTML_Impress_File \
    XHTML_Writer_File

# -----------------------------------------------
# count = 0
L4_XSLT =

# -----------------------------------------------
# count = 0
C4_XSLT =

# -----------------------------------------------
TYPES_4fcfg_xslt           = $(foreach,i,$(T4_XSLT) types$/$i.xcu          )
FILTERS_4fcfg_xslt         = $(foreach,i,$(F4_XSLT) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_xslt    = $(foreach,i,$(L4_XSLT) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_xslt = $(foreach,i,$(C4_XSLT) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_xslt = \
    $(TYPES_4fcfg_xslt) \
    $(FILTERS_4fcfg_xslt) \
    $(FRAMELOADERS_4fcfg_xslt) \
    $(CONTENTHANDLERS_4fcfg_xslt)
