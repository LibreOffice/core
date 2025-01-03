/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <vcl/seleng.hxx>
#include <vcl/cursor.hxx>
#include <vcl/idle.hxx>
#include <vcl/textdata.hxx>

#include <cstddef>
#include <limits>
#include <vector>

class TextLine
{
private:
    sal_Int32           mnStart;
    sal_Int32           mnEnd;
    std::size_t         mnStartPortion;
    std::size_t         mnEndPortion;

    short               mnStartX;

    bool                mbInvalid;  // for clever formatting/output

public:
                    TextLine()
                        : mnStart {0}
                        , mnEnd {0}
                        , mnStartPortion {0}
                        , mnEndPortion {0}
                        , mnStartX {0}
                        , mbInvalid {true}
                    {}

    bool            IsIn( sal_Int32 nIndex, bool bInclEnd ) const
                        { return nIndex >= mnStart && ( bInclEnd ? nIndex <= mnEnd : nIndex < mnEnd ); }

    void            SetStart( sal_Int32 n )         { mnStart = n; }
    sal_Int32       GetStart() const                { return mnStart; }

    void            SetEnd( sal_Int32 n )           { mnEnd = n; }
    sal_Int32       GetEnd() const                  { return mnEnd; }

    void            SetStartPortion( std::size_t n ) { mnStartPortion = n; }
    std::size_t     GetStartPortion() const         { return mnStartPortion; }

    void            SetEndPortion( std::size_t n )  { mnEndPortion = n; }
    std::size_t     GetEndPortion() const           { return mnEndPortion; }

    sal_Int32       GetLen() const                  { return mnEnd - mnStart; }

    bool            IsInvalid() const               { return mbInvalid; }
    bool            IsValid() const                 { return !mbInvalid; }
    void            SetInvalid()                    { mbInvalid = true; }
    void            SetValid()                      { mbInvalid = false; }

    short           GetStartX() const               { return mnStartX; }
    void            SetStartX( short n )            { mnStartX = n; }

    inline bool operator == ( const TextLine& rLine ) const;
};

inline bool TextLine::operator == ( const TextLine& rLine ) const
{
    return mnStart == rLine.mnStart &&
           mnEnd == rLine.mnEnd &&
           mnStartPortion == rLine.mnStartPortion &&
           mnEndPortion == rLine.mnEndPortion;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
