#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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



