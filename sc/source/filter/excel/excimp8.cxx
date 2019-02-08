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

#include <config_features.h>

#include <excimp8.hxx>

#include <scitems.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/fltrcfg.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docinf.hxx>
#include <sot/storage.hxx>
#include <svl/sharedstringpool.hxx>

#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <unotools/localedatawrapper.hxx>

#include <document.hxx>
#include <attrib.hxx>
#include <dbdata.hxx>
#include <globalnames.hxx>
#include <docoptio.hxx>
#include <xihelper.hxx>
#include <xicontent.hxx>
#include <xilink.hxx>
#include <xiescher.hxx>
#include <xistyle.hxx>
#include <excdefs.hxx>

#include <excform.hxx>
#include <queryentry.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <cppuhelper/implbase.hxx>
#include "xltoolbar.hxx"
#include <oox/ole/vbaproject.hxx>
#include <oox/ole/olestorage.hxx>

using namespace com::sun::star;
using namespace ::comphelper;

//OleNameOverrideContainer

class OleNameOverrideContainer : public ::cppu::WeakImplHelper< container::XNameContainer >
{
private:
    typedef std::unordered_map< OUString, uno::Reference< container::XIndexContainer > > NamedIndexToOleName;
    NamedIndexToOleName  IdToOleNameHash;
    ::osl::Mutex m_aMutex;
public:
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override { return  cppu::UnoType<container::XIndexContainer>::get(); }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( !IdToOleNameHash.empty() );
    }
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName(aName) )
            throw container::NoSuchElementException();
        return uno::makeAny( IdToOleNameHash[ aName ] );
    }
    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return comphelper::mapKeysToSequence( IdToOleNameHash);
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( IdToOleNameHash.find( aName ) != IdToOleNameHash.end() );
    }

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const uno::Any& aElement ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw container::ElementExistException();
        uno::Reference< container::XIndexContainer > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw lang::IllegalArgumentException();
        IdToOleNameHash[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const OUString& aName ) override
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( IdToOleNameHash.erase( aName ) == 0 )
            throw container::NoSuchElementException();
    }
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override
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
    pFormConv.reset(new ExcelToSc8( GetRoot() ));
    pExcRoot->pFmlaConverter = pFormConv.get();
}

ImportExcel8::~ImportExcel8()
{
}

void ImportExcel8::Calccount()
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterCount( aIn.ReaduInt16() );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8::Precision()
{
    ScDocOptions aOpt = pD->GetDocOptions();
    aOpt.SetCalcAsShown( aIn.ReaduInt16() == 0 );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8::Delta()
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIterEps( aIn.ReadDouble() );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8::Iteration()
{
    ScDocOptions    aOpt = pD->GetDocOptions();
    aOpt.SetIter( aIn.ReaduInt16() == 1 );
    pD->SetDocOptions( aOpt );
}

void ImportExcel8::Boundsheet()
{
    sal_uInt8           nLen;
    sal_uInt16          nGrbit;

    aIn.DisableDecryption();
    maSheetOffsets.push_back( aIn.ReaduInt32() );
    aIn.EnableDecryption();
    nGrbit = aIn.ReaduInt16();
    nLen = aIn.ReaduInt8();

    OUString aName( aIn.ReadUniString( nLen ) );
    GetTabInfo().AppendXclTabName( aName, nBdshtTab );

    SCTAB nScTab = nBdshtTab;
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

void ImportExcel8::Scenman()
{
    sal_uInt16              nLastDispl;

    aIn.Ignore( 4 );
    nLastDispl = aIn.ReaduInt16();

    maScenList.nLastScenario = nLastDispl;
}

void ImportExcel8::Scenario()
{
    maScenList.aEntries.push_back( std::make_unique<ExcScenario>( aIn, *pExcRoot ) );
}

void ImportExcel8::Labelsst()
{
    XclAddress aXclPos;
    sal_uInt16 nXF;
    sal_uInt32  nSst;

    aIn >> aXclPos;
    nXF = aIn.ReaduInt16();
    nSst = aIn.ReaduInt32(  );

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, aXclPos, GetCurrScTab(), true ) )
    {
        GetXFRangeBuffer().SetXF( aScPos, nXF );
        const XclImpString* pXclStr = GetSst().GetString(nSst);
        if (pXclStr)
            XclImpStringHelper::SetToDocument(GetDocImport(), aScPos, *this, *pXclStr, nXF);
    }
}

