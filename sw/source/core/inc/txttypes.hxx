/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX

#include "swtypes.hxx"

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

#endif // INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
