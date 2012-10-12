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


#include "excimp8.hxx"

#include <boost/bind.hpp>

#include <scitems.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/fltrcfg.hxx>

#include <svtools/wmf.hxx>

#include <editeng/eeitem.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/frame.hxx>

#include <editeng/brshitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/flditem.hxx>
#include <svx/xflclit.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <sot/exchange.hxx>

#include <svl/stritem.hxx>

#include <tools/string.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/charclass.hxx>
#include <drwlayer.hxx>

#include <boost/scoped_array.hpp>

#include "cell.hxx"
#include "document.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "conditio.hxx"
#include "dbdata.hxx"
#include "globalnames.hxx"
#include "editutil.hxx"
#include "markdata.hxx"
#include "rangenam.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "fprogressbar.hxx"
#include "xltracer.hxx"
#include "xihelper.hxx"
#include "xipage.hxx"
#include "xicontent.hxx"
#include "xilink.hxx"
#include "xiescher.hxx"
#include "xipivot.hxx"

#include "excform.hxx"
#include "scextopt.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "detfunc.hxx"
#include "macromgr.hxx"
#include "queryentry.hxx"

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <cppuhelper/component_context.hxx>
#include "xltoolbar.hxx"
#include <oox/ole/vbaproject.hxx>
#include <oox/ole/olestorage.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;
using namespace ::comphelper;
using ::rtl::OUString;

//OleNameOverrideContainer

typedef ::cppu::WeakImplHelper1< container::XNameContainer > OleNameOverrideContainer_BASE;

class OleNameOverrideContainer : public OleNameOverrideContainer_BASE
{
private:
    typedef boost::unordered_map< rtl::OUString, uno::Reference< container::XIndexContainer >, ::rtl::OUStringHash,
       ::std::equal_to< ::rtl::OUString > > NamedIndexToOleName;
    NamedIndexToOleName  IdToOleNameHash;
    ::osl::Mutex m_aMutex;
public:
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException) { return  container::XIndexContainer::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( IdToOleNameHash.size() > 0 );
    }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return uno::makeAny( IdToOleNameHash[ aName ] );
    }
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        uno::Sequence< ::rtl::OUString > aResult( IdToOleNameHash.size() );
        NamedIndexToOleName::iterator it = IdToOleNameHash.begin();
        NamedIndexToOleName::iterator it_end = IdToOleNameHash.end();
        rtl::OUString* pName = aResult.getArray();
        for (; it != it_end; ++it, ++pName )
            *pName = it->first;
        return aResult;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( IdToOleNameHash.find( aName ) != IdToOleNameHash.end() );
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return IdToOleNameHash.size();
    }
    // XNameContainer
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw container::ElementExistException();
        uno::Reference< container::XIndexContainer > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw lang::IllegalArgumentException();
       IdToOleNameHash[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const ::rtl::OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        IdToOleNameHash.erase( IdToOleNameHash.find( aName ) );
    }
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        uno::Reference< container::XIndexContainer > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw lang::IllegalArgumentException();
        IdToOleNameHash[ aName ] = xElement;
    }
};

// defined in docfunc.cxx ( really this needs a new name )
static script::ModuleInfo lcl_InitModuleInfo( SfxObjectShell& rDocSh, String& sModule );

ImportExcel8::ImportExcel8( XclImpRootData& rImpData, SvStream& rStrm ) :
    ImportExcel( rImpData, rStrm )
{
    // replace BIFF2-BIFF5 formula importer with BIFF8 formula importer
    delete pFormConv;
    pFormConv = pExcRoot->pFmlaConverter = new ExcelToSc8( GetRoot() );
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


void ImportExcel8::Boundsheet( void )
{
    sal_uInt8           nLen;
    sal_uInt16          nGrbit;

    aIn.DisableDecryption();
    maSheetOffsets.push_back( aIn.ReaduInt32() );
    aIn.EnableDecryption();
    aIn >> nGrbit >> nLen;

    rtl::OUString aName( aIn.ReadUniString( nLen ) );
    GetTabInfo().AppendXclTabName( aName, nBdshtTab );

    SCTAB nScTab = static_cast< SCTAB >( nBdshtTab );
    if( nScTab > 0 )
    {
        OSL_ENSURE( !pD->HasTable( nScTab ), "ImportExcel8::Boundsheet - sheet exists already" );
        pD->MakeTable( nScTab );
    }

    if( ( nGrbit & 0x0001 ) || ( nGrbit & 0x0002 ) )
        pD->SetVisible( nScTab, false );

    if( !pD->RenameTab( nScTab, aName ) )
    {
        pD->CreateValidTabName( aName );
        pD->RenameTab( nScTab, aName );
    }

    nBdshtTab++;
}


void ImportExcel8::Scenman( void )
{
    sal_uInt16              nLastDispl;

    aIn.Ignore( 4 );
    aIn >> nLastDispl;

    maScenList.nLastScenario = nLastDispl;
}


void ImportExcel8::Scenario( void )
{
    maScenList.aEntries.push_back( new ExcScenario( aIn, *pExcRoot ) );
}


void ImportExcel8::Labelsst( void )
{
    XclAddress aXclPos;
    sal_uInt16 nXF;
    sal_uInt32  nSst;

    aIn >> aXclPos >> nXF >> nSst;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        GetXFRangeBuffer().SetXF( aScPos, nXF );
        if( ScBaseCell* pCell = GetSst().CreateCell( nSst, nXF ) )
            GetDoc().PutCell( aScPos.Col(), aScPos.Row(), aScPos.Tab(), pCell );
    }
}


