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

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <sfx2/printer.hxx>
#include <svl/inethist.hxx>
#include <svl/poolitem.hxx>
#include <svl/flagitem.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>

#include <editeng/measfld.hxx>
#include <editeng/editstat.hxx>
#include <editeng/editeng.hxx>

#include <svx/dialogs.hrc>
#include <svx/svdotext.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopage.hxx>

#include <sfx2/sfxdlg.hxx>

#include <svx/sdr/contact/displayinfo.hxx>

#include "sdmod.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "ViewShell.hxx"
#include "FrameView.hxx"
#include "sdattr.hxx"
#include "optsitem.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "Outliner.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include "OutlineView.hxx"
#include "ViewShellBase.hxx"
#include "sdpage.hxx"
#include "sdxfer.hxx"
#include "sdabstdlg.hxx"
#include <svl/intitem.hxx>

/** retrieves the page that is currently painted. This will only be the master page
    if the current drawn view only shows the master page*/
static SdPage* GetCurrentPage( sd::ViewShell* pViewSh, EditFieldInfo* pInfo, bool& bMasterView )
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
    if( dynamic_cast<const sd::OutlineViewShell* >(pViewSh) !=  nullptr )
        pSdView = static_cast<sd::OutlineView*> (static_cast<sd::OutlineViewShell*>(pViewSh)->GetView());

    if (pSdView != nullptr && (pOutliner ==  &pSdView->GetOutliner()))
    {
        // outline mode
        int nPgNum = 0;
        Outliner& rOutl = pSdView->GetOutliner();
        long nPos = pInfo->GetPara();
        sal_Int32 nParaPos = 0;

        for( Paragraph* pPara = rOutl.GetParagraph( 0 ); pPara && nPos >= 0; pPara = rOutl.GetParagraph( ++nParaPos ), nPos-- )
        {
            if( Outliner::HasParaFlag( pPara, ParaFlag::ISPAGE ) )
                nPgNum++;
        }

        pPage = pViewSh->GetDoc()->GetSdPage( (sal_uInt16)nPgNum, PK_STANDARD );
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
                pPage = dynamic_cast< SdPage* >( pTextObj->GetPage() );
            }
        }

        if(pPage)
        {
            bMasterView = pPage && pPage->IsMasterPage();
        }
    }

    return pPage;
}

/**
 * Link for CalcFieldValue of Outliners
 */
