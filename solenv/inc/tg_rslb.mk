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



#######################################################
# unroll begin

.IF "$(RESLIB$(TNR)TARGETN)"!=""

$(RSC_MULTI$(TNR)) : \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)TARGETN) \
        $(RESLIB$(TNR)BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB$(TNR)NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARSRC)/$(RSCDEFIMG)/$(PRJNAME) $(RESLIB$(TNR)IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARSRC)/$(RSCDEFIMG)/res/$i -lip=$(SOLARSRC)/$(RSCDEFIMG)/res \
    -lip=$(SOLARSRC)/$(RSCDEFIMG)/$i -lip=$(SOLARSRC)/$(RSCDEFIMG) ) \
    -subMODULE=$(SOLARSRC)/$(RSCDEFIMG) \
    -subGLOBALRES=$(SOLARSRC)/$(RSCDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB$(TNR)NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARSRC)/$(RSCDEFIMG)/$(PRJNAME) $(RESLIB$(TNR)IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARSRC)/$(RSCDEFIMG)/res/$i -lip=$(SOLARSRC)/$(RSCDEFIMG)/res \
    -lip=$(SOLARSRC)/$(RSCDEFIMG)/$i -lip=$(SOLARSRC)/$(RSCDEFIMG) ) \
    -subMODULE=$(SOLARSRC)/$(RSCDEFIMG) \
    -subGLOBALRES=$(SOLARSRC)/$(RSCDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI$(TNR))
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI$(TNR))
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI$(TNR))
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI$(TNR))
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# unroll end
#######################################################

