/*************************************************************************
 *
 *  $RCSfile: docshel4.cxx,v $
 *
 *  $Revision: 1.61 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:44:12 $
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

#include "DrawDocShell.hxx"

#ifndef _COM_SUN_STAR_DOCUMENT_PRINTERINDEPENDENTLAYOUT_HPP_
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXFLAGITEM_HXX //autogen
#include <svtools/flagitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _DISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXSTBMGR_HXX //autogen
#include <sfx2/stbmgr.hxx>
#endif
#ifndef _SFX_ECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifdef TF_STARONE
#include "unomodel.hxx"
#endif

#include <svtools/fltrcfg.hxx>

#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif
#ifndef _SD_UNODRAWVIEW_HXX
#include "SdUnoDrawView.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif

#pragma hdrstop

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "strmname.h"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "optsitem.hxx"
#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif
#include "sdattr.hxx"
#include "drawdoc.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "app.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdpage.hxx"
#include "sdresid.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
//CHINA001 #include "new_foil.hxx"
#include "sdmod.hxx"
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#include "sdxmlwrp.hxx"
#include "sdpptwrp.hxx"
#include "sdcgmfilter.hxx"
#include "sdgrffilter.hxx"
#include "sdbinfilter.hxx"
#include "sdhtmlfilter.hxx"

namespace sd {

/*************************************************************************
|*
|* SfxPrinter ggf. erzeugen und zurueckgeben
|*
\************************************************************************/

SfxPrinter* DrawDocShell::GetPrinter(BOOL bCreate)
{
    if (bCreate && !pPrinter)
    {
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                            0 );
        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT,
                            SD_MOD()->GetSdOptions(pDoc->GetDocumentType()));
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        USHORT      nFlags = 0;

        nFlags =  (aPrintItem.IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        pPrinter = new SfxPrinter(pSet);
        bOwnPrinter = TRUE;

        // Ausgabequalitaet setzen
        UINT16 nQuality = aPrintItem.GetOutputQuality();

        ULONG nMode = DRAWMODE_DEFAULT;

        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

        pPrinter->SetDrawMode( nMode );

        MapMode aMM (pPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        pPrinter->SetMapMode(aMM);
        UpdateRefDevice();
    }
    return pPrinter;
}

/*************************************************************************
|*
|* neuen SfxPrinter setzen (Eigentuemeruebergang)
|*
\************************************************************************/

