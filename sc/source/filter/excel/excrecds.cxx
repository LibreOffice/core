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



//------------------------------------------------------------------------

#include "excrecds.hxx"

#include <map>
#include <filter/msfilter/countryid.hxx>

#include "scitems.hxx"
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
#include <editeng/brshitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escpitem.hxx>
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svtools/ctrltool.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

#include <string.h>

#include "global.hxx"
#include "globstr.hrc"
#include "docpool.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"
#include "formula/errorcodes.hxx"

#include "excdoc.hxx"
#include "xeescher.hxx"
#include "xeformula.hxx"
#include "xelink.hxx"
#include "xename.hxx"
#include "xecontent.hxx"

#include "xcl97rec.hxx"

using namespace ::oox;

using ::com::sun::star::uno::Sequence;
using ::rtl::OString;

//--------------------------------------------------------- class ExcDummy_00 -
const sal_uInt8     ExcDummy_00::pMyData[] = {
    0x5c, 0x00, 0x20, 0x00, 0x04, 'C',  'a',  'l',  'c',    // WRITEACCESS
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
const sal_Size ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );

//-------------------------------------------------------- class ExcDummy_04x -
const sal_uInt8     ExcDummy_040::pMyData[] = {
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HIDEOBJ
};
const sal_Size ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );

const sal_uInt8     ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const sal_Size ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );

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
const sal_Size ExcDummy_02a::nMyLen = sizeof( ExcDummy_02a::pMyData );

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


sal_Size ExcEmptyRec::GetLen() const
{
    return 0;
}



//------------------------------------------------------- class ExcRecordList -

ExcRecordList::~ExcRecordList()
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        delete pRec;
}


void ExcRecordList::Save( XclExpStream& rStrm )
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        pRec->Save( rStrm );
}



//--------------------------------------------------------- class ExcDummyRec -

void ExcDummyRec::Save( XclExpStream& rStrm )
{
    rStrm.Write( GetData(), GetLen() );        // raw write mode
}


sal_uInt16 ExcDummyRec::GetNum( void ) const
{
    return 0x0000;
}



//------------------------------------------------------- class ExcBoolRecord -

void ExcBoolRecord::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16)(bVal ? 0x0001 : 0x0000);
}


sal_Size ExcBoolRecord::GetLen( void ) const
{
    return 2;
}




//--------------------------------------------------------- class ExcBof_Base -

ExcBof_Base::ExcBof_Base() :
    nRupBuild( 0x096C ),    // copied from Excel
    nRupYear( 0x07C9 )      // copied from Excel
{
}



//-------------------------------------------------------------- class ExcBof -

ExcBof::ExcBof( void )
{
    nDocType = 0x0010;
    nVers = 0x0500;
}


void ExcBof::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}


sal_uInt16 ExcBof::GetNum( void ) const
{
    return 0x0809;
}


sal_Size ExcBof::GetLen( void ) const
{
    return 8;
}



//------------------------------------------------------------- class ExcBofW -

ExcBofW::ExcBofW( void )
{
    nDocType = 0x0005;
    nVers = 0x0500;
}


void ExcBofW::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}



sal_uInt16 ExcBofW::GetNum( void ) const
{
    return 0x0809;
}



sal_Size ExcBofW::GetLen( void ) const
{
    return 8;
}



//-------------------------------------------------------------- class ExcEof -

sal_uInt16 ExcEof::GetNum( void ) const
{
    return 0x000A;
}


sal_Size ExcEof::GetLen( void ) const
{
    return 0;
}



//--------------------------------------------------------- class ExcDummy_00 -

sal_Size ExcDummy_00::GetLen( void ) const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy_00::GetData( void ) const
{
    return pMyData;
}



//-------------------------------------------------------- class ExcDummy_04x -

sal_Size ExcDummy_040::GetLen( void ) const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy_040::GetData( void ) const
{
    return pMyData;
}




sal_Size ExcDummy_041::GetLen( void ) const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy_041::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------------- class Exc1904 -

