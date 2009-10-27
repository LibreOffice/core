#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile,v $
#
# $Revision: 1.4 $
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
#***********************************************************************/

PRJ = ..
PRJNAME = connectivity
TARGET = connectivity

.INCLUDE : settings.mk
.INCLUDE : target.mk

# For any given platform, for each driver .xcu (in $(MY_XCUS)) built on that
# platform (in $(MISC)/registry/data/org/openoffice/Office/DataAccess) there are
# corresponding language-specific .xcu files (in
# $(MISC)/registry/res/%/org/openoffice/Office/DataAccess).  For each language,
# all language-specific .xcu files for that language are assembled into
# $(BIN)$/fcfg_drivers_%.zip.  To meet the requirements of dmake percent rules,
# the first item from $(MY_XCUS) is arbitrarily taken to be the main
# prerequisite while all the items from $(MY_XCUS) are made into indirect
# prerequisites (harmlessly doubling the first item).

MY_XCUS := \
    $(shell cd $(MISC)/registry/data/org/openoffice/Office/DataAccess && \
    ls *.xcu)

.IF "$(MY_XCUS)" != ""

ALLTAR : $(BIN)/fcfg_drivers_{$(alllangiso)}.zip

$(BIN)/fcfg_drivers_{$(alllangiso)}.zip : \
        $(MISC)/registry/res/$$(@:b:s/fcfg_drivers_//)/org/openoffice/Office/DataAccess/{$(MY_XCUS)}

$(BIN)/fcfg_drivers_%.zip : \
        $(MISC)/registry/res/%/org/openoffice/Office/DataAccess/$(MY_XCUS:1)
    zip -j $@ \
        $(foreach,i,$(MY_XCUS) \
            $(MISC)/registry/res/$*/org/openoffice/Office/DataAccess/$i)

.ENDIF