void DrawDocShell::SetPrinter(SfxPrinter *pNewPrinter)
{
    if ( pViewShell )
    {
        ::sd::View* pView = pViewShell->GetView();
        if ( pView->IsTextEdit() )
            pView->EndTextEdit();
    }

    if ( pPrinter && bOwnPrinter && (pPrinter != pNewPrinter) )
    {
        delete pPrinter;
    }

    pPrinter = pNewPrinter;
    bOwnPrinter = TRUE;

    delete pFontList;
    pFontList = new FontList( GetPrinter(TRUE), Application::GetDefaultDevice(), FALSE );
    SvxFontListItem aFontListItem( pFontList );
    PutItem( aFontListItem );
    UpdateRefDevice();
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
Printer* DrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(FALSE);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
void DrawDocShell::OnDocumentPrinterChanged(Printer* pNewPrinter)
{
    // if we already have a printer, see if its the same
    if( pPrinter )
    {
        // easy case
        if( pPrinter == pNewPrinter )
            return;

        // compare if its the same printer with the same job setup
        if( (pPrinter->GetName() == pNewPrinter->GetName()) &&
            (pPrinter->GetJobSetup() == pNewPrinter->GetJobSetup()))
            return;
    }

    //  if (pPrinter->IsA(SfxPrinter))
    {
        // Da kein RTTI verfuegbar, wird hart gecasted (...)
        SetPrinter((SfxPrinter*) pNewPrinter);

        // Printer gehoert dem Container
        bOwnPrinter = FALSE;
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
void DrawDocShell::UpdateRefDevice()
{
    if( pDoc )
    {
        // Determine the device for which the output will be formatted.
        OutputDevice* pRefDevice = NULL;
        switch (pDoc->GetPrinterIndependentLayout())
        {
            case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
                pRefDevice = pPrinter;
                break;

            case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
                pRefDevice = SD_MOD()->GetVirtualRefDevice();
                break;

            default:
                // We are confronted with an invalid or un-implemented
                // layout mode.  Use the printer as formatting device
                // as a fall-back.
                DBG_ASSERT(false, "DrawDocShell::UpdateRefDevice(): Unexpected printer layout mode");

                pRefDevice = pPrinter;
                break;
        }
        pDoc->SetRefDevice( pRefDevice );

        ::sd::Outliner* pOutl = pDoc->GetOutliner( FALSE );

        if( pOutl )
            pOutl->SetRefDevice( pRefDevice );

        ::sd::Outliner* pInternalOutl = pDoc->GetInternalOutliner( FALSE );

        if( pInternalOutl )
            pInternalOutl->SetRefDevice( pRefDevice );
    }
}

/*************************************************************************
|*
|* InitNew, (Dokument wird neu erzeugt): Streams oeffnen
|*
\************************************************************************/

BOOL DrawDocShell::InitNew( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    bRet = SfxInPlaceObject::InitNew( pStor );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {
        if( !bSdDataObj )
            pDoc->NewOrLoadCompleted(NEW_DOC);  // otherwise calling
                                                // NewOrLoadCompleted(NEW_LOADED) in
                                                // SdDrawDocument::AllocModel()
    }
    return bRet;
}

/*************************************************************************
|*
|* Load: Pools und Dokument laden
|*
\************************************************************************/

sal_Bool DrawDocShell::IsNewDocument() const
{
    return( mbNewDocument &&
            ( !GetMedium() || GetMedium()->GetURLObject().GetProtocol() == INET_PROT_NOT_VALID ) );
}

/*************************************************************************
|*
|* Load: Pools und Dokument laden
|*
\************************************************************************/

BOOL DrawDocShell::Load( SvStorage* pStore )
{
    mbNewDocument = sal_False;

    ULONG   nStoreVer = pStore->GetVersion();
    BOOL    bRet = FALSE;
    BOOL    bXML = ( nStoreVer >= SOFFICE_FILEFORMAT_60 );
    BOOL    bBinary = ( nStoreVer < SOFFICE_FILEFORMAT_60 );
    bool    bStartPresentation = false;

    if( bBinary || bXML )
    {
        SfxItemSet* pSet = GetMedium()->GetItemSet();


        if( pSet )
        {
            if( (  SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW ) ) && ( (SfxBoolItem&) ( pSet->Get( SID_PREVIEW ) ) ).GetValue() )
            {
                pDoc->SetStarDrawPreviewMode( TRUE );
            }

            if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
                ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
            {
                bStartPresentation = true;
                pDoc->SetStartWithPresentation( true );
            }
        }

        bRet = SfxInPlaceObject::Load( pStore );

        if( bRet )
        {
            SdFilter*   pFilter = NULL;
            SfxMedium* pMedium = 0L;

            if( bBinary )
            {
                pMedium = new SfxMedium( pStore );
                pFilter = new SdBINFilter( *pMedium, *this, sal_True );
            }
            else if( bXML )
            {
                // #80365# use the medium from the DrawDocShell, do not construct an own one
                pFilter = new SdXMLFilter( *GetMedium(), *this, sal_True, SDXMLMODE_Normal, nStoreVer );
            }

            bRet = pFilter ? pFilter->Import() : FALSE;


            if(pFilter)
                delete pFilter;
            if(pMedium)
                delete pMedium;
        }
    }
    else
        pStore->SetError( SVSTREAM_WRONGVERSION );

    if( bRet )
    {
        UpdateTablePointers();

        // #108451# If we're an embedded OLE object, use tight bounds
        // for our visArea. No point in showing the user lots of empty
        // space. Had to remove the check for empty VisArea below,
        // since XML load always sets a VisArea before.
        if( ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ) && SfxInPlaceObject::GetVisArea( ASPECT_CONTENT ).IsEmpty() )
        {
            SdPage* pPage = pDoc->GetSdPage( 0, PK_STANDARD );

            if( pPage )
                SetVisArea( Rectangle( pPage->GetAllObjBoundRect() ) );
        }

        FinishedLoading( SFX_LOADED_ALL );
    }
    else
    {
        if( pStore->GetError() == ERRCODE_IO_BROKENPACKAGE )
            SetError( ERRCODE_IO_BROKENPACKAGE );

        pStore->SetError( SVSTREAM_WRONGVERSION );
    }

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() || bStartPresentation )
    {
        SfxItemSet *pSet = GetMedium()->GetItemSet();

        if( pSet )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, bStartPresentation ? 1 : 5 ) );
    }

    return bRet;
}

