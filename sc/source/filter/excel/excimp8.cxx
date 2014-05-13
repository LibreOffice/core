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

#include "excimp8.hxx"

#include <boost/bind.hpp>

#include <scitems.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/fltrcfg.hxx>

#include <vcl/wmf.hxx>

#include <editeng/eeitem.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docinf.hxx>
#include <sfx2/frame.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/xflclit.hxx>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>
#include <sot/exchange.hxx>

#include <svl/stritem.hxx>
#include <svl/sharedstringpool.hxx>

#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/charclass.hxx>
#include <drwlayer.hxx>

#include <boost/scoped_array.hpp>

#include "formulacell.hxx"
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

using namespace com::sun::star;
using namespace ::comphelper;

//OleNameOverrideContainer

typedef ::cppu::WeakImplHelper1< container::XNameContainer > OleNameOverrideContainer_BASE;

class OleNameOverrideContainer : public OleNameOverrideContainer_BASE
{
private:
    typedef boost::unordered_map< OUString, uno::Reference< container::XIndexContainer >, OUStringHash,
       ::std::equal_to< OUString > > NamedIndexToOleName;
    NamedIndexToOleName  IdToOleNameHash;
    ::osl::Mutex m_aMutex;
public:
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE { return  cppu::UnoType<container::XIndexContainer>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( IdToOleNameHash.size() > 0 );
    }
    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return uno::makeAny( IdToOleNameHash[ aName ] );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        uno::Sequence< OUString > aResult( IdToOleNameHash.size() );
        NamedIndexToOleName::iterator it = IdToOleNameHash.begin();
        NamedIndexToOleName::iterator it_end = IdToOleNameHash.end();
        OUString* pName = aResult.getArray();
        for (; it != it_end; ++it, ++pName )
            *pName = it->first;
        return aResult;
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException, std::exception) SAL_OVERRIDE
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
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw container::ElementExistException();
        uno::Reference< container::XIndexContainer > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw lang::IllegalArgumentException();
       IdToOleNameHash[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) SAL_OVERRIDE
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        IdToOleNameHash.erase( IdToOleNameHash.find( aName ) );
    }
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) SAL_OVERRIDE
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


namespace {

/** Future Record Type header.
    @return whether read rt matches nRecordID
 */
bool readFrtHeader( XclImpStream& rStrm, sal_uInt16 nRecordID )
{
    sal_uInt16 nRt = rStrm.ReaduInt16();
    rStrm.Ignore(10);   // grbitFrt (2 bytes) and reserved (8 bytes)
    return nRt == nRecordID;
}

}


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

    OUString aName( aIn.ReadUniString( nLen ) );
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
        const XclImpString* pXclStr = GetSst().GetString(nSst);
        if (pXclStr)
            XclImpStringHelper::SetToDocument(GetDocImport(), aScPos, *this, *pXclStr, nXF);
    }
}


void ImportExcel8::FeatHdr( void )
{
    if (!readFrtHeader( aIn, 0x0867))
        return;

    // Feature type (isf) can be EXC_ISFPROTECTION, EXC_ISFFEC2 or
    // EXC_ISFFACTOID.
    sal_uInt16 nFeatureType = aIn.ReaduInt16();
    if (nFeatureType != EXC_ISFPROTECTION)
        // We currently only support import of enhanced protection data.
        return;

    aIn.Ignore(1); // always 1

    GetSheetProtectBuffer().ReadOptions( aIn, GetCurrScTab() );
}


void ImportExcel8::Feat( void )
{
    if (!readFrtHeader( aIn, 0x0868))
        return;

    // Feature type (isf) can be EXC_ISFPROTECTION, EXC_ISFFEC2 or
    // EXC_ISFFACTOID.
    sal_uInt16 nFeatureType = aIn.ReaduInt16();
    if (nFeatureType != EXC_ISFPROTECTION)
        // We currently only support import of enhanced protection data.
        return;

    aIn.Ignore(5);                          // reserved1 (1 byte) and reserved2 (4 bytes)

    sal_uInt16 nCref = aIn.ReaduInt16();    // number of ref elements
    aIn.Ignore(4);                          // size if EXC_ISFFEC2, else 0 and to be ignored
    aIn.Ignore(2);                          // reserved3 (2 bytes)

    ScEnhancedProtection aProt;
    if (nCref)
    {
        XclRangeList aRefs;
        aRefs.Read( aIn, true, nCref);
        if (!aRefs.empty())
        {
            aProt.maRangeList = new ScRangeList;
            GetAddressConverter().ConvertRangeList( *aProt.maRangeList, aRefs, GetCurrScTab(), false);
        }
    }

    // FeatProtection structure follows in record.

    aProt.mnAreserved = aIn.ReaduInt32();
    aProt.mnPasswordVerifier = aIn.ReaduInt32();
    aProt.maTitle = aIn.ReadUniString();
    if ((aProt.mnAreserved & 0x00000001) == 0x00000001)
    {
        sal_uInt32 nCbSD = aIn.ReaduInt32();
        // TODO: could here be some sanity check applied to not allocate 4GB?
        aProt.maSecurityDescriptor.resize( nCbSD);
        sal_Size nRead = aIn.Read( &aProt.maSecurityDescriptor.front(), nCbSD);
        if (nRead < nCbSD)
            aProt.maSecurityDescriptor.resize( nRead);
    }

    GetSheetProtectBuffer().AppendEnhancedProtection( aProt, GetCurrScTab() );
}


