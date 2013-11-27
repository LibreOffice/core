/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawDocShell.hxx"
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <tools/urlobj.hxx>
#include <sfx2/progress.hxx>
#include <vcl/waitobj.hxx>
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <editeng/flstitem.hxx>
#include <editeng/eeitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/flagitem.hxx>
#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#ifndef _DISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#include <svx/svdotext.hxx>
#include <svl/style.hxx>
#include <sfx2/printer.hxx>
#include <svtools/ctrltool.hxx>
#ifndef _SFX_ECODE_HXX //autogen
#include <svtools/sfxecode.hxx>
#endif
#include <sot/clsids.hxx>
#include <sot/formats.hxx>
#include <sfx2/request.hxx>
#ifdef TF_STARONE
#include "unomodel.hxx"
#endif

#include <unotools/fltrcfg.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
//#include <svx/svxmsbas.hxx>
#include <unotools/saveopt.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdlegacy.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "strmname.h"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "optsitem.hxx"
#include "Outliner.hxx"
#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "ViewShell.hxx"
#include "app.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellBase.hxx"
#include "Window.hxx"
#include "sdmod.hxx"
#include "OutlineViewShell.hxx"
#include "sdxmlwrp.hxx"
#include "sdpptwrp.hxx"
#include "sdcgmfilter.hxx"
#include "sdgrffilter.hxx"
#include "sdhtmlfilter.hxx"
#include "framework/FrameworkHelper.hxx"
#include <tools/urlobj.hxx>

//IAccessibility2 Implementation 2009-----
#include <sfx2/viewfrm.hxx>
#include "SdUnoDrawView.hxx"
//-----IAccessibility2 Implementation 2009
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::sd::framework::FrameworkHelper;


namespace sd {

/*************************************************************************
|*
|* SfxPrinter ggf. erzeugen und zurueckgeben
|*
\************************************************************************/

SfxPrinter* DrawDocShell::GetPrinter(bool bCreate)
{
    if (bCreate && !mpPrinter)
    {
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,  SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,   SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT,         ATTR_OPTIONS_PRINT,
                            0 );
        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT,
                            SD_MOD()->GetSdOptions(mpDoc->GetDocumentType()));
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        sal_uInt16      nFlags = 0;

        nFlags =  (aPrintItem.GetOptionsPrint().IsWarningSize() ? SFX_PRINTER_CHG_SIZE : 0) |
                (aPrintItem.GetOptionsPrint().IsWarningOrientation() ? SFX_PRINTER_CHG_ORIENTATION : 0);
        aFlagItem.SetValue( nFlags );

        pSet->Put( aPrintItem );
        pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, aPrintItem.GetOptionsPrint().IsWarningPrinter() ) );
        pSet->Put( aFlagItem );

        mpPrinter = new SfxPrinter(pSet);
        mbOwnPrinter = true;

        // Ausgabequalitaet setzen
        sal_uInt16 nQuality = aPrintItem.GetOptionsPrint().GetOutputQuality();

        sal_uLong nMode = DRAWMODE_DEFAULT;

        if( nQuality == 1 )
            nMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
        else if( nQuality == 2 )
            nMode = DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;

        mpPrinter->SetDrawMode( nMode );

        MapMode aMM (mpPrinter->GetMapMode());
        aMM.SetMapUnit(MAP_100TH_MM);
        mpPrinter->SetMapMode(aMM);
        UpdateRefDevice();
    }
    return mpPrinter;
}

/*************************************************************************
|*
|* neuen SfxPrinter setzen (Eigentuemeruebergang)
|*
\************************************************************************/

void DrawDocShell::SetPrinter(SfxPrinter *pNewPrinter)
{
    if ( mpViewShell )
    {
        ::sd::View* pView = mpViewShell->GetView();
        if ( pView->IsTextEdit() )
            pView->SdrEndTextEdit();
    }

    if ( mpPrinter && mbOwnPrinter && (mpPrinter != pNewPrinter) )
    {
        delete mpPrinter;
    }

    mpPrinter = pNewPrinter;
    mbOwnPrinter = true;
    if ( mpDoc->GetPrinterIndependentLayout() == ::com::sun::star::document::PrinterIndependentLayout::DISABLED )
        UpdateFontList();
    UpdateRefDevice();
}

