#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.5 $
#*
#*  last change: $Author: hr $ $Date: 2003-07-16 17:43:39 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
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

# MACOSX: manually initialization some static data members
.IF "$(OS)$(CVER)"=="MACOSXC295"
SLOFILES+=$(SLO)$/staticmbtransliteration.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk



