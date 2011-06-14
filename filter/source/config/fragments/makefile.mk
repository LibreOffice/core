#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ     = ..$/..$/..
TARGET  = FCFGPkg
PRJNAME = filter

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE: settings.mk

# -----------------------------------------------------------------------------
# include all package definition files
# -----------------------------------------------------------------------------

.INCLUDE: packagedef.mk

# -----------------------------------------------------------------------------
# list of all required destination files
# -----------------------------------------------------------------------------

REALFILTERPACKAGES_TYPES_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_base.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_writer.types_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_web.types_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_global.types_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.types_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_database.types_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.types_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.types_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.types_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.types_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.types_flag

.IF "$(WITH_BINFILTER)" != "NO"
REALFILTERPACKAGES_TYPES_FLAG += \
    $(DIR_FILTERCFGOUT)$/fcfg_writer_bf.types_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_web_bf.types_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_global_bf.types_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_calc_bf.types_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_draw_bf.types_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_impress_bf.types_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_chart_bf.types_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_math_bf.types_flag
.ENDIF

REALFILTERPACKAGES_FILTERS_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_base.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_writer.filters_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_web.filters_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_global.filters_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.filters_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_database.filters_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.filters_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.filters_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.filters_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.filters_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.filters_flag

.IF "$(WITH_BINFILTER)" != "NO"
REALFILTERPACKAGES_FILTERS_FLAG += \
    $(DIR_FILTERCFGOUT)$/fcfg_writer_bf.filters_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_web_bf.filters_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_global_bf.filters_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_calc_bf.filters_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_draw_bf.filters_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_impress_bf.filters_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_chart_bf.filters_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_math_bf.filters_flag
.ENDIF

REALFILTERPACKAGES_FILTERS_UI_MERGE = \
    $(DIR_LOCMERGE)$/filter_ui.xcu

REALFILTERPACKAGES_FILTERS_UI_LANGPACKS = \
    $(foreach,i,$(alllangiso) $(DIR_LANGPACK)$/$i$/org$/openoffice$/TypeDetection$/Filter.xcu)

REALFILTERPACKAGES_FILTERS_UI_LANGPACKS_PACKED = \
    $(foreach,i,$(alllangiso) $(COMMONBIN)$/fcfg_langpack_$i.zip)

REALFILTERPACKAGES_OTHERS_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_base.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_writer.others_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_web.others_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_global.others_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.others_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_database.others_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.others_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.others_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.others_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.others_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.others_flag

.IF "$(WITH_BINFILTER)" != "NO"
REALFILTERPACKAGES_OTHERS_FLAG += \
    $(DIR_FILTERCFGOUT)$/fcfg_writer_bf.others_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_web_bf.others_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_global_bf.others_flag          \
    $(DIR_FILTERCFGOUT)$/fcfg_calc_bf.others_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_draw_bf.others_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_impress_bf.others_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_chart_bf.others_flag           \
    $(DIR_FILTERCFGOUT)$/fcfg_math_bf.others_flag
.ENDIF

INTERNALFILTERPACKAGES_TYPES_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_internalgraphics.types_flag

INTERNALFILTERPACKAGES_FILTERS_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_internalgraphics.filters_flag

ALL_FLAGS = \
    $(REALFILTERPACKAGES_TYPES_FLAG)                    \
    $(REALFILTERPACKAGES_FILTERS_FLAG)                  \
    $(REALFILTERPACKAGES_FILTERS_UI_MERGE)              \
    $(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS)          \
    $(REALFILTERPACKAGES_OTHERS_FLAG)                   \
    $(INTERNALFILTERPACKAGES_TYPES_FLAG)                \
    $(INTERNALFILTERPACKAGES_FILTERS_FLAG)

# -----------------------------------------------------------------------------
# build all
# -----------------------------------------------------------------------------

zip1generatedlangs=TRUE
zip1langdirs=$(alllangiso)
ZIP1TARGET=fcfg_langpack
ZIP1FLAGS= -u -r
ZIP1DIR=$(DIR_LANGPACK)$/$(LANGDIR)
ZIP1LIST=$(LANGDIR)$/*

.INCLUDE: target.mk

ALLTAR : $(ALL_FLAGS)

$(ALL_FLAGS) : $(INCLUDE_FRAGMENTS)

.IF "$(SOLAR_JAVA)"==""
#cmc, hack to workaround the java build requirement
.IF "$(SYSTEM_PYTHON)" == "YES"
MERGE:=$(PYTHON) ../tools/merge/pyAltFCFGMerge
.ELSE
MERGE:=$(AUGMENT_LIBRARY_PATH) PYTHONPATH=$(SOLARLIBDIR)/python $(SOLARBINDIR)/python ../tools/merge/pyAltFCFGMerge
.ENDIF
.ELSE
MERGE    := $(JAVAI) $(JAVAIFLAGS) -jar $(SOLARBINDIR)$/FCFGMerge.jar
.ENDIF

PACKLANG := $(XSLTPROC) --nonet
PACKLANG_IN :=
PACKLANG_PARAM := --stringparam
PACKLANG_XSL :=

# -----------------------------------------------------------------------------
# build every module seperated
# -----------------------------------------------------------------------------

$(REALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
     @echo ===================================================================
     @echo Merge type fragments for package $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_MODPACKS)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_types.xcu xmlpackage=Types tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(REALFILTERPACKAGES_FILTERS_UI_MERGE) : $(ALL_UI_FILTERS)
     @echo ===================================================================
     @echo Merge localized filter fragments for package $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_LOCMERGE)
     $(MERGE) fragmentsdir=$(DIR_LOCFRAG) tempdir=$(TEMP) pkg=$@ xmlpackage=Filter fcfg=$(mktmp items=$(ALL_UI_FILTERS:b:t",")) languagepack=true

# Note: The used XSL transformation returns errors on stderr.
# But they dont do it in case we use the -OUT parameter.
# So we have to dump stdout to the target file and let stderr messages break the build!
$(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS) : $(REALFILTERPACKAGES_FILTERS_UI_MERGE)
     @echo ===================================================================
     @echo Building language packages $(@:b:s/Filter_//) for filters
     @echo ===================================================================
     +-$(MKDIRHIER) $(@:d)
     $(PACKLANG) $(PACKLANG_PARAM) lang $(@:d:d:d:d:d:d:d:d:b) $(PACKLANG_XSL) langfilter.xsl $(PACKLANG_IN) $< > $@

$(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS_PACKED) : $(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS)

$(REALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     @echo ===================================================================
     @echo Building packages filter $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_MODPACKS)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_filters.xcu xmlpackage=Filter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(REALFILTERPACKAGES_OTHERS_FLAG) : $$(ALL_4$$(@:b))
     @echo ===================================================================
     @echo Building packages others $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_MODPACKS)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_others.xcu xmlpackage=Misc lcfg=$(mktmp items=$(FRAMELOADERS_4$(@:b):b:t",":s/.xcu//)) ccfg=$(mktmp items=$(CONTENTHANDLERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
    @echo ===================================================================
     @echo Building special packages types $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_MODPACKS)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_types.xcu xmlpackage=Types tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     @echo ===================================================================
     @echo Building special packages filter $@
     @echo ===================================================================
     -$(MKDIRHIER) $(DIR_MODPACKS)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_filters.xcu xmlpackage=GraphicFilter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) subdir_filters=internalgraphicfilters && $(TOUCH) $@

