/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excimp8.cxx,v $
 *
 *  $Revision: 1.122 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:24:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "excimp8.hxx"


#ifndef SC_ITEMS_HXX
#include <scitems.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#include <svtools/fltrcfg.hxx>

#include <svtools/wmf.hxx>

#include <svx/eeitem.hxx>

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

#include <sfx2/docinf.hxx>

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

#include <boost/scoped_array.hpp>

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
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_FPROGRESSBAR_HXX
#include "fprogressbar.hxx"
#endif
#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif
#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XIPIVOT_HXX
#include "xipivot.hxx"
#endif

#include "excform.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "detfunc.hxx"

using namespace com::sun::star;


#define INVALID_POS     0xFFFFFFFF




ImportExcel8::ImportExcel8( XclImpRootData& rImpData, SvStream& rStrm ) :
    ImportExcel( rImpData, rStrm )
{
    delete pFormConv;

    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc8( GetRoot() );

    bHasBasic = FALSE;
}


ImportExcel8::~ImportExcel8()
{
}


void ImportExcel8::Calccount( void )
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterCount( aIn.ReaduInt16() );
    pD->SetDocOptions( aOpt );
}


void ImportExcel8::Precision( void )
{
    ScDocOptions aOpt = pD->GetDocOptions();
    aOpt.SetCalcAsShown( aIn.ReaduInt16() == 0 );
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
    if( aIn.ReaduInt16() != 0 )
        GetExtDocOptions().GetDocSettings().mbWinProtected = true;
}

void ImportExcel8::Note( void )
{
    GetObjectManager().ReadNote( maStrm );
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

    SCTAB nScTab = static_cast< SCTAB >( nBdshtTab );
    if( nScTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nScTab ), "ImportExcel8::Boundsheet - sheet exists already" );
        pD->MakeTable( nScTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nScTab, FALSE );

    if( !pD->RenameTab( nScTab, aName ) )
    {
        pD->CreateValidTabName( aName );
        pD->RenameTab( nScTab, aName );
    }

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


void ImportExcel8::Labelsst( void )
{
    XclAddress aXclPos;
    UINT16 nXF;
    UINT32  nSst;

    aIn >> aXclPos >> nXF >> nSst;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        GetXFRangeBuffer().SetXF( aScPos, nXF );
        if( ScBaseCell* pCell = GetSst().CreateCell( nSst, nXF ) )
            GetDoc().PutCell( aScPos.Col(), aScPos.Row(), aScPos.Tab(), pCell );
    }
}


void ImportExcel8::Codename( BOOL bWorkbookGlobals )
{
    if( bHasBasic )
    {
        String aName( aIn.ReadUniString() );
        if( aName.Len() )
        {
            if( bWorkbookGlobals )
                GetExtDocOptions().GetDocSettings().maGlobCodeName = aName;
            else
                GetExtDocOptions().AppendCodeName( aName );
        }
    }
}

bool lcl_hasVBAEnabled()
{
    uno::Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY);
        // test if vba service is present
    uno::Reference< uno::XComponentContext > xCtx( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xGlobals( xCtx->getValueByName( ::rtl::OUString::createFromAscii( "/singletons/org.openoffice.vba.theGlobals") ), uno::UNO_QUERY );

    return xGlobals.is();
}

