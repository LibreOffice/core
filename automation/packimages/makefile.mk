#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:15:20 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    +$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/stt*.ilst $(MISC) 
    +$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/svt*.ilst $(MISC) 
    +$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/fps*.ilst $(MISC) 
    +$(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/vcl*.ilst $(MISC) 
    +-$(MKDIR) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))
    +$(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(MISC) -o $@

