/*************************************************************************
 *
 *  $RCSfile: drviewsb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:44 $
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

#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif
#define ITEMID_HYPERLINK    SID_HYPERLINK_SETLINK
#ifndef _SVX_DLG_HYPERLINK_HXX
#include <offmgr/hyprlink.hxx>
#endif
#ifndef _SVX_HLNKITEM_HXX
#include <svx/hlnkitem.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX
#include <svx/adritem.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#define ITEMID_FIELD    EE_FEATURE_FIELD
#ifndef _FLDITEM_HXX
#include <svx/flditem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDORECT_HXX //autogen
#include <svx/svdorect.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SB_SBERRORS_HXX //autogen
#include <basic/sberrors.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif


#pragma hdrstop

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"

#include "sdoutl.hxx"
#include "sdwindow.hxx"
#include "app.hxx"
#include "sdattr.hxx"
#include "ins_page.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "drviewsh.hxx"
#include "dlgfield.hxx"
#include "drawview.hxx"


/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void SdDrawViewShell::FuTemp02(SfxRequest& rReq)
{
    USHORT nSId = rReq.GetSlot();
    switch( nSId )
    {
        case SID_INSERTLAYER:
        {
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
            USHORT nLayerCnt = rLayerAdmin.GetLayerCount();
            USHORT nLayer = nLayerCnt - 2 + 1;
            String aLayerName ( SdResId(STR_LAYER) );
            aLayerName += String::CreateFromInt32( (sal_Int32)nLayer );
            BOOL bIsVisible;
            BOOL bIsLocked;
            BOOL bIsPrintable;

            const SfxItemSet* pArgs = rReq.GetArgs();

            if (! pArgs)
            {
                SfxItemSet aNewAttr( pDoc->GetPool(), ATTR_LAYER_START, ATTR_LAYER_END );

                aNewAttr.Put( SdAttrLayerName( aLayerName ) );
                aNewAttr.Put( SdAttrLayerVisible() );
                aNewAttr.Put( SdAttrLayerPrintable() );
                aNewAttr.Put( SdAttrLayerLocked() );
                aNewAttr.Put( SdAttrLayerThisPage() );

                SdInsertLayerDlg* pDlg = new SdInsertLayerDlg( NULL, aNewAttr );

                pDlg->SetHelpId( SID_INSERTLAYER );

                // Ueberpruefung auf schon vorhandene Namen
                BOOL bLoop = TRUE;
                while( bLoop && pDlg->Execute() == RET_OK )
                {
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = ((SdAttrLayerName &) aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();

                    if( rLayerAdmin.GetLayer( aLayerName, FALSE )
                        || aLayerName.Len()==0 )
                    {
                        // Name ist schon vorhanden
                        WarningBox aWarningBox( &GetViewFrame()->GetWindow(),
                                                WinBits( WB_OK ),
                                                String(SdResId( STR_WARN_NAME_DUPLICATE ) ) );
                        aWarningBox.Execute();
                    }
                    else
                        bLoop = FALSE;
                }
                if( bLoop ) // wurde abgebrochen
                {
                    delete pDlg;

                    Cancel();
                    rReq.Ignore ();
                    break;
                }
                else
                {
                    //pDlg->GetAttr( aNewAttr );
                    //aLayerName     = ((SdAttrLayerName &) aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();
                    bIsVisible   = ((SdAttrLayerVisible &) aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                    bIsLocked    = ((SdAttrLayerLocked &) aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue () ;
                    bIsPrintable = ((SdAttrLayerPrintable &) aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue () ;

                    delete pDlg;
                }
            }
            else if (pArgs->Count () != 4)
                 {
                     StarBASIC::FatalError (SbERR_WRONG_ARGS);
                     Cancel();
                     rReq.Ignore ();
                     break;
                 }
                 else
                 {
                     SFX_REQUEST_ARG (rReq, pLayerName, SfxStringItem, ID_VAL_LAYERNAME, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsVisible, SfxBoolItem, ID_VAL_ISVISIBLE, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsLocked, SfxBoolItem, ID_VAL_ISLOCKED, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsPrintable, SfxBoolItem, ID_VAL_ISPRINTABLE, FALSE);

                     aLayerName   = pLayerName->GetValue ();
                     bIsVisible   = pIsVisible->GetValue ();
                     bIsLocked    = pIsLocked->GetValue ();
                     bIsPrintable = pIsPrintable->GetValue ();
                 }

            String aPrevLayer = pDrView->GetActiveLayer();
            String aName;
            SdrLayer* pLayer;
            USHORT nPrevLayer = 0;
            nLayerCnt = rLayerAdmin.GetLayerCount();

            for ( nLayer = 0; nLayer < nLayerCnt; nLayer++ )
            {
                pLayer = rLayerAdmin.GetLayer(nLayer);
                aName = pLayer->GetName();

                if ( aPrevLayer == aName )
                {
//                    nPrevLayer = nLayer;
                    nPrevLayer = Max(nLayer, (USHORT) 4);
                }
            }

            SdrLayer* pNewLayer = pDrView->InsertNewLayer(aLayerName, nPrevLayer + 1);

            pDrView->SetLayerVisible( aLayerName, bIsVisible );
            pDrView->SetLayerLocked( aLayerName, bIsLocked);
            pDrView->SetLayerPrintable(aLayerName, bIsPrintable);

            pDrView->SetActiveLayer(aLayerName);

            ResetActualLayer();

            pDoc->SetChanged(TRUE);

            GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHLAYER,
                    SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MODIFYLAYER:
        {
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
            USHORT nCurPage = aLayerTab.GetCurPageId();
            String aLayerName = aLayerTab.GetPageText(nCurPage);
            SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, FALSE);
            const SfxItemSet* pArgs = rReq.GetArgs();

            BOOL bIsVisible;
            BOOL bIsLocked;
            BOOL bIsPrintable;

             // darf der Layer geloescht werden ?
            BOOL bDelete;

            String aLayoutLayer ( SdResId(STR_LAYER_LAYOUT) );
            String aControlsLayer ( SdResId(STR_LAYER_CONTROLS) );
            String aMeasureLinesLayer ( SdResId(STR_LAYER_MEASURELINES) );
            String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
            String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );

            if( aLayerName == aLayoutLayer       || aLayerName == aControlsLayer ||
                aLayerName == aMeasureLinesLayer ||
                aLayerName == aBackgroundLayer   || aLayerName == aBackgroundObjLayer )
            {
                bDelete = FALSE;
            }
            else
                bDelete = TRUE;

            if (! pArgs)
            {
                SfxItemSet aNewAttr( pDoc->GetPool(), ATTR_LAYER_START, ATTR_LAYER_END );

                aNewAttr.Put( SdAttrLayerName( aLayerName ) );
                aNewAttr.Put( SdAttrLayerVisible( pDrView->IsLayerVisible(aLayerName) ) );
                aNewAttr.Put( SdAttrLayerLocked( pDrView->IsLayerLocked(aLayerName) ) );
                aNewAttr.Put( SdAttrLayerPrintable( pDrView->IsLayerPrintable(aLayerName) ) );
                aNewAttr.Put( SdAttrLayerThisPage() );

                SdInsertLayerDlg* pDlg = new SdInsertLayerDlg( NULL,
                                                aNewAttr, bDelete, String( SdResId( STR_MODIFYLAYER ) ) );

                pDlg->SetHelpId( SID_MODIFYLAYER );

                // Ueberpruefung auf schon vorhandene Namen
                BOOL    bLoop = TRUE;
                USHORT  nRet;
                String  aOldLayerName( aLayerName );
                while( bLoop && ( nRet = pDlg->Execute() == RET_OK ) )
                {
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = ((SdAttrLayerName &) aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();

                    if( (rLayerAdmin.GetLayer( aLayerName, FALSE ) &&
                         aLayerName != aOldLayerName) || aLayerName.Len()==0 )
                    {
                        // Name ist schon vorhanden
                        WarningBox aWarningBox( &GetViewFrame()->GetWindow(),
                                                WinBits( WB_OK ),
                                                String( SdResId( STR_WARN_NAME_DUPLICATE ) ) );
                        aWarningBox.Execute();
                    }
                    else
                        bLoop = FALSE;
                }
                switch (nRet)
                {
                    case RET_OK :
                        //pDlg->GetAttr( aNewAttr );
                        //aLayerName   = ((SdAttrLayerName &) aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ();
                        bIsVisible   = ((const SdAttrLayerVisible &) aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                        bIsLocked    = ((const SdAttrLayerLocked &) aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue ();
                        bIsPrintable = ((const SdAttrLayerLocked &) aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue ();

                        delete pDlg;
                        break;

                    case RET_DELETE :
                        pDrView->DeleteLayer (((SdAttrLayerName &) aNewAttr.Get (ATTR_LAYER_NAME)).GetValue ());
                        aLayerTab.RemovePage(nCurPage);

                    default :
                        delete pDlg;
                        rReq.Ignore ();
                        Cancel ();
                        return;
                }
            }
            else if (pArgs->Count () == 4)
                 {
                     SFX_REQUEST_ARG (rReq, pLayerName, SfxStringItem, ID_VAL_LAYERNAME, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsVisible, SfxBoolItem, ID_VAL_ISVISIBLE, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsLocked, SfxBoolItem, ID_VAL_ISLOCKED, FALSE);
                     SFX_REQUEST_ARG (rReq, pIsPrintable, SfxBoolItem, ID_VAL_ISPRINTABLE, FALSE);

                     aLayerName   = pLayerName->GetValue ();
                     bIsVisible   = pIsVisible->GetValue ();
                     bIsLocked    = pIsLocked->GetValue ();
                     bIsPrintable = pIsPrintable->GetValue ();
                 }
                 else
                 {
                     StarBASIC::FatalError (SbERR_WRONG_ARGS);
                     Cancel ();
                     rReq.Ignore ();
                     break;
                 }

            pLayer->SetName( aLayerName );
            pDrView->SetLayerVisible( aLayerName, bIsVisible );
            pDrView->SetLayerLocked( aLayerName, bIsLocked);
            pDrView->SetLayerPrintable(aLayerName, bIsPrintable);

            pDoc->SetChanged(TRUE);

            aLayerTab.SetPageText(nCurPage, aLayerName);

            TabBarPageBits nBits = 0;

            if (!bIsVisible)
            {
                // Unsichtbare Layer werden anders dargestellt
                nBits = TPB_SPECIAL;
            }

            aLayerTab.SetPageBits(nCurPage, nBits);

            GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHLAYER,
                            SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);

            FmFormShell* pFmShell = (FmFormShell*) aShellTable.Get(RID_FORMLAYER_TOOLBOX);
            if (pFmShell)
                pFmShell->Invalidate();

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMELAYER:
        {
            if ( pDrView->IsTextEdit() )
            {
                pDrView->EndTextEdit();
            }

            aLayerTab.StartEditMode( aLayerTab.GetCurPageId() );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EDIT_HYPERLINK :
        {
            SFX_DISPATCHER().Execute( SID_HYPERLINK_DIALOG );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                SvxHyperlinkItem* pHLItem =
                (SvxHyperlinkItem*) &pReqArgs->Get(ITEMID_HYPERLINK);

                if (pHLItem->GetInsertMode() == HLINK_FIELD)
                {
                    InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                   pHLItem->GetTargetFrame(), NULL);
                }
                else if (pHLItem->GetInsertMode() == HLINK_BUTTON)
                {
                    InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                    pHLItem->GetTargetFrame(), NULL);
                }
                else if (pHLItem->GetInsertMode() == HLINK_DEFAULT)
                {
                    OutlinerView* pOlView = pDrView->GetTextEditOutlinerView();

                    if (pOlView)
                    {
                        InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                       pHLItem->GetTargetFrame(), NULL);
                    }
                    else
                    {
                        InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                        pHLItem->GetTargetFrame(), NULL);
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_FILE:
        {
            USHORT nMul = 1;
            SvxFieldItem* pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxDateField( Date(), SVXDATETYPE_FIX ) );
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem = new SvxFieldItem( SvxDateField() );
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem = new SvxFieldItem(
                        SvxExtTimeField( Time(), SVXTIMETYPE_FIX ) );
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem = new SvxFieldItem( SvxExtTimeField() );
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvxAddressItem aAdrItem( *SFX_INIMANAGER() );
                    pFieldItem = new SvxFieldItem( SvxAuthorField( aAdrItem ) );
                }
                break;

                case SID_INSERT_FLD_PAGE:
                {
                    pFieldItem = new SvxFieldItem( SvxPageField() );
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_FILE:
                {
                    String aName;
                    if( pDocSh->HasName() )
                        aName = pDocSh->GetMedium()->GetName();
                    //else
                    //  aName = pDocSh->GetName();
                    pFieldItem = new SvxFieldItem( SvxExtFileField( aName ) );
                }
                break;
            }

            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pOldFldItem = pOLV->GetFieldAtSelection();

                if( pOldFldItem && ( pOldFldItem->GetField()->ISA( SvxURLField ) ||
                                    pOldFldItem->GetField()->ISA( SvxDateField ) ||
                                    pOldFldItem->GetField()->ISA( SvxTimeField ) ||
                                    pOldFldItem->GetField()->ISA( SvxExtTimeField ) ||
                                    pOldFldItem->GetField()->ISA( SvxExtFileField ) ||
                                    pOldFldItem->GetField()->ISA( SvxAuthorField ) ||
                                    pOldFldItem->GetField()->ISA( SvxPageField ) ) )
                {
                    // Feld selektieren, so dass es beim Insert geloescht wird
                    ESelection aSel = pOLV->GetSelection();
                    if( aSel.nStartPos == aSel.nEndPos )
                        aSel.nEndPos++;
                    pOLV->SetSelection( aSel );
                }

                pOLV->InsertField( *pFieldItem );
            }
            else
            {
                Outliner* pOutl = pDoc->GetInternalOutliner();
                pOutl->Init( OUTLINERMODE_TEXTOBJECT );
                USHORT nOutlMode = pOutl->GetMode();
                USHORT nMinDepth = pOutl->GetMinDepth();
                pOutl->SetMinDepth( 0 );
                pOutl->SetStyleSheet( 0, NULL );
                pOutl->QuickInsertField( *pFieldItem, ESelection() );
                OutlinerParaObject* pOutlParaObject = pOutl->CreateParaObject();

                SdrRectObj* pRectObj = new SdrRectObj( OBJ_TEXT );
                SfxItemSet aAttr( pDoc->GetPool() );
                SdrTextAutoGrowWidthItem aAutoGrowWidthOn(TRUE);
                aAttr.Put( SdrTextAutoGrowWidthItem(TRUE) );
                pRectObj->NbcSetAttributes(aAttr, FALSE);

                pOutl->UpdateFields();
                pOutl->SetUpdateMode( TRUE );
                Size aSize( pOutl->CalcTextSize() );
                aSize.Width() *= nMul;
                pOutl->SetUpdateMode( FALSE );

                Point aPos;
                Rectangle aRect( aPos, pWindow->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = pWindow->PixelToLogic(aPos);
                aPos.X() -= aSize.Width() / 2;
                aPos.Y() -= aSize.Height() / 2;

                Rectangle aLogicRect(aPos, aSize);
                pRectObj->SetLogicRect(aLogicRect);
                pRectObj->SetOutlinerParaObject( pOutlParaObject );
                pDrView->InsertObject(pRectObj, *pDrView->GetPageViewPvNum(0));
                pOutl->Init( nOutlMode );
                pOutl->SetMinDepth( nMinDepth );
            }

            delete pFieldItem;

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            OutlinerView* pOLV = pDrView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( pFldItem && (pFldItem->GetField()->ISA( SvxDateField ) ||
                                 pFldItem->GetField()->ISA( SvxAuthorField ) ||
                                 pFldItem->GetField()->ISA( SvxExtFileField ) ||
                                 pFldItem->GetField()->ISA( SvxExtTimeField ) ) )
                {
                    // Dialog...
                    SdModifyFieldDlg aDlg( pWindow, pFldItem->GetField() );
                    if( aDlg.Execute() == RET_OK )
                    {
                        SvxFieldData* pField = aDlg.GetField();
                        if( pField )
                        {
                            SvxFieldItem aFieldItem( *pField );
                            //pOLV->DeleteSelected(); <-- fehlt leider !
                            // Feld selektieren, so dass es beim Insert geloescht wird
                            ESelection aSel = pOLV->GetSelection();
                            BOOL bSel = TRUE;
                            if( aSel.nStartPos == aSel.nEndPos )
                            {
                                bSel = FALSE;
                                aSel.nEndPos++;
                            }
                            pOLV->SetSelection( aSel );

                            pOLV->InsertField( aFieldItem );

                            // Selektion wird wieder in den Ursprungszustand gebracht
                            if( !bSel )
                                aSel.nEndPos--;
                            pOLV->SetSelection( aSel );

                            delete pField;
                        }
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        default:
        {
            // switch Anweisung wegen CLOOKS aufgeteilt. Alle case-Anweisungen die
            // eine Fu???? -Funktion aufrufen, sind in die Methode FuTemp03 (drviewsb)
            // gewandert.
            FuTemp03(rReq);
        }
        break;
    };
};


