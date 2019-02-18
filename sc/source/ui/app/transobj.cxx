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

#include <scitems.hxx>
#include <editeng/justifyitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/storagehelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <sot/storage.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/docfile.hxx>

#include <transobj.hxx>
#include <patattr.hxx>
#include <cellvalue.hxx>
#include <cellform.hxx>
#include <document.hxx>
#include <viewopti.hxx>
#include <editutil.hxx>
#include <impex.hxx>
#include <formulacell.hxx>
#include <printfun.hxx>
#include <docfunc.hxx>
#include <scmod.hxx>
#include <dragdata.hxx>

#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <formula/errorcodes.hxx>
#include <docsh.hxx>
#include <markdata.hxx>
#include <stlpool.hxx>
#include <viewdata.hxx>
#include <dociter.hxx>
#include <cellsuno.hxx>
#include <stringutil.hxx>
#include <formulaiter.hxx>

using namespace com::sun::star;

constexpr sal_uInt32 SCTRANS_TYPE_IMPEX              = 1;
constexpr sal_uInt32 SCTRANS_TYPE_EDIT_RTF           = 2;
constexpr sal_uInt32 SCTRANS_TYPE_EDIT_BIN           = 3;
constexpr sal_uInt32 SCTRANS_TYPE_EMBOBJ             = 4;
constexpr sal_uInt32 SCTRANS_TYPE_EDIT_ODF_TEXT_FLAT = 5;

void ScTransferObj::GetAreaSize( const ScDocument* pDoc, SCTAB nTab1, SCTAB nTab2, SCROW& nRow, SCCOL& nCol )
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
                                const ScRange& rBlock )
{
    if (!pDoc)
        return;

    tools::Rectangle aBound( Point(), pDev->GetOutputSize() );      //! use size from clip area?

    ScViewData aViewData(nullptr,nullptr);
    aViewData.InitData( pDoc );

    aViewData.SetTabNo( rBlock.aEnd.Tab() );
    aViewData.SetScreen( rBlock.aStart.Col(), rBlock.aStart.Row(),
                            rBlock.aEnd.Col(), rBlock.aEnd.Row() );

    ScPrintFunc::DrawToDev( pDoc, pDev, nPrintFactor, aBound, &aViewData, false/*bMetaFile*/ );
}

ScTransferObj::ScTransferObj( ScDocumentUniquePtr pClipDoc, const TransferableObjectDescriptor& rDesc ) :
    m_pDoc( std::move(pClipDoc ) ),
    m_nNonFiltered(0),
    m_aObjDesc( rDesc ),
    m_nDragHandleX( 0 ),
    m_nDragHandleY( 0 ),
    m_nSourceCursorX( MAXCOL + 1 ),
    m_nSourceCursorY( MAXROW + 1 ),
    m_nDragSourceFlags( ScDragSrc::Undefined ),
    m_bDragWasInternal( false ),
    m_bUsedForLink( false ),
    m_bUseInApi( false )
{
    OSL_ENSURE(m_pDoc->IsClipboard(), "wrong document");

    // get aBlock from clipboard doc

    SCCOL nCol1;
    SCROW nRow1;
    SCCOL nCol2;
    SCROW nRow2;
    m_pDoc->GetClipStart( nCol1, nRow1 );
    m_pDoc->GetClipArea( nCol2, nRow2, true );    // real source area - include filtered rows
    nCol2 = sal::static_int_cast<SCCOL>( nCol2 + nCol1 );
    nRow2 = sal::static_int_cast<SCROW>( nRow2 + nRow1 );

    SCCOL nDummy;
    m_pDoc->GetClipArea( nDummy, m_nNonFiltered, false );
    m_bHasFiltered = (m_nNonFiltered < (nRow2 - nRow1));
    ++m_nNonFiltered;     // to get count instead of diff

    SCTAB nTab1=0;
    SCTAB nTab2=0;
    bool bFirst = true;
    for (SCTAB i=0; i< m_pDoc->GetTableCount(); i++)
        if (m_pDoc->HasTable(i))
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
        GetAreaSize( m_pDoc.get(), nTab1, nTab2, nMaxRow, nMaxCol );
        if( nMaxRow < nRow2 )
            nRow2 = nMaxRow;
        if( nMaxCol < nCol2 )
            nCol2 = nMaxCol;
    }

    m_aBlock = ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    m_nVisibleTab = nTab1;    // valid table as default

    tools::Rectangle aMMRect = m_pDoc->GetMMRect( nCol1,nRow1, nCol2,nRow2, nTab1 );
    m_aObjDesc.maSize = aMMRect.GetSize();
    PrepareOLE( m_aObjDesc );
}

