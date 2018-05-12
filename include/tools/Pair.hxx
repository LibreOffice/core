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

#ifndef INCLUDED_TOOLS_PAIR_HXX
#define INCLUDED_TOOLS_PAIR_HXX

#include <tools/toolsdllapi.h>

class Point;
class Size;
class SvStream;

namespace rtl
{
class OString;
}

class SAL_WARN_UNUSED Pair
{
public:
    Pair()
        : nA(0)
        , nB(0)
    {
    }
    Pair(long _nA, long _nB)
        : nA(_nA)
        , nB(_nB)
    {
    }

    long A() const { return nA; }
    long B() const { return nB; }

    long& A() { return nA; }
    long& B() { return nB; }

    TOOLS_DLLPUBLIC rtl::OString toString() const;
    TOOLS_DLLPUBLIC friend SvStream& ReadPair(SvStream& rIStream, Pair& rPair);
    TOOLS_DLLPUBLIC friend SvStream& WritePair(SvStream& rOStream, const Pair& rPair);

protected:
    long nA;
    long nB;
};

namespace tools
{
namespace detail
{
// Used to implement operator == for subclasses of Pair:
inline bool equal(Pair const& p1, Pair const& p2) { return p1.A() == p2.A() && p1.B() == p2.B(); }

} // namespace detail
} // namespace tools

SAL_DLLPUBLIC_EXPORT SvStream& ReadPair(SvStream& rIStream, Point& v);
SAL_DLLPUBLIC_EXPORT SvStream& WritePair(SvStream& rOStream, const Point& v);
SAL_DLLPUBLIC_EXPORT SvStream& ReadPair(SvStream& rIStream, Size& v);
SAL_DLLPUBLIC_EXPORT SvStream& WritePair(SvStream& rOStream, const Size& v);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
