#*************************************************************************
#*
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 17:33:17 $
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
TARGET=transliterationImpl

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk

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



