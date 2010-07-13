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

ALLTAR : $(MISC)/$(TARGET).xtx.pmerge.mk $(XTXFILES)

.IF "$(L10NEXT)"==""
L10NEXT:=.txt
.ENDIF


$(MISC)/$(TARGET).xtx.pmerge.mk :  $(XTXFILES)

$(XTXFILES) : $(LOCALIZESDF)

.INCLUDE .IGNORE : $(MISC)/$(TARGET).xtx.pmerge.mk

.IF "$(alllangiso)"!="$(last_merge)"
XTXMERGEPHONY:=.PHONY
.ENDIF          # "$(alllangiso)" != "$(last_merge)"

$(COMMONMISC)/$(PACKAGE)/%$(L10NEXT) $(XTXMERGEPHONY) : %.xtx
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)"==""
    $(COMMAND_ECHO)$(COPY) $< $@
.ELSE          # "$(WITH_LANG)"==""
    @@-$(RM) $@
    $(COMMAND_ECHO)@noop $(assign PMERGELIST+:=$(<:f))
    $(COMMAND_ECHO)@noop $(assign PDESTDIR:=$(@:d))
.ENDIF          # "$(WITH_LANG)"==""

$(MISC)/$(TARGET).xtx.pmerge.mk : $(PMERGELIST)
.IF "$(WITH_LANG)"!=""
# xtxex command file requirements:
# - one file per line
# - no spaces
# - no empty lines
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign PMERGEFILELIST:=$(uniq $(PMERGELIST)))
#    $(COMMAND_ECHO) 
    $(SOLARBINDIR)/xtxex -p $(PRJNAME) -r $(PRJ) -o $(PDESTDIR) -i @$(mktmp $(PMERGEFILELIST:t"\n":s/ //)) -l $(alllangiso:s/ /,/) -f $(alllangiso:s/ /,/) -m $(LOCALIZESDF) -s"[path]/[fileNoExt]_[language]$(L10NEXT)" 

.ENDIF          # "$(WITH_LANG)"!=""

    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@

.ENDIF          # "$(XTXFILES)"!=""

.IF "$(XHTFILES)"!=""
ALLTAR : $(MISC)/$(TARGET).xht.pmerge.mk $(XHTFILES)

$(MISC)/$(TARGET).xht.pmerge.mk :  $(XHTFILES)

$(XHTFILES) : $(LOCALIZESDF)

.INCLUDE .IGNORE : $(MISC)/$(TARGET).xht.pmerge.mk

.IF "$(alllangiso)"!="$(last_merge)"
XHTMERGEPHONY:=.PHONY
.ENDIF          # "$(alllangiso)" != "$(last_merge)"

$(COMMONMISC)/$(PACKAGE)/en-US/%.tree $(XHTMERGEPHONY) : %.xht
    @@-$(MKDIRHIER) $(@:d)
.IF "$(WITH_LANG)"==""
    $(COMMAND_ECHO)$(COPY) $< $@
.ELSE          # "$(WITH_LANG)"==""
    @@-$(RM) $@
    $(COMMAND_ECHO)@noop $(assign PMERGELIST1+:=$(<:f))
    $(COMMAND_ECHO)@noop $(assign PDESTDIR1:=$(@:d:d:d))
.ENDIF          # "$(WITH_LANG)"==""

$(MISC)/$(TARGET).xht.pmerge.mk : $(PMERGELIST1)
.IF "$(WITH_LANG)"!=""
# xtxex command file requirements:
# - one file per line
# - no spaces
# - no empty lines
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign PMERGEFILELIST1:=$(uniq $(PMERGELIST1)))
#    $(COMMAND_ECHO) 
# filename,fileNoExt,language,extension,pathPrefix,pathPostFix,path
    $(SOLARBINDIR)/xhtex -p $(PRJNAME) -r $(PRJ) -o $(PDESTDIR1) -i @$(mktmp $(PMERGEFILELIST1:t"\n":s/ //)) -l $(alllangiso:s/ /,/) -f $(alllangiso:s/ /,/) -m $(LOCALIZESDF) -s"[path]/[language]/[fileNoExt].tree" 
.ENDIF          # "$(WITH_LANG)"!=""

    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@

.ENDIF          # "$(XHTFILES)"!=""


