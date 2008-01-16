#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: ihi $ $Date: 2008-01-16 14:32:22 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

