# -----------------------------------------------
# count = 9
T4_MATH = \
    math_MathML_XML_Math \
    math_MathType_3x \
    math_StarMath_20 \
    math_StarMath_30 \
    math_StarMath_40 \
    math_StarMath_50 \
    math_StarOffice_XML_Math \
    pdf_Portable_Document_Format \
    math8

# -----------------------------------------------
# count = 9
F4_MATH = \
    MathML_XML__Math_ \
    MathType_3_x \
    StarMath_2_0 \
    StarMath_3_0 \
    StarMath_4_0 \
    StarMath_5_0 \
    StarOffice_XML__Math_ \
    math_pdf_Export \
    math8

# -----------------------------------------------
# count = 0
L4_MATH =

# -----------------------------------------------
# count = 0
C4_MATH =

# -----------------------------------------------
TYPES_4fcfg_math           = $(foreach,i,$(T4_MATH) types$/$i.xcu          )
FILTERS_4fcfg_math         = $(foreach,i,$(F4_MATH) filters$/$i.xcu        )
FRAMELOADERS_4fcfg_math    = $(foreach,i,$(L4_MATH) frameloaders$/$i.xcu   )
CONTENTHANDLERS_4fcfg_math = $(foreach,i,$(C4_MATH) contenthandlers$/$i.xcu)

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_math = \
    $(TYPES_4fcfg_math) \
    $(FILTERS_4fcfg_math) \
    $(FRAMELOADERS_4fcfg_math) \
    $(CONTENTHANDLERS_4fcfg_math)