/*************************************************************************
|*
|* LoadFrom: Inhalte fuer Organizer laden
|*
\************************************************************************/

BOOL DrawDocShell::LoadFrom(SvStorage* pStor)
{
    mbNewDocument = sal_False;

    const ULONG nStoreVer = pStor->GetVersion();
    const BOOL bBinary = ( nStoreVer < SOFFICE_FILEFORMAT_60 );

    WaitObject* pWait = NULL;
    if( pViewShell )
        pWait = new WaitObject( (Window*) pViewShell->GetActiveWindow() );

    BOOL bRet = FALSE;
    if( bBinary )
    {

        BOOL bRet1 = SfxObjectShell::LoadFrom(pStor);
        BOOL bRet2 = TRUE;

        // da trotz eines erfolgten InitNew() noch LoadFrom() gerufen werden kann,
        // muessen die Vorlagen hier geloescht werden
        GetStyleSheetPool()->Clear();

        // Pool und StyleSheet Pool laden
        SvStorageStreamRef aPoolStm = pStor->OpenStream(pSfxStyleSheets);
        aPoolStm->SetVersion(pStor->GetVersion());
        aPoolStm->SetKey(pStor->GetKey());
        bRet2 = aPoolStm->GetError() == 0;
        if (bRet2)
        {
            aPoolStm->SetBufferSize( 32768 );
            GetPool().SetFileFormatVersion((USHORT)pStor->GetVersion());
            GetPool().Load(*aPoolStm);
            bRet2 = aPoolStm->GetError() == 0;
            DBG_ASSERT(bRet2, "Fehler beim Laden des Item-Pools");
        }

        if (bRet2)
        {
            GetStyleSheetPool()->Load(*aPoolStm);
            bRet2 = aPoolStm->GetError() == 0;
            aPoolStm->SetBufferSize(0);
            DBG_ASSERT(bRet2, "Fehler beim Laden des StyleSheet-Pools");
        }

        bRet = bRet1 || bRet2;
    }
    else
    {
        // #90691# return to old behaviour (before #80365#): construct own medium
        SfxMedium aMedium(pStor);

        // #90691# for having a progress bar nonetheless for XML copy it
        // from the local DocShell medium (GetMedium()) to the constructed one
        SfxMedium* pLocalMedium = GetMedium();
        if(pLocalMedium)
        {
            SfxItemSet* pLocalItemSet = pLocalMedium->GetItemSet();
            SfxItemSet* pDestItemSet = aMedium.GetItemSet();

            if(pLocalItemSet && pDestItemSet)
            {
                const SfxUnoAnyItem* pItem = static_cast<
                    const SfxUnoAnyItem*>(
                        pLocalItemSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));

                if(pItem)
                {
                    pDestItemSet->Put(*pItem);
                }
            }
        }

        pDoc->NewOrLoadCompleted( NEW_DOC );
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();

        SdFilter* pFilter = new SdXMLFilter( aMedium, *this, sal_True, SDXMLMODE_Organizer, nStoreVer );

        bRet = pFilter ? pFilter->Import() : FALSE;
        delete pFilter;

    }

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pSet = GetMedium()->GetItemSet();

        if( pSet )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }

    delete pWait;

    return bRet;
}

/*************************************************************************
|*
|* ConvertFrom: aus Fremdformat laden
|*
\************************************************************************/

