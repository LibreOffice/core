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



.IF "$(L10NPROPERTYFILES)"!=""
ALLTAR : $(MISC)/$(TARGET).pmerge.mk $(L10NPROPERTYFILES)

$(MISC)/$(TARGET).pmerge.mk :  $(L10NPROPERTYFILES)

$(L10NPROPERTYFILES) : $(LOCALIZESDF)

.INCLUDE .IGNORE : $(MISC)/$(TARGET).pmerge.mk

.IF "$(alllangiso)"!="$(last_merge)"
MERGEPHONY:=.PHONY
.ENDIF          # "$(alllangiso)" != "$(last_merge)"

$(CLASSDIR)/$(PACKAGE)/%.properties $(MERGEPHONY) : %.properties
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)"==""
    $(COMMAND_ECHO)$(COPY) $< $@
.ELSE          # "$(WITH_LANG)"==""
    @@-$(RM) $@
    $(COMMAND_ECHO)@noop $(assign PMERGELIST+:=$(<:f))
    $(COMMAND_ECHO)@noop $(assign PDESTDIR:=$(@:d))
.ENDIF          # "$(WITH_LANG)"==""

$(MISC)/$(TARGET).pmerge.mk : $(PMERGELIST)
.IF "$(WITH_LANG)"!=""
# jpropex command file requirements:
# - one file per line
# - no spaces
# - no empty lines
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign PMERGEFILELIST:=$(uniq $(PMERGELIST)))
    $(COMMAND_ECHO)$(SOLARBINDIR)/jpropex -p $(PRJNAME) -r $(PRJ) -o $(PDESTDIR) -i @$(mktmp $(PMERGEFILELIST:t"\n":s/ //)) -l all -lf $(alllangiso:s/ /,/) -m $(LOCALIZESDF)
.ENDIF          # "$(WITH_LANG)"!=""
    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@

.ENDIF          # "$(L10NPROPERTYFILES)"!=""