IMPL_LINK_TYPED(SdModule, CalcFieldValueHdl, EditFieldInfo*, pInfo, void)
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
            pDoc = dynamic_cast< SdDrawDocument* >( pTextObj->GetModel() );

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
        if( pDocShell && (pExtFileField->GetType() != SVXFILETYPE_FIX) )
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
        if( pAuthorField->GetType() != SVXAUTHORTYPE_FIX )
        {
            SvtUserOptions aUserOptions;
            SvxAuthorField aAuthorField(
                    aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID(),
                    pAuthorField->GetType(), pAuthorField->GetFormat() );

            *(const_cast< SvxAuthorField* >(pAuthorField)) = aAuthorField;
        }
        pInfo->SetRepresentation( pAuthorField->GetFormatted() );

    }
    else if( dynamic_cast< const SvxPageField*  >(pField) )
    {
        OUString aRepresentation(" ");

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

            if( (pPage->GetPageKind() == PK_HANDOUT) && pViewSh )
            {
                nPgNum = pViewSh->GetPrintedHandoutPageNum();
            }
            else
            {
                nPgNum = (pPage->GetPageNum() - 1) / 2 + 1;
            }
            aRepresentation = pDoc->CreatePageNumValue((sal_uInt16)nPgNum);
        }
        else
            aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_NUMBER).toString();

        pInfo->SetRepresentation( aRepresentation );
    }

    else if( dynamic_cast< const SvxPageTitleField*  >(pField) )
    {
        OUString aRepresentation(" ");

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
            DocumentType eDocType = pDoc ? pDoc->GetDocumentType() : DOCUMENT_TYPE_IMPRESS;
            aRepresentation = ( ( eDocType == DOCUMENT_TYPE_IMPRESS )
                                ? SdResId(STR_FIELD_PLACEHOLDER_SLIDENAME).toString()
                                : SdResId(STR_FIELD_PLACEHOLDER_PAGENAME).toString() );
        }

        pInfo->SetRepresentation( aRepresentation );
    }
    else if( dynamic_cast< const SvxPagesField*  >(pField) )
    {
        OUString aRepresentation(" ");

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
            if( pPage && (pPage->GetPageKind() == PK_HANDOUT) && pViewSh )
            {
                nPageCount = pViewSh->GetPrintedHandoutPageCount();
            }
            else if( pDoc )
            {
                nPageCount = (sal_uInt16)pDoc->GetActiveSdPageCount();
            }
        }

        if( nPageCount > 0 )
            aRepresentation = pDoc->CreatePageNumValue(nPageCount);
        else
            aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_COUNT).toString();

        pInfo->SetRepresentation( aRepresentation );
    }
    else if( (pURLField = dynamic_cast< const SvxURLField* >(pField)) != nullptr )
    {
        switch ( pURLField->GetFormat() )
        {
            case SVXURLFORMAT_APPDEFAULT: //!!! adjustable at App???
            case SVXURLFORMAT_REPR:
                pInfo->SetRepresentation( pURLField->GetRepresentation() );
                break;

            case SVXURLFORMAT_URL:
                pInfo->SetRepresentation( pURLField->GetURL() );
                break;
        }

        OUString aURL = pURLField->GetURL();

        svtools::ColorConfig aConfig;
        svtools::ColorConfigEntry eEntry =
            INetURLHistory::GetOrCreate()->QueryUrl( aURL ) ? svtools::LINKSVISITED : svtools::LINKS;
        pInfo->SetTextColor( aConfig.GetColorValue(eEntry).nColor );
    }
    else if ( dynamic_cast< const SdrMeasureField* >(pField))
    {
        pInfo->ClearFieldColor();
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
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_HEADER).toString();
                else if (bFooterField )
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_FOOTER).toString();
                else if (bDateTimeField )
                    aRepresentation = SdResId(STR_FIELD_PLACEHOLDER_DATETIME).toString();
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
                        Date aDate( Date::SYSTEM );
                        tools::Time aTime( tools::Time::SYSTEM );
                        LanguageType eLang = pInfo->GetOutliner()->GetLanguage( pInfo->GetPara(), pInfo->GetPos() );
                        aRepresentation = SvxDateTimeField::GetFormatted( aDate, aTime, (SvxDateFormat)rSettings.meDateTimeFormat, *GetNumberFormatter(), eLang );
                    }
                }
            }
        }
        else
        {
            OSL_FAIL("sd::SdModule::CalcFieldValueHdl(), unknown field type!");
        }

        if( aRepresentation.isEmpty() )                // TODO: Edit engine doesn't handle empty fields?
            aRepresentation = " ";
        pInfo->SetRepresentation( aRepresentation );
    }
}

/**
 * virtual methods for option dialog
 */
