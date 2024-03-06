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
#include <svl/intitem.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <unotools/moduleoptions.hxx>
#include <framework/FrameworkHelper.hxx>
#include <osl/diagnose.h>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/errinf.hxx>
#include <editeng/langitem.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include <sfx2/templatedlg.hxx>
#include <svl/stritem.hxx>
#include <editeng/eeitem.hxx>

#include <svx/svxids.hrc>
#include <strings.hrc>

#include <sdmod.hxx>
#include <pres.hxx>
#include <optsitem.hxx>
#include <ViewShell.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <OutlineView.hxx>
#include <OutlineViewShell.hxx>
#include <ViewShellBase.hxx>
#include <FactoryIds.hxx>
#include <memory>
#include <slideshow.hxx>

using ::sd::framework::FrameworkHelper;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

namespace {

class OutlineToImpressFinalizer final
{
public:
    OutlineToImpressFinalizer (
        ::sd::ViewShellBase& rBase,
        SdDrawDocument& rDocument,
        css::uno::Sequence<sal_Int8> const & rBytes);
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
    sal_uInt16 nSlotId = rReq.GetSlot();

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
            const SfxBoolItem* pItem;
            if( pSet && (pItem = pSet->GetItemIfSet( SID_AUTOSPELL_CHECK, false ) ) )
            {
                bool bOnlineSpelling = pItem->GetValue();
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
            const SfxUInt16Item* pItem;
            if ( pSet && (pItem = pSet->GetItemIfSet( SID_ATTR_METRIC ) ) )
            {
                FieldUnit eUnit = static_cast<FieldUnit>(pItem->GetValue());
                switch( eUnit )
                {
                    case FieldUnit::MM:      // only the units which are also in the dialog
                    case FieldUnit::CM:
                    case FieldUnit::INCH:
                    case FieldUnit::PICA:
                    case FieldUnit::POINT:
                        {
                            ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
                            if(pDocSh)
                            {
                                DocumentType eDocType = pDocSh->GetDoc()->GetDocumentType();

                                PutItem( *pItem );
                                SdOptions* pOptions = GetSdOptions( eDocType );
                                if(pOptions)
                                    pOptions->SetMetric( static_cast<sal_uInt16>(eUnit) );
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
            ::sd::ViewShell* pViewShell = pDocShell ? pDocShell->GetViewShell() : nullptr;
            if (pViewShell)
            {
                if( sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() )
                    && !sd::SlideShow::IsInteractiveSlideshow( &pViewShell->GetViewShellBase() ) ) // IASS
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

            if (!bIntercept)
            {
                if (const SfxStringItem* pURLItem = rReq.GetArg<SfxStringItem>(SID_FILE_NAME))
                {
                    if (!pViewShell || !SfxObjectShell::AllowedLinkProtocolFromDocument(pURLItem->GetValue(),
                                                                                        pViewShell->GetObjectShell(),
                                                                                        pViewShell->GetFrameWeld()))
                    {
                        return;
                    }
                }
                SfxGetpApp()->ExecuteSlot(rReq, SfxGetpApp()->GetInterface());
            }
            else
            {
                std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(rReq.GetFrameWeld(),
                                                               VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_CANT_PERFORM_IN_LIVEMODE)));

                xErrorBox->run();

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

bool SdModule::OutlineToImpress(SfxRequest const & rRequest)
{
    const SfxItemSet* pSet = rRequest.GetArgs();

    if (pSet)
    {
        css::uno::Sequence<sal_Int8> pBytes = static_cast<const SfxLockBytesItem&>(pSet->Get(SID_OUTLINE_TO_IMPRESS)).GetValue();

        if (pBytes.getLength())
        {
            rtl::Reference<sd::DrawDocShell> pDocSh = new ::sd::DrawDocShell(
                SfxObjectCreateMode::STANDARD, false, DocumentType::Impress);
            SfxObjectShellLock xDocShell(pDocSh.get());

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
                        OutlineToImpressFinalizer(*pBase, *pDoc, pBytes));
                }
            }
        }
    }

    return rRequest.IsDone();
}

void SdModule::GetState(SfxItemSet& rItemSet)
{
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
        const SfxPoolItemHolder aItem(SfxGetpApp()->GetSlotState(SID_OPENDOC, SfxGetpApp()->GetInterface()));
        if (aItem)
            rItemSet.Put(*aItem.getItem());
    }

    // state of SID_OPENHYPERLINK is determined by the base class
    if (rItemSet.GetItemState(SID_OPENHYPERLINK) != SfxItemState::UNKNOWN)
    {
        const SfxPoolItemHolder aItem(SfxGetpApp()->GetSlotState(SID_OPENHYPERLINK, SfxGetpApp()->GetInterface()));
        if (aItem)
            rItemSet.Put(*aItem.getItem());
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

    if ( mbEventListenerAdded )
        return;

    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
    if( pDocShell ) // Impress or Draw ?
    {
        ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

        if( pViewShell && (pDocShell->GetDocumentType() == DocumentType::Impress) )
        {
            // add our event listener as soon as possible
            Application::AddEventListener( LINK( this, SdModule, EventListenerHdl ) );
            mbEventListenerAdded = true;
        }
    }
}

IMPL_STATIC_LINK( SdModule, EventListenerHdl, VclSimpleEvent&, rSimpleEvent, void )
{
    if( !((rSimpleEvent.GetId() == VclEventId::WindowCommand) && static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData()) )
        return;

    const CommandEvent& rEvent = *static_cast<const CommandEvent*>(static_cast<VclWindowEvent*>(&rSimpleEvent)->GetData());

    if( rEvent.GetCommand() != CommandEventId::Media )
        return;

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
                if( pViewShell && (pDocShell->GetDocumentType() == DocumentType::Impress) )
                    pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION );
            }
            break;
        }
        default:
            pMediaData->SetPassThroughToOS(true);
            break;
    }
}


