/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <vcl/layout.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <aboutdialog.hxx>

#include <tools/stream.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/geometry/RealRectangle2D.hpp>

#include <config_buildid.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmap.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace vcl;

AboutDialog::AboutDialog(vcl::Window* pParent, WinBits nStyle, Dialog::InitFlag eFlag)
    : Dialog(pParent, nStyle, eFlag)
    , m_xBuilder(new VclBuilder(this, getUIRootDir(), "vcl/ui/aboutbox.ui"))
{
    m_xBuilder->get(m_xContents, "about");
    m_xBuilder->get(m_xLogoReplacement, "logoreplacement");
    m_xBuilder->get(m_xLogoImage, "logo");
    m_xBuilder->get(m_xVersion, "version");
    m_xBuilder->get(m_xDescriptionText, "description");
    m_xBuilder->get(m_xCopyrightText, "copyright");
    m_xBuilder->get(m_xBuildIdLink, "buildIdLink");

    m_xVersion->RequestDoubleBuffering(true);
}

void AboutDialog::set_content_area(VclBox* pBox)
{
    Dialog::set_content_area(pBox);

    // move it now that the content area exists
    m_xContents->SetParent(pBox);

    StyleControls();
}

AboutDialog::~AboutDialog() { disposeOnce(); }

void AboutDialog::dispose()
{
    m_xVersion.clear();
    m_xDescriptionText.clear();
    m_xCopyrightText.clear();
    m_xLogoImage.clear();
    m_xLogoReplacement.clear();
    m_xBuildIdLink.clear();
    m_xContents.clear();
    m_xBuilder.reset();
    Dialog::dispose();
}

void AboutDialog::StyleControls()
{
    // Make all the controls have a transparent background
    m_xLogoImage->SetBackground();
    m_xLogoReplacement->SetPaintTransparent(true);
    m_xVersion->SetPaintTransparent(true);
    m_xDescriptionText->SetPaintTransparent(true);
    m_xCopyrightText->SetPaintTransparent(true);

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    const vcl::Font& aLabelFont = rStyleSettings.GetLabelFont();
    vcl::Font aLargeFont = aLabelFont;
    aLargeFont.SetFontSize(Size(0, aLabelFont.GetFontSize().Height() * 3));

    // Logo Replacement Text
    m_xLogoReplacement->SetControlFont(aLargeFont);

    // Description Text
    aLargeFont.SetFontSize(Size(0, aLabelFont.GetFontSize().Height() * 1.3));
    m_xDescriptionText->SetControlFont(aLargeFont);
}

void AboutDialog::SetLogo(const Image& rLogoBitmap)
{
    if (!rLogoBitmap)
    {
        m_xLogoImage->Hide();
        m_xLogoReplacement->Show();
    }
    else
    {
        m_xLogoReplacement->Hide();
        m_xLogoImage->SetImage(rLogoBitmap);
        m_xLogoImage->Show();
    }
}

void AboutDialog::SetBackground(const Image& rBackgroundBitmap)
{
    m_aBackgroundBitmap = rBackgroundBitmap.GetBitmapEx();
    Invalidate();
}

void AboutDialog::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& /*rRect*/)
{
    Size aSize(GetOutputSizePixel());
    Point aPos(aSize.Width() - m_aBackgroundBitmap.GetSizePixel().Width(),
               aSize.Height() - m_aBackgroundBitmap.GetSizePixel().Height());

    rRenderContext.DrawBitmapEx(aPos, m_aBackgroundBitmap);
}

bool AboutDialog::set_property(const OString& rKey, const OUString& rValue)
{
    if (rKey == "version")
        m_xVersion->SetText(rValue);
    else if (rKey == "copyright")
        m_xCopyrightText->SetText(rValue);
    else if (rKey == "comments")
        m_xDescriptionText->SetText(rValue);
    else if (rKey == "website")
        m_xBuildIdLink->SetURL(rValue);
    else if (rKey == "website_label")
        m_xBuildIdLink->SetText(rValue);
    else if (rKey == "program_name")
        m_xLogoReplacement->SetText(rValue);
    else
        return Dialog::set_property(rKey, rValue);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