Exc1904::Exc1904( ScDocument& rDoc )
{
    Date* pDate = rDoc.GetFormatTable()->GetNullDate();
    bVal = pDate ? (*pDate == Date( 1, 1, 1904 )) : false;
    bDateCompatibility = pDate ? !( *pDate == Date( 30, 12, 1899 )) : false;
}


sal_uInt16 Exc1904::GetNum( void ) const
{
    return 0x0022;
}


void Exc1904::SaveXml( XclExpXmlStream& rStrm )
{
    bool bISOIEC = ( rStrm.getVersion() == oox::core::ISOIEC_29500_2008 );

    if( bISOIEC )
    {
        rStrm.WriteAttributes(
            XML_dateCompatibility, XclXmlUtils::ToPsz( bDateCompatibility ),
            FSEND );
    }

    if( !bISOIEC || bDateCompatibility )
    {
        rStrm.WriteAttributes(
            XML_date1904, XclXmlUtils::ToPsz( bVal ),
            FSEND );
    }
}



//------------------------------------------------------ class ExcBundlesheet -

ExcBundlesheetBase::ExcBundlesheetBase( RootData& rRootData, SCTAB nTabNum ) :
    nStrPos( STREAM_SEEK_TO_END ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nGrbit( rRootData.pER->GetTabInfo().IsVisibleTab( nTabNum ) ? 0x0000 : 0x0001 ),
    nTab( nTabNum )
{
}


ExcBundlesheetBase::ExcBundlesheetBase() :
    nStrPos( STREAM_SEEK_TO_END ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nGrbit( 0x0000 ),
    nTab( SCTAB_GLOBAL )
{
}


void ExcBundlesheetBase::UpdateStreamPos( XclExpStream& rStrm )
{
    rStrm.SetSvStreamPos( nOwnPos );
    rStrm.DisableEncryption();
    rStrm << static_cast<sal_uInt32>(nStrPos);
    rStrm.EnableEncryption();
}


sal_uInt16 ExcBundlesheetBase::GetNum( void ) const
{
    return 0x0085;
}




ExcBundlesheet::ExcBundlesheet( RootData& rRootData, SCTAB _nTab ) :
    ExcBundlesheetBase( rRootData, _nTab )
{
    String sTabName = rRootData.pER->GetTabInfo().GetScTabName( _nTab );
    DBG_ASSERT( sTabName.Len() < 256, "ExcBundlesheet::ExcBundlesheet - table name too long" );
    aName = ByteString( sTabName, rRootData.pER->GetTextEncoding() );
}


void ExcBundlesheet::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetSvStreamPos();
    rStrm   << (sal_uInt32) 0x00000000              // dummy (stream position of the sheet)
            << nGrbit;
    rStrm.WriteByteString( aName );             // 8 bit length, max 255 chars
}


sal_Size ExcBundlesheet::GetLen() const
{
    return 7 + Min( aName.Len(), (xub_StrLen) 255 );
}


//--------------------------------------------------------- class ExcDummy_02 -

sal_Size ExcDummy_02a::GetLen( void ) const
{
    return nMyLen;
}

const sal_uInt8* ExcDummy_02a::GetData( void ) const
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

XclExpWsbool::XclExpWsbool( bool bFitToPages, SCTAB nScTab, XclExpFilterManager* pManager )
    : XclExpUInt16Record( EXC_ID_WSBOOL, EXC_WSBOOL_DEFAULTFLAGS )
    , mnScTab( nScTab )
    , mpManager( pManager )
{
    if( bFitToPages )
        SetValue( GetValue() | EXC_WSBOOL_FITTOPAGE );
}

void XclExpWsbool::SaveXml( XclExpXmlStream& rStrm )
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
            XML_filterMode, mpManager ? XclXmlUtils::ToPsz( mpManager->HasFilterMode( mnScTab ) ) : NULL,
            // OOXTODO: XML_enableFormatConditionsCalculation,
            FSEND );
    // OOXTODO: elements XML_tabColor, XML_outlinePr
    rWorksheet->singleElement( XML_pageSetUpPr,
            // OOXTODO: XML_autoPageBreaks,
            XML_fitToPage,  XclXmlUtils::ToPsz( GetValue() & EXC_WSBOOL_FITTOPAGE ),
            FSEND );
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
            XML_lockWindows, XclXmlUtils::ToPsz( GetBool() ),
            FSEND );
}

