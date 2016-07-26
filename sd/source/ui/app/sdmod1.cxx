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

#include <svl/lckbitem.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/moduleoptions.hxx>
#include "framework/FrameworkHelper.hxx"

#include <svx/dialogs.hrc>

#include <editeng/langitem.hxx>
#include <editeng/editdata.hxx>
#include <vcl/msgbox.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/templatedlg.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/eeitem.hxx>
#include <unotools/useroptions.hxx>
#include <com/sun/star/uno/Sequence.h>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#include "sdmod.hxx"
#include "pres.hxx"
#include "optsitem.hxx"
#include "ViewShell.hxx"
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "assclass.hxx"
#include "sdenumdef.hxx"
#include "sdresid.hxx"
#include "OutlineViewShell.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "FactoryIds.hxx"
#include "sdabstdlg.hxx"
#include <memory>
#include "slideshow.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

using ::sd::framework::FrameworkHelper;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;
using ::com::sun::star::uno::Sequence;

namespace {

class OutlineToImpressFinalizer
{
public:
    OutlineToImpressFinalizer (
        ::sd::ViewShellBase& rBase,
        SdDrawDocument& rDocument,
        SvLockBytes& rBytes);
    virtual ~OutlineToImpressFinalizer() {};
    void operator() (bool bEventSeen);
private:
    ::sd::ViewShellBase& mrBase;
    SdDrawDocument& mrDocument;
    std::shared_ptr<SvMemoryStream> mpStream;
};

} //end of anonymous namespace

void SdModule::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    sal_uLong nSlotId = rReq.GetSlot();

    switch ( nSlotId )
    {
        case SID_NEWDOC:
        {
            SfxGetpApp()->ExecuteSlot(rReq, SfxGetpApp()->GetInterface());
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            // automatic spell checker
            const SfxPoolItem* pItem;
            if( pSet && SfxItemState::SET == pSet->GetItemState(
                        SID_AUTOSPELL_CHECK, false, &pItem ) )
            {
                bool bOnlineSpelling = static_cast<const SfxBoolItem*>( pItem )->GetValue();
                // save at document:
                ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
                if( pDocSh )
                {
                    SdDrawDocument* pDoc = pDocSh->GetDoc();
                    pDoc->SetOnlineSpell( bOnlineSpelling );
                }
            }
        }
        break;

        case SID_ATTR_METRIC:
        {
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET == pSet->GetItemState( SID_ATTR_METRIC, true, &pItem ) )
            {
                FieldUnit eUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                switch( eUnit )
                {
                    case FUNIT_MM:      // only the units which are also in the dialog
                    case FUNIT_CM:
                    case FUNIT_INCH:
                    case FUNIT_PICA:
                    case FUNIT_POINT:
                        {
                            ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
                            if(pDocSh)
                            {
                                DocumentType eDocType = pDocSh->GetDoc()->GetDocumentType();

                                PutItem( *pItem );
                                SdOptions* pOptions = GetSdOptions( eDocType );
                                if(pOptions)
                                    pOptions->SetMetric( (sal_uInt16)eUnit );
                                rReq.Done();
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }

        }
        break;

        case SID_ATTR_LANGUAGE:
        case SID_ATTR_CHAR_CJK_LANGUAGE:
        case SID_ATTR_CHAR_CTL_LANGUAGE:
        {
            const SfxPoolItem* pItem;
            if( pSet &&
                (
                SfxItemState::SET == pSet->GetItemState(SID_ATTR_LANGUAGE, false, &pItem ) ||
                SfxItemState::SET == pSet->GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, false, &pItem ) ||
                SfxItemState::SET == pSet->GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, false, &pItem )
                )
              )
            {
                // save at the document:
                ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
                if ( pDocSh )
                {
                    LanguageType eLanguage = static_cast<const SvxLanguageItem*>(pItem)->GetValue();
                    SdDrawDocument* pDoc = pDocSh->GetDoc();

                    if( nSlotId == SID_ATTR_CHAR_CJK_LANGUAGE )
                        pDoc->SetLanguage( eLanguage, EE_CHAR_LANGUAGE_CJK );
                    else if( nSlotId == SID_ATTR_CHAR_CTL_LANGUAGE )
                        pDoc->SetLanguage( eLanguage, EE_CHAR_LANGUAGE_CTL );
                    else
                        pDoc->SetLanguage( eLanguage, EE_CHAR_LANGUAGE );

                    if( pDoc->GetOnlineSpell() )
                    {
                        pDoc->StopOnlineSpelling();
                        pDoc->StartOnlineSpelling();
                    }
                }
            }
        }
        break;

        case SID_SD_AUTOPILOT:
        case SID_NEWSD:
            {
                SfxFrame* pFrame = ExecuteNewDocument( rReq );
                // if a frame was created, set it as return value
                if(pFrame)
                    rReq.SetReturnValue(SfxFrameItem(0, pFrame));
            }

            break;

        case SID_OPENHYPERLINK:
        case SID_OPENDOC:
        {
            bool bIntercept = false;
            ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
            if (pDocShell)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if( sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ) )
                    {
                        // Prevent documents from opening while the slide
                        // show is running, except when this request comes
                        // from a shape interaction.
                        if (rReq.GetArgs() == nullptr)
                        {
                            bIntercept = true;
                        }
                    }
                }
            }

            if (!bIntercept)
            {
                SfxGetpApp()->ExecuteSlot(rReq, SfxGetpApp()->GetInterface());
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog>(nullptr, SD_RESSTR(STR_CANT_PERFORM_IN_LIVEMODE))->Execute();

                const SfxLinkItem* pLinkItem = rReq.GetArg<SfxLinkItem>(SID_DONELINK);
                if( pLinkItem )
                    pLinkItem->GetValue().Call( nullptr );
            }
        }
        break;

        case SID_OUTLINE_TO_IMPRESS:
            OutlineToImpress (rReq);
            break;

        default:
        break;
    }
}

