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

#include <excrecds.hxx>

#include <map>
#include <filter/msfilter/countryid.hxx>

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/objsh.hxx>

#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>

#include <editeng/flditem.hxx>
#include <editeng/flstitem.hxx>

#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svtools/ctrltool.hxx>

#include <string.h>

#include <global.hxx>
#include <globstr.hrc>
#include <docpool.hxx>
#include <patattr.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <scextopt.hxx>
#include <attrib.hxx>
#include <progress.hxx>
#include <dociter.hxx>
#include <rangenam.hxx>
#include <dbdata.hxx>
#include <stlsheet.hxx>
#include <stlpool.hxx>
#include <editutil.hxx>
#include <formula/errorcodes.hxx>
#include <oox/export/utils.hxx>
#include <queryentry.hxx>
#include <queryparam.hxx>

#include <excdoc.hxx>
#include <xeescher.hxx>
#include <xeformula.hxx>
#include <xelink.hxx>
#include <xename.hxx>
#include <xecontent.hxx>

#include <xcl97rec.hxx>
#include <tabprotection.hxx>

using namespace ::oox;

using ::com::sun::star::uno::Sequence;

//--------------------------------------------------------- class ExcDummy_00 -
const sal_uInt8     ExcDummy_00::pMyData[] = {
    0x5c, 0x00, 0x20, 0x00, 0x04, 'C',  'a',  'l',  'c',    // WRITEACCESS
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
const std::size_t ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );

//-------------------------------------------------------- class ExcDummy_04x -
const sal_uInt8     ExcDummy_040::pMyData[] = {
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HIDEOBJ
};
const std::size_t ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );

const sal_uInt8     ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const std::size_t ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );

//-------------------------------------------------------- class ExcDummy_02a -
const sal_uInt8      ExcDummy_02a::pMyData[] = {
    0x0d, 0x00, 0x02, 0x00, 0x01, 0x00,                     // CALCMODE
    0x0c, 0x00, 0x02, 0x00, 0x64, 0x00,                     // CALCCOUNT
    0x0f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // REFMODE
    0x11, 0x00, 0x02, 0x00, 0x00, 0x00,                     // ITERATION
    0x10, 0x00, 0x08, 0x00, 0xfc, 0xa9, 0xf1, 0xd2, 0x4d,   // DELTA
    0x62, 0x50, 0x3f,
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00                      // SAVERECALC
};
const std::size_t ExcDummy_02a::nMyLen = sizeof( ExcDummy_02a::pMyData );

//----------------------------------------------------------- class ExcRecord -

void ExcRecord::Save( XclExpStream& rStrm )
{
    SetRecHeader( GetNum(), GetLen() );
    XclExpRecord::Save( rStrm );
}

void ExcRecord::SaveCont( XclExpStream& /*rStrm*/ )
{
}

void ExcRecord::WriteBody( XclExpStream& rStrm )
{
    SaveCont( rStrm );
}

void ExcRecord::SaveXml( XclExpXmlStream& /*rStrm*/ )
{
}

//--------------------------------------------------------- class ExcEmptyRec -

void ExcEmptyRec::Save( XclExpStream& /*rStrm*/ )
{
}

sal_uInt16 ExcEmptyRec::GetNum() const
{
    return 0;
}

std::size_t ExcEmptyRec::GetLen() const
{
    return 0;
}

//--------------------------------------------------------- class ExcDummyRec -

void ExcDummyRec::Save( XclExpStream& rStrm )
{
    rStrm.Write( GetData(), GetLen() );        // raw write mode
}

sal_uInt16 ExcDummyRec::GetNum() const
{
    return 0x0000;
}

//------------------------------------------------------- class ExcBoolRecord -

void ExcBoolRecord::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16)(bVal ? 0x0001 : 0x0000);
}

std::size_t ExcBoolRecord::GetLen() const
{
    return 2;
}

//--------------------------------------------------------- class ExcBof_Base -

ExcBof_Base::ExcBof_Base()
    : nDocType(0)
    , nVers(0)
    , nRupBuild(0x096C)    // copied from Excel
    , nRupYear(0x07C9)      // copied from Excel
{
}

//-------------------------------------------------------------- class ExcBof -

ExcBof::ExcBof()
{
    nDocType = 0x0010;
    nVers = 0x0500;
}

void ExcBof::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}

sal_uInt16 ExcBof::GetNum() const
{
    return 0x0809;
}

std::size_t ExcBof::GetLen() const
{
    return 8;
}

//------------------------------------------------------------- class ExcBofW -

ExcBofW::ExcBofW()
{
    nDocType = 0x0005;
    nVers = 0x0500;
}

void ExcBofW::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}

sal_uInt16 ExcBofW::GetNum() const
{
    return 0x0809;
}

