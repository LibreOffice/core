#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************


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
    $(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/stt*.ilst $(MISC) 
    $(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/svt*.ilst $(MISC) 
    $(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/fps*.ilst $(MISC) 
    $(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/vcl*.ilst $(MISC) 
    $(COPY) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SOLARRESDIR))$/img$/sb*.ilst $(MISC) 
    -$(MKDIR) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))
    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(MISC) -o $@

