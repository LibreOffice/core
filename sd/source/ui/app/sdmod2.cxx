/*************************************************************************
 *
 *  $RCSfile: sdmod2.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-13 09:58:07 $
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

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#define ITEMID_FIELD    EE_FEATURE_FIELD

#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _INETHIST_HXX //autogen
#include <svtools/inethist.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SVX_ADRITEM_HXX //autogen
#include <svx/adritem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#include <svx/svdfield.hxx>

#define ITEMID_SPELLCHECK   0
#include <svx/dialogs.hrc>

#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif
#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDOPAGE_HXX
#include <svx/svdopage.hxx>
#endif

#pragma hdrstop

#ifndef _OFF_OFAIDS_HRC
#include <offmgr/ofaids.hrc>
#endif

#define _SD_DLL                 // fuer SD_MOD()
#include "sdmod.hxx"
#include "sddll.hxx"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "viewshel.hxx"
#include "frmview.hxx"
#include "sdattr.hxx"
#include "tpoption.hrc"
#include "optsitem.hxx"
#include "docshell.hxx"
#include "drawdoc.hxx"
#include "sdoutl.hxx"
#include "dragserv.hxx"
#include "sdresid.hxx"
#include "pres.hxx"
#include "drviewsh.hxx"
#include "outlnvsh.hxx"
#include "outlview.hxx"
#include "sdpage.hxx"
#include "tpoption.hxx"
#include "tpscale.hxx"
#include "prntopts.hxx"


/*************************************************************************
|*
|* Options-Dialog
|*
\************************************************************************/



/*************************************************************************
|*
|* Link fuer CalcFieldValue des Outliners
|*
\************************************************************************/

