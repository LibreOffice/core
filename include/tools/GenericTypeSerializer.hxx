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
#ifndef INCLUDED_TOOLS_GENERICTYPESERIALIZER_HXX
#define INCLUDED_TOOLS_GENERICTYPESERIALIZER_HXX

#include <tools/toolsdllapi.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/fract.hxx>

namespace tools
{
class TOOLS_DLLPUBLIC GenericTypeSerializer
{
public:
    SvStream& mrStream;

    GenericTypeSerializer(SvStream& rStream)
        : mrStream(rStream)
    {
    }

    void readColor(Color& rColor);
    void writeColor(const Color& rColor);

    void readPoint(Point& rPoint);
    void writePoint(const Point& rPoint);

    void readSize(Size& rSize);
    void writeSize(const Size& rSize);

    void readRectangle(Rectangle& rRectangle);
    void writeRectangle(const Rectangle& rRectangle);

    void readFraction(Fraction& rFraction);
    void writeFraction(Fraction const& rFraction);
};

} // end namespace tools

#endif // INCLUDED_TOOLS_GENERICTYPESERIALIZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
