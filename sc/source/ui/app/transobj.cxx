/*************************************************************************
 *
 *  $RCSfile: transobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-14 19:14:35 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <com/sun/star/uno/Sequence.hxx>

#include <sot/storage.hxx>
#include <so3/svstor.hxx>
#include <vcl/virdev.hxx>

#include "transobj.hxx"
#include "document.hxx"
#include "viewopti.hxx"
#include "editutil.hxx"
#include "impex.hxx"
#include "cell.hxx"
#include "printfun.hxx"
#include "scmod.hxx"

// for InitDocShell
#include "scitems.hxx"
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svx/algitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include "docsh.hxx"
#include "markdata.hxx"
#include "stlpool.hxx"
#include "viewdata.hxx"
#include "dociter.hxx"

// -----------------------------------------------------------------------

#define SCTRANS_TYPE_IMPEX          1
#define SCTRANS_TYPE_EDIT_RTF       2
#define SCTRANS_TYPE_EDIT_BIN       3
#define SCTRANS_TYPE_EMBOBJ         4

// -----------------------------------------------------------------------

// static
void ScTransferObj::GetAreaSize( ScDocument* pDoc, USHORT nTab1, USHORT nTab2, USHORT& nRow, USHORT& nCol )
{
    USHORT nMaxCol = 0, nMaxRow = 0;
    for( USHORT nTab = nTab1; nTab <= nTab2; nTab++ )
    {
        USHORT nLastCol = 0, nLastRow = 0;
        if( pDoc->GetCellArea( nTab, nLastCol, nLastRow ) )
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

// static
void ScTransferObj::PaintToDev( OutputDevice* pDev, ScDocument* pDoc, double nPrintFactor,
                                const ScRange& rBlock, BOOL bMetaFile )
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
    aObjDesc( rDesc ),
    pDoc( pClipDoc )
{
    DBG_ASSERT(pDoc->IsClipboard(), "wrong document");

    //
    // get aBlock from clipboard doc
    //

    USHORT nCol1;
    USHORT nRow1;
    USHORT nCol2;
    USHORT nRow2;
    pDoc->GetClipStart( nCol1, nRow1 );
    pDoc->GetClipArea( nCol2, nRow2 );
    nCol2 += nCol1;
    nRow2 += nRow1;

    USHORT nTab1=0;
    USHORT nTab2=0;
    BOOL bFirst = TRUE;
    for (USHORT i=0; i<=MAXTAB; i++)
        if (pDoc->HasTable(i))
        {
            if (bFirst)
                nTab1 = i;
            nTab2 = i;
            bFirst = FALSE;
        }
    DBG_ASSERT(!bFirst, "no sheet selected");

    //  only limit to used cells if whole sheet was marked
    //  (so empty cell areas can be copied)
    if ( nCol2>=MAXCOL && nRow2>=MAXROW )
    {
        USHORT nMaxRow, nMaxCol;
        GetAreaSize( pDoc, nTab1, nTab2, nMaxRow, nMaxCol );
        if( nMaxRow < nRow2 )
            nRow2 = nMaxRow;
        if( nMaxCol < nCol2 )
            nCol2 = nMaxCol;
    }

    aBlock = ScRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
}

ScTransferObj::~ScTransferObj()
{
    ScModule* pScMod = SC_MOD();
    if ( pScMod->GetClipData().pCellClipboard == this )
    {
        DBG_ERROR("ScTransferObj wasn't released");
        pScMod->SetClipObject( NULL, NULL );
    }

    delete pDoc;        // ScTransferObj is owner of clipboard document
}

// static
ScTransferObj* ScTransferObj::GetOwnClipboard()
{
    ScTransferObj* pObj = SC_MOD()->GetClipData().pCellClipboard;
    if ( pObj )
    {
        //  check formats to see if pObj is really in the system clipboard

        TransferableDataHelper aDataHelper( TransferableDataHelper::CreateFromSystemClipboard() );
        if ( !aDataHelper.HasFormat( SOT_FORMATSTR_ID_DIF ) )
        {
//          DBG_ERROR("ScTransferObj wasn't released");
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

sal_Bool ScTransferObj::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bOK = sal_False;

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

            USHORT nCol = aBlock.aStart.Col();
            USHORT nRow = aBlock.aStart.Row();
            USHORT nTab = aBlock.aStart.Tab();

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
            ScImportExport aObj( pDoc, aBlock );
            aObj.SetFormulas( pDoc->GetViewOptions().GetOption( VOPT_FORMULAS ) );

            //  DataType depends on format type:

            if ( rFlavor.DataType.equals( ::getCppuType( (const ::rtl::OUString*) 0 ) ) )
            {
                String aString;
                if ( aObj.ExportString( aString, nFormat ) )
                    bOK = SetString( aString, rFlavor );
            }
            else if ( rFlavor.DataType.equals( ::getCppuType(
                                    (const ::com::sun::star::uno::Sequence< sal_Int8 >*) 0 ) ) )
            {
                //  SetObject converts a stream into a Int8-Sequence
                bOK = SetObject( &aObj, SCTRANS_TYPE_IMPEX, rFlavor );
            }
            else
            {
                DBG_ERROR("unknown DataType");
            }
        }
        else if ( nFormat == SOT_FORMAT_BITMAP )
        {
            Rectangle aMMRect = pDoc->GetMMRect( aBlock.aStart.Col(), aBlock.aStart.Row(),
                                                 aBlock.aEnd.Col(), aBlock.aEnd.Row(),
                                                 aBlock.aStart.Tab() );
            VirtualDevice aVirtDev;
            aVirtDev.SetOutputSizePixel( aVirtDev.LogicToPixel( aMMRect.GetSize(), MAP_100TH_MM ) );

            PaintToDev( &aVirtDev, pDoc, 1.0, aBlock, FALSE );

            aVirtDev.SetMapMode( MapMode( MAP_PIXEL ) );
            Bitmap aBmp = aVirtDev.GetBitmap( Point(), aVirtDev.GetOutputSize() );
            bOK = SetBitmap( aBmp, rFlavor );
        }
        else if ( nFormat == SOT_FORMAT_GDIMETAFILE )
        {
            InitDocShell();
            SvEmbeddedObject* pEmbObj = aDocShellRef;

            // like SvEmbeddedTransfer::GetData:

            GDIMetaFile     aMtf;
            VirtualDevice   aVDev;
            MapMode         aMapMode( pEmbObj->GetMapUnit() );
            Rectangle       aVisArea( pEmbObj->GetVisArea( ASPECT_CONTENT ) );

            aVDev.EnableOutput( FALSE );
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
            InitDocShell();         // set aDocShellRef

            SvEmbeddedObject* pEmbObj = aDocShellRef;
            bOK = SetObject( pEmbObj, SCTRANS_TYPE_EMBOBJ, rFlavor );
        }
    }
    return bOK;
}

sal_Bool ScTransferObj::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    // called from SetObject, put data into stream

    sal_Bool bRet = sal_False;
    switch (nUserObjectId)
    {
        case SCTRANS_TYPE_IMPEX:
            {
                ScImportExport* pImpEx = (ScImportExport*)pUserObject;

                sal_uInt32 nFormat = SotExchange::GetFormat( rFlavor );
                if ( pImpEx->ExportStream( *rxOStm, nFormat ) )
                    bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        case SCTRANS_TYPE_EDIT_RTF:
        case SCTRANS_TYPE_EDIT_BIN:
            {
                ScTabEditEngine* pEngine = (ScTabEditEngine*)pUserObject;
                if ( nUserObjectId == SCTRANS_TYPE_EDIT_RTF )
                    pEngine->Write( *rxOStm, EE_FORMAT_RTF );
                else
                    pEngine->Write( *rxOStm, EE_FORMAT_BIN );
                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        case SCTRANS_TYPE_EMBOBJ:
            {
                SvEmbeddedObject* pEmbObj = (SvEmbeddedObject*)pUserObject;

                SvStorageRef xWorkStore( new SvStorage( *rxOStm ) );
                rxOStm->SetBufferSize( 0xff00 );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore );
                bRet = pEmbObj->DoSaveAs( xWorkStore );
                pEmbObj->DoSaveCompleted();
                xWorkStore->Commit();
                rxOStm->Commit();

                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            break;

        default:
            DBG_ERROR("unknown object id");
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
        aDestMark.SelectTable( 0, TRUE );

        String aTabName;
        pDoc->GetName( aBlock.aStart.Tab(), aTabName );
        pDestDoc->RenameTab( 0, aTabName, FALSE );          // no UpdateRef (empty)

        //  cell range is copied to the original position, but on the first sheet
        //  -> bCutMode must be set
        //  pDoc is always a Clipboard-document

        pDestDoc->CopyStdStylesFrom( pDoc );

        USHORT nStartX = aBlock.aStart.Col();
        USHORT nStartY = aBlock.aStart.Row();
        USHORT nEndX = aBlock.aEnd.Col();
        USHORT nEndY = aBlock.aEnd.Row();
        ScRange aDestRange( nStartX,nStartY,0, nEndX,nEndY,0 );
        BOOL bWasCut = pDoc->IsCutMode();
        if (!bWasCut)
            pDoc->SetClipArea( aDestRange, TRUE );          // Cut
        pDestDoc->CopyFromClip( aDestRange, aDestMark, IDF_ALL, NULL, pDoc, FALSE );
        pDoc->SetClipArea( aDestRange, bWasCut );

        StripRefs( pDoc, nStartX,nStartY, nEndX,nEndY, pDestDoc, 0,0 );

        ScRange aMergeRange = aDestRange;
        pDestDoc->ExtendMerge( aMergeRange, TRUE );

        pDoc->CopyDdeLinks( pDestDoc );         // copy values of DDE Links

        //  widths / heights

        USHORT nCol;
        USHORT nRow;
        for (nCol=nStartX; nCol<=nEndX; nCol++)
            if ( pDoc->GetColFlags( nCol, aBlock.aStart.Tab() ) & CR_HIDDEN )
                pDestDoc->ShowCol( nCol, 0, FALSE );
            else
                pDestDoc->SetColWidth( nCol, 0,
                    pDoc->GetColWidth( nCol, aBlock.aStart.Tab() ) );
        for (nRow=nStartY; nRow<=nEndY; nRow++)
            if ( pDoc->GetRowFlags( nRow, aBlock.aStart.Tab() ) & CR_HIDDEN )
                pDestDoc->ShowRow( nRow, 0, FALSE );
            else
                pDestDoc->SetRowHeight( nRow, 0,
                    pDoc->GetRowHeight( nRow, aBlock.aStart.Tab() ) );

        //  page format (grid etc) and page size (maximum size for ole object)

        Size aPaperSize = SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 );       // Twips
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
        for (nRow=0; nRow<nStartY; nRow++)
            nPosY += pDestDoc->FastGetRowHeight( nRow, 0 );
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
        for (nRow=nStartY; nRow<=nEndY; nRow++)
        {
            long nAdd = pDestDoc->FastGetRowHeight( nRow, 0 );
            if ( nSizeY+nAdd > aPaperSize.Height() && nSizeY )  // above limit?
                break;
            nSizeY += nAdd;
        }
        nSizeX = (long) ( nSizeX * HMM_PER_TWIPS );
        nSizeY = (long) ( nSizeY * HMM_PER_TWIPS );

//      pDocSh->SetVisAreaSize( Size(nSizeX,nSizeY) );

        Rectangle aNewArea( Point(nPosX,nPosY), Size(nSizeX,nSizeY) );
        pDocSh->SvInPlaceObject::SetVisArea( aNewArea );
        pDocSh->SetVisArea( aNewArea );

        pDocSh->UpdateOle(&aViewData, TRUE);
    }
}

//  static
void ScTransferObj::StripRefs( ScDocument* pDoc,
                    USHORT nStartX, USHORT nStartY, USHORT nEndX, USHORT nEndY,
                    ScDocument* pDestDoc, USHORT nSubX, USHORT nSubY )
{
    if (!pDestDoc)
    {
        pDestDoc = pDoc;
        DBG_ASSERT(nSubX==0&&nSubY==0, "can't move within the document");
    }

    //  In a clipboard doc the data don't have to be on the first sheet

    USHORT nSrcTab = 0;
    while (nSrcTab<MAXTAB && !pDoc->HasTable(nSrcTab))
        ++nSrcTab;
    USHORT nDestTab = 0;
    while (nDestTab<MAXTAB && !pDestDoc->HasTable(nDestTab))
        ++nDestTab;

    if (!pDoc->HasTable(nSrcTab) || !pDestDoc->HasTable(nDestTab))
    {
        DBG_ERROR("Sheet not found in ScTransferObj::StripRefs");
        return;
    }

    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
    ScTripel aStart, aEnd;

    ScCellIterator aIter( pDoc, nStartX, nStartY, nSrcTab, nEndX, nEndY, nSrcTab );
    ScBaseCell* pCell = aIter.GetFirst();
    while (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
            BOOL bOut = FALSE;
            ScDetectiveRefIter aRefIter( pFCell );
            while ( !bOut && aRefIter.GetNextRef( aStart, aEnd ) )
            {
                if ( aStart.GetTab() != nSrcTab || aEnd.GetTab() != nSrcTab ||
                        aStart.GetCol() < nStartX || aEnd.GetCol() > nEndX ||
                        aStart.GetRow() < nStartY || aEnd.GetRow() > nEndY )
                    bOut = TRUE;
            }
            if (bOut)
            {
                USHORT nCol = aIter.GetCol() - nSubX;
                USHORT nRow = aIter.GetRow() - nSubY;

                ScBaseCell* pNew = 0;
                USHORT nErrCode = pFCell->GetErrCode();
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
                    pNew = new ScStringCell( aStr );
                }
                pDestDoc->PutCell( nCol,nRow,nDestTab, pNew );

                //  number formats

                ULONG nOldFormat = ((const SfxUInt32Item*)
                                pDestDoc->GetAttr(nCol,nRow,nDestTab, ATTR_VALUE_FORMAT))->GetValue();
                if ( (nOldFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                {
                    ULONG nNewFormat = pFCell->GetStandardFormat( *pFormatter,
                        nOldFormat );
                    pDestDoc->ApplyAttr( nCol,nRow,nDestTab,
                                SfxUInt32Item(ATTR_VALUE_FORMAT, nNewFormat) );
                }
            }
        }
        pCell = aIter.GetNext();
    }
}


