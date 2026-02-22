/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinknewdoctabpage.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <dialmgr.hxx>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

namespace
{
// Data struct for document types in listbox
struct DocumentTypeData
{
    OUString aStrURL;
    OUString aStrExt;
    DocumentTypeData(OUString aURL, OUString aExt)
        : aStrURL(std::move(aURL))
        , aStrExt(std::move(aExt))
    {
    }
};

struct ExecuteInfo
{
    bool bRbtEditLater;
    bool bRbtEditNow;
    INetURLObject aURL;
    OUString aStrDocName;
    css::uno::Reference<css::frame::XFrame> xFrame;
    SfxDispatcher* pDispatcher;
};
}

HyperlinkDocNewTabPage::HyperlinkDocNewTabPage(weld::Container* pParent,
                                               weld::DialogController* pController,
                                               const SfxItemSet* pSet)
    : HyperlinkTabPageBase(pParent, pController, u"cui/ui/hyperlinknewdocpage.ui"_ustr,
                           u"HyperlinkNewDocPage"_ustr, pSet)
    , m_xRbtEditNow(m_xBuilder->weld_radio_button(u"editnow"_ustr))
    , m_xRbtEditLater(m_xBuilder->weld_radio_button(u"editlater"_ustr))
    , m_xCbbPath(std::make_unique<SvxHyperURLBox>(m_xBuilder->weld_combo_box(u"path"_ustr)))
    , m_xBtCreate(m_xBuilder->weld_button(u"create"_ustr))
    , m_xLbDocTypes(m_xBuilder->weld_tree_view(u"types"_ustr))
{
    m_xCbbPath->SetSmartProtocol(INetProtocol::File);
    m_xLbDocTypes->set_size_request(-1, m_xLbDocTypes->get_height_rows(5));

    InitStdControls();

    m_xCbbPath->show();
    m_xCbbPath->SetBaseURL(SvtPathOptions().GetWorkPath());
    m_xRbtEditNow->set_active(true);

    m_xBtCreate->connect_clicked(LINK(this, HyperlinkDocNewTabPage, ClickNewHdl_Impl));

    FillDocumentList();
}

HyperlinkDocNewTabPage::~HyperlinkDocNewTabPage()
{
    if (m_xLbDocTypes)
    {
        for (sal_Int32 n = 0, nEntryCount = m_xLbDocTypes->n_children(); n < nEntryCount; ++n)
            delete weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(n));
        m_xLbDocTypes.reset();
    }
}

std::unique_ptr<SfxTabPage> HyperlinkDocNewTabPage::Create(weld::Container* pParent,
                                                           weld::DialogController* pController,
                                                           const SfxItemSet* pSet)
{
    return std::make_unique<HyperlinkDocNewTabPage>(pParent, pController, pSet);
}

bool HyperlinkDocNewTabPage::ImplGetURLObject(const OUString& rPath, std::u16string_view rBase,
                                              INetURLObject& aURLObject) const
{
    bool bIsValidURL = !rPath.isEmpty();
    if (bIsValidURL)
    {
        aURLObject.SetURL(rPath);
        if (aURLObject.GetProtocol() == INetProtocol::NotValid)
        {
            bool wasAbs;
            INetURLObject base(rBase);
            base.setFinalSlash();
            aURLObject = base.smartRel2Abs(rPath, wasAbs, true, INetURLObject::EncodeMechanism::All,
                                           RTL_TEXTENCODING_UTF8, true);
        }
        bIsValidURL = aURLObject.GetProtocol() != INetProtocol::NotValid;
        if (bIsValidURL)
        {
            OUString aBase(aURLObject.getName(INetURLObject::LAST_SEGMENT, false));
            if (aBase.isEmpty() || (aBase[0] == '.'))
                bIsValidURL = false;
        }
        if (bIsValidURL)
        {
            sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
            if (nPos != -1)
                aURLObject.SetExtension(
                    weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrExt);
        }
    }
    return bIsValidURL;
}