SfxFrame* SdModule::CreateFromTemplate(const OUString& rTemplatePath, const Reference<XFrame>& i_rFrame,
                                       const bool bReplaceable)
{
    SfxFrame* pFrame = nullptr;

    SfxObjectShellLock xDocShell;

    std::unique_ptr<SfxItemSet> pSet(new SfxAllItemSet( SfxGetpApp()->GetPool() ));
    pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );

    ErrCodeMsg lErr = SfxGetpApp()->LoadTemplate( xDocShell, rTemplatePath, std::move(pSet) );

    SfxObjectShell* pDocShell = xDocShell;

    if( lErr )
    {
        ErrorHandler::HandleError(lErr);
    }
    else if( pDocShell )
    {
        if (pDocShell->GetMedium())
            pDocShell->GetMedium()->GetItemSet().Put(SfxBoolItem(SID_REPLACEABLE, bReplaceable));
        SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pDocShell, i_rFrame );
        OSL_ENSURE( pViewFrame, "SdModule::CreateFromTemplate: no view frame - was the document really loaded?" );
        pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    }

    return pFrame;

}

SfxFrame* SdModule::ExecuteNewDocument( SfxRequest const & rReq )
{
    SfxFrame* pFrame = nullptr;
    if ( SvtModuleOptions().IsImpress() )
    {
        Reference< XFrame > xTargetFrame;
        const SfxUnoFrameItem* pFrmItem = rReq.GetArg<SfxUnoFrameItem>(SID_FILLFRAME);
        if ( pFrmItem )
            xTargetFrame = pFrmItem->GetFrame();

        SdOptions* pOpt = GetSdOptions(DocumentType::Impress);
        bool bStartWithTemplate = pOpt->IsStartWithTemplate();

        bool bNewDocDirect = rReq.GetSlot() == SID_NEWSD;

        if( bNewDocDirect )
        {
            //we start without wizard

            //check whether we should load a template document
            OUString aStandardTemplate( SfxObjectFactory::GetStandardTemplate( u"com.sun.star.presentation.PresentationDocument" ) );

            if( !aStandardTemplate.isEmpty() )
            {
                //load a template document
                pFrame = CreateFromTemplate(aStandardTemplate, xTargetFrame, true);
            }
            else
            {
                //create an empty document
                pFrame = CreateEmptyDocument( xTargetFrame );
            }
        }

        if (bStartWithTemplate)
        {
            //Launch TemplateSelectionDialog
            SfxTemplateSelectionDlg aTemplDlg(SfxGetpApp()->GetTopWindow());
            aTemplDlg.run();

            //check to disable the dialog
            pOpt->SetStartWithTemplate( aTemplDlg.IsStartWithTemplate() );

            //pFrame is loaded with the desired template
            if (!aTemplDlg.getTemplatePath().isEmpty())
                pFrame = CreateFromTemplate(aTemplDlg.getTemplatePath(), xTargetFrame, false);

            // show tip-of-the-day dialog if it was deferred because SfxTemplateSelectionDlg
            // was open
            if (pFrame && SfxApplication::IsTipOfTheDayDue() && !SfxApplication::IsHeadlessOrUITest())
            {
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                {
                    // tdf#127946 pass in argument for dialog parent
                    SfxUnoFrameItem aDocFrame(SID_FILLFRAME, pFrame->GetFrameInterface());
                    pDispatcher->ExecuteList(SID_TIPOFTHEDAY, SfxCallMode::SLOT, {}, { &aDocFrame });
                }
            }
        }
    }

    return pFrame;
}

