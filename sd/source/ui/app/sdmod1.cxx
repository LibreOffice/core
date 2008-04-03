/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdmod1.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:26:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <svtools/lckbitem.hxx>
#ifndef _SFXFRAME_HXX //autogen
#include <sfx2/frame.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef SD_FRAMEWORK_FRAMEWORK_HELPER_HXX
#include "framework/FrameworkHelper.hxx"
#endif

#include <svx/dialogs.hrc>

#include <svx/langitem.hxx>
#include <svx/editdata.hxx>

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX //autogen
#include <svx/paperinf.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

#define _SD_DLL                 // fuer SD_MOD()
#include "sdmod.hxx"
#include "sddll.hxx"
#include "pres.hxx"
#include "optsitem.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "sdattr.hxx"
#include "sdpage.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#include "drawdoc.hxx"
#include "assclass.hxx"
#include "sdenumdef.hxx"
#include "sdresid.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif
#include "TaskPaneViewShell.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#ifndef SD_FACTORY_IDS_HXX
#include "FactoryIds.hxx"
#endif
#include "sdabstdlg.hxx"
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include "slideshow.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>


using ::sd::framework::FrameworkHelper;

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


} //end of anonymous namespace


/*************************************************************************
|*
|* Execute
|*
\************************************************************************/

void SdModule::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    ULONG nSlotId = rReq.GetSlot();

    switch ( nSlotId )
    {
        case SID_NEWDOC:
        {
            SFX_APP()->ExecuteSlot(rReq, SFX_APP()->GetInterface());
        }
        break;

        case SID_AUTOSPELL_CHECK:
        {
            // automatische Rechtschreibpruefung
            const SfxPoolItem* pItem;
            if( pSet && SFX_ITEM_SET == pSet->GetItemState(
                        SID_AUTOSPELL_CHECK, FALSE, &pItem ) )
            {
                BOOL bOnlineSpelling = ( (const SfxBoolItem*) pItem )->GetValue();
                // am Dokument sichern:
                ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
                if( pDocSh )
                {
                    SdDrawDocument* pDoc = pDocSh->GetDoc();
                    pDoc->SetOnlineSpell( bOnlineSpelling );
                }
            }
        }
        break;

        case SID_AUTOSPELL_MARKOFF:
        {
            const SfxPoolItem* pItem;
            if( pSet && SFX_ITEM_SET == pSet->GetItemState(
                        SID_AUTOSPELL_MARKOFF, FALSE, &pItem ) )
            {
                BOOL bHideSpell = ( (const SfxBoolItem*) pItem )->GetValue();
                // am Dokument sichern:
                ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
                if( pDocSh )
                {
                    SdDrawDocument* pDoc = pDocSh->GetDoc();
                    pDoc->SetHideSpell( bHideSpell );
                }
            }
        }
        break;

        case SID_ATTR_METRIC:
        {
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_ATTR_METRIC, TRUE, &pItem ) )
            {
                FieldUnit eUnit = (FieldUnit)((const SfxUInt16Item*)pItem)->GetValue();
                switch( eUnit )
                {
                    case FUNIT_MM:      // nur die Einheiten, die auch im Dialog stehen
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
                                    pOptions->SetMetric( (UINT16)eUnit );
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
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_LANGUAGE, FALSE, &pItem ) ||
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR_CJK_LANGUAGE, FALSE, &pItem ) ||
                SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_CHAR_CTL_LANGUAGE, FALSE, &pItem ) )
            {
                // am Dokument sichern:
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
                // #94442# if a frame was created, set it as return value
                if(pFrame)
                    rReq.SetReturnValue(SfxFrameItem(0, pFrame));
            }

            break;

        case SID_OPENDOC:
        {
            BOOL bIntercept = FALSE;
            ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
            if (pDocShell)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if( sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ) )
                    {
                        bIntercept = TRUE;
                    }
                }
            }

            if (!bIntercept)
            {
                SFX_APP()->ExecuteSlot(rReq, SFX_APP()->GetInterface());
            }
            else
            {
                ErrorBox(NULL, WB_OK, String(SdResId(STR_CANT_PERFORM_IN_LIVEMODE))).Execute();

                SFX_REQUEST_ARG( rReq, pLinkItem, SfxLinkItem, SID_DONELINK, FALSE );
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
                SFX_CREATE_MODE_STANDARD, FALSE);
            if(pDocSh)
            {
                pDocSh->DoInitNew(NULL);
                SdDrawDocument* pDoc = pDocSh->GetDoc();
                if(pDoc)
                {
                    pDoc->CreateFirstPages();
                    pDoc->StopWorkStartupDelay();
                }

                SFX_REQUEST_ARG( rRequest, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                if ( pFrmItem )
                {
                    SfxFrame* pFrame = pFrmItem->GetFrame();
                    pFrame->InsertDocument( pDocSh );
                }
                else
                    SfxViewFrame::CreateViewFrame(*pDocSh, ::sd::OUTLINE_FACTORY_ID);

                ::sd::ViewShell* pViewSh = pDocSh->GetViewShell();

                if (pViewSh)
                {
                    // AutoLayouts muessen fertig sein
                    pDoc->StopWorkStartupDelay();

                    SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();

                    // When the view frame has not been just created we have
                    // to switch synchronously to the outline view.
                    // (Otherwise the request will be ignored anyway.)
                    ::sd::ViewShellBase* pBase
                        = dynamic_cast< ::sd::ViewShellBase*>(pViewFrame->GetViewShell());
                    if (pBase != NULL)
                    {
                        ::boost::shared_ptr<FrameworkHelper> pHelper (
                            FrameworkHelper::Instance(*pBase));
                        pHelper->RequestView(
                            FrameworkHelper::msOutlineViewURL,
                            FrameworkHelper::msCenterPaneURL);
                        pHelper->RunOnConfigurationEvent(
                            ::rtl::OUString::createFromAscii("ConfigurationUpdateEnd"),
                                OutlineToImpressFinalizer(*pBase, *pDoc, *pBytes));
                    }
                }
            }
        }
    }

    rRequest.IsDone();
}




