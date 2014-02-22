/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
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
    virtual ~OutlineToImpressFinalizer (void) {};
    void operator() (bool bEventSeen);
private:
    ::sd::ViewShellBase& mrBase;
    SdDrawDocument& mrDocument;
    ::boost::shared_ptr<SvMemoryStream> mpStream;
};


} 


void SdModule::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    sal_uLong nSlotId = rReq.GetSlot();

    switch ( nSlotId )
    {
        case SID_NEWDOC:
        {
            SFX_APP()->ExecuteSlot(rReq, SFX_APP()->GetInterface());
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            
            const SfxPoolItem* pItem;
            if( pSet && SFX_ITEM_SET == pSet->GetItemState(
                        SID_AUTOSPELL_CHECK, false, &pItem ) )
            {
                sal_Bool bOnlineSpelling = ( (const SfxBoolItem*) pItem )->GetValue();
                
                ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
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
            if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_ATTR_METRIC, true, &pItem ) )
            {
                FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
                switch( eUnit )
                {
                    case FUNIT_MM:      
                    case FUNIT_CM:
                    case FUNIT_INCH:
                    case FUNIT_PICA:
                    case FUNIT_POINT:
                        {
                            ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current() );
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
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_LANGUAGE, false, &pItem ) ||
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, false, &pItem ) ||
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, false, &pItem )
                )
              )
            {
                
                ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
                if ( pDocSh )
                {
                    LanguageType eLanguage = ( (SvxLanguageItem*)pItem )->GetValue();
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
                
                if(pFrame)
                    rReq.SetReturnValue(SfxFrameItem(0, pFrame));
            }

            break;

        case SID_OPENHYPERLINK:
        case SID_OPENDOC:
        {
            sal_Bool bIntercept = sal_False;
            ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
            if (pDocShell)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if( sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ) )
                    {
                        
                        
                        
                        if (rReq.GetArgs() == NULL)
                        {
                            bIntercept = sal_True;
                        }
                    }
                }
            }

            if (!bIntercept)
            {
                SFX_APP()->ExecuteSlot(rReq, SFX_APP()->GetInterface());
            }
            else
            {
                ErrorBox(NULL, WB_OK, SD_RESSTR(STR_CANT_PERFORM_IN_LIVEMODE)).Execute();

                SFX_REQUEST_ARG( rReq, pLinkItem, SfxLinkItem, SID_DONELINK, false );
                if( pLinkItem )
                    pLinkItem->GetValue().Call( 0 );
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


void SdModule::OutlineToImpress (SfxRequest& rRequest)
{
    const SfxItemSet* pSet = rRequest.GetArgs();

    if (pSet)
    {
        SvLockBytes* pBytes = ((SfxLockBytesItem&) pSet->Get(SID_OUTLINE_TO_IMPRESS)).GetValue();

        if (pBytes)
        {
            SfxObjectShellLock xDocShell;
            ::sd::DrawDocShell* pDocSh;
            xDocShell = pDocSh = new ::sd::DrawDocShell(
                SFX_CREATE_MODE_STANDARD, sal_False);
            if(pDocSh)
            {
                pDocSh->DoInitNew(NULL);
                SdDrawDocument* pDoc = pDocSh->GetDoc();
                if(pDoc)
                {
                    pDoc->CreateFirstPages();
                    pDoc->StopWorkStartupDelay();
                }

                SFX_REQUEST_ARG( rRequest, pFrmItem, SfxFrameItem, SID_DOCFRAME, false);
                SfxViewFrame::LoadDocumentIntoFrame( *pDocSh, pFrmItem, ::sd::OUTLINE_FACTORY_ID );

                ::sd::ViewShell* pViewSh = pDocSh->GetViewShell();

                if (pViewSh)
                {
                    
                    pDoc->StopWorkStartupDelay();

                    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();

                    
                    
                    
                    ::sd::ViewShellBase* pBase
                        = dynamic_cast< ::sd::ViewShellBase*>(pViewFrame->GetViewShell());
                    if (pBase != NULL)
                    {
                        ::boost::shared_ptr<FrameworkHelper> pHelper (
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
    }

    rRequest.IsDone();
}


static bool bOnce = false;

void SdModule::GetState(SfxItemSet& rItemSet)
{
    
    if (rItemSet.GetItemState(SID_SD_AUTOPILOT) != SFX_ITEM_UNKNOWN)
    {
        if (!SvtModuleOptions().IsImpress())
        {
            rItemSet.DisableItem(SID_SD_AUTOPILOT);
        }
        else
        {
            ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
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

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_METRIC ) )
    {
        ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current() );
        if(pDocSh)
        {
            DocumentType eDocType = pDocSh->GetDoc()->GetDocumentType();

            SdOptions* pOptions = GetSdOptions(eDocType);
            rItemSet.Put( SfxUInt16Item( SID_ATTR_METRIC, pOptions->GetMetric() ) );
        }
    }

    
    if (rItemSet.GetItemState(SID_OPENDOC) != SFX_ITEM_UNKNOWN)
    {
        const SfxPoolItem* pItem = SFX_APP()->GetSlotState(SID_OPENDOC, SFX_APP()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    
    if (rItemSet.GetItemState(SID_OPENHYPERLINK) != SFX_ITEM_UNKNOWN)
    {
        const SfxPoolItem* pItem = SFX_APP()->GetSlotState(SID_OPENHYPERLINK, SFX_APP()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_AUTOSPELL_CHECK ) )
    {
        ::sd::DrawDocShell* pDocSh =
              PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
        {
            SdDrawDocument* pDoc = pDocSh->GetDoc();
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_CHECK, pDoc->GetOnlineSpell() ) );
        }
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE ), SID_ATTR_LANGUAGE ) );
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CJK ), SID_ATTR_CHAR_CJK_LANGUAGE ) );
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE ) )
    {
        ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CTL ), SID_ATTR_CHAR_CTL_LANGUAGE ) );
    }

    if ( !bOnce )
    {
        ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocShell ) 
        {
            ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

            if( pViewShell && (pDocShell->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) )
            {
                
                Application::AddEventListener( LINK( this, SdModule, EventListenerHdl ) );
                bOnce = true;
            }
        }
    }
}

