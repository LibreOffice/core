/*************************************************************************
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.95 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:51:18 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef SC_ITEMS_HXX
#include <scitems.hxx>
#endif

#include <svtools/fltrcfg.hxx>

#include <svtools/wmf.hxx>

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <svx/brshitem.hxx>
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/colritem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/postitem.hxx>
#include <svx/crsditem.hxx>
#include <svx/flditem.hxx>
#include <svx/xflclit.hxx>
#include <svx/svxmsbas.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#include <unotools/charclass.hxx>

#ifndef SC_DRWLAYER_HXX
#include <drwlayer.hxx>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "conditio.hxx"
#include "dbcolect.hxx"
#include "editutil.hxx"
#include "markdata.hxx"
#include "rangenam.hxx"
#include "docoptio.hxx"
#ifndef SC_DETFUNC_HXX
#include "detfunc.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif

#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif
#ifndef SC_XIPIVOT_HXX
#include "xipivot.hxx"
#endif

#include "excimp8.hxx"
#include "excform.hxx"
#include "flttools.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"

using namespace com::sun::star;


#define INVALID_POS     0xFFFFFFFF




ImportExcel8::ImportExcel8( SfxMedium& rMedium, SvStream& rStream, XclBiff eBiff, ScDocument* pDoc ) :
    ImportExcel( rMedium, rStream, eBiff, pDoc )
{
    delete pFormConv;

    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc8( pExcRoot, aIn );

    bHasBasic = FALSE;
}


ImportExcel8::~ImportExcel8()
{
}


void ImportExcel8::RecString( void )
{
    if( pLastFormCell )
    {
        pLastFormCell->SetString( aIn.ReadUniString() );

        pLastFormCell = NULL;
    }
}


void ImportExcel8::Calccount( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterCount( aIn.ReaduInt16() );
    pD->SetDocOptions( aOpt );
}


void ImportExcel8::Delta( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterEps( aIn.ReadDouble() );
    pD->SetDocOptions( aOpt );
}


void ImportExcel8::Iteration( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIter( aIn.ReaduInt16() == 1 );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8:: WinProtection( void )
{
    if( aIn.ReaduInt16() )
    {
        GetExtDocOptions().SetWinProtection( true );
    }
}

void ImportExcel8::Note( void )
{
    UINT16  nCol, nRow, nFlags, nId;

    aIn >> nRow >> nCol >> nFlags >> nId;

    SCTAB nScTab = GetCurrScTab();
    if( nRow <= static_cast<sal_uInt16>(MAXROW) && nCol <= static_cast<sal_uInt16>(MAXCOL) )
    {
        if( nId )
        {
            if( const XclImpEscherNote* pNoteObj = GetObjectManager().GetEscherNote( nScTab, nId ) )
            {
                if( const XclImpString* pString = pNoteObj->GetString() )
                {
                    bool bVisible = ::get_flag( nFlags, EXC_NOTE_VISIBLE );
                    ScPostIt aNote( pString->GetText() );
                    aNote.SetShown( bVisible );
                    GetDoc().SetNote( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), nScTab, aNote );
                    if( bVisible )
                    {
                        ScDocument* pDoc = GetDocPtr();
                        ScDetectiveFunc( pDoc, nScTab ).ShowComment( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), TRUE );
                    }
                }
            }
        }
    }
    else
    {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow( nScTab, nRow, MAXROW );
        }

    pLastFormCell = NULL;
}


void ImportExcel8::Cont( void )
{
    if( bObjSection )
        GetObjectManager().ReadMsodrawing( aIn );
}


void ImportExcel8::Obj()
{
    GetObjectManager().ReadObj( maStrm );
}


void ImportExcel8::Boundsheet( void )
{
    UINT8           nLen;
    UINT16          nGrbit;

    aIn.Ignore( 4 );
    aIn >> nGrbit >> nLen;

    String aName( aIn.ReadUniString( nLen ) );
    GetTabInfo().AppendXclTabName( aName, nBdshtTab );

    ScfTools::ConvertToScSheetName( aName );
    *pExcRoot->pTabNameBuff << aName;

    if( nBdshtTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nBdshtTab ),
            "*ImportExcel::Boundsheet8(): Tabelle schon vorhanden!" );

        pD->MakeTable( nBdshtTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nBdshtTab, FALSE );
    else if( nFirstVisTab == 0xFFFF )
        nFirstVisTab = static_cast<sal_uInt16>(nBdshtTab);       // first visible for WINDOW2 import

    pD->RenameTab( nBdshtTab, aName );
    nBdshtTab++;
}


void ImportExcel8::Scenman( void )
{
    UINT16              nLastDispl;

    aIn.Ignore( 4 );
    aIn >> nLastDispl;

    aScenList.SetLast( nLastDispl );
}


void ImportExcel8::Scenario( void )
{
    aScenList.Append( new ExcScenario( aIn, *pExcRoot ) );
}


void ImportExcel8::Cellmerging( void )
{
    UINT16  nCount, nRow1, nRow2, nCol1, nCol2;
    aIn >> nCount;

    DBG_ASSERT( aIn.GetRecLeft() >= (ULONG)(nCount * 8), "ImportExcel8::Cellmerging - wrong record size" );

    while( nCount-- )
    {
        aIn >> nRow1 >> nRow2 >> nCol1 >> nCol2;
        bTabTruncated |= (nRow1 > static_cast<sal_uInt16>(MAXROW)) || (nRow2 > static_cast<sal_uInt16>(MAXROW)) || (nCol1 > static_cast<sal_uInt16>(MAXCOL)) || (nCol2 > static_cast<sal_uInt16>(MAXCOL));
        if( (nRow1 <= static_cast<sal_uInt16>(MAXROW)) && (nCol1 <= static_cast<sal_uInt16>(MAXCOL)) )
        {
            nRow2 = Min( nRow2, static_cast<sal_uInt16>( MAXROW ) );
            nCol2 = Min( nCol2, static_cast<sal_uInt16>( MAXCOL ) );
            GetXFIndexBuffer().SetMerge( static_cast<SCCOL>(nCol1), static_cast<SCROW>(nRow1), static_cast<SCCOL>(nCol2), static_cast<SCROW>(nRow2) );
        }
        else
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow1 > static_cast<sal_uInt16>(MAXROW) ? nRow1 : nRow2, MAXROW);
    }
}


void ImportExcel8::Msodrawinggroup( void )
{
    GetObjectManager().ReadMsodrawinggroup( maStrm );
}


void ImportExcel8::Msodrawing( void )
{
    GetObjectManager().ReadMsodrawing( maStrm );
}


void ImportExcel8::Msodrawingselection( void )
{
    GetObjectManager().ReadMsodrawingselection( maStrm );
}

void ImportExcel8::Labelsst( void )
{
    UINT16                      nRow, nCol, nXF;
    UINT32                      nSst;


    aIn >> nRow >> nCol >> nXF >> nSst;

    if( nRow <= static_cast<sal_uInt16>(MAXROW) && nCol <= static_cast<sal_uInt16>(MAXCOL) )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        ScBaseCell* pCell = GetSst().CreateCell( nSst, nXF );
        if( pCell )
            GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), GetCurrScTab(), pCell );

        pColRowBuff->Used( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow) );
    }
    else
    {
        bTabTruncated = TRUE;
        GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
    }

    pLastFormCell = NULL;
}


void ImportExcel8::Rstring( void )
{
    UINT16 nRow, nCol, nXF;
    aIn >> nRow >> nCol >> nXF;

    if( nRow <= static_cast<sal_uInt16>(MAXROW) && nCol <= static_cast<sal_uInt16>(MAXCOL) )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        // unformatted Unicode string with separate formatting information
        XclImpString aString( maStrm );
        if( !aString.IsRich() )
            aString.ReadFormats( maStrm );

        ScBaseCell* pCell = XclImpStringHelper::CreateCell( *this, aString, nXF );
        if( pCell )
            GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), GetCurrScTab(), pCell );

        pColRowBuff->Used( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow) );
    }
    else
    {
        bTabTruncated = TRUE;
        GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
    }

    pLastFormCell = NULL;
}


void ImportExcel8::Label( void )
{
    UINT16  nRow, nCol, nXF;
    aIn >> nRow >> nCol >> nXF;

    if( nRow <= static_cast<sal_uInt16>(MAXROW) && nCol <= static_cast<sal_uInt16>(MAXCOL) )
    {
        GetXFIndexBuffer().SetXF( nCol, nRow, nXF );

        XclImpString aString( maStrm );
        ScBaseCell* pCell = XclImpStringHelper::CreateCell( *this, aString, nXF );
        if( pCell )
            GetDoc().PutCell( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), GetCurrScTab(), pCell );

        pColRowBuff->Used( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow) );

    }
    else
    {
        bTabTruncated = TRUE;
            GetTracer().TraceInvalidRow(GetCurrScTab(), nRow, MAXROW);
    }


    pLastFormCell = NULL;
}


void ImportExcel8::Txo( void )
{
    GetObjectManager().ReadTxo( maStrm );
}


void ImportExcel8::Codename( BOOL bWorkbookGlobals )
{
    if( bHasBasic )
    {
        String aName( aIn.ReadUniString() );

        if( bWorkbookGlobals )
            GetExtDocOptions().SetCodename( aName );
        else
            GetExtDocOptions().AddCodename( aName );
    }
}


void ImportExcel8::Dimensions( void )
{
    UINT32  nRowFirst, nRowLast;
    UINT16  nColFirst, nColLast;

    aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

    if( nRowLast > MAXROW )
        nRowLast = MAXROW;
    if( nColLast > static_cast<sal_uInt16>(MAXCOL) )
        nColLast = static_cast<sal_uInt16>(MAXCOL);
    if( nRowFirst > nRowLast )
        nRowFirst = nRowLast;
    if( nColFirst > nColLast )
        nColFirst = nColLast;

    pColRowBuff->SetDimension(
        ScRange( static_cast<SCCOL>(nColFirst), static_cast<SCROW>(nRowFirst), GetCurrScTab(), static_cast<SCCOL>(nColLast), static_cast<SCROW>(nRowLast), GetCurrScTab() ) );
}

void ImportExcel8::ReadBasic( void )
{
    bHasBasic = TRUE;

    SfxObjectShell* pShell = GetDocShell();

    if( pShell )
    {
        SvtFilterOptions* pFltOpts = SvtFilterOptions::Get();
        if( pFltOpts )
        {
            if( pFltOpts->IsLoadExcelBasicCode() || pFltOpts->IsLoadExcelBasicStorage() )
            {
                DBG_ASSERT( pExcRoot->pRootStorage, "-ImportExcel8::PostDocLoad(): no storage, no cookies!" );

                SvxImportMSVBasic   aBasicImport( *pShell, *pExcRoot->pRootStorage, pFltOpts->IsLoadExcelBasicCode(), pFltOpts->IsLoadExcelBasicStorage() );

                aBasicImport.Import( String::CreateFromAscii( pVBAStorageName ),
                                     String::CreateFromAscii( pVBASubStorageName ) );
            }
        }
    }
}


void ImportExcel8::EndSheet( void )
{
    GetCondFormatManager().Apply();
    ImportExcel::EndSheet();
}


void ImportExcel8::PostDocLoad( void )
{
    if( pExcRoot->pAutoFilterBuffer )
        pExcRoot->pAutoFilterBuffer->Apply();

    GetWebQueryBuffer().Apply();    //! test if extant

    ApplyEscherObjects();

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( !pD->IsClipboard() && aScenList.Count() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( GetRoot() );
    }

    SfxObjectShell* pShell = GetDocShell();

    // BASIC
    if( bHasBasic && pShell )
    {
        SvtFilterOptions*   pFiltOpt = SvtFilterOptions::Get();

        if( pFiltOpt )
        {
            if( pFiltOpt->IsLoadExcelBasicCode() || pFiltOpt->IsLoadExcelBasicStorage() )
            {
                DBG_ASSERT( GetRootStorage(), "-ImportExcel8::PostDocLoad(): no storage, no cookies!" );

                SvxImportMSVBasic   aBasicImport( *pShell, *GetRootStorage(),
                                                    pFiltOpt->IsLoadExcelBasicCode(),
                                                    pFiltOpt->IsLoadExcelBasicStorage() );

                aBasicImport.Import( EXC_STORAGE_VBA_PROJECT, EXC_STORAGE_VBA );
            }
        }
    }

    // read doc info
    // no docshell while pasting from clipboard
    if( pShell )
    {
        SfxDocumentInfo     aNewDocInfo;
        SfxDocumentInfo&    rOldDocInfo = pShell->GetDocInfo();

        aNewDocInfo.LoadPropertySet( GetRootStorage() );

        rOldDocInfo = aNewDocInfo;
        pShell->Broadcast( SfxDocumentInfoHint( &rOldDocInfo ) );
    }

    // building pivot tables
    GetPivotTableManager().Apply();
}


void ImportExcel8::ApplyEscherObjects()
{
    XclImpObjectManager& rObjManager = GetObjectManager();
    if( rObjManager.HasEscherStream() )
    {
        XclImpDffManager& rDffManager = rObjManager.GetDffManager();
        if( const SvxMSDffShapeInfos* pShapeInfos = rDffManager.GetShapeInfos() )
        {
            for( sal_uInt16 nInfo = 0, nInfoCount = pShapeInfos->Count(); nInfo < nInfoCount; ++nInfo )
            {
                if( const SvxMSDffShapeInfo* pShapeInfo = pShapeInfos->GetObject( nInfo ) )
                {
                    sal_uInt32 nShapeId = pShapeInfo->nShapeId;
                    XclImpEscherObj* pEscherObj = rObjManager.GetEscherObjAcc( pShapeInfo->nFilePos );
                    if( pEscherObj && !pEscherObj->GetIsSkip() && !pEscherObj->GetSdrObj() )
                    {
                        SvxMSDffImportData aDffImportData;
                        rDffManager.SetSdrObject( pEscherObj, nShapeId, aDffImportData );

                        // *** find some comboboxes to skip ***
                        if( const XclImpEscherTbxCtrl* pCtrlObj = PTR_CAST( XclImpEscherTbxCtrl, pEscherObj ) )
                        {
                            if( pCtrlObj->GetType() == EXC_OBJ_CMO_COMBOBOX )
                            {
                                if( const XclEscherAnchor* pAnchor = rObjManager.GetEscherAnchor( pShapeInfo->nFilePos ) )
                                {
                                    bool bSkipObj = false;
                                    if( pExcRoot->pAutoFilterBuffer )
                                        bSkipObj = pExcRoot->pAutoFilterBuffer->HasDropDown( static_cast<SCCOL>(pAnchor->mnLCol), static_cast<SCROW>(pAnchor->mnTRow), pAnchor->mnScTab );
                                    if( bSkipObj )
                                        pEscherObj->SetSkip();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    rObjManager.Apply();
}



void ImportExcel8::EndAllChartObjects( void )
{
}


//___________________________________________________________________
// autofilter

void ImportExcel8::FilterMode( void )
{
    // The FilterMode record exists: if either the AutoFilter
    // record exists or an Advanced Filter is saved and stored
    // in the sheet. Thus if the FilterMode records only exists
    // then the latter is true..
    if( !pExcRoot->pAutoFilterBuffer ) return;

    pExcRoot->pAutoFilterBuffer->IncrementActiveAF();

    XclImpAutoFilterData* pData = pExcRoot->pAutoFilterBuffer->GetByTab( GetCurrScTab() );
    if( pData )
        pData->SetAutoOrAdvanced();
}

void ImportExcel8::AutoFilterInfo( void )
{
    if( !pExcRoot->pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pExcRoot->pAutoFilterBuffer->GetByTab( GetCurrScTab() );
    if( pData )
    {
        pData->SetAdvancedRange( NULL );
        pData->Activate();
    }
}

void ImportExcel8::AutoFilter( void )
{
    if( !pExcRoot->pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pExcRoot->pAutoFilterBuffer->GetByTab( GetCurrScTab() );
    if( pData )
        pData->ReadAutoFilter( aIn );
}



XclImpAutoFilterData::XclImpAutoFilterData( RootData* pRoot, const ScRange& rRange, const String& rName ) :
        ExcRoot( pRoot ),
        pCurrDBData(NULL),
        nFirstEmpty( 0 ),
        bActive( FALSE ),
        bHasDropDown( FALSE ),
        bHasConflict( FALSE ),
        bCriteria( FALSE ),
        bAutoOrAdvanced(FALSE),
        aFilterName(rName)
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    // Excel defaults to always in place regardless
    // of whether an extract record exists. The user
    // must choose to explicity set the Copy To in the UI.
    aParam.bInplace = TRUE;

}

void XclImpAutoFilterData::CreateFromDouble( String& rStr, double fVal )
{
    rStr += String( ::rtl::math::doubleToUString( fVal,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), TRUE));
}

void XclImpAutoFilterData::SetCellAttribs()
{
    bHasDropDown = TRUE;
    for ( SCCOL nCol = StartCol(); nCol <= EndCol(); nCol++ )
    {
        INT16 nFlag = ((ScMergeFlagAttr*) pExcRoot->pDoc->
            GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG ))->GetValue();
        pExcRoot->pDoc->ApplyAttr( nCol, StartRow(), Tab(),
            ScMergeFlagAttr( nFlag | SC_MF_AUTO) );
    }
}

void XclImpAutoFilterData::InsertQueryParam()
{
    if( pCurrDBData && !bHasConflict )
    {
        ScRange aAdvRange;
        BOOL    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pDoc->CreateQueryParam( aAdvRange.aStart.Col(),
                aAdvRange.aStart.Row(), aAdvRange.aEnd.Col(), aAdvRange.aEnd.Row(),
                aAdvRange.aStart.Tab(), aParam );

        pCurrDBData->SetQueryParam( aParam );
        if( bHasAdv )
            pCurrDBData->SetAdvancedQuerySource( &aAdvRange );
        else
        {
            pCurrDBData->SetAutoFilter( TRUE );
            SetCellAttribs();
        }
    }
}

BOOL XclImpAutoFilterData::HasDropDown( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    return (bHasDropDown && (StartCol() <= nCol) && (nCol <= EndCol()) &&
            (nRow == StartRow()) && (nTab == Tab()));
}

void XclImpAutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
{
    UINT16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect  eConn       = (nFlags & EXC_AFFLAG_ANDORMASK) ? SC_OR : SC_AND;
    BOOL            bTop10      = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10 );
    BOOL            bTopOfTop10 = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10TOP );
    BOOL            bPercent    = TRUEBOOL( nFlags & EXC_AFFLAG_TOP10PERC );
    UINT16          nCntOfTop10 = nFlags >> 7;
    SCSIZE          nCount      = aParam.GetEntryCount();

    if( bTop10 )
    {
        if( nFirstEmpty < nCount )
        {
            ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
            aEntry.bDoQuery = TRUE;
            aEntry.bQueryByString = TRUE;
            aEntry.nField = static_cast<SCCOLROW>(StartCol() + static_cast<SCCOL>(nCol));
            aEntry.eOp = bTopOfTop10 ?
                (bPercent ? SC_TOPPERC : SC_TOPVAL) : (bPercent ? SC_BOTPERC : SC_BOTVAL);
            aEntry.eConnect = SC_AND;
            aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nCntOfTop10 ) );

            rStrm.Ignore( 20 );
            nFirstEmpty++;
        }
    }
    else
    {
        UINT8   nE, nType, nOper, nBoolErr, nVal;
        INT32   nRK;
        double  fVal;
        BOOL    bIgnore;

        UINT8   nStrLen[ 2 ]    = { 0, 0 };
        String* pEntryStr[ 2 ]  = { NULL, NULL };

        for( nE = 0; nE < 2; nE++ )
        {
            if( nFirstEmpty < nCount )
            {
                ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
                pEntryStr[ nE ] = aEntry.pStr;
                bIgnore = FALSE;

                rStrm >> nType >> nOper;
                switch( nOper )
                {
                    case EXC_AFOPER_LESS:
                        aEntry.eOp = SC_LESS;
                    break;
                    case EXC_AFOPER_EQUAL:
                        aEntry.eOp = SC_EQUAL;
                    break;
                    case EXC_AFOPER_LESSEQUAL:
                        aEntry.eOp = SC_LESS_EQUAL;
                    break;
                    case EXC_AFOPER_GREATER:
                        aEntry.eOp = SC_GREATER;
                    break;
                    case EXC_AFOPER_NOTEQUAL:
                        aEntry.eOp = SC_NOT_EQUAL;
                    break;
                    case EXC_AFOPER_GREATEREQUAL:
                        aEntry.eOp = SC_GREATER_EQUAL;
                    break;
                    default:
                        aEntry.eOp = SC_EQUAL;
                }

                switch( nType )
                {
                    case EXC_AFTYPE_RK:
                        rStrm >> nRK;
                        rStrm.Ignore( 4 );
                        CreateFromDouble( *aEntry.pStr, XclTools::GetDoubleFromRK( nRK ) );
                    break;
                    case EXC_AFTYPE_DOUBLE:
                        rStrm >> fVal;
                        CreateFromDouble( *aEntry.pStr, fVal );
                    break;
                    case EXC_AFTYPE_STRING:
                        rStrm.Ignore( 4 );
                        rStrm >> nStrLen[ nE ];
                        rStrm.Ignore( 3 );
                        aEntry.pStr->Erase();
                    break;
                    case EXC_AFTYPE_BOOLERR:
                        rStrm >> nBoolErr >> nVal;
                        rStrm.Ignore( 6 );
                        aEntry.pStr->Assign( String::CreateFromInt32( (sal_Int32) nVal ) );
                        bIgnore = (BOOL) nBoolErr;
                    break;
                    case EXC_AFTYPE_EMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_EMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    case EXC_AFTYPE_NOTEMPTY:
                        aEntry.bQueryByString = FALSE;
                        aEntry.nVal = SC_NONEMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    default:
                        rStrm.Ignore( 8 );
                        bIgnore = TRUE;
                }

                if( (nE > 0) && (nCol > 0) && (eConn == SC_OR) && !bIgnore )
                    bHasConflict = TRUE;
                if( !bHasConflict && !bIgnore )
                {
                    aEntry.bDoQuery = TRUE;
                    aEntry.bQueryByString = TRUE;
                    aEntry.nField = static_cast<SCCOLROW>(StartCol() + static_cast<SCCOL>(nCol));
                    aEntry.eConnect = nE ? eConn : SC_AND;
                    nFirstEmpty++;
                }
            }
            else
                rStrm.Ignore( 10 );
        }

        for( nE = 0; nE < 2; nE++ )
            if( nStrLen[ nE ] && pEntryStr[ nE ] )
                pEntryStr[ nE ]->Assign( rStrm.ReadUniString( nStrLen[ nE ] ) );
    }
}

void XclImpAutoFilterData::SetAdvancedRange( const ScRange* pRange )
{
    if (pRange)
    {
        aCriteriaRange = *pRange;
        bCriteria = TRUE;
    }
    else
        bCriteria = FALSE;
}

void XclImpAutoFilterData::SetExtractPos( const ScAddress& rAddr )
{
    aParam.nDestCol = rAddr.Col();
    aParam.nDestRow = rAddr.Row();
    aParam.nDestTab = rAddr.Tab();
    aParam.bDestPers = TRUE;

}

void XclImpAutoFilterData::Apply( const BOOL bUseUnNamed )
{
    CreateScDBData(bUseUnNamed);

    if( bActive )
    {
        InsertQueryParam();

        BYTE nFlags;
        for( SCROW nRow = StartRow(); nRow <= EndRow(); nRow++ )
        {
            nFlags = pExcRoot->pDoc->GetRowFlags( nRow, Tab() );
            if( nFlags & CR_HIDDEN )
                nFlags |= CR_FILTERED;
            pExcRoot->pDoc->SetRowFlags( nRow, Tab(), nFlags );
        }
    }
}

void XclImpAutoFilterData::CreateScDBData( const BOOL bUseUnNamed )
{

    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDBCollection& rColl = *pExcRoot->pDoc->GetDBCollection();
        pCurrDBData = rColl.GetDBAtArea( Tab(), StartCol(), StartRow(), EndCol(), EndRow() );
        if( !pCurrDBData )
        {
            AmendAFName(bUseUnNamed);

            pCurrDBData = new ScDBData( aFilterName, Tab(), StartCol(), StartRow(), EndCol(), EndRow() );

            if( pCurrDBData )
            {
                if(bCriteria)
                {
                    EnableRemoveFilter();

                    pCurrDBData->SetQueryParam( aParam );
                    pCurrDBData->SetAdvancedQuerySource(&aCriteriaRange);
                }
                else
                    pCurrDBData->SetAdvancedQuerySource(NULL);
                rColl.Insert( pCurrDBData );
            }
        }
    }

}

void XclImpAutoFilterData::EnableRemoveFilter()
{
    // only if this is a saved Advanced filter
    if( !bActive && bAutoOrAdvanced )
    {
        ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
        aEntry.bDoQuery = TRUE;
        ++nFirstEmpty;
    }

    // TBD: force the automatic activation of the
    // "Remove Filter" by setting a virtual mouse click
    // inside the advanced range
}

void XclImpAutoFilterData::AmendAFName(const BOOL bUseUnNamed)
{
    // If-and-only-if we have one AF filter then
    // use the Calc "unnamed" range name. Calc
    // only supports one in total while Excel
    // supports one per sheet.
    if( bUseUnNamed && bAutoOrAdvanced )
        aFilterName = ScGlobal::GetRscString(STR_DB_NONAME);
}

XclImpAutoFilterBuffer::XclImpAutoFilterBuffer() :
    nAFActiveCount( 0 )
{
}

XclImpAutoFilterBuffer::~XclImpAutoFilterBuffer()
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        delete pData;
}

void XclImpAutoFilterBuffer::Insert( RootData* pRoot, const ScRange& rRange,
                                    const String& rName )
{
    if( !GetByTab( rRange.aStart.Tab() ) )
        Append( new XclImpAutoFilterData( pRoot, rRange, rName ) );
}

void XclImpAutoFilterBuffer::AddAdvancedRange( const ScRange& rRange )
{
    XclImpAutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetAdvancedRange( &rRange );
}

void XclImpAutoFilterBuffer::AddExtractPos( const ScRange& rRange )
{
    XclImpAutoFilterData* pData = GetByTab( rRange.aStart.Tab() );
    if( pData )
        pData->SetExtractPos( rRange.aStart );
}

void XclImpAutoFilterBuffer::Apply()
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        pData->Apply(UseUnNamed());
}

XclImpAutoFilterData* XclImpAutoFilterBuffer::GetByTab( SCTAB nTab )
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->Tab() == nTab )
            return pData;
    return NULL;
}

BOOL XclImpAutoFilterBuffer::HasDropDown( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    for( XclImpAutoFilterData* pData = _First(); pData; pData = _Next() )
        if( pData->HasDropDown( nCol, nRow, nTab ) )
            return TRUE;
    return FALSE;
}