void ImportExcel8::SheetProtection( void )
{
    GetSheetProtectBuffer().ReadOptions( aIn, GetCurrScTab() );
}

void ImportExcel8::ReadBasic( void )
{
    SfxObjectShell* pShell = GetDocShell();
    SotStorageRef xRootStrg = GetRootStorage();
    const SvtFilterOptions& rFilterOpt = SvtFilterOptions::Get();
    if( pShell && xRootStrg.Is() ) try
    {
        bool bLoadCode = rFilterOpt.IsLoadExcelBasicCode();
        bool bLoadExecutable = rFilterOpt.IsLoadExcelBasicExecutable();
        bool bLoadStrg = rFilterOpt.IsLoadExcelBasicStorage();
        // #FIXME need to get rid of this, we can also do this from within oox
        // via the "ooo.vba.VBAGlobals" service
        if( bLoadCode || bLoadStrg )
        {
            bool bAsComment = !bLoadExecutable;

            if ( !bAsComment )
            {
                // see if we have the XCB stream
                SvStorageStreamRef xXCB = xRootStrg->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM( "XCB" ) ), STREAM_STD_READ | STREAM_NOCREATE  );
                if ( xXCB.Is()|| SVSTREAM_OK == xXCB->GetError() )
                {
                    ScCTBWrapper wrapper;
                    if ( wrapper.Read( *xXCB ) )
                    {
#if OSL_DEBUG_LEVEL > 1
                        wrapper.Print( stderr );
#endif
                        wrapper.ImportCustomToolBar( *pShell );
                    }
                }
            }
        }
        try
        {
            uno::Reference< uno::XComponentContext > aCtx( ::comphelper::getProcessComponentContext() );
            SfxMedium& rMedium = GetMedium();
            uno::Reference< io::XInputStream > xIn = rMedium.GetInputStream();
            oox::ole::OleStorage root( aCtx, xIn, false );
            oox::StorageRef vbaStg = root.openSubStorage( "_VBA_PROJECT_CUR", false );
            if ( vbaStg.get() )
            {
                oox::ole::VbaProject aVbaPrj( aCtx, pShell->GetModel(), "Calc" );
                // collect names of embedded form controls, as specified in the VBA project
                uno::Reference< container::XNameContainer > xOleNameOverrideSink( new OleNameOverrideContainer );
                aVbaPrj.setOleOverridesSink( xOleNameOverrideSink );
                aVbaPrj.importVbaProject( *vbaStg );
                GetObjectManager().SetOleNameOverrideInfo( xOleNameOverrideSink );
            }
        }
        catch( uno::Exception& )
        {
        }
    }
    catch( uno::Exception& )
    {
    }
}


void ImportExcel8::EndSheet( void )
{
    ImportExcel::EndSheet();
    GetCondFormatManager().Apply();
    GetValidationManager().Apply();
}


void ImportExcel8::PostDocLoad( void )
{
#ifndef DISABLE_SCRIPTING
    // reading basic has been delayed until sheet objects (codenames etc.) are read
    if( HasBasic() )
        ReadBasic();
#endif
    // #i11776# filtered ranges before outlines and hidden rows
    if( pExcRoot->pAutoFilterBuffer )
        pExcRoot->pAutoFilterBuffer->Apply();

    GetWebQueryBuffer().Apply();    //! test if extant
    GetSheetProtectBuffer().Apply();
    GetDocProtectBuffer().Apply();

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( !pD->IsClipboard() && maScenList.aEntries.size() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        maScenList.Apply( GetRoot() );
    }

    // read doc info (no docshell while pasting from clipboard)
    LoadDocumentProperties();

    // #i45843# Pivot tables are now handled outside of PostDocLoad, so they are available
    // when formula cells are calculated, for the GETPIVOTDATA function.
}

