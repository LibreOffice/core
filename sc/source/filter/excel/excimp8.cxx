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

#include "excimp8.hxx"

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
#include <tools/urlobj.hxx>
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
#include "dbcolect.hxx"
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

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <cppuhelper/component_context.hxx>
#include <sfx2/app.hxx>
#include "xltoolbar.hxx"


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
script::ModuleInfo lcl_InitModuleInfo( SfxObjectShell& rDocSh, String& sModule );

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

    String aName( aIn.ReadUniString( nLen ) );
    GetTabInfo().AppendXclTabName( aName, nBdshtTab );

    SCTAB nScTab = static_cast< SCTAB >( nBdshtTab );
    if( nScTab > 0 )
    {
        DBG_ASSERT( !pD->HasTable( nScTab ), "ImportExcel8::Boundsheet - sheet exists already" );
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

    aScenList.nLastScenario = nLastDispl;
}


void ImportExcel8::Scenario( void )
{
    aScenList.aEntries.push_back( new ExcScenario( aIn, *pExcRoot ) );
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
    if( pShell && xRootStrg.Is() ) try
    {
        bool bLoadCode = pFilterOpt->IsLoadExcelBasicCode();
        bool bLoadExecutable = pFilterOpt->IsLoadExcelBasicExecutable();
        bool bLoadStrg = pFilterOpt->IsLoadExcelBasicStorage();
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
                    CTBWrapper wrapper;
                    if ( wrapper.Read( xXCB ) )
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
            uno::Reference< lang::XComponent > xComponent( pShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Sequence< beans::NamedValue > aArgSeq( 1 );

            // collect names of embedded form controls, as specified in the VBA project
            aArgSeq[ 0 ].Name = CREATE_OUSTRING( "OleNameOverrideInfo" );
            uno::Reference< container::XNameContainer > xOleNameOverrideSink( new OleNameOverrideContainer );
            aArgSeq[ 0 ].Value <<= xOleNameOverrideSink;

            uno::Sequence< uno::Any > aArgs( 2 );
            // framework calls filter objects with factory as first argument
            aArgs[ 0 ] <<= getProcessServiceFactory();
            aArgs[ 1 ] <<= aArgSeq;

            uno::Reference< document::XImporter > xImporter( ScfApiHelper::CreateInstanceWithArgs( CREATE_OUSTRING( "com.sun.star.comp.oox.ExcelVBAProjectFilter" ), aArgs ), uno::UNO_QUERY_THROW );
            xImporter->setTargetDocument( xComponent );

            MediaDescriptor aMediaDesc;
            SfxMedium& rMedium = GetMedium();
            SfxItemSet* pItemSet = rMedium.GetItemSet();
            if( pItemSet )
            {
                if( const SfxStringItem* pItem = static_cast< const SfxStringItem* >( pItemSet->GetItem( SID_FILE_NAME ) ) )
                    aMediaDesc[ MediaDescriptor::PROP_URL() ] <<= ::rtl::OUString( pItem->GetValue() );
                if( const SfxStringItem* pItem = static_cast< const SfxStringItem* >( pItemSet->GetItem( SID_PASSWORD ) ) )
                    aMediaDesc[ MediaDescriptor::PROP_PASSWORD() ] <<= ::rtl::OUString( pItem->GetValue() );
            }
            aMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ] <<= rMedium.GetInputStream();
            aMediaDesc[ MediaDescriptor::PROP_INTERACTIONHANDLER() ] <<= rMedium.GetInteractionHandler();

            // call the filter
            uno::Reference< document::XFilter > xFilter( xImporter, uno::UNO_QUERY_THROW );
            xFilter->filter( aMediaDesc.getAsConstPropertyValueList() );
            GetObjectManager().SetOleNameOverrideInfo( xOleNameOverrideSink );
        }
        catch( uno::Exception& )
        {
        }
        aMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ] <<= rMedium.GetInputStream();
        aMediaDesc[ MediaDescriptor::PROP_INTERACTIONHANDLER() ] <<= rMedium.GetInteractionHandler();

        // call the filter
        uno::Reference< document::XFilter > xFilter( xImporter, uno::UNO_QUERY_THROW );
        xFilter->filter( aMediaDesc.getAsConstPropertyValueList() );
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
    // reading basic has been delayed until sheet objects (codenames etc.) are read
    if( HasBasic() )
        ReadBasic();
    // #i11776# filtered ranges before outlines and hidden rows
    if( pExcRoot->pAutoFilterBuffer )
        pExcRoot->pAutoFilterBuffer->Apply();

    GetWebQueryBuffer().Apply();    //! test if extant
    GetSheetProtectBuffer().Apply();
    GetDocProtectBuffer().Apply();

    ImportExcel::PostDocLoad();

    // Scenarien bemachen! ACHTUNG: Hier wird Tabellen-Anzahl im Dokument erhoeht!!
    if( !pD->IsClipboard() && aScenList.aEntries.size() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        aScenList.Apply( GetRoot() );
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
        bActive( false ),
        bHasConflict( false ),
        bCriteria( false ),
        bAutoOrAdvanced(false),
        aFilterName(rName)
{
    aParam.nCol1 = rRange.aStart.Col();
    aParam.nRow1 = rRange.aStart.Row();
    aParam.nTab = rRange.aStart.Tab();
    aParam.nCol2 = rRange.aEnd.Col();
    aParam.nRow2 = rRange.aEnd.Row();

    aParam.bInplace = sal_True;

}

void XclImpAutoFilterData::CreateFromDouble( String& rStr, double fVal )
{
    rStr += String( ::rtl::math::doubleToUString( fVal,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), sal_True));
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
    if( pCurrDBData && !bHasConflict )
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
    if( ( rEntry.eOp != SC_EQUAL && rEntry.eOp != SC_NOT_EQUAL ) || rEntry.pStr == NULL )
        return;
    else
    {
        xub_StrLen nLen = rEntry.pStr->Len();
        sal_Unicode nStart = rEntry.pStr->GetChar( 0 );
        sal_Unicode nEnd   = rEntry.pStr->GetChar( nLen-1 );
        if( nLen >2 && nStart == '*' && nEnd == '*' )
        {
            rEntry.pStr->Erase( nLen-1, 1 );
            rEntry.pStr->Erase( 0, 1 );
            rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_CONTAINS : SC_DOES_NOT_CONTAIN;
        }
        else if( nLen > 1 && nStart == '*' && nEnd != '*' )
        {
            rEntry.pStr->Erase( 0, 1 );
            rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_ENDS_WITH : SC_DOES_NOT_END_WITH;
        }
        else if( nLen > 1 && nStart != '*' && nEnd == '*' )
        {
            rEntry.pStr->Erase( nLen-1, 1 );
            rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_BEGINS_WITH : SC_DOES_NOT_BEGIN_WITH;
        }
        else if( nLen == 2 && nStart == '*' && nEnd == '*' )
        {
            rEntry.pStr->Erase( 0, 1 );
        }
    }
}