std::size_t ExcBofW::GetLen() const
{
    return 8;
}

//-------------------------------------------------------------- class ExcEof -

sal_uInt16 ExcEof::GetNum() const
{
    return 0x000A;
}

std::size_t ExcEof::GetLen() const
{
    return 0;
}

//--------------------------------------------------------- class ExcDummy_00 -

std::size_t ExcDummy_00::GetLen() const
{
    return nMyLen;
}

const sal_uInt8* ExcDummy_00::GetData() const
{
    return pMyData;
}

//-------------------------------------------------------- class ExcDummy_04x -

std::size_t ExcDummy_040::GetLen() const
{
    return nMyLen;
}

const sal_uInt8* ExcDummy_040::GetData() const
{
    return pMyData;
}

std::size_t ExcDummy_041::GetLen() const
{
    return nMyLen;
}

const sal_uInt8* ExcDummy_041::GetData() const
{
    return pMyData;
}

//------------------------------------------------------------- class Exc1904 -

Exc1904::Exc1904( const ScDocument& rDoc )
{
    const Date& rDate = rDoc.GetFormatTable()->GetNullDate();
    bVal = (rDate == Date( 1, 1, 1904 ));
    bDateCompatibility = !(rDate == Date( 30, 12, 1899 ));
}

sal_uInt16 Exc1904::GetNum() const
{
    return 0x0022;
}

void Exc1904::SaveXml( XclExpXmlStream& rStrm )
{
    bool bISOIEC = ( rStrm.getVersion() == oox::core::ISOIEC_29500_2008 );

    if( bISOIEC )
    {
        rStrm.WriteAttributes(
            XML_dateCompatibility, ToPsz( bDateCompatibility ),
            FSEND );
    }

    if( !bISOIEC || bDateCompatibility )
    {
        rStrm.WriteAttributes(
            XML_date1904, ToPsz( bVal ),
            FSEND );
    }
}

//------------------------------------------------------ class ExcBundlesheet -

ExcBundlesheetBase::ExcBundlesheetBase( const RootData& rRootData, SCTAB nTabNum ) :
    m_nStrPos( STREAM_SEEK_TO_END ),
    m_nOwnPos( STREAM_SEEK_TO_END ),
    nGrbit( rRootData.pER->GetTabInfo().IsVisibleTab( nTabNum ) ? 0x0000 : 0x0001 ),
    nTab( nTabNum )
{
}

ExcBundlesheetBase::ExcBundlesheetBase() :
    m_nStrPos( STREAM_SEEK_TO_END ),
    m_nOwnPos( STREAM_SEEK_TO_END ),
    nGrbit( 0x0000 ),
    nTab( SCTAB_GLOBAL )
{
}

void ExcBundlesheetBase::UpdateStreamPos( XclExpStream& rStrm )
{
    rStrm.SetSvStreamPos( m_nOwnPos );
    rStrm.DisableEncryption();
    rStrm << static_cast<sal_uInt32>(m_nStrPos);
    rStrm.EnableEncryption();
}

sal_uInt16 ExcBundlesheetBase::GetNum() const
{
    return 0x0085;
}

ExcBundlesheet::ExcBundlesheet( const RootData& rRootData, SCTAB _nTab ) :
    ExcBundlesheetBase( rRootData, _nTab )
{
    OUString sTabName = rRootData.pER->GetTabInfo().GetScTabName( _nTab );
    OSL_ENSURE( sTabName.getLength() < 256, "ExcBundlesheet::ExcBundlesheet - table name too long" );
    aName = OUStringToOString(sTabName, rRootData.pER->GetTextEncoding());
}

void ExcBundlesheet::SaveCont( XclExpStream& rStrm )
{
    m_nOwnPos = rStrm.GetSvStreamPos();
    rStrm   << (sal_uInt32) 0x00000000              // dummy (stream position of the sheet)
            << nGrbit;
    rStrm.WriteByteString(aName);             // 8 bit length, max 255 chars
}

std::size_t ExcBundlesheet::GetLen() const
{
    return 7 + std::min( aName.getLength(), (sal_Int32) 255 );
}

//--------------------------------------------------------- class ExcDummy_02 -

std::size_t ExcDummy_02a::GetLen() const
{
    return nMyLen;
}

const sal_uInt8* ExcDummy_02a::GetData() const
{
    return pMyData;
}
//--------------------------------------------------------- class ExcDummy_02 -

XclExpCountry::XclExpCountry( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_COUNTRY, 4 )
{
    /*  #i31530# set document country as UI country too -
        needed for correct behaviour of number formats. */
    mnUICountry = mnDocCountry = static_cast< sal_uInt16 >(
        ::msfilter::ConvertLanguageToCountry( rRoot.GetDocLanguage() ) );
}

