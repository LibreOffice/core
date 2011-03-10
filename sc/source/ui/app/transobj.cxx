/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------



#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/justifyitem.hxx>


#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <sot/storage.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <osl/mutex.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>

#include "transobj.hxx"
#include "document.hxx"
#include "viewopti.hxx"
#include "editutil.hxx"
#include "impex.hxx"
#include "cell.hxx"
#include "printfun.hxx"
#include "docfunc.hxx"
#include "scmod.hxx"

// for InitDocShell
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/algitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include "docsh.hxx"
#include "markdata.hxx"
#include "stlpool.hxx"
#include "viewdata.hxx"
#include "dociter.hxx"
#include "cellsuno.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

#define SCTRANS_TYPE_IMPEX          1
#define SCTRANS_TYPE_EDIT_RTF       2
#define SCTRANS_TYPE_EDIT_BIN       3
#define SCTRANS_TYPE_EMBOBJ         4

// -----------------------------------------------------------------------

void ScTransferObj::GetAreaSize( ScDocument* pDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol )
{
    SCCOL nMaxCol = 0;
    SCROW nMaxRow = 0;
    for( SCTAB nTab = nTab1; nTab <= nTab2; nTab++ )
    {
        SCCOL nLastCol = 0;
        SCROW nLastRow = 0;
        // GetPrintArea instead of GetCellArea - include drawing objects
        if( pDoc->GetPrintArea( nTab, nLastCol, nLastRow ) )
        {
            if( nLastCol > nMaxCol )
                nMaxCol = nLastCol;
            if( nLastRow > nMaxRow  )
                nMaxRow = nLastRow;
        }
    }
    nRow = nMaxRow;
    nCol = nMaxCol;
}

void ScTransferObj::PaintToDev( OutputDevice* pDev, ScDocument* pDoc, double nPrintFactor,
                                const ScRange& rBlock, sal_Bool bMetaFile )
{
    if (!pDoc)
        return;

    Point aPoint;
    Rectangle aBound( aPoint, pDev->GetOutputSize() );      //! use size from clip area?

    ScViewData aViewData(NULL,NULL);
    aViewData.InitData( pDoc );

    aViewData.SetTabNo( rBlock.aEnd.Tab() );
    aViewData.SetScreen( rBlock.aStart.Col(), rBlock.aStart.Row(),
                            rBlock.aEnd.Col(), rBlock.aEnd.Row() );

    ScPrintFunc::DrawToDev( pDoc, pDev, nPrintFactor, aBound, &aViewData, bMetaFile );
}

// -----------------------------------------------------------------------

ScTransferObj::ScTransferObj( ScDocument* pClipDoc, const TransferableObjectDescriptor& rDesc ) :
    pDoc( pClipDoc ),
    aObjDesc( rDesc ),
    nDragHandleX( 0 ),
    nDragHandleY( 0 ),
    nDragSourceFlags( 0 ),
    bDragWasInternal( false ),
    bUsedForLink( false ),
    bUseInApi( false )
{
    DBG_ASSERT(pDoc->IsClipboard(), "wrong document");

    //
    // get aBlock from clipboard doc
    //

    SCCOL nCol1;
    SCROW nRow1;
    SCCOL nCol2;
    SCROW nRow2;
    pDoc->GetClipStart( nCol1, nRow1 );
    pDoc->GetClipArea( nCol2, nRow2, sal_True );    // real source area - include filtered rows
    nCol2 = sal::static_int_cast<SCCOL>( nCol2 + nCol1 );
    nRow2 = sal::static_int_cast<SCROW>( nRow2 + nRow1 );

    SCCOL nDummy;
    pDoc->GetClipArea( nDummy, nNonFiltered, false );
    bHasFiltered = (nNonFiltered < (nRow2 - nRow1));
    ++nNonFiltered;     // to get count instead of diff

    SCTAB nTab1=0;
    SCTAB nTab2=0;
    sal_Bool bFirst = sal_True;
    for (SCTAB i=0; i<=MAXTAB; i++)
        if (pDoc->HasTable(i))
        {
            if (bFirst)
                nTab1 = i;
            nTab2 = i;
            bFirst = false;
        }
    DBG_ASSERT(!bFirst, "no sheet selected");

    //  only limit to used cells if whole sheet was marked
    //  (so empty cell areas can be copied)
    if ( nCol2>=MAXCOL && nRow2>=MAXROW )
    {
        SCROW nMaxRow;
        SCCOL nMaxCol;
        GetAreaSize( pDoc, nTab1, nTab2, nMaxRow, nMaxCol );
        if( nMaxRow < nRow2 )
            nRow2 = nMaxRow;
        if( nMaxCol < nCol2 )
            nCol2 = nMaxCol;
    }

    aBlock = ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    nVisibleTab = nTab1;    // valid table as default

    Rectangle aMMRect = pDoc->GetMMRect( nCol1,nRow1, nCol2,nRow2, nTab1 );
    aObjDesc.maSize = aMMRect.GetSize();
    PrepareOLE( aObjDesc );
}