IMPL_LINK( SdModule, EventListenerHdl, VclSimpleEvent*, pEvent )
{
    if( pEvent && (pEvent->GetId() == VCLEVENT_WINDOW_COMMAND) && static_cast<VclWindowEvent*>(pEvent)->GetData() )
    {
        const CommandEvent& rEvent = *(const CommandEvent*)static_cast<VclWindowEvent*>(pEvent)->GetData();

        if( rEvent.GetCommand() == COMMAND_MEDIA )
        {
            switch( rEvent.GetMediaCommand() )
            {
                case MEDIA_COMMAND_PLAY:
                {
                    ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
                    if( pDocShell )  
                    {
                        ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

                        
                        if( pViewShell && (pDocShell->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) )
                            pViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_PRESENTATION );
                    }
                }
                    break;

                default:
                    break;
            }
        }
    }
    return 0;
}


void SdModule::AddSummaryPage (SfxViewFrame* pViewFrame, SdDrawDocument* pDocument)
{
    if( !pViewFrame || !pViewFrame->GetDispatcher() || !pDocument )
        return;

    pViewFrame->GetDispatcher()->Execute(SID_SUMMARY_PAGE,
        SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

    OSL_ASSERT (pDocument!=NULL);

    sal_Int32 nPageCount = pDocument->GetSdPageCount (PK_STANDARD);

    
    
    if (nPageCount >= 2)
    {
        
        SdPage* pTemplatePage = pDocument->GetSdPage (0, PK_STANDARD);
        OSL_ASSERT (pTemplatePage!=NULL);

        
        SdPage* pSummaryPage = pDocument->GetSdPage (
            (sal_uInt16)nPageCount-1, PK_STANDARD);
        OSL_ASSERT (pSummaryPage!=NULL);

        
        
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
    SfxFrame* pFrame = 0;

    SfxObjectShellLock xDocShell;

    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, true ) );

    sal_uLong lErr = SFX_APP()->LoadTemplate( xDocShell, rTemplatePath, sal_True, pSet );

    SfxObjectShell* pDocShell = xDocShell;

    if( lErr )
    {
        ErrorHandler::HandleError(lErr);
    }
    else if( pDocShell )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pDocShell, i_rFrame );
        OSL_ENSURE( pViewFrame, "SdModule::CreateFromTemplate: no view frame - was the document really loaded?" );
        pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;
    }

    return pFrame;

}

