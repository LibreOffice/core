/*************************************************************************
 *
 *  $RCSfile: xlformula.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-10-18 15:16:53 $
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

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

// Token array ================================================================

bool XclTokenArrayHelper::GetString( String& rString, const ScTokenArray& rScTokArr )
{
    bool bRet = false;
    ScToken** ppToken = rScTokArr.GetArray();
    ScToken** ppEndToken = ppToken + rScTokArr.GetLen();
    if( ppToken && (ppToken < ppEndToken) )
    {
        enum { stBegin, stStr, stError } eState = stBegin;  // last read token
        while( (eState != stError) && (ppToken < ppEndToken) )
        {
            if( const ScToken* pToken = *ppToken++ )
            {
                if( pToken->GetOpCode() != ocSpaces )
                {
                    switch( eState )
                    {
                        case stBegin:
                            eState = (pToken->GetType() == svString) ? stStr : stError;
                            if( eState == stStr )
                                rString = pToken->GetString();
                        break;
                        case stStr:
                            // something is following the string token -> error
                            eState = stError;
                        break;
                        default:
                            eState = stError;
                    }
                }
            }
            else
                eState = stError;
        }
        bRet = (eState != stError);
    }
    return bRet;
}

bool XclTokenArrayHelper::GetStringList( String& rStringList, const ScTokenArray& rScTokArr, sal_Unicode cSep )
{
    bool bRet = false;
    ScToken** ppToken = rScTokArr.GetArray();
    ScToken** ppEndToken = ppToken + rScTokArr.GetLen();
    if( ppToken && (ppToken < ppEndToken) )
    {
        enum { stBegin, stStr, stSep, stError } eState = stBegin;   // last read token
        while( (eState != stError) && (ppToken < ppEndToken) )
        {
            if( const ScToken* pToken = *ppToken++ )
            {
                if( pToken->GetOpCode() != ocSpaces )
                {
                    switch( eState )
                    {
                        case stBegin:
                        case stSep:
                            eState = (pToken->GetType() == svString) ? stStr : stError;
                            if( eState == stStr )
                                rStringList.Append( pToken->GetString() );
                        break;
                        case stStr:
                            eState = (pToken->GetOpCode() == ocSep) ? stSep : stError;
                            if( eState == stSep )
                                rStringList.Append( cSep );
                        break;
                        default:
                            eState = stError;
                    }
                }
            }
            else
                eState = stError;
        }
        bRet = (eState != stError);
    }
    return bRet;
}

void XclTokenArrayHelper::ConvertStringToList( ScTokenArray& rScTokArr, sal_Unicode cStringSep )
{
    String aString;
    if( GetString( aString, rScTokArr ) )
    {
        rScTokArr.Clear();
        xub_StrLen nTokenCnt = aString.GetTokenCount( cStringSep );
        xub_StrLen nStringIx = 0;
        for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
        {
            String aToken( aString.GetToken( 0, cStringSep, nStringIx ) );
            if( nToken > 0 )
                rScTokArr.AddOpCode( ocSep );
            rScTokArr.AddString( aToken );
        }
    }
}

// ----------------------------------------------------------------------------

const ScRangeData* XclTokenArrayHelper::GetSharedFormula( const XclRoot& rRoot, const ScTokenArray& rScTokArr )
{
    if( rScTokArr.GetLen() == 1 )
        if( const ScToken* pToken = const_cast< ScTokenArray& >( rScTokArr ).First() )
            if( pToken->GetOpCode() == ocName )
                if( const ScRangeData* pData = rRoot.GetNamedRanges().FindIndex( pToken->GetIndex() ) )
                    if( pData->HasType( RT_SHARED ) )
                        return pData;
    return 0;
}

// ----------------------------------------------------------------------------

namespace {

inline bool lclGetAddress( ScAddress& rAddress, const ScToken& rToken )
{
    OpCode eOpCode = rToken.GetOpCode();
    bool bIsSingleRef = (eOpCode == ocPush) && (rToken.GetType() == svSingleRef);
    if( bIsSingleRef )
    {
        const SingleRefData& rRef = rToken.GetSingleRef();
        rAddress.Set( rRef.nCol, rRef.nRow, rRef.nTab );
        bIsSingleRef = !rRef.IsDeleted();
    }
    return bIsSingleRef;
}

} // namespace

bool XclTokenArrayHelper::GetMultipleOpRefs( XclMultipleOpRefs& rRefs, const ScTokenArray& rScTokArr )
{
    rRefs.mbDblRefMode = false;
    bool bRet = false;
    ScToken** ppToken = rScTokArr.GetArray();
    ScToken** ppEndToken = ppToken + rScTokArr.GetLen();
    if( ppToken && (ppToken < ppEndToken) )
    {
        enum
        {
            stBegin, stTableOp, stOpen, stFormula, stFormulaSep,
            stColFirst, stColFirstSep, stColRel, stColRelSep,
            stRowFirst, stRowFirstSep, stRowRel, stClose, stError
        } eState = stBegin;     // last read token

        while( (eState != stError) && (ppToken < ppEndToken) )
        {
            if( const ScToken* pToken = *ppToken++ )
            {
                OpCode eOpCode = pToken->GetOpCode();
                if( eOpCode != ocSpaces )
                {
                    bool bIsSep = (eOpCode == ocSep);
                    switch( eState )
                    {
                        case stBegin:
                            eState = (eOpCode == ocTableOp) ? stTableOp : stError;
                        break;
                        case stTableOp:
                            eState = (eOpCode == ocOpen) ? stOpen : stError;
                        break;
                        case stOpen:
                            eState = lclGetAddress( rRefs.maFmlaScPos, *pToken ) ? stFormula : stError;
                        break;
                        case stFormula:
                            eState = bIsSep ? stFormulaSep : stError;
                        break;
                        case stFormulaSep:
                            eState = lclGetAddress( rRefs.maColFirstScPos, *pToken ) ? stColFirst : stError;
                        break;
                        case stColFirst:
                            eState = bIsSep ? stColFirstSep : stError;
                        break;
                        case stColFirstSep:
                            eState = lclGetAddress( rRefs.maColRelScPos, *pToken ) ? stColRel : stError;
                        break;
                        case stColRel:
                            eState = bIsSep ? stColRelSep : ((eOpCode == ocClose) ? stClose : stError);
                        break;
                        case stColRelSep:
                            eState = lclGetAddress( rRefs.maRowFirstScPos, *pToken ) ? stRowFirst : stError;
                            rRefs.mbDblRefMode = true;
                        break;
                        case stRowFirst:
                            eState = bIsSep ? stRowFirstSep : stError;
                        break;
                        case stRowFirstSep:
                            eState = lclGetAddress( rRefs.maRowRelScPos, *pToken ) ? stRowRel : stError;
                        break;
                        case stRowRel:
                            eState = (eOpCode == ocClose) ? stClose : stError;
                        break;
                        default:
                            eState = stError;
                    }
                }
            }
            else
                eState = stError;
        }
        bRet = (eState == stClose);
    }
    return bRet;
}

// ============================================================================