void DrawDocShell::UpdateFontList()
{
    delete mpFontList;
    OutputDevice* pRefDevice = NULL;
    if ( mpDoc->GetPrinterIndependentLayout() == ::com::sun::star::document::PrinterIndependentLayout::DISABLED )
        pRefDevice = GetPrinter(true);
    else
        pRefDevice = SD_MOD()->GetVirtualRefDevice();
    mpFontList = new FontList( pRefDevice, NULL, false );
    SvxFontListItem aFontListItem( mpFontList, SID_ATTR_CHAR_FONTLIST );
    PutItem( aFontListItem );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
Printer* DrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(false);
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
void DrawDocShell::OnDocumentPrinterChanged(Printer* pNewPrinter)
{
    // if we already have a printer, see if its the same
    if( mpPrinter )
    {
        // easy case
        if( mpPrinter == pNewPrinter )
            return;

        // compare if its the same printer with the same job setup
        if( (mpPrinter->GetName() == pNewPrinter->GetName()) &&
            (mpPrinter->GetJobSetup() == pNewPrinter->GetJobSetup()))
            return;
    }

    //  if (mpPrinter->IsA(SfxPrinter))
    {
        // Da kein RTTI verfuegbar, wird hart gecasted (...)
        SetPrinter((SfxPrinter*) pNewPrinter);

        // Printer gehoert dem Container
        mbOwnPrinter = false;
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/
void DrawDocShell::UpdateRefDevice()
{
    if( mpDoc )
    {
        // Determine the device for which the output will be formatted.
        OutputDevice* pRefDevice = NULL;
        switch (mpDoc->GetPrinterIndependentLayout())
        {
            case ::com::sun::star::document::PrinterIndependentLayout::DISABLED:
                pRefDevice = mpPrinter;
                break;

            case ::com::sun::star::document::PrinterIndependentLayout::ENABLED:
                pRefDevice = SD_MOD()->GetVirtualRefDevice();
                break;

            default:
                // We are confronted with an invalid or un-implemented
                // layout mode.  Use the printer as formatting device
                // as a fall-back.
                DBG_ASSERT(false, "DrawDocShell::UpdateRefDevice(): Unexpected printer layout mode");

                pRefDevice = mpPrinter;
                break;
        }

        mpDoc->SetReferenceDevice( pRefDevice );

        ::sd::Outliner* pOutl = mpDoc->GetOutliner( false );

        if( pOutl )
            pOutl->SetRefDevice( pRefDevice );

        ::sd::Outliner* pInternalOutl = mpDoc->GetInternalOutliner( false );

        if( pInternalOutl )
            pInternalOutl->SetRefDevice( pRefDevice );
    }
}

/*************************************************************************
|*
|* InitNew, (Dokument wird neu erzeugt): Streams oeffnen
|*
\************************************************************************/

sal_Bool DrawDocShell::InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    bool bRet = false;

    bRet = SfxObjectShell::InitNew( xStorage );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {

        if( !mbSdDataObj )
            mpDoc->NewOrLoadCompleted(NEW_DOC);  // otherwise calling
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

sal_Bool DrawDocShell::Load( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    bool    bRet = false;
    bool    bStartPresentation = false;
    ErrCode nError = ERRCODE_NONE;

    SfxItemSet* pSet = rMedium.GetItemSet();


    if( pSet )
    {
        if( (  SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW ) ) && ( (SfxBoolItem&) ( pSet->Get( SID_PREVIEW ) ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    bRet = SfxObjectShell::Load( rMedium );
    if( bRet )
    {
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );
    }

    if( bRet )
    {
        UpdateTablePointers();

        // #108451# If we're an embedded OLE object, use tight bounds
        // for our visArea. No point in showing the user lots of empty
        // space. Had to remove the check for empty VisArea below,
        // since XML load always sets a VisArea before.
        //TODO/LATER: looks a little bit strange!
        if( ( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ) && SfxObjectShell::GetVisArea( ASPECT_CONTENT ).IsEmpty() )
        {
            SdPage* pPage = mpDoc->GetSdPage( 0, PK_STANDARD );

            if( pPage )
                SetVisArea( Rectangle( sdr::legacy::GetAllObjBoundRect(pPage->getSdrObjectVector()) ) );
        }

        FinishedLoading( SFX_LOADED_ALL );

        const INetURLObject aUrl;
        SfxObjectShell::SetAutoLoad( aUrl, 0, sal_False );
    }
    else
    {
        if( nError == ERRCODE_IO_BROKENPACKAGE )
            SetError( ERRCODE_IO_BROKENPACKAGE, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

        // TODO/LATER: correct error handling?!
        //pStore->SetError( SVSTREAM_WRONGVERSION, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
        else
            SetError( ERRCODE_ABORT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
    }

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() || bStartPresentation )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();
        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, bStartPresentation ? 1 : 5 ) );
    }

    return bRet;
}

/*************************************************************************
|*
|* LoadFrom: Inhalte fuer Organizer laden
|*
\************************************************************************/

sal_Bool DrawDocShell::LoadFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    WaitObject* pWait = NULL;
    if( mpViewShell )
        pWait = new WaitObject( (Window*) mpViewShell->GetActiveWindow() );

    bool bRet = false;

        /*
        // #90691# return to old behaviour (before #80365#): construct own medium
        SfxMedium aMedium(xStorage);

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
        }                           */

        mpDoc->NewOrLoadCompleted( NEW_DOC );
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();

        // TODO/LATER: nobody is interested in the error code?!
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Organizer, SotStorage::GetVersion( rMedium.GetStorage() ) ).Import( nError );


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

sal_Bool DrawDocShell::ConvertFrom( SfxMedium& rMedium )
{
    mbNewDocument = sal_False;

    const String    aFilterName( rMedium.GetFilter()->GetFilterName() );
    bool            bRet = false;
    bool    bStartPresentation = false;

    SetWaitCursor( true );

    SfxItemSet* pSet = rMedium.GetItemSet();
    if( pSet )
    {
        if( (  SFX_ITEM_SET == pSet->GetItemState(SID_PREVIEW ) ) && ( (SfxBoolItem&) ( pSet->Get( SID_PREVIEW ) ) ).GetValue() )
        {
            mpDoc->SetStarDrawPreviewMode( true );
        }

        if( SFX_ITEM_SET == pSet->GetItemState(SID_DOC_STARTPRESENTATION)&&
            ( (SfxBoolItem&) ( pSet->Get( SID_DOC_STARTPRESENTATION ) ) ).GetValue() )
        {
            bStartPresentation = true;
            mpDoc->SetStartWithPresentation( true );
        }
    }

    if( aFilterName == pFilterPowerPoint97 || aFilterName == pFilterPowerPoint97Template)
    {
        mpDoc->StopWorkStartupDelay();
        bRet = SdPPTFilter( rMedium, *this, sal_True ).Import();
    }
    else if (aFilterName.SearchAscii("impress8" )  != STRING_NOTFOUND ||
             aFilterName.SearchAscii("draw8")  != STRING_NOTFOUND )
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True ).Import( nError );

    }
    else if (aFilterName.SearchAscii("StarOffice XML (Draw)" )  != STRING_NOTFOUND || aFilterName.SearchAscii("StarOffice XML (Impress)")  != STRING_NOTFOUND )
    {
        // TODO/LATER: nobody is interested in the error code?!
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        ErrCode nError = ERRCODE_NONE;
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SOFFICE_FILEFORMAT_60 ).Import( nError );
    }
    else if( aFilterName.EqualsAscii( "CGM - Computer Graphics Metafile" ) )
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdCGMFilter( rMedium, *this, sal_True ).Import();
    }
    else
    {
        mpDoc->CreateFirstPages();
        mpDoc->StopWorkStartupDelay();
        bRet = SdGRFFilter( rMedium, *this ).Import();
    }

    FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();

        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, 5 ) );
    }
    SetWaitCursor( false );

    // tell SFX to change viewshell when in preview mode
    if( IsPreview() || bStartPresentation )
    {
        SfxItemSet *pMediumSet = GetMedium()->GetItemSet();
        if( pMediumSet )
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, bStartPresentation ? 1 : 5 ) );
    }

    return bRet;
}

