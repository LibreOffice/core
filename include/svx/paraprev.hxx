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
#ifndef INCLUDED_SVX_PARAPREV_HXX
#define INCLUDED_SVX_PARAPREV_HXX

#include <editeng/svxenum.hxx>
#include <svx/svxdllapi.h>
#include <vcl/weld/customweld.hxx>

enum class SvxPrevLineSpace
{
    N1 = 0,
    N115,
    N15,
    N2,
    Prop,
    Min,
    Leading
};

class SVX_DLLPUBLIC SvxParaPrevWindow final : public weld::CustomWidgetController
{
    Size                m_aSize;

    // indentation
    tools::Long                m_nLeftMargin;
    tools::Long                m_nRightMargin;
    short               m_nFirstLineOffset;
    // distances
    sal_uInt16          m_nUpper;
    sal_uInt16          m_nLower;
    // adjustment
    SvxAdjust           m_eAdjust;
    // last line in justification
    SvxAdjust           m_eLastLine;
    // line distance
    SvxPrevLineSpace    m_eLine;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void DrawParagraph(vcl::RenderContext& rRenderContext);

public:
    SvxParaPrevWindow();

    void SetFirstLineOffset( short nNew )
    {
        m_nFirstLineOffset = nNew;
    }
    void SetLeftMargin( tools::Long nNew )
    {
        m_nLeftMargin = nNew;
    }
    void SetRightMargin( tools::Long nNew )
    {
        m_nRightMargin = nNew;
    }
    void SetUpper( sal_uInt16 nNew )
    {
        m_nUpper = nNew;
    }
    void SetLower( sal_uInt16 nNew )
    {
        m_nLower = nNew;
    }
    void SetAdjust( SvxAdjust eNew )
    {
        m_eAdjust = eNew;
    }
    void SetLastLine( SvxAdjust eNew )
    {
        m_eLastLine = eNew;
    }
    void SetLineSpace( SvxPrevLineSpace eNew )
    {
        m_eLine = eNew;
    }
    void SetSize( Size aNew )
    {
        m_aSize = aNew;
    }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