SfxFrame* SdModule::ExecuteNewDocument( SfxRequest& rReq )
{
    SfxFrame* pFrame = 0;
    if ( SvtModuleOptions().IsImpress() )
    {
        Reference< XFrame > xTargetFrame;
        SFX_REQUEST_ARG( rReq, pFrmItem, SfxUnoFrameItem, SID_FILLFRAME, false);
        if ( pFrmItem )
            xTargetFrame = pFrmItem->GetFrame();

        SfxViewFrame* pViewFrame = NULL;

        SdOptions* pOpt = GetSdOptions(DOCUMENT_TYPE_IMPRESS);
        bool bStartWithTemplate = pOpt->IsStartWithTemplate();

        bool bNewDocDirect = rReq.GetSlot() == SID_NEWSD;
        if( bNewDocDirect && !bStartWithTemplate )
        {
            

            
            const OUString aServiceName( "com.sun.star.presentation.PresentationDocument" );
            OUString aStandardTemplate( SfxObjectFactory::GetStandardTemplate( aServiceName ) );

            if( !aStandardTemplate.isEmpty() )
            {
                
                pFrame = CreateFromTemplate( aStandardTemplate, xTargetFrame );
            }
            else
            {
                
                pFrame = CreateEmptyDocument( DOCUMENT_TYPE_IMPRESS, xTargetFrame );
            }
        }
        else
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            boost::scoped_ptr< AbstractAssistentDlg > pPilotDlg( pFact ? pFact->CreateAssistentDlg( NULL, !bNewDocDirect ) : 0 );

            
            if( pPilotDlg.get() && pPilotDlg->Execute()==RET_OK )
            {
                const OUString aDocPath( pPilotDlg->GetDocPath());
                const sal_Bool bIsDocEmpty = pPilotDlg->IsDocEmpty();

                
                pOpt->SetStartWithTemplate(sal_False);
                if(bNewDocDirect && !pPilotDlg->GetStartWithFlag())
                    bStartWithTemplate = false;

                if( pPilotDlg->GetStartType() == ST_OPEN )
                {
                    OUString aFileToOpen = aDocPath;

                    DBG_ASSERT( !aFileToOpen.isEmpty(), "The autopilot should have asked for a file itself already!" );
                    if (!aFileToOpen.isEmpty())
                    {
                        com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > aPasswrd( pPilotDlg->GetPassword() );

                        SfxStringItem aFile( SID_FILE_NAME, aFileToOpen );
                        SfxStringItem aReferer( SID_REFERER, OUString());
                        SfxUnoAnyItem aPassword( SID_ENCRYPTIONDATA, com::sun::star::uno::makeAny(aPasswrd) );

                        if ( xTargetFrame.is() )
                        {
                            SfxAllItemSet aSet( *rReq.GetArgs()->GetPool() );
                            aSet.Put( aFile );
                            aSet.Put( aReferer );
                            
                            
                            if (aPasswrd.getLength() > 0)
                                aSet.Put( aPassword );

                            const SfxPoolItem* pRet = SfxFrame::OpenDocumentSynchron( aSet, xTargetFrame );
                            const SfxViewFrameItem* pFrameItem = PTR_CAST( SfxViewFrameItem, pRet );
                            if ( pFrameItem && pFrameItem->GetFrame() )
                                pFrame = &pFrameItem->GetFrame()->GetFrame();
                        }
                        else
                        {
                            SfxRequest aRequest (SID_OPENDOC, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool());
                            aRequest.AppendItem (aFile);
                            aRequest.AppendItem (aReferer);
                            
                            
                            if (aPasswrd.getLength() > 0)
                                aRequest.AppendItem (aPassword);
                            aRequest.AppendItem (SfxStringItem (
                                SID_TARGETNAME,
                                OUString("_default")));
                            try
                            {
                                const SfxPoolItem* pRet = SFX_APP()->ExecuteSlot (aRequest);
                                const SfxViewFrameItem* pFrameItem = PTR_CAST( SfxViewFrameItem, pRet );
                                if ( pFrameItem )
                                    pFrame = &pFrameItem->GetFrame()->GetFrame();
                            }
                            catch (const ::com::sun::star::uno::Exception&)
                            {
                                DBG_ASSERT (false, "caught IllegalArgumentException while loading document from Impress autopilot");
                            }
                        }
                    }

                    pOpt->SetStartWithTemplate(bStartWithTemplate);
                    if(bNewDocDirect && !bStartWithTemplate)
                    {
                        boost::scoped_ptr< SfxItemSet > pRet( CreateItemSet( SID_SD_EDITOPTIONS ) );
                        if(pRet.get())
                            ApplyItemSet( SID_SD_EDITOPTIONS, *pRet.get() );

                    }
                }
                else
                {
                    SfxObjectShellLock xShell( pPilotDlg->GetDocument() );
                    SfxObjectShell* pShell = xShell;
                    if( pShell )
                    {
                        pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pShell, xTargetFrame );
                        DBG_ASSERT( pViewFrame, "no ViewFrame!!" );
                        pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;

                        if(bNewDocDirect && !bStartWithTemplate)
                        {
                            boost::scoped_ptr< SfxItemSet > pRet( CreateItemSet( SID_SD_EDITOPTIONS ) );
                            if(pRet.get())
                                ApplyItemSet( SID_SD_EDITOPTIONS, *pRet.get() );
                        }

                        if( pShell && pViewFrame )
                        {
                            ::sd::DrawDocShell* pDocShell =
                                  PTR_CAST(::sd::DrawDocShell,pShell);
                            SdDrawDocument* pDoc = pDocShell->GetDoc();

                            ::sd::ViewShellBase* pBase =
                                  ::sd::ViewShellBase::GetViewShellBase (
                                      pViewFrame);
                            OSL_ASSERT (pBase!=NULL);
                            ::boost::shared_ptr<sd::ViewShell> pViewSh = pBase->GetMainViewShell();
                            SdOptions* pOptions = GetSdOptions(pDoc->GetDocumentType());

                            if (pOptions && pViewSh.get())
                            {
                                
                                ::sd::FrameView* pFrameView = pViewSh->GetFrameView();
                                pFrameView->Update(pOptions);
                                pViewSh->ReadFrameViewData(pFrameView);
                            }

                            ChangeMedium( pDocShell, pViewFrame, pPilotDlg->GetOutputMedium() );

                            if(pPilotDlg->IsSummary())
                                AddSummaryPage(pViewFrame, pDoc);

                            
                            if (aDocPath.isEmpty() && pViewFrame && pViewFrame->GetDispatcher())
                            {
                                SfxBoolItem aIsChangedItem(SID_MODIFYPAGE, !bIsDocEmpty);
                                SfxUInt32Item eAutoLayout( ID_VAL_WHATLAYOUT, (sal_uInt32) AUTOLAYOUT_TITLE );
                                pViewFrame->GetDispatcher()->Execute(SID_MODIFYPAGE,
                                   SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aIsChangedItem, &eAutoLayout, 0L);
                            }

                            
                            using namespace ::com::sun::star;
                            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
                            uno::Reference<document::XDocumentProperties>
                                xDocProps(xDPS->getDocumentProperties());
                            DBG_ASSERT(xDocProps.is(), "no DocumentProperties");
                            xDocProps->resetUserData(
                                SvtUserOptions().GetFullName() );
                            xDocProps->setTemplateName(xDocProps->getTitle());
                            xDocProps->setTemplateURL(pPilotDlg->GetDocPath());

                            pDoc->SetChanged(!bIsDocEmpty);

                            pDocShell->SetUseUserData(sal_True);

                            
                            pDocShell->ClearUndoBuffer();
                        }
                    }
                    pOpt->SetStartWithTemplate(bStartWithTemplate);
                }
            }
        }
    }

    return pFrame;
}

