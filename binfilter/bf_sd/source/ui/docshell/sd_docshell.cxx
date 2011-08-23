/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define ITEMID_FONTLIST 				SID_ATTR_CHAR_FONTLIST
#define ITEMID_COLOR_TABLE				SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST			SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST				SID_HATCH_LIST
#define ITEMID_BITMAP_LIST				SID_BITMAP_LIST
#define ITEMID_DASH_LIST				SID_DASH_LIST
#define ITEMID_LINEEND_LIST 			SID_LINEEND_LIST
#define ITEMID_SEARCH					SID_SEARCH_ITEM

#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <comphelper/classids.hxx>

#include <tools/urlobj.hxx>
#include <sot/formats.hxx>

#include <bf_svtools/ctrltool.hxx>
#include <bf_svtools/flagitem.hxx>
#include <bf_svtools/itemset.hxx>

#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/svxids.hrc>
#include <bf_svx/flstitem.hxx>
#include <bf_svx/drawitem.hxx>
#include <bf_sfx2/docfilt.hxx>

#include "sdresid.hxx"
#include "strings.hrc"
#include "optsitem.hxx"
#include "sdattr.hxx"
#include "bf_sd/docshell.hxx"
#include "drawdoc.hxx"
#include "unomodel.hxx"
#include "sdpage.hxx"
#include "sdxmlwrp.hxx"
#include "sdbinfilter.hxx"
#include "sdoutl.hxx"

namespace binfilter
{
TYPEINIT1( SdDrawDocShell, SfxObjectShell );

SFX_IMPL_OBJECTFACTORY_LOD(SdDrawDocShell, simpress, SvGlobalName(BF_SO3_SIMPRESS_CLASSID), Sd)

void SdDrawDocShell::Construct()
{
    bInDestruction = FALSE;
    SetSlotFilter();
    SetShell(this);

    pDoc = new SdDrawDocument(eDocType, this);
    SetModel( new SdXImpressDocument( this ) );
    SetPool( &pDoc->GetItemPool() );
    UpdateTablePointers();
    SetStyleFamily(5);
}

SdDrawDocShell::SdDrawDocShell(SfxObjectCreateMode eMode, BOOL bDataObject, DocumentType eDocumentType) :
    SfxObjectShell(eMode),
    pPrinter(NULL),
    pDoc(NULL),
    bUIActive(FALSE),
    pProgress(NULL),
    bSdDataObj(bDataObject),
    bOwnPrinter(FALSE),
    eDocType(eDocumentType),
    mbNewDocument( sal_True )
{
    Construct();
}

SdDrawDocShell::~SdDrawDocShell()
{
    bInDestruction = TRUE;

    if (bOwnPrinter)
        delete pPrinter;

    delete pDoc;

}

void SdDrawDocShell::Deactivate( BOOL )
{
}

void SdDrawDocShell::UpdateTablePointers()
{
    PutItem( SvxColorTableItem( pDoc->GetColorTable() ) );
    PutItem( SvxGradientListItem( pDoc->GetGradientList() ) );
    PutItem( SvxHatchListItem( pDoc->GetHatchList() ) );
    PutItem( SvxBitmapListItem( pDoc->GetBitmapList() ) );
    PutItem( SvxDashListItem( pDoc->GetDashList() ) );
    PutItem( SvxLineEndListItem( pDoc->GetLineEndList() ) );
}

void SdDrawDocShell::SetModified( BOOL bSet /* = TRUE */ )
{
    SfxInPlaceObject::SetModified( bSet );

    if( IsEnableSetModified() && pDoc )
        pDoc->NbcSetChanged( bSet );

    Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
}


void SdDrawDocShell::SetVisArea(const Rectangle& rRect)
{
    if (GetCreateMode() == SFX_CREATE_MODE_EMBEDDED)
    {
        SfxInPlaceObject::SetVisArea(rRect);
    }
    else
    {
        SvEmbeddedObject::SetVisArea(rRect);
    }
}

Rectangle SdDrawDocShell::GetVisArea(USHORT nAspect) const
{
    Rectangle aVisArea;

    if( ( ASPECT_THUMBNAIL == nAspect ) || ( ASPECT_DOCPRINT == nAspect ) )
    {
         MapMode aSrcMapMode(MAP_PIXEL);
         MapMode aDstMapMode(MAP_100TH_MM);
         Size aSize = pDoc->GetSdPage(0, PK_STANDARD)->GetSize();
         aSrcMapMode.SetMapUnit(MAP_100TH_MM);
 
         aSize = Application::GetDefaultDevice()->LogicToLogic(aSize, &aSrcMapMode, &aDstMapMode);
         aVisArea.SetSize(aSize);
    }
    else
    {
        aVisArea = SfxInPlaceObject::GetVisArea(nAspect);
    }

    return (aVisArea);
}


SfxPrinter* SdDrawDocShell::GetPrinter(BOOL bCreate)
{
    if (bCreate && !pPrinter)
    {
        // ItemSet mit speziellem Poolbereich anlegen
        SfxItemSet* pSet = new SfxItemSet( GetPool(),
                            SID_PRINTER_NOTFOUND_WARN,	SID_PRINTER_NOTFOUND_WARN,
                            SID_PRINTER_CHANGESTODOC,	SID_PRINTER_CHANGESTODOC,
                            ATTR_OPTIONS_PRINT, 		ATTR_OPTIONS_PRINT,
                            0 );
        // PrintOptionsSet setzen
        SdOptionsPrintItem aPrintItem( ATTR_OPTIONS_PRINT,
                            SD_MOD()->GetSdOptions(pDoc->GetDocumentType()));
        SfxFlagItem aFlagItem( SID_PRINTER_CHANGESTODOC );
        USHORT		nFlags = 0;

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

void SdDrawDocShell::SetPrinter(SfxPrinter *pNewPrinter)
{
    if ( pPrinter && bOwnPrinter && (pPrinter != pNewPrinter) )
    {
        delete pPrinter;
    }

    pPrinter = pNewPrinter;
    bOwnPrinter = TRUE;

    UpdateRefDevice();
}

Printer* SdDrawDocShell::GetDocumentPrinter()
{
    return GetPrinter(FALSE);
}

void SdDrawDocShell::UpdateRefDevice()
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
                DBG_ASSERT(false, "SdDrawDocShell::UpdateRefDevice(): Unexpected printer layout mode");
                
                pRefDevice = pPrinter;
                break;
        }
        pDoc->SetRefDevice( pRefDevice );

        SdOutliner* pOutl = pDoc->GetOutliner( FALSE );

        if( pOutl )
/*?*/ 			pOutl->SetRefDevice( pRefDevice );

        SdOutliner* pInternalOutl = pDoc->GetInternalOutliner( FALSE );

        if( pInternalOutl )
            pInternalOutl->SetRefDevice( pRefDevice );
    }
}