bool SdModule::OutlineToImpress(SfxRequest& rRequest)
{
    const SfxItemSet* pSet = rRequest.GetArgs();

    if (pSet)
    {
        SvLockBytes* pBytes = static_cast<const SfxLockBytesItem&>(pSet->Get(SID_OUTLINE_TO_IMPRESS)).GetValue();

        if (pBytes)
        {
            SfxObjectShellLock xDocShell;
            ::sd::DrawDocShell* pDocSh;
            xDocShell = pDocSh = new ::sd::DrawDocShell(
                SfxObjectCreateMode::STANDARD, false);

            pDocSh->DoInitNew();
            SdDrawDocument* pDoc = pDocSh->GetDoc();
            if(pDoc)
            {
                pDoc->CreateFirstPages();
                pDoc->StopWorkStartupDelay();
            }

            const SfxFrameItem* pFrmItem = rRequest.GetArg<SfxFrameItem>(SID_DOCFRAME);
            SfxViewFrame::LoadDocumentIntoFrame( *pDocSh, pFrmItem, ::sd::OUTLINE_FACTORY_ID );

            ::sd::ViewShell* pViewSh = pDocSh->GetViewShell();

            if (pViewSh && pDoc)
            {
                // AutoLayouts have to be finished
                pDoc->StopWorkStartupDelay();

                SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();

                // When the view frame has not been just created we have
                // to switch synchronously to the outline view.
                // (Otherwise the request will be ignored anyway.)
                ::sd::ViewShellBase* pBase
                    = dynamic_cast< ::sd::ViewShellBase*>(pViewFrame->GetViewShell());
                if (pBase != nullptr)
                {
                    std::shared_ptr<FrameworkHelper> pHelper (
                        FrameworkHelper::Instance(*pBase));
                    pHelper->RequestView(
                        FrameworkHelper::msOutlineViewURL,
                        FrameworkHelper::msCenterPaneURL);

                    pHelper->RunOnResourceActivation(
                        FrameworkHelper::CreateResourceId(
                        FrameworkHelper::msOutlineViewURL,
                        FrameworkHelper::msCenterPaneURL),
                        OutlineToImpressFinalizer(*pBase, *pDoc, *pBytes));
                }
            }
        }
    }

    return rRequest.IsDone();
}

