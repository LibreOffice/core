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

#ifndef INCLUDED_SD_SOURCE_FILTER_HTML_HTMLATTR_HXX
#define INCLUDED_SD_SOURCE_FILTER_HTML_HTMLATTR_HXX

#include <vcl/ctrl.hxx>
#include <tools/color.hxx>

class SdHtmlAttrPreview : public Control
{
protected:

    Color   m_aBackColor, m_aTextColor, m_aLinkColor;
    Color   m_aVLinkColor, m_aALinkColor;

public:
    SdHtmlAttrPreview( vcl::Window* pParent, WinBits nStyle );
    virtual ~SdHtmlAttrPreview() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

    void    SetColors( Color const & aBack, Color const & aText, Color const & aLink,
                       Color const & aVLink, Color const & aALink );
};

#endif // INCLUDED_SD_SOURCE_FILTER_HTML_HTMLATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
