/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
