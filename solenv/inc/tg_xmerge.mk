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



.IF "$(XTXFILES)"!=""

L10NEXT*=.txt
XTX_TARGET_PATH*=$(MISC)/$(TARGET)

ALLTAR : $(XTX_TARGET_PATH).xtx.pmerge.mk $(XTXFILES)

$(XTX_TARGET_PATH).xtx.pmerge.mk :  $(XTXFILES)

$(XTXFILES) : $(LOCALIZESDF)

.INCLUDE .IGNORE : $(XTX_TARGET_PATH).xtx.pmerge.mk

.IF "$(alllangiso)"!="$(last_merge)"
XTXMERGEPHONY:=.PHONY
.ENDIF          # "$(alllangiso)" != "$(last_merge)"

$(XTX_TARGET_PATH)/%$(L10NEXT) $(XTXMERGEPHONY) : %.xtx
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)"==""
    $(COMMAND_ECHO)$(COPY) $< $(@:d)$(@:b)_en-US$(L10NEXT)
.ELSE          # "$(WITH_LANG)"==""
    @@-$(RM) $@
    $(COMMAND_ECHO)@noop $(assign XTXMERGELIST+:=$(<:f))
    $(COMMAND_ECHO)@noop $(assign XTXDESTDIR:=$(@:d))
.ENDIF          # "$(WITH_LANG)"==""

$(XTX_TARGET_PATH).xtx.pmerge.mk : $(XTXMERGELIST)
.IF "$(WITH_LANG)"!=""
# xtxex command file requirements:
# - one file per line
# - no spaces
# - no empty lines
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign XTXMERGEFILELIST:=$(uniq $(XTXMERGELIST)))
    $(COMMAND_ECHO) $(SOLARBINDIR)/xtxex -p $(PRJNAME) -r $(PRJ) -o $(XTXDESTDIR) -i @$(mktmp $(XTXMERGEFILELIST:t"\n":s/ //)) -l $(alllangiso:s/ /,/) -f $(alllangiso:s/ /,/) -m $(LOCALIZESDF) -s"[path]/[fileNoExt]_[language]$(L10NEXT)" 
.ENDIF          # "$(WITH_LANG)"!=""

    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@
.ENDIF          # "$(XTXFILES)"!=""

.IF "$(TREEFILE)"!=""
ALLTAR : $(MISC)/$(TARGET).tree.pmerge.mk $(TREEFILE)

$(MISC)/$(TARGET).tree.pmerge.mk :  $(TREEFILE)

$(TREEFILE) : $(LOCALIZESDF)

.INCLUDE .IGNORE : $(MISC)/$(TARGET).tree.pmerge.mk

.IF "$(alllangiso)"!="$(last_merge)"
TREEMERGEPHONY:=.PHONY
.ENDIF          # "$(alllangiso)" != "$(last_merge)"

$(OUT_HELP)/en-US/help.tree $(TREEMERGEPHONY) : help.tree
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)"==""
    $(COMMAND_ECHO)$(COPY) $< $@
.ELSE          # "$(WITH_LANG)"==""
    @@-$(RM) $@
    $(COMMAND_ECHO)@noop $(assign TREEMERGELIST+:=$(<:f))
    $(COMMAND_ECHO)@noop $(assign TREEDESTDIR:=$(@:d:d:d))
.ENDIF          # "$(WITH_LANG)"==""

$(MISC)/$(TARGET).tree.pmerge.mk : $(TREEMERGELIST)
.IF "$(WITH_LANG)"!=""
# xtxex command file requirements:
# - one file per line
# - no spaces
# - no empty lines
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign TREEMERGEFILELIST:=$(uniq $(TREEMERGELIST)))
# Variables for the pattern filename,fileNoExt,language,extension,pathPrefix,pathPostFix,path
    $(COMMAND_ECHO) $(SOLARBINDIR)/xhtex -p $(PRJNAME) -r $(PRJ) -o $(TREEDESTDIR) -i @$(mktmp $(TREEMERGEFILELIST:t"\n":s/ //)) -l $(alllangiso:s/ /,/) -f $(alllangiso:s/ /,/) -m $(LOCALIZESDF) -s"[path]/[language]/[fileNoExt].tree" 
.ENDIF          # "$(WITH_LANG)"!=""
    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@
.ENDIF          # "$(TREEFILE)"!=""


