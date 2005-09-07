#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:07:53 $
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

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
        $(SLO)$/collatorImpl.obj \
        $(SLO)$/chaptercollator.obj \
        $(SLO)$/collator_unicode.obj \
        $(SLO)$/collator_zh_pinyin.obj \
        $(SLO)$/collator_dz_charset.obj \
        $(SLO)$/collator_zh_radical.obj \
        $(SLO)$/collator_zh_stroke.obj \
        $(SLO)$/collator_zh_charset.obj \
        $(SLO)$/collator_zh_zhuyin.obj \
        $(SLO)$/collator_zh_TW_radical.obj \
        $(SLO)$/collator_zh_TW_stroke.obj \
        $(SLO)$/collator_zh_TW_charset.obj \
        $(SLO)$/collator_ne_charset.obj \
        $(SLO)$/collator_ko_charset.obj \
        $(SLO)$/collator_km_charset.obj \
        $(SLO)$/collator_ja_charset.obj \
        $(SLO)$/collator_ja_phonetic_alphanumeric_first.obj \
        $(SLO)$/collator_ja_phonetic_alphanumeric_last.obj


APP1TARGET = gencoll_rule

APP1OBJS   = $(OBJ)$/gencoll_rule.obj

APP1STDLIBS = $(SALLIB) \
        $(TOOLSLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


data/%.txt : $(APP1TARGETN)
    +@echo dummy
$(MISC)$/collator_%.cxx : data/%.txt
    $(APP1TARGETN)$ $< $@ $*