ScTransferObj::~ScTransferObj()
{
    SolarMutexGuard aSolarGuard;

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetDragData().pCellTransfer == this )
    {
        OSL_FAIL("ScTransferObj wasn't released");
        pScMod->ResetDragObject();
    }

    m_pDoc.reset();        // ScTransferObj is owner of clipboard document

    m_aDocShellRef.clear();   // before releasing the mutex

    m_aDrawPersistRef.clear();                    // after the model

}

ScTransferObj* ScTransferObj::GetOwnClipboard(const uno::Reference<datatransfer::XTransferable2>& xTransferable)
{
    ScTransferObj* pObj = nullptr;
    if (xTransferable.is())
    {
        uno::Reference<XUnoTunnel> xTunnel( xTransferable, uno::UNO_QUERY );
        if ( xTunnel.is() )
        {
            sal_Int64 nHandle = xTunnel->getSomething( getUnoTunnelId() );
            if ( nHandle )
                pObj = dynamic_cast<ScTransferObj*>(reinterpret_cast<TransferableHelper*>( static_cast<sal_IntPtr>(nHandle) ));
        }
    }

    return pObj;
}

void ScTransferObj::AddSupportedFormats()
{
    AddFormat( SotClipboardFormatId::EMBED_SOURCE );
    AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
    AddFormat( SotClipboardFormatId::GDIMETAFILE );
    AddFormat( SotClipboardFormatId::PNG );
    AddFormat( SotClipboardFormatId::BITMAP );

    // ScImportExport formats
    AddFormat( SotClipboardFormatId::HTML );
    AddFormat( SotClipboardFormatId::SYLK );
    AddFormat( SotClipboardFormatId::LINK );
    AddFormat( SotClipboardFormatId::DIF );
    AddFormat( SotClipboardFormatId::STRING );
    AddFormat( SotClipboardFormatId::STRING_TSVC );

    AddFormat( SotClipboardFormatId::RTF );
    AddFormat( SotClipboardFormatId::RICHTEXT );
    if ( m_aBlock.aStart == m_aBlock.aEnd )
    {
        AddFormat( SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT );
    }
}