void HyperlinkDocNewTabPage::FillDocumentList()
{
    weld::WaitObject aWaitObj(GetFrameWeld());

    std::vector<SvtDynMenuEntry> aDynamicMenuEntries(
        SvtDynamicMenuOptions::GetMenu(EDynamicMenuType::NewMenu));

    for (const SvtDynMenuEntry& rDynamicMenuEntry : aDynamicMenuEntries)
    {
        OUString aDocumentUrl = rDynamicMenuEntry.sURL;
        OUString aTitle = rDynamicMenuEntry.sTitle;

        if (aDocumentUrl == "private:factory/swriter?slot=21051"
            || aDocumentUrl == "private:factory/swriter?slot=21052"
            || aDocumentUrl == "private:factory/sdatabase?Interactive")
            continue;

        if (!aDocumentUrl.isEmpty())
        {
            if (aDocumentUrl == "private:factory/simpress?slot=6686")
                aDocumentUrl = "private:factory/simpress";

            std::shared_ptr<const SfxFilter> pFilter
                = SfxFilter::GetDefaultFilterFromFactory(aDocumentUrl);
            if (pFilter)
            {
                OUString aTitleName = aTitle.replaceFirst("~", "");

                OUString aStrDefExt(pFilter->GetDefaultExtension());
                DocumentTypeData* pTypeData
                    = new DocumentTypeData(aDocumentUrl, aStrDefExt.copy(2));
                OUString sId(weld::toId(pTypeData));
                m_xLbDocTypes->append(sId, aTitleName);
            }
        }
    }
    m_xLbDocTypes->select(0);
}

void HyperlinkDocNewTabPage::FillDlgFields(const OUString& /*rStrURL*/) {}

void HyperlinkDocNewTabPage::GetCurrentItemData(OUString& rStrURL, OUString& aStrName,
                                                OUString& aStrIntName, OUString& aStrFrame,
                                                SvxLinkInsertMode& eMode)
{
    rStrURL = m_xCbbPath->get_active_text();
    INetURLObject aURL;
    if (ImplGetURLObject(rStrURL, m_xCbbPath->GetBaseURL(), aURL))
    {
        rStrURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    }

    GetDataFromCommonFields(aStrName, aStrIntName, aStrFrame, eMode);
}

void HyperlinkDocNewTabPage::SetInitFocus() { m_xCbbPath->grab_focus(); }

void HyperlinkDocNewTabPage::ClearPageSpecificControls() { m_xCbbPath->set_entry_text(OUString()); }

IMPL_LINK_NOARG(HyperlinkDocNewTabPage, ClickNewHdl_Impl, weld::Button&, void)
{
    const uno::Reference<uno::XComponentContext>& xContext
        = ::comphelper::getProcessComponentContext();
    uno::Reference<ui::dialogs::XFolderPicker2> xFolderPicker
        = sfx2::createFolderPicker(xContext, GetFrameWeld());

    OUString aStrURL;
    OUString aTempStrURL(m_xCbbPath->get_active_text());
    osl::FileBase::getFileURLFromSystemPath(aTempStrURL, aStrURL);

    OUString aStrPath = aStrURL;
    bool bZeroPath = aStrPath.isEmpty();
    bool bHandleFileName = bZeroPath;

    if (bZeroPath)
        aStrPath = SvtPathOptions().GetWorkPath();
    else if (!::utl::UCBContentHelper::IsFolder(aStrURL))
        bHandleFileName = true;

    xFolderPicker->setDisplayDirectory(aStrPath);
    sal_Int16 nResult = xFolderPicker->execute();
    if (ExecutableDialogResults::OK != nResult)
        return;

    char const sSlash[] = "/";

    INetURLObject aURL(aStrPath, INetProtocol::File);
    OUString aStrName;
    if (bHandleFileName)
        aStrName = bZeroPath ? aTempStrURL : aURL.getName();

    m_xCbbPath->SetBaseURL(xFolderPicker->getDirectory());
    OUString aStrTmp(xFolderPicker->getDirectory());

    if (aStrTmp[aStrTmp.getLength() - 1] != sSlash[0])
        aStrTmp += sSlash;

    if (bHandleFileName)
        aStrTmp += aStrName;

    INetURLObject aNewURL(aStrTmp);

    if (!aStrName.isEmpty() && !aNewURL.getExtension().isEmpty()
        && m_xLbDocTypes->get_selected_index() != -1)
    {
        const sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
        aNewURL.setExtension(weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrExt);
    }

    if (aNewURL.GetProtocol() == INetProtocol::File)
    {
        osl::FileBase::getSystemPathFromFileURL(
            aNewURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), aStrTmp);
    }
    else
    {
        aStrTmp = aNewURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
    }

    m_xCbbPath->set_entry_text(aStrTmp);
}

