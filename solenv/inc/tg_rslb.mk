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
# unroll begin

.IF "$(RESLIB$(TNR)TARGETN)"!=""

$(RSC_MULTI$(TNR)) : \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)TARGETN) \
        $(RESLIB$(TNR)BMPS)
    @echo Compiling: $(@:f)
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB$(TNR)NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SRC_ROOT)/$(RSCDEFIMG)/$(PRJNAME) $(RESLIB$(TNR)IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SRC_ROOT)/$(RSCDEFIMG)/res/$i -lip=$(SRC_ROOT)/$(RSCDEFIMG)/res ) \
    -subMODULE=$(SRC_ROOT)/$(RSCDEFIMG) \
    -subGLOBALRES=$(SRC_ROOT)/$(RSCDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo "Making:   " $(@:f)
.IF "$(OS)"!="WNT"
    @@-$(RM) $(RSC_MULTI$(TNR))
.ELSE				# "$(OS)"!="WNT"
    @@-$(RM) $(RSC_MULTI$(TNR))
.ENDIF              # "$(OS)"!="WNT"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# unroll end
#######################################################

