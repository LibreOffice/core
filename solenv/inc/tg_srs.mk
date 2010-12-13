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

.IF "$(SRS$(TNR)NAME)"!=""

$(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr: $(LOCALIZE_ME_DEST)

$(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr: $(SRC$(TNR)FILES)
    @echo "Making:   " $(@:f)
    @@-$(RM) $(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr
    $(COMMAND_ECHO)$(RSC) $(VERBOSITY) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)/$(SRS$(TNR)NAME).srs} -fo=$@ -p=$(TARGET) $(SRC$(TNR)FILES)

.IF "$(WITH_LANG)"!=""

$(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF)  
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)-$(MKDIRHIER)  $(COMMONMISC)$/$(PRJNAME)
    $(COMMAND_ECHO)$(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

$(SRS)/$(SRS$(TNR)NAME).srs: $(LOCALIZE_ME_DEST)

$(SRS)/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)/$(TARGET)/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)/$(SRS$(TNR)NAME).srs: $(SRC$(TNR)FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS$(TNR)NAME)"!=""

# unroll end
#######################################################