// XclExpDocProtection ===============================================================

XclExpProtection::XclExpProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_PROTECT, bValue)
{
}

// ============================================================================

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

// ============================================================================

XclExpFiltermode::XclExpFiltermode() :
    XclExpEmptyRecord( EXC_ID_FILTERMODE )
{
}

// ----------------------------------------------------------------------------

XclExpAutofilterinfo::XclExpAutofilterinfo( const ScAddress& rStartPos, SCCOL nScCol ) :
    XclExpUInt16Record( EXC_ID_AUTOFILTERINFO, static_cast< sal_uInt16 >( nScCol ) ),
    maStartPos( rStartPos )
{
}

// ----------------------------------------------------------------------------

ExcFilterCondition::ExcFilterCondition() :
        nType( EXC_AFTYPE_NOTUSED ),
        nOper( EXC_AFOPER_EQUAL ),
        fVal( 0.0 ),
        pText( NULL )
{
}

ExcFilterCondition::~ExcFilterCondition()
{
    if( pText )
        delete pText;
}

sal_Size ExcFilterCondition::GetTextBytes() const
{
    return pText ? (1 + pText->GetBufferSize()) : 0;
}

void ExcFilterCondition::SetCondition( sal_uInt8 nTp, sal_uInt8 nOp, double fV, String* pT )
{
    nType = nTp;
    nOper = nOp;
    fVal = fV;

    delete pText;
    pText = pT ? new XclExpString( *pT, EXC_STR_8BITLENGTH ) : NULL;
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
            DBG_ASSERT( pText, "ExcFilterCondition::Save() -- pText is NULL!" );
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

static OString lcl_GetValue( sal_uInt8 nType, double fVal, XclExpString* pStr )
{
    switch( nType )
    {
        case EXC_AFTYPE_STRING:     return XclXmlUtils::ToOString( *pStr );
        case EXC_AFTYPE_DOUBLE:     return OString::valueOf( fVal );
        case EXC_AFTYPE_BOOLERR:    return OString::valueOf( (sal_Int32) ( fVal != 0 ? 1 : 0 ) );
        default:                    return OString();
    }
}

void ExcFilterCondition::SaveXml( XclExpXmlStream& rStrm )
{
    if( IsEmpty() )
        return;

    rStrm.GetCurrentStream()->singleElement( XML_customFilter,
            XML_operator,   lcl_GetOperator( nOper ),
            XML_val,        lcl_GetValue( nType, fVal, pText ).getStr(),
            FSEND );
}

void ExcFilterCondition::SaveText( XclExpStream& rStrm )
{
    if( nType == EXC_AFTYPE_STRING )
    {
        DBG_ASSERT( pText, "ExcFilterCondition::SaveText() -- pText is NULL!" );
        pText->WriteFlagField( rStrm );
        pText->WriteBuffer( rStrm );
    }
}

// ----------------------------------------------------------------------------

XclExpAutofilter::XclExpAutofilter( const XclExpRoot& rRoot, sal_uInt16 nC ) :
    XclExpRecord( EXC_ID_AUTOFILTER, 24 ),
    XclExpRoot( rRoot ),
    nCol( nC ),
    nFlags( 0 )
{
}

sal_Bool XclExpAutofilter::AddCondition( ScQueryConnect eConn, sal_uInt8 nType, sal_uInt8 nOp,
                                    double fVal, String* pText, sal_Bool bSimple )
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

    return sal_True;
}

