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

#ifndef INCLUDED_VCL_INC_TYPESERIALIZER_HXX
#define INCLUDED_VCL_INC_TYPESERIALIZER_HXX

#include <vcl/dllapi.h>
#include <tools/GenericTypeSerializer.hxx>
#include <vcl/gradient.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/graph.hxx>

constexpr sal_uInt32 createMagic(char char1, char char2, char char3, char char4)
{
    return (static_cast<sal_uInt32>(char1) << 24) | (static_cast<sal_uInt32>(char2) << 16)
           | (static_cast<sal_uInt32>(char3) << 8) | (static_cast<sal_uInt32>(char4) << 0);
}

constexpr sal_uInt32 constSvgMagic = createMagic('s', 'v', 'g', '0');
constexpr sal_uInt32 constWmfMagic = createMagic('w', 'm', 'f', '0');
constexpr sal_uInt32 constEmfMagic = createMagic('e', 'm', 'f', '0');
constexpr sal_uInt32 constPdfMagic = createMagic('p', 'd', 'f', '0');

class VCL_DLLPUBLIC TypeSerializer : public tools::GenericTypeSerializer
{
public:
    TypeSerializer(SvStream& rStream);

    void readGradient(Gradient& rGradient);
    void writeGradient(const Gradient& rGradient);

    void readGfxLink(GfxLink& rGfxLink);
    void writeGfxLink(const GfxLink& rGfxLink);

    void readGraphic(Graphic& rGraphic);
    void writeGraphic(const Graphic& rGraphic);

    void readMapMode(MapMode& rMapMode);
    void writeMapMode(MapMode const& rMapMode);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