void SdModule::GetState(SfxItemSet& rItemSet)
{
    // disable Autopilot during presentation
    if (rItemSet.GetItemState(SID_SD_AUTOPILOT) != SfxItemState::UNKNOWN)
    {
        if (!SvtModuleOptions().IsImpress())
        {
            rItemSet.DisableItem(SID_SD_AUTOPILOT);
        }
        else
        {
            ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
            if (pDocShell)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if( sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ) )
                    {
                        rItemSet.DisableItem(SID_SD_AUTOPILOT);
                    }
                }
            }
        }
    }

    if( SfxItemState::DEFAULT == rItemSet.GetItemState( SID_ATTR_METRIC ) )
    {
        ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if(pDocSh)
        {
            DocumentType eDocType = pDocSh->GetDoc()->GetDocumentType();

            SdOptions* pOptions = GetSdOptions(eDocType);
            rItemSet.Put( SfxUInt16Item( SID_ATTR_METRIC, pOptions->GetMetric() ) );
        }
    }

    // state of SID_OPENDOC is determined by the base class
    if (rItemSet.GetItemState(SID_OPENDOC) != SfxItemState::UNKNOWN)
    {
        const SfxPoolItem* pItem = SfxGetpApp()->GetSlotState(SID_OPENDOC, SfxGetpApp()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    // state of SID_OPENHYPERLINK is determined by the base class
    if (rItemSet.GetItemState(SID_OPENHYPERLINK) != SfxItemState::UNKNOWN)
    {
        const SfxPoolItem* pItem = SfxGetpApp()->GetSlotState(SID_OPENHYPERLINK, SfxGetpApp()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    if( SfxItemState::DEFAULT == rItemSet.GetItemState( SID_AUTOSPELL_CHECK ) )
    {
        ::sd::DrawDocShell* pDocSh =
              dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if( pDocSh )
        {
            SdDrawDocument* pDoc = pDocSh->GetDoc();
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_CHECK, pDoc->GetOnlineSpell() ) );
        }
    }

    if( SfxItemState::DEFAULT == rItemSet.GetItemState( SID_ATTR_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE ), SID_ATTR_LANGUAGE ) );
    }

    if( SfxItemState::DEFAULT == rItemSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CJK ), SID_ATTR_CHAR_CJK_LANGUAGE ) );
    }

    if( SfxItemState::DEFAULT == rItemSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CTL ), SID_ATTR_CHAR_CTL_LANGUAGE ) );
    }

    if ( !mbEventListenerAdded )
    {
        ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        if( pDocShell ) // Impress or Draw ?
        {
            ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

            if( pViewShell && (pDocShell->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) )
            {
                // add our event listener as soon as possible
                Application::AddEventListener( LINK( this, SdModule, EventListenerHdl ) );
                mbEventListenerAdded = true;
            }
        }
    }
}

IMPL_STATIC_LINK_TYPED( SdModule, EventListenerHdl, VclSimpleEvent&, rSimpleEvent, void )
{
    if( (rSimpleEvent.GetId() == VCLEVENT_WINDOW_COMMAND) && static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData() )
    {
        const CommandEvent& rEvent = *static_cast<const CommandEvent*>(static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData());

        if( rEvent.GetCommand() == CommandEventId::Media )
        {
            CommandMediaData* pMediaData = rEvent.GetMediaData();
            pMediaData->SetPassThroughToOS(false);
            switch (pMediaData->GetMediaId())
            {
                case MediaCommand::Play:
                {
                    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
                    if( pDocShell )  // Impress or Draw ?
                    {
                        ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

                        // #i97925# start the presentation if and only if an Impress document is focused
                        if( pViewShell && (pDocShell->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) )
                            pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION );
                    }
                    break;
                }
                default:
                    pMediaData->SetPassThroughToOS(true);
                    break;
            }
        }
    }
}

