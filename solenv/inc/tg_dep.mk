#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(SRCFILES)$(SRC1FILES)$(SRC2FILES)$(SRC3FILES)$(RCFILES)$(HDBDEPNTARGET)$(IDLFILES)$(PARFILES)$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"!=""
ALLDEP .PHONY: 
    @-$(IFEXIST) $(SRS)/$(TARGET).*.dpr $(THEN) $(RM:s/+//) $(SRS)/$(TARGET).*.dpr >& $(NULLDEV) $(FI)
    @@-$(RM) $(MISC)/$(TARGET).dpr
    @@-$(RM) $(MISC)/$(TARGET).*.dprr
    @@-$(RM) $(MISC)/$(TARGET).dpj
    @@-$(RM) $(MISC)/$(TARGET).dpz
    @@-$(RM) $(MISC)/$(COMP1TYPELIST).mk $(MISC)/$(COMP2TYPELIST).mk $(MISC)/$(COMP3TYPELIST).mk $(MISC)/$(COMP4TYPELIST).mk $(MISC)/$(COMP5TYPELIST).mk $(MISC)/$(COMP6TYPELIST).mk $(MISC)/$(COMP7TYPELIST).mk $(MISC)/$(COMP8TYPELIST).mk $(MISC)/$(COMP9TYPELIST).mk
.IF "$(nodep)"==""
.IF "$(DEPFILE_SLO)"!=""	
    @@-$(RM) $(DEPFILE_SLO)
    $(foreach,i,$(all_local_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) > $(MISC)/s_$(i:b).dpcc ))
    $(foreach,i,$(all_misc_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) > $(MISC)/s_$(i:b).dpcc ))
.ENDIF			# "$(DEPFILE_SLO)"!=""	
.IF "$(DEPFILE_OBJ)"!=""	
    @@-$(RM) $(DEPFILE_OBJ)
    $(foreach,i,$(all_local_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $i ) > $(MISC)/o_$(i:b).dpcc ))
    $(foreach,i,$(all_misc_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $i ) > $(MISC)/o_$(i:b).dpcc ))
.ENDIF			# "$(DEPFILE_OBJ)"!=""	
.ENDIF			# "$(nodep)"==""
.IF "$(DEPFILES)" != ""
    echo xxx$(DEPFILES)xxx
    @@-$(RM) $(DEPFILES)
.ENDIF			# "$(DEPFILES)" != ""


ALLDPC: \
        $(DEPFILES) \
        $(RCFILES) \
        $(IDLFILES) \
        $(HDBDEPNTARGET)
.IF "$(nodep)"!=""
    @echo NOT-Making: Depend-Lists
    @echo ttt: ppp > $(MISC)/$(TARGET).dpc
.ELSE
    @echo Making : Dependencies 
.IF "$(CFILES)$(CXXFILES)"!=""
.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)"==""
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    @echo 		something is wrong!!
    @echo 		source files but no obj
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    force_dmake_to_error
.ENDIF
.ENDIF
.IF "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)$(PARFILES)"!=""
    @$(IFEXIST) $(MISC)/$(TARGET).dpj $(THEN) $(RM:s/+//) $(MISC)/$(TARGET).dpj >& $(NULLDEV) $(FI)
    @$(IFEXIST) $(MISC)/genjava.mk $(THEN) $(RM:s/+//) $(MISC)/genjava.mk >& $(NULLDEV) $(FI)
    $(TOUCH) $(MISC)/$(TARGET).dpc
.IF "$($(SECOND_BUILD)_SLOFILES)"!=""
    @$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_SLOFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $i )) >> $(MISC)/$(TARGET).dpc
.ENDIF
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""
    @$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_OBJFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)/$(TARGET).dpc
.ENDIF
.ELSE			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
    @echo ttt: ppp > $(MISC)/$(TARGET).dpc
.ENDIF			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
.ENDIF			# "$(nodep)"!=""
.ELSE		# anything requiring dependencies

ALLDPC:
    @echo $(EMQ)# > $(MISC)/$(TARGET).dpc

ALLDEP:
    @echo nothing to do here...

.ENDIF