/*************************************************************************
|*
|* Save: Pools und Dokument in die offenen Streams schreiben
|*
\************************************************************************/

sal_Bool DrawDocShell::Save()
{
    mpDoc->StopWorkStartupDelay();

    //TODO/LATER: why this?!
    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    bool bRet = SfxObjectShell::Save();

    if( bRet )
    {
        // #86834# Call UpdateDocInfoForSave() before export
        UpdateDocInfoForSave();

        bRet = SdXMLFilter( *GetMedium(), *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( GetMedium()->GetStorage() ) ).Export();
    }

    return bRet;
}

/*************************************************************************
|*
|* SaveAs: Pools und Dokument in den angegebenen Storage sichern
|*
\************************************************************************/

sal_Bool DrawDocShell::SaveAs( SfxMedium& rMedium )
{
//IAccessibility2 Implementation 2009-----
    mpDoc->setDocAccTitle(String());
    SfxViewFrame* pFrame1 = SfxViewFrame::GetFirst( this );
    if (pFrame1)
    {
        ::Window* pWindow = &pFrame1->GetWindow();
        if ( pWindow )
        {
            ::Window* pSysWin = pWindow->GetSystemWindow();
            if ( pSysWin )
            {
                pSysWin->SetAccessibleName(String());
            }
        }
    }
//-----IAccessibility2 Implementation 2009
    mpDoc->StopWorkStartupDelay();

    //TODO/LATER: why this?!
    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SfxObjectShell::SetVisArea( Rectangle() );

    sal_uInt32  nVBWarning = ERRCODE_NONE;
    bool    bRet = SfxObjectShell::SaveAs( rMedium );

    if( bRet )
    {
        // #86834# Call UpdateDocInfoForSave() before export
        UpdateDocInfoForSave();
        bRet = SdXMLFilter( rMedium, *this, sal_True, SDXMLMODE_Normal, SotStorage::GetVersion( rMedium.GetStorage() ) ).Export();
    }

    if( GetError() == ERRCODE_NONE )
        SetError( nVBWarning, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );

    return bRet;
}

