/*************************************************************************
 *
 *  $RCSfile: sdmod1.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:34:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#include <svx/dialogs.hrc>
#include <offmgr/ofaids.hrc>

#define ITEMID_LANGUAGE     SID_ATTR_LANGUAGE
#include <svx/langitem.hxx>
#include <svx/editdata.hxx>

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
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
#include "viewshel.hxx"
#include "sdattr.hxx"
#include "sdpage.hxx"
#include "docshell.hxx"
#include "drawdoc.hxx"
#include "assclass.hxx"
#include "dlgass.hxx"
#include "sdresid.hxx"
#include "outlnvsh.hxx"
#include "frmview.hxx"


/*************************************************************************
|*
|* Execute
|*
\************************************************************************/

void SdModule::Execute(SfxRequest& rReq)
{
    const SfxItemSet* pSet = rReq.GetArgs();
    ULONG nSlotId = rReq.GetSlot();

    // #94442# keep track of created frames
    SfxFrame* pFrame = NULL;

    switch ( nSlotId )
    {
        case SID_NEWDOC:
        {
            OFF_APP()->ExecuteSlot(rReq, OFF_APP()->GetInterface());
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
                SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
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
                SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
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
                            SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
                            if(pDocSh)
                            {
                                DocumentType eDocType = pDocSh->GetDoc()->GetDocumentType();

                                PutItem( *pItem );
                                SdOptions* pOptions = GetSdOptions( eDocType );
                                if(pOptions)
                                    pOptions->SetMetric( eUnit );
                                rReq.Done();
                            }
                        }
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
                SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
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
            if ( SvtModuleOptions().IsImpress() )
            {
                SdOptions* pOpt = GetSdOptions(DOCUMENT_TYPE_IMPRESS);
                BOOL bStartWithTemplate = pOpt->IsStartWithTemplate();

                BOOL bNewDocDirect = rReq.GetSlot() == SID_NEWSD;
                if( bNewDocDirect && !bStartWithTemplate )
                {
                    SfxObjectShellLock xDocShell;
                    SdDrawDocShell* pNewDocSh;
                    xDocShell = pNewDocSh = new SdDrawDocShell(SFX_CREATE_MODE_STANDARD, FALSE);
                    if(pNewDocSh)
                    {
                        pNewDocSh->DoInitNew(NULL);
                        SdDrawDocument* pDoc = pNewDocSh->GetDoc();
                        if(pDoc)
                        {
                            pDoc->CreateFirstPages();
                            pDoc->StopWorkStartupDelay();
                        }

                        SFX_REQUEST_ARG( rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                        if ( pFrmItem )
                        {
                            pFrame = pFrmItem->GetFrame();
                            pFrame->InsertDocument( pNewDocSh );
                        }
                        else
                            SFX_APP()->CreateViewFrame( *pNewDocSh );
                    }
                    break;
                }

                String aFileToOpen;
                AssistentDlg* pPilotDlg=new AssistentDlg( DIALOG_NO_PARENT, !bNewDocDirect );

                // Open the Pilot
                if( pPilotDlg->Execute()==RET_CANCEL )
                {
                    delete pPilotDlg;
                    break;
                }
                else
                {
                    const String aPasswrd( pPilotDlg->GetPassword() );
                    const sal_Bool bSummary = pPilotDlg->IsSummary();
                    const sal_Int32 eMedium = pPilotDlg->GetOutputMedium();
                    const String aDocPath( pPilotDlg->GetDocPath());
                    const sal_Bool bIsDocEmpty = pPilotDlg->IsDocEmpty();

                    // So that you can open the document without AutoLayout-Dialog
                    pOpt->SetStartWithTemplate(FALSE);
                    if(bNewDocDirect && !pPilotDlg->GetStartWithFlag())
                        bStartWithTemplate = FALSE;

                    if( pPilotDlg->GetStartType() == ST_OPEN )
                    {
                        String aFileToOpen = aDocPath;
                        delete pPilotDlg;

                        DBG_ASSERT( aFileToOpen.Len()!=0, "The autopilot should have asked for a file itself already!" );
                        if(aFileToOpen.Len() != 0)
                        {
                            SfxStringItem aFile( SID_FILE_NAME, aFileToOpen );
                            SfxStringItem aReferer( SID_REFERER, UniString() );
                            SfxStringItem aPassword( SID_PASSWORD, aPasswrd );

                            SFX_REQUEST_ARG( rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                            if ( pFrmItem && pFrmItem->GetFrame())
                            {
                                pFrame = pFrmItem->GetFrame();

                                SfxAllItemSet aSet( *pSet->GetPool() );
                                aSet.Put( aFile );
                                aSet.Put( aReferer );
                                // Put the password into the request
                                // only if it is not empty.
                                if (aPasswrd.Len() > 0)
                                    aSet.Put( aPassword );

                                const SfxPoolItem* pRet = pFrame->LoadDocumentSynchron( aSet );
                            }
                            else
                            {
                                SfxRequest aRequest (SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                                    SFX_APP()->GetPool());
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
                                    SFX_APP()->ExecuteSlot (aRequest);
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
                            SfxItemSet* pRet = CreateItemSet( SID_SD_EDITOPTIONS );
                            if(pRet)
                            {
                                ApplyItemSet( SID_SD_EDITOPTIONS, *pRet );
                                delete pRet;
                            }

                        }
                        break;
                    }


                    SfxObjectShellLock xShell( pPilotDlg->GetDocument() );
                    SfxObjectShell* pShell = xShell;

                    delete pPilotDlg;

                    SfxViewFrame* pViewFrame = NULL;
                    SFX_REQUEST_ARG( rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                    if ( pFrmItem && pShell )
                    {
                        pFrame = pFrmItem->GetFrame();
                        pFrame->InsertDocument( pShell );
                        pViewFrame = pFrame->GetCurrentViewFrame();
                    }
                    else if( pShell )
                        pViewFrame = SFX_APP()->CreateViewFrame( *pShell );

                    DBG_ASSERT( pViewFrame, "Kein ViewFrame!!" );

                    pOpt->SetStartWithTemplate(bStartWithTemplate);
                    if(bNewDocDirect && !bStartWithTemplate)
                    {
                        SfxItemSet* pRet = CreateItemSet( SID_SD_EDITOPTIONS );
                        if(pRet)
                        {
                            ApplyItemSet( SID_SD_EDITOPTIONS, *pRet );
                            delete pRet;
                        }
                    }

                    if( pShell && pViewFrame )
                    {
                        SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell,pShell);
                        SdDrawDocument* pDoc = pDocShell->GetDoc();

                        SdViewShell* pViewSh = (SdViewShell*) pViewFrame->GetViewShell();
                        SdOptions* pOptions = GetSdOptions(pDoc->GetDocumentType());

                        if (pOptions && pViewSh)
                        {
                            // The AutoPilot-document shall be open without its own options
                            FrameView* pFrameView = pViewSh->GetFrameView();
                            pFrameView->Update(pOptions);
                            pViewSh->ReadFrameViewData(pFrameView);
                        }

                        USHORT nPages=pDoc->GetPageCount();

                        // settings for the Outputmedium
                        Size aNewSize;
                        UINT32 nLeft;
                        UINT32 nRight;
                        UINT32 nLower;
                        UINT32 nUpper;
                        switch(eMedium)
                        {
                            case OUTPUT_PAGE:
                            case OUTPUT_OVERHEAD:
                            {
                                SfxPrinter* pPrinter = pDocShell->GetPrinter(TRUE);

                                if (pPrinter->IsValid())
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

                        for (i = 0; i < nPageCnt; i++)
                        {
                            // ********************************************************************
                            // Erst alle MasterPages bearbeiten
                            // ********************************************************************
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
                            // *********************************************************************
                            // Danach alle Pages bearbeiten
                            // *********************************************************************
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

                        if(eMedium != OUTPUT_ORIGINAL)
                        {
                            pViewFrame->GetDispatcher()->Execute(SID_SIZE_PAGE,
                            SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
                        }

                        if(bSummary)
                            AddSummaryPage (pViewFrame, pDoc);

                        if(aDocPath.Len() == 0) // leeres Document?
                        {
                            SfxBoolItem aIsChangedItem(SID_MODIFYPAGE, !bIsDocEmpty);
                            SfxUInt32Item eAutoLayout( ID_VAL_WHATLAYOUT, (UINT32) AUTOLAYOUT_TITLE );
                            pViewFrame->GetDispatcher()->Execute(SID_MODIFYPAGE,
                               SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aIsChangedItem, &eAutoLayout, 0L);
                        }

                        pDoc->SetChanged(!bIsDocEmpty);

                        // clear document info
                        SfxDocumentInfo& rInfo = pDocShell->GetDocInfo();

                        SfxStamp aCreated;
                        SvtUserOptions aOptions;
                        aCreated.SetName( aOptions.GetFullName() );
                        rInfo.SetCreated( aCreated );

                        SfxStamp aInvalid( TIMESTAMP_INVALID_DATETIME );
                        rInfo.SetChanged( aInvalid );
                        rInfo.SetPrinted( aInvalid );
                        rInfo.SetTime( 0L );
                        rInfo.SetDocumentNumber( 1 );
                        rInfo.SetUseUserData( TRUE );

                        // #94652# clear UNDO stack after autopilot
                        SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
                        DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");
                        if(pUndoManager->GetUndoActionCount())
                            pUndoManager->Clear();

                    }
                }
            }
        }
        break;

        case SID_OPENDOC:
        {
            BOOL bIntercept = FALSE;
            SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
            if (pDocShell)
            {
                SdViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if (pViewShell->GetSlideShow())
                    {
                        const SfxItemSet* pSet = rReq.GetArgs();
                        if (pSet == NULL)
                        {
                            // e.g. open button during a presentation.
                            bIntercept = TRUE;
                        }
                        /* Since #110295# the following block is not
                           necessary anymore: Star/OpenOffice
                           documents can be opened during a show.

                        else
                        {
                            String aBookmark(((SfxStringItem&)pSet->Get(SID_FILE_NAME)).GetValue());

                            // interner Sprung?
                            String aDocName(aBookmark.GetToken(0, '#'));
                            if (aBookmark.Search( '#' ) != STRING_NOTFOUND &&
                                aDocName.Len() > 0 &&
                                pDocShell->GetMedium()->GetName() != aDocName &&
                                pDocShell->GetName() != aDocName              &&
                                aDocName.Search( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( ".wav" ) ) ) == STRING_NOTFOUND )
                            {
                                bIntercept = TRUE;

                                // nur ein Frame in meinem Doc?
                                SFX_REQUEST_ARG(rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                                if (pFrmItem)
                                {
                                    SfxFrame* pFrame = pFrmItem->GetFrame();
                                    SFX_REQUEST_ARG(rReq, pTargetItem, SfxStringItem, SID_TARGETNAME, FALSE);
                                    if (pTargetItem)
                                    {
                                        pFrame = pFrame->SearchFrame(pTargetItem->GetValue());
                                    }

                                    SfxFrame* pMyFrame = pViewShell->GetViewFrame()->GetFrame();
                                    SfxFrame* pParent = pFrame->GetParentFrame();

                                    while (pParent)
                                    {
                                        if (pParent == pMyFrame)
                                        {
                                            bIntercept = FALSE;
                                            break;
                                        }
                                        pParent = pParent->GetParentFrame();
                                    }
                                }
                            }
                        }
                        */
                    }
                }
            }

            if (!bIntercept)
            {
                OFF_APP()->ExecuteSlot(rReq, OFF_APP()->GetInterface());
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
        {
            const SfxItemSet* pSet = rReq.GetArgs();

            if (pSet)
            {
                SvLockBytes* pBytes = ((SfxLockBytesItem&) pSet->Get(SID_OUTLINE_TO_IMPRESS)).GetValue();

                if (pBytes)
                {
                    SfxObjectShellLock xDocShell;
                    SdDrawDocShell* pDocSh;
                    xDocShell = pDocSh = new SdDrawDocShell(SFX_CREATE_MODE_STANDARD, FALSE);
                    if(pDocSh)
                    {
                        pDocSh->DoInitNew(NULL);
                        SdDrawDocument* pDoc = pDocSh->GetDoc();
                        if(pDoc)
                        {
                            pDoc->CreateFirstPages();
                            pDoc->StopWorkStartupDelay();
                        }

                        SFX_REQUEST_ARG( rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
                        if ( pFrmItem )
                        {
                            SfxFrame* pFrame = pFrmItem->GetFrame();
                            pFrame->InsertDocument( pDocSh );
                        }
                        else
                            SFX_APP()->CreateViewFrame( *pDocSh );

                        SdViewShell* pViewSh = pDocSh->GetViewShell();

                        if (pViewSh)
                        {
                            // AutoLayouts muessen fertig sein
                            SdDrawDocument* pDoc = pDocSh->GetDoc();
                            pDoc->StopWorkStartupDelay();

                            // hide preview
                            SfxViewFrame* pViewFrame = pViewSh->GetViewFrame();
                            SfxBoolItem aItem( SID_PREVIEW_WIN, FALSE );
                            pViewFrame->GetDispatcher()->Execute( SID_PREVIEW_WIN, SFX_CALLMODE_SYNCHRON |
                                                    SFX_CALLMODE_RECORD, &aItem, 0L );

                            // In den Gliederungsmodus wechseln
                            pViewFrame->GetDispatcher()->Execute(
                            SID_VIEWSHELL2, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);

                            // OutlineViewShell holen
                            SdOutlineViewShell* pViewSh = PTR_CAST(SdOutlineViewShell, pViewFrame->GetViewShell());

                            if (pViewSh)
                            {
                                SvStream* pStream = (SvStream*) pBytes->GetStream();
                                if ( pViewSh->Read(*pStream, EE_FORMAT_RTF) == 0 )
                                {
                                    // Remove the first empty pages
                                    USHORT nPageCount = pDoc->GetPageCount();
                                    pDoc->RemovePage( --nPageCount );  // notes page
                                    pDoc->RemovePage( --nPageCount );  // standard page
                                }
                            }
                        }

                        // #97231# Undo-Stack needs to be cleared, else the user may remove the
                        // only drawpage and this is a state we cannot handle ATM.
                        SfxUndoManager* pUndoManager = pDocSh->GetUndoManager();
                        DBG_ASSERT(pUndoManager, "No UNDO MANAGER ?!?");
                        if(pUndoManager->GetUndoActionCount())
                            pUndoManager->Clear();
                    }
                }
            }

            rReq.IsDone();
        }
        break;

        default:
        break;
    }

    // #94442# if a frame was created, set it as return value
    if(pFrame)
    {
        rReq.SetReturnValue(SfxFrameItem(0, pFrame));
    }
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
            SdDrawDocShell* pDocShell = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
            if (pDocShell)
            {
                SdViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell)
                {
                    if (pViewShell->GetSlideShow())
                    {
                        rItemSet.DisableItem(SID_SD_AUTOPILOT);
                    }
                }
            }
        }
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_METRIC ) )
    {
        SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
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
        const SfxPoolItem* pItem = OFF_APP()->GetSlotState(SID_OPENDOC, OFF_APP()->GetInterface());
        if (pItem)
            rItemSet.Put(*pItem);
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_AUTOSPELL_CHECK ) ||
        SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_AUTOSPELL_MARKOFF ) )
    {
        SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
        {
            SdDrawDocument* pDoc = pDocSh->GetDoc();
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_CHECK, pDoc->GetOnlineSpell() ) );
            rItemSet.Put( SfxBoolItem( SID_AUTOSPELL_MARKOFF, pDoc->GetHideSpell() ) );
        }
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_LANGUAGE ) )
    {
        SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE ), SID_ATTR_LANGUAGE ) );
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_CHAR_CJK_LANGUAGE ) )
    {
        SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CJK ), SID_ATTR_CHAR_CJK_LANGUAGE ) );
    }

    if( SFX_ITEM_AVAILABLE == rItemSet.GetItemState( SID_ATTR_CHAR_CTL_LANGUAGE ) )
    {
        SdDrawDocShell* pDocSh = PTR_CAST(SdDrawDocShell, SfxObjectShell::Current());
        if( pDocSh )
            rItemSet.Put( SvxLanguageItem( pDocSh->GetDoc()->GetLanguage( EE_CHAR_LANGUAGE_CTL ), SID_ATTR_CHAR_CTL_LANGUAGE ) );
    }
}




void SdModule::AddSummaryPage (SfxViewFrame* pViewFrame, SdDrawDocument* pDocument)
{
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
        SdPage* pSummaryPage = pDocument->GetSdPage (nPageCount-1, PK_STANDARD);
        OSL_ASSERT (pSummaryPage!=NULL);

        // Take the change mode of the template page as indication of the
        // document's kiosk mode.
        pSummaryPage->SetFadeSpeed (pTemplatePage->GetFadeSpeed());
        pSummaryPage->SetFadeEffect (pTemplatePage->GetFadeEffect());
        pSummaryPage->SetPresChange (pTemplatePage->GetPresChange());
        pSummaryPage->SetTime (pTemplatePage->GetTime());
        pSummaryPage->SetSound (pTemplatePage->IsSoundOn());
        pSummaryPage->SetSoundFile (pTemplatePage->GetSoundFile());
    }
}

