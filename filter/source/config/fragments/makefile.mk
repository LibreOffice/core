#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Date: 2004-06-16 10:42:01 $
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
PRJNAME = $(TARGET)

# -----------------------------------------------------------------------------
# include all package definition files
# -----------------------------------------------------------------------------

.INCLUDE: fcfg_base.mk
.INCLUDE: fcfg_writer.mk
.INCLUDE: fcfg_web.mk
.INCLUDE: fcfg_global.mk
.INCLUDE: fcfg_w4w.mk
.INCLUDE: fcfg_calc.mk
.INCLUDE: fcfg_draw.mk
.INCLUDE: fcfg_impress.mk
.INCLUDE: fcfg_chart.mk
.INCLUDE: fcfg_math.mk
.INCLUDE: fcfg_xslt.mk
.INCLUDE: fcfg_palm.mk
.INCLUDE: fcfg_pocketword.mk
.INCLUDE: fcfg_pocketexcel.mk
.INCLUDE: fcfg_internalgraphics.mk
.INCLUDE: fcfg_drawgraphics.mk
.INCLUDE: fcfg_impressgraphics.mk

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE: settings.mk

# -----------------------------------------------------------------------------
# list of all required destination files
# -----------------------------------------------------------------------------

REALFILTERPACKAGES  =   \
    $(BIN)$/fcfg_base.zip               \
    $(BIN)$/fcfg_writer.zip             \
    $(BIN)$/fcfg_web.zip                \
    $(BIN)$/fcfg_global.zip             \
    $(BIN)$/fcfg_w4w.zip                \
    $(BIN)$/fcfg_calc.zip               \
    $(BIN)$/fcfg_draw.zip               \
    $(BIN)$/fcfg_impress.zip            \
    $(BIN)$/fcfg_chart.zip              \
    $(BIN)$/fcfg_math.zip               \
    $(BIN)$/fcfg_xslt.zip               \
    $(BIN)$/fcfg_palm.zip               \
    $(BIN)$/fcfg_pocketexcel.zip        \
    $(BIN)$/fcfg_pocketword.zip         \
    $(BIN)$/fcfg_drawgraphics.zip       \
    $(BIN)$/fcfg_impressgraphics.zip

INTERNALFILTERPACKAGES  =   \
    $(BIN)$/fcfg_internalgraphics.zip

# -----------------------------------------------------------------------------
# build all
# -----------------------------------------------------------------------------

.INCLUDE: target.mk

ALLTAR: $(REALFILTERPACKAGES) $(INTERNALFILTERPACKAGES)

.IF "$(SOLAR_JAVA)"==""
#cmc, hack to workaround the java build requirement
MERGE:=python ../tools/merge/pyAltFCFGMerge
.ELSE
MERGE:=$(JAVA) -jar $(CLASSDIR)$/FCFGMerge.jar
.ENDIF

# -----------------------------------------------------------------------------
# build every module seperated
# -----------------------------------------------------------------------------

$(REALFILTERPACKAGES) : $$(ALL_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building package $@ with $(MERGE)
     +@echo TEMP is $(TEMP)
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_types.xcu   xmlpackage=Types  tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu/))
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_filters.xcu xmlpackage=Filter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu/))
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_others.xcu  xmlpackage=Misc   lcfg=$(mktmp items=$(FRAMELOADERS_4$(@:b):b:t",":s/.xcu/)) ccfg=$(mktmp items=$(CONTENTHANDLERS_4$(@:b):b:t",":s/.xcu/))
     +-@$(RM) $(BIN)$/$(@:b)_$(INPATH).zip
     +@zip -j $(BIN)$/$(@:b)_$(INPATH).zip $(MISC)$/$(@:b)_*.xcu
     +@$(RM) $(MISC)$/$(@:b)_*.xcu
     +-@$(RM) $@
     +@$(RENAME) $(BIN)$/$(@:b)_$(INPATH).zip $@

$(INTERNALFILTERPACKAGES) : $$(ALL_4$$(@:b))
     +@echo -------------------------------------------------------------------
     +@echo Building special packages $@
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_types.xcu   xmlpackage=Types  tcfg=$(mktmp items=$(TYPES_4$(@:b):b:t",":s/.xcu/))
     $(MERGE) fragmentsdir=. tempdir=$(TEMP) outdir=$(MISC) pkg=$(MISC)$/$(@:b)_filters.xcu xmlpackage=GraphicFilter fcfg=$(mktmp items=$(FILTERS_4$(@:b):b:t",":s/.xcu/)) subdir_filters=internalgraphicfilters
     +-@$(RM) $(BIN)$/$(@:b)_$(INPATH).zip
     +@zip -j $(BIN)$/$(@:b)_$(INPATH).zip $(MISC)$/$(@:b)_*.xcu
     +@$(RM) $(MISC)$/$(@:b)_*.xcu
     +-@$(RM) $@
     +@$(RENAME) $(BIN)$/$(@:b)_$(INPATH).zip $@

# -----------------------------------------------------------------------------
# TODO move files to the common tree ... 
#      but dont forget to patch targets and scp module!
# -----------------------------------------------------------------------------
#    +@$(RM) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(@:b)_$(INPATH).zip
#    +@zip -j $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(@:b)_$(INPATH).zip $(MISC)$/$(@:b)_*.xcu
#    +@$(RM) $(MISC)$/$(@:b)_*.xcu
#    +@$(RM) $@
#    +@$(RENAME) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(@:b)_$(INPATH).zip $@
#    +@$(RM) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(@:b)_$(INPATH).zip
     
