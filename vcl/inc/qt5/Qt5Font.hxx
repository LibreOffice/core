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

#include <vcl/dllapi.h>
#include <QtGui/QFont>

#include <fontinstance.hxx>
#include <hb-ot.h>

class VCL_DLLPUBLIC Qt5Font : public QFont, public LogicalFontInstance
{
    hb_font_t* m_pHbFont;

public:
    Qt5Font(const PhysicalFontFace& rPFF, const FontSelectPattern& rFSP)
        : LogicalFontInstance(rPFF, rFSP)
        , m_pHbFont(nullptr)
    {
    }
    virtual ~Qt5Font() override;

    hb_font_t* GetHbFont() const { return m_pHbFont; }
    void SetHbFont(hb_font_t* pHbFont) { m_pHbFont = pHbFont; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
