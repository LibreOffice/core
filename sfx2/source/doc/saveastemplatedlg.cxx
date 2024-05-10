/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/docfilt.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sot/storage.hxx>

#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <sfx2/strings.hrc>

#include <saveastemplatedlg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

// Class SfxSaveAsTemplateDialog --------------------------------------------------

SfxSaveAsTemplateDialog::SfxSaveAsTemplateDialog(weld::Window* pParent, uno::Reference<frame::XModel> xModel)
    : GenericDialogController(pParent, u"sfx/ui/saveastemplatedlg.ui"_ustr, u"SaveAsTemplateDialog"_ustr)
    , m_xLBCategory(m_xBuilder->weld_tree_view(u"categorylb"_ustr))
    , m_xCBXDefault(m_xBuilder->weld_check_button(u"defaultcb"_ustr))
    , m_xTemplateNameEdit(m_xBuilder->weld_entry(u"name_entry"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , mnRegionPos(0)
    , m_xModel(std::move(xModel))
{
    m_xLBCategory->append_text(SfxResId(STR_CATEGORY_NONE));
    initialize();
    SetCategoryLBEntries(msCategories);

    m_xTemplateNameEdit->connect_changed(LINK(this, SfxSaveAsTemplateDialog, TemplateNameEditHdl));
    m_xLBCategory->connect_changed(LINK(this, SfxSaveAsTemplateDialog, SelectCategoryHdl));
    m_xLBCategory->set_size_request(m_xLBCategory->get_approximate_digit_width() * 32,
                                    m_xLBCategory->get_height_rows(8));
    m_xOKButton->connect_clicked(LINK(this, SfxSaveAsTemplateDialog, OkClickHdl));

    m_xOKButton->set_sensitive(false);
    m_xOKButton->set_label(SfxResId(STR_SAVEDOC));
}

IMPL_LINK_NOARG(SfxSaveAsTemplateDialog, OkClickHdl, weld::Button&, void)
{
    std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Question,
                VclButtonsType::YesNo, OUString()));
    if(!IsTemplateNameUnique())
    {
        OUString sQueryMsg(SfxResId(STR_QMSG_TEMPLATE_OVERWRITE));
        sQueryMsg = sQueryMsg.replaceFirst("$1",msTemplateName);
        xQueryDlg->set_primary_text(sQueryMsg.replaceFirst("$2", msSelectedCategory));

        if (xQueryDlg->run() == RET_NO)
            return;
    }

    if (SaveTemplate())
        m_xDialog->response(RET_OK);
    else
    {
        OUString sText( SfxResId(STR_ERROR_SAVEAS) );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                    VclButtonsType::Ok, sText.replaceFirst("$1", msTemplateName)));
        xBox->run();
    }
}

IMPL_LINK_NOARG(SfxSaveAsTemplateDialog, TemplateNameEditHdl, weld::Entry&, void)
{
    msTemplateName = comphelper::string::strip(m_xTemplateNameEdit->get_text(), ' ');
    SelectCategoryHdl(*m_xLBCategory);
}

IMPL_LINK_NOARG(SfxSaveAsTemplateDialog, SelectCategoryHdl, weld::TreeView&, void)
{
    if (m_xLBCategory->get_selected_index() == 0)
    {
        msSelectedCategory = OUString();
        m_xOKButton->set_sensitive(false);
    }
    else
    {
        msSelectedCategory = m_xLBCategory->get_selected_text();
        m_xOKButton->set_sensitive(!msTemplateName.isEmpty());
    }
}

void SfxSaveAsTemplateDialog::initialize()
{
    sal_uInt16 nCount = maDocTemplates.GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString sCategoryName(maDocTemplates.GetFullRegionName(i));
        msCategories.push_back(sCategoryName);
    }
}

void SfxSaveAsTemplateDialog::SetCategoryLBEntries(const std::vector<OUString>& rFolderNames)
{
    for (size_t i = 0, n = rFolderNames.size(); i < n; ++i)
        m_xLBCategory->insert_text(i+1, rFolderNames[i]);
    m_xLBCategory->select(0);
}

bool SfxSaveAsTemplateDialog::IsTemplateNameUnique()
{
    std::vector<OUString>::iterator it=find(msCategories.begin(), msCategories.end(), msSelectedCategory);
    mnRegionPos = std::distance(msCategories.begin(), it);

    sal_uInt16 nEntries = maDocTemplates.GetCount(mnRegionPos);
    for(sal_uInt16 i = 0; i < nEntries; i++)
    {
        OUString aName = maDocTemplates.GetName(mnRegionPos, i);
        if(aName == msTemplateName)
            return false;
    }

    return true;
}

bool SfxSaveAsTemplateDialog::SaveTemplate()
{
    uno::Reference< frame::XStorable > xStorable(m_xModel, uno::UNO_QUERY_THROW );

    uno::Reference< frame::XDocumentTemplates > xTemplates(frame::DocumentTemplates::create(comphelper::getProcessComponentContext()) );

    if (!xTemplates->storeTemplate( msSelectedCategory, msTemplateName, xStorable ))
        return false;

    sal_uInt16 nDocId = maDocTemplates.GetCount(mnRegionPos);
    OUString     sURL = maDocTemplates.GetTemplateTargetURLFromComponent(msSelectedCategory, msTemplateName);
    bool bIsSaved = maDocTemplates.InsertTemplate( mnRegionPos, nDocId, msTemplateName, sURL);

    if (!bIsSaved)
        return false;

    if (!sURL.isEmpty() && m_xCBXDefault->get_active())
    {
        OUString aServiceName;
        try
        {
            uno::Reference< embed::XStorage > xStorage =
                    comphelper::OStorageHelper::GetStorageFromURL( sURL, embed::ElementModes::READ );

            SotClipboardFormatId nFormat = SotStorage::GetFormatID( xStorage );

            std::shared_ptr<const SfxFilter> pFilter = SfxGetpApp()->GetFilterMatcher().GetFilter4ClipBoardId( nFormat );

            if ( pFilter )
                aServiceName = pFilter->GetServiceName();
        }
        catch( uno::Exception& )
        {}

        if(!aServiceName.isEmpty())
            SfxObjectFactory::SetStandardTemplate(aServiceName, sURL);
    }

    maDocTemplates.Update();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
