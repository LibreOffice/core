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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/justifyitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/servicehelper.hxx>
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
#include "formulacell.hxx"
#include "printfun.hxx"
#include "docfunc.hxx"
#include "scmod.hxx"
#include "dragdata.hxx"
#include "clipdata.hxx"
#include "clipparam.hxx"

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
#include "stringutil.hxx"
#include "formulaiter.hxx"
#include <gridwin.hxx>

using namespace com::sun::star;

#define SCTRANS_TYPE_IMPEX          1
#define SCTRANS_TYPE_EDIT_RTF       2
#define SCTRANS_TYPE_EDIT_BIN       3
#define SCTRANS_TYPE_EMBOBJ         4

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
                                const ScRange& rBlock, bool bMetaFile )
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

ScTransferObj::ScTransferObj( ScDocument* pClipDoc, const TransferableObjectDescriptor& rDesc ) :
    pDoc( pClipDoc ),
    nNonFiltered(0),
    aObjDesc( rDesc ),
    nDragHandleX( 0 ),
    nDragHandleY( 0 ),
    nDragSourceFlags( 0 ),
    bDragWasInternal( false ),
    bUsedForLink( false ),
    bUseInApi( false )
{
    OSL_ENSURE(pDoc->IsClipboard(), "wrong document");

    // get aBlock from clipboard doc

    SCCOL nCol1;
    SCROW nRow1;
    SCCOL nCol2;
    SCROW nRow2;
    pDoc->GetClipStart( nCol1, nRow1 );
    pDoc->GetClipArea( nCol2, nRow2, true );    // real source area - include filtered rows
    nCol2 = sal::static_int_cast<SCCOL>( nCol2 + nCol1 );
    nRow2 = sal::static_int_cast<SCROW>( nRow2 + nRow1 );

    SCCOL nDummy;
    pDoc->GetClipArea( nDummy, nNonFiltered, false );
    bHasFiltered = (nNonFiltered < (nRow2 - nRow1));
    ++nNonFiltered;     // to get count instead of diff

    SCTAB nTab1=0;
    SCTAB nTab2=0;
    bool bFirst = true;
    for (SCTAB i=0; i< pDoc->GetTableCount(); i++)
        if (pDoc->HasTable(i))
        {
            if (bFirst)
                nTab1 = i;
            nTab2 = i;
            bFirst = false;
        }
    OSL_ENSURE(!bFirst, "no sheet selected");

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

ScTransferObj* ScTransferObj::GetOwnClipboard( vcl::Window* pUIWin )
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
    AddFormat( SOT_FORMATSTR_ID_PNG );
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

bool ScTransferObj::GetData( const datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    bool        bOK = false;

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
            ScAddress aPos(nCol, nRow, nTab);

            const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            ScTabEditEngine aEngine( *pPattern, pDoc->GetEditPool() );
            if (pDoc->GetCellType(aPos) == CELLTYPE_EDIT)
            {
                const EditTextObject* pObj = pDoc->GetEditText(aPos);
                if (pObj)
                    aEngine.SetText(*pObj);
            }
            else
            {
                OUString aText = pDoc->GetString(nCol, nRow, nTab);
                if (!aText.isEmpty())
                    aEngine.SetText(aText);
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
                bUsedForLink = true;

            bool bIncludeFiltered = pDoc->IsCutMode() || bUsedForLink;

            bool bReduceBlockFormat = nFormat == SOT_FORMATSTR_ID_HTML || nFormat == SOT_FORMAT_RTF;
            ScRange aReducedBlock = aBlock;
            if (bReduceBlockFormat && (aBlock.aEnd.Col() == MAXCOL || aBlock.aEnd.Row() == MAXROW) && aBlock.aStart.Tab() == aBlock.aEnd.Tab())
            {
                bool bShrunk = false;
                //shrink the area to allow pasting to external applications
                SCCOL aStartCol = aReducedBlock.aStart.Col();
                SCROW aStartRow = aReducedBlock.aStart.Row();
                SCCOL aEndCol = aReducedBlock.aEnd.Col();
                SCROW aEndRow = aReducedBlock.aEnd.Row();
                pDoc->ShrinkToUsedDataArea( bShrunk, aReducedBlock.aStart.Tab(), aStartCol, aStartRow, aEndCol, aEndRow, false);
                aReducedBlock = ScRange(aStartCol, aStartRow, aReducedBlock.aStart.Tab(), aEndCol, aEndRow, aReducedBlock.aEnd.Tab());
            }

            ScImportExport aObj( pDoc, aReducedBlock );
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

            if ( rFlavor.DataType.equals( ::cppu::UnoType<OUString>::get() ) )
            {
                OUString aString;
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
        else if ( nFormat == SOT_FORMAT_BITMAP || nFormat == SOT_FORMATSTR_ID_PNG )
        {
            Rectangle aMMRect = pDoc->GetMMRect( aBlock.aStart.Col(), aBlock.aStart.Row(),
                                                 aBlock.aEnd.Col(), aBlock.aEnd.Row(),
                                                 aBlock.aStart.Tab() );
            VirtualDevice aVirtDev;
            aVirtDev.SetOutputSizePixel( aVirtDev.LogicToPixel( aMMRect.GetSize(), MAP_100TH_MM ) );

            PaintToDev( &aVirtDev, pDoc, 1.0, aBlock, false );

            aVirtDev.SetMapMode( MapMode( MAP_PIXEL ) );
            Bitmap aBmp = aVirtDev.GetBitmap( Point(), aVirtDev.GetOutputSize() );
            bOK = SetBitmapEx( aBmp, rFlavor );
        }
        else if ( nFormat == SOT_FORMAT_GDIMETAFILE )
        {
            // #i123405# Do not limit visual size calculation for metafile creation.
            // It seems unlikely that removing the limitation causes problems since
            // metafile creation means that no real pixel device in the needed size is
            // created.
            InitDocShell(false);

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
            // #i123405# Do limit visual size calculation to PageSize
            InitDocShell(true);         // set aDocShellRef

            SfxObjectShell* pEmbObj = aDocShellRef;
            bOK = SetObject( pEmbObj, SCTRANS_TYPE_EMBOBJ, rFlavor );
        }
    }
    return bOK;
}

bool ScTransferObj::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const datatransfer::DataFlavor& rFlavor )
{
    // called from SetObject, put data into stream

    bool bRet = false;
    switch (nUserObjectId)
    {
        case SCTRANS_TYPE_IMPEX:
            {
                ScImportExport* pImpEx = (ScImportExport*)pUserObject;

                sal_uInt32 nFormat = SotExchange::GetFormat( rFlavor );
                // mba: no BaseURL for data exchange
                if ( pImpEx->ExportStream( *rxOStm, OUString(), nFormat ) )
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

                    sal_Int32 nParCnt = pEngine->GetParagraphCount();
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
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false, false );

                // mba: no relative URLs for clipboard!
                SfxMedium aMedium( xWorkStore, OUString() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    rxOStm->WriteStream( *pSrcStm );
                    delete pSrcStm;
                }

                bRet = true;

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
    bDragWasInternal = true;
}

void ScTransferObj::SetUseInApi( bool bSet )
{
    bUseInApi = bSet;
}

ScDocument* ScTransferObj::GetSourceDocument()
{
    ScDocShell* pSourceDocSh = GetSourceDocShell();
    if (pSourceDocSh)
        return &pSourceDocSh->GetDocument();
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

//  initialize aDocShellRef with a live document from the ClipDoc

// #i123405# added parameter to allow size calculation without limitation
// to PageSize, e.g. used for Metafile creation for clipboard.

void ScTransferObj::InitDocShell(bool bLimitToPageSize)
{
    if ( !aDocShellRef.Is() )
    {
        ScDocShell* pDocSh = new ScDocShell;
        aDocShellRef = pDocSh;      // ref must be there before InitNew

        pDocSh->DoInitNew(NULL);

        ScDocument& rDestDoc = pDocSh->GetDocument();
        ScMarkData aDestMark;
        aDestMark.SelectTable( 0, true );

        rDestDoc.SetDocOptions( pDoc->GetDocOptions() );   // #i42666#

        OUString aTabName;
        pDoc->GetName( aBlock.aStart.Tab(), aTabName );
        rDestDoc.RenameTab( 0, aTabName, false );          // no UpdateRef (empty)

        rDestDoc.CopyStdStylesFrom( pDoc );

        SCCOL nStartX = aBlock.aStart.Col();
        SCROW nStartY = aBlock.aStart.Row();
        SCCOL nEndX = aBlock.aEnd.Col();
        SCROW nEndY = aBlock.aEnd.Row();

        //  widths / heights
        //  (must be copied before CopyFromClip, for drawing objects)

        SCCOL nCol;
        SCTAB nSrcTab = aBlock.aStart.Tab();
        rDestDoc.SetLayoutRTL(0, pDoc->IsLayoutRTL(nSrcTab));
        for (nCol=nStartX; nCol<=nEndX; nCol++)
            if ( pDoc->ColHidden(nCol, nSrcTab) )
                rDestDoc.ShowCol( nCol, 0, false );
            else
                rDestDoc.SetColWidth( nCol, 0, pDoc->GetColWidth( nCol, nSrcTab ) );

        for (SCROW nRow = nStartY; nRow <= nEndY; ++nRow)
        {
            if ( pDoc->RowHidden(nRow, nSrcTab) )
                rDestDoc.ShowRow( nRow, 0, false );
            else
            {
                rDestDoc.SetRowHeight( nRow, 0, pDoc->GetOriginalHeight( nRow, nSrcTab ) );

                //  if height was set manually, that flag has to be copied, too
                bool bManual = pDoc->IsManualRowHeight(nRow, nSrcTab);
                rDestDoc.SetManualHeight(nRow, nRow, 0, bManual);
            }
        }

        if (pDoc->GetDrawLayer() || pDoc->HasNotes())
            pDocSh->MakeDrawLayer();

        //  cell range is copied to the original position, but on the first sheet
        //  -> bCutMode must be set
        //  pDoc is always a Clipboard-document

        ScRange aDestRange( nStartX,nStartY,0, nEndX,nEndY,0 );
        bool bWasCut = pDoc->IsCutMode();
        if (!bWasCut)
            pDoc->SetClipArea( aDestRange, true );          // Cut
        rDestDoc.CopyFromClip( aDestRange, aDestMark, IDF_ALL, NULL, pDoc, false );
        pDoc->SetClipArea( aDestRange, bWasCut );

        StripRefs( pDoc, nStartX,nStartY, nEndX,nEndY, &rDestDoc, 0,0 );

        ScRange aMergeRange = aDestRange;
        rDestDoc.ExtendMerge( aMergeRange, true );

        pDoc->CopyDdeLinks( &rDestDoc );         // copy values of DDE Links

        //  page format (grid etc) and page size (maximum size for ole object)

        Size aPaperSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );       // Twips
        ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
        OUString aStyleName = pDoc->GetPageStyle( aBlock.aStart.Tab() );
        SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
        if (pStyleSheet)
        {
            const SfxItemSet& rSourceSet = pStyleSheet->GetItemSet();
            aPaperSize = static_cast<const SvxSizeItem&>( rSourceSet.Get(ATTR_PAGE_SIZE)).GetSize();

            //  CopyStyleFrom kopiert SetItems mit richtigem Pool
            ScStyleSheetPool* pDestPool = rDestDoc.GetStyleSheetPool();
            pDestPool->CopyStyleFrom( pStylePool, aStyleName, SFX_STYLE_FAMILY_PAGE );
        }

        ScViewData aViewData( pDocSh, NULL );
        aViewData.SetScreen( nStartX,nStartY, nEndX,nEndY );
        aViewData.SetCurX( nStartX );
        aViewData.SetCurY( nStartY );

        rDestDoc.SetViewOptions( pDoc->GetViewOptions() );

        //      Size
        //! get while copying sizes

        long nPosX = 0;
        long nPosY = 0;

        for (nCol=0; nCol<nStartX; nCol++)
            nPosX += rDestDoc.GetColWidth( nCol, 0 );
        nPosY += rDestDoc.GetRowHeight( 0, nStartY-1, 0 );
        nPosX = (long) ( nPosX * HMM_PER_TWIPS );
        nPosY = (long) ( nPosY * HMM_PER_TWIPS );

        aPaperSize.Width()  *= 2;       // limit OLE object to double of page size
        aPaperSize.Height() *= 2;

        long nSizeX = 0;
        long nSizeY = 0;
        for (nCol=nStartX; nCol<=nEndX; nCol++)
        {
            long nAdd = rDestDoc.GetColWidth( nCol, 0 );
            if ( bLimitToPageSize && nSizeX+nAdd > aPaperSize.Width() && nSizeX )   // above limit?
                break;
            nSizeX += nAdd;
        }
        for (SCROW nRow=nStartY; nRow<=nEndY; nRow++)
        {
            long nAdd = rDestDoc.GetRowHeight( nRow, 0 );
            if ( bLimitToPageSize && nSizeY+nAdd > aPaperSize.Height() && nSizeY )  // above limit?
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

        pDocSh->UpdateOle(&aViewData, true);

        //! SetDocumentModified?
        if ( rDestDoc.IsChartListenerCollectionNeedsUpdate() )
            rDestDoc.UpdateChartListenerCollection();
    }
}

SfxObjectShell* ScTransferObj::SetDrawClipDoc( bool bAnyOle )
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
        OSL_ENSURE(nSubX==0&&nSubY==0, "can't move within the document");
    }

    //  In a clipboard doc the data don't have to be on the first sheet

    SCTAB nSrcTab = 0;
    while (nSrcTab<pDoc->GetTableCount() && !pDoc->HasTable(nSrcTab))
        ++nSrcTab;
    SCTAB nDestTab = 0;
    while (nDestTab<pDestDoc->GetTableCount() && !pDestDoc->HasTable(nDestTab))
        ++nDestTab;

    if (!pDoc->HasTable(nSrcTab) || !pDestDoc->HasTable(nDestTab))
    {
        OSL_FAIL("Sheet not found in ScTransferObj::StripRefs");
        return;
    }

    ScRange aRef;

    ScCellIterator aIter( pDoc, ScRange(nStartX, nStartY, nSrcTab, nEndX, nEndY, nSrcTab) );
    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
    {
        if (aIter.getType() != CELLTYPE_FORMULA)
            continue;

        ScFormulaCell* pFCell = aIter.getFormulaCell();
        bool bOut = false;
        ScDetectiveRefIter aRefIter( pFCell );
        while ( !bOut && aRefIter.GetNextRef( aRef ) )
        {
            if ( aRef.aStart.Tab() != nSrcTab || aRef.aEnd.Tab() != nSrcTab ||
                    aRef.aStart.Col() < nStartX || aRef.aEnd.Col() > nEndX ||
                    aRef.aStart.Row() < nStartY || aRef.aEnd.Row() > nEndY )
                bOut = true;
        }
        if (bOut)
        {
            SCCOL nCol = aIter.GetPos().Col() - nSubX;
            SCROW nRow = aIter.GetPos().Row() - nSubY;

            sal_uInt16 nErrCode = pFCell->GetErrCode();
            ScAddress aPos(nCol, nRow, nDestTab);
            if (nErrCode)
            {
                if ( static_cast<const SvxHorJustifyItem*>(pDestDoc->GetAttr(
                        nCol,nRow,nDestTab, ATTR_HOR_JUSTIFY))->GetValue() ==
                        SVX_HOR_JUSTIFY_STANDARD )
                    pDestDoc->ApplyAttr( nCol,nRow,nDestTab,
                            SvxHorJustifyItem(SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY) );

                ScSetStringParam aParam;
                aParam.setTextInput();
                pDestDoc->SetString(aPos, ScGlobal::GetErrorString(nErrCode), &aParam);
            }
            else if (pFCell->IsValue())
            {
                pDestDoc->SetValue(aPos, pFCell->GetValue());
            }
            else
            {
                OUString aStr = pFCell->GetString().getString();
                if ( pFCell->IsMultilineResult() )
                {
                    ScFieldEditEngine& rEngine = pDestDoc->GetEditEngine();
                    rEngine.SetText(aStr);
                    pDestDoc->SetEditText(ScAddress(nCol,nRow,nDestTab), rEngine.CreateTextObject());
                }
                else
                {
                    ScSetStringParam aParam;
                    aParam.setTextInput();
                    pDestDoc->SetString(aPos, aStr, &aParam);
                }
            }
        }
    }
}

namespace
{
    class theScTransferUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScTransferUnoTunnelId> {};
}

const com::sun::star::uno::Sequence< sal_Int8 >& ScTransferObj::getUnoTunnelId()
{
    return theScTransferUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL ScTransferObj::getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;
    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = reinterpret_cast< sal_Int64 >( this );
    }
    else
        nRet = TransferableHelper::getSomething(rId);
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
