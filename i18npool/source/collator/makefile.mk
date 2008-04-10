#*************************************************************************
#*
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.18 $
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
#************************************************************************/

PRJ=..$/..

PRJNAME=i18npool
TARGET=collator

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

txtlist:=$(shell @cd data && ls *.txt)
LOCAL_RULE_LANGS:=$(uniq $(foreach,i,$(txtlist) $(i:s/-/_/:s/_/ /:1)))
rules_dependencies:=$(foreach,i,$(txtlist) data$/$i) $(INCCOM)$/lrl_include.hxx

rules_obj = $(SLO)$/collator_unicode.obj

SLOFILES=   \
        $(SLO)$/collatorImpl.obj \
        $(SLO)$/chaptercollator.obj \
        $(rules_obj)

APP1TARGET = gencoll_rule

APP1OBJS   = $(OBJ)$/gencoll_rule.obj

DEPOBJFILES = $(APP1OBJS)

APP1STDLIBS = $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(rules_obj) : $(rules_dependencies)

$(INCCOM)$/lrl_include.hxx: $(foreach,i,$(txtlist) data$/$i)
    @@$(RM) $@
    @echo $(EMQ)#define LOCAL_RULE_LANGS $(EMQ)"$(LOCAL_RULE_LANGS)$(EMQ)" >& $@