bool ScTransferObj::GetData( const datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );
    bool        bOK = false;

    if( HasFormat( nFormat ) )
    {
        ScRange aReducedBlock = m_aBlock;

        bool bReduceBlockFormat =
            nFormat == SotClipboardFormatId::HTML
            || nFormat == SotClipboardFormatId::RTF
            || nFormat == SotClipboardFormatId::RICHTEXT
            || nFormat == SotClipboardFormatId::BITMAP
            || nFormat == SotClipboardFormatId::PNG;

        if (bReduceBlockFormat && (m_aBlock.aEnd.Col() == MAXCOL || m_aBlock.aEnd.Row() == MAXROW) &&
                m_aBlock.aStart.Tab() == m_aBlock.aEnd.Tab())
        {
            // Shrink the block here so we don't waste time creating huge
            // output when whole columns or rows are selected.

            SCCOL nPrintAreaEndCol = 0;
            SCROW nPrintAreaEndRow = 0;
            const bool bIncludeVisual = (nFormat == SotClipboardFormatId::BITMAP ||
                    nFormat == SotClipboardFormatId::PNG);
            if (bIncludeVisual)
                m_pDoc->GetPrintArea( m_aBlock.aStart.Tab(), nPrintAreaEndCol, nPrintAreaEndRow, true );

            // Shrink the area to allow pasting to external applications.
            // Shrink to real data area for HTML, RTF and RICHTEXT, but include
            // all objects and top-left area for BITMAP and PNG.
            SCCOL nStartCol = aReducedBlock.aStart.Col();
            SCROW nStartRow = aReducedBlock.aStart.Row();
            SCCOL nEndCol = aReducedBlock.aEnd.Col();
            SCROW nEndRow = aReducedBlock.aEnd.Row();
            bool bShrunk = false;
            m_pDoc->ShrinkToUsedDataArea( bShrunk, aReducedBlock.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow,
                    false, bIncludeVisual /*bStickyTopRow*/, bIncludeVisual /*bStickyLeftCol*/,
                    bIncludeVisual /*bConsiderCellNotes*/, bIncludeVisual /*bConsiderCellDrawObjects*/);

            if ( nPrintAreaEndRow > nEndRow )
                nEndRow = nPrintAreaEndRow;

            if ( nPrintAreaEndCol > nEndCol )
                nEndCol = nPrintAreaEndCol;

            aReducedBlock = ScRange(nStartCol, nStartRow, aReducedBlock.aStart.Tab(), nEndCol, nEndRow, aReducedBlock.aEnd.Tab());
        }

        if ( nFormat == SotClipboardFormatId::LINKSRCDESCRIPTOR || nFormat == SotClipboardFormatId::OBJECTDESCRIPTOR )
        {
            bOK = SetTransferableObjectDescriptor( m_aObjDesc );
        }
        else if ( ( nFormat == SotClipboardFormatId::RTF || nFormat == SotClipboardFormatId::RICHTEXT ||
            nFormat == SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT ) && m_aBlock.aStart == m_aBlock.aEnd )
        {
            //  RTF from a single cell is handled by EditEngine

            SCCOL nCol = m_aBlock.aStart.Col();
            SCROW nRow = m_aBlock.aStart.Row();
            SCTAB nTab = m_aBlock.aStart.Tab();
            ScAddress aPos(nCol, nRow, nTab);

            const ScPatternAttr* pPattern = m_pDoc->GetPattern( nCol, nRow, nTab );
            ScTabEditEngine aEngine( *pPattern, m_pDoc->GetEditPool(), m_pDoc.get() );
            ScRefCellValue aCell(*m_pDoc, aPos);
            if (aCell.meType == CELLTYPE_EDIT)
            {
                const EditTextObject* pObj = aCell.mpEditText;
                aEngine.SetText(*pObj);
            }
            else
            {
                SvNumberFormatter* pFormatter = m_pDoc->GetFormatTable();
                sal_uInt32 nNumFmt = pPattern->GetNumberFormat(pFormatter);
                OUString aText;
                Color* pColor;
                ScCellFormat::GetString(aCell, nNumFmt, aText, &pColor, *pFormatter, m_pDoc.get());
                if (!aText.isEmpty())
                    aEngine.SetText(aText);
            }

            bOK = SetObject( &aEngine,
                    ((nFormat == SotClipboardFormatId::RTF) ? SCTRANS_TYPE_EDIT_RTF :
                     ((nFormat == SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT) ?
                      SCTRANS_TYPE_EDIT_ODF_TEXT_FLAT : SCTRANS_TYPE_EDIT_BIN)),
                    rFlavor );
        }
        else if ( ScImportExport::IsFormatSupported( nFormat ) || nFormat == SotClipboardFormatId::RTF
            || nFormat == SotClipboardFormatId::RICHTEXT )
        {
            //  if this transfer object was used to create a DDE link, filtered rows
            //  have to be included for subsequent calls (to be consistent with link data)
            if ( nFormat == SotClipboardFormatId::LINK )
                m_bUsedForLink = true;

            bool bIncludeFiltered = m_pDoc->IsCutMode() || m_bUsedForLink;

            ScImportExport aObj( m_pDoc.get(), aReducedBlock );
            // Plain text ("Unformatted text") may contain embedded tabs and
            // line breaks but is not enclosed in quotes. Which makes it
            // unsuitable for multiple cells, especially if one of them is
            // multi-line, but otherwise is expected behavior for plain text.
            // For multiple cells replace embedded line breaks (and tabs) with
            // space character, otherwise pasting would yield odd results.
            /* XXX: it's debatable whether this is actually expected, but
             * there's no way to satisfy all possible requirements when
             * copy/pasting unformatted text. */
            const bool bPlainMulti = (nFormat == SotClipboardFormatId::STRING &&
                    aReducedBlock.aStart != aReducedBlock.aEnd);
            // Add quotes only for STRING_TSVC.
            /* TODO: a possible future STRING_TSV should not contain embedded
             * line breaks nor tab (separator) characters and not be quoted.
             * A possible STRING_CSV should. */
            ScExportTextOptions aTextOptions( ScExportTextOptions::None, 0,
                    (nFormat == SotClipboardFormatId::STRING_TSVC));
            if ( bPlainMulti || m_bUsedForLink )
            {
                // For a DDE link or plain text multiple cells, convert line
                // breaks and separators to space.
                aTextOptions.meNewlineConversion = ScExportTextOptions::ToSpace;
                aTextOptions.mcSeparatorConvertTo = ' ';
                aTextOptions.mbAddQuotes = false;
            }
            aObj.SetExportTextOptions(aTextOptions);
            aObj.SetFormulas( m_pDoc->GetViewOptions().GetOption( VOPT_FORMULAS ) );
            aObj.SetIncludeFiltered( bIncludeFiltered );

            //  DataType depends on format type:

            if ( rFlavor.DataType.equals( ::cppu::UnoType<OUString>::get() ) )
            {
                OUString aString;
                if ( aObj.ExportString( aString, nFormat ) )
                    bOK = SetString( aString, rFlavor );
            }
            else if ( rFlavor.DataType.equals( cppu::UnoType<uno::Sequence< sal_Int8 >>::get() ) )
            {
                //  SetObject converts a stream into a Int8-Sequence
                bOK = SetObject( &aObj, SCTRANS_TYPE_IMPEX, rFlavor );
            }
            else
            {
                OSL_FAIL("unknown DataType");
            }
        }
        else if ( nFormat == SotClipboardFormatId::BITMAP || nFormat == SotClipboardFormatId::PNG )
        {
            tools::Rectangle aMMRect = m_pDoc->GetMMRect( aReducedBlock.aStart.Col(), aReducedBlock.aStart.Row(),
                                                 aReducedBlock.aEnd.Col(), aReducedBlock.aEnd.Row(),
                                                 aReducedBlock.aStart.Tab() );
            ScopedVclPtrInstance< VirtualDevice > pVirtDev;
            pVirtDev->SetOutputSizePixel(pVirtDev->LogicToPixel(aMMRect.GetSize(), MapMode(MapUnit::Map100thMM)));

            PaintToDev( pVirtDev, m_pDoc.get(), 1.0, aReducedBlock );

            pVirtDev->SetMapMode( MapMode( MapUnit::MapPixel ) );
            BitmapEx aBmp = pVirtDev->GetBitmapEx( Point(), pVirtDev->GetOutputSize() );
            bOK = SetBitmapEx( aBmp, rFlavor );
        }
        else if ( nFormat == SotClipboardFormatId::GDIMETAFILE )
        {
            // #i123405# Do not limit visual size calculation for metafile creation.
            // It seems unlikely that removing the limitation causes problems since
            // metafile creation means that no real pixel device in the needed size is
            // created.
            InitDocShell(false);

            SfxObjectShell* pEmbObj = m_aDocShellRef.get();

            // like SvEmbeddedTransfer::GetData:
            GDIMetaFile     aMtf;
            ScopedVclPtrInstance< VirtualDevice > pVDev;
            MapMode         aMapMode( pEmbObj->GetMapUnit() );
            tools::Rectangle       aVisArea( pEmbObj->GetVisArea( ASPECT_CONTENT ) );

            pVDev->EnableOutput( false );
            pVDev->SetMapMode( aMapMode );
            aMtf.SetPrefSize( aVisArea.GetSize() );
            aMtf.SetPrefMapMode( aMapMode );
            aMtf.Record( pVDev );

            pEmbObj->DoDraw( pVDev, Point(), aVisArea.GetSize(), JobSetup() );

            aMtf.Stop();
            aMtf.WindStart();

            bOK = SetGDIMetaFile( aMtf );
        }
        else if ( nFormat == SotClipboardFormatId::EMBED_SOURCE )
        {
            //TODO/LATER: differentiate between formats?!
            // #i123405# Do limit visual size calculation to PageSize
            InitDocShell(true);         // set aDocShellRef

            SfxObjectShell* pEmbObj = m_aDocShellRef.get();
            bOK = SetObject( pEmbObj, SCTRANS_TYPE_EMBOBJ, rFlavor );
        }
    }
    return bOK;
}