/*************************************************************************
|*
|* ConvertTo: im Fremdformat speichern
|*
\************************************************************************/

sal_Bool DrawDocShell::ConvertTo( SfxMedium& rMedium )
{
    bool bRet = false;

    if( mpDoc->GetPageCount() )
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
            pFilter = new SdGRFFilter( rMedium, *this );
        }

        if( pFilter )
        {
            const sal_uLong nOldSwapMode = mpDoc->GetSwapGraphicsMode();

            mpDoc->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

            bRet = pFilter->Export();
            if( !bRet )
                mpDoc->SetSwapGraphicsMode( nOldSwapMode );

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

sal_Bool DrawDocShell::SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
{
    bool bRet = false;

    if( SfxObjectShell::SaveCompleted(xStorage) )
    {
        mpDoc->NbcSetChanged( false );

        if( mpViewShell )
        {
            if( dynamic_cast< OutlineViewShell* >(mpViewShell) )
            {
                static_cast<OutlineView*>(mpViewShell->GetView())->GetOutliner()->ClearModifyFlag();
            }

            SdrOutliner* pOutl = mpViewShell->GetView()->GetTextEditOutliner();
            if( pOutl )
            {
                SdrObject* pObj = mpViewShell->GetView()->GetTextEditObject();
                if( pObj )
                    pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );

                pOutl->ClearModifyFlag();
            }
        }

        bRet = true;

        SfxViewFrame* pFrame = ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                               mpViewShell->GetViewFrame() :
                               SfxViewFrame::Current();

        if( pFrame )
            pFrame->GetBindings().Invalidate( SID_NAVIGATOR_STATE, true, false );
    }
    return bRet;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SdDrawDocument* DrawDocShell::GetDoc()
{
    return mpDoc;
}

/*************************************************************************
|*
|* Referenz auf Dokument
|*
\************************************************************************/

SfxStyleSheetBasePool* DrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) mpDoc->GetStyleSheetPool() );
}

/*************************************************************************
|*
|* Sprung zu Bookmark
|*
\************************************************************************/

