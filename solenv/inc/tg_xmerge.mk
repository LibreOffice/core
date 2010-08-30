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


