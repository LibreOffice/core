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
# $(uniq ...) to workaround $assign adding the value twice...
    @noop $(assign PMERGEFILELIST:=$(uniq $(PMERGELIST)))
.ENDIF          # "$(WITH_LANG)"!=""
    @-$(RM) $@
    $(COMMAND_ECHO)echo last_merge=$(alllangiso) > $@

.ENDIF          # "$(L10NPROPERTYFILES)"!=""


