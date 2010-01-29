#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.22 $
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

PRJ     = ..
TARGET  = connectivity
PRJNAME = connectivity

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE :  settings.mk

DIR_FILTERCFGOUT := $(MISC)$/drivers
DIR_LANGPACK     := $(DIR_FILTERCFGOUT)
.IF "$(WITH_LANG)"!=""
DIR_LANG_SOURCE  := $(MISC)$/merge
.ELSE
DIR_LANG_SOURCE  := $(MISC)$/registry$/data
.ENDIF
DRIVER_MERGE_XCU := $(shell -@$(FIND) $(DIR_LANG_SOURCE)$/org$/openoffice$/Office$/DataAccess -name "*.xcu")
   
REALFILTERPACKAGES_FILTERS_UI_LANGPACKS = \
    $(foreach,i,$(alllangiso) $(foreach,j,$(DRIVER_MERGE_XCU) $(DIR_LANGPACK)$/$i$/org$/openoffice$/Office$/DataAccess$/$(j:f)))

.INCLUDE: target.mk

PACKLANG := $(XSLTPROC) --nonet
PACKLANG_IN :=
PACKLANG_PARAM := --stringparam
PACKLANG_XSL :=
    
$(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS) : 
      @echo ===================================================================
      @echo Building language package for driver $(@:b:s/Filter_//) 
      @echo ===================================================================
      +-$(MKDIRHIER) $(@:d)
      $(PACKLANG) $(PACKLANG_PARAM) lang $(@:d:d:d:d:d:d:d:d:d:d:b) $(PACKLANG_XSL) langfilter.xsl $(PACKLANG_IN) $(DIR_LANG_SOURCE)$/org$/openoffice$/Office$/DataAccess$/$(@:f) > $@

$(MISC)$/$(TARGET)_delzip :
    -$(RM) $(BIN)$/fcfg_drivers_{$(alllangiso)}.zip	

$(BIN)$/fcfg_drivers_{$(alllangiso)}.zip : $(REALFILTERPACKAGES_FILTERS_UI_LANGPACKS)
    cd $(DIR_FILTERCFGOUT)$/$(@:b:s/fcfg_drivers_//) && zip -ru ..$/..$/..$/bin$/fcfg_drivers_$(@:b:s/fcfg_drivers_//).zip org/*
.IF "$(USE_SHELL)"!="4nt"
    $(PERL) -w $(SOLARENV)$/bin$/cleanzip.pl $@
.ENDIF			# "$(USE_SHELL)"!="4nt"

ALLTAR: \
    $(MISC)$/$(TARGET)_delzip \
    $(BIN)$/fcfg_drivers_{$(alllangiso)}.zip

