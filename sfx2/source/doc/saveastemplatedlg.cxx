/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/saveastemplatedlg.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <sot/storage.hxx>

#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include "doc.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::frame;

// Class SfxSaveAsTemplateDialog --------------------------------------------------

SfxSaveAsTemplateDialog::SfxSaveAsTemplateDialog( vcl::Window* pParent):
        ModalDialog(pParent, "SaveAsTemplateDialog", "sfx/ui/saveastemplatedlg.ui"),
        msSelectedCategory(OUString()),
        msTemplateName(OUString()),
        mnRegionPos(0),
        mpDocTemplates(new SfxDocumentTemplates)
{
    get(mpLBCategory, "categorylb");
    get(mpCBXDefault, "defaultcb");
    get(mpTemplateNameEdit, "name_entry");
    get(mpOKButton, "ok");

    initialize();
    SetCategoryLBEntries(msCategories);

    mpTemplateNameEdit->SetModifyHdl(LINK(this, SfxSaveAsTemplateDialog, TemplateNameEditHdl));
    mpLBCategory->SetSelectHdl(LINK(this, SfxSaveAsTemplateDialog, SelectCategoryHdl));
    mpOKButton->SetClickHdl(LINK(this, SfxSaveAsTemplateDialog, OkClickHdl));

    mpOKButton->Disable();
    mpOKButton->SetText(SfxResId(STR_SAVEDOC).toString());
}

SfxSaveAsTemplateDialog::~SfxSaveAsTemplateDialog()
{
    disposeOnce();
}

void SfxSaveAsTemplateDialog::dispose()
{
    mpLBCategory.clear();
    mpTemplateNameEdit.clear();
    mpOKButton.clear();
    mpCBXDefault.clear();

    ModalDialog::dispose();
}

void SfxSaveAsTemplateDialog::setDocumentModel(const uno::Reference<frame::XModel> &rModel)
{
    m_xModel = rModel;
}

IMPL_LINK_NOARG_TYPED(SfxSaveAsTemplateDialog, OkClickHdl, Button*, void)
{
    ScopedVclPtrInstance< MessageDialog > aQueryDlg(this, OUString(), VclMessageType::Question, VCL_BUTTONS_YES_NO);

    if(!IsTemplateNameUnique())
    {
        OUString sQueryMsg(SfxResId(STR_QMSG_TEMPLATE_OVERWRITE).toString());
        sQueryMsg = sQueryMsg.replaceFirst("$1",msTemplateName);
        aQueryDlg->set_primary_text(sQueryMsg.replaceFirst("$2", msSelectedCategory));

        if( aQueryDlg->Execute() == RET_NO )
            return;
    }

    if(SaveTemplate())
        Close();
    else
    {
        OUString sText( SfxResId(STR_ERROR_SAVEAS).toString() );
        ScopedVclPtrInstance<MessageDialog>(this, sText.replaceFirst("$1", msTemplateName))->Execute();
    }
}

IMPL_LINK_NOARG_TYPED(SfxSaveAsTemplateDialog, TemplateNameEditHdl, Edit&, void)
{
    msTemplateName = comphelper::string::strip(mpTemplateNameEdit->GetText(), ' ');
    SelectCategoryHdl(*mpLBCategory);
}

IMPL_LINK_NOARG_TYPED(SfxSaveAsTemplateDialog, SelectCategoryHdl, ListBox&, void)
{
    if(mpLBCategory->GetSelectEntryPos() == 0)
    {
        msSelectedCategory = OUString();
        mpOKButton->Disable();
    }
    else
    {
        msSelectedCategory = mpLBCategory->GetSelectEntry();
        mpOKButton->Enable(!msTemplateName.isEmpty());
    }
}

void SfxSaveAsTemplateDialog::initialize()
{
    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString sCategoryName(mpDocTemplates->GetFullRegionName(i));
        msCategories.push_back(sCategoryName);
    }
}

void SfxSaveAsTemplateDialog::SetCategoryLBEntries(std::vector<OUString> aFolderNames)
{
    if (!aFolderNames.empty())
    {
        for (size_t i = 0, n = aFolderNames.size(); i < n; ++i)
            mpLBCategory->InsertEntry(aFolderNames[i], i+1);
    }
    mpLBCategory->SelectEntryPos(0);
}

bool SfxSaveAsTemplateDialog::IsTemplateNameUnique()
{
    std::vector<OUString>::iterator it;
    it=find(msCategories.begin(), msCategories.end(), msSelectedCategory);
    mnRegionPos = std::distance(msCategories.begin(), it);

    sal_uInt16 nEntries = mpDocTemplates->GetCount(mnRegionPos);
    for(sal_uInt16 i = 0; i < nEntries; i++)
    {
        OUString aName = mpDocTemplates->GetName(mnRegionPos, i);
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

    sal_uInt16 nDocId = mpDocTemplates->GetCount(mnRegionPos);
    OUString     sURL = mpDocTemplates->GetTemplateTargetURLFromComponent(msSelectedCategory, msTemplateName);
    bool bIsSaved = mpDocTemplates->InsertTemplate( mnRegionPos, nDocId, msTemplateName, sURL);

    if (!bIsSaved)
        return false;

    if ( !sURL.isEmpty() && mpCBXDefault->IsChecked() )
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

    mpDocTemplates->Update();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