void XclExpCountry::WriteBody( XclExpStream& rStrm )
{
    rStrm << mnUICountry << mnDocCountry;
}

// XclExpWsbool ===============================================================

XclExpWsbool::XclExpWsbool( bool bFitToPages )
    : XclExpUInt16Record( EXC_ID_WSBOOL, EXC_WSBOOL_DEFAULTFLAGS )
{
    if( bFitToPages )
        SetValue( GetValue() | EXC_WSBOOL_FITTOPAGE );
}

XclExpXmlSheetPr::XclExpXmlSheetPr( bool bFitToPages, SCTAB nScTab, const Color& rTabColor, XclExpFilterManager* pManager ) :
    mnScTab(nScTab), mpManager(pManager), mbFitToPage(bFitToPages), maTabColor(rTabColor) {}

void XclExpXmlSheetPr::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_sheetPr,
            // OOXTODO: XML_syncHorizontal,
            // OOXTODO: XML_syncVertical,
            // OOXTODO: XML_syncRef,
            // OOXTODO: XML_transitionEvaluation,
            // OOXTODO: XML_transitionEntry,
            // OOXTODO: XML_published,
            // OOXTODO: XML_codeName,
            XML_filterMode, mpManager ? ToPsz( mpManager->HasFilterMode( mnScTab ) ) : nullptr,
            // OOXTODO: XML_enableFormatConditionsCalculation,
            FSEND );

    // Note : the order of child elements is significant. Don't change the order.

    // OOXTODO: XML_outlinePr

    if (maTabColor != Color(COL_AUTO))
        rWorksheet->singleElement(
            XML_tabColor, XML_rgb, XclXmlUtils::ToOString(maTabColor).getStr(), FSEND);

    rWorksheet->singleElement(XML_pageSetUpPr,
            // OOXTODO: XML_autoPageBreaks,
        XML_fitToPage,  ToPsz(mbFitToPage), FSEND);

    rWorksheet->endElement( XML_sheetPr );
}

// XclExpWindowProtection ===============================================================

XclExpWindowProtection::XclExpWindowProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_WINDOWPROTECT, bValue)
{
}

void XclExpWindowProtection::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.WriteAttributes(
            XML_lockWindows, ToPsz( GetBool() ),
            FSEND );
}

// XclExpDocProtection ===============================================================

XclExpProtection::XclExpProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_PROTECT, bValue)
{
}

XclExpSheetProtection::XclExpSheetProtection(bool bValue, SCTAB nTab ) :
    XclExpProtection( bValue),
    mnTab(nTab)
{
}

