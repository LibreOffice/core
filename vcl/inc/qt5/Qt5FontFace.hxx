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

#include <vclpluginapi.h>
#include <PhysicalFontFace.hxx>

#include <tools/ref.hxx>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <QtCore/QString>
#include <QtGui/QFont>

class FontAttributes;
class FontSelectPattern;

class Qt5FontFace final : public PhysicalFontFace
{
public:
    static Qt5FontFace* fromQFont(const QFont& rFont);
    static Qt5FontFace* fromQFontDatabase(const QString& aFamily, const QString& aStyle);
    static void fillAttributesFromQFont(const QFont& rFont, FontAttributes& rFA);

    VCLPLUG_QT5_PUBLIC static FontWeight toFontWeight(const int nWeight);
    VCLPLUG_QT5_PUBLIC static FontWidth toFontWidth(const int nStretch);
    VCLPLUG_QT5_PUBLIC static FontItalic toFontItalic(const QFont::Style eStyle);

    sal_IntPtr GetFontId() const override;

    QFont CreateFont() const;
    int GetFontTable(const char pTagName[5], unsigned char*) const;

    FontCharMapRef GetFontCharMap() const override;
    bool GetFontCapabilities(vcl::FontCapabilities&) const override;
    bool HasChar(sal_uInt32 cChar) const;

    rtl::Reference<LogicalFontInstance>
    CreateFontInstance(const FontSelectPattern& rFSD) const override;

private:
    Qt5FontFace(const Qt5FontFace&);
    Qt5FontFace(const FontAttributes& rFA, const QString& rFontID);

    const QString m_aFontId;
    mutable FontCharMapRef m_xCharMap;
    mutable vcl::FontCapabilities m_aFontCapabilities;
    mutable bool m_bFontCapabilitiesRead;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
