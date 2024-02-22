/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include "cuigrfflt.hxx"

class WhatsNewImg : public weld::CustomWidgetController
{
    OUString m_sImage;
    OUString m_sText;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;

public:
    WhatsNewImg(){};
    void Update(const OUString& sImage, const OUString& sText)
    {
        m_sImage = sImage;
        m_sText = sText;
        SetAccessibleName(m_sText);
        Invalidate();
    };
};

class WhatsNewProgress : public weld::CustomWidgetController
{
    sal_Int32 m_nTotal;
    sal_Int32 m_nCurrent;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;

public:
    WhatsNewProgress(){};
    void Update(const sal_Int32 nCurrent, const sal_Int32 nTotal)
    {
        m_nTotal = nTotal;
        m_nCurrent = nCurrent;
        Invalidate();
    };
};

class WhatsNewDialog : public weld::GenericDialogController
{
private:
    const bool m_bWelcome;

    WhatsNewImg m_aPreview;
    WhatsNewProgress m_aProgress;

    std::unique_ptr<weld::Button> m_pPrevBtn;
    std::unique_ptr<weld::Button> m_pNextBtn;
    std::unique_ptr<weld::CustomWeld> m_pProgress;
    std::unique_ptr<weld::CustomWeld> m_pImage;

    DECL_LINK(OnPrevClick, weld::Button&, void);
    DECL_LINK(OnNextClick, weld::Button&, void);

    void LoadImage(); // loads WHATSNEW_STRINGARRAY[m_nCurrentNews]

    sal_Int32 m_nNumberOfNews;
    sal_Int32 m_nCurrentNews;

public:
    WhatsNewDialog(weld::Window* pParent, const bool bWelcome);
    virtual ~WhatsNewDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
