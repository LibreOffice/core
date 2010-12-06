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