void ImportExcel8::LoadDocumentProperties()
{
    // no docshell while pasting from clipboard
    if( SfxObjectShell* pShell = GetDocShell() )
    {
        // BIFF5+ without storage is possible
        SotStorageRef xRootStrg = GetRootStorage();
        if( xRootStrg.Is() ) try
        {
            uno::Reference< document::XDocumentPropertiesSupplier > xDPS( pShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< document::XDocumentProperties > xDocProps( xDPS->getDocumentProperties(), uno::UNO_SET_THROW );
            sfx2::LoadOlePropertySet( xDocProps, xRootStrg );
        }
        catch( uno::Exception& )
        {
        }
    }
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



XclImpAutoFilterData::XclImpAutoFilterData( RootData* pRoot, const ScRange& rRange ) :
        ExcRoot( pRoot ),
        pCurrDBData(NULL),
        bActive( false ),
        bCriteria( false ),
        bAutoOrAdvanced(false)
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    aParam.bInplace = sal_True;

}

void XclImpAutoFilterData::CreateFromDouble( rtl::OUString& rStr, double fVal )
{
    rStr += ::rtl::math::doubleToUString(fVal,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::pLocaleData->getNumDecimalSep()[0], true);
}

void XclImpAutoFilterData::SetCellAttribs()
{
    ScDocument& rDoc = pExcRoot->pIR->GetDoc();
    for ( SCCOL nCol = StartCol(); nCol <= EndCol(); nCol++ )
    {
        sal_Int16 nFlag = ((ScMergeFlagAttr*) rDoc.GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG ))->GetValue();
        rDoc.ApplyAttr( nCol, StartRow(), Tab(), ScMergeFlagAttr( nFlag | SC_MF_AUTO) );
    }
}

void XclImpAutoFilterData::InsertQueryParam()
{
    if (pCurrDBData)
    {
        ScRange aAdvRange;
        sal_Bool    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pIR->GetDoc().CreateQueryParam( aAdvRange.aStart.Col(),
                aAdvRange.aStart.Row(), aAdvRange.aEnd.Col(), aAdvRange.aEnd.Row(),
                aAdvRange.aStart.Tab(), aParam );

        pCurrDBData->SetQueryParam( aParam );
        if( bHasAdv )
            pCurrDBData->SetAdvancedQuerySource( &aAdvRange );
        else
        {
            pCurrDBData->SetAutoFilter( sal_True );
            SetCellAttribs();
        }
    }
}