BOOL DrawDocShell::ConvertFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    const String    aFilterName( rMedium.GetFilter()->GetFilterName() );
    SdFilter*       pFilter = NULL;
    BOOL            bRet = FALSE;

    SetWaitCursor( TRUE );

    if( aFilterName == pFilterPowerPoint97 || aFilterName == pFilterPowerPoint97Template)
    {
        pDoc->StopWorkStartupDelay();
        pFilter = new SdPPTFilter( rMedium, *this, sal_True );
    }
    else if (aFilterName.SearchAscii("impress8" )  != STRING_NOTFOUND ||
             aFilterName.SearchAscii("draw8")  != STRING_NOTFOUND )
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
        pFilter = new SdXMLFilter( rMedium, *this, sal_True );
    }
    else if (aFilterName.SearchAscii("StarOffice XML (Draw)" )  != STRING_NOTFOUND || aFilterName.SearchAscii("StarOffice XML (Impress)")  != STRING_NOTFOUND )
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
        pFilter = new SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 );
    }
    else if( aFilterName.EqualsAscii( "CGM - Computer Graphics Metafile" ) )
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
        pFilter = new SdCGMFilter( rMedium, *this, sal_True );
    }
    else
    {
        pDoc->CreateFirstPages();
        pDoc->StopWorkStartupDelay();
        pFilter = new SdGRFFilter( rMedium, *this, sal_True );
    }

    bRet = pFilter ? pFilter->Import() : FALSE;
    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
    delete pFilter;

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pSet = GetMedium()->GetItemSet();

        if( pSet )
            pSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }
    SetWaitCursor( FALSE );

    return bRet;
}

/*************************************************************************
|*
|* Save: Pools und Dokument in die offenen Streams schreiben
|*
\************************************************************************/

BOOL DrawDocShell::Save()
{
    pDoc->StopWorkStartupDelay();

    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SvInPlaceObject::SetVisArea( Rectangle() );

    BOOL bRet = SfxInPlaceObject::Save();

    if( bRet )
    {
        SvStorage*  pStore = GetStorage();
        SfxMedium   aMedium( pStore );
        SdFilter*   pFilter = NULL;

        if( pStore->GetVersion() >= SOFFICE_FILEFORMAT_60 )
            pFilter = new SdXMLFilter( aMedium, *this, sal_True, SDXMLMODE_Normal, pStore->GetVersion() );
        else
            pFilter = new SdBINFilter( aMedium, *this, sal_True );

        // #86834# Call UpdateDocInfoForSave() before export
        UpdateDocInfoForSave();

        bRet = pFilter ? pFilter->Export() : FALSE;
        delete pFilter;
    }

    return bRet;
}

/*************************************************************************
|*
|* SaveAs: Pools und Dokument in den angegebenen Storage sichern
|*
\************************************************************************/

BOOL DrawDocShell::SaveAs( SvStorage* pStore )
{
    pDoc->StopWorkStartupDelay();

    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SvInPlaceObject::SetVisArea( Rectangle() );

    UINT32  nVBWarning = ERRCODE_NONE;
    BOOL    bRet = SfxInPlaceObject::SaveAs( pStore );

    if( bRet )
    {
        SdFilter* pFilter = NULL;

        if( pStore->GetVersion() >= SOFFICE_FILEFORMAT_60 )
        {
            SfxMedium aMedium( pStore );
            pFilter = new SdXMLFilter( aMedium, *this, sal_True, SDXMLMODE_Normal, pStore->GetVersion() );

            // #86834# Call UpdateDocInfoForSave() before export
            UpdateDocInfoForSave();

            bRet = pFilter->Export();
        }
        else
        {
            SvtFilterOptions* pBasOpt = SvtFilterOptions::Get();

            if( pBasOpt && pBasOpt->IsLoadPPointBasicStorage() )
                nVBWarning = SvxImportMSVBasic::GetSaveWarningOfMSVBAStorage( *this );

            SfxMedium aMedium( pStore );
            pFilter = new SdBINFilter( aMedium, *this, sal_True );

            // #86834# Call UpdateDocInfoForSave() before export
            UpdateDocInfoForSave();

            const ULONG nOldSwapMode = pDoc->GetSwapGraphicsMode();
            pDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );
            if( !( bRet = pFilter->Export() ) )
                pDoc->SetSwapGraphicsMode( nOldSwapMode );

        }

        delete pFilter;
    }

    if( GetError() == ERRCODE_NONE )
        SetError( nVBWarning );

    return bRet;
}