void SdModule::AddSummaryPage (SfxViewFrame* pViewFrame, SdDrawDocument* pDocument)
{
    if( !pViewFrame || !pViewFrame->GetDispatcher() || !pDocument )
        return;

    pViewFrame->GetDispatcher()->Execute(SID_SUMMARY_PAGE,
        SfxCallMode::SYNCHRON | SfxCallMode::RECORD);

    OSL_ASSERT (pDocument!=nullptr);

    sal_Int32 nPageCount = pDocument->GetSdPageCount (PK_STANDARD);

    // We need at least two pages: the summary page and one to use as
    // template to take the transition parameters from.
    if (nPageCount >= 2)
    {
        // Get a page from which to retrieve the transition parameters.
        SdPage* pTemplatePage = pDocument->GetSdPage (0, PK_STANDARD);
        OSL_ASSERT (pTemplatePage!=nullptr);

        // The summary page, if it exists, is the last page.
        SdPage* pSummaryPage = pDocument->GetSdPage (
            (sal_uInt16)nPageCount-1, PK_STANDARD);
        OSL_ASSERT (pSummaryPage!=nullptr);

        // Take the change mode of the template page as indication of the
        // document's kiosk mode.
        pSummaryPage->setTransitionDuration(pTemplatePage->getTransitionDuration());
        pSummaryPage->SetPresChange(pTemplatePage->GetPresChange());
        pSummaryPage->SetTime(pTemplatePage->GetTime());
        pSummaryPage->SetSound(pTemplatePage->IsSoundOn());
        pSummaryPage->SetSoundFile(pTemplatePage->GetSoundFile());
        pSummaryPage->setTransitionType(pTemplatePage->getTransitionType());
        pSummaryPage->setTransitionSubtype(pTemplatePage->getTransitionSubtype());
        pSummaryPage->setTransitionDirection(pTemplatePage->getTransitionDirection());
        pSummaryPage->setTransitionFadeColor(pTemplatePage->getTransitionFadeColor());
        pSummaryPage->setTransitionDuration(pTemplatePage->getTransitionDuration());
    }
}

SfxFrame* SdModule::CreateFromTemplate( const OUString& rTemplatePath, const Reference< XFrame >& i_rFrame )
{
    SfxFrame* pFrame = nullptr;

    SfxObjectShellLock xDocShell;

    SfxItemSet* pSet = new SfxAllItemSet( SfxGetpApp()->GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );

    sal_uLong lErr = SfxGetpApp()->LoadTemplate( xDocShell, rTemplatePath, pSet );

    SfxObjectShell* pDocShell = xDocShell;

    if( lErr )
    {
        ErrorHandler::HandleError(lErr);
    }
    else if( pDocShell )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pDocShell, i_rFrame );
        OSL_ENSURE( pViewFrame, "SdModule::CreateFromTemplate: no view frame - was the document really loaded?" );
        pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    }

    return pFrame;

}

SfxFrame* SdModule::ExecuteNewDocument( SfxRequest& rReq )
{
    SfxFrame* pFrame = nullptr;
    if ( SvtModuleOptions().IsImpress() )
    {
        Reference< XFrame > xTargetFrame;
        const SfxUnoFrameItem* pFrmItem = rReq.GetArg<SfxUnoFrameItem>(SID_FILLFRAME);
        if ( pFrmItem )
            xTargetFrame = pFrmItem->GetFrame();

        SdOptions* pOpt = GetSdOptions(DOCUMENT_TYPE_IMPRESS);
        bool bStartWithTemplate = pOpt->IsStartWithTemplate();

        bool bNewDocDirect = rReq.GetSlot() == SID_NEWSD;

        if( bNewDocDirect )
        {
            //we start without wizard

            //check whether we should load a template document
            const OUString aServiceName( "com.sun.star.presentation.PresentationDocument" );
            OUString aStandardTemplate( SfxObjectFactory::GetStandardTemplate( aServiceName ) );

            if( !aStandardTemplate.isEmpty() )
            {
                //load a template document
                pFrame = CreateFromTemplate( aStandardTemplate, xTargetFrame );
            }
            else
            {
                //create an empty document
                pFrame = CreateEmptyDocument( xTargetFrame );
            }
        }

        if(bStartWithTemplate)
        {
            //Launch TemplateSelectionDialog
            ScopedVclPtrInstance< SfxTemplateSelectionDlg > aTemplDlg( SfxGetpApp()->GetTopWindow());
            aTemplDlg->Execute();

            //check to disable the dialog
            pOpt->SetStartWithTemplate( aTemplDlg->IsStartWithTemplate() );

            //pFrame is loaded with the desired template
            if(!aTemplDlg->getTemplatePath().isEmpty())
                pFrame = CreateFromTemplate(aTemplDlg->getTemplatePath(), xTargetFrame);
        }
    }

    return pFrame;
}

SfxFrame* SdModule::CreateEmptyDocument( const Reference< XFrame >& i_rFrame )
{
    SfxFrame* pFrame = nullptr;

    SfxObjectShellLock xDocShell;
    ::sd::DrawDocShell* pNewDocSh;
    xDocShell = pNewDocSh = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD,false,DOCUMENT_TYPE_IMPRESS);
    pNewDocSh->DoInitNew();
    SdDrawDocument* pDoc = pNewDocSh->GetDoc();
    if (pDoc)
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
    }

    SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pNewDocSh, i_rFrame );
    OSL_ENSURE( pViewFrame, "SdModule::CreateEmptyDocument: no view frame - was the document really loaded?" );
    pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;

    return pFrame;
}