void XclExpSheetProtection::SaveXml( XclExpXmlStream& rStrm )
{
   ScDocument& rDoc = rStrm.GetRoot().GetDoc();
   const ScTableProtection* pTabProtect = rDoc.GetTabProtection(mnTab);
   if ( pTabProtect )
   {
        Sequence<sal_Int8> aHash = pTabProtect->getPasswordHash(PASSHASH_XL);
        OString sHash;
        if (aHash.getLength() >= 2)
        {
            sHash = OString::number(
                ( static_cast<sal_uInt8>(aHash[0]) << 8
                  | static_cast<sal_uInt8>(aHash[1]) ),
                16 );
        }
        sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
        rWorksheet->singleElement( XML_sheetProtection,
            XML_sheet,  ToPsz( true ),
            XML_password, sHash.isEmpty()? nullptr : sHash.getStr(),
            XML_objects, pTabProtect->isOptionEnabled( ScTableProtection::OBJECTS ) ? nullptr : ToPsz( true ),
            XML_scenarios, pTabProtect->isOptionEnabled( ScTableProtection::SCENARIOS ) ? nullptr : ToPsz( true ),
            XML_formatCells, pTabProtect->isOptionEnabled( ScTableProtection::FORMAT_CELLS ) ? ToPsz( false ) : nullptr,
            XML_formatColumns, pTabProtect->isOptionEnabled( ScTableProtection::FORMAT_COLUMNS ) ? ToPsz( false ) : nullptr,
            XML_formatRows, pTabProtect->isOptionEnabled( ScTableProtection::FORMAT_ROWS ) ? ToPsz( false ) : nullptr,
            XML_insertColumns, pTabProtect->isOptionEnabled( ScTableProtection::INSERT_COLUMNS ) ? ToPsz( false ) : nullptr,
            XML_insertRows, pTabProtect->isOptionEnabled( ScTableProtection::INSERT_ROWS ) ? ToPsz( false ) : nullptr,
            XML_insertHyperlinks, pTabProtect->isOptionEnabled( ScTableProtection::INSERT_HYPERLINKS ) ? ToPsz( false ) : nullptr,
            XML_deleteColumns, pTabProtect->isOptionEnabled( ScTableProtection::DELETE_COLUMNS ) ? ToPsz( false ) : nullptr,
            XML_deleteRows, pTabProtect->isOptionEnabled( ScTableProtection::DELETE_ROWS ) ? ToPsz( false ) : nullptr,
            XML_selectLockedCells, pTabProtect->isOptionEnabled( ScTableProtection::SELECT_LOCKED_CELLS ) ? nullptr : ToPsz( true ),
            XML_sort, pTabProtect->isOptionEnabled( ScTableProtection::SORT ) ? ToPsz( false ) : nullptr,
            XML_autoFilter, pTabProtect->isOptionEnabled( ScTableProtection::AUTOFILTER ) ? ToPsz( false ) : nullptr,
            XML_pivotTables, pTabProtect->isOptionEnabled( ScTableProtection::PIVOT_TABLES ) ? ToPsz( false ) : nullptr,
            XML_selectUnlockedCells, pTabProtect->isOptionEnabled( ScTableProtection::SELECT_UNLOCKED_CELLS ) ? nullptr : ToPsz( true ),
            FSEND );

        const ::std::vector<ScEnhancedProtection>& rProts( pTabProtect->getEnhancedProtection());
        if (!rProts.empty())
        {
            rWorksheet->startElement( XML_protectedRanges, FSEND);
            for (::std::vector<ScEnhancedProtection>::const_iterator it( rProts.begin()), end( rProts.end());
                    it != end; ++it)
            {
                SAL_WARN_IF( (*it).maSecurityDescriptorXML.isEmpty() && !(*it).maSecurityDescriptor.empty(),
                        "sc.filter", "XclExpSheetProtection::SaveXml: losing BIFF security descriptor");
                rWorksheet->singleElement( XML_protectedRange,
                        XML_name, (*it).maTitle.isEmpty() ? nullptr : XclXmlUtils::ToOString( (*it).maTitle).getStr(),
                        XML_securityDescriptor, (*it).maSecurityDescriptorXML.isEmpty() ? nullptr : XclXmlUtils::ToOString( (*it).maSecurityDescriptorXML).getStr(),
                        /* XXX 'password' is not part of OOXML, but Excel2013
                         * writes it if loaded from BIFF, in which case
                         * 'algorithmName', 'hashValue', 'saltValue' and
                         * 'spinCount' are absent; so do we if it was present. */
                        XML_password, (*it).mnPasswordVerifier ? OString::number( (*it).mnPasswordVerifier, 16).getStr() : nullptr,
                        XML_algorithmName, (*it).maAlgorithmName.isEmpty() ? nullptr : XclXmlUtils::ToOString( (*it).maAlgorithmName).getStr(),
                        XML_hashValue, (*it).maHashValue.isEmpty() ? nullptr : XclXmlUtils::ToOString( (*it).maHashValue).getStr(),
                        XML_saltValue, (*it).maSaltValue.isEmpty() ? nullptr : XclXmlUtils::ToOString( (*it).maSaltValue).getStr(),
                        XML_spinCount, (*it).mnSpinCount ? OString::number( (*it).mnSpinCount).getStr() : nullptr,
                        XML_sqref, (*it).maRangeList.is() ? XclXmlUtils::ToOString( *(*it).maRangeList).getStr() : nullptr,
                        FSEND);
            }
            rWorksheet->endElement( XML_protectedRanges);
        }
    }
}

XclExpPassHash::XclExpPassHash(const Sequence<sal_Int8>& aHash) :
    XclExpRecord(EXC_ID_PASSWORD, 2),
    mnHash(0x0000)
{
    if (aHash.getLength() >= 2)
    {
        mnHash  = ((aHash[0] << 8) & 0xFFFF);
        mnHash |= (aHash[1] & 0xFF);
    }
}

XclExpPassHash::~XclExpPassHash()
{
}

void XclExpPassHash::WriteBody(XclExpStream& rStrm)
{
    rStrm << mnHash;
}

XclExpFiltermode::XclExpFiltermode() :
    XclExpEmptyRecord( EXC_ID_FILTERMODE )
{
}

XclExpAutofilterinfo::XclExpAutofilterinfo( const ScAddress& rStartPos, SCCOL nScCol ) :
    XclExpUInt16Record( EXC_ID_AUTOFILTERINFO, static_cast< sal_uInt16 >( nScCol ) ),
    maStartPos( rStartPos )
{
}

ExcFilterCondition::ExcFilterCondition() :
        nType( EXC_AFTYPE_NOTUSED ),
        nOper( EXC_AFOPER_EQUAL ),
        fVal( 0.0 ),
        pText( nullptr )
{
}