bool ScTransferObj::WriteObject( tools::SvRef<SotStorageStream>& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const datatransfer::DataFlavor& rFlavor )
{
    // called from SetObject, put data into stream

    bool bRet = false;
    switch (nUserObjectId)
    {
        case SCTRANS_TYPE_IMPEX:
            {
                ScImportExport* pImpEx = static_cast<ScImportExport*>(pUserObject);

                SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );
                // mba: no BaseURL for data exchange
                if ( pImpEx->ExportStream( *rxOStm, OUString(), nFormat ) )
                    bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        case SCTRANS_TYPE_EDIT_RTF:
        case SCTRANS_TYPE_EDIT_BIN:
            {
                ScTabEditEngine* pEngine = static_cast<ScTabEditEngine*>(pUserObject);
                if ( nUserObjectId == SCTRANS_TYPE_EDIT_RTF )
                {
                    pEngine->Write( *rxOStm, EETextFormat::Rtf );
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

        case SCTRANS_TYPE_EDIT_ODF_TEXT_FLAT:
            {
                ScTabEditEngine* pEngine = static_cast<ScTabEditEngine*>(pUserObject);
                pEngine->Write(*rxOStm, EETextFormat::Xml);
                bRet = (rxOStm->GetError() == ERRCODE_NONE);
            }
            break;

        case SCTRANS_TYPE_EMBOBJ:
            {
                // TODO/MBA: testing
                SfxObjectShell*   pEmbObj = static_cast<SfxObjectShell*>(pUserObject);
                ::utl::TempFile     aTempFile;
                aTempFile.EnableKillingFile();
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false );

                // mba: no relative URLs for clipboard!
                SfxMedium aMedium( xWorkStore, OUString() );
                pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                std::unique_ptr<SvStream> pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), StreamMode::READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    rxOStm->WriteStream( *pSrcStm );
                    pSrcStm.reset();
                }

                bRet = true;

                xWorkStore->dispose();
                xWorkStore.clear();
                rxOStm->Commit();
            }
            break;

        default:
            OSL_FAIL("unknown object id");
    }
    return bRet;
}

