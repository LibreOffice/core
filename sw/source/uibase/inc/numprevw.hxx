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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMPREVW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMPREVW_HXX

#include <vcl/customweld.hxx>

class SwNumRule;

class NumberingPreview final : public weld::CustomWidgetController
{
    const SwNumRule*    m_pActNum;
    vcl::Font           m_aStdFont;
    tools::Long                m_nPageWidth;
    const UIName*       m_pOutlineNames;
    bool                m_bPosition;
    sal_uInt16          m_nActLevel;

private:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

public:
    NumberingPreview()
        : m_pActNum(nullptr)
        , m_nPageWidth(0)
        , m_pOutlineNames(nullptr)
        , m_bPosition(false)
        , m_nActLevel(USHRT_MAX)
    {
    }

    void    SetNumRule(const SwNumRule* pNum)
    {
        m_pActNum = pNum;
        Invalidate();
    }

    void    SetPageWidth(tools::Long nPgWidth)
                            {m_nPageWidth = nPgWidth;}
    void    SetOutlineNames(const UIName* pNames)
                    {m_pOutlineNames = pNames;}
    void    SetPositionMode()
                    { m_bPosition = true;}
    void    SetLevel(sal_uInt16 nSet) {m_nActLevel = nSet;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