ExcFilterCondition::~ExcFilterCondition()
{
}

std::size_t ExcFilterCondition::GetTextBytes() const
{
    return pText ? (1 + pText->GetBufferSize()) : 0;
}

void ExcFilterCondition::SetCondition( sal_uInt8 nTp, sal_uInt8 nOp, double fV, const OUString* pT )
{
    nType = nTp;
    nOper = nOp;
    fVal = fV;
    pText.reset( pT ? new XclExpString( *pT, XclStrFlags::EightBitLength ) : nullptr);
}

void ExcFilterCondition::Save( XclExpStream& rStrm )
{
    rStrm << nType << nOper;
    switch( nType )
    {
        case EXC_AFTYPE_DOUBLE:
            rStrm << fVal;
        break;
        case EXC_AFTYPE_STRING:
            OSL_ENSURE( pText, "ExcFilterCondition::Save() -- pText is NULL!" );
            rStrm << (sal_uInt32)0 << (sal_uInt8) pText->Len() << (sal_uInt16)0 << (sal_uInt8)0;
        break;
        case EXC_AFTYPE_BOOLERR:
            rStrm << (sal_uInt8)0 << (sal_uInt8)((fVal != 0) ? 1 : 0) << (sal_uInt32)0 << (sal_uInt16)0;
        break;
        default:
            rStrm << (sal_uInt32)0 << (sal_uInt32)0;
    }
}

static const char* lcl_GetOperator( sal_uInt8 nOper )
{
    switch( nOper )
    {
        case EXC_AFOPER_EQUAL:          return "equal";
        case EXC_AFOPER_GREATER:        return "greaterThan";
        case EXC_AFOPER_GREATEREQUAL:   return "greaterThanOrEqual";
        case EXC_AFOPER_LESS:           return "lessThan";
        case EXC_AFOPER_LESSEQUAL:      return "lessThanOrEqual";
        case EXC_AFOPER_NOTEQUAL:       return "notEqual";
        case EXC_AFOPER_NONE:
        default:                        return "**none**";
    }
}

static OString lcl_GetValue( sal_uInt8 nType, double fVal, const XclExpString* pStr )
{
    switch( nType )
    {
        case EXC_AFTYPE_STRING:     return XclXmlUtils::ToOString( *pStr );
        case EXC_AFTYPE_DOUBLE:     return OString::number( fVal );
        case EXC_AFTYPE_BOOLERR:    return OString::number(  ( fVal != 0 ? 1 : 0 ) );
        default:                    return OString();
    }
}

void ExcFilterCondition::SaveXml( XclExpXmlStream& rStrm )
{
    if( IsEmpty() )
        return;

    rStrm.GetCurrentStream()->singleElement( XML_customFilter,
            XML_operator,   lcl_GetOperator( nOper ),
            XML_val,        lcl_GetValue( nType, fVal, pText.get() ).getStr(),
            FSEND );
}

void ExcFilterCondition::SaveText( XclExpStream& rStrm )
{
    if( nType == EXC_AFTYPE_STRING )
    {
        OSL_ENSURE( pText, "ExcFilterCondition::SaveText() -- pText is NULL!" );
        pText->WriteFlagField( rStrm );
        pText->WriteBuffer( rStrm );
    }
}

XclExpAutofilter::XclExpAutofilter( const XclExpRoot& rRoot, sal_uInt16 nC ) :
    XclExpRecord( EXC_ID_AUTOFILTER, 24 ),
    XclExpRoot( rRoot ),
    meType(FilterCondition),
    nCol( nC ),
    nFlags( 0 )
{
}

bool XclExpAutofilter::AddCondition( ScQueryConnect eConn, sal_uInt8 nType, sal_uInt8 nOp,
                                     double fVal, const OUString* pText, bool bSimple )
{
    if( !aCond[ 1 ].IsEmpty() )
        return false;

    sal_uInt16 nInd = aCond[ 0 ].IsEmpty() ? 0 : 1;

    if( nInd == 1 )
        nFlags |= (eConn == SC_OR) ? EXC_AFFLAG_OR : EXC_AFFLAG_AND;
    if( bSimple )
        nFlags |= (nInd == 0) ? EXC_AFFLAG_SIMPLE1 : EXC_AFFLAG_SIMPLE2;

    aCond[ nInd ].SetCondition( nType, nOp, fVal, pText );

    AddRecSize( aCond[ nInd ].GetTextBytes() );

    return true;
}

bool XclExpAutofilter::HasCondition() const
{
    return !aCond[0].IsEmpty();
}