ScTransferObj::~ScTransferObj()
{
    SolarMutexGuard aSolarGuard;

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetClipData().pCellClipboard == this )
    {
        OSL_FAIL("ScTransferObj wasn't released");
        pScMod->SetClipObject( NULL, NULL );
    }
    if ( pScMod->GetDragData().pCellTransfer == this )
    {
        OSL_FAIL("ScTransferObj wasn't released");
        pScMod->ResetDragObject();
    }

    delete pDoc;        // ScTransferObj is owner of clipboard document

    aDocShellRef.Clear();   // before releasing the mutex

    aDrawPersistRef.Clear();                    // after the model

}

ScTransferObj* ScTransferObj::GetOwnClipboard( Window* pUIWin )
{
    ScTransferObj* pObj = SC_MOD()->GetClipData().pCellClipboard;
    if ( pObj && pUIWin )
    {
        //  check formats to see if pObj is really in the system clipboard

        //  pUIWin is NULL when called from core (IsClipboardSource),
        //  in that case don't access the system clipboard, because the call
        //  may be from other clipboard operations (like flushing, #86059#)

        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard( pUIWin ) );
        if ( !aDataHelper.HasFormat( SOT_FORMATSTR_ID_DIF ) )
        {
//          OSL_FAIL("ScTransferObj wasn't released");
            pObj = NULL;
        }
    }
    return pObj;
}

void ScTransferObj::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
    AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
    AddFormat( SOT_FORMAT_GDIMETAFILE );
    AddFormat( SOT_FORMAT_BITMAP );

    // ScImportExport formats
    AddFormat( SOT_FORMATSTR_ID_HTML );
    AddFormat( SOT_FORMATSTR_ID_SYLK );
    AddFormat( SOT_FORMATSTR_ID_LINK );
    AddFormat( SOT_FORMATSTR_ID_DIF );
    AddFormat( SOT_FORMAT_STRING );

    AddFormat( SOT_FORMAT_RTF );
    if ( aBlock.aStart == aBlock.aEnd )
        AddFormat( SOT_FORMATSTR_ID_EDITENGINE );
}