SfxFrame* SdModule::CreateEmptyDocument( DocumentType eDocType, const Reference< XFrame >& i_rFrame )
{
    SfxFrame* pFrame = 0;

    SfxObjectShellLock xDocShell;
    ::sd::DrawDocShell* pNewDocSh;
    xDocShell = pNewDocSh = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD,sal_False,eDocType);
    if(pNewDocSh)
    {
        pNewDocSh->DoInitNew(NULL);
        SdDrawDocument* pDoc = pNewDocSh->GetDoc();
        if(pDoc)
        {
            pDoc->CreateFirstPages();
            pDoc->StopWorkStartupDelay();
        }

        SfxViewFrame* pViewFrame = SfxViewFrame::LoadDocumentIntoFrame( *pNewDocSh, i_rFrame );
        OSL_ENSURE( pViewFrame, "SdModule::CreateEmptyDocument: no view frame - was the document really loaded?" );
        pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;
    }

    return pFrame;
}

void SdModule::ChangeMedium( ::sd::DrawDocShell* pDocShell, SfxViewFrame* pViewFrame, const sal_Int32 eMedium )
{
    if( !pDocShell )
        return;

    SdDrawDocument* pDoc = pDocShell->GetDoc();
    if( !pDoc )
        return;

    
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
            SfxPrinter* pPrinter = pDocShell->GetPrinter(sal_True);

            if( pPrinter && pPrinter->IsValid())
            {
                
                
                Size aSize(pPrinter->GetPaperSize());
                Paper ePaper = SvxPaperInfo::GetSvxPaper( aSize, MAP_100TH_MM, true);

                if (ePaper != PAPER_USER)
                {
                    
                    aSize = SvxPaperInfo::GetPaperSize(ePaper, MAP_100TH_MM);
                }

                if (aSize.Height() > aSize.Width())
                {
                     
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

    sal_Bool bScaleAll = sal_True;
    sal_uInt16 nPageCnt = pDoc->GetMasterSdPageCount(PK_STANDARD);
    sal_uInt16 i;
    SdPage* pPage;

    
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
    pHandoutPage->CreateTitleAndLayout(sal_True);

    if( (eMedium != OUTPUT_ORIGINAL) && pViewFrame && pViewFrame->GetDispatcher())
    {
        pViewFrame->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
    }
}






namespace {

OutlineToImpressFinalizer::OutlineToImpressFinalizer (
    ::sd::ViewShellBase& rBase,
    SdDrawDocument& rDocument,
    SvLockBytes& rBytes)
    : mrBase(rBase),
      mrDocument(rDocument),
      mpStream()
{
    
    
    
    const SvStream* pStream (rBytes.GetStream());
    if (pStream != NULL)
    {
        
        
        mpStream.reset(new SvMemoryStream());
        static const sal_Size nBufferSize = 4096;
        ::boost::scoped_array<sal_Int8> pBuffer (new sal_Int8[nBufferSize]);

        sal_Size nReadPosition (0);
        bool bLoop (true);
        while (bLoop)
        {
            
            sal_Size nReadByteCount (0);
            const ErrCode nErrorCode (
                rBytes.ReadAt(
                    nReadPosition,
                    reinterpret_cast<void*>(pBuffer.get()),
                    nBufferSize,
                    &nReadByteCount));

            
            
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

            
            if (nReadByteCount > 0)
            {
                mpStream->Write(reinterpret_cast<void*>(pBuffer.get()), nReadByteCount);
                nReadPosition += nReadByteCount;
            }
        }

        
        
        mpStream->Seek(STREAM_SEEK_TO_BEGIN);
    }
}




void OutlineToImpressFinalizer::operator() (bool)
{
    
    ::sd::OutlineViewShell* pOutlineShell
        = dynamic_cast<sd::OutlineViewShell*>(FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());

    if (pOutlineShell != NULL && mpStream.get() != NULL)
    {
        sd::OutlineView* pView = static_cast<sd::OutlineView*>(pOutlineShell->GetView());
        
        
        if ( pOutlineShell->Read(*mpStream, OUString(), EE_FORMAT_RTF) == 0 )
        {
        }

        
        
        sal_uInt16 nPageCount (mrDocument.GetSdPageCount(PK_STANDARD));
        for (sal_uInt16 nIndex=0; nIndex<nPageCount; nIndex++)
        {
            SdPage* pPage = mrDocument.GetSdPage(nIndex, PK_STANDARD);
            
            
            
            pView->SetActualPage(pPage);
            pOutlineShell->UpdatePreview(pPage, true);
        }
        
        SdPage* pPage = mrDocument.GetSdPage(0, PK_STANDARD);
        pView->SetActualPage(pPage);
        pOutlineShell->UpdatePreview(pPage, true);
    }


    
    
    ::sd::DrawDocShell* pDocShell = mrDocument.GetDocSh();
    if( pDocShell )
        pDocShell->ClearUndoBuffer();
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
