/*************************************************************************
 *
 *  $RCSfile: xiname.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:05:56 $
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

#ifndef SC_XINAME_HXX
#include "xiname.hxx"
#endif

#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif

// for formula compiler
#include "excform.hxx"
// for filter manager
#include "excimp8.hxx"

// ============================================================================
// *** Helper classes ***
// ============================================================================

// ============================================================================
// *** Implementation ***
// ============================================================================

XclImpName::XclImpName( XclImpStream& rStrm, sal_uInt16 nScIndex ) :
    XclImpRoot( rStrm.GetRoot() ),
    mpScData( 0 ),
    mcBuiltIn( EXC_BUILTIN_UNKNOWN ),
    mnScTab( SCTAB_MAX ),
    mbVBName( false )
{
    ExcelToSc& rFmlaConv = GetFmlaConverter();
    ScRangeName& rRangeNames = GetNamedRanges();

    // 1) *** read data from stream *** ---------------------------------------

    sal_uInt16 nFlags = 0, nFmlaSize, nExtSheet = EXC_NAME_GLOBAL;
    sal_uInt8 nNameLen, nShortCut;

    switch( GetBiff() )
    {
        case xlBiff2:
        {
            sal_uInt8 nFlagsBiff2;
            rStrm >> nFlagsBiff2;
            rStrm.Ignore( 1 );
            rStrm >> nShortCut >> nNameLen;
            nFmlaSize = rStrm.ReaduInt8();
            ::set_flag( nFlags, EXC_NAME_FUNC, ::get_flag( nFlagsBiff2, EXC_NAME2_FUNC ) );
        }
        break;

        case xlBiff3:
        case xlBiff4:
        {
            rStrm >> nFlags >> nShortCut >> nNameLen >> nFmlaSize;
        }
        break;

        case xlBiff5:
        case xlBiff7:
        case xlBiff8:
        {
            rStrm >> nFlags >> nShortCut >> nNameLen >> nFmlaSize;
            rStrm.Ignore( 2 );
            rStrm >> nExtSheet;
            rStrm.Ignore( 4 );
        }
        break;

        default: DBG_ERROR_BIFF();
    }

    if( GetBiff() < xlBiff8 )
        maXclName = rStrm.ReadRawByteString( nNameLen );
    else
        maXclName = rStrm.ReadUniString( nNameLen );

    // 2) *** convert sheet index and name *** --------------------------------

    // Visual Basic procedure
    mbVBName = ::get_flag( nFlags, EXC_NAME_VB );

    // get built-in name, or convert characters invalid in Calc
    bool bBuiltIn = ::get_flag( nFlags, EXC_NAME_BUILTIN );

    // special case for BIFF5 filter range - name appears as plain text without built-in flag
    if( ((GetBiff() == xlBiff5) || (GetBiff() == xlBiff7)) &&
        (maXclName == XclTools::GetXclBuiltInDefName( EXC_BUILTIN_FILTERDATABASE )) )
    {
        bBuiltIn = true;
        maXclName.Assign( EXC_BUILTIN_FILTERDATABASE );
    }

    // convert Excel name to Calc name
    if( mbVBName )
    {
        // VB macro name
        maScName = maXclName;
    }
    else if( bBuiltIn )
    {
        // built-in name
        if( maXclName.Len() )
            mcBuiltIn = maXclName.GetChar( 0 );
        if( mcBuiltIn == '?' )      // NUL character is imported as '?'
            mcBuiltIn = '\0';
        maScName = XclTools::GetBuiltInDefName( mcBuiltIn );
    }
    else
    {
        // any other name
        maScName = maXclName;
        ScfTools::ConvertToScDefinedName( maScName );
    }

    // add index for local names
    if( nExtSheet != EXC_NAME_GLOBAL )
    {
        maScName.Append( '_' ).Append( String::CreateFromInt32( nExtSheet ) );
        mnScTab = static_cast< SCTAB >( nExtSheet - 1 );
    }

    // find an unused name
    String aOrigName( maScName );
    sal_Int32 nCounter = 0;
    USHORT nDummy;
    while( rRangeNames.SearchName( maScName, nDummy ) )
        maScName.Assign( aOrigName ).Append( ' ' ).Append( String::CreateFromInt32( ++nCounter ) );

    // 3) *** convert the name definition formula *** -------------------------

    rFmlaConv.Reset();
    const ScTokenArray* pTokArr = 0; // pointer to token array, owned by rFmlaConv
    RangeType nNameType = RT_NAME;

    const sal_uInt16 nUnsupported = EXC_NAME_VB | EXC_NAME_PROC | EXC_NAME_BIG;
    if( ::get_flag( nFlags, nUnsupported ) )
    {
        // special, unsupported name
        rFmlaConv.GetDummy( pTokArr );
    }
    else if( bBuiltIn )
    {
        // --- print ranges or title ranges ---
        rStrm.PushPosition();
        switch( mcBuiltIn )
        {
            case EXC_BUILTIN_PRINTAREA:
                if( rFmlaConv.Convert( GetPrintAreaBuffer(), nFmlaSize, FT_RangeName ) == ConvOK )
                    nNameType |= RT_PRINTAREA;
            break;
            case EXC_BUILTIN_PRINTTITLES:
                if( rFmlaConv.Convert( GetTitleAreaBuffer(), nFmlaSize, FT_RangeName ) == ConvOK )
                    nNameType |= RT_COLHEADER | RT_ROWHEADER;
            break;
        }
        rStrm.PopPosition();

        // --- name formula ---
        rFmlaConv.Convert( pTokArr, nFmlaSize, FT_RangeName );

        // --- auto or advanced filter ---
        if( (GetBiff() >= xlBiff8) && pTokArr && bBuiltIn )
        {
            ScRange aRange;
            if( pTokArr->IsReference( aRange ) )
            {
                switch( mcBuiltIn )
                {
                    case EXC_BUILTIN_FILTERDATABASE:
                        GetFilterManager().Insert( mpRD, aRange, maScName );
                    break;
                    case EXC_BUILTIN_CRITERIA:
                        GetFilterManager().AddAdvancedRange( aRange );
                        nNameType |= RT_CRITERIA;
                    break;
                    case EXC_BUILTIN_EXTRACT:
                        if( pTokArr->IsValidReference( aRange ) )
                            GetFilterManager().AddExtractPos( aRange );
                    break;
                }
            }
        }
    }
    else if( nFmlaSize > 0 )
    {
        // regular defined name
        rFmlaConv.Convert( pTokArr, nFmlaSize, FT_RangeName );
    }

    // 4) *** create a defined name in the Calc document *** ------------------

    if( pTokArr && (bBuiltIn || !::get_flag( nFlags, EXC_NAME_HIDDEN )) && !mbVBName )
    {
        // create the Calc name data
        ScRangeData* pData = new ScRangeData( GetDocPtr(), maScName, *pTokArr, ScAddress(), nNameType );
        pData->GuessPosition();
        pData->SetIndex( nScIndex );
        rRangeNames.Insert( pData );        // takes ownership of pData
        mpScData = pData;                   // cache for later use
    }
}

// ----------------------------------------------------------------------------

XclImpNameManager::XclImpNameManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpNameManager::ReadName( XclImpStream& rStrm )
{
    ULONG nCount = maNameList.Count();
    if( nCount < 0xFFFF )
        maNameList.Append( new XclImpName( rStrm, static_cast< sal_uInt16 >( nCount + 1 ) ) );
}

const XclImpName* XclImpNameManager::FindName( const String& rXclName, SCTAB nScTab ) const
{
    const XclImpName* pGlobalName = 0;   // a found global name
    const XclImpName* pLocalName = 0;    // a found local name
    for( const XclImpName* pName = maNameList.First(); pName && !pLocalName; pName = maNameList.Next() )
    {
        if( pName->GetXclName() == rXclName )
        {
            if( pName->GetScTab() == nScTab )
                pLocalName = pName;
            else if( pName->IsGlobal() )
                pGlobalName = pName;
        }
    }
    return pLocalName ? pLocalName : pGlobalName;
}

const XclImpName* XclImpNameManager::GetName( sal_uInt16 nXclNameIdx ) const
{
    DBG_ASSERT( nXclNameIdx > 0, "XclImpNameManager::GetName - index must be >0" );
    return maNameList.GetObject( nXclNameIdx - 1 );
}

// ============================================================================

