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

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SCP$(TNR)TARGETN)"!=""

# try to get missing parfiles
$(PAR)/%.par : $(SOLARPARDIR)/%.par
    @@-$(MKDIRHIER) $(@:d:d)
    $(COMMAND_ECHO)$(COPY) $< $@

LOCALSCP$(TNR)FILES+=$(foreach,i,$(SCP$(TNR)FILES) $(foreach,j,$(SCP$(TNR)LINK_PRODUCT_TYPE) $(PAR)/$j/$i ))

$(SCP$(TNR)TARGETN): $(LOCALSCP$(TNR)FILES)
    @echo "Making:   " $(@:f)
    @@-$(MKDIRHIER) $(BIN)/$(SCP$(TNR)LINK_PRODUCT_TYPE)
    $(COMMAND_ECHO)$(SCPLINK) $(SCPLINKFLAGS) @@$(mktmp $(foreach,i,$(SCP$(TNR)FILES) $(subst,$(@:d:d:d), $(@:d:d))/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