SfxFrame* SdModule::CreateEmptyDocument( const Reference< XFrame >& i_rFrame )
{
    SfxFrame* pFrame = nullptr;

    rtl::Reference<sd::DrawDocShell> pNewDocSh = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD,false,DocumentType::Impress);
    SfxObjectShellLock xDocShell(pNewDocSh.get());
    pNewDocSh->DoInitNew();
    SdDrawDocument* pDoc = pNewDocSh->GetDoc();
    if (pDoc)
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
    }
    if (pNewDocSh->GetMedium())
        pNewDocSh->GetMedium()->GetItemSet().Put(SfxBoolItem(SID_REPLACEABLE, true));

    SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pNewDocSh, i_rFrame );
    OSL_ENSURE( pViewFrame, "SdModule::CreateEmptyDocument: no view frame - was the document really loaded?" );
    pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;

    return pFrame;
}

//===== OutlineToImpressFinalize ==============================================

namespace {

OutlineToImpressFinalizer::OutlineToImpressFinalizer (
    ::sd::ViewShellBase& rBase,
    SdDrawDocument& rDocument,
    css::uno::Sequence<sal_Int8> const & rBytes)
    : mrBase(rBase),
      mrDocument(rDocument)
{
    // Create a memory stream to fill it with the content of
    // the original stream.
    mpStream = std::make_shared<SvMemoryStream>(static_cast<void*>(const_cast<sal_Int8*>(rBytes.getConstArray())), rBytes.getLength(), StreamMode::READ);

    // Rewind the memory stream so that in the operator() method its
    // content is properly read.
    mpStream->Seek(STREAM_SEEK_TO_BEGIN);
}

void OutlineToImpressFinalizer::operator() (bool)
{
    // Fetch the new outline view shell.
    ::sd::OutlineViewShell* pOutlineShell
        = dynamic_cast<sd::OutlineViewShell*>(FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());

    if (pOutlineShell != nullptr && mpStream != nullptr)
    {
        sd::OutlineView* pView = static_cast<sd::OutlineView*>(pOutlineShell->GetView());
        // mba: the stream can't contain any relative URLs, because we don't
        // have any information about a BaseURL!
        pOutlineShell->ReadRtf(*mpStream);

        // Call UpdatePreview once for every slide to resync the
        // document with the outliner of the OutlineViewShell.
        sal_uInt16 nPageCount (mrDocument.GetSdPageCount(PageKind::Standard));
        for (sal_uInt16 nIndex=0; nIndex<nPageCount; nIndex++)
        {
            SdPage* pPage = mrDocument.GetSdPage(nIndex, PageKind::Standard);
            // Make the page the actual page so that the
            // following UpdatePreview() call accesses the
            // correct paragraphs.
            pView->SetActualPage(pPage);
            pOutlineShell->UpdatePreview(pPage);
        }
        // Select the first slide.
        SdPage* pPage = mrDocument.GetSdPage(0, PageKind::Standard);
        pView->SetActualPage(pPage);
        pOutlineShell->UpdatePreview(pPage);
    }

    // Undo-Stack needs to be cleared, else the user may remove the
    // only drawpage and this is a state we cannot handle ATM.
    ::sd::DrawDocShell* pDocShell = mrDocument.GetDocSh();
    if( pDocShell )
        pDocShell->ClearUndoBuffer();
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