bool XclExpAutofilter::AddEntry( const ScQueryEntry& rEntry )
{
    const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
    if (rItems.empty())
        return true;

    if (GetOutput() != EXC_OUTPUT_BINARY && rItems.size() > 1)
        return AddMultiValueEntry(rEntry);

    bool bConflict = false;
    OUString  sText;
    const ScQueryEntry::Item& rItem = rItems[0];
    if (!rItem.maString.isEmpty())
    {
        sText = rItem.maString.getString();
        switch( rEntry.eOp )
        {
            case SC_CONTAINS:
            case SC_DOES_NOT_CONTAIN:
            {
                sText = "*" + sText + "*";
            }
            break;
            case SC_BEGINS_WITH:
            case SC_DOES_NOT_BEGIN_WITH:
                sText += "*";
            break;
            case SC_ENDS_WITH:
            case SC_DOES_NOT_END_WITH:
                sText = "*" + sText;
            break;
            default:
            {
                //nothing
            }
        }
    }

    bool bLen = sText.getLength() > 0;

    // empty/nonempty fields
    if (rEntry.IsQueryByEmpty())
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_EMPTY, EXC_AFOPER_NONE, 0.0, nullptr, true );
    else if(rEntry.IsQueryByNonEmpty())
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_NOTEMPTY, EXC_AFOPER_NONE, 0.0, nullptr, true );
    // other conditions
    else
    {
        double  fVal    = 0.0;
        sal_uInt32  nIndex  = 0;
        bool bIsNum  = !bLen || GetFormatter().IsNumberFormat( sText, nIndex, fVal );
        OUString* pText;
        bIsNum ? pText = nullptr : pText = &sText;

        // top10 flags
        sal_uInt16 nNewFlags = 0x0000;
        switch( rEntry.eOp )
        {
            case SC_TOPVAL:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP);
            break;
            case SC_BOTVAL:
                nNewFlags = EXC_AFFLAG_TOP10;
            break;
            case SC_TOPPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP | EXC_AFFLAG_TOP10PERC);
            break;
            case SC_BOTPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10PERC);
            break;
            default:;
        }
        bool bNewTop10 = ::get_flag( nNewFlags, EXC_AFFLAG_TOP10 );

        bConflict = HasTop10() && bNewTop10;
        if( !bConflict )
        {
            if( bNewTop10 )
            {
                if( fVal < 0 )      fVal = 0;
                if( fVal >= 501 )   fVal = 500;
                nFlags |= (nNewFlags | (sal_uInt16)fVal << 7);
            }
            // normal condition
            else
            {
                sal_uInt8 nType = bIsNum ? EXC_AFTYPE_DOUBLE : EXC_AFTYPE_STRING;
                sal_uInt8 nOper = EXC_AFOPER_NONE;

                switch( rEntry.eOp )
                {
                    case SC_EQUAL:          nOper = EXC_AFOPER_EQUAL;           break;
                    case SC_LESS:           nOper = EXC_AFOPER_LESS;            break;
                    case SC_GREATER:        nOper = EXC_AFOPER_GREATER;         break;
                    case SC_LESS_EQUAL:     nOper = EXC_AFOPER_LESSEQUAL;       break;
                    case SC_GREATER_EQUAL:  nOper = EXC_AFOPER_GREATEREQUAL;    break;
                    case SC_NOT_EQUAL:      nOper = EXC_AFOPER_NOTEQUAL;        break;
                    case SC_CONTAINS:
                    case SC_BEGINS_WITH:
                    case SC_ENDS_WITH:
                                            nOper = EXC_AFOPER_EQUAL;           break;
                    case SC_DOES_NOT_CONTAIN:
                    case SC_DOES_NOT_BEGIN_WITH:
                    case SC_DOES_NOT_END_WITH:
                                            nOper = EXC_AFOPER_NOTEQUAL;        break;
                    default:;
                }
                bConflict = !AddCondition( rEntry.eConnect, nType, nOper, fVal, pText );
            }
        }
    }
    return bConflict;
}

bool XclExpAutofilter::AddMultiValueEntry( const ScQueryEntry& rEntry )
{
    meType = MultiValue;
    const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
    ScQueryEntry::QueryItemsType::const_iterator itr = rItems.begin(), itrEnd = rItems.end();
    for (; itr != itrEnd; ++itr)
        maMultiValues.push_back(itr->maString.getString());

    return false;
}

void XclExpAutofilter::WriteBody( XclExpStream& rStrm )
{
    rStrm << nCol << nFlags;
    aCond[ 0 ].Save( rStrm );
    aCond[ 1 ].Save( rStrm );
    aCond[ 0 ].SaveText( rStrm );
    aCond[ 1 ].SaveText( rStrm );
}