/*************************************************************************
|*
|* GetState
|*
\************************************************************************/

void SdModule::GetState(SfxItemSet& rItemSet)
{
    // Autopilot waehrend der Praesentation disablen
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

    // der Status von SID_OPENDOC wird von der Basisklasse bestimmt
    if (rItemSet.GetItemState(SID_OPENDOC) != SFX_ITEM_UNKNOWN)
    {
        const SfxPoolItem* pItem = SFX_APP()->GetSlotState(SID_OPENDOC, SFX_APP()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_AUTOSPELL_CHECK ) ||
        SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_AUTOSPELL_MARKOFF ) )
    {
        ::sd::DrawDocShell* pDocSh =
              PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
        {
            SdDrawDocument* pDoc = pDocSh->GetDoc();
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_CHECK, pDoc->GetOnlineSpell() ) );
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_MARKOFF, pDoc->GetHideSpell() ) );
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
}




void SdModule::AddSummaryPage (SfxViewFrame* pViewFrame, SdDrawDocument* pDocument)
{
    if( !pViewFrame || !pViewFrame->GetDispatcher() || !pDocument )
        return;

    pViewFrame->GetDispatcher()->Execute(SID_SUMMARY_PAGE,
        SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

    OSL_ASSERT (pDocument!=NULL);

    sal_Int32 nPageCount = pDocument->GetSdPageCount (PK_STANDARD);

    // We need at least two pages: the summary page and one to use as
    // template to take the transition parameters from.
    if (nPageCount >= 2)
    {
        // Get a page from which to retrieve the transition parameters.
        SdPage* pTemplatePage = pDocument->GetSdPage (0, PK_STANDARD);
        OSL_ASSERT (pTemplatePage!=NULL);

        // The summary page, if it exists, is the last page.
        SdPage* pSummaryPage = pDocument->GetSdPage (
            (USHORT)nPageCount-1, PK_STANDARD);
        OSL_ASSERT (pSummaryPage!=NULL);

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

SfxFrame* SdModule::CreateFromTemplate( const String& rTemplatePath, SfxFrame* pTargetFrame )
{
    SfxFrame* pFrame = 0;

    SfxObjectShellLock xDocShell;

    SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );

    ULONG lErr = SFX_APP()->LoadTemplate( xDocShell, rTemplatePath, TRUE, pSet );

    SfxObjectShell* pDocShell = xDocShell;

    if( lErr )
    {
        ErrorHandler::HandleError(lErr);
    }
    else if( pDocShell )
    {
        if ( pTargetFrame )
        {
            pFrame = pTargetFrame;
            pFrame->InsertDocument( pDocShell );
        }
        else
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::CreateViewFrame( *pDocShell );
            if( pViewFrame )
                pFrame = pViewFrame->GetFrame();
        }
    }

    return pFrame;

}

SfxFrame* SdModule::ExecuteNewDocument( SfxRequest& rReq )
{
    SfxFrame* pFrame = 0;
    if ( SvtModuleOptions().IsImpress() )
    {
        SfxFrame* pTargetFrame = 0;
        SFX_REQUEST_ARG( rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
        if ( pFrmItem )
            pTargetFrame = pFrmItem->GetFrame();

        bool bMakeLayoutVisible = false;
        SfxViewFrame* pViewFrame = NULL;

        SdOptions* pOpt = GetSdOptions(DOCUMENT_TYPE_IMPRESS);
        bool bStartWithTemplate = pOpt->IsStartWithTemplate();

        bool bNewDocDirect = rReq.GetSlot() == SID_NEWSD;
        if( bNewDocDirect && !bStartWithTemplate )
        {
            //we start without wizard

            //check wether we should load a template document
            const ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) );
            String aStandardTemplate( SfxObjectFactory::GetStandardTemplate( aServiceName ) );

            if( aStandardTemplate.Len() > 0 )
            {
                //load a template document
                pFrame = CreateFromTemplate( aStandardTemplate, pTargetFrame );
            }
            else
            {
                //create an empty document
                pFrame = CreateEmptyDocument( DOCUMENT_TYPE_IMPRESS, pTargetFrame );
                bMakeLayoutVisible = true;
            }
        }
        else
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            std::auto_ptr< AbstractAssistentDlg > pPilotDlg( pFact ? pFact->CreateAssistentDlg( NULL, !bNewDocDirect ) : 0 );

            // Open the Pilot
            if( pPilotDlg.get() && pPilotDlg->Execute()==RET_OK )
            {
                const String aDocPath( pPilotDlg->GetDocPath());
                const sal_Bool bIsDocEmpty = pPilotDlg->IsDocEmpty();

                // So that you can open the document without AutoLayout-Dialog
                pOpt->SetStartWithTemplate(FALSE);
                if(bNewDocDirect && !pPilotDlg->GetStartWithFlag())
                    bStartWithTemplate = FALSE;

                if( pPilotDlg->GetStartType() == ST_OPEN )
                {
                    String aFileToOpen = aDocPath;

                    DBG_ASSERT( aFileToOpen.Len()!=0, "The autopilot should have asked for a file itself already!" );
                    if(aFileToOpen.Len() != 0)
                    {
                        const String aPasswrd( pPilotDlg->GetPassword() );

                        SfxStringItem aFile( SID_FILE_NAME, aFileToOpen );
                        SfxStringItem aReferer( SID_REFERER, UniString() );
                        SfxStringItem aPassword( SID_PASSWORD, aPasswrd );

                        if ( pTargetFrame )
                        {
                            SfxAllItemSet aSet( *rReq.GetArgs()->GetPool() );
                            aSet.Put( aFile );
                            aSet.Put( aReferer );
                            // Put the password into the request
                            // only if it is not empty.
                            if (aPasswrd.Len() > 0)
                                aSet.Put( aPassword );

                            const SfxPoolItem* pRet = pTargetFrame->LoadDocumentSynchron( aSet );
                            const SfxViewFrameItem* pFrameItem = PTR_CAST( SfxViewFrameItem, pRet );
                            if ( pFrameItem && pFrameItem->GetFrame() )
                                pFrame = pFrameItem->GetFrame()->GetFrame();
                        }
                        else
                        {
                            SfxRequest aRequest (SID_OPENDOC, SFX_CALLMODE_SYNCHRON, SFX_APP()->GetPool());
                            aRequest.AppendItem (aFile);
                            aRequest.AppendItem (aReferer);
                            // Put the password into the request
                            // only if it is not empty.
                            if (aPasswrd.Len() > 0)
                                aRequest.AppendItem (aPassword);
                            aRequest.AppendItem (SfxStringItem (
                                SID_TARGETNAME,
                                String (RTL_CONSTASCII_USTRINGPARAM ("_default"))));
                            try
                            {
                                const SfxPoolItem* pRet = SFX_APP()->ExecuteSlot (aRequest);
                                const SfxViewFrameItem* pFrameItem = PTR_CAST( SfxViewFrameItem, pRet );
                                if ( pFrameItem )
                                    pFrame = pFrameItem->GetFrame()->GetFrame();
                            }
                            catch (::com::sun::star::uno::Exception e)
                            {
                                DBG_ASSERT (FALSE, "caught IllegalArgumentException while loading document from Impress autopilot");
                            }
                        }
                    }

                    pOpt->SetStartWithTemplate(bStartWithTemplate);
                    if(bNewDocDirect && !bStartWithTemplate)
                    {
                        std::auto_ptr< SfxItemSet > pRet( CreateItemSet( SID_SD_EDITOPTIONS ) );
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
                        if ( pTargetFrame )
                        {
                            pFrame = pTargetFrame;
                            pFrame->InsertDocument( pShell );
                            pViewFrame = pFrame->GetCurrentViewFrame();
                        }
                        else
                        {
                            pViewFrame = SfxViewFrame::CreateViewFrame( *pShell );
                            if( pViewFrame )
                                pFrame = pViewFrame->GetFrame();
                        }

                        DBG_ASSERT( pViewFrame, "no ViewFrame!!" );

                        if(bNewDocDirect && !bStartWithTemplate)
                        {
                            std::auto_ptr< SfxItemSet > pRet( CreateItemSet( SID_SD_EDITOPTIONS ) );
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
                                // The AutoPilot-document shall be open without its own options
                                ::sd::FrameView* pFrameView = pViewSh->GetFrameView();
                                pFrameView->Update(pOptions);
                                pViewSh->ReadFrameViewData(pFrameView);
                            }

                            ChangeMedium( pDocShell, pViewFrame, pPilotDlg->GetOutputMedium() );

                            if(pPilotDlg->IsSummary())
                                AddSummaryPage(pViewFrame, pDoc);

                            // empty document
                            if((aDocPath.Len() == 0) && pViewFrame && pViewFrame->GetDispatcher())
                            {
                                SfxBoolItem aIsChangedItem(SID_MODIFYPAGE, !bIsDocEmpty);
                                SfxUInt32Item eAutoLayout( ID_VAL_WHATLAYOUT, (UINT32) AUTOLAYOUT_TITLE );
                                pViewFrame->GetDispatcher()->Execute(SID_MODIFYPAGE,
                                   SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aIsChangedItem, &eAutoLayout, 0L);
                            }

                            pDoc->SetChanged(!bIsDocEmpty);

                            // clear document info
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

                            pDocShell->SetUseUserData(TRUE);

                            // #94652# clear UNDO stack after autopilot
                            pDocShell->ClearUndoBuffer();

                            bMakeLayoutVisible = true;
                        }
                    }
                    pOpt->SetStartWithTemplate(bStartWithTemplate);
                }
            }
        }

        if (bMakeLayoutVisible && pViewFrame!=NULL)
        {
            // Make the layout menu visible in the tool pane.
            ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase(pViewFrame);
            if (pBase != NULL)
            {
                FrameworkHelper::Instance(*pBase)->RequestTaskPanel(
                    FrameworkHelper::msLayoutTaskPanelURL);
            }
        }
    }

    return pFrame;
}