/*************************************************************************
|*
|* ConvertTo: im Fremdformat speichern
|*
\************************************************************************/

BOOL DrawDocShell::ConvertTo( SfxMedium& rMedium )
{
    BOOL bRet = FALSE;

    if( pDoc->GetPageCount() )
    {
        const SfxFilter*    pMediumFilter = rMedium.GetFilter();
        const String        aTypeName( pMediumFilter->GetTypeName() );
        SdFilter*           pFilter = NULL;

        if( aTypeName.SearchAscii( "graphic_HTML" ) != STRING_NOTFOUND )
        {
            pFilter = new SdHTMLFilter( rMedium, *this, sal_True );
        }
        else if( aTypeName.SearchAscii( "MS_PowerPoint_97" ) != STRING_NOTFOUND )
        {
            pFilter = new SdPPTFilter( rMedium, *this, sal_True );
            ((SdPPTFilter*)pFilter)->PreSaveBasic();
        }
        else if ( aTypeName.SearchAscii( "CGM_Computer_Graphics_Metafile" ) != STRING_NOTFOUND )
        {
            pFilter = new SdCGMFilter( rMedium, *this, sal_True );
        }
        else if( ( aTypeName.SearchAscii( "draw8" ) != STRING_NOTFOUND ) ||
                 ( aTypeName.SearchAscii( "impress8" ) != STRING_NOTFOUND ) )
        {
            pFilter = new SdXMLFilter( rMedium, *this, sal_True );
            UpdateDocInfoForSave();
        }
        else if( ( aTypeName.SearchAscii( "StarOffice_XML_Impress" ) != STRING_NOTFOUND ) ||
                 ( aTypeName.SearchAscii( "StarOffice_XML_Draw" ) != STRING_NOTFOUND ) )
        {
            pFilter = new SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 );
            UpdateDocInfoForSave();
        }
        else
        {
            pFilter = new SdGRFFilter( rMedium, *this, sal_True );
        }

        if( pFilter )
        {
            const ULONG nOldSwapMode = pDoc->GetSwapGraphicsMode();

            pDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

            if( !( bRet = pFilter->Export() ) )
                pDoc->SetSwapGraphicsMode( nOldSwapMode );

            delete pFilter;
        }
    }

    return  bRet;
}

/*************************************************************************
|*
|* SaveCompleted: die eigenen Streams wieder oeffnen, damit kein anderer
|*                                sie "besetzt"
|*
\************************************************************************/

BOOL DrawDocShell::SaveCompleted( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    if( SfxInPlaceObject::SaveCompleted(pStor) )
    {
        pDoc->NbcSetChanged( FALSE );

        if( pViewShell )
        {
            if( pViewShell->ISA( OutlineViewShell ) )
                static_cast<OutlineView*>(pViewShell->GetView())
                    ->GetOutliner()->ClearModifyFlag();

            SdrOutliner* pOutl = pViewShell->GetView()->GetTextEditOutliner();
            if( pOutl )
            {
                SdrObject* pObj = pViewShell->GetView()->GetTextEditObject();
                if( pObj )
                    pObj->NbcSetOutlinerParaObject( pOutl->CreateParaObject() );

                pOutl->ClearModifyFlag();
            }
        }

        bRet = TRUE;

        SfxViewFrame* pFrame = ( pViewShell && pViewShell->GetViewFrame() ) ?
                               pViewShell->GetViewFrame() :
                               SfxViewFrame::Current();

        if( pFrame )
            pFrame->GetBindings().Invalidate( SID_NAVIGATOR_STATE, TRUE, FALSE );

#ifndef SVX_LIGHT
        // throw away old streams
        if( pDoc )
            pDoc->HandsOff();
#endif
    }
    return bRet;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawDocShell::HandsOff()
{
    SfxInPlaceObject::HandsOff();

#ifndef SVX_LIGHT
    // throw away old streams
    if( pDoc )
        pDoc->HandsOff();
#endif
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SdDrawDocument* DrawDocShell::GetDoc()
{
    return pDoc;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SfxStyleSheetBasePool* DrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) pDoc->GetStyleSheetPool() );
}

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

