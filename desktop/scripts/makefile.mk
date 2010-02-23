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

PRJ=..

PRJNAME=desktop
TARGET=scripts

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Targets -------------------------------------------------------------

UNIXTEXT= \
    $(MISC)$/sbase.sh \
    $(MISC)$/scalc.sh \
    $(MISC)$/sdraw.sh \
    $(MISC)$/simpress.sh \
    $(MISC)$/smaster.sh \
    $(MISC)$/smath.sh \
    $(MISC)$/sweb.sh \
    $(MISC)$/swriter.sh \
    $(MISC)$/mozwrapper.sh \
    $(MISC)$/unoinfo.sh \
    $(MISC)$/unopkg.sh

.IF "$(OS)" != "MACOSX"

ALLTAR: $(MISC)$/soffice.sh-expanded

$(MISC)/soffice.sh-expanded: $(MISC)/soffice.sh
.IF "$(OS)" == "LINUX" && "$(CPUNAME)" == "POWERPC"
    $(SED) 's/^#@# //' $< > $@
.ELSE
    $(COPY) $< $@
.ENDIF

UNIXTEXT+= $(MISC)$/soffice.sh

.ENDIF

.INCLUDE :  target.mk