IMPL_STATIC_LINK(HyperlinkDocNewTabPage, DispatchDocument, void*, p, void)
{
    std::unique_ptr<ExecuteInfo> xExecuteInfo(static_cast<ExecuteInfo*>(p));
    if (!xExecuteInfo->xFrame.is())
        return;
    try
    {
        css::uno::Reference<css::awt::XTopWindow>(xExecuteInfo->xFrame->getContainerWindow(),
                                                  css::uno::UNO_QUERY_THROW);

        SfxViewFrame* pViewFrame = nullptr;

        // create items
        SfxStringItem aName(SID_FILE_NAME, xExecuteInfo->aStrDocName);
        SfxStringItem aReferer(SID_REFERER, u"private:user"_ustr);
        SfxStringItem aFrame(SID_TARGETNAME, u"_blank"_ustr);

        OUString aStrFlags('S');
        if (xExecuteInfo->bRbtEditLater)
        {
            aStrFlags += "H";
        }
        SfxStringItem aFlags(SID_OPTIONS, aStrFlags);

        const SfxPoolItemHolder aResult(xExecuteInfo->pDispatcher->ExecuteList(
            SID_OPENDOC, SfxCallMode::SYNCHRON, { &aName, &aFlags, &aFrame, &aReferer }));

        const SfxViewFrameItem* pItem = dynamic_cast<const SfxViewFrameItem*>(aResult.getItem());
        if (pItem)
        {
            pViewFrame = pItem->GetFrame();
            if (pViewFrame)
            {
                SfxStringItem aNewName(SID_FILE_NAME, xExecuteInfo->aURL.GetMainURL(
                                                          INetURLObject::DecodeMechanism::NONE));
                SfxUnoFrameItem aDocFrame(SID_FILLFRAME,
                                          pViewFrame->GetFrame().GetFrameInterface());
                pViewFrame->GetDispatcher()->ExecuteList(SID_SAVEASDOC, SfxCallMode::SYNCHRON,
                                                         { &aNewName }, { &aDocFrame });
            }
        }

        if (xExecuteInfo->bRbtEditNow)
        {
            css::uno::Reference<css::awt::XTopWindow> xWindow(
                xExecuteInfo->xFrame->getContainerWindow(), css::uno::UNO_QUERY);
            if (xWindow.is())
                xWindow->toFront();
        }

        if (pViewFrame && xExecuteInfo->bRbtEditLater)
        {
            SfxObjectShell* pObjShell = pViewFrame->GetObjectShell();
            pObjShell->DoClose();
        }
    }
    catch (...)
    {
    }
}

void HyperlinkDocNewTabPage::DoApply()
{
    weld::WaitObject aWait(GetFrameWeld());

    OUString aStrNewName = m_xCbbPath->get_active_text();

    INetURLObject aURL;
    if (!ImplGetURLObject(aStrNewName, m_xCbbPath->GetBaseURL(), aURL))
        return;

    aStrNewName = aURL.GetURLPath(INetURLObject::DecodeMechanism::NONE);
    bool bCreate = true;
    try
    {
        std::unique_ptr<SvStream> pIStm = ::utl::UcbStreamHelper::CreateStream(
            aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ);

        bool bOk = pIStm && (pIStm->GetError() == ERRCODE_NONE);

        pIStm.reset();

        if (bOk)
        {
            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(
                GetFrameWeld(), VclMessageType::Warning, VclButtonsType::YesNo,
                CuiResId(RID_CUISTR_HYPERDLG_QUERYOVERWRITE)));
            bCreate = xWarn->run() == RET_YES;
        }
    }
    catch (const uno::Exception&)
    {
    }

    if (!bCreate || aStrNewName.isEmpty())
        return;

    auto pExecuteInfo = std::make_unique<ExecuteInfo>();

    pExecuteInfo->bRbtEditLater = m_xRbtEditLater->get_active();
    pExecuteInfo->bRbtEditNow = m_xRbtEditNow->get_active();
    sal_Int32 nPos = m_xLbDocTypes->get_selected_index();
    if (nPos == -1)
        nPos = 0;
    pExecuteInfo->aURL = std::move(aURL);
    pExecuteInfo->aStrDocName
        = weld::fromId<DocumentTypeData*>(m_xLbDocTypes->get_id(nPos))->aStrURL;

    pExecuteInfo->xFrame = GetFrame();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    pExecuteInfo->pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : nullptr;

    Application::PostUserEvent(LINK(nullptr, HyperlinkDocNewTabPage, DispatchDocument),
                               pExecuteInfo.release());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
