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
PRJ=..
PRJNAME=automation
TARGET=packimages
RSCCUSTOMIMG*=$(PRJ)$/util

# point to an existing directory as default that poses no threat
# to a "find" that looks for "*.png"
RSCCUSTOMIMG*=$(PRJNAME)$/util

# point to an existing directory as default that poses no threat
# to a "find" that looks for "*.png"
RSCCUSTOMIMG*=$(PRJNAME)$/util

.INCLUDE:  settings.mk

.INCLUDE: target.mk

ALLTAR : \
    $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/images_tt.zip

#solarenv = j:\so-cwsserv04\gh7\SRC680\src.m50\solenv
#outpath = wntmsci10
#common_outdir = common
#solarresdir = j:\so-cwsserv04\gh7\SRC680\wntmsci10\res.m50


$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/images_tt.zip .PHONY:
    @echo ------------------------------
    @echo Making: $@
    $(COMMAND_ECHO)$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/stt*.ilst $(MISC)
    $(COMMAND_ECHO)$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/svt*.ilst $(MISC)
    $(COMMAND_ECHO)$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/fps*.ilst $(MISC)
    $(COMMAND_ECHO)$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/vcl*.ilst $(MISC)
    $(COMMAND_ECHO)$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/sb*.ilst $(MISC)
    @-$(MKDIR) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN)) >& $(NULLDEV)
    $(COMMAND_ECHO)$(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(MISC) -o $@