void ImportExcel8::FeatHdr()
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

void ImportExcel8::Feat()
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
        std::size_t nRead = aIn.Read(aProt.maSecurityDescriptor.data(), nCbSD);
        if (nRead < nCbSD)
            aProt.maSecurityDescriptor.resize( nRead);
    }

    GetSheetProtectBuffer().AppendEnhancedProtection( aProt, GetCurrScTab() );
}

void ImportExcel8::ReadBasic()
{
    SfxObjectShell* pShell = GetDocShell();
    tools::SvRef<SotStorage> xRootStrg = GetRootStorage();
    const SvtFilterOptions& rFilterOpt = SvtFilterOptions::Get();
    if( pShell && xRootStrg.is() ) try
    {
        // #FIXME need to get rid of this, we can also do this from within oox
        // via the "ooo.vba.VBAGlobals" service
        if( ( rFilterOpt.IsLoadExcelBasicCode() ||
              rFilterOpt.IsLoadExcelBasicStorage() ) &&
            rFilterOpt.IsLoadExcelBasicExecutable() )
        {
            // see if we have the XCB stream
            tools::SvRef<SotStorageStream> xXCB = xRootStrg->OpenSotStream( "XCB", StreamMode::STD_READ );
            if ( xXCB.is()|| ERRCODE_NONE == xXCB->GetError() )
            {
                ScCTBWrapper wrapper;
                if ( wrapper.Read( *xXCB ) )
                {
#ifdef DEBUG_SC_EXCEL
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

void ImportExcel8::EndSheet()
{
    ImportExcel::EndSheet();
    GetCondFormatManager().Apply();
    GetValidationManager().Apply();
}

void ImportExcel8::PostDocLoad()
{
#if HAVE_FEATURE_SCRIPTING
    // reading basic has been delayed until sheet objects (codenames etc.) are read
    if( HasBasic() )
        ReadBasic();
#endif
    // #i11776# filtered ranges before outlines and hidden rows
    if( pExcRoot->pAutoFilterBuffer )
        pExcRoot->pAutoFilterBuffer->Apply();

    GetWebQueryBuffer().Apply();    //TODO: test if extant
    GetSheetProtectBuffer().Apply();
    GetDocProtectBuffer().Apply();

    ImportExcel::PostDocLoad();

    // check scenarios; Attention: This increases the table count of the document!!
    if( !pD->IsClipboard() && !maScenList.aEntries.empty() )
    {
        pD->UpdateChartListenerCollection();    // references in charts must be updated

        maScenList.Apply( GetRoot() );
    }

    // read doc info (no docshell while pasting from clipboard)
    if( SfxObjectShell* pShell = GetDocShell() )
    {
        // BIFF5+ without storage is possible
        tools::SvRef<SotStorage> xRootStrg = GetRootStorage();
        if( xRootStrg.is() ) try
        {
            uno::Reference< document::XDocumentPropertiesSupplier > xDPS( pShell->GetModel(), uno::UNO_QUERY_THROW );
            uno::Reference< document::XDocumentProperties > xDocProps( xDPS->getDocumentProperties(), uno::UNO_SET_THROW );
            sfx2::LoadOlePropertySet( xDocProps, xRootStrg.get() );
        }
        catch( uno::Exception& )
        {
        }
    }

    // #i45843# Pivot tables are now handled outside of PostDocLoad, so they are available
    // when formula cells are calculated, for the GETPIVOTDATA function.
}

// autofilter

void ImportExcel8::FilterMode()
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

void ImportExcel8::AutoFilterInfo()
{
    if( !pExcRoot->pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pExcRoot->pAutoFilterBuffer->GetByTab( GetCurrScTab() );
    if( pData )
    {
        pData->SetAdvancedRange( nullptr );
        pData->Activate();
    }
}

void ImportExcel8::AutoFilter()
{
    if( !pExcRoot->pAutoFilterBuffer ) return;

    XclImpAutoFilterData* pData = pExcRoot->pAutoFilterBuffer->GetByTab( GetCurrScTab() );
    if( pData )
        pData->ReadAutoFilter(aIn, GetDoc().GetSharedStringPool());
}

XclImpAutoFilterData::XclImpAutoFilterData( RootData* pRoot, const ScRange& rRange ) :
        ExcRoot( pRoot ),
        pCurrDBData(nullptr),
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
        ScMF nFlag = rDoc.GetAttr( nCol, StartRow(), Tab(), ATTR_MERGE_FLAG )->GetValue();
        rDoc.ApplyAttr( nCol, StartRow(), Tab(), ScMergeFlagAttr( nFlag | ScMF::Auto) );
    }
}

void XclImpAutoFilterData::InsertQueryParam()
{
    if (pCurrDBData)
    {
        ScRange aAdvRange;
        bool    bHasAdv = pCurrDBData->GetAdvancedQuerySource( aAdvRange );
        if( bHasAdv )
            pExcRoot->pIR->GetDoc().CreateQueryParam(aAdvRange, aParam);

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
    nCol = rStrm.ReaduInt16();
    nFlags = rStrm.ReaduInt16();

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
    double      fVal;

    sal_uInt8 nStrLen[2] = { 0, 0 };
    ScQueryEntry aEntries[2];

    for (size_t nE = 0; nE < 2; ++nE)
    {
        ScQueryEntry& rEntry = aEntries[nE];
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        bool bIgnore = false;

        nType = rStrm.ReaduInt8();
        nOper = rStrm.ReaduInt8();
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
                nRK = rStrm.ReadInt32();
                rStrm.Ignore( 4 );
                rItem.maString = rPool.intern(
                    CreateFromDouble(XclTools::GetDoubleFromRK(nRK)));
            break;
            case EXC_AFTYPE_DOUBLE:
                fVal = rStrm.ReadDouble();
                rItem.maString = rPool.intern(CreateFromDouble(fVal));
            break;
            case EXC_AFTYPE_STRING:
                rStrm.Ignore( 4 );
                nStrLen[ nE ] = rStrm.ReaduInt8();
                rStrm.Ignore( 3 );
                rItem.maString = svl::SharedString();
            break;
            case EXC_AFTYPE_BOOLERR:
                nBoolErr = rStrm.ReaduInt8();
                nVal = rStrm.ReaduInt8();
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
        assert( eConn == SC_OR && "eConn should be SC_AND or SC_OR");
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
    // Create the ScDBData() object if the AutoFilter is activated
    // or if we need to create the Advanced Filter.
    if( bActive || bCriteria)
    {
        ScDocument& rDoc = pExcRoot->pIR->GetDocRef();
        pCurrDBData = new ScDBData(STR_DB_LOCAL_NONAME, Tab(),
                                StartCol(),StartRow(), EndCol(),EndRow() );
        if(bCriteria)
        {
            EnableRemoveFilter();

            pCurrDBData->SetQueryParam( aParam );
            pCurrDBData->SetAdvancedQuerySource(&aCriteriaRange);
        }
        else
            pCurrDBData->SetAdvancedQuerySource(nullptr);
        rDoc.SetAnonymousDBData(Tab(), std::unique_ptr<ScDBData>(pCurrDBData));
    }

    if( bActive )
    {
        InsertQueryParam();
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
        maFilters.push_back( std::make_shared<XclImpAutoFilterData>( pRoot, rRange ));
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
    for( const auto& rFilterPtr : maFilters )
        rFilterPtr->Apply();
}

XclImpAutoFilterData* XclImpAutoFilterBuffer::GetByTab( SCTAB nTab )
{
    for( const auto& rFilterPtr : maFilters )
    {
        if( rFilterPtr->Tab() == nTab )
            return rFilterPtr.get();
    }
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
