/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _TXTTYPES_HXX
#define _TXTTYPES_HXX

#include "swtypes.hxx"

// Ueberpruefte USHORTs, z.B. Indizes auf Arrays oder garantiert kleine
// Integerwerte, auch von aussen vorgegebene
#define MSHORT sal_uInt16
#define MSHRT_MAX USHRT_MAX
// Koordinaten: Breite, Hoehe und Offsets in X-/Y-Richtung sowie Ascent etc.
#define KSHORT sal_uInt16
#define KSHRT_MAX USHRT_MAX

// Portiontypen
#define POR_LIN         0x0000
#define POR_FLYCNT      0x0001

#define POR_HOLE        0x0080
#define POR_TMPEND      0x0081
#define POR_BRK         0x0082
#define POR_KERN        0x0083
#define POR_ARROW       0x0084
#define POR_MULTI       0x0085
#define POR_HIDDEN_TXT  0x0086
#define POR_CONTROLCHAR 0x0087

#define POR_TXT         0x8000
#define POR_LAY         0x8001
#define POR_PARA        0x8002
#define POR_URL         0x8003
#define POR_HNG         0x8004
#define POR_INPUTFLD    0x8005

#define POR_DROP        0x8080
#define POR_TOX         0x8089
#define POR_ISOTOX      0x808a
#define POR_REF         0x808b
#define POR_ISOREF      0x808c
#define POR_META        0x808d

#define POR_EXP         0xc080
#define POR_BLANK       0xc081
#define POR_POSTITS     0xc082

#define POR_HYPH        0xd080
#define POR_HYPHSTR     0xd081
#define POR_SOFTHYPH    0xd082
#define POR_SOFTHYPHSTR 0xd083
#define POR_SOFTHYPH_COMP 0xd084

#define POR_FLD         0xe080
#define POR_HIDDEN      0xe081
#define POR_QUOVADIS    0xe082
#define POR_ERGOSUM     0xe083
#define POR_COMBINED    0xe084
#define POR_FTN         0xe085

#define POR_FTNNUM      0xe880
#define POR_NUMBER      0xe881
#define POR_BULLET      0xe882
#define POR_GRFNUM      0xe883

#define POR_GLUE        0x0480

#define POR_MARGIN      0x04c0

#define POR_FIX         0x06c0
#define POR_FLY         0x06c1

#define POR_TAB         0x0750

#define POR_TABRIGHT    0x07d0
#define POR_TABCENTER   0x07d1
#define POR_TABDECIMAL  0x07d2

#define POR_TABLEFT     0x0740

#endif  //_TXTTYPES_HXX