SfxItemSet*  SdModule::CreateItemSet( sal_uInt16 nSlot )
{
    ::sd::FrameView* pFrameView = nullptr;
    ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = nullptr;

    // Here we set the DocType of the option dialog (not document!)
    DocumentType eDocType = DOCUMENT_TYPE_IMPRESS;
    if( nSlot == SID_SD_GRAPHIC_OPTIONS )
        eDocType = DOCUMENT_TYPE_DRAW;

    if (pDocSh)
    {
        pDoc = pDocSh->GetDoc();

        // If the option dialog is identical to the document type,
        // we can pass the FrameView too:
        if( pDoc && eDocType == pDoc->GetDocumentType() )
            pFrameView = pDocSh->GetFrameView();

        ::sd::ViewShell* pViewShell = pDocSh->GetViewShell();
        if (pViewShell != nullptr)
            pViewShell->WriteFrameViewData();
    }

    SdOptions* pOptions = GetSdOptions(eDocType);

    // Pool has by default MapUnit Twips (Awgh!)
    SfxItemPool& rPool = GetPool();
    rPool.SetDefaultMetric( SFX_MAPUNIT_100TH_MM );

    SfxItemSet*  pRet = new SfxItemSet( rPool,
                        SID_ATTR_METRIC, SID_ATTR_METRIC,
                        SID_ATTR_DEFTABSTOP, SID_ATTR_DEFTABSTOP,

                        ATTR_OPTIONS_LAYOUT, ATTR_OPTIONS_LAYOUT,
                        ATTR_OPTIONS_CONTENTS, ATTR_OPTIONS_CONTENTS,
                        ATTR_OPTIONS_MISC, ATTR_OPTIONS_MISC,

                        ATTR_OPTIONS_SNAP, ATTR_OPTIONS_SNAP,

                        ATTR_OPTIONS_SCALE_START, ATTR_OPTIONS_SCALE_END,

                        ATTR_OPTIONS_PRINT, ATTR_OPTIONS_PRINT,

                        SID_ATTR_GRID_OPTIONS, SID_ATTR_GRID_OPTIONS,
                        0 );

    // TP_OPTIONS_LAYOUT:
    pRet->Put( SdOptionsLayoutItem( ATTR_OPTIONS_LAYOUT, pOptions, pFrameView ) );

    sal_uInt16 nDefTab = 0;
    if( pFrameView)
        nDefTab = pDoc->GetDefaultTabulator();
    else
        nDefTab = pOptions->GetDefTab();
    pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP, nDefTab ) );

    FieldUnit nMetric = (FieldUnit)0xffff;
    if( pFrameView)
        nMetric = pDoc->GetUIUnit();
    else
        nMetric = (FieldUnit)pOptions->GetMetric();

    if( nMetric == (FieldUnit)0xffff )
        nMetric = GetFieldUnit();

    pRet->Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)nMetric ) );

    // TP_OPTIONS_CONTENTS:
    pRet->Put( SdOptionsContentsItem( ATTR_OPTIONS_CONTENTS, pOptions, pFrameView ) );

    // TP_OPTIONS_MISC:
    SdOptionsMiscItem aSdOptionsMiscItem( ATTR_OPTIONS_MISC, pOptions, pFrameView );
    if ( pFrameView )
    {
        aSdOptionsMiscItem.GetOptionsMisc().SetSummationOfParagraphs( pDoc->IsSummationOfParagraphs() );
        aSdOptionsMiscItem.GetOptionsMisc().SetPrinterIndependentLayout (
            (sal_uInt16)pDoc->GetPrinterIndependentLayout());
    }
    pRet->Put( aSdOptionsMiscItem );

    // TP_OPTIONS_SNAP:
    pRet->Put( SdOptionsSnapItem( ATTR_OPTIONS_SNAP, pOptions, pFrameView ) );

    // TP_SCALE:
    sal_uInt32 nW = 10L;
    sal_uInt32 nH = 10L;
    sal_Int32  nX;
    sal_Int32  nY;
    if( pDocSh )
    {
        SdrPage* pPage = static_cast<SdrPage*>(pDoc->GetSdPage(0, PK_STANDARD));
        Size aSize(pPage->GetSize());
        nW = aSize.Width();
        nH = aSize.Height();
    }

    if(pFrameView)
    {
        const Fraction& rFraction =  pDoc->GetUIScale();
        nX=rFraction.GetNumerator();
        nY=rFraction.GetDenominator();
    }
    else
    {
        // Get options from configuration file
        pOptions->GetScale( nX, nY );
    }

    pRet->Put( SfxInt32Item( ATTR_OPTIONS_SCALE_X, nX ) );
    pRet->Put( SfxInt32Item( ATTR_OPTIONS_SCALE_Y, nY ) );
    pRet->Put( SfxUInt32Item( ATTR_OPTIONS_SCALE_WIDTH, nW ) );
    pRet->Put( SfxUInt32Item( ATTR_OPTIONS_SCALE_HEIGHT, nH ) );

    // TP_OPTIONS_PRINT:
    pRet->Put( SdOptionsPrintItem( ATTR_OPTIONS_PRINT, pOptions ) );

    // RID_SVXPAGE_GRID:
    pRet->Put( SdOptionsGridItem( SID_ATTR_GRID_OPTIONS, pOptions ) );

    return pRet;
}
void SdModule::ApplyItemSet( sal_uInt16 nSlot, const SfxItemSet& rSet )
{
    const SfxPoolItem*  pItem = nullptr;
    bool bNewDefTab = false;
    bool bNewPrintOptions = false;
    bool bMiscOptions = false;

    ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
    SdDrawDocument* pDoc = nullptr;
    // Here we set the DocType of the option dialog (not document!)
    DocumentType eDocType = DOCUMENT_TYPE_IMPRESS;
    if( nSlot == SID_SD_GRAPHIC_OPTIONS )
        eDocType = DOCUMENT_TYPE_DRAW;

    ::sd::ViewShell* pViewShell = nullptr;

    if (pDocSh)
    {
        pDoc = pDocSh->GetDoc();

        pViewShell = pDocSh->GetViewShell();
        if (pViewShell != nullptr)
            pViewShell->WriteFrameViewData();
    }
    SdOptions* pOptions = GetSdOptions(eDocType);
    // Grid
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS ,
                            false, &pItem ))
    {
        const SdOptionsGridItem* pGridItem = static_cast<const SdOptionsGridItem*>(pItem);
        pGridItem->SetOptions( pOptions );
    }

    // Layout
    const SdOptionsLayoutItem* pLayoutItem = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_LAYOUT,
                            false, reinterpret_cast<const SfxPoolItem**>(&pLayoutItem) ))
    {
        pLayoutItem->SetOptions( pOptions );
    }

    // Metric
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_METRIC, false, &pItem ) )
    {
        if( pDoc && eDocType == pDoc->GetDocumentType() )
            PutItem( *pItem );
        pOptions->SetMetric( static_cast<const SfxUInt16Item*>( pItem )->GetValue() );
    }
    sal_uInt16 nDefTab = pOptions->GetDefTab();
    // Default-Tabulator
    if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_DEFTABSTOP, false, &pItem ) )
    {
        nDefTab = static_cast<const SfxUInt16Item*>( pItem )->GetValue();
        pOptions->SetDefTab( nDefTab );

        bNewDefTab = true;
    }

    // Scale
    if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_SCALE_X, false, &pItem ) )
    {
        sal_Int32 nX = static_cast<const SfxInt32Item*>( pItem )->GetValue();
        if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_SCALE_Y, false, &pItem ) )
        {
            sal_Int32 nY = static_cast<const SfxInt32Item*>( pItem )->GetValue();
            pOptions->SetScale( nX, nY );

            // Apply to document only if doc type match
            if( pDocSh && pDoc && eDocType == pDoc->GetDocumentType() )
            {
                pDoc->SetUIScale( Fraction( nX, nY ) );
                if( pViewShell )
                    pViewShell->SetRuler( pViewShell->HasRuler() );
            }
        }
    }

    // Misc
    const SdOptionsMiscItem* pMiscItem = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_MISC,
                            false, reinterpret_cast<const SfxPoolItem**>(&pMiscItem) ))
    {
        pMiscItem->SetOptions( pOptions );
        bMiscOptions = true;
    }

    // Snap
    const SdOptionsSnapItem* pSnapItem = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_SNAP,
                            false, reinterpret_cast<const SfxPoolItem**>(&pSnapItem) ))
    {
        pSnapItem->SetOptions( pOptions );
    }

    SfxItemSet aPrintSet( GetPool(),
                    SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                    ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                    0 );

    // Print
    const SdOptionsPrintItem* pPrintItem = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( ATTR_OPTIONS_PRINT,
                            false, reinterpret_cast<const SfxPoolItem**>(&pPrintItem) ))
    {
        pPrintItem->SetOptions( pOptions );

        // set PrintOptionsSet
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT, pOptions );
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        SfxPrinterChangeFlags nFlags =
                (aPrintItem.GetOptionsPrint().IsWarningSize() ? SfxPrinterChangeFlags::CHG_SIZE : SfxPrinterChangeFlags::NONE) |
                (aPrintItem.GetOptionsPrint().IsWarningOrientation() ? SfxPrinterChangeFlags::CHG_ORIENTATION : SfxPrinterChangeFlags::NONE);
        aFlagItem.SetValue( static_cast<int>(nFlags) );

        aPrintSet.Put( aPrintItem );
        aPrintSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.GetOptionsPrint().IsWarningPrinter() ) );
        aPrintSet.Put( aFlagItem );

        bNewPrintOptions = true;
    }

    // Only if also the document type matches...
    if( pDocSh && pDoc && eDocType == pDoc->GetDocumentType() )
    {
        if( bNewPrintOptions )
        {
            pDocSh->GetPrinter(true)->SetOptions( aPrintSet );
        }

        // set DefTab at Model
        if( bNewDefTab )
        {
            SdDrawDocument* pDocument = pDocSh->GetDoc();
            pDocument->SetDefaultTabulator( nDefTab );

            ::sd::Outliner* pOutl = pDocument->GetOutliner( false );
            if( pOutl )
                pOutl->SetDefTab( nDefTab );

            ::sd::Outliner* pInternalOutl = pDocument->GetInternalOutliner( false );
            if( pInternalOutl )
                pInternalOutl->SetDefTab( nDefTab );
        }
        if ( bMiscOptions )
        {
            pDoc->SetSummationOfParagraphs( pMiscItem->GetOptionsMisc().IsSummationOfParagraphs() );
            EEControlBits nSum = pMiscItem->GetOptionsMisc().IsSummationOfParagraphs() ? EEControlBits::ULSPACESUMMATION : EEControlBits::NONE;
            EEControlBits nCntrl;

            SdDrawDocument* pDocument = pDocSh->GetDoc();
            SdrOutliner& rOutl = pDocument->GetDrawOutliner();
            nCntrl = rOutl.GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
            rOutl.SetControlWord( nCntrl | nSum );
            ::sd::Outliner* pOutl = pDocument->GetOutliner( false );
            if( pOutl )
            {
                nCntrl = pOutl->GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                pOutl->SetControlWord( nCntrl | nSum );
            }
            pOutl = pDocument->GetInternalOutliner( false );
            if( pOutl )
            {
                nCntrl = pOutl->GetControlWord() &~ EEControlBits::ULSPACESUMMATION;
                pOutl->SetControlWord( nCntrl | nSum );
            }

            // Set printer independent layout mode.
            if( pDoc->GetPrinterIndependentLayout() != pMiscItem->GetOptionsMisc().GetPrinterIndependentLayout() )
                pDoc->SetPrinterIndependentLayout (pMiscItem->GetOptionsMisc().GetPrinterIndependentLayout());
        }
    }

    pOptions->StoreConfig();

    // Only if also the document type matches...
    if( pDocSh && pDoc && eDocType == pDoc->GetDocumentType() )
    {
        FieldUnit eUIUnit = (FieldUnit) pOptions->GetMetric();
        pDoc->SetUIUnit(eUIUnit);

        if (pViewShell)
        {
            // make sure no one is in text edit mode, cause there
            // are some pointers remembered else (!)
            if(pViewShell->GetView())
                pViewShell->GetView()->SdrEndTextEdit();

            ::sd::FrameView* pFrame = pViewShell->GetFrameView();
            pFrame->Update(pOptions);
            pViewShell->ReadFrameViewData(pFrame);
            pViewShell->SetUIUnit(eUIUnit);
            pViewShell->SetDefTabHRuler( nDefTab );
        }
    }

    if( pViewShell && pViewShell->GetViewFrame() )
        pViewShell->GetViewFrame()->GetBindings().InvalidateAll( true );
}

