/*************************************************************************
 *
 *  $RCSfile: unitconv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:19 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef SC_UNITCONV_HXX
#include "unitconv.hxx"
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_INIMGR_HXX //autogen
#include <sfx2/inimgr.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _TOOLS_INTN_HXX //autogen wg. International
#include <tools/intn.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

const sal_Unicode cDelim = 0x01;        // Delimiter zwischen From und To


// --- ScUnitConverterData --------------------------------------------

ScUnitConverterData::ScUnitConverterData( const String& rFromUnit,
            const String& rToUnit, double fVal )
        :
        StrData( rFromUnit ),
        fValue( fVal )
{
    String aTmp;
    ScUnitConverterData::BuildIndexString( aTmp, rFromUnit, rToUnit );
    SetString( aTmp );
}


ScUnitConverterData::ScUnitConverterData( const ScUnitConverterData& r )
        :
        StrData( r ),
        fValue( r.fValue )
{
}


DataObject* ScUnitConverterData::Clone() const
{
    return new ScUnitConverterData( *this );
}


// static
void ScUnitConverterData::BuildIndexString( String& rStr,
            const String& rFromUnit, const String& rToUnit )
{
#if 1
// case sensitive
    rStr = rFromUnit;
    rStr += cDelim;
    rStr += rToUnit;
#else
// not case sensitive
    rStr = rFromUnit;
    String aTo( rToUnit );
    ScGlobal::pCharClass->toUpper( rStr );
    ScGlobal::pCharClass->toUpper( aTo );
    rStr += cDelim;
    rStr += aTo;
#endif
}


// --- ScUnitConverter ------------------------------------------------

ScUnitConverter::ScUnitConverter( USHORT nInit, USHORT nDelta )
        :
        StrCollection( nInit, nDelta, FALSE )
{
    ReadIniFiles();
}


inline const sal_Unicode* SkipWhite( const sal_Unicode* p )
{
    while ( *p == ' ' || *p == '\t' )
        p++;
    return p;
}


void ScUnitConverter::ReadIniFiles()
{
    International aIntl( LANGUAGE_ENGLISH_US );
    String aName( RTL_CONSTASCII_USTRINGPARAM("convert.ini") );
    INetURLObject aURL;

    // share/config/convert.ini first, global definitions
    String aPath( SFX_INIMANAGER()->Get( SFX_KEY_CONFIG_DIR ) );
    if ( aPath.Len() > 0 )
    {
        aURL.SetSmartURL( aPath );
        aURL.setFinalSlash();
        aURL.Append( aName );
        ReadIniFile( aURL.GetMainURL(), aIntl );
    }

    // user/config/convert.ini may NOT overwrite globals (StrCollecion no dupes)
    aPath = SFX_INIMANAGER()->Get( SFX_KEY_USERCONFIG_PATH );
    if ( aPath.Len() > 0 )
    {
        aURL.SetSmartURL( aPath );
        aURL.setFinalSlash();
        aURL.Append( aName );
        ReadIniFile( aURL.GetMainURL(), aIntl );
    }
}


void ScUnitConverter::ReadIniFile( const String& rFileName, International& rIntl )
{
    SfxMedium aMedium( rFileName,
        STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE, TRUE );
    SvStream* pStream = aMedium.GetInStream();
    if ( !pStream || pStream->GetError() != 0 )
        return ;
    SvStream& rStream = *pStream;

    ByteString aByteLine;
    BOOL bIsConv = FALSE;
    while ( !bIsConv && rStream.ReadLine( aByteLine ) )
    {
        aByteLine.EraseAllChars( ' ' );
        bIsConv = aByteLine.Equals( "[UnitConversion]" );
    }

    while ( bIsConv && rStream.ReadLine( aByteLine ) )
    {
        String aLine( aByteLine, osl_getThreadTextEncoding() );
        const sal_Unicode* const pBeg = aLine.GetBuffer();
        register const sal_Unicode* p = SkipWhite( pBeg );
        // Format: "FromUnit";"ToUnit";1.23 ;evtl. Kommentar
        if ( *p == '"' )
        {
            //! keine '"' in Unit
            const sal_Unicode* const p1 = ++p;
            while ( *p != '"' )
                p++;
            String aFromUnit( aLine.Copy( p1 - pBeg, p - p1 ) );
            p = SkipWhite( ++p );
            if ( *p == ';' )
            {
                p = SkipWhite( ++p );
                if ( *p == '"' )
                {
                    //! keine '"' in Unit
                    const sal_Unicode* const p1 = ++p;
                    while ( *p != '"' )
                        p++;
                    String aToUnit( aLine.Copy( p1 - pBeg, p - p1 ) );
                    p = SkipWhite( ++p );
                    if ( *p == ';' )
                    {
                        p = SkipWhite( ++p );
                        int nErrno;
                        const sal_Unicode* pEnd;
                        double fValue = SolarMath::StringToDouble(
                            p, rIntl, nErrno, &pEnd );
                        if ( !nErrno && pEnd > p )
                        {
                            ScUnitConverterData* pData =
                                new ScUnitConverterData( aFromUnit, aToUnit,
                                fValue );
                            if ( !Insert( pData ) )
                                delete pData;
                        }
                    }
                }
            }
        }
        else if ( *p == '[' && ScGlobal::UnicodeStrChr( p+1, ']' ) )
            bIsConv = FALSE;        // neue [Section]
    }
}


BOOL ScUnitConverter::GetValue( double& fValue, const String& rFromUnit,
                const String& rToUnit ) const
{
    ScUnitConverterData aSearch( rFromUnit, rToUnit );
    USHORT nIndex;
    if ( Search( &aSearch, nIndex ) )
    {
        fValue = ((const ScUnitConverterData*)(At( nIndex )))->GetValue();
        return TRUE;
    }
    fValue = 1.0;
    return FALSE;
}