sal_Bool ScTransferObj::GetData( const datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bOK = false;

    if( HasFormat( nFormat ) )
    {
        if ( nFormat == SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR || nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
        {
            bOK = SetTransferableObjectDescriptor( aObjDesc, rFlavor );
        }
        else if ( ( nFormat == SOT_FORMAT_RTF || nFormat == SOT_FORMATSTR_ID_EDITENGINE ) &&
                        aBlock.aStart == aBlock.aEnd )
        {
            //  RTF from a single cell is handled by EditEngine

            SCCOL nCol = aBlock.aStart.Col();
            SCROW nRow = aBlock.aStart.Row();
            SCTAB nTab = aBlock.aStart.Tab();

            const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            ScTabEditEngine aEngine( *pPattern, pDoc->GetEditPool() );
            ScBaseCell* pCell = NULL;
            pDoc->GetCell( nCol, nRow, nTab, pCell );
            if (pCell)
            {
                if (pCell->GetCellType() == CELLTYPE_EDIT)
                {
                    const EditTextObject* pObj;
                    ((ScEditCell*)pCell)->GetData(pObj);
                    aEngine.SetText( *pObj );
                }
                else
                {
                    String aText;
                    pDoc->GetString( nCol, nRow, nTab, aText );
                    aEngine.SetText(aText);
                }
            }

            bOK = SetObject( &aEngine,
                            (nFormat == FORMAT_RTF) ? SCTRANS_TYPE_EDIT_RTF : SCTRANS_TYPE_EDIT_BIN,
                            rFlavor );
        }
        else if ( ScImportExport::IsFormatSupported( nFormat ) || nFormat == SOT_FORMAT_RTF )
        {
            //  if this transfer object was used to create a DDE link, filtered rows
            //  have to be included for subsequent calls (to be consistent with link data)
            if ( nFormat == SOT_FORMATSTR_ID_LINK )
                bUsedForLink = sal_True;

            sal_Bool bIncludeFiltered = pDoc->IsCutMode() || bUsedForLink;

            ScImportExport aObj( pDoc, aBlock );
            ScExportTextOptions aTextOptions(ScExportTextOptions::None, 0, true);
            if ( bUsedForLink )
            {
                // For a DDE link, convert line breaks and separators to space.
                aTextOptions.meNewlineConversion = ScExportTextOptions::ToSpace;
                aTextOptions.mcSeparatorConvertTo = ' ';
                aTextOptions.mbAddQuotes = false;
            }
            aObj.SetExportTextOptions(aTextOptions);
            aObj.SetFormulas( pDoc->GetViewOptions().GetOption( VOPT_FORMULAS ) );
            aObj.SetIncludeFiltered( bIncludeFiltered );

            //  DataType depends on format type:

            if ( rFlavor.DataType.equals( ::getCppuType( (const ::rtl::OUString*) 0 ) ) )
            {
                rtl::OUString aString;
                if ( aObj.ExportString( aString, nFormat ) )
                    bOK = SetString( aString, rFlavor );
            }
            else if ( rFlavor.DataType.equals( ::getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) ) )
            {
                //  SetObject converts a stream into a Int8-Sequence
                bOK = SetObject( &aObj, SCTRANS_TYPE_IMPEX, rFlavor );
            }
            else
            {
                OSL_FAIL("unknown DataType");
            }
        }
        else if ( nFormat == SOT_FORMAT_BITMAP )
        {
            Rectangle aMMRect = pDoc->GetMMRect( aBlock.aStart.Col(), aBlock.aStart.Row(),
                                                 aBlock.aEnd.Col(), aBlock.aEnd.Row(),
                                                 aBlock.aStart.Tab() );
            VirtualDevice aVirtDev;
            aVirtDev.SetOutputSizePixel( aVirtDev.LogicToPixel( aMMRect.GetSize(), MAP_100TH_MM ) );

            PaintToDev( &aVirtDev, pDoc, 1.0, aBlock, false );

            aVirtDev.SetMapMode( MapMode( MAP_PIXEL ) );
            Bitmap aBmp = aVirtDev.GetBitmap( Point(), aVirtDev.GetOutputSize() );
            bOK = SetBitmap( aBmp, rFlavor );
        }
        else if ( nFormat == SOT_FORMAT_GDIMETAFILE )
        {
            InitDocShell();
            SfxObjectShell* pEmbObj = aDocShellRef;

            // like SvEmbeddedTransfer::GetData:

            GDIMetaFile     aMtf;
            VirtualDevice   aVDev;
            MapMode         aMapMode( pEmbObj->GetMapUnit() );
            Rectangle       aVisArea( pEmbObj->GetVisArea( ASPECT_CONTENT ) );

            aVDev.EnableOutput( false );
            aVDev.SetMapMode( aMapMode );
            aMtf.SetPrefSize( aVisArea.GetSize() );
            aMtf.SetPrefMapMode( aMapMode );
            aMtf.Record( &aVDev );

            pEmbObj->DoDraw( &aVDev, Point(), aVisArea.GetSize(), JobSetup(), ASPECT_CONTENT );

            aMtf.Stop();
            aMtf.WindStart();

            bOK = SetGDIMetaFile( aMtf, rFlavor );
        }
        else if ( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
        {
            //TODO/LATER: differentiate between formats?!
            InitDocShell();         // set aDocShellRef

            SfxObjectShell* pEmbObj = aDocShellRef;
            bOK = SetObject( pEmbObj, SCTRANS_TYPE_EMBOBJ, rFlavor );
        }
    }
    return bOK;
}

sal_Bool ScTransferObj::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const datatransfer::DataFlavor& rFlavor )
{
    // called from SetObject, put data into stream

    sal_Bool bRet = false;
    switch (nUserObjectId)
    {
        case SCTRANS_TYPE_IMPEX:
            {
                ScImportExport* pImpEx = (ScImportExport*)pUserObject;

                sal_uInt32 nFormat = SotExchange::GetFormat( rFlavor );
                // mba: no BaseURL for data exchange
                if ( pImpEx->ExportStream( *rxOStm, String(), nFormat ) )
                    bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        case SCTRANS_TYPE_EDIT_RTF:
        case SCTRANS_TYPE_EDIT_BIN:
            {
                ScTabEditEngine* pEngine = (ScTabEditEngine*)pUserObject;
                if ( nUserObjectId == SCTRANS_TYPE_EDIT_RTF )
                {
                    pEngine->Write( *rxOStm, EE_FORMAT_RTF );
                    bRet = ( rxOStm->GetError() == ERRCODE_NONE );
                }
                else
                {
                    //  can't use Write for EditEngine format because that would
                    //  write old format without support for unicode characters.
                    //  Get the data from the EditEngine's transferable instead.

                    sal_uInt16 nParCnt = pEngine->GetParagraphCount();
                    if ( nParCnt == 0 )
                        nParCnt = 1;
                    ESelection aSel( 0, 0, nParCnt-1, pEngine->GetTextLen(nParCnt-1) );

                    uno::Reference<datatransfer::XTransferable> xEditTrans = pEngine->CreateTransferable( aSel );
                    TransferableDataHelper aEditHelper( xEditTrans );

                    bRet = aEditHelper.GetSotStorageStream( rFlavor, rxOStm );
                }
            }
            break;

        case SCTRANS_TYPE_EMBOBJ:
            {
                // TODO/MBA: testing
                SfxObjectShell*   pEmbObj = (SfxObjectShell*) pUserObject;
                ::utl::TempFile     aTempFile;
                aTempFile.EnableKillingFile();
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false );

                // mba: no relative ULRs for clipboard!
                SfxMedium aMedium( xWorkStore, String() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    *rxOStm << *pSrcStm;
                    delete pSrcStm;
                }

                bRet = sal_True;

                xWorkStore->dispose();
                xWorkStore = uno::Reference < embed::XStorage >();
                rxOStm->Commit();
            }
            break;

        default:
            OSL_FAIL("unknown object id");
    }
    return bRet;
}