void XclExpAutofilter::SaveXml( XclExpXmlStream& rStrm )
{
    if (meType == FilterCondition && !HasCondition())
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_filterColumn,
            XML_colId,          OString::number(  nCol ).getStr(),
            // OOXTODO: XML_hiddenButton,   AutoFilter12 fHideArrow?
            // OOXTODO: XML_showButton,
            FSEND );

    switch (meType)
    {
        case FilterCondition:
        {
            if( HasTop10() )
            {
                rWorksheet->singleElement( XML_top10,
                        XML_top,        ToPsz( get_flag( nFlags, EXC_AFFLAG_TOP10TOP ) ),
                        XML_percent,    ToPsz( get_flag( nFlags, EXC_AFFLAG_TOP10PERC ) ),
                        XML_val,        OString::number(  (nFlags >> 7 ) ).getStr(),
                        // OOXTODO: XML_filterVal,
                        FSEND );
            }

            rWorksheet->startElement( XML_customFilters,
                    XML_and,    ToPsz( (nFlags & EXC_AFFLAG_ANDORMASK) == EXC_AFFLAG_AND ),
                    FSEND );
            aCond[ 0 ].SaveXml( rStrm );
            aCond[ 1 ].SaveXml( rStrm );
            rWorksheet->endElement( XML_customFilters );
            // OOXTODO: XLM_colorFilter, XML_dynamicFilter,
            // XML_extLst, XML_filters, XML_iconFilter, XML_top10
        }
        break;
        case MultiValue:
        {
            rWorksheet->startElement(XML_filters, FSEND);
            std::vector<OUString>::const_iterator itr = maMultiValues.begin(), itrEnd = maMultiValues.end();
            for (; itr != itrEnd; ++itr)
            {
                OString aStr = OUStringToOString(*itr, RTL_TEXTENCODING_UTF8);
                const char* pz = aStr.getStr();
                rWorksheet->singleElement(XML_filter, XML_val, pz, FSEND);
            }
            rWorksheet->endElement(XML_filters);
        }
        break;
    }
    rWorksheet->endElement( XML_filterColumn );
}