void XclImpAutoFilterData::ReadAutoFilter( XclImpStream& rStrm )
{
    sal_uInt16 nCol, nFlags;
    rStrm >> nCol >> nFlags;

    ScQueryConnect  eConn       = ::get_flagvalue( nFlags, EXC_AFFLAG_ANDORMASK, SC_OR, SC_AND );
    sal_Bool            bTop10      = ::get_flag( nFlags, EXC_AFFLAG_TOP10 );
    sal_Bool            bTopOfTop10 = ::get_flag( nFlags, EXC_AFFLAG_TOP10TOP );
    sal_Bool            bPercent    = ::get_flag( nFlags, EXC_AFFLAG_TOP10PERC );
    sal_uInt16          nCntOfTop10 = nFlags >> 7;
    SCSIZE          nCount      = aParam.GetEntryCount();

    if( bTop10 )
    {
        if( nFirstEmpty < nCount )
        {
            ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
            aEntry.bDoQuery = sal_True;
            aEntry.bQueryByString = sal_True;
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
        sal_uInt8   nE, nType, nOper, nBoolErr, nVal;
        sal_Int32   nRK;
        double  fVal;
        sal_Bool    bIgnore;

        sal_uInt8   nStrLen[ 2 ]    = { 0, 0 };
        ScQueryEntry *pQueryEntries[ 2 ] = { NULL, NULL };

        for( nE = 0; nE < 2; nE++ )
        {
            if( nFirstEmpty < nCount )
            {
                ScQueryEntry& aEntry = aParam.GetEntry( nFirstEmpty );
                pQueryEntries[ nE ] = &aEntry;
                bIgnore = false;

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
                        bIgnore = (sal_Bool) nBoolErr;
                    break;
                    case EXC_AFTYPE_EMPTY:
                        aEntry.bQueryByString = false;
                        aEntry.nVal = SC_EMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    case EXC_AFTYPE_NOTEMPTY:
                        aEntry.bQueryByString = false;
                        aEntry.nVal = SC_NONEMPTYFIELDS;
                        aEntry.eOp = SC_EQUAL;
                    break;
                    default:
                        rStrm.Ignore( 8 );
                        bIgnore = sal_True;
                }

                /*  #i39464# conflict, if two conditions of one column are 'OR'ed,
                    and they follow conditions of other columns.
                    Example: Let A1 be a condition of column A, and B1 and B2
                    conditions of column B, connected with OR. Excel performs
                    'A1 AND (B1 OR B2)' in this case, but Calc would do
                    '(A1 AND B1) OR B2' instead. */
                if( (nFirstEmpty > 1) && nE && (eConn == SC_OR) && !bIgnore )
                    bHasConflict = sal_True;
                if( !bHasConflict && !bIgnore )
                {
                    aEntry.bDoQuery = sal_True;
                    aEntry.bQueryByString = sal_True;
                    aEntry.nField = static_cast<SCCOLROW>(StartCol() + static_cast<SCCOL>(nCol));
                    aEntry.eConnect = nE ? eConn : SC_AND;
                    nFirstEmpty++;
                }
            }
            else
                rStrm.Ignore( 10 );
        }

        for( nE = 0; nE < 2; nE++ )
            if( nStrLen[ nE ] && pQueryEntries[ nE ] )
            {
                pQueryEntries[ nE ]->pStr->Assign ( rStrm.ReadUniString( nStrLen[ nE ] ) );
                ExcelQueryToOooQuery( *pQueryEntries[ nE ] );
            }

    }
}

void XclImpAutoFilterData::SetAdvancedRange( const ScRange* pRange )
{
    if (pRange)
    {
        aCriteriaRange = *pRange;
        bCriteria = sal_True;
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

void XclImpAutoFilterData::Apply( const sal_Bool bUseUnNamed )
{
    CreateScDBData(bUseUnNamed);

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

void XclImpAutoFilterData::CreateScDBData( const sal_Bool bUseUnNamed )
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
        aEntry.bDoQuery = sal_True;
        ++nFirstEmpty;
    }

    // TBD: force the automatic activation of the
    // "Remove Filter" by setting a virtual mouse click
    // inside the advanced range
}

void XclImpAutoFilterData::AmendAFName(const sal_Bool bUseUnNamed)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
