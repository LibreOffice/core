/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinkdoctabpage.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/filedlghelper.hxx>
#include <osl/file.hxx>
#include <comphelper/lok.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <hlmarkwn_def.hxx>

using namespace ::com::sun::star;

namespace
{
char const sHash[] = "#";
}

HyperlinkDocTabPage::HyperlinkDocTabPage(weld::Container* pParent,
                                         weld::DialogController* pController,
                                         const SfxItemSet* pSet)
    : HyperlinkTabPageBase(pParent, pController, u"cui/ui/hyperlinkdocpage.ui"_ustr,
                           u"HyperlinkDocPage"_ustr, pSet)
    , m_xCbbPath(std::make_unique<SvxHyperURLBox>(m_xBuilder->weld_combo_box(u"path"_ustr)))
    , m_xBtFileopen(m_xBuilder->weld_button(u"fileopen"_ustr))
    , m_xEdTarget(m_xBuilder->weld_entry(u"target"_ustr))
    , m_xFtFullURL(m_xBuilder->weld_label(u"url"_ustr))
    , m_xBtBrowse(m_xBuilder->weld_button(u"browse"_ustr))
{
    m_xCbbPath->SetSmartProtocol(INetProtocol::File);

    InitStdControls();

    // Clear any default text from UI file
    m_xIndication->set_text(OUString());

    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xBuilder->weld_label(u"label3"_ustr)->hide();
        m_xBtFileopen->hide();
        m_xBuilder->weld_label(u"path_label"_ustr)->hide();
        m_xCbbPath->hide();
        m_xBuilder->weld_label(u"url_label"_ustr)->hide();
        m_xFtFullURL->hide();
    }

    m_xCbbPath->SetBaseURL(INET_FILE_SCHEME);

    m_xBtFileopen->connect_clicked(LINK(this, HyperlinkDocTabPage, ClickFileopenHdl_Impl));
    m_xBtBrowse->connect_clicked(LINK(this, HyperlinkDocTabPage, ClickTargetHdl_Impl));
    m_xCbbPath->connect_changed(LINK(this, HyperlinkDocTabPage, ModifiedPathHdl_Impl));
    m_xEdTarget->connect_changed(LINK(this, HyperlinkDocTabPage, ModifiedTargetHdl_Impl));
    m_xCbbPath->connect_focus_out(LINK(this, HyperlinkDocTabPage, LostFocusPathHdl_Impl));
    maTimer.SetInvokeHandler(LINK(this, HyperlinkDocTabPage, TimeoutHdl_Impl));
}

HyperlinkDocTabPage::~HyperlinkDocTabPage()
{
    if (mxMarkWnd)
    {
        mxMarkWnd->response(RET_CANCEL);
        mxMarkWnd.reset();
    }
}

std::unique_ptr<SfxTabPage> HyperlinkDocTabPage::Create(weld::Container* pParent,
                                                        weld::DialogController* pController,
                                                        const SfxItemSet* pSet)
{
    return std::make_unique<HyperlinkDocTabPage>(pParent, pController, pSet);
}

void HyperlinkDocTabPage::FillDlgFields(const OUString& rStrURL)
{
    sal_Int32 nPos = rStrURL.indexOf(sHash);

    m_xCbbPath->set_entry_text(rStrURL.copy(0, (nPos == -1 ? rStrURL.getLength() : nPos)));

    OUString aStrMark;
    if (nPos != -1 && nPos < rStrURL.getLength() - 1)
        aStrMark = rStrURL.copy(nPos + 1);
    m_xEdTarget->set_text(aStrMark);

    ModifiedPathHdl_Impl(*m_xCbbPath->getWidget());
}

void HyperlinkDocTabPage::GetCurrentItemData(OUString& rStrURL, OUString& aStrName,
                                             OUString& aStrIntName, OUString& aStrFrame,
                                             SvxLinkInsertMode& eMode)
{
    rStrURL = GetCurrentURL();

    if (rStrURL.equalsIgnoreAsciiCase(INET_FILE_SCHEME))
        rStrURL.clear();

    GetDataFromCommonFields(aStrName, aStrIntName, aStrFrame, eMode);
}

HyperlinkDocTabPage::EPathType HyperlinkDocTabPage::GetPathType(std::u16string_view rStrPath)
{
    INetURLObject aURL(rStrPath, INetProtocol::File);

    if (aURL.HasError())
        return EPathType::Invalid;
    else
        return EPathType::ExistsFile;
}