static void ExcelQueryToOooQuery( ScQueryEntry& rEntry )
{
    if (rEntry.eOp != SC_EQUAL && rEntry.eOp != SC_NOT_EQUAL)
        return;

    String aStr = rEntry.GetQueryItem().maString;
    xub_StrLen nLen = aStr.Len();
    sal_Unicode nStart = aStr.GetChar( 0 );
    sal_Unicode nEnd   = aStr.GetChar( nLen-1 );
    if( nLen >2 && nStart == '*' && nEnd == '*' )
    {
        aStr.Erase( nLen-1, 1 );
        aStr.Erase( 0, 1 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_CONTAINS : SC_DOES_NOT_CONTAIN;
    }
    else if( nLen > 1 && nStart == '*' && nEnd != '*' )
    {
        aStr.Erase( 0, 1 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_ENDS_WITH : SC_DOES_NOT_END_WITH;
    }
    else if( nLen > 1 && nStart != '*' && nEnd == '*' )
    {
        aStr.Erase( nLen-1, 1 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_BEGINS_WITH : SC_DOES_NOT_BEGIN_WITH;
    }
    else if( nLen == 2 && nStart == '*' && nEnd == '*' )
    {
        aStr.Erase( 0, 1 );
    }
    rEntry.GetQueryItem().maString = aStr;
}

void XclImpAutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
{
    sal_uInt16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect eConn = ::get_flagvalue( nFlags, EXC_AFFLAG_ANDORMASK, SC_OR, SC_AND );
    bool bSimple1    = ::get_flag(nFlags, EXC_AFFLAG_SIMPLE1);
    bool bSimple2    = ::get_flag(nFlags, EXC_AFFLAG_SIMPLE2);
    bool bTop10      = ::get_flag(nFlags, EXC_AFFLAG_TOP10);
    bool bTopOfTop10 = ::get_flag(nFlags, EXC_AFFLAG_TOP10TOP);
    bool bPercent    = ::get_flag(nFlags, EXC_AFFLAG_TOP10PERC);
    sal_uInt16 nCntOfTop10 = nFlags >> 7;

    if( bTop10 )
    {
        ScQueryEntry& aEntry = aParam.AppendEntry();
        ScQueryEntry::Item& rItem = aEntry.GetQueryItem();
        aEntry.bDoQuery = true;
        aEntry.nField = static_cast<SCCOLROW>(StartCol() + static_cast<SCCOL>(nCol));
        aEntry.eOp = bTopOfTop10 ?
            (bPercent ? SC_TOPPERC : SC_TOPVAL) : (bPercent ? SC_BOTPERC : SC_BOTVAL);
        aEntry.eConnect = SC_AND;

        rItem.meType = ScQueryEntry::ByString;
        rItem.maString = rtl::OUString::valueOf(static_cast<sal_Int32>(nCntOfTop10));

        rStrm.Ignore(20);
        return;
    }

    sal_uInt8   nType, nOper, nBoolErr, nVal;
    sal_Int32   nRK;
    double  fVal;
    bool bIgnore;

    sal_uInt8 nStrLen[2] = { 0, 0 };
    ScQueryEntry aEntries[2];

    for (size_t nE = 0; nE < 2; ++nE)
    {
        ScQueryEntry& rEntry = aEntries[nE];
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        bIgnore = false;

        rStrm >> nType >> nOper;
        switch( nOper )
        {
            case EXC_AFOPER_LESS:
                rEntry.eOp = SC_LESS;
            break;
            case EXC_AFOPER_EQUAL:
                rEntry.eOp = SC_EQUAL;
            break;
            case EXC_AFOPER_LESSEQUAL:
                rEntry.eOp = SC_LESS_EQUAL;
            break;
            case EXC_AFOPER_GREATER:
                rEntry.eOp = SC_GREATER;
            break;
            case EXC_AFOPER_NOTEQUAL:
                rEntry.eOp = SC_NOT_EQUAL;
            break;
            case EXC_AFOPER_GREATEREQUAL:
                rEntry.eOp = SC_GREATER_EQUAL;
            break;
            default:
                rEntry.eOp = SC_EQUAL;
        }

        rtl::OUString aStr;

        switch( nType )
        {
            case EXC_AFTYPE_RK:
                rStrm >> nRK;
                rStrm.Ignore( 4 );
                CreateFromDouble(
                    rItem.maString, XclTools::GetDoubleFromRK(nRK));
            break;
            case EXC_AFTYPE_DOUBLE:
                rStrm >> fVal;
                CreateFromDouble(rItem.maString, fVal);
            break;
            case EXC_AFTYPE_STRING:
                rStrm.Ignore( 4 );
                rStrm >> nStrLen[ nE ];
                rStrm.Ignore( 3 );
                rItem.maString = rtl::OUString();
            break;
            case EXC_AFTYPE_BOOLERR:
                rStrm >> nBoolErr >> nVal;
                rStrm.Ignore( 6 );
                rItem.maString = rtl::OUString::valueOf(static_cast<sal_Int32>(nVal));
                bIgnore = (nBoolErr != 0);
            break;
            case EXC_AFTYPE_EMPTY:
                rEntry.SetQueryByEmpty();
            break;
            case EXC_AFTYPE_NOTEMPTY:
                rEntry.SetQueryByNonEmpty();
            break;
            default:
                rStrm.Ignore( 8 );
                bIgnore = true;
        }

        if (!bIgnore)
        {
            rEntry.bDoQuery = true;
            rItem.meType = ScQueryEntry::ByString;
            rEntry.nField = static_cast<SCCOLROW>(StartCol() + static_cast<SCCOL>(nCol));
            rEntry.eConnect = nE ? eConn : SC_AND;
        }
    }

    if (eConn == SC_AND)
    {
        for (size_t nE = 0; nE < 2; ++nE)
        {
            if (nStrLen[nE] && aEntries[nE].bDoQuery)
            {
                aEntries[nE].GetQueryItem().maString = rStrm.ReadUniString(nStrLen[nE]);
                ExcelQueryToOooQuery(aEntries[nE]);
                aParam.AppendEntry() = aEntries[nE];
            }
        }
    }
    else
    {
        OSL_ASSERT(eConn == SC_OR);
        // Import only when both conditions are for simple equality, else
        // import only the 1st condition due to conflict with the ordering of
        // conditions. #i39464#.
        //
        // Example: Let A1 be a condition of column A, and B1 and B2
        // conditions of column B, connected with OR. Excel performs 'A1 AND
        // (B1 OR B2)' in this case, but Calc would do '(A1 AND B1) OR B2'
        // instead.

        if (bSimple1 && bSimple2 && nStrLen[0] && nStrLen[1])
        {
            // Two simple OR'ed equal conditions.  We can import this correctly.
            ScQueryEntry& rEntry = aParam.AppendEntry();
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_EQUAL;
            rEntry.eConnect = SC_AND;
            ScQueryEntry::QueryItemsType aItems;
            aItems.reserve(2);
            ScQueryEntry::Item aItem1, aItem2;
            aItem1.maString = rStrm.ReadUniString(nStrLen[0]);
            aItem1.meType = ScQueryEntry::ByString;
            aItem2.maString = rStrm.ReadUniString(nStrLen[1]);
            aItem2.meType = ScQueryEntry::ByString;
            aItems.push_back(aItem1);
            aItems.push_back(aItem2);
            rEntry.GetQueryItems().swap(aItems);
        }
        else if (nStrLen[0] && aEntries[0].bDoQuery)
        {
            // Due to conflict, we can import only the first condition.
            aEntries[0].GetQueryItem().maString = rStrm.ReadUniString(nStrLen[0]);
            ExcelQueryToOooQuery(aEntries[0]);
            aParam.AppendEntry() = aEntries[0];
        }
    }
}

void XclImpAutoFilterData::SetAdvancedRange( const ScRange* pRange )
{
    if (pRange)
    {
        aCriteriaRange = *pRange;
        bCriteria = true;
    }
    else
        bCriteria = false;
}

void XclImpAutoFilterData::SetExtractPos( const ScAddress& rAddr )
{
    aParam.nDestCol = rAddr.Col();
    aParam.nDestRow = rAddr.Row();
    aParam.nDestTab = rAddr.Tab();
    aParam.bInplace = false;
    aParam.bDestPers = sal_True;
}

void XclImpAutoFilterData::Apply()
{
    CreateScDBData();

    if( bActive )
    {
        InsertQueryParam();

        // #i38093# rows hidden by filter need extra flag, but CR_FILTERED is not set here yet
//        SCROW nRow1 = StartRow();
//        SCROW nRow2 = EndRow();
//        size_t nRows = nRow2 - nRow1 + 1;
//        boost::scoped_array<sal_uInt8> pFlags( new sal_uInt8[nRows]);
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

void XclImpAutoFilterData::CreateScDBData()
{

    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDocument* pDoc = pExcRoot->pIR->GetDocPtr();
        String aNewName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(STR_DB_LOCAL_NONAME));
        pCurrDBData = new ScDBData(aNewName , Tab(),
                                StartCol(),StartRow(), EndCol(),EndRow() );
        if(bCriteria)
        {
            EnableRemoveFilter();

            pCurrDBData->SetQueryParam( aParam );
            pCurrDBData->SetAdvancedQuerySource(&aCriteriaRange);
        }
        else
            pCurrDBData->SetAdvancedQuerySource(NULL);
        pDoc->SetAnonymousDBData(Tab(), pCurrDBData);
    }

}

void XclImpAutoFilterData::EnableRemoveFilter()
{
    // only if this is a saved Advanced filter
    if( !bActive && bAutoOrAdvanced )
    {
        ScQueryEntry& aEntry = aParam.AppendEntry();
        aEntry.bDoQuery = true;
    }

    // TBD: force the automatic activation of the
    // "Remove Filter" by setting a virtual mouse click
    // inside the advanced range
}

void XclImpAutoFilterBuffer::Insert( RootData* pRoot, const ScRange& rRange)
{
    if( !GetByTab( rRange.aStart.Tab() ) )
        maFilters.push_back( new XclImpAutoFilterData( pRoot, rRange) );
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
    std::for_each(maFilters.begin(),maFilters.end(),
        boost::bind(&XclImpAutoFilterData::Apply,_1));
}

XclImpAutoFilterData* XclImpAutoFilterBuffer::GetByTab( SCTAB nTab )
{
    boost::ptr_vector<XclImpAutoFilterData>::iterator it;
    for( it = maFilters.begin(); it != maFilters.end(); ++it )
    {
        if( it->Tab() == nTab )
            return const_cast<XclImpAutoFilterData*>(&(*it));
    }
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
