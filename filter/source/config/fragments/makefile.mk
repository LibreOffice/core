#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Date: 2005-02-02 13:51:42 $
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
# include all package definition files
# -----------------------------------------------------------------------------

.INCLUDE: packagedef.mk

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE: settings.mk

# -----------------------------------------------------------------------------
# list of all required destination files
# -----------------------------------------------------------------------------

REALFILTERPACKAGES_TYPES_FLAG = \
    $(MISC)$/fcfg_base.types_flag               \
    $(MISC)$/fcfg_writer.types_flag             \
    $(MISC)$/fcfg_web.types_flag                \
    $(MISC)$/fcfg_global.types_flag             \
    $(MISC)$/fcfg_w4w.types_flag                \
    $(MISC)$/fcfg_calc.types_flag               \
    $(MISC)$/fcfg_draw.types_flag               \
    $(MISC)$/fcfg_impress.types_flag            \
    $(MISC)$/fcfg_chart.types_flag              \
    $(MISC)$/fcfg_math.types_flag               \
    $(MISC)$/fcfg_xslt.types_flag               \
    $(MISC)$/fcfg_palm.types_flag               \
    $(MISC)$/fcfg_pocketexcel.types_flag        \
    $(MISC)$/fcfg_pocketword.types_flag         \
    $(MISC)$/fcfg_drawgraphics.types_flag       \
    $(MISC)$/fcfg_impressgraphics.types_flag
    
REALFILTERPACKAGES_FILTERS_FLAG = \
    $(MISC)$/fcfg_base.filters_flag               \
    $(MISC)$/fcfg_writer.filters_flag             \
    $(MISC)$/fcfg_web.filters_flag                \
    $(MISC)$/fcfg_global.filters_flag             \
    $(MISC)$/fcfg_w4w.filters_flag                \
    $(MISC)$/fcfg_calc.filters_flag               \
    $(MISC)$/fcfg_draw.filters_flag               \
    $(MISC)$/fcfg_impress.filters_flag            \
    $(MISC)$/fcfg_chart.filters_flag              \
    $(MISC)$/fcfg_math.filters_flag               \
    $(MISC)$/fcfg_xslt.filters_flag               \
    $(MISC)$/fcfg_palm.filters_flag               \
    $(MISC)$/fcfg_pocketexcel.filters_flag        \
    $(MISC)$/fcfg_pocketword.filters_flag         \
    $(MISC)$/fcfg_drawgraphics.filters_flag       \
    $(MISC)$/fcfg_impressgraphics.filters_flag
    
REALFILTERPACKAGES_OTHERS_FLAG = \
    $(MISC)$/fcfg_base.others_flag               \
    $(MISC)$/fcfg_writer.others_flag             \
    $(MISC)$/fcfg_web.others_flag                \
    $(MISC)$/fcfg_global.others_flag             \
    $(MISC)$/fcfg_w4w.others_flag                \
    $(MISC)$/fcfg_calc.others_flag               \
    $(MISC)$/fcfg_draw.others_flag               \
    $(MISC)$/fcfg_impress.others_flag            \
    $(MISC)$/fcfg_chart.others_flag              \
    $(MISC)$/fcfg_math.others_flag               \
    $(MISC)$/fcfg_xslt.others_flag               \
    $(MISC)$/fcfg_palm.others_flag               \
    $(MISC)$/fcfg_pocketexcel.others_flag        \
    $(MISC)$/fcfg_pocketword.others_flag         \
    $(MISC)$/fcfg_drawgraphics.others_flag       \
    $(MISC)$/fcfg_impressgraphics.others_flag

INTERNALFILTERPACKAGES_FILTERS_FLAG = \
    $(MISC)$/fcfg_internalgraphics.filters_flag

INTERNALFILTERPACKAGES_TYPES_FLAG = \
    $(MISC)$/fcfg_internalgraphics.types_flag

# -----------------------------------------------------------------------------
# build all
# -----------------------------------------------------------------------------

.INCLUDE: target.mk

ALLTAR: \
    $(REALFILTERPACKAGES_TYPES_FLAG)        \
    $(REALFILTERPACKAGES_FILTERS_FLAG)      \
    $(REALFILTERPACKAGES_OTHERS_FLAG)       \
    $(INTERNALFILTERPACKAGES_TYPES_FLAG)    \
    $(INTERNALFILTERPACKAGES_FILTERS_FLAG)

.IF "$(SOLAR_JAVA)"==""
#cmc, hack to workaround the java build requirement
MERGE:=python ../tools/merge/pyAltFCFGMerge
.ELSE
MERGE:=$(JAVA) -jar $(CLASSDIR)$/FCFGMerge.jar
.ENDIF

# -----------------------------------------------------------------------------
# build every module seperated
# -----------------------------------------------------------------------------

$(REALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building packages types $@
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_types.xcu xmlpackage=Types tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@
     
$(REALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building localized packages filter $@
     +$(MERGE) fragmentsdir=$(MISC) tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_filters.xcu xmlpackage=Filter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@
     
$(REALFILTERPACKAGES_OTHERS_FLAG) : $$(ALL_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building packages others $@
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_others.xcu xmlpackage=Misc lcfg=$(mktmp items=$(FRAMELOADERS_4$(@:b):b:t",":s/.xcu//)) ccfg=$(mktmp items=$(CONTENTHANDLERS_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_TYPES_FLAG) : $$(TYPES_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building special packages types $@
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_types.xcu   xmlpackage=Types  tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu//)) && $(TOUCH) $@

$(INTERNALFILTERPACKAGES_FILTERS_FLAG) : $$(FILTERS_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building special packages filter $@
     +$(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_filters.xcu xmlpackage=GraphicFilter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu//)) subdir_filters=internalgraphicfilters && $(TOUCH) $@