SfxFrame* SdModule::CreateEmptyDocument( DocumentType eDocType, SfxFrame* pTargetFrame )
{
    SfxFrame* pFrame = 0;

    SfxObjectShellLock xDocShell;
    ::sd::DrawDocShell* pNewDocSh;
    xDocShell = pNewDocSh = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD,FALSE,eDocType);
    if(pNewDocSh)
    {
        pNewDocSh->DoInitNew(NULL);
        SdDrawDocument* pDoc = pNewDocSh->GetDoc();
        if(pDoc)
        {
            pDoc->CreateFirstPages();
            pDoc->StopWorkStartupDelay();
        }

        if ( pTargetFrame )
        {
            pFrame = pTargetFrame;
            pFrame->InsertDocument( pNewDocSh );
        }
        else
        {
            SfxViewFrame* pViewFrame = SfxViewFrame::CreateViewFrame( *pNewDocSh );
            if( pViewFrame )
                pFrame = pViewFrame->GetFrame();
        }
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

    // settings for the Outputmedium
    Size aNewSize;
    UINT32 nLeft = 0;
    UINT32 nRight = 0;
    UINT32 nLower = 0;
    UINT32 nUpper = 0;
    switch(eMedium)
    {
        case OUTPUT_PAGE:
        case OUTPUT_OVERHEAD:
        {
            SfxPrinter* pPrinter = pDocShell->GetPrinter(TRUE);

            if( pPrinter && pPrinter->IsValid())
            {
                // Der Printer gibt leider kein exaktes
                // Format (z.B. A4) zurueck
                Size aSize(pPrinter->GetPaperSize());
                SvxPaper ePaper = SvxPaperInfo::GetSvxPaper( aSize, MAP_100TH_MM, TRUE);

                if (ePaper != SVX_PAPER_USER)
                {
                    // Korrekte Size holen
                    aSize = SvxPaperInfo::GetPaperSize(ePaper, MAP_100TH_MM);
                }

                if (aSize.Height() > aSize.Width())
                {
                     // Stets Querformat
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

    BOOL bScaleAll = TRUE;
    USHORT nPageCnt = pDoc->GetMasterSdPageCount(PK_STANDARD);
    USHORT i;
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
    pHandoutPage->CreateTitleAndLayout(TRUE);

    if( (eMedium != OUTPUT_ORIGINAL) && pViewFrame && pViewFrame->GetDispatcher())
    {
        pViewFrame->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
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
    if (pStream != NULL)
    {
        // Create a memory stream and prepare to fill it with the content of
        // the original stream.
        mpStream.reset(new SvMemoryStream());
        static const sal_Size nBufferSize = 4096;
        ::boost::scoped_array<sal_Int8> pBuffer (new sal_Int8[nBufferSize]);

        sal_Size nReadPosition (0);
        bool bLoop (true);
        while (bLoop)
        {
            // Read the next part of the original stream.
            sal_Size nReadByteCount (0);
            const ErrCode nErrorCode (
                rBytes.ReadAt(
                    nReadPosition,
                    reinterpret_cast<void*>(pBuffer.get()),
                    nBufferSize,
                    &nReadByteCount));

            // Check the error code and stop copying the stream data when an
            // error has occured.
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
                mpStream->Write(reinterpret_cast<void*>(pBuffer.get()), nReadByteCount);
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
        = dynamic_cast<sd::OutlineViewShell*>(
            FrameworkHelper::Instance(mrBase)->GetViewShell(FrameworkHelper::msCenterPaneURL).get());

    if (pOutlineShell != NULL && mpStream.get() != NULL)
    {
        sd::OutlineView* pView = static_cast<sd::OutlineView*>(pOutlineShell->GetView());
        // mba: the stream can't contain any relative URLs, because we don't
        // have any information about a BaseURL!
        if ( pOutlineShell->Read(*mpStream, String(), EE_FORMAT_RTF) == 0 )
        {
            sd::OutlineViewPageChangesGuard aGuard( pView );

            // Remove the first empty pages
            USHORT nPageCount = mrDocument.GetPageCount();
            mrDocument.RemovePage( --nPageCount );  // notes page
            mrDocument.RemovePage( --nPageCount );  // standard page
        }

        // Call UpdatePreview once for every slide to resync the
        // document with the outliner of the OutlineViewShell.
        USHORT nPageCount (mrDocument.GetSdPageCount(PK_STANDARD));
        for (USHORT nIndex=0; nIndex<nPageCount; nIndex++)
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


    // #97231# Undo-Stack needs to be cleared, else the user may remove the
    // only drawpage and this is a state we cannot handle ATM.
    ::sd::DrawDocShell* pDocShell = mrDocument.GetDocSh();
    if( pDocShell )
        pDocShell->ClearUndoBuffer();
}


} // end of anonymous namespace