ExcAutoFilterRecs::ExcAutoFilterRecs( const XclExpRoot& rRoot, SCTAB nTab, const ScDBData* pDefinedData ) :
    XclExpRoot( rRoot ),
    mbAutoFilter (false)
{
    XclExpNameManager& rNameMgr = GetNameManager();

    bool        bFound  = false;
    bool        bAdvanced = false;
    const ScDBData* pData = (pDefinedData ? pDefinedData : rRoot.GetDoc().GetAnonymousDBData(nTab));
    ScRange     aAdvRange;
    if (pData)
    {
        bAdvanced = pData->GetAdvancedQuerySource( aAdvRange );
        bFound = (pData->HasQueryParam() || pData->HasAutoFilter() || bAdvanced);
    }
    if( bFound )
    {
        ScQueryParam    aParam;
        pData->GetQueryParam( aParam );

        ScRange aRange( aParam.nCol1, aParam.nRow1, aParam.nTab,
                        aParam.nCol2, aParam.nRow2, aParam.nTab );
        SCCOL   nColCnt = aParam.nCol2 - aParam.nCol1 + 1;

        maRef = aRange;

        // #i2394# built-in defined names must be sorted by containing sheet name
        if (!pDefinedData)
            rNameMgr.InsertBuiltInName( EXC_BUILTIN_FILTERDATABASE, aRange );

        // advanced filter
        if( bAdvanced )
        {
            // filter criteria, excel allows only same table
            if( !pDefinedData && aAdvRange.aStart.Tab() == nTab )
                rNameMgr.InsertBuiltInName( EXC_BUILTIN_CRITERIA, aAdvRange );

            // filter destination range, excel allows only same table
            if( !aParam.bInplace )
            {
                ScRange aDestRange( aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
                aDestRange.aEnd.IncCol( nColCnt - 1 );
                if( !pDefinedData && aDestRange.aStart.Tab() == nTab )
                    rNameMgr.InsertBuiltInName( EXC_BUILTIN_EXTRACT, aDestRange );
            }

            m_pFilterMode.reset(new XclExpFiltermode);
        }
        // AutoFilter
        else
        {
            bool    bConflict   = false;
            bool    bContLoop   = true;
            bool        bHasOr      = false;
            SCCOLROW nFirstField = aParam.GetEntry( 0 ).nField;

            // create AUTOFILTER records for filtered columns
            for( SCSIZE nEntry = 0; !bConflict && bContLoop && (nEntry < aParam.GetEntryCount()); nEntry++ )
            {
                const ScQueryEntry& rEntry  = aParam.GetEntry( nEntry );

                bContLoop = rEntry.bDoQuery;
                if( bContLoop )
                {
                    XclExpAutofilter* pFilter = GetByCol( static_cast<SCCOL>(rEntry.nField) - aRange.aStart.Col() );

                    if( nEntry > 0 )
                        bHasOr |= (rEntry.eConnect == SC_OR);

                    bConflict = (nEntry > 1) && bHasOr;
                    if( !bConflict )
                        bConflict = (nEntry == 1) && (rEntry.eConnect == SC_OR) &&
                                    (nFirstField != rEntry.nField);
                    if( !bConflict )
                        bConflict = pFilter->AddEntry( rEntry );
                }
            }

            // additional tests for conflicts
            for( size_t nPos = 0, nSize = maFilterList.GetSize(); !bConflict && (nPos < nSize); ++nPos )
            {
                XclExpAutofilterRef xFilter = maFilterList.GetRecord( nPos );
                bConflict = xFilter->HasCondition() && xFilter->HasTop10();
            }

            if( bConflict )
                maFilterList.RemoveAllRecords();

            if( !maFilterList.IsEmpty() )
                m_pFilterMode.reset(new XclExpFiltermode);
            m_pFilterInfo.reset(new XclExpAutofilterinfo( aRange.aStart, nColCnt ));

            if (maFilterList.IsEmpty () && !bConflict)
                mbAutoFilter = true;
        }
    }
}

ExcAutoFilterRecs::~ExcAutoFilterRecs()
{
}

XclExpAutofilter* ExcAutoFilterRecs::GetByCol( SCCOL nCol )
{
    XclExpAutofilterRef xFilter;
    for( size_t nPos = 0, nSize = maFilterList.GetSize(); nPos < nSize; ++nPos )
    {
        xFilter = maFilterList.GetRecord( nPos );
        if( xFilter->GetCol() == static_cast<sal_uInt16>(nCol) )
            return xFilter.get();
    }
    xFilter.reset( new XclExpAutofilter( GetRoot(), static_cast<sal_uInt16>(nCol) ) );
    maFilterList.AppendRecord( xFilter );
    return xFilter.get();
}

bool ExcAutoFilterRecs::IsFiltered( SCCOL nCol )
{
    for( size_t nPos = 0, nSize = maFilterList.GetSize(); nPos < nSize; ++nPos )
        if( maFilterList.GetRecord( nPos )->GetCol() == static_cast<sal_uInt16>(nCol) )
            return true;
    return false;
}

void ExcAutoFilterRecs::AddObjRecs()
{
    if( m_pFilterInfo )
    {
        ScAddress aAddr( m_pFilterInfo->GetStartPos() );
        for( SCCOL nObj = 0, nCount = m_pFilterInfo->GetColCount(); nObj < nCount; nObj++ )
        {
            XclObj* pObjRec = new XclObjDropDown( GetObjectManager(), aAddr, IsFiltered( nObj ) );
            GetObjectManager().AddObj( pObjRec );
            aAddr.IncCol();
        }
    }
}

void ExcAutoFilterRecs::Save( XclExpStream& rStrm )
{
    if( m_pFilterMode )
        m_pFilterMode->Save( rStrm );
    if( m_pFilterInfo )
        m_pFilterInfo->Save( rStrm );
    maFilterList.Save( rStrm );
}

void ExcAutoFilterRecs::SaveXml( XclExpXmlStream& rStrm )
{
    if( maFilterList.IsEmpty() && !mbAutoFilter )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_autoFilter,
            XML_ref,    XclXmlUtils::ToOString( maRef ).getStr(),
            FSEND );
    // OOXTODO: XML_extLst, XML_sortState
    if( !maFilterList.IsEmpty() )
        maFilterList.SaveXml( rStrm );
    rWorksheet->endElement( XML_autoFilter );
}

bool ExcAutoFilterRecs::HasFilterMode() const
{
    return m_pFilterMode != nullptr;
}

XclExpFilterManager::XclExpFilterManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpFilterManager::InitTabFilter( SCTAB nScTab )
{
    maFilterMap[ nScTab ].reset( new ExcAutoFilterRecs( GetRoot(), nScTab, nullptr ) );
}

XclExpRecordRef XclExpFilterManager::CreateRecord( SCTAB nScTab )
{
    XclExpTabFilterRef xRec;
    XclExpTabFilterMap::iterator aIt = maFilterMap.find( nScTab );
    if( aIt != maFilterMap.end() )
    {
        xRec = aIt->second;
        xRec->AddObjRecs();
    }
    return xRec;
}

bool XclExpFilterManager::HasFilterMode( SCTAB nScTab )
{
    XclExpTabFilterRef xRec;
    XclExpTabFilterMap::iterator aIt = maFilterMap.find( nScTab );
    if( aIt != maFilterMap.end() )
    {
        return aIt->second->HasFilterMode();
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
