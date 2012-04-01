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

#aux_alllangiso*:=$(foreach,i,$(alllangiso) $(foreach,j,$(aux_langdirs) $(eq,$i,$j  $i $(NULL))))
aux_alllangiso*:=$(alllangiso)

#########################################################


HELPLINKALLTARGETS=$(foreach,i,$(aux_alllangiso) $(MISC)/$(TARGET)_$(LINKNAME)_$i.done)
HELPLINKALLADDEDDEPS=$(foreach,i,$(aux_alllangiso) $(subst,LANGUAGE,$i $(LINKADDEDDPES)))

ALLTAR : $(HELPLINKALLTARGETS)

XSL_DIR*:=$(SOLARBINDIR)
XHPLINKSRC*:=$(XHPDEST)
STY_SWITCH:= -sty $(XSL_DIR)/embed.xsl

.IF "$(XHPFILES)"!="" && "$(HELP_OUT)"!=""
$(foreach,i,$(LINKLINKFILES) $(XHPLINKSRC)/{$(aux_alllangiso)}/$i) : $(HELP_OUT)/$(TARGET).done
LINKDEPS+=$(HELP_OUT)/xhp_changed.flag
.ENDIF          # "$(XHPFILES)"!="" && "$(HELP_OUT)"!=""

$(HELPLINKALLTARGETS) : $(foreach,i,$(LINKLINKFILES) $(XHPLINKSRC)/$$(@:b:s/_/./:e:s/.//)/$i) $(subst,LANGUAGE,$$(@:b:s/_/./:e:s/.//) $(LINKDEPS))
    @echo Building help index for $(@:b:s/_/./:e:s/.//)
    $(COMMAND_ECHO)$(HELPLINKER) -mod $(LINKNAME) -extlangsrc $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} $(STY_SWITCH) -extlangdest $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} -idxcaption $(XSL_DIR)/idxcaption.xsl -idxcontent $(XSL_DIR)/idxcontent.xsl $(LINKLINKFILES)
    $(COMMAND_ECHO)cd $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} && zip -u -r $(LINKNAME).jar $(PACKAGE)/* $(CHECKZIPRESULT)
# cleanup index dir
    -$(RM) $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))}/$(LINKNAME).idxl/*
    $(HELPINDEXER) -lang $(@:b:s/_/./:e:s/.//) -mod $(LINKNAME) -dir $(XHPLINKSRC)/{$(subst,$(TARGET)_$(LINKNAME)_, $(@:b))} && $(TOUCH) $@
    -$(RM) $(XHPLINKSRC)/$(@:b:s/_/./:e:s/.//)/content/*.*
    -$(RMDIR) $(XHPLINKSRC)/$(@:b:s/_/./:e:s/.//)/content
    -$(RM) $(XHPLINKSRC)/$(@:b:s/_/./:e:s/.//)/caption/*.*
    -$(RMDIR) $(XHPLINKSRC)/$(@:b:s/_/./:e:s/.//)/caption
