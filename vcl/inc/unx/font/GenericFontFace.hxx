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

#pragma once

#include <sal/config.h>

#include <font/PhysicalFontFace.hxx>
#include <fontattributes.hxx>
#include <glyphid.hxx>

// GenericFontFace corresponds to an unscaled font face
class GenericFontFace final : public vcl::font::PhysicalFontFace
{
public:
    GenericFontFace( const FontAttributes&, OString aNativeFileName,
                      int nFaceNum, int nFaceVariation, sal_IntPtr nFontId );

    const OString&        GetFontFileName() const   { return maNativeFileName; }
    int                   GetFontFaceIndex() const  { return mnFaceNum; }
    int                   GetFontFaceVariation() const  { return mnFaceVariation; }

    virtual rtl::Reference<LogicalFontInstance> CreateFontInstance(const vcl::font::FontSelectPattern&) const override;
    virtual sal_IntPtr      GetFontId() const override { return mnFontId; }

    virtual hb_face_t* GetHbFace() const override;

    const std::vector<vcl::font::Variation>& GetVariations(const LogicalFontInstance&) const override;

private:
    const OString           maNativeFileName;
    const int               mnFaceNum;
    const int               mnFaceVariation;
    const sal_IntPtr        mnFontId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