bool DrawDocShell::GotoBookmark(const String& rBookmark)
{
    bool bFound = false;
    DrawViewShell* pDrawViewShell = dynamic_cast< DrawViewShell* >(mpViewShell);

    if (pDrawViewShell)
    {
        ViewShellBase& rBase (mpViewShell->GetViewShellBase());

        bool bIsMasterPage = false;
        sal_uInt32 nPageNumber = SDRPAGE_NOTFOUND;
        SdrObject* pObj = NULL;

        rtl::OUString sBookmark( rBookmark );
        const rtl::OUString sInteraction( RTL_CONSTASCII_USTRINGPARAM( "action?" ) );
        if ( sBookmark.match( sInteraction ) )
        {
            const rtl::OUString sJump( RTL_CONSTASCII_USTRINGPARAM( "jump=" ) );
            if ( sBookmark.match( sJump, sInteraction.getLength() ) )
            {
                rtl::OUString aDestination( sBookmark.copy( sInteraction.getLength() + sJump.getLength() ) );
                if ( aDestination.match( String( RTL_CONSTASCII_USTRINGPARAM( "firstslide" ) ) ) )
                {
                    nPageNumber = 1;
                }
                else if ( aDestination.match( String( RTL_CONSTASCII_USTRINGPARAM( "lastslide" ) ) ) )
                {
                    nPageNumber = mpDoc->GetPageCount() - 2;
                }
                else if ( aDestination.match( String( RTL_CONSTASCII_USTRINGPARAM( "previousslide" ) ) ) )
                {
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    nPageNumber = pPage->GetPageNumber();
                    nPageNumber = nPageNumber > 2 ? nPageNumber - 2 : SDRPAGE_NOTFOUND;
                }
                else if ( aDestination.match( String( RTL_CONSTASCII_USTRINGPARAM( "nextslide" ) ) ) )
                {
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    nPageNumber = pPage->GetPageNumber() + 2;
                    if ( nPageNumber >= mpDoc->GetPageCount() )
                        nPageNumber = SDRPAGE_NOTFOUND;
                }
            }
        }
        else
        {
            String aBookmark( rBookmark );

            // Ist das Bookmark eine Seite?
            nPageNumber = mpDoc->GetPageByName( aBookmark, bIsMasterPage );

            if (nPageNumber == SDRPAGE_NOTFOUND)
            {
                // Ist das Bookmark ein Objekt?
                pObj = mpDoc->GetObj(aBookmark);

                if (pObj)
                {
                    SdrPage* pOwningPage = pObj->getSdrPageFromSdrObject();

                    if(pOwningPage)
                    {
                        nPageNumber = pOwningPage->GetPageNumber();
                    }
                }
            }
        }
        if (nPageNumber != SDRPAGE_NOTFOUND)
        {
            // Jump to the bookmarked page.  This is done in three steps.

            bFound = true;
            SdPage* pPage;
            if (bIsMasterPage)
                pPage = (SdPage*) mpDoc->GetMasterPage(nPageNumber);
            else
                pPage = (SdPage*) mpDoc->GetPage(nPageNumber);

            // 1.) Change the view shell to the edit view, the notes view,
            // or the handout view.
            PageKind eNewPageKind = pPage->GetPageKind();

            if( (eNewPageKind != PK_STANDARD) && (mpDoc->GetDocumentType() == DOCUMENT_TYPE_DRAW) )
                return false;

            if (eNewPageKind != pDrawViewShell->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ::rtl::OUString sViewURL;
                switch (eNewPageKind)
                {
                    case PK_STANDARD:
                        sViewURL = FrameworkHelper::msImpressViewURL;
                        break;
                    case PK_NOTES:
                        sViewURL = FrameworkHelper::msNotesViewURL;
                        break;
                    case PK_HANDOUT:
                        sViewURL = FrameworkHelper::msHandoutViewURL;
                        break;
                    default:
                        break;
                }
                if (sViewURL.getLength() > 0)
                {
                    ::boost::shared_ptr<FrameworkHelper> pHelper (
                        FrameworkHelper::Instance(rBase));
                    pHelper->RequestView(
                        sViewURL,
                        FrameworkHelper::msCenterPaneURL);
                    pHelper->WaitForUpdate();

                    // Get the new DrawViewShell.
                    mpViewShell = pHelper->GetViewShell(FrameworkHelper::msCenterPaneURL).get();
                    pDrawViewShell = dynamic_cast<sd::DrawViewShell*>(mpViewShell);
                }
                else
                {
                    pDrawViewShell = NULL;
                }
            }

            if (pDrawViewShell != NULL)
            {
                // Set the edit mode to either the normal edit mode or the
                // master page mode.
                EditMode eNewEditMode = EM_PAGE;
                if (bIsMasterPage)
                {
                    eNewEditMode = EM_MASTERPAGE;
                }

                if (eNewEditMode != pDrawViewShell->GetEditMode())
                {
                    // EditMode setzen
                    pDrawViewShell->ChangeEditMode(eNewEditMode, false);
                }

                // Make the bookmarked page the current page.  This is done
                // by using the API because this takes care of all the
                // little things to be done.  Especially writing the view
                // data to the frame view (see bug #107803#).
                sal_uInt32 nSdPgNum = (nPageNumber - 1) / 2;
                Reference<drawing::XDrawView> xController (rBase.GetController(), UNO_QUERY);
                if (xController.is())
                {
                    Reference<drawing::XDrawPage> xDrawPage (pPage->getUnoPage(), UNO_QUERY);
                    xController->setCurrentPage (xDrawPage);
                }
                else
                {
                    // As a fall back switch to the page via the core.
                    DBG_ASSERT (xController.is(),
                        "DrawDocShell::GotoBookmark: can't switch page via API");
                    pDrawViewShell->SwitchPage(nSdPgNum);
                }

                if (pObj != NULL)
                {
                    // Objekt einblenden und selektieren
                    pDrawViewShell->MakeVisibleAtView(sdr::legacy::GetLogicRange(*pObj), *pDrawViewShell->GetActiveWindow());
                    pDrawViewShell->GetView()->UnmarkAll();
                    pDrawViewShell->GetView()->MarkObj(*pObj, false);
                }
            }
        }

        SfxBindings& rBindings = (pDrawViewShell->GetViewFrame()!=NULL
            ? pDrawViewShell->GetViewFrame()
            : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, true, false);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}

//IAccessibility2 Implementation 2009-----
//Solution: If  object  is marked , return true , else return false .
sal_Bool DrawDocShell::IsMarked(  SdrObject* pObject  )
{
    sal_Bool bisMarked =sal_False;
    DrawViewShell* pDrViewSh = dynamic_cast< DrawViewShell* >(mpViewShell);

    if(pDrViewSh && pObject)
    {
        bisMarked = pDrViewSh->GetView()->isSdrObjectSelected(*pObject);
    }

    return  bisMarked;
}
//Solution: If  object  is marked , return true , else return false .
sal_Bool DrawDocShell::GetObjectIsmarked(const String& rBookmark)
{
    OSL_TRACE("GotoBookmark %s", ::rtl::OUStringToOString(rBookmark, RTL_TEXTENCODING_UTF8).getStr());
    sal_Bool bUnMark = sal_False;
    DrawViewShell* pDrViewSh = dynamic_cast< DrawViewShell* >(mpViewShell);

    if(pDrViewSh)
    {
        String aBookmark( rBookmark );

        if( rBookmark.Len() && rBookmark.GetChar( 0 ) == sal_Unicode('#') )
            aBookmark = rBookmark.Copy( 1 );

        // Ist das Bookmark eine Seite?
        bool bIsMasterPage;
        sal_uInt32 nPgNum(mpDoc->GetPageByName(aBookmark, bIsMasterPage));
        SdrObject* pObj = NULL;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // Ist das Bookmark ein Objekt?
            pObj = mpDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNumber();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /**********************************************************
            * Zur Seite springen
            **********************************************************/

            SdPage* pPage = (SdPage*) mpDoc->GetPage(nPgNum);

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                  mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                // Die aktuelle ViewShell hat sich geaendert!
                pDrViewSh = (DrawViewShell*) mpViewShell;
            }

            EditMode eNewEditMode = EM_PAGE;

            if( bIsMasterPage )
            {
                eNewEditMode = EM_MASTERPAGE;
            }

            if (eNewEditMode != pDrViewSh->GetEditMode())
            {
                // EditMode setzen
                pDrViewSh->ChangeEditMode(eNewEditMode, sal_False);
            }

            // Jump to the page.  This is done by using the API because this
            // takes care of all the little things to be done.  Especially
            // writing the view data to the frame view (see bug #107803#).
            sal_uInt16 nSdPgNum = (nPgNum - 1) / 2;
            SdUnoDrawView* pUnoDrawView = new SdUnoDrawView (
                pDrViewSh->GetViewShellBase().GetDrawController(),
                *pDrViewSh,
                *pDrViewSh->GetView());
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
                    "SdDrawDocShell::GotoBookmark: can't switch page via API");
                pDrViewSh->SwitchPage(nSdPgNum);
            }
            delete pUnoDrawView;


            if(pObj)
            {
                // Objekt einblenden und selektieren
                pDrViewSh->MakeVisibleAtView(
                    pObj->getObjectRange(pDrViewSh->GetView()),
                    *pDrViewSh->GetActiveWindow());

                bUnMark = pDrViewSh->GetView()->isSdrObjectSelected(*pObj);
            }
        }
    }

    return ( bUnMark);
}
//Solution: realize multi-selection of objects
sal_Bool DrawDocShell::GotoTreeBookmark(const String& rBookmark)
{
    OSL_TRACE("GotoBookmark %s", ::rtl::OUStringToOString(rBookmark, RTL_TEXTENCODING_UTF8).getStr());
    sal_Bool bFound = sal_False;
    DrawViewShell* pDrViewSh = dynamic_cast< DrawViewShell* >(mpViewShell);

    if(pDrViewSh)
    {
        String aBookmark( rBookmark );

        if( rBookmark.Len() && rBookmark.GetChar( 0 ) == sal_Unicode('#') )
            aBookmark = rBookmark.Copy( 1 );

        // Ist das Bookmark eine Seite?
        bool bIsMasterPage;
        sal_uInt32 nPgNum(mpDoc->GetPageByName(aBookmark, bIsMasterPage));
        SdrObject*  pObj = NULL;

        if (nPgNum == SDRPAGE_NOTFOUND)
        {
            // Ist das Bookmark ein Objekt?
            pObj = mpDoc->GetObj(aBookmark);

            if (pObj)
            {
                nPgNum = pObj->getSdrPageFromSdrObject()->GetPageNumber();
            }
        }

        if (nPgNum != SDRPAGE_NOTFOUND)
        {
            /**********************************************************
            * Zur Seite springen
            **********************************************************/
            bFound = sal_True;
            SdPage* pPage = (SdPage*) mpDoc->GetPage(nPgNum);

            PageKind eNewPageKind = pPage->GetPageKind();

            if (eNewPageKind != pDrViewSh->GetPageKind())
            {
                // Arbeitsbereich wechseln
                GetFrameView()->SetPageKind(eNewPageKind);
                ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                  mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->
                  GetDispatcher()->Execute( SID_VIEWSHELL0, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );

                // Die aktuelle ViewShell hat sich geaendert!
                pDrViewSh = (DrawViewShell*) mpViewShell;
            }

            EditMode eNewEditMode = EM_PAGE;

            if( bIsMasterPage )
            {
                eNewEditMode = EM_MASTERPAGE;
            }

            if (eNewEditMode != pDrViewSh->GetEditMode())
            {
                // EditMode setzen
                pDrViewSh->ChangeEditMode(eNewEditMode, sal_False);
            }

            // Jump to the page.  This is done by using the API because this
            // takes care of all the little things to be done.  Especially
            // writing the view data to the frame view (see bug #107803#).
            sal_uInt16 nSdPgNum = (nPgNum - 1) / 2;
            SdUnoDrawView* pUnoDrawView = new SdUnoDrawView (
                pDrViewSh->GetViewShellBase().GetDrawController(),
                *pDrViewSh,
                *pDrViewSh->GetView());
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
                    "SdDrawDocShell::GotoBookmark: can't switch page via API");
                pDrViewSh->SwitchPage(nSdPgNum);
            }
            delete pUnoDrawView;


            if (pObj)
            {
                // Objekt einblenden und selektieren
                pDrViewSh->MakeVisibleAtView(
                    pObj->getObjectRange(pDrViewSh->GetView()),
                    *pDrViewSh->GetActiveWindow());
                const bool bUnMark(pDrViewSh->GetView()->isSdrObjectSelected(*pObj));

                pDrViewSh->GetView()->MarkObj(*pObj, bUnMark);
            }
        }

        SfxBindings& rBindings = ( ( mpViewShell && mpViewShell->GetViewFrame() ) ?
                                 mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings();

        rBindings.Invalidate(SID_NAVIGATOR_STATE, sal_True, sal_False);
        rBindings.Invalidate(SID_NAVIGATOR_PAGENAME);
    }

    return (bFound);
}
//-----IAccessibility2 Implementation 2009
/*************************************************************************
|*
|* SaveAsOwnFormat: wenn es eine Dokumentvorlage werden soll,
|*
\************************************************************************/

