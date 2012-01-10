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




.IF "$(VERBOSE)"=="FALSE"
ZIP_VERBOSITY=-q
.ENDIF

.IF "$(JARTARGETN)"!=""

.IF "$(JARCOMPRESS)"==""
JARCOMPRESS_FLAG=0
.ENDIF

.IF "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) : $(JARMANIFEST) $(JAVACLASSFILES) $(JAVATARGET)
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(NEW_JAR_PACK)"!=""
.IF "$(JARMANIFEST)"!=""

.IF "$(CUSTOMMANIFESTFILE)"!=""

CUSTOMMANIFESTFILEDEP:=$(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f)

$(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f) : $(subst,/,/ $(DMAKE_WORK_DIR))/$(CUSTOMMANIFESTFILE)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(COPY) $< $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""

$(JARMANIFEST) .PHONY : $(CUSTOMMANIFESTFILEDEP)
    @@-$(MKDIRHIER) $(@:d)
    @@-$(RM) $@
    $(COMMAND_ECHO)echo Manifest-Version: 1.0 > $@
.IF "$(JARCLASSPATH)" != ""
    $(COMMAND_ECHO)echo $(USQ)Class-Path: $(JARCLASSPATH)$(USQ) >> $@
.ENDIF
# $(RSCREVISION) contains chars that must be quoted (for *NIX shells)
    $(COMMAND_ECHO)echo $(USQ)Solar-Version: $(RSCREVISION)$(USQ) >> $@
.IF "$(CUSTOMMANIFESTFILE)"!=""
    $(COMMAND_ECHO)$(TYPE) $(MISC)/$(TARGET)_$(CUSTOMMANIFESTFILE:f) >> $@
.ENDIF			# "$(CUSTOMMANIFESTFILE)"!=""
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF			# "$(NEW_JAR_PACK)"!=""

.IF "$(JARCLASSEXCLUDES)" == ""
ZIPJAR_EXCLUDE=
.ELSE   # "$(JARCLASSEXCLUDES)" == ""
ZIPJAR_EXCLUDE=$(foreach,i,$(JARCLASSEXCLUDES) -x "$i")
# format:
# JARCLASSEXCLUDES=foo/* bar/*
.ENDIF  # "$(JARCLASSEXCLUDES)" == ""

#
# build jar 
#
.IF "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
$(JARTARGETN) .PHONY :
#  $(JARMANIFEST)
.ELSE			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.DIRCACHE = no
$(JARTARGETN) :
#$(JARTARGETN) .SETDIR=$(CLASSDIR) .SEQUENTIAL : $(JARTARGETDEP) $(shell @-cat -s $(MISC)/$(JARTARGETN).dep )
.ENDIF			# "$(NOJARDEP)"!="" || "$(NEW_JAR_PACK)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @-find . -type d -user $(USER) ! -perm -5 -print | xargs test "$$1" != "" && chmod +r $$1 
.ENDIF
.IF "$(JARMANIFEST)"!=""
    $(COMMAND_ECHO)cd $(CLASSDIR)/$(TARGET) && zip $(ZIP_VERBOSITY) -u -rX ../$(@:f) $(subst,$(CLASSDIR)/$(TARGET)/, $(JARMANIFEST)) $(ZIPJAR_EXCLUDE) $(CHECKZIPRESULT)
.ENDIF			# "$(JARMANIFEST)"!=""
    $(COMMAND_ECHO)cd $(CLASSDIR) && zip $(ZIP_VERBOSITY) -u -rX $(@:f) $(subst,\,/ $(JARCLASSDIRS)) $(ZIPJAR_EXCLUDE) $(CHECKZIPRESULT)
.ENDIF