IMPL_LINK(SdModule, CalcFieldValueHdl, EditFieldInfo*, pInfo)
{
    if (pInfo)
    {
        const SvxFieldItem& rField = pInfo->GetField();
        const SvxFieldData* pField = rField.GetField();

        if (pField && pField->ISA(SvxDateField))
        {
            /******************************************************************
            * Date-Field
            ******************************************************************/
            pInfo->SetRepresentation(
                ((const SvxDateField*) pField)->GetFormatted(LANGUAGE_SYSTEM,
                                                             LANGUAGE_SYSTEM) );
        }
        else if( pField && pField->ISA( SvxExtTimeField ) )
        {
            /******************************************************************
            * Time-Field
            ******************************************************************/
            pInfo->SetRepresentation( ( (const SvxExtTimeField*) pField)->
                            GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ) );
        }
        else if( pField && pField->ISA( SvxExtFileField ) )
        {
            /******************************************************************
            * File-Field
            ******************************************************************/
            const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
            String aFile;
            if( pFileField->GetType() == SVXFILETYPE_FIX )
                aFile =  pFileField->GetFormatted();
            else
            {
                SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell,
                                            SfxObjectShell::Current() );

                if( pDocSh )
                {
                    SvxExtFileField aFileField( *pFileField );

                    String aName;
                    if( pDocSh->HasName() )
                        aName = pDocSh->GetMedium()->GetName();
                    else
                        aName = pDocSh->GetName();

                    aFileField.SetFile( aName );
                    aFile = aFileField.GetFormatted();
                }
            }

            pInfo->SetRepresentation( aFile );

        }
        else if( pField && pField->ISA( SvxAuthorField ) )
        {
            /******************************************************************
            * Author-Field
            ******************************************************************/
            const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
            String aAuthor;
            if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
                aAuthor = pAuthorField->GetFormatted();
            else
            {
                SvxAddressItem aAdrItem;
                SvxAuthorField aAuthorField( aAdrItem, pAuthorField->GetType(),
                                                pAuthorField->GetFormat() );
                aAuthor = aAuthorField.GetFormatted();
            }
            pInfo->SetRepresentation( aAuthor );

        }
        else if( pField && pField->ISA( SvxPageField ) )
        {
            /******************************************************************
            * Page-Field
            ******************************************************************/
            USHORT nPgNum = 1;
            String aRepresentation;
            aRepresentation += sal_Unicode( ' ' );

            SdViewShell* pViewSh = PTR_CAST( SdViewShell, SfxViewShell::Current() );

            if( pViewSh )
            {
                if( pViewSh->ISA( SdOutlineViewShell ) &&
                    pInfo->GetOutliner() == ( (SdOutlineView*) ( (SdOutlineViewShell*) pViewSh)->GetView() )->GetOutliner()  )
                {
                    // outline mode
                    nPgNum = 0;
                    Outliner* pOutl = ((SdOutlineView*)pViewSh->GetView())->GetOutliner();
                    long nPos = pInfo->GetPara();
                    ULONG nParaPos = 0;

                    for( Paragraph* pPara = pOutl->GetParagraph( 0 ); pPara && nPos >= 0; pPara = pOutl->GetParagraph( ++nParaPos ), nPos-- )
                    {
                        if( pOutl->GetDepth( nParaPos ) == 0 )
                            nPgNum++;
                    }
                }
                else
                {
                    // draw mode, slide mode and preview
                    SdPage* pPage = NULL;
                    SdrPageView* pPV = pViewSh->GetDoc()->GetPaintingPageView();

                    if (pPV)
                    {
                        pPage = (SdPage*) pPV->GetPage();
                    }

                    if (pPage && pPage->GetPageKind() != PK_HANDOUT)
                    {
                        // Keine Handzettelseite
                        nPgNum = (pPage->GetPageNum() + 1) / 2;
                    }
                    else
                    {
                        // Handzettelseite
                        const SdrTextObj* pTextObj = pViewSh->GetDoc()->GetFormattingTextObj();

                        if (pTextObj && pTextObj->GetPage())
                        {
                            if (((SdPage*) pTextObj->GetPage())->GetPageKind() == PK_HANDOUT)
                            {
                                // Handzettelseite
                                nPgNum = pViewSh->GetPrintedHandoutPageNum();
                            }
                            else if ( pPV && pPV->GetPaintingPageObj() )
                            {
                                // Textobjekt innerhalb eines Seitendarstellungsobjekts
                                nPgNum = (pPV->GetPaintingPageObj()->GetPageNum() - 1) / 2 + 1;
                            }
                            else
                            {
                                // Textobjekt innerhalb eines Seitendarstellungsobjekts
                                nPgNum = (pTextObj->GetPage()->GetPageNum() - 1) / 2 + 1;
                            }
                        }
                    }
                }

                aRepresentation = pViewSh->GetDoc()->CreatePageNumValue(nPgNum);
            }

            pInfo->SetRepresentation( aRepresentation );
        }
        else if (pField && pField->ISA(SvxURLField))
        {
            /******************************************************************
            * URL-Field
            ******************************************************************/

            switch ( ((const SvxURLField*) pField)->GetFormat() )
            {
                case SVXURLFORMAT_APPDEFAULT: //!!! einstellbar an App???
                case SVXURLFORMAT_REPR:
                {
                    pInfo->SetRepresentation(
                           ((const SvxURLField*)pField)->GetRepresentation() );
                }
                break;

                case SVXURLFORMAT_URL:
                {
                    pInfo->SetRepresentation(
                           ((const SvxURLField*)pField)->GetURL() );
                }
                break;
            }

            String aURL = ((const SvxURLField*)pField)->GetURL();
            Color aColor(INetURLHistory::GetOrCreate()->QueryUrl(aURL)?COL_LIGHTRED:COL_LIGHTBLUE);
            pInfo->SetTxtColor(aColor);
        }
        else if (pField && pField->ISA(SdrMeasureField))
        {
            /******************************************************************
            * Measure-Field
            ******************************************************************/
            pInfo->ClearFldColor();
        }
        else
        {
            DBG_ERROR("unbekannter Feldbefehl");
            String aStr;
            aStr += sal_Unicode( '?' );
            pInfo->SetRepresentation( aStr );
        }
    }

    return(0);
}



/*************************************************************************
|*
|* QueryUnload
|*
\************************************************************************/