sal_Bool DrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
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
        if( rMedium.GetItemSet()->GetItemState(SID_TEMPLATE_NAME, false, (const SfxPoolItem**) & pLayoutItem ) == SFX_ITEM_SET )
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
            sal_uInt32 nCount = mpDoc->GetMasterSdPageCount(PK_STANDARD);
            for(sal_uInt32 i = 0; i < nCount; i++)
            {
                String aOldPageLayoutName = mpDoc->GetMasterSdPage(i, PK_STANDARD)->GetLayoutName();
                String aNewLayoutName = aLayoutName;
                // Don't add suffix for the first master page
                if( i > 0 )
                    aNewLayoutName += String::CreateFromInt32(i);

                mpDoc->RenameLayoutTemplate(aOldPageLayoutName, aNewLayoutName);
            }
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
                                        sal_uInt32*  pFormat,
                                        String* ,
                                        String* pFullTypeName,
                                        String* pShortTypeName,
                                        sal_Int32 nFileFormat,
                                        sal_Bool bTemplate /* = sal_False */) const
{
    if (nFileFormat == SOFFICE_FILEFORMAT_60)
    {
        if ( meDocType == DOCUMENT_TYPE_DRAW )
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
        if ( meDocType == DOCUMENT_TYPE_DRAW )
        {
                *pClassName = SvGlobalName(SO3_SDRAW_CLASSID_60);
                *pFormat = bTemplate ? SOT_FORMATSTR_ID_STARDRAW_8_TEMPLATE : SOT_FORMATSTR_ID_STARDRAW_8;
                *pFullTypeName = String(RTL_CONSTASCII_USTRINGPARAM("Draw 8")); // HACK: method will be removed with new storage API
        }
        else
        {
                *pClassName = SvGlobalName(SO3_SIMPRESS_CLASSID_60);
                *pFormat = bTemplate ? SOT_FORMATSTR_ID_STARIMPRESS_8_TEMPLATE : SOT_FORMATSTR_ID_STARIMPRESS_8;
                *pFullTypeName = String(RTL_CONSTASCII_USTRINGPARAM("Impress 8")); // HACK: method will be removed with new storage API
        }
    }

    *pShortTypeName = String(SdResId( (meDocType == DOCUMENT_TYPE_DRAW) ?
                                      STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT ));
}