BOOL DrawDocShell::GotoBookmark(const String& rBookmark)
{
    OSL_TRACE("GotoBookmark %s",
        ::rtl::OUStringToOString(rBookmark, RTL_TEXTENCODING_UTF8).getStr());
    BOOL bFound = FALSE;

    if (pViewShell && pViewShell->ISA(DrawViewShell))
    {
        DrawViewShell* pDrViewSh = static_cast<DrawViewShell*>(pViewShell);

        String aBookmark( rBookmark );

        if( rBookmark.Len() && rBookmark.GetChar( 0 ) == sal_Unicode('#') )
            aBookmark = rBookmark.Copy( 1 );

        // Ist das Bookmark eine Seite?
        BOOL        bIsMasterPage;
        USHORT      nPgNum = pDoc->GetPageByName( aBookmark, bIsMasterPage );
        SdrObject*  pObj = NULL;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // Ist das Bookmark ein Objekt?
            pObj = pDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->GetPage()->GetPageNum();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /**********************************************************
            * Zur Seite springen
            **********************************************************/
            bFound = TRUE;
            SdPage* pPage;
            if (bIsMasterPage)
                pPage = (SdPage*) pDoc->GetMasterPage(nPgNum);
            else
                pPage = (SdPage*) pDoc->GetPage(nPgNum);

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( pViewShell && pViewShell->GetViewFrame() ) ?
                  pViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                // Die aktuelle ViewShell hat sich geaendert!
                pDrViewSh = static_cast<DrawViewShell*>(pViewShell);
            }

            EditMode eNewEditMode = EM_PAGE;

            if( bIsMasterPage )
            {
                eNewEditMode = EM_MASTERPAGE;
            }

            if (eNewEditMode != pDrViewSh->GetEditMode())
            {
                // EditMode setzen
                pDrViewSh->ChangeEditMode(eNewEditMode, FALSE);
            }

            // Jump to the page.  This is done by using the API because this
            // takes care of all the little things to be done.  Especially
            // writing the view data to the frame view (see bug #107803#).
            USHORT nSdPgNum = (nPgNum - 1) / 2;
            SdUnoDrawView* pUnoDrawView = static_cast<SdUnoDrawView*>(
                pDrViewSh->GetController());
            if (pUnoDrawView != NULL)
            {
                ::com::sun::star::uno::Reference<
                      ::com::sun::star::drawing::XDrawPage> xDrawPage (
                          pPage->getUnoPage(), ::com::sun::star::uno::UNO_QUERY);
                pUnoDrawView->setCurrentPage (xDrawPage);
            }
            else
            {
                // As a fall back switch to the page via the core.
                DBG_ASSERT (pUnoDrawView!=NULL,
                    "DrawDocShell::GotoBookmark: can't switch page via API");
                pDrViewSh->SwitchPage(nSdPgNum);
            }


            if (pObj)
            {
                // Objekt einblenden und selektieren
                pDrViewSh->MakeVisible(pObj->GetLogicRect(),
                                       *pDrViewSh->GetActiveWindow());
                pDrViewSh->GetView()->UnmarkAll();
                pDrViewSh->GetView()->MarkObj(pObj, pDrViewSh->GetView()->GetPageViewPvNum(0), FALSE);
            }
        }

        SfxBindings& rBindings = ( ( pViewShell && pViewShell->GetViewFrame() ) ?
                                 pViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, TRUE, FALSE);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

/*************************************************************************
|*
|* SaveAsOwnFormat: wenn es eine Dokumentvorlage werden soll,
|*
\************************************************************************/
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