BOOL SdModule::QueryUnload()
{
    if (pClipboardData)
    {
        /**********************************************************************
        * Sind viele Daten im Clipboard?
        **********************************************************************/
        BOOL bQuery = FALSE;
        SdDrawDocument* pDrDoc = pClipboardData->pSdDrawDocument;
        USHORT nPageCount = pDrDoc->GetPageCount();
        SdrPage* pPage = pDrDoc->GetPage(0);
        SdrObject* pObj = NULL;
        SdrObjKind eObjKind;
        ULONG nObjCount = pPage->GetObjCount();

        if ( nObjCount < 10 )
        {
            // Objekte naeher betrachten
            for (ULONG nObj = 0; nObj < nObjCount; nObj++)
            {
                pObj = pPage->GetObj(nObj);

                if (pObj && pObj->GetObjInventor() == SdrInventor)
                {
                    eObjKind = (SdrObjKind) pObj->GetObjIdentifier();

                    if (eObjKind == OBJ_GRAF || eObjKind == OBJ_OLE2)
                    {
                        // Ev. groessere Objekte: Query!
                        bQuery = TRUE;
                    }
                }
            }
        }
        else
        {
            // Mehr als 10 Objekte: Query!
            bQuery = TRUE;
        }

        if (bQuery)
        {
            QueryBox aBox( NULL, WinBits( WB_YES_NO | WB_DEF_NO ), String(SdResId(STR_CLPBRD_CLEAR)) );

            if ( RET_YES != aBox.Execute() )
            {
                // Objekte nicht ins Clipboard stellen: loeschen
                SvDataObjectRef aRef = new SvDataObject();
                aRef->CopyClipboard();
            }
        }
    }

    return(TRUE);
}
/*************************************************************************
|*
|* virt. Methoden fuer Optionendialog
|*
\************************************************************************/
SfxItemSet*  SdModule::CreateItemSet( USHORT nSlot )
{
    FrameView* pFrameView = NULL;
    SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
    SdDrawDocument* pDoc = NULL;

    // Hier wird der DocType vom Optionsdialog gesetzt (nicht Dokument!)
    DocumentType eDocType = DOCUMENT_TYPE_IMPRESS;
    if( nSlot == SID_SD_GRAPHIC_OPTIONS )
        eDocType = DOCUMENT_TYPE_DRAW;

    SdViewShell* pViewShell = NULL;

    if (pDocSh)
    {
        pDoc = pDocSh->GetDoc();

        // Wenn der Optionsdialog zum Dokumenttyp identisch ist,
        // kann auch die FrameView mit uebergeben werden:
        if( pDoc && eDocType == pDoc->GetDocumentType() )
            pFrameView = pDocSh->GetFrameView();

        pViewShell = pDocSh->GetViewShell();
        pViewShell->WriteFrameViewData();
    }

    SdOptions* pOptions = GetSdOptions(eDocType);

    // Pool hat standardmaessig MapUnit Twips (Baeh!)
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

    UINT16 nDefTab = 0;
    if( pFrameView)
        nDefTab = pDoc->GetDefaultTabulator();
    else
        nDefTab = pOptions->GetDefTab();
    pRet->Put( SfxUInt16Item( SID_ATTR_DEFTABSTOP, nDefTab ) );

    UINT16 nMetric = 0xffff;
    if( pFrameView)
        nMetric = pDoc->GetUIUnit();
    else
        nMetric = pOptions->GetMetric();

    if( nMetric == 0xffff )
        nMetric = GetModuleFieldUnit();

    pRet->Put( SfxUInt16Item( SID_ATTR_METRIC, nMetric ) );

    // TP_OPTIONS_CONTENTS:
    pRet->Put( SdOptionsContentsItem( ATTR_OPTIONS_CONTENTS, pOptions, pFrameView ) );

    // TP_OPTIONS_MISC:
    pRet->Put( SdOptionsMiscItem( ATTR_OPTIONS_MISC, pOptions, pFrameView ) );

    // TP_OPTIONS_SNAP:
    pRet->Put( SdOptionsSnapItem( ATTR_OPTIONS_SNAP, pOptions, pFrameView ) );

    // TP_SCALE:
    UINT32 nW = 0L;
    UINT32 nH = 0L;
    INT32  nX;
    INT32  nY;
    if( pDocSh )
    {
        Fraction aFract( pDoc->GetUIScale() );
        nX = aFract.GetNumerator();
        nY = aFract.GetDenominator();

        SdrPage* pPage = (SdrPage*) pDoc->GetSdPage(0, PK_STANDARD);
        Size aSize(pPage->GetSize());
        nW = aSize.Width();
        nH = aSize.Height();
    }
    else
    {
        // Optionen aus Configdatei holen
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
void SdModule::ApplyItemSet( USHORT nSlot, const SfxItemSet& rSet )
{
    const SfxPoolItem*  pItem = NULL;
    BOOL bNewDefTab = FALSE;
    BOOL bNewPrintOptions = FALSE;
    FrameView* pFrameView = NULL;
    SdDrawDocShell* pDocSh = PTR_CAST( SdDrawDocShell, SfxObjectShell::Current() );
    SdDrawDocument* pDoc = NULL;
    // Hier wird der DocType vom Optionsdialog gesetzt (nicht Dokument!)
    DocumentType eDocType = DOCUMENT_TYPE_IMPRESS;
    if( nSlot == SID_SD_GRAPHIC_OPTIONS )
        eDocType = DOCUMENT_TYPE_DRAW;

    SdViewShell* pViewShell = NULL;

    if (pDocSh)
    {
        pDoc = pDocSh->GetDoc();

        // Wenn der Optionsdialog zum Dokumenttyp identisch ist,
        // kann auch die FrameView mit uebergeben werden:
        if( pDoc && eDocType == pDoc->GetDocumentType() )
            pFrameView = pDocSh->GetFrameView();

        pViewShell = pDocSh->GetViewShell();
        pViewShell->WriteFrameViewData();
    }
    // TP_GENERAL
    //!!! const weg-casten, da Store nicht-const:
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_ADDRESS, TRUE, &pItem ) )
    {
        ((SfxAddressItem*)pItem)->Store();
    }

    SdOptions* pOptions = GetSdOptions(eDocType);
    // Raster
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS ,
                            FALSE, (const SfxPoolItem**) &pItem ))
    {
        const SdOptionsGridItem* pGridItem = (SdOptionsGridItem*) pItem;
        pGridItem->SetOptions( pOptions );
    }

    // Layout
    const SdOptionsLayoutItem* pLayoutItem = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_LAYOUT,
                            FALSE, (const SfxPoolItem**) &pLayoutItem ))
    {
        pLayoutItem->SetOptions( pOptions );
    }

    // Metric
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC, FALSE, &pItem ) )
    {
        if( pDoc && eDocType == pDoc->GetDocumentType() )
            PutItem( *pItem );
        pOptions->SetMetric( ( (SfxUInt16Item*) pItem )->GetValue() );
    }
    UINT16 nDefTab = pOptions->GetDefTab();
    // Default-Tabulator
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_DEFTABSTOP, FALSE, &pItem ) )
    {
        nDefTab = ( (SfxUInt16Item*) pItem )->GetValue();
        pOptions->SetDefTab( nDefTab );

        bNewDefTab = TRUE;
    }

    // Massstab
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_SCALE_X, FALSE, &pItem ) )
    {
        INT32 nX = ( (SfxInt32Item*) pItem )->GetValue();
        if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_SCALE_Y, FALSE, &pItem ) )
        {
            INT32 nY = ( (SfxInt32Item*) pItem )->GetValue();
            pOptions->SetScale( nX, nY );

            if( pDocSh )
            {
                pDoc->SetUIScale( Fraction( nX, nY ) );
                if( pViewShell )
                    pViewShell->SetRuler( pViewShell->HasRuler() );
            }
        }
    }

    // Contents (Inhalte)
    const SdOptionsContentsItem* pContentsItem = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_CONTENTS,
                            FALSE, (const SfxPoolItem**) &pContentsItem ))
    {
        pContentsItem->SetOptions( pOptions );
    }

    // Misc (Sonstiges)
    const SdOptionsMiscItem* pMiscItem = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_MISC,
                            FALSE, (const SfxPoolItem**) &pMiscItem ))
    {
        pMiscItem->SetOptions( pOptions );
    }

    // Fangen/Einrasten
    const SdOptionsSnapItem* pSnapItem = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_SNAP,
                            FALSE, (const SfxPoolItem**) &pSnapItem ))
    {
        pSnapItem->SetOptions( pOptions );
    }

    SfxItemSet aPrintSet( GetPool(),
                    SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                    ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                    0 );

    // Drucken
    const SdOptionsPrintItem* pPrintItem = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( ATTR_OPTIONS_PRINT,
                            FALSE, (const SfxPoolItem**) &pPrintItem ))
    {
        pPrintItem->SetOptions( pOptions );

        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT, pOptions );
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        USHORT      nFlags = 0;

        nFlags =  (aPrintItem.IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        aPrintSet.Put( aPrintItem );
        aPrintSet.Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.IsWarningPrinter() ) );
        aPrintSet.Put( aFlagItem );

        bNewPrintOptions = TRUE;
    }

    // Nur, wenn auch der Dokumenttyp uebereinstimmt...
    if( pDocSh && pDoc && eDocType == pDoc->GetDocumentType() )
    {
        if( bNewPrintOptions )
        {
            pDocSh->GetPrinter(TRUE)->SetOptions( aPrintSet );
        }

        // Am Model den DefTab setzen
        if( bNewDefTab )
        {
            SdDrawDocument* pDocument = pDocSh->GetDoc();
            pDocument->SetDefaultTabulator( nDefTab );

            SdOutliner* pOutl = pDocument->GetOutliner( FALSE );
            if( pOutl )
                pOutl->SetDefTab( nDefTab );

            SdOutliner* pInternalOutl = pDocument->GetInternalOutliner( FALSE );
            if( pInternalOutl )
                pInternalOutl->SetDefTab( nDefTab );
        }
    }

    pOptions->StoreConfig();
    SFX_APP()->SaveConfiguration();

    // Nur, wenn auch der Dokumenttyp uebereinstimmt...
    if( pDocSh && pDoc && eDocType == pDoc->GetDocumentType() )
    {
        FieldUnit eUIUnit = (FieldUnit) pOptions->GetMetric();
        pDoc->SetUIUnit(eUIUnit);

        if (pViewShell)
        {
            // #74495# make sure no one is in text edit mode, cause there
            // are some pointers remembered else (!)
            if(pViewShell->GetView())
                pViewShell->GetView()->EndTextEdit();

            FrameView* pFrameView = pViewShell->GetFrameView();
            pFrameView->Update(pOptions);
            pViewShell->ReadFrameViewData(pFrameView);
            pViewShell->SetUIUnit(eUIUnit);
            pViewShell->SetDefTabHRuler( nDefTab );
        }
    }

    ( ( pViewShell && pViewShell->GetViewFrame() ) ? pViewShell->GetViewFrame() : SfxViewFrame::Current() )->
        GetBindings().InvalidateAll( TRUE );
}
SfxTabPage*  SdModule::CreateTabPage( USHORT nId, Window* pParent, const SfxItemSet& rSet )
{
    SfxTabPage*  pRet = 0;
    switch(nId)
    {
        case SID_SD_TP_CONTENTS:
        case SID_SI_TP_CONTENTS:
            pRet = SdTpOptionsContents::Create(pParent, rSet);
        break;
        case SID_SD_TP_LAYOUT:
        case SID_SI_TP_LAYOUT:
            pRet = SdTpOptionsLayout::Create(pParent, rSet);
        break;
        case SID_SD_TP_SNAP:
        case SID_SI_TP_SNAP:
            pRet = SdTpOptionsSnap::Create(pParent, rSet);
        break;
        case SID_SD_TP_SCALE:
               pRet = SdTpScale::Create(pParent, rSet);
        break;
        case SID_SI_TP_GRID:
        case SID_SD_TP_GRID:
            pRet = SvxGridTabPage::Create(pParent, rSet);
        break;
        case SID_SD_TP_PRINT:
        case SID_SI_TP_PRINT:
            pRet = SdPrintOptions::Create(pParent, rSet);
            if(SID_SD_TP_PRINT == nId)
            {
                ( (SdPrintOptions*) pRet )->aCbxNotes.Hide();
                ( (SdPrintOptions*) pRet )->aCbxHandout.Hide();
                ( (SdPrintOptions*) pRet )->aCbxOutline.Hide();

                ( (SdPrintOptions*) pRet )->aCbxDraw.Check(); // Wohl nicht noetig !?
                ( (SdPrintOptions*) pRet )->aCbxDraw.Disable();
                ( (SdPrintOptions*) pRet )->aGrpPrint.Disable();
            }
        break;
        case SID_SI_TP_MISC:
        case SID_SD_TP_MISC:
            pRet = SdTpOptionsMisc::Create(pParent, rSet);
            if(SID_SD_TP_MISC == nId)
            {
                ( (SdTpOptionsMisc*) pRet )->aCbxStartWithTemplate.Hide();
                ( (SdTpOptionsMisc*) pRet )->aGrpProgramStart.Hide();

                ( (SdTpOptionsMisc*) pRet )->aCbxStartWithActualPage.Hide();
                ( (SdTpOptionsMisc*) pRet )->aGrpStartWithActualPage.Hide();
            }
            else
            {
                ( (SdTpOptionsMisc*) pRet )->aCbxCrookNoContortion.Hide();
            }
        break;
    }
    DBG_ASSERT(pRet, "Id unbekannt");
    return pRet;


}