void ScTransferObj::DragFinished( sal_Int8 nDropAction )
{
    if ( nDropAction == DND_ACTION_MOVE && !m_bDragWasInternal && !(m_nDragSourceFlags & ScDragSrc::Navigator) )
    {
        //  move: delete source data
        ScDocShell* pSourceSh = GetSourceDocShell();
        if (pSourceSh)
        {
            ScMarkData aMarkData = GetSourceMarkData();
            //  external drag&drop doesn't copy objects, so they also aren't deleted:
            //  bApi=TRUE, don't show error messages from drag&drop
            pSourceSh->GetDocFunc().DeleteContents( aMarkData, InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS, true, true );
        }
    }

    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetDragData().pCellTransfer == this )
        pScMod->ResetDragObject();

    m_xDragSourceRanges = nullptr;       // don't keep source after dropping

    TransferableHelper::DragFinished( nDropAction );
}

void ScTransferObj::SetDragHandlePos( SCCOL nX, SCROW nY )
{
    m_nDragHandleX = nX;
    m_nDragHandleY = nY;
}

void ScTransferObj::SetSourceCursorPos( SCCOL nX, SCROW nY )
{
    m_nSourceCursorX = nX;
    m_nSourceCursorY = nY;
}

bool ScTransferObj::WasSourceCursorInSelection() const
{
    return
        m_nSourceCursorX >= m_aBlock.aStart.Col() && m_nSourceCursorX <= m_aBlock.aEnd.Col() &&
        m_nSourceCursorY >= m_aBlock.aStart.Row() && m_nSourceCursorY <= m_aBlock.aEnd.Row();
}

void ScTransferObj::SetVisibleTab( SCTAB nNew )
{
    m_nVisibleTab = nNew;
}

void ScTransferObj::SetDrawPersist( const SfxObjectShellRef& rRef )
{
    m_aDrawPersistRef = rRef;
}

void ScTransferObj::SetDragSource( ScDocShell* pSourceShell, const ScMarkData& rMark )
{
    ScRangeList aRanges;
    rMark.FillRangeListWithMarks( &aRanges, false );
    m_xDragSourceRanges = new ScCellRangesObj( pSourceShell, aRanges );
}