void ImportExcel8::ReadBasic( void )
{
    SfxObjectShell* pShell = GetDocShell();
    SotStorageRef xRootStrg = GetRootStorage();
    const SvtFilterOptions& rFilterOpt = SvtFilterOptions::Get();
    if( pShell && xRootStrg.Is() ) try
    {
        // #FIXME need to get rid of this, we can also do this from within oox
        // via the "ooo.vba.VBAGlobals" service
        if( ( rFilterOpt.IsLoadExcelBasicCode() ||
              rFilterOpt.IsLoadExcelBasicStorage() ) &&
            rFilterOpt.IsLoadExcelBasicExecutable() )
        {
            // see if we have the XCB stream
            SvStorageStreamRef xXCB = xRootStrg->OpenSotStream( OUString("XCB"), STREAM_STD_READ | STREAM_NOCREATE  );
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
        pData->ReadAutoFilter(aIn, GetDoc().GetSharedStringPool());
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

    aParam.bInplace = true;

}

namespace {

OUString CreateFromDouble( double fVal )
{
    return rtl::math::doubleToUString(fVal,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                ScGlobal::pLocaleData->getNumDecimalSep()[0], true);
}

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
        bool    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pIR->GetDoc().CreateQueryParam( aAdvRange.aStart.Col(),
                aAdvRange.aStart.Row(), aAdvRange.aEnd.Col(), aAdvRange.aEnd.Row(),
                aAdvRange.aStart.Tab(), aParam );

        pCurrDBData->SetQueryParam( aParam );
        if( bHasAdv )
            pCurrDBData->SetAdvancedQuerySource( &aAdvRange );
        else
        {
            pCurrDBData->SetAutoFilter( true );
            SetCellAttribs();
        }
    }
}

static void ExcelQueryToOooQuery( OUString& aStr, ScQueryEntry& rEntry )
{
    if (rEntry.eOp != SC_EQUAL && rEntry.eOp != SC_NOT_EQUAL)
        return;

    sal_Int32   nLen = aStr.getLength();
    sal_Unicode nStart = aStr[0];
    sal_Unicode nEnd   = aStr[ nLen-1 ];
    if( nLen > 2 && nStart == '*' && nEnd == '*' )
    {
        aStr = aStr.copy( 1, nLen-2 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_CONTAINS : SC_DOES_NOT_CONTAIN;
    }
    else if( nLen > 1 && nStart == '*' && nEnd != '*' )
    {
        aStr = aStr.copy( 1 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_ENDS_WITH : SC_DOES_NOT_END_WITH;
    }
    else if( nLen > 1 && nStart != '*' && nEnd == '*' )
    {
        aStr = aStr.copy( 0, nLen-1 );
        rEntry.eOp = ( rEntry.eOp == SC_EQUAL ) ? SC_BEGINS_WITH : SC_DOES_NOT_BEGIN_WITH;
    }
    else if( nLen == 2 && nStart == '*' && nEnd == '*' )
    {
        aStr = aStr.copy( 1 );
    }
}

void XclImpAutoFilterData::ReadAutoFilter(
    XclImpStream& rStrm, svl::SharedStringPool& rPool )
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
        rItem.maString = rPool.intern(OUString::number(nCntOfTop10));

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

        switch( nType )
        {
            case EXC_AFTYPE_RK:
                rStrm >> nRK;
                rStrm.Ignore( 4 );
                rItem.maString = rPool.intern(
                    CreateFromDouble(XclTools::GetDoubleFromRK(nRK)));
            break;
            case EXC_AFTYPE_DOUBLE:
                rStrm >> fVal;
                rItem.maString = rPool.intern(CreateFromDouble(fVal));
            break;
            case EXC_AFTYPE_STRING:
                rStrm.Ignore( 4 );
                rStrm >> nStrLen[ nE ];
                rStrm.Ignore( 3 );
                rItem.maString = svl::SharedString();
            break;
            case EXC_AFTYPE_BOOLERR:
                rStrm >> nBoolErr >> nVal;
                rStrm.Ignore( 6 );
                rItem.maString = rPool.intern(OUString::number(nVal));
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
                OUString aStr = rStrm.ReadUniString(nStrLen[nE]);
                ExcelQueryToOooQuery(aStr, aEntries[nE]);
                aEntries[nE].GetQueryItem().maString = rPool.intern(aStr);
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
            aItem1.maString = rPool.intern(rStrm.ReadUniString(nStrLen[0]));
            aItem1.meType = ScQueryEntry::ByString;
            aItem2.maString = rPool.intern(rStrm.ReadUniString(nStrLen[1]));
            aItem2.meType = ScQueryEntry::ByString;
            aItems.push_back(aItem1);
            aItems.push_back(aItem2);
            rEntry.GetQueryItems().swap(aItems);
        }
        else if (nStrLen[0] && aEntries[0].bDoQuery)
        {
            // Due to conflict, we can import only the first condition.
            OUString aStr = rStrm.ReadUniString(nStrLen[0]);
            ExcelQueryToOooQuery(aStr, aEntries[0]);
            aEntries[0].GetQueryItem().maString = rPool.intern(aStr);
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
    aParam.bDestPers = true;
}

void XclImpAutoFilterData::Apply()
{
    CreateScDBData();

    if( bActive )
    {
        InsertQueryParam();
    }
}

void XclImpAutoFilterData::CreateScDBData()
{

    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDocument* pDoc = pExcRoot->pIR->GetDocPtr();
        OUString aNewName(STR_DB_LOCAL_NONAME);
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