void ScTransferObj::ObjectReleased()
{
    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetClipData().pCellClipboard == this )
        pScMod->SetClipObject( NULL, NULL );

    TransferableHelper::ObjectReleased();
}

void ScTransferObj::DragFinished( sal_Int8 nDropAction )
{
    if ( nDropAction == DND_ACTION_MOVE && !bDragWasInternal && !(nDragSourceFlags & SC_DROP_NAVIGATOR) )
    {
        //  move: delete source data
        ScDocShell* pSourceSh = GetSourceDocShell();
        if (pSourceSh)
        {
            ScMarkData aMarkData = GetSourceMarkData();
            //  external drag&drop doesn't copy objects, so they also aren't deleted:
            //  bApi=TRUE, don't show error messages from drag&drop
            pSourceSh->GetDocFunc().DeleteContents( aMarkData, IDF_ALL & ~IDF_OBJECTS, true, true );
        }
    }

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetDragData().pCellTransfer == this )
        pScMod->ResetDragObject();

    xDragSourceRanges = NULL;       // don't keep source after dropping

    TransferableHelper::DragFinished( nDropAction );
}

void ScTransferObj::SetDragHandlePos( SCCOL nX, SCROW nY )
{
    nDragHandleX = nX;
    nDragHandleY = nY;
}

void ScTransferObj::SetVisibleTab( SCTAB nNew )
{
    nVisibleTab = nNew;
}

void ScTransferObj::SetDrawPersist( const SfxObjectShellRef& rRef )
{
    aDrawPersistRef = rRef;
}

void ScTransferObj::SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark )
{
    ScRangeList aRanges;
    rMark.FillRangeListWithMarks( &aRanges, false );
    xDragSourceRanges = new ScCellRangesObj( pSourceShell, aRanges );
}

