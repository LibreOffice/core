#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.1 $
#*
#*  last change: $Author: bustamam $ $Date: 2002-03-26 13:23:20 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
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
                        $(SLO)$/oneToOneMapping.obj \
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
                        $(SLO)$/numtotext_cjk.obj

# MACOSX: manually initialization some static data members
.IF "$(OS)"=="MACOSX"
SLOFILES+=$(SLO)$/staticmbtransliteration.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk



