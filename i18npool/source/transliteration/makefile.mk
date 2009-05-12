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
# $Revision: 1.9 $
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
TARGET=transliterationImpl

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
                        $(SLO)$/transliteration_body.obj \
                        $(SLO)$/transliteration_caseignore.obj \
                        $(SLO)$/transliterationImpl.obj \
                        $(SLO)$/transliteration_commonclass.obj \
                        $(SLO)$/transliteration_OneToOne.obj \
                        $(SLO)$/transliteration_Ignore.obj \
                        $(SLO)$/transliteration_Numeric.obj \
                        $(SLO)$/hiraganaToKatakana.obj \
                        $(SLO)$/katakanaToHiragana.obj \
                        $(SLO)$/ignoreKana.obj \
                        $(SLO)$/halfwidthToFullwidth.obj \
                        $(SLO)$/fullwidthToHalfwidth.obj \
                        $(SLO)$/ignoreWidth.obj \
                        $(SLO)$/smallToLarge_ja_JP.obj \
                        $(SLO)$/largeToSmall_ja_JP.obj \
                        $(SLO)$/ignoreSize_ja_JP.obj \
                        $(SLO)$/ignoreMinusSign_ja_JP.obj \
                        $(SLO)$/ignoreIterationMark_ja_JP.obj \
                        $(SLO)$/ignoreTraditionalKana_ja_JP.obj \
                        $(SLO)$/ignoreTraditionalKanji_ja_JP.obj \
                        $(SLO)$/ignoreProlongedSoundMark_ja_JP.obj \
                        $(SLO)$/ignoreZiZu_ja_JP.obj \
                        $(SLO)$/ignoreBaFa_ja_JP.obj \
                        $(SLO)$/ignoreTiJi_ja_JP.obj \
                        $(SLO)$/ignoreHyuByu_ja_JP.obj \
                        $(SLO)$/ignoreSeZe_ja_JP.obj \
                        $(SLO)$/ignoreIandEfollowedByYa_ja_JP.obj \
                        $(SLO)$/ignoreKiKuFollowedBySa_ja_JP.obj \
                        $(SLO)$/ignoreSeparator_ja_JP.obj \
                        $(SLO)$/ignoreSpace_ja_JP.obj \
                        $(SLO)$/ignoreMiddleDot_ja_JP.obj \
                        $(SLO)$/textToPronounce_zh.obj \
                        $(SLO)$/numtochar.obj \
                        $(SLO)$/numtotext_cjk.obj \
                        $(SLO)$/chartonum.obj \
                        $(SLO)$/texttonum.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk



