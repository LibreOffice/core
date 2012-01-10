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



# to make it usable with special lang selection of
# helpcontent2

aux_alllangiso*:=$(alllangiso)

SHELL_PACKAGE:=$(subst,/,$/ $(PACKAGE))

.IF "$(XHPDEST)"==""
XHPDEST*:=$(COMMONMISC)
HELP_OUT:=$(COMMONMISC)
.ELSE          # "$(XHPDEST)"==""
HELP_OUT:=$(MISC)
.ENDIF          # "$(XHPDEST)"==""

HLANGXHPFILES*:=$(foreach,i,$(XHPFILES) $(foreach,j,$(aux_alllangiso) $(XHPDEST)$/$j$/$(SHELL_PACKAGE)$/$(i:f)))

ALLTAR : $(HELP_OUT)$/$(TARGET).done $(HELP_OUT)$/xhp_changed.flag optix

$(HLANGXHPFILES) : $$(@:d)thisdir.created

$(XHPDEST)$/{$(aux_alllangiso)}$/$(SHELL_PACKAGE)$/%.xhp :| %.xhp
    @$(TOUCH) $@
# internal dependencies not sufficient to trigger merge?
#    @$(NULL)


$(HELP_OUT)$/$(TARGET).done : $(HLANGXHPFILES)
.IF "$(WITH_LANG)"!=""
    $(AUGMENT_LIBRARY_PATH) $(HELPEX) -p $(PRJNAME) -r $(PRJ) -i @$(mktmp $(uniq $(foreach,i,$? $(!eq,$(i:f),$(i:f:s/.xhp//) $(i:f) $(XHPFILES))))) -x $(XHPDEST) -y $(SHELL_PACKAGE) -l all -lf $(aux_alllangiso:t",") -m $(LOCALIZESDF) && $(TOUCH) $@
.ELSE			# "$(WITH_LANG)"!=""
    cp $(uniq $(foreach,i,$? $(!eq,$(i:f),$(i:f:s/.xhp//) $(i:f) $(XHPFILES)))) $(XHPDEST)$/en-US$/$(SHELL_PACKAGE) && $(TOUCH) $@
.ENDIF			# "$(WITH_LANG)"!=""
.IF "$(OS)"=="SOLARIS"
    @$(ECHONL) " "
.ELSE			# "$(OS)"=="SOLARIS"
    @$(ECHONL)
.ENDIF			# "$(OS)"=="SOLARIS"

$(HELP_OUT)$/xhp_changed.flag : $(HLANGXHPFILES)
    @$(TOUCH) $@

# urks - dmake mixes up operators and strings :-(
.IF "$(HLANGXHPFILES:s/defined/xxx/)"!=""

.IF "$(HELPTRANSPHONY)"!=""
$(HELP_OUT)$/$(TARGET).done .PHONY :
.ELSE           # "$(HELPTRANSPHONY)"!=""

$(HELP_OUT)$/$(TARGET).done : makefile.mk
.IF "$(WITH_LANG)"!=""
$(HELP_OUT)$/$(TARGET).done : $(LOCALIZESDF)
.ENDIF			# "$(WITH_LANG)"!=""
.ENDIF          # "$(HELPTRANSPHONY)"!=""
.ENDIF          # "$(HLANGXHPFILES)"!=""

optix: $(HELP_OUT)$/$(TARGET).done
    @echo done

%.created :
    @@-$(MKDIRHIER) $(@:d)
    @$(TOUCH) $@