OutputDevice* DrawDocShell::GetDocumentRefDev (void)
{
    OutputDevice* pReferenceDevice = SfxObjectShell::GetDocumentRefDev ();
    // Only when our parent does not have a reference device then we return
    // our own.
    if (pReferenceDevice == NULL && mpDoc != NULL)
        pReferenceDevice = mpDoc->GetReferenceDevice();
    return pReferenceDevice;
}

/** executes the SID_OPENDOC slot to let the framework open a document
    with the given URL and this document as a referer */
void DrawDocShell::OpenBookmark( const String& rBookmarkURL )
{
    SfxStringItem   aStrItem( SID_FILE_NAME, rBookmarkURL );
    SfxStringItem   aReferer( SID_REFERER, GetMedium()->GetName() );
    const SfxPoolItem* ppArgs[] = { &aStrItem, &aReferer, 0 };
    ( mpViewShell ? mpViewShell->GetViewFrame() : SfxViewFrame::Current() )->GetBindings().Execute( SID_OPENHYPERLINK, ppArgs );
}

//IAccessibility2 Implementation 2009-----
void DrawDocShell::setDocAccTitle( const String& rTitle )
{
    if (mpDoc )
    {
        mpDoc->setDocAccTitle( rTitle );
    }
}
const String DrawDocShell::getDocAccTitle() const
{
    String sRet;
    if  (mpDoc)
    {
        sRet =  mpDoc->getDocAccTitle();
    }

    return sRet;
}
void DrawDocShell::setDocReadOnly( sal_Bool bReadOnly)
{
    if (mpDoc )
    {
        mpDoc->setDocReadOnly( bReadOnly );
    }
}
sal_Bool DrawDocShell::getDocReadOnly() const
{
    if  (mpDoc)
    {
        return mpDoc->getDocReadOnly();
    }

    return sal_False;
}
//-----IAccessibility2 Implementation 2009
} // end of namespace sd
