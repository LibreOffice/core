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

MKFILENAME:=tg_merge.mk

# look for the according rules in "rules.mk"

.IF "$(WITH_LANG)"!=""
.IF "$(ULFFILES)"!=""
$(foreach,i,$(ULFFILES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF)
.ENDIF

# *.xrb merge
.IF "$(XMLPROPERTIES)"!=""
$(foreach,i,$(XMLPROPERTIES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF)
.ENDIF

# *.xrm merge
.IF "$(READMEFILES)"!=""
$(foreach,i,$(READMEFILES) $(COMMONMISC)/$(TARGET)/$(i:b).xrm) : $$(@:f) $(LOCALIZESDF)
.ENDIF

# *.xcu merge
.IF "$(LOCALIZEDFILES)"!=""
$(foreach,i,$(LOCALIZEDFILES) $(PROCESSOUT)/merge/{$(subst,.,/ $(PACKAGE))}/$(i:b).xcu) : $$(@:f) $(LOCALIZESDF) 
.ENDIF
.ENDIF # "$(WITH_LANG)"!=""

# dependencies from *.ulf to par-files
.IF "$(ULFPARFILES)"!=""
$(ULFPARFILES) : $(COMMONMISC)/$(TARGET)/$$(@:b).$(LANGFILEEXT)
.ENDIF          # "$(ULFPARFILES)"!=""

