/*************************************************************************
 *
 *  $RCSfile: excrecds.cxx,v $
 *
 *  $Revision: 1.76 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-09 15:01:04 $
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

#include "excrecds.hxx"

#include <map>

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#include <svx/countryid.hxx>
#endif

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sfx2/objsh.hxx>

#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>

#include <svx/flditem.hxx>
#include <svx/flstitem.hxx>

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/fhgtitem.hxx>
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <svtools/ctrltool.hxx>

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

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
#include "errorcodes.hxx"

#include "excdoc.hxx"

#ifndef SC_XEFORMULA_HXX
#include "xeformula.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif

#include "xcl97rec.hxx"



//--------------------------------------------------------- class ExcDummy_00 -
const BYTE      ExcDummy_00::pMyData[] = {
    0xe1, 0x00, 0x00, 0x00,                                 // INTERFACEHDR
    0xc1, 0x00, 0x02, 0x00, 0x00, 0x00,                     // MMS
    0xbf, 0x00, 0x00, 0x00,                                 // TOOLBARHDR
    0xc0, 0x00, 0x00, 0x00,                                 // TOOLBAREND
    0xe2, 0x00, 0x00, 0x00,                                 // INTERFACEEND
    0x5c, 0x00, 0x20, 0x00, 0x04, 0x4d, 0x72, 0x20, 0x58,   // WRITEACCESS
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x42, 0x00, 0x02, 0x00, 0xe4, 0x04,                     // CODEPAGE
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      // FNGROUPCOUNT
};
const ULONG ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );

//-------------------------------------------------------- class ExcDummy_04x -
const BYTE      ExcDummy_040::pMyData[] = {
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HIDEOBJ
};
const ULONG ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );

const BYTE      ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const ULONG ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );

//-------------------------------------------------------- class ExcDummy_02a -
const BYTE      ExcDummy_02a::pMyData[] = {
    0x0d, 0x00, 0x02, 0x00, 0x01, 0x00,                     // CALCMODE
    0x0c, 0x00, 0x02, 0x00, 0x64, 0x00,                     // CALCCOUNT
    0x0f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // REFMODE
    0x11, 0x00, 0x02, 0x00, 0x00, 0x00,                     // ITERATION
    0x10, 0x00, 0x08, 0x00, 0xfc, 0xa9, 0xf1, 0xd2, 0x4d,   // DELTA
    0x62, 0x50, 0x3f,
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00                      // SAVERECALC
};
const ULONG ExcDummy_02a::nMyLen = sizeof( ExcDummy_02a::pMyData );

//----------------------------------------------------------- class ExcRecord -

void ExcRecord::Save( XclExpStream& rStrm )
{
    SetRecId( GetNum() );
    SetRecSize( GetLen() );
    XclExpRecord::Save( rStrm );
}

void ExcRecord::SaveCont( XclExpStream& rStrm )
{
}

void ExcRecord::WriteBody( XclExpStream& rStrm )
{
    SaveCont( rStrm );
}


//--------------------------------------------------------- class ExcEmptyRec -

void ExcEmptyRec::Save( XclExpStream& rStrm )
{
}


UINT16 ExcEmptyRec::GetNum() const
{
    return 0;
}


ULONG ExcEmptyRec::GetLen() const
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
    rStrm.Write( GetData(), GetLen() );     // raw write mode
}


UINT16 ExcDummyRec::GetNum( void ) const
{
    return 0x0000;
}



//------------------------------------------------------- class ExcBoolRecord -

ExcBoolRecord::ExcBoolRecord( SfxItemSet* pItemSet, USHORT nWhich, BOOL bDefault )
{
    bVal = pItemSet? ( ( const SfxBoolItem& ) pItemSet->Get( nWhich ) ).GetValue() : bDefault;
}


void ExcBoolRecord::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16)(bVal ? 0x0001 : 0x0000);
}


ULONG ExcBoolRecord::GetLen( void ) const
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


UINT16 ExcBof::GetNum( void ) const
{
    return 0x0809;
}


ULONG ExcBof::GetLen( void ) const
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



UINT16 ExcBofW::GetNum( void ) const
{
    return 0x0809;
}



ULONG ExcBofW::GetLen( void ) const
{
    return 8;
}



//-------------------------------------------------------------- class ExcEof -

UINT16 ExcEof::GetNum( void ) const
{
    return 0x000A;
}


ULONG ExcEof::GetLen( void ) const
{
    return 0;
}



//----------------------------------------------------- class ExcFngroupcount -

void ExcFngroupcount::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT16 ) 0x000E;     // copied from Excel
}


UINT16 ExcFngroupcount::GetNum( void ) const
{
    return 0x009C;
}


ULONG ExcFngroupcount::GetLen( void ) const
{
    return 2;
}



//--------------------------------------------------------- class ExcDummy_00 -

ULONG ExcDummy_00::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_00::GetData( void ) const
{
    return pMyData;
}



//-------------------------------------------------------- class ExcDummy_04x -

ULONG ExcDummy_040::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_040::GetData( void ) const
{
    return pMyData;
}




ULONG ExcDummy_041::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_041::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------------- class Exc1904 -

Exc1904::Exc1904( ScDocument& rDoc )
{
    Date* pDate = rDoc.GetFormatTable()->GetNullDate();
    bVal = pDate ? (*pDate == Date( 1, 1, 1904 )) : FALSE;
}


UINT16 Exc1904::GetNum( void ) const
{
    return 0x0022;
}



//------------------------------------------------------ class ExcBundlesheet -

ExcBundlesheetBase::ExcBundlesheetBase( RootData& rRootData, SCTAB nTab ) :
    nGrbit( rRootData.pER->GetTabInfo().IsVisibleTab( nTab ) ? 0x0000 : 0x0001 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


ExcBundlesheetBase::ExcBundlesheetBase() :
    nGrbit( 0x0000 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


void ExcBundlesheetBase::UpdateStreamPos( XclExpStream& rStrm )
{
    rStrm.SetStreamPos( nOwnPos );
    rStrm << nStrPos;
}


UINT16 ExcBundlesheetBase::GetNum( void ) const
{
    return 0x0085;
}




ExcBundlesheet::ExcBundlesheet( RootData& rRootData, SCTAB nTab ) :
    ExcBundlesheetBase( rRootData, nTab )
{
    String sTabName;
    rRootData.pDoc->GetName( nTab, sTabName );
    DBG_ASSERT( sTabName.Len() < 256, "ExcBundlesheet::ExcBundlesheet - table name too long" );
    aName = ByteString( sTabName, *rRootData.pCharset );
}


void ExcBundlesheet::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetStreamPos();
    rStrm   << (UINT32) 0x00000000              // dummy (stream position of the sheet)
            << nGrbit;
    rStrm.WriteByteString( aName );             // 8 bit length, max 255 chars
}


ULONG ExcBundlesheet::GetLen() const
{
    return 7 + Min( aName.Len(), (xub_StrLen) 255 );
}


//--------------------------------------------------------- class ExcDummy_02 -

ULONG ExcDummy_02a::GetLen( void ) const
{
    return nMyLen;
}

const BYTE* ExcDummy_02a::GetData( void ) const
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
        ::svx::ConvertLanguageToCountry( rRoot.GetDocLanguage() ) );
}

void XclExpCountry::WriteBody( XclExpStream& rStrm )
{
    rStrm << mnUICountry << mnDocCountry;
}


//---------------------------------------------------- class ExcNameListEntry -

ExcNameListEntry::ExcNameListEntry() :
    mxTokArr( new XclExpTokenArray ),
    nTabNum( 0 ),
    nBuiltInKey( EXC_BUILTIN_UNKNOWN ),
    bDummy( FALSE )
{
}


ExcNameListEntry::ExcNameListEntry( RootData& rRootData, SCTAB nScTab, UINT8 nKey ) :
    mxTokArr( new XclExpTokenArray ),
    nTabNum( rRootData.pER->GetTabInfo().GetXclTab( nScTab ) + 1 ),
    nBuiltInKey( nKey ),
    bDummy( FALSE )
{
}


ExcNameListEntry::~ExcNameListEntry()
{
}


void ExcNameListEntry::SaveCont( XclExpStream& rStrm )
{
    if(rStrm.GetRoot().GetBiff() < xlBiff8)
    {
        rStrm   << (UINT16) EXC_NAME_BUILTIN    // grbit (built in only)
            << (UINT8)  0x00                // chKey (keyboard shortcut)
            << (UINT8)  0x01                // cch (string len)
            << mxTokArr->GetSize()          // cce (formula len)
            << nTabNum                      // set ixals = itab
            << nTabNum                      // itab (table index, 1-based)
            << (UINT32) 0x00000000          // cch
            << nBuiltInKey;                 // string
    }
    else
    {
        rStrm   << (UINT16) EXC_NAME_BUILTIN    // grbit (built in only)
            << (UINT8)  0x00                // chKey (keyboard shortcut)
            << (UINT8)  0x01                // cch (string len)
            << mxTokArr->GetSize()          // cce (formula len)
            << (UINT16) 0x0000              // ixals
            << nTabNum                      // itab (table index, 1-based)
            << (UINT32) 0x00000000          // cch
            << (UINT8)  0x00                // string grbit
            << nBuiltInKey;                 // string
    }
    mxTokArr->WriteArray( rStrm );
}


UINT16 ExcNameListEntry::GetNum() const
{
    return 0x0018;
}


ULONG ExcNameListEntry::GetLen() const
{
    return 16 + mxTokArr->GetSize();
}

const String& ExcNameListEntry::GetName() const
{
    return EMPTY_STRING;
}


//------------------------------------------------------------- class ExcName -

void ExcName::Init( BOOL bHid, BOOL bBIn, BOOL bBMacro )
{
    eBiff = pExcRoot->eDateiTyp;
    bHidden = bHid;
    bBuiltIn = bBIn;
    bMacro = bBMacro;
}


void ExcName::BuildFormula( const ScRange& rRange )
{
    mxTokArr = pExcRoot->pER->GetFormulaCompiler().CreateRangeRefFormula( rRange, true );
}


ExcName::ExcName( RootData& rRootData, ScRangeData* pRange ) :
        ExcRoot( &rRootData )
{
    Init();

    String aRangeName;
    pRange->GetName( aRangeName );

    // no PrintRanges, no PrintTitles
    if( SetBuiltInName( aRangeName, EXC_BUILTIN_PRINTAREA ) ||
        SetBuiltInName( aRangeName, EXC_BUILTIN_PRINTTITLES ) )
        return;

    SetName( aRangeName );
    const ScTokenArray* pTokArray = pRange->GetCode();
    if( pTokArray && pTokArray->GetLen() )
        mxTokArr = pExcRoot->pER->GetFormulaCompiler().CreateNameFormula( *pTokArray );
}


ExcName::ExcName( RootData& rRootData, ScDBData* pArea ) :
        ExcRoot( &rRootData )
{
    Init();

    String aRangeName;
    pArea->GetName( aRangeName );

    if( IsBuiltInAFName( aRangeName, EXC_BUILTIN_AUTOFILTER ))
        return;

    SetUniqueName( aRangeName );

    ScRange aRange;
    pArea->GetArea( aRange );
    BuildFormula( aRange );
}


ExcName::ExcName( RootData& rRootData, const ScRange& rRange, const String& rName ) :
        ExcRoot( &rRootData )
{
    Init();
    SetUniqueName( rName );
    BuildFormula( rRange );
}


ExcName::ExcName( RootData& rRootData, const ScRange& rRange, UINT8 nKey, BOOL bHid ) :
        ExcNameListEntry( rRootData, rRange.aStart.Tab(), nKey ),
        ExcRoot( &rRootData )
{
    Init( bHid, TRUE, FALSE );
    aName = sal_Unicode( nKey );
    BuildFormula( rRange );
}

ExcName::ExcName( RootData& rRootData, const String& rName ) :
        ExcRoot( &rRootData )
{
    Init();
    bMacro = TRUE;
    aName = rName;
}


void ExcName::SetName( const String& rRangeName )
{
    DBG_ASSERT( pExcRoot->pScNameList, "ExcName::SetName - missing name list" );
    ScRangeName& rNameList = *pExcRoot->pScNameList;
    aName = rRangeName;

    // insert dummy range name
    ScRangeData* pRangeData = new ScRangeData( pExcRoot->pDoc, aName, ScTokenArray() );
    if( !rNameList.Insert( pRangeData ) )
        delete pRangeData;
}


void ExcName::SetUniqueName( const String& rRangeName )
{
    DBG_ASSERT( pExcRoot->pScNameList, "ExcName::SetUniqueName - missing name list" );
    ScRangeName& rNameList = *pExcRoot->pScNameList;

    USHORT nPos;
    if( rNameList.SearchName( rRangeName, nPos ) )
    {
        String aNewName;
        sal_Int32 nAppendValue = 1;
        do
        {
            aNewName = rRangeName;
            aNewName += '_';
            aNewName += String::CreateFromInt32( nAppendValue++ );
        }
        while( rNameList.SearchName( aNewName, nPos ) );
        SetName( aNewName );
    }
    else
        SetName( rRangeName );
}


BOOL ExcName::SetBuiltInName( const String& rName, UINT8 nKey )
{
    if( XclTools::IsBuiltInDefName( nTabNum, rName, nKey ) )
    {
        nBuiltInKey = nKey;
        bDummy = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL ExcName::IsBuiltInAFName( const String& rName, UINT8 nKey )
{
    if( XclTools::IsBuiltInDefName( nTabNum, rName, nKey ) ||
        (rName == ScGlobal::GetRscString( STR_DB_NONAME )))
    {
        bDummy = TRUE;
        return TRUE;
    }
    return FALSE;
}

void ExcName::SaveCont( XclExpStream& rStrm )
{
    UINT8   nNameLen = (UINT8) Min( aName.Len(), (xub_StrLen)255 );
    UINT16  nGrbit = (bHidden ? EXC_NAME_HIDDEN : 0) | (bBuiltIn ? EXC_NAME_BUILTIN : 0);
    if( mxTokArr->Empty() || bMacro )
        nGrbit |= EXC_NAME_FUNC | EXC_NAME_VB | EXC_NAME_PROC;

    rStrm   << nGrbit                   // grbit
            << (BYTE) 0x00              // chKey
            << nNameLen                 // cch
            << mxTokArr->GetSize()      // cce
            << (UINT16) 0x0000          // ixals
            << nTabNum                  // itab
            << (UINT32) 0x00000000;     // cch...

    if ( eBiff < Biff8 )
        rStrm.WriteByteStringBuffer( ByteString( aName, *pExcRoot->pCharset ), nNameLen );
    else
    {
        XclExpString aUni( aName, EXC_STR_8BITLENGTH );
        aUni.WriteFlagField( rStrm );
        aUni.WriteBuffer( rStrm );
    }

    mxTokArr->WriteArray( rStrm );
}


ULONG ExcName::GetLen() const
{   // only a guess for Biff8 (8bit/16bit unknown)
    return 14 + mxTokArr->GetSize() + (eBiff < Biff8 ? 0 : 1) + Min( aName.Len(), (xub_StrLen)255 );
}



// ---- class XclBuildInName -----------------------------------------

XclBuildInName::XclBuildInName( RootData& rRootData, SCTAB nScTab, UINT8 nKey ) :
    ExcNameListEntry( rRootData, nScTab, nKey )
{
}


void XclBuildInName::CreateFormula( RootData& rRootData )
{
    mxTokArr = rRootData.pER->GetFormulaCompiler().CreateRangeListFormula( aRL, true );
    bDummy = !mxTokArr;
}


// ---- class XclPrintRange, class XclTitleRange ---------------------

XclPrintRange::XclPrintRange( RootData& rRootData, SCTAB nScTab ) :
        XclBuildInName( rRootData, nScTab, EXC_BUILTIN_PRINTAREA )
{
    if( rRootData.pDoc->HasPrintRange() )
    {
        UINT16 nCount = rRootData.pDoc->GetPrintRangeCount( nScTab );
        for( UINT16 nIx = 0 ; nIx < nCount ; nIx++ )
        {
            ScRange aRange( *rRootData.pDoc->GetPrintRange( nScTab, nIx ) );
            // ScDocument does not care about sheet index in print ranges
            aRange.aStart.SetTab( nScTab );
            aRange.aEnd.SetTab( nScTab );
            Append( aRange );
        }
    }
    CreateFormula( rRootData );
}




XclPrintTitles::XclPrintTitles( RootData& rRootData, SCTAB nScTab ) :
        XclBuildInName( rRootData, nScTab, EXC_BUILTIN_PRINTTITLES )
{
    UINT16 nXclTab = rRootData.pER->GetTabInfo().GetXclTab( nScTab );
    const ScRange* pRange = rRootData.pDoc->GetRepeatColRange( nScTab );
    if( pRange )
        Append( ScRange( pRange->aStart.Col(), 0, static_cast<SCTAB>(nXclTab),
            pRange->aEnd.Col(), rRootData.nRowMax, static_cast<SCTAB>(nXclTab) ) );
    pRange = rRootData.pDoc->GetRepeatRowRange( nScTab );
    if( pRange )
        Append( ScRange( 0, pRange->aStart.Row(), static_cast<SCTAB>(nXclTab),
            rRootData.nColMax, pRange->aEnd.Row(), static_cast<SCTAB>(nXclTab) ) );
    CreateFormula( rRootData );
}



//--------------------------------------------------------- class ExcNameList -

ExcNameList::ExcNameList( RootData& rRootData ) :
    nFirstPrintRangeIx( 0 ),
    nFirstPrintTitleIx( 0 ),
    nFirstOtherNameIx( 0 )
{
    ScDocument&         rDoc = *rRootData.pDoc;
    XclExpTabInfo&      rTabInfo = rRootData.pER->GetTabInfo();
    USHORT              nCount, nIndex;
    SCTAB               nScTab, nTab;
    UINT16              nExpIx;

    // print ranges and print titles, insert in table name sort order
    SCTAB nScTabCount = rTabInfo.GetScTabCount();
    for( nTab = 0; nTab < nScTabCount; ++nTab )
    {
        nScTab = rTabInfo.GetRealScTab( nTab ); // sorted -> real
        if( rTabInfo.IsExportTab( nScTab ) )
            Append( new XclPrintRange( rRootData, nScTab ) );
    }
    nFirstPrintTitleIx = List::Count();
    for( nTab = 0; nTab < nScTabCount; ++nTab )
    {
        nScTab = rTabInfo.GetRealScTab( nTab ); // sorted -> real
        if( rTabInfo.IsExportTab( nScTab ) )
            Append( new XclPrintTitles( rRootData, nScTab ) );
    }
    nFirstOtherNameIx = List::Count();

    // named ranges
    ScRangeName* pRangeNames = rDoc.GetRangeName();
    DBG_ASSERT( pRangeNames, "ExcNameList::ExcNameList - missing range name list" );
    nCount = pRangeNames->GetCount();
    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScRangeData* pData = (*pRangeNames)[ nIndex ];
        DBG_ASSERT( pData, "ExcNameList::ExcNameList - missing range name" );

        if ( !pData->HasType( RT_SHARED ) )
        {
            ExcName* pExcName = new ExcName( rRootData, pData );
            if( pExcName->IsDummy() )
            {
                nExpIx = GetBuiltInIx( pExcName );
                delete pExcName;
            }
            else
                nExpIx = Append( pExcName );
            pData->SetExportIndex( nExpIx );
        }
    }

    // data base ranges
    ScDBCollection* pDBAreas = rDoc.GetDBCollection();
    DBG_ASSERT( pDBAreas, "ExcNameList::ExcNameList - missing db area list" );
    nCount = pDBAreas->GetCount();
    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScDBData* pData = (*pDBAreas)[ nIndex ];
        DBG_ASSERT( pData, "ExcNameList::ExcNameList - missing db area" );

        ExcName* pExcName = new ExcName( rRootData, pData );
        if( pExcName->IsDummy() )
        {
            delete pExcName;
        }
        else
        {
            nExpIx = Append( pExcName );
            pData->SetExportIndex( nExpIx );
        }
    }

    maNextInsVec.resize( nScTabCount, Count() );
}


ExcNameList::~ExcNameList()
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        delete pName;
}


UINT16 ExcNameList::Append( ExcNameListEntry* pName )
{
    DBG_ASSERT( pName, "ExcNameList::Append - missing ExcName" );

    BOOL bDelete = (pName->IsDummy() || (List::Count() >= 0xFFFE));
    if( bDelete )
        delete pName;
    else
        List::Insert( pName, LIST_APPEND );
    return bDelete ? 0xFFFF : (UINT16) List::Count();
}


void ExcNameList::InsertSorted( RootData& rRootData, ExcNameListEntry* pName, SCTAB nScTab )
{
    // real -> sorted
    SCTAB nSortedScTab = rRootData.pER->GetTabInfo().GetSortedScTab( nScTab );
    List::Insert( pName, maNextInsVec[ nSortedScTab ] );
    for( SCTAB nCount = static_cast< SCTAB >( maNextInsVec.size() ); nSortedScTab < nCount; ++nSortedScTab )
        ++maNextInsVec[ nSortedScTab ];
}

UINT16 ExcNameList::GetBuiltInIx( const ExcNameListEntry* pName )
{
    DBG_ASSERT( pName, "ExcNameList::GetBuiltInIx - missing ExcName" );

    ULONG nFirstIx = 0;
    ULONG nLastIx = 0;

    switch( pName->GetBuiltInKey() )
    {
        case EXC_BUILTIN_PRINTAREA:
            nFirstIx = nFirstPrintRangeIx; nLastIx = nFirstPrintTitleIx;
        break;
        case EXC_BUILTIN_PRINTTITLES:
            nFirstIx = nFirstPrintTitleIx; nLastIx = nFirstOtherNameIx;
        break;
        default:
            return 0xFFFF;
    }

    for( ULONG nIndex = nFirstIx; nIndex < nLastIx; nIndex++ )
    {
        ExcNameListEntry* pEntry = _Get( nIndex );
        if( pEntry && (pEntry->GetTabIndex() == pName->GetTabIndex()) )
        {
            DBG_ASSERT( pEntry->GetBuiltInKey() == pName->GetBuiltInKey(),
                        "ExcNameList::GetBuiltInIx - builtin key mismatch" );
            return (UINT16)(nIndex + 1);
        }
    }
    return 0xFFFF;
}

UINT16 ExcNameList::GetMacroIdx( RootData& rRootData, const String &rName )
{
    for( USHORT nIndex = 0; nIndex < List::Count(); nIndex++ )
    {
        ExcNameListEntry* pName = _Get( nIndex );
        if(pName && pName->GetName().Equals(rName))
            return nIndex+1;
    }
    return Append( new ExcName( rRootData, rName ) );
}

void ExcNameList::Save( XclExpStream& rStrm )
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        pName->Save( rStrm );
}


// ============================================================================

ExcExterncount::ExcExterncount( RootData* pRD, const BOOL bTableNew ) :
    ExcRoot( pRD ),
    bTable( bTableNew )
{
}


void ExcExterncount::SaveCont( XclExpStream& rStrm )
{
    UINT16  nNumTabs = pExcRoot->pER->GetTabInfo().GetXclTabCount();

    if( nNumTabs && bTable )
        nNumTabs--;

    rStrm << nNumTabs;
}


UINT16 ExcExterncount::GetNum( void ) const
{
    return 0x0016;
}


ULONG ExcExterncount::GetLen( void ) const
{
    return 2;
}



//------------------------------------------------------ class ExcExternsheet -

ExcExternsheet::ExcExternsheet( RootData* pExcRoot, const SCTAB nNewTabNum ) : ExcRoot( pExcRoot )
{
    DBG_ASSERT( pExcRoot->pDoc->HasTable( nNewTabNum ),
        "*ExcExternsheet::ExcExternsheet(): table not found" );

    pExcRoot->pDoc->GetName( nNewTabNum, aTabName );
    DBG_ASSERT( aTabName.Len() < 255,
        "*ExcExternsheet::ExcExternsheet(): table name too long" );
}


void ExcExternsheet::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT8 ) Min( (xub_StrLen)(aTabName.Len() ), (xub_StrLen) 255 ) << ( UINT8 ) 0x03;
    rStrm.WriteByteStringBuffer(
        ByteString( aTabName, *pExcRoot->pCharset ), 254 );     // max 254 chars (leading 0x03!)
}


UINT16 ExcExternsheet::GetNum( void ) const
{
    return 0x0017;
}


ULONG ExcExternsheet::GetLen( void ) const
{
    return 2 + Min( aTabName.Len(), (xub_StrLen) 254 );
}



//-------------------------------------------------- class ExcExternsheetList -

ExcExternsheetList::~ExcExternsheetList()
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        delete pSheet;
}


void ExcExternsheetList::Save( XclExpStream& rStrm )
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        pSheet->Save( rStrm );
}



//-------------------------------------------------------- class ExcExternDup -

ExcExternDup::ExcExternDup( ExcExterncount& rC, ExcExternsheetList& rL ) :
    rExtCnt( rC ), rExtSheetList( rL )
{
}


ExcExternDup::ExcExternDup( const ExcExternDup& r ) :
    rExtCnt( r.rExtCnt ), rExtSheetList( r.rExtSheetList )
{
}


void ExcExternDup::Save( XclExpStream& rStrm )
{
    rExtCnt.Save( rStrm );
    rExtSheetList.Save( rStrm );
}



//---------------------------------------------------------- class ExcWindow2 -

ExcWindow2::ExcWindow2( UINT16 nTab ) : nTable( nTab )
{
}


void ExcWindow2::SaveCont( XclExpStream& rStrm )
{
    BYTE pData[] = { 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    DBG_ASSERT( sizeof(pData) == GetLen(), "ExcWindow2::SaveCont: length mismatch" );
    if ( GetTable() == 0 )
        pData[1] |= 0x06;       // displayed and selected
    rStrm.Write( pData, GetLen() );
}


UINT16 ExcWindow2::GetNum( void ) const
{
    return 0x023E;
}


ULONG ExcWindow2::GetLen( void ) const
{
    return 10;
}



//-------------------------------------------------------- class ExcSelection -

void ExcSelection::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nPane                // pane
            << nRow << nCol         // active cell
            << (UINT16) 0           // index in ref array
            << (UINT16) 1           // size of ref array
            << nRow << nRow         // ref array (activ cell only)
            << (UINT8) nCol << (UINT8) nCol;
}


UINT16 ExcSelection::GetNum( void ) const
{
    return 0x001D;
}


ULONG ExcSelection::GetLen( void ) const
{
    return 15;
}


// XclExpWsbool ===============================================================

XclExpWsbool::XclExpWsbool( bool bFitToPages ) :
    XclExpUInt16Record( EXC_ID_WSBOOL, EXC_WSBOOL_DEFAULTFLAGS )
{
    if( bFitToPages )
        SetValue( GetValue() | EXC_WSBOOL_FITTOPAGE );
}


// XclExpWindowProtection ===============================================================

XclExpWindowProtection::XclExpWindowProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_WINDOWPROTECT,bValue)
{
}

// XclExpDocProtection ===============================================================

XclExpDocProtection::XclExpDocProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_PROTECT,bValue)
{
}

// ============================================================================
//---------------------------------------------------------------- AutoFilter -

UINT16 ExcFilterMode::GetNum() const
{
    return 0x009B;
}

ULONG ExcFilterMode::GetLen() const
{
    return 0;
}




ExcAutoFilterInfo::~ExcAutoFilterInfo()
{
}

void ExcAutoFilterInfo::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCount;
}

UINT16 ExcAutoFilterInfo::GetNum() const
{
    return 0x009D;
}

ULONG ExcAutoFilterInfo::GetLen() const
{
    return 2;
}




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

ULONG ExcFilterCondition::GetTextBytes() const
{
    return pText ? 1 + pText->GetBufferSize() : 0;
}

void ExcFilterCondition::SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT )
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
            rStrm << (UINT32)0 << (UINT8) pText->Len() << (UINT16)0 << (UINT8)0;
        break;
        case EXC_AFTYPE_BOOLERR:
            rStrm << (UINT8)0 << (UINT8)((fVal != 0) ? 1 : 0) << (UINT32)0 << (UINT16)0;
        break;
        default:
            rStrm << (UINT32)0 << (UINT32)0;
    }
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




ExcAutoFilter::ExcAutoFilter( UINT16 nC ) :
        nCol( nC ),
        nFlags( 0 )
{
}

BOOL ExcAutoFilter::AddCondition( ScQueryConnect eConn, UINT8 nType, UINT8 nOp,
                                    double fVal, String* pText, BOOL bSimple )
{
    if( !aCond[ 1 ].IsEmpty() )
        return FALSE;

    UINT16 nInd = aCond[ 0 ].IsEmpty() ? 0 : 1;

    if( nInd == 1 )
        nFlags |= (eConn == SC_OR) ? EXC_AFFLAG_OR : EXC_AFFLAG_AND;
    if( bSimple )
        nFlags |= (nInd == 0) ? EXC_AFFLAG_SIMPLE1 : EXC_AFFLAG_SIMPLE2;

    aCond[ nInd ].SetCondition( nType, nOp, fVal, pText );
    return TRUE;
}

BOOL ExcAutoFilter::AddEntry( RootData& rRoot, const ScQueryEntry& rEntry )
{
    BOOL    bConflict = FALSE;
    String  sText;

    if( rEntry.pStr )
        sText.Assign( *rEntry.pStr );

    BOOL bLen = TRUEBOOL( sText.Len() );

    // empty/nonempty fields
    if( !bLen && (rEntry.nVal == SC_EMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_EMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    else if( !bLen && (rEntry.nVal == SC_NONEMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_NOTEMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    // other conditions
    else
    {
        double  fVal    = 0.0;
        ULONG   nIndex  = 0;
        BOOL    bIsNum  = bLen ? rRoot.pDoc->GetFormatTable()->IsNumberFormat( sText, nIndex, fVal ) : TRUE;
        String* pText   = bIsNum ? NULL : &sText;

        // top10 flags
        UINT16 nNewFlags = 0x0000;
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
        }
        BOOL bNewTop10 = TRUEBOOL( nNewFlags & EXC_AFFLAG_TOP10 );

        bConflict = HasTop10() && bNewTop10;
        if( !bConflict )
        {
            if( bNewTop10 )
            {
                if( fVal < 0 )      fVal = 0;
                if( fVal >= 501 )   fVal = 500;
                nFlags |= (nNewFlags | (UINT16)(fVal) << 7);
            }
            // normal condition
            else
            {
                UINT8 nType = bIsNum ? EXC_AFTYPE_DOUBLE : EXC_AFTYPE_STRING;
                UINT8 nOper = EXC_AFOPER_NONE;

                switch( rEntry.eOp )
                {
                    case SC_EQUAL:          nOper = EXC_AFOPER_EQUAL;           break;
                    case SC_LESS:           nOper = EXC_AFOPER_LESS;            break;
                    case SC_GREATER:        nOper = EXC_AFOPER_GREATER;         break;
                    case SC_LESS_EQUAL:     nOper = EXC_AFOPER_LESSEQUAL;       break;
                    case SC_GREATER_EQUAL:  nOper = EXC_AFOPER_GREATEREQUAL;    break;
                    case SC_NOT_EQUAL:      nOper = EXC_AFOPER_NOTEQUAL;        break;
                }
                bConflict = !AddCondition( rEntry.eConnect, nType, nOper, fVal, pText );
            }
        }
    }
    return bConflict;
}

void ExcAutoFilter::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCol << nFlags;
    aCond[ 0 ].Save( rStrm );
    aCond[ 1 ].Save( rStrm );
    aCond[ 0 ].SaveText( rStrm );
    aCond[ 1 ].SaveText( rStrm );
}

UINT16 ExcAutoFilter::GetNum() const
{
    return 0x009E;
}

ULONG ExcAutoFilter::GetLen() const
{
    return 24 + aCond[ 0 ].GetTextBytes() + aCond[ 1 ].GetTextBytes();
}




ExcAutoFilterRecs::ExcAutoFilterRecs( RootData& rRoot, SCTAB nTab ) :
        pFilterMode( NULL ),
        pFilterInfo( NULL )
{
    ScDBCollection& rDBColl = *rRoot.pDoc->GetDBCollection();

    // search for first DB-range with filter
    UINT16      nIndex  = 0;
    BOOL        bFound  = FALSE;
    BOOL        bAdvanced = FALSE;
    ScDBData*   pData   = NULL;
    ScRange     aRange;
    ScRange     aAdvRange;
    while( (nIndex < rDBColl.GetCount()) && !bFound )
    {
        pData = rDBColl[ nIndex ];
        if( pData )
        {
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

        rRoot.pNameList->InsertSorted( rRoot, new ExcName( rRoot, aRange, EXC_BUILTIN_AUTOFILTER, TRUE ), nTab );

        // advanced filter
        if( bAdvanced )
        {
            // filter criteria, excel allows only same table
            if( aAdvRange.aStart.Tab() == nTab )
                rRoot.pNameList->InsertSorted( rRoot,
                    new ExcName( rRoot, aAdvRange, EXC_BUILTIN_CRITERIA ), nTab );

            // filter destination range, excel allows only same table
            if( !aParam.bInplace )
            {
                ScRange aDestRange( aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
                aDestRange.aEnd.IncCol( nColCnt - 1 );
                if( aDestRange.aStart.Tab() == nTab )
                    rRoot.pNameList->InsertSorted( rRoot,
                        new ExcName( rRoot, aDestRange, EXC_BUILTIN_EXTRACT ), nTab );
            }

            pFilterMode = new ExcFilterMode;
        }
        // AutoFilter
        else
        {
            BOOL    bConflict   = FALSE;
            BOOL    bContLoop   = TRUE;
            BOOL    bHasOr      = FALSE;
            SCCOLROW nFirstField = aParam.GetEntry( 0 ).nField;

            // create AUTOFILTER records for filtered columns
            for( SCSIZE nEntry = 0; !bConflict && bContLoop && (nEntry < aParam.GetEntryCount()); nEntry++ )
            {
                const ScQueryEntry& rEntry  = aParam.GetEntry( nEntry );

                bContLoop = rEntry.bDoQuery;
                if( bContLoop )
                {
                    ExcAutoFilter* pFilter = GetByCol( static_cast<SCCOL>(rEntry.nField) - aRange.aStart.Col() );

                    if( nEntry > 0 )
                        bHasOr |= (rEntry.eConnect == SC_OR);

                    bConflict = (nEntry > 1) && bHasOr;
                    if( !bConflict )
                        bConflict = (nEntry == 1) && (rEntry.eConnect == SC_OR) &&
                                    (nFirstField != rEntry.nField);
                    if( !bConflict )
                        bConflict = pFilter->AddEntry( rRoot, rEntry );
                }
            }

            // additional tests for conflicts
            for( ExcAutoFilter* pFilter = _First(); !bConflict && pFilter; pFilter = _Next() )
                bConflict = pFilter->HasCondition() && pFilter->HasTop10();

            if( bConflict )
                DeleteList();

            if( List::Count() )
                pFilterMode = new ExcFilterMode;
            pFilterInfo = new ExcAutoFilterInfo( nColCnt );
            AddObjRecs( rRoot, aRange.aStart, nColCnt );
        }
    }
}

ExcAutoFilterRecs::~ExcAutoFilterRecs()
{
    if( pFilterMode )
        delete pFilterMode;
    if( pFilterInfo )
        delete pFilterInfo;
    DeleteList();
}

void ExcAutoFilterRecs::DeleteList()
{
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        delete pFilter;
    List::Clear();
}

ExcAutoFilter* ExcAutoFilterRecs::GetByCol( SCCOL nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == static_cast<sal_uInt16>(nCol) )
            return pFilter;
    pFilter = new ExcAutoFilter( static_cast<sal_uInt16>(nCol) );
    Append( pFilter );
    return pFilter;
}

BOOL ExcAutoFilterRecs::IsFiltered( SCCOL nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == static_cast<sal_uInt16>(nCol) )
            return TRUE;
    return FALSE;
}

void ExcAutoFilterRecs::AddObjRecs( RootData& rRoot, const ScAddress& rPos, SCCOL nCols )
{
    ScAddress aAddr( rPos );
    for( SCCOL nObj = 0; nObj < nCols; nObj++ )
    {
        XclObjDropDown* pObj = new XclObjDropDown( *rRoot.pER, aAddr, IsFiltered( nObj ) );
        rRoot.pObjRecs->Add( pObj );
        aAddr.IncCol( 1 );
    }
}

void ExcAutoFilterRecs::Save( XclExpStream& rStrm )
{
    if( pFilterMode )
        pFilterMode->Save( rStrm );
    if( pFilterInfo )
        pFilterInfo->Save( rStrm );
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        pFilter->Save( rStrm );
}