sal_Bool XclExpAutofilter::AddEntry( const ScQueryEntry& rEntry )
{
    sal_Bool    bConflict = false;
    String  sText;

    if( rEntry.pStr )
    {
        sText.Assign( *rEntry.pStr );
        switch( rEntry.eOp )
        {
            case SC_CONTAINS:
            case SC_DOES_NOT_CONTAIN:
            {
                sText.InsertAscii( "*" , 0 );
                sText.AppendAscii( "*" );
            }
            break;
            case SC_BEGINS_WITH:
            case SC_DOES_NOT_BEGIN_WITH:
                sText.AppendAscii( "*" );
            break;
            case SC_ENDS_WITH:
            case SC_DOES_NOT_END_WITH:
                sText.InsertAscii( "*" , 0 );
            break;
            default:
            {
                //nothing
            }
        }
    }

    sal_Bool bLen = sText.Len() > 0;

    // empty/nonempty fields
    if( !bLen && (rEntry.nVal == SC_EMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_EMPTY, EXC_AFOPER_NONE, 0.0, NULL, sal_True );
    else if( !bLen && (rEntry.nVal == SC_NONEMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_NOTEMPTY, EXC_AFOPER_NONE, 0.0, NULL, sal_True );
    // other conditions
    else
    {
        double  fVal    = 0.0;
        sal_uInt32  nIndex  = 0;
        sal_Bool    bIsNum  = bLen ? GetFormatter().IsNumberFormat( sText, nIndex, fVal ) : sal_True;
        String* pText   = bIsNum ? NULL : &sText;

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
        sal_Bool bNewTop10 = ::get_flag( nNewFlags, EXC_AFFLAG_TOP10 );

        bConflict = HasTop10() && bNewTop10;
        if( !bConflict )
        {
            if( bNewTop10 )
            {
                if( fVal < 0 )      fVal = 0;
                if( fVal >= 501 )   fVal = 500;
                nFlags |= (nNewFlags | (sal_uInt16)(fVal) << 7);
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
    if( !HasCondition() )
        return;

    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();

    rWorksheet->startElement( XML_filterColumn,
            XML_colId,          OString::valueOf( (sal_Int32) nCol ).getStr(),
            // OOXTODO: XML_hiddenButton,   AutoFilter12 fHideArrow?
            // OOXTODO: XML_showButton,
            FSEND );

    if( HasTop10() )
    {
        rWorksheet->singleElement( XML_top10,
                XML_top,        XclXmlUtils::ToPsz( get_flag( nFlags, EXC_AFFLAG_TOP10TOP ) ),
                XML_percent,    XclXmlUtils::ToPsz( get_flag( nFlags, EXC_AFFLAG_TOP10PERC ) ),
                XML_val,        OString::valueOf( (sal_Int32) (nFlags >> 7 ) ).getStr(),
                // OOXTODO: XML_filterVal,
                FSEND );
    }

    rWorksheet->startElement( XML_customFilters,
            XML_and,    XclXmlUtils::ToPsz( (nFlags & EXC_AFFLAG_ANDORMASK) == EXC_AFFLAG_AND ),
            FSEND );
    aCond[ 0 ].SaveXml( rStrm );
    aCond[ 1 ].SaveXml( rStrm );
    rWorksheet->endElement( XML_customFilters );
    // OOXTODO: XLM_colorFilter, XML_dynamicFilter,
    // XML_extLst, XML_filters, XML_iconFilter, XML_top10
    rWorksheet->endElement( XML_filterColumn );
}

// ----------------------------------------------------------------------------

ExcAutoFilterRecs::ExcAutoFilterRecs( const XclExpRoot& rRoot, SCTAB nTab ) :
    XclExpRoot( rRoot ),
    pFilterMode( NULL ),
    pFilterInfo( NULL )
    , mbAutoFilter (false)
{
    ScDBCollection& rDBColl = GetDatabaseRanges();
    XclExpNameManager& rNameMgr = GetNameManager();

    // search for first DB-range with filter
    sal_uInt16      nIndex  = 0;
    sal_Bool        bFound  = false;
    sal_Bool        bAdvanced = false;
    ScDBData*   pData   = NULL;
    ScRange     aAdvRange;
    while( (nIndex < rDBColl.GetCount()) && !bFound )
    {
        pData = rDBColl[ nIndex ];
        if( pData )
        {
            ScRange aRange;
            pData->GetArea( aRange );
            bAdvanced = pData->GetAdvancedQuerySource( aAdvRange );
            bFound = (aRange.aStart.Tab() == nTab) &&
                (pData->HasQueryParam() || pData->HasAutoFilter() || bAdvanced);
        }
        if( !bFound )
            nIndex++;
    }

    if( pData && bFound )
    {
        ScQueryParam    aParam;
        pData->GetQueryParam( aParam );

        ScRange aRange( aParam.nCol1, aParam.nRow1, aParam.nTab,
                        aParam.nCol2, aParam.nRow2, aParam.nTab );
        SCCOL   nColCnt = aParam.nCol2 - aParam.nCol1 + 1;

        maRef = aRange;

        // #i2394# built-in defined names must be sorted by containing sheet name
        rNameMgr.InsertBuiltInName( EXC_BUILTIN_FILTERDATABASE, aRange );

        // advanced filter
        if( bAdvanced )
        {
            // filter criteria, excel allows only same table
            if( aAdvRange.aStart.Tab() == nTab )
                rNameMgr.InsertBuiltInName( EXC_BUILTIN_CRITERIA, aAdvRange );

            // filter destination range, excel allows only same table
            if( !aParam.bInplace )
            {
                ScRange aDestRange( aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
                aDestRange.aEnd.IncCol( nColCnt - 1 );
                if( aDestRange.aStart.Tab() == nTab )
                    rNameMgr.InsertBuiltInName( EXC_BUILTIN_EXTRACT, aDestRange );
            }

            pFilterMode = new XclExpFiltermode;
        }
        // AutoFilter
        else
        {
            sal_Bool    bConflict   = false;
            sal_Bool    bContLoop   = sal_True;
            sal_Bool    bHasOr      = false;
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
                pFilterMode = new XclExpFiltermode;
            pFilterInfo = new XclExpAutofilterinfo( aRange.aStart, nColCnt );

            if (maFilterList.IsEmpty () && !bConflict)
                mbAutoFilter = true;
        }
    }
}

ExcAutoFilterRecs::~ExcAutoFilterRecs()
{
    delete pFilterMode;
    delete pFilterInfo;
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

sal_Bool ExcAutoFilterRecs::IsFiltered( SCCOL nCol )
{
    for( size_t nPos = 0, nSize = maFilterList.GetSize(); nPos < nSize; ++nPos )
        if( maFilterList.GetRecord( nPos )->GetCol() == static_cast<sal_uInt16>(nCol) )
            return sal_True;
    return false;
}

void ExcAutoFilterRecs::AddObjRecs()
{
    if( pFilterInfo )
    {
        ScAddress aAddr( pFilterInfo->GetStartPos() );
        for( SCCOL nObj = 0, nCount = pFilterInfo->GetColCount(); nObj < nCount; nObj++ )
        {
            XclObj* pObjRec = new XclObjDropDown( GetObjectManager(), aAddr, IsFiltered( nObj ) );
            GetObjectManager().AddObj( pObjRec );
            aAddr.IncCol( 1 );
        }
    }
}

void ExcAutoFilterRecs::Save( XclExpStream& rStrm )
{
    if( pFilterMode )
        pFilterMode->Save( rStrm );
    if( pFilterInfo )
        pFilterInfo->Save( rStrm );
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
    return pFilterMode != NULL;
}

// ----------------------------------------------------------------------------

XclExpFilterManager::XclExpFilterManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpFilterManager::InitTabFilter( SCTAB nScTab )
{
    maFilterMap[ nScTab ].reset( new ExcAutoFilterRecs( GetRoot(), nScTab ) );
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

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