BOOL SdDrawDocShell::InitNew( SvStorage * pStor )
{
    BOOL bRet = SfxInPlaceObject::InitNew( pStor );

    Rectangle aVisArea( Point(0, 0), Size(14100, 10000) );
    SetVisArea(aVisArea);

    if (bRet)
    {
        if( !bSdDataObj )
            pDoc->NewOrLoadCompleted(NEW_DOC);
    }
    return bRet;
}

BOOL SdDrawDocShell::Load( SvStorage* pStore )
{
    mbNewDocument = sal_False;

    ULONG	nStoreVer = pStore->GetVersion();
    BOOL	bRet = FALSE;
    BOOL	bXML = ( nStoreVer >= SOFFICE_FILEFORMAT_60 );
    BOOL	bBinary = ( nStoreVer < SOFFICE_FILEFORMAT_60 );
    bool	bStartPresentation = false;

    if( bBinary || bXML )
    {
        bRet = SfxInPlaceObject::Load( pStore );

        if( bRet )
        {
            SdFilter*	pFilter = NULL;
            SfxMedium* pMedium = 0L;

            if( bBinary )
            {
                pMedium = new SfxMedium( pStore );
                pFilter = new SdBINFilter( *pMedium, *this, sal_True );
            }
            else if( bXML )
            {
                pFilter = new SdXMLFilter( *GetMedium(), *this, sal_True );
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

    return bRet;
}

 BOOL SdDrawDocShell::Save()
 {
     if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
         SvInPlaceObject::SetVisArea( Rectangle() );
 
     BOOL bRet = SfxInPlaceObject::Save();
 
     if( bRet )
     {
         SvStorage*	pStore = GetStorage();
         SfxMedium	aMedium( pStore );
         SdFilter*	pFilter = NULL;
 
         if( pStore->GetVersion() >= SOFFICE_FILEFORMAT_60 )
             pFilter = new SdXMLFilter( aMedium, *this, sal_True );
         else
             pFilter = new SdBINFilter( aMedium, *this, sal_True );
 
         UpdateDocInfoForSave();
 
         bRet = pFilter ? pFilter->Export() : FALSE;
         delete pFilter;
     }
 
     return bRet;
 }

BOOL SdDrawDocShell::SaveAs( SvStorage* pStore )
{
    if( GetCreateMode() == SFX_CREATE_MODE_STANDARD )
        SvInPlaceObject::SetVisArea( Rectangle() );

    UINT32	nVBWarning = ERRCODE_NONE;
    BOOL	bRet = SfxInPlaceObject::SaveAs( pStore );

    if( bRet )
    {
        SdFilter* pFilter = NULL;

        if( pStore->GetVersion() >= SOFFICE_FILEFORMAT_60 )
        {
            SfxMedium aMedium( pStore );
            pFilter = new SdXMLFilter( aMedium, *this, sal_True );

            UpdateDocInfoForSave();
            
            bRet = pFilter->Export();
        }
        else
        {

            SfxMedium aMedium( pStore );
            pFilter = new SdBINFilter( aMedium, *this, sal_True );

            UpdateDocInfoForSave();

            const ULONG	nOldSwapMode = pDoc->GetSwapGraphicsMode();
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

BOOL SdDrawDocShell::SaveCompleted( SvStorage * pStor )
{
    BOOL bRet = FALSE;

    if( SfxInPlaceObject::SaveCompleted(pStor) )
    {
        pDoc->NbcSetChanged( FALSE );

        bRet = TRUE;

        if( pDoc )
            pDoc->HandsOff();
    }
    return bRet;
}

void SdDrawDocShell::HandsOff()
{
    SfxInPlaceObject::HandsOff();

    if( pDoc )
        pDoc->HandsOff();
}

SdDrawDocument* SdDrawDocShell::GetDoc()
{
    return pDoc;
}

SfxStyleSheetBasePool* SdDrawDocShell::GetStyleSheetPool()
{
    return( (SfxStyleSheetBasePool*) pDoc->GetStyleSheetPool() );
}


BOOL SdDrawDocShell::SaveAsOwnFormat( SfxMedium& rMedium )
{

    const SfxFilter* pFilter = rMedium.GetFilter();

    if (pFilter->IsOwnTemplateFormat())
    {
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

void SdDrawDocShell::FillClass(SvGlobalName* pClassName,ULONG*  pFormat, String* pAppName, String* pFullTypeName, String* pShortTypeName, long    nFileFormat) const
{
    SfxInPlaceObject::FillClass(pClassName, pFormat, pAppName, pFullTypeName,
                                pShortTypeName, nFileFormat);

    if (nFileFormat == SOFFICE_FILEFORMAT_31)
    {
        *pClassName = SvGlobalName(BF_SO3_SIMPRESS_CLASSID_30);
        *pFormat = SOT_FORMATSTR_ID_STARDRAW;
        *pAppName = String(RTL_CONSTASCII_USTRINGPARAM("Sdraw 3.1"));
        *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_31));;
        *pShortTypeName = String(SdResId(STR_IMPRESS_DOCUMENT));
    }
    else if (nFileFormat == SOFFICE_FILEFORMAT_40)
    {
        *pClassName = SvGlobalName(BF_SO3_SIMPRESS_CLASSID_40);
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
                *pClassName = SvGlobalName(BF_SO3_SDRAW_CLASSID_50);
                *pFormat = SOT_FORMATSTR_ID_STARDRAW_50;
                *pFullTypeName = String(SdResId(STR_GRAPHIC_DOCUMENT_FULLTYPE_50));
            }
            else
            {
                *pClassName = SvGlobalName(BF_SO3_SIMPRESS_CLASSID_50);
                *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_50;
                *pFullTypeName = String(SdResId(STR_IMPRESS_DOCUMENT_FULLTYPE_50));
            }
        }
        else if (nFileFormat == SOFFICE_FILEFORMAT_CURRENT)
        {
            *pFullTypeName = String(SdResId( (eDocType == DOCUMENT_TYPE_DRAW) ?
                                              STR_GRAPHIC_DOCUMENT_FULLTYPE_60 : STR_IMPRESS_DOCUMENT_FULLTYPE_60 ));

                if(eDocType == DOCUMENT_TYPE_DRAW)
                {
                    *pClassName = SvGlobalName(BF_SO3_SDRAW_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARDRAW_60;
                }
                else
                {
                    *pClassName = SvGlobalName(BF_SO3_SIMPRESS_CLASSID_60);
                    *pFormat = SOT_FORMATSTR_ID_STARIMPRESS_60;
                }

        }

        *pShortTypeName = String(SdResId( (eDocType == DOCUMENT_TYPE_DRAW) ?
                                          STR_GRAPHIC_DOCUMENT : STR_IMPRESS_DOCUMENT ));
    }
}

OutputDevice* SdDrawDocShell::GetDocumentRefDev (void)
{
    OutputDevice* pReferenceDevice = SfxInPlaceObject::GetDocumentRefDev ();
    if (pReferenceDevice == NULL && pDoc != NULL)
        pReferenceDevice = pDoc->GetRefDevice ();
    return pReferenceDevice;
}

SfxPrinter* SdDrawDocShell::CreatePrinter( SvStream& rIn, SdDrawDocument& rDoc )
{
    SfxItemSet* pSet = new SfxItemSet( rDoc.GetPool(),
                    SID_PRINTER_NOTFOUND_WARN,	SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC,	SID_PRINTER_CHANGESTODOC,
                    ATTR_OPTIONS_PRINT, 		ATTR_OPTIONS_PRINT,
                    0 );
    // PrintOptionsSet setzen
    SdOptionsPrintItem aPrintItem(ATTR_OPTIONS_PRINT,SD_MOD()->GetSdOptions(rDoc.GetDocumentType()) );

    pSet->Put( aPrintItem );
    pSet->Put( SfxBoolItem( SID_PRINTER_NOTFOUND_WARN, false ) );

    return SfxPrinter::Create(rIn, pSet);
}

}