void ScTransferObj::SetDragSourceFlags( sal_uInt16 nFlags )
{
    nDragSourceFlags = nFlags;
}

void ScTransferObj::SetDragWasInternal()
{
    bDragWasInternal = sal_True;
}

void ScTransferObj::SetUseInApi( bool bSet )
{
    bUseInApi = bSet;
}

ScDocument* ScTransferObj::GetSourceDocument()
{
    ScDocShell* pSourceDocSh = GetSourceDocShell();
    if (pSourceDocSh)
        return pSourceDocSh->GetDocument();
    return NULL;
}

ScDocShell* ScTransferObj::GetSourceDocShell()
{
    ScCellRangesBase* pRangesObj = ScCellRangesBase::getImplementation( xDragSourceRanges );
    if (pRangesObj)
        return pRangesObj->GetDocShell();

    return NULL;    // none set
}

ScMarkData ScTransferObj::GetSourceMarkData()
{
    ScMarkData aMarkData;
    ScCellRangesBase* pRangesObj = ScCellRangesBase::getImplementation( xDragSourceRanges );
    if (pRangesObj)
    {
        const ScRangeList& rRanges = pRangesObj->GetRangeList();
        aMarkData.MarkFromRangeList( rRanges, false );
    }
    return aMarkData;
}

//
//  initialize aDocShellRef with a live document from the ClipDoc
//