OUString HyperlinkDocTabPage::GetCurrentURL() const
{
    OUString aStrURL;
    OUString aStrPath(m_xCbbPath->get_active_text());
    OUString aStrMark(m_xEdTarget->get_text());

    if (!aStrPath.isEmpty())
    {
        INetURLObject aURL(aStrPath);
        if (aURL.GetProtocol() != INetProtocol::NotValid)
            aStrURL = aStrPath;
        else
        {
            osl::FileBase::getFileURLFromSystemPath(aStrPath, aStrURL);
            aStrURL = INetURLObject::decode(aStrURL, INetURLObject::DecodeMechanism::ToIUri,
                                            RTL_TEXTENCODING_UTF8);
        }

        if (aStrURL.isEmpty())
            aStrURL = aStrPath;
    }

    if (!aStrMark.isEmpty())
    {
        aStrURL += sHash + aStrMark;
    }

    return aStrURL;
}

void HyperlinkDocTabPage::SetInitFocus() { m_xCbbPath->grab_focus(); }

bool HyperlinkDocTabPage::ShouldOpenMarkWnd() { return m_bMarkWndOpen; }

void HyperlinkDocTabPage::SetMarkWndShouldOpen(bool bOpen) { m_bMarkWndOpen = bOpen; }

void HyperlinkDocTabPage::ClearPageSpecificControls()
{
    m_xCbbPath->set_entry_text(OUString());
    m_xEdTarget->set_text(OUString());
    maStrURL.clear();
    m_xFtFullURL->set_label(maStrURL);
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, ClickFileopenHdl_Impl, weld::Button&, void)
{
    sfx2::FileDialogHelper aDlg(css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                                FileDialogFlags::NONE, GetFrameWeld());

    OUString aOldURL(GetCurrentURL());
    if (aOldURL.startsWithIgnoreAsciiCase(INET_FILE_SCHEME))
    {
        OUString aPath;
        osl::FileBase::getSystemPathFromFileURL(aOldURL, aPath);
        aDlg.SetDisplayFolder(aPath);
    }

    ErrCode nError = aDlg.Execute();

    if (ERRCODE_NONE != nError)
        return;

    OUString aURL(aDlg.GetPath());
    OUString aPath;

    osl::FileBase::getSystemPathFromFileURL(aURL, aPath);

    m_xCbbPath->SetBaseURL(aURL);
    m_xCbbPath->set_entry_text(aPath);

    if (aOldURL != GetCurrentURL())
        ModifiedPathHdl_Impl(*m_xCbbPath->getWidget());
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, ClickTargetHdl_Impl, weld::Button&, void)
{
    mxMarkWnd = std::make_shared<SvxHlinkDlgMarkWnd>(GetFrameWeld());

    if (GetPathType(maStrURL) == EPathType::ExistsFile || maStrURL.isEmpty()
        || maStrURL.equalsIgnoreAsciiCase(INET_FILE_SCHEME) || maStrURL.startsWith(sHash))
    {
        mxMarkWnd->SetError(LERR_NOERROR);

        weld::WaitObject aWait(GetFrameWeld());

        if (maStrURL.equalsIgnoreAsciiCase(INET_FILE_SCHEME))
            mxMarkWnd->RefreshTree(u""_ustr);
        else
            mxMarkWnd->RefreshTree(maStrURL);
    }
    else
        mxMarkWnd->SetError(LERR_DOCNOTOPEN);

    weld::DialogController::runAsync(mxMarkWnd, [this](sal_Int32 nResult) {
        if (nResult == RET_OK && mxMarkWnd)
        {
            OUString aSelectedMark = mxMarkWnd->GetSelectedMark();
            if (!aSelectedMark.isEmpty())
            {
                m_xEdTarget->set_text(aSelectedMark);
                ModifiedTargetHdl_Impl(*m_xEdTarget);
            }
        }
        mxMarkWnd.reset();
    });
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, ModifiedPathHdl_Impl, weld::ComboBox&, void)
{
    maStrURL = GetCurrentURL();

    maTimer.SetTimeout(2500);
    maTimer.Start();

    m_xFtFullURL->set_label(maStrURL);
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, ModifiedTargetHdl_Impl, weld::Entry&, void)
{
    maStrURL = GetCurrentURL();

    if (IsMarkWndVisible())
        mxMarkWnd->SelectEntry(m_xEdTarget->get_text());

    m_xFtFullURL->set_label(maStrURL);
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, LostFocusPathHdl_Impl, weld::Widget&, void)
{
    maStrURL = GetCurrentURL();
    m_xFtFullURL->set_label(maStrURL);
}

IMPL_LINK_NOARG(HyperlinkDocTabPage, TimeoutHdl_Impl, Timer*, void)
{
    if (IsMarkWndVisible()
        && (GetPathType(maStrURL) == EPathType::ExistsFile || maStrURL.isEmpty()
            || maStrURL.equalsIgnoreAsciiCase(INET_FILE_SCHEME)))
    {
        weld::WaitObject aWait(GetFrameWeld());

        if (mxMarkWnd)
        {
            if (maStrURL.equalsIgnoreAsciiCase(INET_FILE_SCHEME))
                mxMarkWnd->RefreshTree(u""_ustr);
            else
                mxMarkWnd->RefreshTree(maStrURL);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
