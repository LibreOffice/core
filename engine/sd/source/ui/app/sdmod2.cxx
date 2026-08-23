/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <editeng/flditem.hxx>
#include <editeng/CustomPropertyField.hxx>
#include <tools/debug.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/styfitem.hxx>
#include <svl/inethist.hxx>
#include <svl/poolitem.hxx>
#include <svl/flagitem.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <osl/diagnose.h>

#include <editeng/measfld.hxx>
#include <editeng/editstat.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/svdotext.hxx>

#include <sfx2/sfxdlg.hxx>

#include <sdmod.hxx>
#include <app.hrc>
#include <family.hrc>
#include <strings.hrc>
#include <sdattr.hrc>

#include <bitmaps.hlst>
#include <ViewShell.hxx>
#include <FrameView.hxx>
#include <optsitem.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <Outliner.hxx>
#include <sdresid.hxx>
#include <pres.hxx>
#include <OutlineViewShell.hxx>
#include <OutlineView.hxx>
#include <ViewShellBase.hxx>
#include <sdpage.hxx>
#include <sdabstdlg.hxx>
#include <svl/intitem.hxx>

#include <officecfg/Office/Draw.hxx>
#include <officecfg/Office/Impress.hxx>

/** retrieves the page that is currently painted. This will only be the master page
    if the current drawn view only shows the master page*/
static SdPage* GetCurrentPage( sd::ViewShell const * pViewSh, EditFieldInfo const * pInfo, bool& bMasterView )
{
    if( !pInfo )
        return nullptr;

    bMasterView = false;
    SdPage* pPage = dynamic_cast< SdPage* >( pInfo->GetSdrPage() );
    SdrOutliner* pOutliner = dynamic_cast< SdrOutliner* >( pInfo->GetOutliner() );

    // special case, someone already set the current page on the EditFieldInfo
    // This is used from the svx::UnoGraphicsExporter f.e.
    if( pPage )
    {
        bMasterView = false;
        return pPage;
    }

    // first try to check if we are inside the outline view
    sd::OutlineView* pSdView = nullptr;
    if( auto pOutlineViewShell = dynamic_cast<const sd::OutlineViewShell* >(pViewSh) )
        pSdView = static_cast<sd::OutlineView*>(pOutlineViewShell->GetView());

    if (pSdView != nullptr && (pOutliner ==  &pSdView->GetOutliner()))
    {
        // outline mode
        int nPgNum = 0;
        Outliner& rOutl = pSdView->GetOutliner();
        tools::Long nPos = pInfo->GetPara();
        sal_Int32 nParaPos = 0;

        for( Paragraph* pPara = rOutl.GetParagraph( 0 ); pPara && nPos >= 0; pPara = rOutl.GetParagraph( ++nParaPos ), nPos-- )
        {
            if( Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE ) )
                nPgNum++;
        }

        pPage = pViewSh->GetDoc()->GetSdPage( static_cast<sal_uInt16>(nPgNum), PageKind::Standard );
    }
    else
    {
        // draw mode, slide mode and preview. Get the processed page from the outliner
        if(pOutliner)
        {
            pPage = dynamic_cast< SdPage* >(const_cast< SdrPage* >(pOutliner->getVisualizedPage()));
        }

        // The path using GetPaintingPageView() and GetCurrentPaintingDisplayInfo()
        // is no longer needed. I debugged and checked all usages of PageNumber decompositions
        // which all use the new possibility of setting the visualized page at the SdrOutliner.

        // if all else failed, geht the current page from the object that is
        // currently formatted from the document
        if(!pPage)
        {
            const SdrTextObj* pTextObj = (pViewSh && pViewSh->GetDoc()) ? pViewSh->GetDoc()->GetFormattingTextObj() : nullptr;

            if( pTextObj )
            {
                pPage = dynamic_cast< SdPage* >( pTextObj->getSdrPageFromSdrObject() );
            }
        }

        if(pPage)
        {
            bMasterView = pPage->IsMasterPage();
        }
    }

    return pPage;
}

