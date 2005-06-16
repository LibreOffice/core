#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Date: 2005-06-16 14:36:29 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
    $(DIR_FILTERCFGOUT)$/fcfg_w4w.types_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.types_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.types_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.types_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.types_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.types_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.types_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.types_flag

REALFILTERPACKAGES_FILTERS_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_base.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_writer.filters_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_web.filters_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_global.filters_flag             \
    $(DIR_FILTERCFGOUT)$/fcfg_w4w.filters_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.filters_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.filters_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.filters_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.filters_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.filters_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.filters_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.filters_flag

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
    $(DIR_FILTERCFGOUT)$/fcfg_w4w.others_flag                \
    $(DIR_FILTERCFGOUT)$/fcfg_calc.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_draw.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_impress.others_flag            \
    $(DIR_FILTERCFGOUT)$/fcfg_chart.others_flag              \
    $(DIR_FILTERCFGOUT)$/fcfg_math.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_xslt.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_palm.others_flag               \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketexcel.others_flag        \
    $(DIR_FILTERCFGOUT)$/fcfg_pocketword.others_flag         \
    $(DIR_FILTERCFGOUT)$/fcfg_drawgraphics.others_flag       \
    $(DIR_FILTERCFGOUT)$/fcfg_impressgraphics.others_flag

INTERNALFILTERPACKAGES_TYPES_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_internalgraphics.types_flag

INTERNALFILTERPACKAGES_FILTERS_FLAG = \
    $(DIR_FILTERCFGOUT)$/fcfg_internalgraphics.filters_flag

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

ALLTAR: \
    $(REALFILTERPACKAGES_TYPES_FLAG)                    \
    $(REALFILTERPACKAGES_FILTERS_FLAG)                  \
    $(REALFILTERPACKAGES_FILTERS_UI_MERGE)              \
    $(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS)          \
    $(REALFILTERPACKAGES_OTHERS_FLAG)                   \
    $(INTERNALFILTERPACKAGES_TYPES_FLAG)                \
    $(INTERNALFILTERPACKAGES_FILTERS_FLAG)

.IF "$(SOLAR_JAVA)"==""
#cmc, hack to workaround the java build requirement
MERGE:=python ../tools/merge/pyAltFCFGMerge
.ELSE
MERGE    := $(JAVA) -jar $(CLASSDIR)$/FCFGMerge.jar
.IF "$(JDK)" == "gcj"
PACKLANG := $(JAVA) $(JAVACPS) $(SOLARBINDIR)/xalan.jar org.apache.xalan.xslt.Process -XSL langfilter.xsl
.ELSE
PACKLANG := $(JAVA) org.apache.xalan.xslt.Process -XSL langfilter.xsl
.ENDIF
.ENDIF

# -----------------------------------------------------------------------------
# build every module seperated
# -----------------------------------------------------------------------------

$(REALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
     +@echo ===================================================================
     +@echo Merge type fragments for package $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_MODPACKS)
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_types.xcu xmlpackage=Types tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(REALFILTERPACKAGES_FILTERS_UI_MERGE) : $(ALL_UI_FILTERS)
     +@echo ===================================================================
     +@echo Merge localized filter fragments for package $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_LOCMERGE)
     +$(MERGE) fragmentsdir=$(DIR_LOCFRAG) tempdir=$(TEMP) pkg=$@ xmlpackage=Filter fcfg=$(mktmp items=$(ALL_UI_FILTERS:b:t",")) languagepack=true

# Note: The used XSL transformation returns errors on stderr.
# But they dont do it in case we use the -OUT parameter.
# So we have to dump stdout to the target file and let stderr messages break the build!
$(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS) : $(REALFILTERPACKAGES_FILTERS_UI_MERGE)
     +@echo ===================================================================
     +@echo Building language packages $(@:b:s/Filter_//) for filters
     +@echo ===================================================================
     +-$(MKDIRHIER) $(@:d)
     +$(PACKLANG) -IN $< -PARAM lang $(@:d:d:d:d:d:d:d:d:b) > $@

$(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS_PACKED) : $(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS)

$(REALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     +@echo ===================================================================
     +@echo Building packages filter $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_MODPACKS)
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_filters.xcu xmlpackage=Filter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(REALFILTERPACKAGES_OTHERS_FLAG) : $$(ALL_4$$(@:b))
     +@echo ===================================================================
     +@echo Building packages others $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_MODPACKS)
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_others.xcu xmlpackage=Misc lcfg=$(mktmp items=$(FRAMELOADERS_4$(@:b):b:t",":s/.xcu//)) ccfg=$(mktmp items=$(CONTENTHANDLERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
    +@echo ===================================================================
     +@echo Building special packages types $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_MODPACKS)
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_types.xcu xmlpackage=Types tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     +@echo ===================================================================
     +@echo Building special packages filter $@
     +@echo ===================================================================
     +-$(MKDIRHIER) $(DIR_MODPACKS)
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(DIR_MODPACKS) pkg=$(DIR_MODPACKS)$/$(@:b)_filters.xcu xmlpackage=GraphicFilter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) subdir_filters=internalgraphicfilters && $(TOUCH) $@