void SdModule::ChangeMedium( ::sd::DrawDocShell* pDocShell, SfxViewFrame* pViewFrame, const sal_Int32 eMedium )
{
    if( !pDocShell )
        return;

    SdDrawDocument* pDoc = pDocShell->GetDoc();
    if( !pDoc )
        return;

    // settings for the Outputmedium
    Size aNewSize;
    sal_uInt32 nLeft = 0;
    sal_uInt32 nRight = 0;
    sal_uInt32 nLower = 0;
    sal_uInt32 nUpper = 0;
    switch(eMedium)
    {
        case OUTPUT_PAGE:
        case OUTPUT_OVERHEAD:
        {
            SfxPrinter* pPrinter = pDocShell->GetPrinter(true);

            if( pPrinter && pPrinter->IsValid())
            {
                // Unfortunately, the printer does not provide an exact format
                // like A4
                Size aSize(pPrinter->GetPaperSize());
                Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, MAP_100TH_MM, true);

                if (ePaper != PAPER_USER)
                {
                    // get correct size
                    aSize = SvxPaperInfo::GetPaperSize(ePaper, MAP_100TH_MM);
                }

                if (aSize.Height() > aSize.Width())
                {
                     // always landscape
                     aNewSize.Width()  = aSize.Height();
                     aNewSize.Height() = aSize.Width();
                }
                else
                {
                     aNewSize = aSize;
                }
            }
            else
            {
                aNewSize=Size(29700, 21000);
            }

            if (eMedium == OUTPUT_PAGE)
            {
                nLeft =1000;
                nRight=1000;
                nUpper=1000;
                nLower=1000;
            }
            else
            {
                nLeft =0;
                nRight=0;
                nUpper=0;
                nLower=0;
            }
        }
        break;

        case OUTPUT_SLIDE:
        {
            aNewSize = Size(27000, 18000);
            nLeft =0;
            nRight=0;
            nUpper=0;
            nLower=0;
        }
        break;

        case OUTPUT_WIDESCREEN:
        {
            aNewSize = Size(28000, 15750);
            nLeft =0;
            nRight=0;
            nUpper=0;
            nLower=0;
        }
        break;

        case OUTPUT_PRESENTATION:
        {
            aNewSize = Size(28000, 21000);
            nLeft =0;
            nRight=0;
            nUpper=0;
            nLower=0;
        }
        break;
    }

    bool bScaleAll = true;
    sal_uInt16 nPageCnt = pDoc->GetMasterSdPageCount(PK_STANDARD);
    sal_uInt16 i;
    SdPage* pPage;

    // master pages first
    for (i = 0; i < nPageCnt; i++)
    {
        pPage = pDoc->GetMasterSdPage(i, PK_STANDARD);

        if (pPage)
        {
            if(eMedium != OUTPUT_ORIGINAL)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(aNewSize, aBorderRect, bScaleAll);
                pPage->SetSize(aNewSize);
                pPage->SetBorder(nLeft, nUpper, nRight, nLower);
            }
            SdPage* pNotesPage = pDoc->GetMasterSdPage(i, PK_NOTES);
            DBG_ASSERT( pNotesPage, "Wrong page ordering!" );
            if( pNotesPage ) pNotesPage->CreateTitleAndLayout();
            pPage->CreateTitleAndLayout();
        }
    }

    nPageCnt = pDoc->GetSdPageCount(PK_STANDARD);

    // then slides
    for (i = 0; i < nPageCnt; i++)
    {
        pPage = pDoc->GetSdPage(i, PK_STANDARD);

        if (pPage)
        {
            if(eMedium != OUTPUT_ORIGINAL)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pPage->ScaleObjects(aNewSize, aBorderRect, bScaleAll);
                pPage->SetSize(aNewSize);
                pPage->SetBorder(nLeft, nUpper, nRight, nLower);
            }
            SdPage* pNotesPage = pDoc->GetSdPage(i, PK_NOTES);
            DBG_ASSERT( pNotesPage, "Wrong page ordering!" );
            if( pNotesPage ) pNotesPage->SetAutoLayout( pNotesPage->GetAutoLayout() );
            pPage->SetAutoLayout( pPage->GetAutoLayout() );
        }
    }

    SdPage* pHandoutPage = pDoc->GetSdPage(0, PK_HANDOUT);
    pHandoutPage->CreateTitleAndLayout(true);

    if( (eMedium != OUTPUT_ORIGINAL) && pViewFrame && pViewFrame->GetDispatcher())
    {
        pViewFrame->GetDispatcher()->Execute(SID_SIZE_PAGE, SfxCallMode::SYNCHRON | SfxCallMode::RECORD);
    }
}

