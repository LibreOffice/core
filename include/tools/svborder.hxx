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
#include <tools/long.hxx>

namespace tools { class Rectangle; }

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC SvBorder
{
    tools::Long nTop, nRight, nBottom, nLeft;

public:
    SvBorder()
    {
        nTop = nRight = nBottom = nLeft = 0;
    }
    SvBorder( tools::Long nLeftP, tools::Long nTopP, tools::Long nRightP, tools::Long nBottomP )
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
    tools::Long &  Top()    { return nTop; }
    tools::Long &  Right()  { return nRight; }
    tools::Long &  Bottom() { return nBottom; }
    tools::Long &  Left()   { return nLeft; }
    tools::Long    Top()    const { return nTop; }
    tools::Long    Right()  const { return nRight; }
    tools::Long    Bottom() const { return nBottom; }
    tools::Long    Left()   const { return nLeft; }
};

TOOLS_DLLPUBLIC tools::Rectangle & operator += ( tools::Rectangle & rRect, const SvBorder & rBorder );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
