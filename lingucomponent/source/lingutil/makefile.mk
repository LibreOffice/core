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

PRJ = ..$/..

PRJNAME	= lingucomponent
TARGET	= ulingu
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS += -I$(SOLARINCDIR)$/hunspell -DHUNSPELL_STATIC
.ENDIF

.IF "$(SYSTEM_DICTS)" == "YES"
CXXFLAGS += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
CFLAGSCXX += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
CFLAGSCC += -DSYSTEM_DICTS -DDICT_SYSTEM_DIR=\"$(DICT_SYSTEM_DIR)\" -DHYPH_SYSTEM_DIR=\"$(HYPH_SYSTEM_DIR)\" -DTHES_SYSTEM_DIR=\"$(THES_SYSTEM_DIR)\"
.ENDIF

CXXFLAGS += $(HUNSPELL_CFLAGS)
CFLAGSCXX += $(HUNSPELL_CFLAGS)
CFLAGSCC += $(HUNSPELL_CFLAGS)

SLOFILES = $(SLO)$/lingutil.obj

LIB1TARGET= $(SLB)$/lib$(TARGET).lib
LIB1ARCHIV= $(LB)/lib$(TARGET).a
LIB1OBJFILES= $(SLOFILES)


# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