//===== OutlineToImpressFinalize ==============================================

namespace {

OutlineToImpressFinalizer::OutlineToImpressFinalizer (
    ::sd::ViewShellBase& rBase,
    SdDrawDocument& rDocument,
    SvLockBytes& rBytes)
    : mrBase(rBase),
      mrDocument(rDocument),
      mpStream()
{
    // The given stream has a lifetime shorter than this new
    // OutlineToImpressFinalizer object.  Therefore a local copy of the
    // stream is created.
    const SvStream* pStream (rBytes.GetStream());
    if (pStream != nullptr)
    {
        // Create a memory stream and prepare to fill it with the content of
        // the original stream.
        mpStream.reset(new SvMemoryStream());
        static const sal_Size nBufferSize = 4096;
        ::std::unique_ptr<sal_Int8[]> pBuffer (new sal_Int8[nBufferSize]);

        sal_uInt64 nReadPosition(0);
        bool bLoop (true);
        while (bLoop)
        {
            // Read the next part of the original stream.
            sal_Size nReadByteCount (0);
            const ErrCode nErrorCode (
                rBytes.ReadAt(
                    nReadPosition,
                    pBuffer.get(),
                    nBufferSize,
                    &nReadByteCount));

            // Check the error code and stop copying the stream data when an
            // error has occurred.
            switch (nErrorCode)
            {
                case ERRCODE_NONE:
                    if (nReadByteCount == 0)
                        bLoop = false;
                    break;
                case ERRCODE_IO_PENDING:
                    break;

                default:
                    bLoop = false;
                    nReadByteCount = 0;
                    break;
            }

            // Append the read bytes to the end of the memory stream.
            if (nReadByteCount > 0)
            {
                mpStream->WriteBytes(pBuffer.get(), nReadByteCount);
                nReadPosition += nReadByteCount;
            }
        }

        // Rewind the memory stream so that in the operator() method its
        // content is properly read.
        mpStream->Seek(STREAM_SEEK_TO_BEGIN);
    }
}

void OutlineToImpressFinalizer::operator() (bool)
{
    // Fetch the new outline view shell.
    ::sd::OutlineViewShell* pOutlineShell
        = dynamic_cast<sd::OutlineViewShell*>(FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());

    if (pOutlineShell != nullptr && mpStream.get() != nullptr)
    {
        sd::OutlineView* pView = static_cast<sd::OutlineView*>(pOutlineShell->GetView());
        // mba: the stream can't contain any relative URLs, because we don't
        // have any information about a BaseURL!
        if ( pOutlineShell->Read(*mpStream, OUString(), EE_FORMAT_RTF) == 0 )
        {
        }

        // Call UpdatePreview once for every slide to resync the
        // document with the outliner of the OutlineViewShell.
        sal_uInt16 nPageCount (mrDocument.GetSdPageCount(PK_STANDARD));
        for (sal_uInt16 nIndex=0; nIndex<nPageCount; nIndex++)
        {
            SdPage* pPage = mrDocument.GetSdPage(nIndex, PK_STANDARD);
            // Make the page the actual page so that the
            // following UpdatePreview() call accesses the
            // correct paragraphs.
            pView->SetActualPage(pPage);
            pOutlineShell->UpdatePreview(pPage, true);
        }
        // Select the first slide.
        SdPage* pPage = mrDocument.GetSdPage(0, PK_STANDARD);
        pView->SetActualPage(pPage);
        pOutlineShell->UpdatePreview(pPage, true);
    }

    // Undo-Stack needs to be cleared, else the user may remove the
    // only drawpage and this is a state we cannot handle ATM.
    ::sd::DrawDocShell* pDocShell = mrDocument.GetDocSh();
    if( pDocShell )
        pDocShell->ClearUndoBuffer();
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
