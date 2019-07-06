/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_ABOUT_DIALOG_HXX
#define INCLUDED_VCL_ABOUT_DIALOG_HXX

#include <vcl/fixed.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/vclmedit.hxx>

namespace vcl
{
class AboutDialog : public Dialog
{
private:
    BitmapEx m_aBackgroundBitmap;

    std::unique_ptr<VclBuilder> m_xBuilder;
    VclPtr<VclContainer> m_xContents;
    VclPtr<VclMultiLineEdit> m_xVersion;
    VclPtr<FixedHyperlink> m_xBuildIdLink;
    VclPtr<FixedText> m_xDescriptionText;
    VclPtr<FixedText> m_xCopyrightText;
    VclPtr<FixedImage> m_xLogoImage;
    VclPtr<FixedText> m_xLogoReplacement;

    void StyleControls();

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext,
                       const ::tools::Rectangle& rRect) override;
    virtual bool set_property(const OString& rKey, const OUString& rValue) override;
    virtual void set_content_area(VclBox* pBox) override;

public:
    explicit AboutDialog(vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG,
                         InitFlag eFlag = InitFlag::Default);
    virtual ~AboutDialog() override;
    virtual void dispose() override;

    void SetLogo(const Image& rLogoBitmap);
    void SetBackground(const Image& rBackgroundBitmap);
    void SetVersion(const OUString& rVersion) { m_xVersion->SetText(rVersion); }
    void SetCopyright(const OUString& rCopyright) { m_xCopyrightText->SetText(rCopyright); }
    void SetWebsiteLink(const OUString& rURL) { m_xBuildIdLink->SetURL(rURL); }
    void SetWebsiteLabel(const OUString& rLabel) { m_xBuildIdLink->SetText(rLabel); }

    OUString GetWebsiteLabel() const { return m_xBuildIdLink->GetText(); }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