/**
 * Link for CalcFieldValue of Outliners
 */
IMPL_LINK(SdModule, CalcFieldValueHdl, EditFieldInfo*, pInfo, void)
{
    if (!pInfo)
        return;

    const SvxFieldData* pField = pInfo->GetField().GetField();
    ::sd::DrawDocShell*     pDocShell = nullptr;
    SdDrawDocument* pDoc = nullptr;

    SdrOutliner* pSdrOutliner = dynamic_cast< SdrOutliner* >( pInfo->GetOutliner() );
    if( pSdrOutliner )
    {
        const SdrTextObj* pTextObj = pSdrOutliner->GetTextObj();

        if( pTextObj )
            pDoc = dynamic_cast< SdDrawDocument* >( &pTextObj->getSdrModelFromSdrObject() );

        if( pDoc )
            pDocShell = pDoc->GetDocSh();
    }

    if( !pDocShell )
        pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );

    const SvxDateField* pDateField = nullptr;
    const SvxExtTimeField* pExtTimeField = nullptr;
    const SvxExtFileField* pExtFileField = nullptr;
    const SvxAuthorField* pAuthorField = nullptr;
    const SvxURLField* pURLField = nullptr;

    const editeng::CustomPropertyField* pCustomPropertyField = nullptr;

    if( (pDateField = dynamic_cast< const SvxDateField* >(pField)) != nullptr )
    {
        LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
        pInfo->SetRepresentation( pDateField->GetFormatted( *GetNumberFormatter(), eLang ) );
    }
    else if( (pExtTimeField = dynamic_cast< const SvxExtTimeField *>(pField)) != nullptr )
    {
        LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
        pInfo->SetRepresentation( pExtTimeField->GetFormatted( *GetNumberFormatter(), eLang ) );
    }
    else if( (pExtFileField = dynamic_cast< const SvxExtFileField * >(pField)) != nullptr )
    {
        if( pDocShell && (pExtFileField->GetType() != SvxFileType::Fix) )
        {
            OUString aName;
            if( pDocShell->HasName() )
                aName = pDocShell->GetMedium()->GetName();
            else
                aName = pDocShell->GetName();

            const_cast< SvxExtFileField* >(pExtFileField)->SetFile( aName );
        }
        pInfo->SetRepresentation( pExtFileField->GetFormatted() );

    }
    else if( (pAuthorField = dynamic_cast< const SvxAuthorField* >( pField )) != nullptr  )
    {
        if( pAuthorField->GetType() != SvxAuthorType::Fix )
        {
            SvtUserOptions aUserOptions;
            *const_cast< SvxAuthorField* >(pAuthorField) = SvxAuthorField(
                    aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID(),
                    pAuthorField->GetType(), pAuthorField->GetFormat());
        }
        pInfo->SetRepresentation( pAuthorField->GetFormatted() );

    }
    else if( dynamic_cast< const SvxPageField*  >(pField) )
    {
        OUString aRepresentation(u" "_ustr);

        ::sd::ViewShell* pViewSh = pDocShell ? pDocShell->GetViewShell() : nullptr;
        if(pViewSh == nullptr)
        {
            ::sd::ViewShellBase* pBase = dynamic_cast< ::sd::ViewShellBase *>( SfxViewShell::Current() );
            if(pBase)
                pViewSh = pBase->GetMainViewShell().get();
        }
        if( !pDoc && pViewSh )
            pDoc = pViewSh->GetDoc();

        bool bMasterView;
        SdPage* pPage = GetCurrentPage( pViewSh, pInfo, bMasterView );

        if( pPage && pDoc && !bMasterView )
        {
            int nPgNum;

            if( (pPage->GetPageKind() == PageKind::Handout) && pViewSh )
            {
                nPgNum = pViewSh->GetPrintedHandoutPageNum();
            }
            else
            {
                nPgNum = (pPage->GetPageNum() - 1) / 2 + 1;
            }
            aRepresentation = pDoc->CreatePageNumValue(static_cast<sal_uInt16>(nPgNum));
        }
        else
            aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_NUMBER);

        pInfo->SetRepresentation( aRepresentation );
    }
    else if( dynamic_cast< const SvxPageTitleField*  >(pField) )
    {
        OUString aRepresentation(u" "_ustr);

        ::sd::ViewShell* pViewSh = pDocShell ? pDocShell->GetViewShell() : nullptr;
        if(pViewSh == nullptr)
        {
            ::sd::ViewShellBase* pBase = dynamic_cast< ::sd::ViewShellBase *>( SfxViewShell::Current() );
            if(pBase)
                pViewSh = pBase->GetMainViewShell().get();
        }
        if( !pDoc && pViewSh )
            pDoc = pViewSh->GetDoc();

        bool bMasterView;
        SdPage* pPage = GetCurrentPage( pViewSh, pInfo, bMasterView );

        if( pPage && pDoc && !bMasterView )
        {
            aRepresentation = pPage->GetName();
        }
        else
        {
            DocumentType eDocType = pDoc ? pDoc->GetDocumentType() : DocumentType::Impress;
            aRepresentation = ( ( eDocType == DocumentType::Impress )
                                ? SdResId(STR_FIELD_PLACEHOLDER_SLIDENAME)
                                : SdResId(STR_FIELD_PLACEHOLDER_PAGENAME) );
        }

        pInfo->SetRepresentation( aRepresentation );
    }
    else if( dynamic_cast< const SvxPagesField*  >(pField) )
    {
        OUString aRepresentation(u" "_ustr);

        ::sd::ViewShell* pViewSh = pDocShell ? pDocShell->GetViewShell() : nullptr;
        if(pViewSh == nullptr)
        {
            ::sd::ViewShellBase* pBase = dynamic_cast< ::sd::ViewShellBase *>( SfxViewShell::Current() );
            if(pBase)
                pViewSh = pBase->GetMainViewShell().get();
        }
        if( !pDoc && pViewSh )
            pDoc = pViewSh->GetDoc();

        bool bMasterView;
        SdPage* pPage = GetCurrentPage( pViewSh, pInfo, bMasterView );

        sal_uInt16 nPageCount = 0;

        if( !bMasterView )
        {
            if (pPage)
            {
                const auto ePageKind = pPage->GetPageKind();
                if (pViewSh && ePageKind == PageKind::Handout)
                {
                    nPageCount = pViewSh->GetPrintedHandoutPageCount();
                }
                else if (pDoc)
                {
                    // tdf#68320 - include hidden pages in slide count field
                    nPageCount = pDoc->GetSdPageCount(ePageKind);
                }
            }
            else if (pDoc)
            {
                nPageCount = pDoc->GetActiveSdPageCount();
            }
        }

        if( nPageCount > 0 )
            aRepresentation = pDoc->CreatePageNumValue(nPageCount);
        else
            aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_COUNT);

        pInfo->SetRepresentation( aRepresentation );
    }
    else if( (pURLField = dynamic_cast< const SvxURLField* >(pField)) != nullptr )
    {
        switch ( pURLField->GetFormat() )
        {
            case SvxURLFormat::AppDefault: //!!! adjustable at App???
            case SvxURLFormat::Repr:
                pInfo->SetRepresentation( pURLField->GetRepresentation() );
                break;

            case SvxURLFormat::Url:
                pInfo->SetRepresentation( pURLField->GetURL() );
                break;
        }

        const OUString& aURL = pURLField->GetURL();

        svtools::ColorConfig aConfig;
        svtools::ColorConfigEntry eEntry =
            INetURLHistory::GetOrCreate()->QueryUrl( aURL ) ? svtools::LINKSVISITED : svtools::LINKS;
        pInfo->SetTextColor( aConfig.GetColorValue(eEntry).nColor );
    }
    else if ( dynamic_cast< const SdrMeasureField* >(pField))
    {
        pInfo->SetFieldColor(std::optional<Color>()); // clear the field color
    }
    else if ((pCustomPropertyField = dynamic_cast<const editeng::CustomPropertyField*>(pField)) != nullptr)
    {
        try
        {
            SfxObjectShell* pObjSh = SfxObjectShell::Current();
            if (pObjSh && pObjSh->IsLoadingFinished())
            {
                auto pNonConstCustomPropertyField = const_cast<editeng::CustomPropertyField*>(pCustomPropertyField);
                OUString sCurrent = pNonConstCustomPropertyField->GetFormatted(pObjSh->getDocProperties());
                pInfo->SetRepresentation(sCurrent);
            }
            else
                pInfo->SetRepresentation(pCustomPropertyField->GetCurrentPresentation());
        }
        catch (...)
        {
            pInfo->SetRepresentation(pCustomPropertyField->GetCurrentPresentation());
        }
    }
    else
    {
        OUString aRepresentation;

        bool bHeaderField = dynamic_cast< const SvxHeaderField* >( pField ) != nullptr;
        bool bFooterField = !bHeaderField && (dynamic_cast< const SvxFooterField* >( pField ) != nullptr );
        bool bDateTimeField = !bHeaderField && !bFooterField && (dynamic_cast< const SvxDateTimeField* >( pField ) != nullptr);

        if( bHeaderField || bFooterField || bDateTimeField )
        {
            sd::ViewShell* pViewSh = pDocShell ? pDocShell->GetViewShell() : nullptr;
            bool bMasterView = false;
            SdPage* pPage = GetCurrentPage( pViewSh, pInfo, bMasterView );

            if( (pPage == nullptr) || bMasterView )
            {
                if( bHeaderField )
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_HEADER);
                else if (bFooterField )
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_FOOTER);
                else if (bDateTimeField )
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_DATETIME);
            }
            else
            {
                const sd::HeaderFooterSettings &rSettings = pPage->getHeaderFooterSettings();

                if( bHeaderField )
                {
                    aRepresentation = rSettings.maHeaderText;
                }
                else if( bFooterField )
                {
                    aRepresentation = rSettings.maFooterText;
                }
                else if( bDateTimeField )
                {
                    if( rSettings.mbDateTimeIsFixed )
                    {
                        aRepresentation = rSettings.maDateTimeText;
                    }
                    else
                    {
                        DateTime aDateTime( DateTime::SYSTEM );
                        LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
                        aRepresentation = SvxDateTimeField::GetFormatted( aDateTime, aDateTime,
                                              rSettings.meDateFormat, rSettings.meTimeFormat, *GetNumberFormatter(), eLang );
                    }
                }
            }
        }
        else
        {
            OSL_FAIL("sd::SdModule::CalcFieldValueHdl(), unknown field type!");
        }

        if( aRepresentation.isEmpty() )                // TODO: Edit engine doesn't handle empty fields?
            aRepresentation = u" "_ustr;
        pInfo->SetRepresentation( aRepresentation );
    }
}

/**
 * virtual methods for option dialog
 */
SfxStyleFamilies SdModule::CreateStyleFamilies()
{
    SfxStyleFamilies aStyleFamilies;
    std::locale resLocale = SdModule::get()->GetResLocale();

    aStyleFamilies.emplace_back(SfxStyleFamily::Para,
                                 SdResId(STR_GRAPHICS_STYLE_FAMILY),
                                 BMP_STYLES_FAMILY_GRAPHICS,
                                 RID_GRAPHICSTYLEFAMILY, resLocale);

    aStyleFamilies.emplace_back(SfxStyleFamily::Pseudo,
                                 SdResId(STR_PRESENTATIONS_STYLE_FAMILY),
                                 BMP_STYLES_FAMILY_PRESENTATIONS,
                                 RID_PRESENTATIONSTYLEFAMILY, resLocale);

    return aStyleFamilies;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
