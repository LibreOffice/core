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
#ifndef _EEDATA_HXX
#define _EEDATA_HXX

//////////////////////////////////////////////////////////////////////////////
// extended data definitions for EditEngine portion stripping (StripPortions()
// support). These may be mapped to some primitive definitions from Drawinglayer
// later.
#include <vector>

namespace EEngineData
{
    // spell checking wrong vector containing the redlining data
    class WrongSpellClass
    {
    public:
        sal_uInt32 nStart;
        sal_uInt32 nEnd;

        WrongSpellClass(sal_uInt32 nS, sal_uInt32 nE) : nStart(nS), nEnd(nE) {}
    };

    typedef std::vector< WrongSpellClass > WrongSpellVector;

    // text marking vector containing the EOC, EOW and EOS TEXT definitions
    enum TextMarkingType
    {
        EndOfCaracter = 0,
        EndOfWord = 1,
        EndOfSentence = 2
    };

    class TextMarkingClass
    {
    public:
        TextMarkingType eType;
        sal_uInt32 nIndex;

        TextMarkingClass(TextMarkingType eT, sal_uInt32 nI) : eType(eT), nIndex(nI) {}
    };

    typedef std::vector< TextMarkingClass > TextMarkingVector;

} // end of namespace EditEngine

#endif // _EEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