void ScTransferObj::InitDocShell()
{
    if ( !aDocShellRef.Is() )
    {
        ScDocShell* pDocSh = new ScDocShell;
        aDocShellRef = pDocSh;      // ref must be there before InitNew

        pDocSh->DoInitNew(NULL);

        ScDocument* pDestDoc = pDocSh->GetDocument();
        ScMarkData aDestMark;
        aDestMark.SelectTable( 0, sal_True );

        pDestDoc->SetDocOptions( pDoc->GetDocOptions() );   // #i42666#

        String aTabName;
        pDoc->GetName( aBlock.aStart.Tab(), aTabName );
        pDestDoc->RenameTab( 0, aTabName, false );          // no UpdateRef (empty)

        pDestDoc->CopyStdStylesFrom( pDoc );

        SCCOL nStartX = aBlock.aStart.Col();
        SCROW nStartY = aBlock.aStart.Row();
        SCCOL nEndX = aBlock.aEnd.Col();
        SCROW nEndY = aBlock.aEnd.Row();

        //  widths / heights
        //  (must be copied before CopyFromClip, for drawing objects)

        SCCOL nCol;
        SCTAB nSrcTab = aBlock.aStart.Tab();
        pDestDoc->SetLayoutRTL(0, pDoc->IsLayoutRTL(nSrcTab));
        for (nCol=nStartX; nCol<=nEndX; nCol++)
            if ( pDoc->ColHidden(nCol, nSrcTab) )
                pDestDoc->ShowCol( nCol, 0, false );
            else
                pDestDoc->SetColWidth( nCol, 0, pDoc->GetColWidth( nCol, nSrcTab ) );

        ScBitMaskCompressedArray< SCROW, sal_uInt8> & rDestRowFlags =
            pDestDoc->GetRowFlagsArrayModifiable(0);

        for (SCROW nRow = nStartY; nRow <= nEndY; ++nRow)
        {
            sal_uInt8 nSourceFlags = pDoc->GetRowFlags(nRow, nSrcTab);
            if ( pDoc->RowHidden(nRow, nSrcTab) )
                pDestDoc->ShowRow( nRow, 0, false );
            else
            {
                pDestDoc->SetRowHeight( nRow, 0, pDoc->GetOriginalHeight( nRow, nSrcTab ) );

                //  if height was set manually, that flag has to be copied, too
                if ( nSourceFlags & CR_MANUALSIZE )
                    rDestRowFlags.OrValue( nRow, CR_MANUALSIZE);
            }
        }

        if ( pDoc->GetDrawLayer() )
            pDocSh->MakeDrawLayer();

        //  cell range is copied to the original position, but on the first sheet
        //  -> bCutMode must be set
        //  pDoc is always a Clipboard-document

        ScRange aDestRange( nStartX,nStartY,0, nEndX,nEndY,0 );
        sal_Bool bWasCut = pDoc->IsCutMode();
        if (!bWasCut)
            pDoc->SetClipArea( aDestRange, sal_True );          // Cut
        pDestDoc->CopyFromClip( aDestRange, aDestMark, IDF_ALL, NULL, pDoc, false );
        pDoc->SetClipArea( aDestRange, bWasCut );

        StripRefs( pDoc, nStartX,nStartY, nEndX,nEndY, pDestDoc, 0,0 );

        ScRange aMergeRange = aDestRange;
        pDestDoc->ExtendMerge( aMergeRange, sal_True );

        pDoc->CopyDdeLinks( pDestDoc );         // copy values of DDE Links

        //  page format (grid etc) and page size (maximum size for ole object)

        Size aPaperSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );       // Twips
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
        String aStyleName = pDoc->GetPageStyle( aBlock.aStart.Tab() );
        SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
        if (pStyleSheet)
        {
            const SfxItemSet& rSourceSet = pStyleSheet->GetItemSet();
            aPaperSize = ((const SvxSizeItem&) rSourceSet.Get(ATTR_PAGE_SIZE)).GetSize();

            //  CopyStyleFrom kopiert SetItems mit richtigem Pool
            ScStyleSheetPool* pDestPool = pDestDoc->GetStyleSheetPool();
            pDestPool->CopyStyleFrom( pStylePool, aStyleName, SFX_STYLE_FAMILY_PAGE );
        }

        ScViewData aViewData( pDocSh, NULL );
        aViewData.SetScreen( nStartX,nStartY, nEndX,nEndY );
        aViewData.SetCurX( nStartX );
        aViewData.SetCurY( nStartY );

        pDestDoc->SetViewOptions( pDoc->GetViewOptions() );

        //      Size
        //! get while copying sizes

        long nPosX = 0;
        long nPosY = 0;

        for (nCol=0; nCol<nStartX; nCol++)
            nPosX += pDestDoc->GetColWidth( nCol, 0 );
        nPosY += pDestDoc->GetRowHeight( 0, nStartY-1, 0 );
        nPosX = (long) ( nPosX * HMM_PER_TWIPS );
        nPosY = (long) ( nPosY * HMM_PER_TWIPS );


        aPaperSize.Width()  *= 2;       // limit OLE object to double of page size
        aPaperSize.Height() *= 2;

        long nSizeX = 0;
        long nSizeY = 0;
        for (nCol=nStartX; nCol<=nEndX; nCol++)
        {
            long nAdd = pDestDoc->GetColWidth( nCol, 0 );
            if ( nSizeX+nAdd > aPaperSize.Width() && nSizeX )   // above limit?
                break;
            nSizeX += nAdd;
        }
        for (SCROW nRow=nStartY; nRow<=nEndY; nRow++)
        {
            long nAdd = pDestDoc->GetRowHeight( nRow, 0 );
            if ( nSizeY+nAdd > aPaperSize.Height() && nSizeY )  // above limit?
                break;
            nSizeY += nAdd;
        }
        nSizeX = (long) ( nSizeX * HMM_PER_TWIPS );
        nSizeY = (long) ( nSizeY * HMM_PER_TWIPS );

//      pDocSh->SetVisAreaSize( Size(nSizeX,nSizeY) );

        Rectangle aNewArea( Point(nPosX,nPosY), Size(nSizeX,nSizeY) );
        //TODO/LATER: why twice?!
        //pDocSh->SvInPlaceObject::SetVisArea( aNewArea );
        pDocSh->SetVisArea( aNewArea );

        pDocSh->UpdateOle(&aViewData, sal_True);

        //! SetDocumentModified?
        if ( pDestDoc->IsChartListenerCollectionNeedsUpdate() )
            pDestDoc->UpdateChartListenerCollection();
    }
}

SfxObjectShell* ScTransferObj::SetDrawClipDoc( sal_Bool bAnyOle )
{
    // update ScGlobal::pDrawClipDocShellRef

    delete ScGlobal::pDrawClipDocShellRef;
    if (bAnyOle)
    {
        ScGlobal::pDrawClipDocShellRef =
                        new ScDocShellRef(new ScDocShell(SFX_CREATE_MODE_INTERNAL));      // there must be a ref
        (*ScGlobal::pDrawClipDocShellRef)->DoInitNew(NULL);
        return *ScGlobal::pDrawClipDocShellRef;
    }
    else
    {
        ScGlobal::pDrawClipDocShellRef = NULL;
        return NULL;
    }
}

