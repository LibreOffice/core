# -----------------------------------------------
# count = 10
T4_GLOBAL = \
    writer_StarWriter_30 \
    writer_StarWriter_40 \
    writer_globaldocument_StarWriter_40GlobalDocument \
    writer_StarWriter_50 \
    writer_globaldocument_StarWriter_50GlobalDocument \
    writer_Text \
    writer_StarOffice_XML_Writer \
    writer_globaldocument_StarOffice_XML_Writer_GlobalDocument \
    pdf_Portable_Document_Format \
    writerglobal8

# -----------------------------------------------
# count = 11
F4_GLOBAL = \
    StarWriter_3_0__StarWriter_GlobalDocument_ \
    StarWriter_4_0__StarWriter_GlobalDocument_ \
    StarWriter_4_0_GlobalDocument \
    StarWriter_5_0__StarWriter_GlobalDocument_ \
    StarWriter_5_0_GlobalDocument \
    Text__encoded___StarWriter_GlobalDocument_ \
    writer_globaldocument_StarOffice_XML_Writer \
    writer_globaldocument_StarOffice_XML_Writer_GlobalDocument \
    writer_globaldocument_pdf_Export \
    writerglobal8 \
    writerglobal8_writer

# -----------------------------------------------
# count = 0
L4_GLOBAL =

# -----------------------------------------------
# count = 0
C4_GLOBAL =

# -----------------------------------------------
TYPES_4fcfg_global           = $(foreach,i,$(T4_GLOBAL) types$/$i.xcu          )
FILTERS_4fcfg_global         = $(foreach,i,$(F4_GLOBAL) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_global    = $(foreach,i,$(L4_GLOBAL) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_global = $(foreach,i,$(C4_GLOBAL) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_global = \
    $(TYPES_4fcfg_global) \
    $(FILTERS_4fcfg_global) \
    $(FRAMELOADERS_4fcfg_global) \
    $(CONTENTHANDLERS_4fcfg_global)