void ScTransferObj::SetDragSourceFlags(ScDragSrc nFlags)
{
    m_nDragSourceFlags = nFlags;
}

void ScTransferObj::SetDragWasInternal()
{
    m_bDragWasInternal = true;
}

void ScTransferObj::SetUseInApi( bool bSet )
{
    m_bUseInApi = bSet;
}

ScDocument* ScTransferObj::GetSourceDocument()
{
    ScDocShell* pSourceDocSh = GetSourceDocShell();
    if (pSourceDocSh)
        return &pSourceDocSh->GetDocument();
    return nullptr;
}

ScDocShell* ScTransferObj::GetSourceDocShell()
{
    ScCellRangesBase* pRangesObj = ScCellRangesBase::getImplementation( m_xDragSourceRanges );
    if (pRangesObj)
        return pRangesObj->GetDocShell();

    return nullptr;    // none set
}

ScMarkData ScTransferObj::GetSourceMarkData()
{
    ScMarkData aMarkData;
    ScCellRangesBase* pRangesObj = ScCellRangesBase::getImplementation( m_xDragSourceRanges );
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
    if ( !m_aDocShellRef.is() )
    {
        ScDocShell* pDocSh = new ScDocShell;
        m_aDocShellRef = pDocSh;      // ref must be there before InitNew

        pDocSh->DoInitNew();

        ScDocument& rDestDoc = pDocSh->GetDocument();
        ScMarkData aDestMark;
        aDestMark.SelectTable( 0, true );

        rDestDoc.SetDocOptions( m_pDoc->GetDocOptions() );   // #i42666#

        OUString aTabName;
        m_pDoc->GetName( m_aBlock.aStart.Tab(), aTabName );
        rDestDoc.RenameTab( 0, aTabName );

        rDestDoc.CopyStdStylesFrom( m_pDoc.get() );

        SCCOL nStartX = m_aBlock.aStart.Col();
        SCROW nStartY = m_aBlock.aStart.Row();
        SCCOL nEndX = m_aBlock.aEnd.Col();
        SCROW nEndY = m_aBlock.aEnd.Row();

        //  widths / heights
        //  (must be copied before CopyFromClip, for drawing objects)

        SCCOL nCol;
        SCTAB nSrcTab = m_aBlock.aStart.Tab();
        rDestDoc.SetLayoutRTL(0, m_pDoc->IsLayoutRTL(nSrcTab));
        for (nCol=nStartX; nCol<=nEndX; nCol++)
            if ( m_pDoc->ColHidden(nCol, nSrcTab) )
                rDestDoc.ShowCol( nCol, 0, false );
            else
                rDestDoc.SetColWidth( nCol, 0, m_pDoc->GetColWidth( nCol, nSrcTab ) );

        if (nStartY > 0)
        {
            // Set manual height for all previous rows so we can ensure
            // that visible area will not change due to autoheight
            rDestDoc.SetManualHeight(0, nStartY - 1, 0, true);
        }
        for (SCROW nRow = nStartY; nRow <= nEndY; ++nRow)
        {
            if ( m_pDoc->RowHidden(nRow, nSrcTab) )
                rDestDoc.ShowRow( nRow, 0, false );
            else
            {
                rDestDoc.SetRowHeight( nRow, 0, m_pDoc->GetOriginalHeight( nRow, nSrcTab ) );

                //  if height was set manually, that flag has to be copied, too
                bool bManual = m_pDoc->IsManualRowHeight(nRow, nSrcTab);
                rDestDoc.SetManualHeight(nRow, nRow, 0, bManual);
            }
        }

        if (m_pDoc->GetDrawLayer() || m_pDoc->HasNotes())
            pDocSh->MakeDrawLayer();

        //  cell range is copied to the original position, but on the first sheet
        //  -> bCutMode must be set
        //  pDoc is always a Clipboard-document

        ScRange aDestRange( nStartX,nStartY,0, nEndX,nEndY,0 );
        bool bWasCut = m_pDoc->IsCutMode();
        if (!bWasCut)
            m_pDoc->SetClipArea( aDestRange, true );          // Cut
        rDestDoc.CopyFromClip( aDestRange, aDestMark, InsertDeleteFlags::ALL, nullptr, m_pDoc.get(), false );
        m_pDoc->SetClipArea( aDestRange, bWasCut );

        StripRefs( m_pDoc.get(), nStartX,nStartY, nEndX,nEndY, &rDestDoc );

        ScRange aMergeRange = aDestRange;
        rDestDoc.ExtendMerge( aMergeRange, true );

        m_pDoc->CopyDdeLinks( &rDestDoc );         // copy values of DDE Links

        //  page format (grid etc) and page size (maximum size for ole object)

        Size aPaperSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );       // Twips
        ScStyleSheetPool* pStylePool = m_pDoc->GetStyleSheetPool();
        OUString aStyleName = m_pDoc->GetPageStyle( m_aBlock.aStart.Tab() );
        SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
        if (pStyleSheet)
        {
            const SfxItemSet& rSourceSet = pStyleSheet->GetItemSet();
            aPaperSize = rSourceSet.Get(ATTR_PAGE_SIZE).GetSize();

            // CopyStyleFrom copies SetItems with correct pool
            ScStyleSheetPool* pDestPool = rDestDoc.GetStyleSheetPool();
            pDestPool->CopyStyleFrom( pStylePool, aStyleName, SfxStyleFamily::Page );
        }

        ScViewData aViewData( pDocSh, nullptr );
        aViewData.SetScreen( nStartX,nStartY, nEndX,nEndY );
        aViewData.SetCurX( nStartX );
        aViewData.SetCurY( nStartY );

        rDestDoc.SetViewOptions( m_pDoc->GetViewOptions() );

        //      Size
        //! get while copying sizes

        long nPosX = 0;
        long nPosY = 0;

        for (nCol=0; nCol<nStartX; nCol++)
            nPosX += rDestDoc.GetColWidth( nCol, 0 );
        nPosY += rDestDoc.GetRowHeight( 0, nStartY-1, 0 );
        nPosX = static_cast<long>( nPosX * HMM_PER_TWIPS );
        nPosY = static_cast<long>( nPosY * HMM_PER_TWIPS );

        aPaperSize.setWidth( aPaperSize.Width() * 2 );       // limit OLE object to double of page size
        aPaperSize.setHeight( aPaperSize.Height() * 2 );

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
        nSizeX = static_cast<long>( nSizeX * HMM_PER_TWIPS );
        nSizeY = static_cast<long>( nSizeY * HMM_PER_TWIPS );

//      pDocSh->SetVisAreaSize( Size(nSizeX,nSizeY) );

        tools::Rectangle aNewArea( Point(nPosX,nPosY), Size(nSizeX,nSizeY) );
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
    // update ScGlobal::xDrawClipDocShellRef

    ScGlobal::xDrawClipDocShellRef.clear();
    if (bAnyOle)
    {
        ScGlobal::xDrawClipDocShellRef = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS); // there must be a ref
        ScGlobal::xDrawClipDocShellRef->DoInitNew();
        return ScGlobal::xDrawClipDocShellRef.get();
    }
    else
    {
        ScGlobal::xDrawClipDocShellRef.clear();
        return nullptr;
    }
}

void ScTransferObj::StripRefs( ScDocument* pDoc,
                    SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY,
                    ScDocument* pDestDoc )
{
    if (!pDestDoc)
    {
        pDestDoc = pDoc;
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
            SCCOL nCol = aIter.GetPos().Col();
            SCROW nRow = aIter.GetPos().Row();

            FormulaError nErrCode = pFCell->GetErrCode();
            ScAddress aPos(nCol, nRow, nDestTab);
            if (nErrCode != FormulaError::NONE)
            {
                if ( pDestDoc->GetAttr( nCol,nRow,nDestTab, ATTR_HOR_JUSTIFY)->GetValue() ==
                        SvxCellHorJustify::Standard )
                    pDestDoc->ApplyAttr( nCol,nRow,nDestTab,
                            SvxHorJustifyItem(SvxCellHorJustify::Right, ATTR_HOR_JUSTIFY) );

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

const css::uno::Sequence< sal_Int8 >& ScTransferObj::getUnoTunnelId()
{
    return theScTransferUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL ScTransferObj::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
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
