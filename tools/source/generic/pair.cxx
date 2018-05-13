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

#include <tools/gen.hxx>
#include <tools/Pair.hxx>
#include <tools/stream.hxx>

SvStream& ReadPair(SvStream& rIStream, Point& v) { return ReadPair(rIStream, v.toPair()); }
SvStream& WritePair(SvStream& rOStream, const Point& v) { return WritePair(rOStream, v.toPair()); }
SvStream& ReadPair(SvStream& rIStream, Size& v) { return ReadPair(rIStream, v.toPair()); }
SvStream& WritePair(SvStream& rOStream, const Size& v) { return WritePair(rOStream, v.toPair()); }

SvStream& ReadPair(SvStream& rIStream, Pair& rPair)
{
    sal_Int32 nTmpA(0), nTmpB(0);
    rIStream.ReadInt32(nTmpA).ReadInt32(nTmpB);
    rPair.nA = nTmpA;
    rPair.nB = nTmpB;

    return rIStream;
}

SvStream& WritePair(SvStream& rOStream, const Pair& rPair)
{
    rOStream.WriteInt32(rPair.nA).WriteInt32(rPair.nB);

    return rOStream;
}

rtl::OString Pair::toString() const
{
    std::stringstream ss;
    // Note that this is not just used for debugging output but the
    // format is parsed by external code (passed in callbacks to
    // LibreOfficeKit clients). So don't change.
    ss << A() << ", " << B();
    return ss.str().c_str();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
