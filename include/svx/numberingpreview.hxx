/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <svx/svxdllapi.h>
#include <editeng/numitem.hxx>
#include <vcl/customweld.hxx>

/// Provides the preview to show how looks bullet or numbering format before the apply.
class SVXCORE_DLLPUBLIC SvxNumberingPreview final : public weld::CustomWidgetController
{
    const SvxNumRule* m_pActNum;
    vcl::Font m_aStdFont;
    bool m_bPosition;
    sal_uInt16 m_nActLevel;

    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const ::tools::Rectangle& rRect) override;

public:
    SvxNumberingPreview();

    void SetNumRule(const SvxNumRule* pNum)
    {
        m_pActNum = pNum;
        Invalidate();
    };
    void SetPositionMode() { m_bPosition = true; }
    void SetLevel(sal_uInt16 nSet) { m_nActLevel = nSet; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