VclPtr<SfxTabPage> SdModule::CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet )
{
    VclPtr<SfxTabPage> pRet;
    SfxAllItemSet aSet(*(rSet.GetPool()));
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    if( pFact )
    {
        switch(nId)
        {
            case SID_SD_TP_CONTENTS:
            case SID_SI_TP_CONTENTS:
            {   ::CreateTabPage fnCreatePage = pFact->GetSdOptionsContentsTabPageCreatorFunc();
                if( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, &rSet );
            }
            break;
            case SID_SD_TP_SNAP:
            case SID_SI_TP_SNAP:
            {   ::CreateTabPage fnCreatePage = pFact->GetSdOptionsSnapTabPageCreatorFunc();
                if( fnCreatePage )
                    pRet = (*fnCreatePage)( pParent, &rSet );
            }
            break;
            case SID_SD_TP_PRINT:
            case SID_SI_TP_PRINT:
            {
                ::CreateTabPage fnCreatePage = pFact->GetSdPrintOptionsTabPageCreatorFunc();
                if( fnCreatePage )
                {
                    pRet = (*fnCreatePage)( pParent, &rSet );
                    if(SID_SD_TP_PRINT == nId)
                        aSet.Put (SfxUInt32Item(SID_SDMODE_FLAG,SD_DRAW_MODE));
                    pRet->PageCreated(aSet);
                }
            }
            break;
            case SID_SI_TP_MISC:
            case SID_SD_TP_MISC:
            {
                ::CreateTabPage fnCreatePage = pFact->GetSdOptionsMiscTabPageCreatorFunc();
                if( fnCreatePage )
                {
                    pRet = (*fnCreatePage)( pParent, &rSet );
                    if(SID_SD_TP_MISC == nId)
                        aSet.Put (SfxUInt32Item(SID_SDMODE_FLAG,SD_DRAW_MODE));
                    else
                        aSet.Put (SfxUInt32Item(SID_SDMODE_FLAG,SD_IMPRESS_MODE));
                    pRet->PageCreated(aSet);
                }
            }
            break;
            case RID_SVXPAGE_TEXTANIMATION :
            {
                SfxAbstractDialogFactory* pSfxFact = SfxAbstractDialogFactory::Create();
                if ( pSfxFact )
                {
                    ::CreateTabPage fnCreatePage = pSfxFact->GetTabPageCreatorFunc( nId );
                    if ( fnCreatePage )
                        pRet = (*fnCreatePage)( pParent, &rSet );
                }
            }
            break;
        }
        DBG_ASSERT( pRet, "SdModule::CreateTabPage(): no valid ID for TabPage!" );
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
