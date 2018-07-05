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

#ifndef INCLUDED_TOOLS_SVBORDER_HXX
#define INCLUDED_TOOLS_SVBORDER_HXX

#include <tools/toolsdllapi.h>
#include <tools/gen.hxx>

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC SvBorder
{
    long nTop, nRight, nBottom, nLeft;

public:
    SvBorder()
    {
        nTop = nRight = nBottom = nLeft = 0;
    }
    SvBorder( long nLeftP, long nTopP, long nRightP, long nBottomP )
    {
        nLeft = nLeftP;
        nTop = nTopP;
        nRight = nRightP;
        nBottom = nBottomP;
    }
    bool    operator == ( const SvBorder & rObj ) const
            {
                return nTop == rObj.nTop && nRight == rObj.nRight &&
                       nBottom == rObj.nBottom && nLeft == rObj.nLeft;
            }
    bool    operator != ( const SvBorder & rObj ) const
            { return !(*this == rObj); }
    SvBorder & operator += ( const SvBorder & rBorder )
            {
                Left()   += rBorder.Left();
                Top()    += rBorder.Top();
                Right()  += rBorder.Right();
                Bottom() += rBorder.Bottom();
                return *this;
            }
    long &  Top()    { return nTop; }
    long &  Right()  { return nRight; }
    long &  Bottom() { return nBottom; }
    long &  Left()   { return nLeft; }
    long    Top()    const { return nTop; }
    long    Right()  const { return nRight; }
    long    Bottom() const { return nBottom; }
    long    Left()   const { return nLeft; }
};

TOOLS_DLLPUBLIC tools::Rectangle & operator += ( tools::Rectangle & rRect, const SvBorder & rBorder );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
