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

#include <PhysicalFontFace.hxx>

#include <tools/ref.hxx>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <QtCore/QString>

class FontAttributes;
class FontSelectPattern;
class QFont;

class Qt5FontFace : public PhysicalFontFace
{
public:
    virtual ~Qt5FontFace() override;

    static Qt5FontFace* fromQFont(const QFont& rFont);

    rtl::Reference<PhysicalFontFace> Clone() const override;
    sal_IntPtr GetFontId() const override;

    int GetFontTable(const char pTagName[5], unsigned char*) const;

    const FontCharMapRef GetFontCharMap() const;
    bool GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const;
    bool HasChar(sal_uInt32 cChar) const;

    LogicalFontInstance* CreateFontInstance(const FontSelectPattern& rFSD) const override;

protected:
    Qt5FontFace(const Qt5FontFace&);
    Qt5FontFace(const FontAttributes& rFA, const QString& rFontID);

private:
    const QString m_aFontId;
    mutable FontCharMapRef m_xCharMap;
    mutable vcl::FontCapabilities m_aFontCapabilities;
    mutable bool m_bFontCapabilitiesRead;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
