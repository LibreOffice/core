#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.6 $
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
PRJ=..


PRJNAME=officecfg
TARGET=util

.INCLUDE : settings.mk
.INCLUDE : target.mk

# --- Targets ------------------------------------------------------

$(MISC)$/$(TARGET)_delzip :
    -$(RM) $(BIN)$/registry_{$(alllangiso)}.zip	
    -$(RM) $(COMMON_OUTPUT)$/bin$/registry_{$(alllangiso)}.zip

$(BIN)$/registry_{$(alllangiso)}.zip : $(MISC)$/$(TARGET)_delzip
    cd $(MISC)$/registry$/res$/$(@:b:s/registry_//) && zip -ru ..$/..$/..$/..$/bin$/registry_$(@:b:s/registry_//).zip org/*
.IF "$(USE_SHELL)"!="4nt"
    $(PERL) -w $(SOLARENV)$/bin$/cleanzip.pl $@
.ENDIF			# "$(USE_SHELL)"!="4nt"

ALLTAR: \
    $(MISC)$/$(TARGET)_delzip \
    $(BIN)$/registry_{$(alllangiso)}.zip