BOOL DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{

    const SfxFilter* pFilter = rMedium.GetFilter();

    if (pFilter->IsOwnTemplateFormat())
    {
        // jetzt die StarDraw-Spezialitaeten:
        // die Layoutvorlagen der ersten Seite werden mit dem jetzt
        // bekannten Layoutnamen versehen, die Layoutnamen der betroffenen
        // Masterpages und Seiten werden gesetzt;
        // alle Textobjekte der betroffenen Standard-, Notiz- und
        // Masterpages werden ueber die Namensaenderung informiert

        String aLayoutName;

        SfxStringItem* pLayoutItem;
        if( rMedium.GetItemSet()->GetItemState(SID_TEMPLATE_NAME, FALSE, (const SfxPoolItem**) & pLayoutItem ) == SFX_ITEM_SET )
        {
            aLayoutName = pLayoutItem->GetValue();
        }
        else
        {
            INetURLObject aURL( rMedium.GetName() );
            aURL.removeExtension();
            aLayoutName = aURL.getName();
        }

        if( aLayoutName.Len() )
        {
            String aOldPageLayoutName = pDoc->GetSdPage(0, PK_STANDARD)->GetLayoutName();
            pDoc->RenameLayoutTemplate(aOldPageLayoutName, aLayoutName);
        }
    }

    return SfxObjectShell::SaveAsOwnFormat(rMedium);
}

/*************************************************************************
|*
|* FillClass
|*
\************************************************************************/

void DrawDocShell::FillClass(SvGlobalName* pClassName,
                                        ULONG*  pFormat,
                                        String* pAppName,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        long    nFileFormat) const
{
    SfxInPlaceObject::FillClass(pClassName, pFormat, pAppName, pFullTypeName,
                                pShortTypeName, nFileFormat);

    if (nFileFormat == SOFFICE_FILEFORMAT_31)
    {
        *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_30);
        *pFormat = SOT_FORMATSTR_ID_STARDRAW;
        *pAppName = String(RTL_CONSTASCII_USTRINGPARAM("Sdraw 3.1"));
        *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_31));;
        *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_40)
    {
        *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_40);
        *pFormat = SOT_FORMATSTR_ID_STARDRAW_40;
        *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_40));
        *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
    }
    else
    {
        if (nFileFormat == SOFFICE_FILEFORMAT_50)
        {
            if (eDocType == DOCUMENT_TYPE_DRAW)
            {
                *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_50);
                *pFormat = SOT_FORMATSTR_ID_STARDRAW_50;
                *pFullTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_50));
            }
            else
            {
                *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_50);
                *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_50;
                *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_50));
            }
        }
        else if (nFileFormat == SOFFICE_FILEFORMAT_60)
        {
            if ( eDocType == DOCUMENT_TYPE_DRAW )
            {
                    *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARDRAW_60;
                    *pFullTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_60));
            }
            else
            {
                    *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_60;
                    *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_60));
            }
        }
        else if (nFileFormat == SOFFICE_FILEFORMAT_8)
        {
            if ( eDocType == DOCUMENT_TYPE_DRAW )
            {
                    *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARDRAW_8;
                    *pFullTypeName = String(RTL_CONSTASCII_USTRINGPARAM("Draw 8")); // HACK: method will be removed with new storage API
            }
            else
            {
                    *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_8;
                    *pFullTypeName = String(RTL_CONSTASCII_USTRINGPARAM("Impress 8")); // HACK: method will be removed with new storage API
            }
        }

        *pShortTypeName = String(SdResId( (eDocType == DOCUMENT_TYPE_DRAW) ?
                                          STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT ));
    }
}

OutputDevice* DrawDocShell::GetDocumentRefDev (void)
{
    OutputDevice* pReferenceDevice = SfxInPlaceObject::GetDocumentRefDev ();
    // Only when our parent does not have a reference device then we return
    // our own.
    if (pReferenceDevice == NULL && pDoc != NULL)
        pReferenceDevice = pDoc->GetRefDevice ();
    return pReferenceDevice;
}
} // end of namespace sd
