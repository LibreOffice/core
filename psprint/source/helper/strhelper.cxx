/*************************************************************************
 *
 *  $RCSfile: strhelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 09:58:39 $
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
#include <psprint/strhelper.hxx>
#if defined(SOLARIS) || defined(IRIX)
#include <ieeefp.h> // finite
#include <alloca.h>
#endif
#include <cstdlib>
#include <math.h> // for isnan
#include <cstring>

// isnan needs special support on MACOSX
#if defined(MACOSX)
#define      isnan( x )         ( ( sizeof ( x ) == sizeof(double) ) ?    \
                              __isnand ( x ) :                            \
                                ( sizeof ( x ) == sizeof( float) ) ?      \
                              __isnanf ( x ) :                            \
                              __isnan  ( x ) )
#endif

namespace psp {

inline int isSpace( char cChar )
{
    return
        cChar == ' '    || cChar == '\t'    ||
        cChar == '\r'   || cChar == '\n'    ||
        cChar == 0x0c   || cChar == 0x0b;
}

inline int isSpace( sal_Unicode cChar )
{
    return
        cChar == ' '    || cChar == '\t'    ||
        cChar == '\r'   || cChar == '\n'    ||
        cChar == 0x0c   || cChar == 0x0b;
}

inline int isProtect( char cChar )
{
    return cChar == '`' || cChar == '\'' || cChar == '"';
}

inline int isProtect( sal_Unicode cChar )
{
    return cChar == '`' || cChar == '\'' || cChar == '"';
}

inline void CopyUntil( char*& pTo, const char*& pFrom, char cUntil, int bIncludeUntil = 0 )
{
    do
    {
        if( *pFrom == '\\' )
        {
            pFrom++;
            if( *pFrom )
            {
                *pTo = *pFrom;
                pTo++;
            }
        }
        else if( bIncludeUntil || ! isProtect( *pFrom ) )
        {
            *pTo = *pFrom;
            pTo++;
        }
        pFrom++;
    } while( *pFrom && *pFrom != cUntil );
    // copy the terminating character unless zero or protector
    if( ! isProtect( *pFrom ) || bIncludeUntil )
    {
        *pTo = *pFrom;
        if( *pTo )
            pTo++;
    }
    if( *pFrom )
        pFrom++;
}

inline void CopyUntil( sal_Unicode*& pTo, const sal_Unicode*& pFrom, sal_Unicode cUntil, int bIncludeUntil = 0 )
{
    do
    {
        if( *pFrom == '\\' )
        {
            pFrom++;
            if( *pFrom )
            {
                *pTo = *pFrom;
                pTo++;
            }
        }
        else if( bIncludeUntil || ! isProtect( *pFrom ) )
        {
            *pTo = *pFrom;
            pTo++;
        }
        pFrom++;
    } while( *pFrom && *pFrom != cUntil );
    // copy the terminating character unless zero or protector
    if( ! isProtect( *pFrom ) || bIncludeUntil )
    {
        *pTo = *pFrom;
        if( *pTo )
            pTo++;
    }
    if( *pFrom )
        pFrom++;
}

String GetCommandLineToken( int nToken, const String& rLine )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return String();

    int nActualToken = 0;
    sal_Unicode* pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*( nLen + 1 ) );
    const sal_Unicode* pRun = rLine.GetBuffer();
    sal_Unicode* pLeap;

    while( *pRun && nActualToken <= nToken )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        pLeap = pBuffer;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`' );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'' );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"' );
            else
            {
                *pLeap = *pRun;
                pLeap++;
                pRun++;
            }
        }
        if( nActualToken != nToken )
            pBuffer[0] = 0;
        nActualToken++;
    }

    *pLeap = 0;

    String aRet( pBuffer );
    return aRet;
}

ByteString GetCommandLineToken( int nToken, const ByteString& rLine )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return ByteString();

    int nActualToken = 0;
    char* pBuffer = (char*)alloca( nLen + 1 );
    const char* pRun = rLine.GetBuffer();
    char* pLeap;

    while( *pRun && nActualToken <= nToken )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        pLeap = pBuffer;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
                CopyUntil( pLeap, pRun, '`' );
            else if( *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'' );
            else if( *pRun == '"' )
                CopyUntil( pLeap, pRun, '"' );
            else
            {
                *pLeap = *pRun;
                pLeap++;
                pRun++;
            }
        }
        if( nActualToken != nToken )
            pBuffer[0] = 0;
        nActualToken++;
    }

    *pLeap = 0;

    ByteString aRet( pBuffer );
    return aRet;
}

int GetCommandLineTokenCount( const String& rLine )
{
    if( ! rLine.Len() )
        return 0;

    int nTokenCount = 0;
    const sal_Unicode *pRun = rLine.GetBuffer();


    while( *pRun )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        if( ! *pRun )
            break;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
            {
                do pRun++; while( *pRun && *pRun != '`' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '\'' )
            {
                do pRun++; while( *pRun && *pRun != '\'' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '"' )
            {
                do pRun++; while( *pRun && *pRun != '"' );
                if( *pRun )
                    pRun++;
            }
            else
                pRun++;
        }
        nTokenCount++;
    }

    return nTokenCount;
}

int GetCommandLineTokenCount( const ByteString& rLine )
{
    if( ! rLine.Len() )
        return 0;

    int nTokenCount = 0;
    const char *pRun = rLine.GetBuffer();


    while( *pRun )
    {
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        if( ! *pRun )
            break;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '`' )
            {
                do pRun++; while( *pRun && *pRun != '`' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '\'' )
            {
                do pRun++; while( *pRun && *pRun != '\'' );
                if( *pRun )
                    pRun++;
            }
            else if( *pRun == '"' )
            {
                do pRun++; while( *pRun && *pRun != '"' );
                if( *pRun )
                    pRun++;
            }
            else
                pRun++;
        }
        nTokenCount++;
    }

    return nTokenCount;
}

String WhitespaceToSpace( const String& rLine, BOOL bProtect )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return String();

    sal_Unicode *pBuffer = (sal_Unicode*)alloca( sizeof(sal_Unicode)*(nLen + 1) );
    const sal_Unicode *pRun = rLine.GetBuffer();
    sal_Unicode *pLeap = pBuffer;

    while( *pRun )
    {
        if( *pRun && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( bProtect && *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', TRUE );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', TRUE );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', TRUE );
            else
            {
                *pLeap = *pRun;
                *pLeap++;
                *pRun++;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    String aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

ByteString WhitespaceToSpace( const ByteString& rLine, BOOL bProtect )
{
    int nLen = rLine.Len();
    if( ! nLen )
        return ByteString();

    char *pBuffer = (char*)alloca( nLen + 1 );
    const char *pRun = rLine.GetBuffer();
    char *pLeap = pBuffer;

    while( *pRun )
    {
        if( *pRun && isSpace( *pRun ) )
        {
            *pLeap = ' ';
            pLeap++;
            pRun++;
        }
        while( *pRun && isSpace( *pRun ) )
            pRun++;
        while( *pRun && ! isSpace( *pRun ) )
        {
            if( *pRun == '\\' )
            {
                // escapement
                pRun++;
                *pLeap = *pRun;
                pLeap++;
                if( *pRun )
                    pRun++;
            }
            else if( bProtect && *pRun == '`' )
                CopyUntil( pLeap, pRun, '`', TRUE );
            else if( bProtect && *pRun == '\'' )
                CopyUntil( pLeap, pRun, '\'', TRUE );
            else if( bProtect && *pRun == '"' )
                CopyUntil( pLeap, pRun, '"', TRUE );
            else
            {
                *pLeap = *pRun;
                *pLeap++;
                *pRun++;
            }
        }
    }

    *pLeap = 0;

    // there might be a space at beginning or end
    pLeap--;
    if( *pLeap == ' ' )
        *pLeap = 0;

    ByteString aRet( *pBuffer == ' ' ? pBuffer+1 : pBuffer );
    return aRet;
}

static double fExpValues[] =
{
    1.0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10,
    1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20
};

static double calcPow10( int nExp )
{
    BOOL bNeg = nExp < 0;
    nExp = bNeg ? -nExp : nExp;
    double fRet = 1.0;
    while( nExp >= 20 )
    {
        fRet *= fExpValues[20];
        nExp -= 20;
    }
    fRet *= fExpValues[ nExp ];
    return bNeg ? 1.0/fRet : fRet;
}

double StringToDouble( const String& rStr )
{
    const sal_Unicode* pStr = rStr.GetBuffer();
    const sal_Unicode* pOrg = pStr;
    const int nLen          = rStr.Len();

    BOOL bExp = FALSE, bNeg = FALSE, bNegExp = FALSE, bDecimal=FALSE;
    int nExp = 0;
    double fRet = 0.0, fDiv = 0.1;

    while( isSpace( *pStr ) )
        pStr++;
    for( ; pStr - pOrg < nLen; pStr++ )
    {
        if( *pStr >= '0' && *pStr <= '9' )
        {
            if( bExp )
                nExp = nExp*10 + ( *pStr - '0' );
            else if( ! bDecimal )
                fRet = 10.0 * fRet + (double)( *pStr - '0' );
            else
            {
                fRet += fDiv * (double)( *pStr - '0' );
                fDiv /= 10.0;
            }
        }
        else if( *pStr == '.' )
        {
            if( bExp || bDecimal )
                break;
            bDecimal = TRUE;
        }
        else if( *pStr == '-' )
        {
            if( bExp )
            {
                if( nExp != 0 )
                    break;
                bNegExp = ! bNegExp;
            }
            else
            {
                if( fRet != 0.0 )
                    break;
                bNeg = ! bNeg;
            }
        }
        else if( *pStr == '+' )
        {
            if( bExp && nExp != 0 )
                break;
            if( fRet != 0.0 )
                break;
        }
        else if( *pStr == 'e' || *pStr == 'E' )
        {
            if( bExp )
                break;
            bExp = TRUE;
        }
        else
            break;
    }
    if( bExp )
    {
        nExp = bNegExp ? -nExp : nExp;
        fRet *= calcPow10( nExp );
    }
    return bNeg ? -fRet : fRet;
}

double StringToDouble( const ByteString& rStr )
{
    const char* pStr    = rStr.GetBuffer();
    const char* pOrg    = pStr;
    const int nLen      = rStr.Len();

    BOOL bExp = FALSE, bNeg = FALSE, bNegExp = FALSE, bDecimal=FALSE;
    int nExp = 0;
    double fRet = 0.0, fDiv = 0.1;

    while( isSpace( *pStr ) )
        pStr++;
    for( ; pStr - pOrg < nLen; pStr++ )
    {
        if( *pStr >= '0' && *pStr <= '9' )
        {
            if( bExp )
                nExp = nExp*10 + ( *pStr - '0' );
            else if( ! bDecimal )
                fRet = 10.0 * fRet + (double)( *pStr - '0' );
            else
            {
                fRet += fDiv * (double)( *pStr - '0' );
                fDiv /= 10.0;
            }
        }
        else if( *pStr == '.' )
        {
            if( bExp || bDecimal )
                break;
            bDecimal = TRUE;
        }
        else if( *pStr == '-' )
        {
            if( bExp )
            {
                if( nExp != 0 )
                    break;
                bNegExp = ! bNegExp;
            }
            else
            {
                if( fRet != 0.0 )
                    break;
                bNeg = ! bNeg;
            }
        }
        else if( *pStr == '+' )
        {
            if( bExp && nExp != 0 )
                break;
            if( fRet != 0.0 )
                break;
        }
        else if( *pStr == 'e' || *pStr == 'E' )
        {
            if( bExp )
                break;
            bExp = TRUE;
        }
        else
            break;
    }
    if( bExp )
    {
        nExp = bNegExp ? -nExp : nExp;
        fRet *= calcPow10( nExp );
    }
    return bNeg ? -fRet : fRet;
}

int getValueOfDouble( char* pBuffer, double f, int nPrecision )
{
    char* pOrigin = pBuffer;

    if( ! finite( f ) )
    {
        strncpy( pBuffer, "Inf", 3 );
        return 3;
    }

    if( isnan( f ) )
    {
        strncpy( pBuffer, "NaN", 3 );
        return 3;
    }

    if( f == 0.0 )
    {
        strncpy( pBuffer, "0", 3 );
        return 1;
    }

    if( f < 0.0 )
    {
        *pBuffer++ = '-';
        f = -f;
    }
    int exponent = (int)log10( f );
    if( exponent < 4 && exponent > -4 )
        exponent = 0;
    f /= pow( 10, (double) exponent );

    int nInt = (int)f;
    f -= nInt;
    // f should really not be 1.0 after this, but some
    // hardware implementations seem to round at this point
    // this should take care of *.9999999999...
    if( f == 1.0 || log10( 1.0 - f ) <= -nPrecision )
    {
        nInt++;
        f = 0.0;
    }

    char pReverseBuffer[12];
    int nRev = 0;
    do
    {
        pReverseBuffer[ nRev++ ] = ( nInt % 10 ) + '0';
        nInt /= 10;
    } while( nInt );
    while( nRev-- )
        *pBuffer++ = pReverseBuffer[ nRev ];

    if( f || exponent )
    {
        *pBuffer++ = '.';
        do
        {
            f *= 10;
            nInt = (int)f;
            f -= nInt;
            // f should really not be 1.0 after this, but some
            // hardware implementations seem to round at this point
            // this should take care of *.*9999999...
            if( f == 1.0 || log10( 1.0 - f ) <= -nPrecision )
            {
                nInt++;
                f = 0.0;
            }
            *pBuffer++ = nInt + '0';
            nPrecision--;
        } while( f && nPrecision != 0);
    }
    if( exponent )
    {
        *pBuffer++ = 'e';
        if( exponent < 0 )
        {
            *pBuffer++ = '-';
            exponent = -exponent;
        }
        nRev = 0;
        do
        {
            pReverseBuffer[ nRev++ ] = (exponent % 10) + '0';
            exponent /= 10;
        } while( exponent );
        while( nRev-- )
            *pBuffer++ = pReverseBuffer[ nRev ];
    }
    *pBuffer++ = 0;

    return (pBuffer - pOrigin)-1;
}

ByteString DoubleToByteString( double f, int nPrecision )
{
    char buffer[128];
    getValueOfDouble( buffer, f, nPrecision );
    return ByteString( buffer );
}

String DoubleToString( double f, int nPrecision )
{
    char buffer[128];
    getValueOfDouble( buffer, f, nPrecision );
    return String::CreateFromAscii( buffer );
}

} // namespace