void ImportExcel8::ReadBasic( void )
{
    bHasBasic = TRUE;

    SfxObjectShell* pShell = GetDocShell();
    SotStorageRef xRootStrg = GetRootStorage();
    SvtFilterOptions* pFilterOpt = SvtFilterOptions::Get();
    if( pShell && xRootStrg.Is() && pFilterOpt )
    {
        bool bLoadCode = pFilterOpt->IsLoadExcelBasicCode();
        bool bLoadStrg = pFilterOpt->IsLoadExcelBasicStorage();
        if( bLoadCode || bLoadStrg )
        {
            SvxImportMSVBasic aBasicImport( *pShell, *xRootStrg, bLoadCode, bLoadStrg );
            aBasicImport.Import( EXC_STORAGE_VBA_PROJECT, EXC_STORAGE_VBA, !lcl_hasVBAEnabled() );
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
    // #i11776# filtered ranges before outlines and hidden rows
    if( pExcRoot->pAutoFilterBuffer )
        pExcRoot->pAutoFilterBuffer->Apply();

    GetWebQueryBuffer().Apply();    //! test if extant

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( !pD->IsClipboard() && aScenList.Count() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( GetRoot() );
    }

    // read doc info (no docshell while pasting from clipboard)
    if( SfxObjectShell* pShell = GetDocShell() )
    {
        // BIFF5+ without storage is possible
        SotStorageRef xRootStrg = GetRootStorage();
        if( xRootStrg.Is() )
        {
            SfxDocumentInfo aNewDocInfo;
            SfxDocumentInfo& rOldDocInfo = pShell->GetDocInfo();
            aNewDocInfo.LoadPropertySet( GetRootStorage() );
            rOldDocInfo = aNewDocInfo;
        }
    }

    // #i45843# Pivot tables are now handled outside of PostDocLoad, so they are available
    // when formula cells are calculated, for the GETPIVOTDATA function.
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
    ScDocument& rDoc = pExcRoot->pIR->GetDoc();
    for ( SCCOL nCol = StartCol(); nCol <= EndCol(); nCol++ )
    {
        INT16 nFlag = ((ScMergeFlagAttr*) rDoc.GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG ))->GetValue();
        rDoc.ApplyAttr( nCol, StartRow(), Tab(), ScMergeFlagAttr( nFlag | SC_MF_AUTO) );
    }
}

void XclImpAutoFilterData::InsertQueryParam()
{
    if( pCurrDBData && !bHasConflict )
    {
        ScRange aAdvRange;
        BOOL    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pIR->GetDoc().CreateQueryParam( aAdvRange.aStart.Col(),
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

void XclImpAutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
{
    UINT16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect  eConn       = ::get_flagvalue( nFlags, EXC_AFFLAG_ANDORMASK, SC_OR, SC_AND );
    BOOL            bTop10      = ::get_flag( nFlags, EXC_AFFLAG_TOP10 );
    BOOL            bTopOfTop10 = ::get_flag( nFlags, EXC_AFFLAG_TOP10TOP );
    BOOL            bPercent    = ::get_flag( nFlags, EXC_AFFLAG_TOP10PERC );
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

                /*  #i39464# conflict, if two conditions of one column are 'OR'ed,
                    and they follow conditions of other columns.
                    Example: Let A1 be a condition of column A, and B1 and B2
                    conditions of column B, connected with OR. Excel performs
                    'A1 AND (B1 OR B2)' in this case, but Calc would do
                    '(A1 AND B1) OR B2' instead. */
                if( (nFirstEmpty > 1) && nE && (eConn == SC_OR) && !bIgnore )
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
    aParam.bInplace = FALSE;
    aParam.bDestPers = TRUE;
}

void XclImpAutoFilterData::Apply( const BOOL bUseUnNamed )
{
    CreateScDBData(bUseUnNamed);

    if( bActive )
    {
        InsertQueryParam();

        // #i38093# rows hidden by filter need extra flag, but CR_FILTERED is not set here yet
//        SCROW nRow1 = StartRow();
//        SCROW nRow2 = EndRow();
//        size_t nRows = nRow2 - nRow1 + 1;
//        boost::scoped_array<BYTE> pFlags( new BYTE[nRows]);
//        pExcRoot->pDoc->GetRowFlagsArray( Tab()).FillDataArray( nRow1, nRow2,
//                pFlags.get());
//        for (size_t j=0; j<nRows; ++j)
//        {
//            if ((pFlags[j] & CR_HIDDEN) && !(pFlags[j] & CR_FILTERED))
//                pExcRoot->pDoc->SetRowFlags( nRow1 + j, Tab(),
//                        pFlags[j] | CR_FILTERED );
//        }
    }
}

void XclImpAutoFilterData::CreateScDBData( const BOOL bUseUnNamed )
{

    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDBCollection& rColl = pExcRoot->pIR->GetDatabaseRanges();
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

