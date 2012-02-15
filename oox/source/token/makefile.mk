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



PRJ=..$/..

PRJNAME=oox
TARGET=token

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/namespacemap.obj \
    $(SLO)$/propertynames.obj \
    $(SLO)$/tokenmap.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
.IF "$(GPERF)" == "" || !DEFINED $(GPERF)
GPERF=gperf
.ENDIF

GENHEADERPATH = $(INCCOM)$/oox$/token

$(MISC)$/tokenhash.gperf $(INCCOM)$/tokennames.inc $(GENHEADERPATH)$/tokens.hxx $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx :
    @@noop $(assign do_phony:=.PHONY)

$(SLO)$/tokenmap.obj : $(INCCOM)$/tokenhash.inc $(INCCOM)$/tokennames.inc $(GENHEADERPATH)$/tokens.hxx $(MISC)$/do_tokens

$(INCCOM)$/tokenhash.inc : $(MISC)$/tokenhash.gperf $(MISC)$/do_tokens
    $(AUGMENT_LIBRARY_PATH) $(GPERF) --compare-strncmp $(MISC)$/tokenhash.gperf | $(SED) -e "s/(char\*)0/(char\*)0, 0/g" | $(GREP) -v "^#line" >$(INCCOM)$/tokenhash.inc

$(MISC)$/do_tokens $(do_phony) : tokens.txt tokens.pl tokens.hxx.head tokens.hxx.tail $(GENHEADERPATH)$/tokens.hxx $(INCCOM)$/tokennames.inc $(MISC)$/tokenhash.gperf
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) tokens.pl tokens.txt $(MISC)$/tokenids.inc $(INCCOM)$/tokennames.inc $(MISC)$/tokenhash.gperf && $(TYPE) tokens.hxx.head $(MISC)$/tokenids.inc tokens.hxx.tail > $(GENHEADERPATH)$/tokens.hxx && $(TOUCH) $@

$(SLO)$/namespacemap.obj : $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx $(MISC)$/do_namespaces

$(MISC)$/do_namespaces $(do_phony) : namespaces.txt namespaces.pl namespaces.hxx.head namespaces.hxx.tail $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt $(GENHEADERPATH)$/namespaces.hxx
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) namespaces.pl namespaces.txt $(MISC)$/namespaceids.inc $(INCCOM)$/namespacenames.inc $(MISC)$/namespaces.txt && $(TYPE) namespaces.hxx.head $(MISC)$/namespaceids.inc namespaces.hxx.tail > $(GENHEADERPATH)$/namespaces.hxx && $(TOUCH) $@

$(SLO)$/propertynames.obj : $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx $(MISC)$/do_properties

$(MISC)$/do_properties $(do_phony) : properties.txt properties.pl properties.hxx.head properties.hxx.tail $(INCCOM)$/propertynames.inc $(GENHEADERPATH)$/properties.hxx
    @@-$(RM) $@
    $(MKDIRHIER) $(GENHEADERPATH)
    $(PERL) properties.pl properties.txt $(MISC)$/propertyids.inc $(INCCOM)$/propertynames.inc && $(TYPE) properties.hxx.head $(MISC)$/propertyids.inc properties.hxx.tail > $(GENHEADERPATH)$/properties.hxx && $(TOUCH) $@
