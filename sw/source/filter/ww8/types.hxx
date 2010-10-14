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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef WW_TYPES
#define WW_TYPES

#include <vector>

//if part of OOo
#include "typessw.hxx"
//else
//include standalonetypes.hxx
//endif

namespace ww
{
    typedef std::vector<sal_uInt8> bytes;

    enum WordVersion {eWW2 = 2, eWW6 = 6, eWW7 = 7, eWW8 = 8};
    inline bool IsSevenMinus(WordVersion eVer) { return eVer <= eWW7; }
    inline bool IsEightPlus(WordVersion eVer) { return eVer >= eWW8; }

    /** For custom wrapping

        When you edit the wrap points of a contour in word, word uses a relative
        scale of 0 to 21600 where 21600 is apparently 100% of the graphic width
    */
    const int nWrap100Percent = 21600;
}

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