void ScTransferObj::StripRefs( ScDocument* pDoc,
                    SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                    ScDocument* pDestDoc, SCCOL nSubX, SCROW nSubY )
{
    if (!pDestDoc)
    {
        pDestDoc = pDoc;
        DBG_ASSERT(nSubX==0&&nSubY==0, "can't move within the document");
    }

    //  In a clipboard doc the data don't have to be on the first sheet

    SCTAB nSrcTab = 0;
    while (nSrcTab<MAXTAB && !pDoc->HasTable(nSrcTab))
        ++nSrcTab;
    SCTAB nDestTab = 0;
    while (nDestTab<MAXTAB && !pDestDoc->HasTable(nDestTab))
        ++nDestTab;

    if (!pDoc->HasTable(nSrcTab) || !pDestDoc->HasTable(nDestTab))
    {
        OSL_FAIL("Sheet not found in ScTransferObj::StripRefs");
        return;
    }

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    ScRange aRef;

    ScCellIterator aIter( pDoc, nStartX, nStartY, nSrcTab, nEndX, nEndY, nSrcTab );
    ScBaseCell* pCell = aIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
            sal_Bool bOut = false;
            ScDetectiveRefIter aRefIter( pFCell );
            while ( !bOut && aRefIter.GetNextRef( aRef ) )
            {
                if ( aRef.aStart.Tab() != nSrcTab || aRef.aEnd.Tab() != nSrcTab ||
                        aRef.aStart.Col() < nStartX || aRef.aEnd.Col() > nEndX ||
                        aRef.aStart.Row() < nStartY || aRef.aEnd.Row() > nEndY )
                    bOut = sal_True;
            }
            if (bOut)
            {
                SCCOL nCol = aIter.GetCol() - nSubX;
                SCROW nRow = aIter.GetRow() - nSubY;

                ScBaseCell* pNew = 0;
                sal_uInt16 nErrCode = pFCell->GetErrCode();
                if (nErrCode)
                {
                    pNew = new ScStringCell( ScGlobal::GetErrorString(nErrCode) );
                    if ( ((const SvxHorJustifyItem*) pDestDoc->GetAttr(
                            nCol,nRow,nDestTab, ATTR_HOR_JUSTIFY))->GetValue() ==
                            SVX_HOR_JUSTIFY_STANDARD )
                        pDestDoc->ApplyAttr( nCol,nRow,nDestTab,
                                SvxHorJustifyItem(SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY) );
                }
                else if (pFCell->IsValue())
                {
                    double fVal = pFCell->GetValue();
                    pNew = new ScValueCell( fVal );
                }
                else
                {
                    String aStr;
                    pFCell->GetString(aStr);
                    if ( pFCell->IsMultilineResult() )
                        pNew = new ScEditCell( aStr, pDestDoc );
                    else
                        pNew = new ScStringCell( aStr );
                }
                pDestDoc->PutCell( nCol,nRow,nDestTab, pNew );

                //  number formats

                sal_uLong nOldFormat = ((const SfxUInt32Item*)
                                pDestDoc->GetAttr(nCol,nRow,nDestTab, ATTR_VALUE_FORMAT))->GetValue();
                if ( (nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                {
                    sal_uLong nNewFormat = pFCell->GetStandardFormat( *pFormatter,
                        nOldFormat );
                    pDestDoc->ApplyAttr( nCol,nRow,nDestTab,
                                SfxUInt32Item(ATTR_VALUE_FORMAT, nNewFormat) );
                }
            }
        }
        pCell = aIter.GetNext();
    }
}

const com::sun::star::uno::Sequence< sal_Int8 >& ScTransferObj::getUnoTunnelId()
{
    static com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    if( !aSeq.getLength() )
    {
        static osl::Mutex           aCreateMutex;
        osl::Guard< osl::Mutex >    aGuard( aCreateMutex );
        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }
    return aSeq;
}

sal_Int64 SAL_CALL ScTransferObj::getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;
    if( ( rId.getLength() == 16 ) &&
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = reinterpret_cast< sal_Int64 >( this );
    }
    else
        nRet = TransferableHelper::getSomething(rId);
    return nRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
