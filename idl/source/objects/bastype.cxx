/*************************************************************************
 *
 *  $RCSfile: bastype.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#include <tools/debug.hxx>

#include <limits.h>
#include <ctype.h>
#include <bastype.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <hash.hxx>
#include <database.hxx>
#pragma hdrstop

/************************************************************************/
ByteString MakeDefineName( const ByteString & rName )
{
    ByteString aDefName;
    if( rName.Len() )
    {
        char * pStr = (char *)rName.GetBuffer();
        aDefName = *pStr;
        while( *++pStr )
        {
            if( isupper( *pStr ) )
                aDefName += '_';
            aDefName += (char)toupper( *pStr );
        }
    }
    return aDefName;
}

#ifdef IDL_COMPILER
/************************************************************************/
static BOOL ReadRangeSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm,
                            long nMin, ULONG nMax, long * pValue )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        BOOL bOk = TRUE;
        BOOL bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            short nSign = 1;
            pTok = rInStm.GetToken_Next();
            if( pTok->IsChar() && pTok->GetChar() == '-' )
            {
                nSign = -1;
                pTok = rInStm.GetToken_Next();
            }
            else if( pTok->IsChar() && pTok->GetChar() == '+' )
                pTok = rInStm.GetToken_Next();

            if( pTok->IsInteger() )
            {
                ULONG n = pTok->GetNumber();
                if( nSign == -1 || n <= nMax )
                { // nich tueber dem Maximum
                    if( (nSign == -1 && n < 0x8000000
                         && -(long)n >= nMin)
                      || (nSign == 1 && n >= 0x8000000 || (long)n > nMin) )
                    {
                        *pValue = (long)n;
                    }
                    else
                        bOk = FALSE;
                }
                if( bOk && bBraket )
                    bOk = rInStm.Read( ')' );
            }
            else
                bOk = pTok->IsChar() && pTok->GetChar() == ')';
        }
        if( bOk )
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}
#endif

/************************************************************************/
/*************************************************************************
|*
|*    RangeUniqueIdContainer::RangeUniqueIdContainer()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
RangeUniqueIdContainer::RangeUniqueIdContainer( const Range & rRange )
    : UniqueIdContainer( rRange.Min() ),
        nMax( rRange.Max() ), nMin( rRange.Min() )
{
}

/*************************************************************************
|*
|*    RangeUniqueIdContainer::RangeUniqueIdContainer()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
RangeUniqueIdContainer::RangeUniqueIdContainer()
    : UniqueIdContainer( 0 ),
        nMax( 0 ), nMin( 0 )
{
}

RangeUniqueIdContainer::~RangeUniqueIdContainer()
{
}

/*************************************************************************
|*
|*    RangeUniqueIdContainer:: opearator =()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
RangeUniqueIdContainer & RangeUniqueIdContainer::operator =
                     ( const RangeUniqueIdContainer & rObj )
{
    UniqueIdContainer::operator = ( rObj );
    nMin = rObj.nMin;
    nMax = rObj.nMax;
    return *this;
}

/*************************************************************************
|*
|*    RangeUniqueIdContainer::CreateId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL RangeUniqueIdContainer::CreateId( UniqueItemId * pId )
{
    {
        // wegen Scope von aId
        UniqueItemId aId = UniqueIdContainer::CreateId();
        if( aId.GetId() <= nMax )
        {
            *pId = aId;
            return TRUE;
        }
    }
    Clear( FALSE ); // nicht gebrauchte freigeben
    // noch einmal versuchen
    {
        // wegen Scope von aId
        UniqueItemId aId = UniqueIdContainer::CreateId();
        if( aId.GetId() <= nMax )
        {
            *pId = aId;
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*    RangeUniqueIdContainer::CreateId()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL RangeUniqueIdContainer::CreateId( UINT32 nIdValue, UniqueItemId * pId )
{
    if( nIdValue >= nMin && nIdValue <= nMax )
    {
        if( IsIndexValid( nIdValue ) )
            return FALSE;
        *pId = CreateIdProt( nIdValue );
        return TRUE;
    }
    *pId = UniqueIdContainer::CreateFreeId( nIdValue );
    return TRUE;
}

/*************************************************************************
|*
|*    SvUINT32::Read()
|*    SvUINT32::Write()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
UINT32 SvUINT32::Read( SvStream & rStm )
{
    return SvPersistStream::ReadCompressed( rStm );
}

void SvUINT32::Write( SvStream & rStm, UINT32 nVal )
{
    SvPersistStream::WriteCompressed( rStm, nVal );
}

/*************************************************************************
|*
|*    SvStream& operator << ( SvBOOL )
|*    SvStream& operator >> ( SvBOOL )
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
SvStream& operator << (SvStream & rStm, const SvBOOL & rb )
{
    BYTE n = rb.nVal;
    if( rb.bSet )
        n |= 0x02;
    rStm << n;
    return rStm;
}
SvStream& operator >> (SvStream & rStm, SvBOOL & rb )
{
    BYTE n;
    rStm >> n;
    rb.nVal = (n & 0x01) ? TRUE : FALSE;
    rb.bSet = (n & 0x02) ? TRUE : FALSE;
    if( n & ~0x03 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "format error" )
    }
    return rStm;
}

/*************************************************************************
|*
|*    SvStream& operator << ( SvVersion )
|*    SvStream& operator >> ( SvVersion )
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
SvStream& operator << (SvStream & rStm, const SvVersion & r )
{
    if( (r.GetMajorVersion() || r.GetMinorVersion())
      && r.GetMajorVersion() <= 0x0F && r.GetMinorVersion() <= 0x0F )
    { // Versionsnummer in 1 Byte komprimieren
        // Format 4 Bit fuer Major, dann 4 Bit fuer Minor.
        // 0.0 wird nicht komprimiert

        BYTE n = r.GetMajorVersion() << 4;
        n |= r.GetMinorVersion();
        rStm << n;
    }
    else
    {
        rStm << (BYTE)0;
        rStm << r.GetMajorVersion();
        rStm << r.GetMinorVersion();
    }
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvVersion & r )
{
    BYTE n;
    rStm >> n;
    if( n == 0 )
    { // nicht komprimiert
        rStm >> r.nMajorVersion;
        rStm >> r.nMinorVersion;
    }
    else
    { // komprimiert
        r.nMajorVersion = (n >> 4) & 0x0F;
        r.nMinorVersion = n & 0x0F;
    }
    return rStm;
}


#ifdef IDL_COMPILER
/************************************************************************/
/*************************************************************************
|*
|*    SvINT16::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvINT16::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    long n;

    if( ReadRangeSvIdl( pName, rInStm, -(long)0x8000, 0x7FFF, &n ) )
    {
        nVal = (short)n;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvINT16::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvINT16::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    rOutStm << pName->GetName().GetBuffer() << "(\""
            << ByteString::CreateFromInt32( nVal ).GetBuffer()  << "\")";
    return TRUE;
}


/*************************************************************************
|*
|*    SvUINT16::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUINT16::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    long n;

    if( ReadRangeSvIdl( pName, rInStm, 0, 0xFFFF, &n ) )
    {
        nVal = (UINT16)n;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvUINT16::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUINT16::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    rOutStm << pName->GetName().GetBuffer() << "(\""
            << ByteString::CreateFromInt32( nVal ).GetBuffer()  << "\")";
    return TRUE;
}


/*************************************************************************
|*
|*    SvINT32::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvINT32::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    long n;

    if( ReadRangeSvIdl( pName, rInStm, (-2147483647L-1L), 0x7FFFFFFF, &n ) )
    {
        nVal = (INT32)n;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvINT32::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvINT32::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    rOutStm << pName->GetName().GetBuffer() << "(\""
            << ByteString::CreateFromInt32( nVal ).GetBuffer()  << "\")";
    return TRUE;
}


/*************************************************************************
|*
|*    SvUINT32::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUINT32::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    long n;

    if( ReadRangeSvIdl( pName, rInStm, 0, 0xFFFFFFFF, &n ) )
    {
        nVal = (UINT32)n;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvUINT32::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUINT32::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    rOutStm << pName->GetName().GetBuffer() << "(\""
            << ByteString::CreateFromInt32( nVal ).GetBuffer()  << "\")";
    return TRUE;
}


/************************************************************************/
/*************************************************************************
|*
|*    Svint::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL Svint::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    long n;

    if( ReadRangeSvIdl( pName, rInStm, INT_MIN, INT_MAX, &n ) )
    {
        nVal = (int)n;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    Svint::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL Svint::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    rOutStm << pName->GetName().GetBuffer() << "(\""
            << ByteString::CreateFromInt32( nVal ).GetBuffer()  << "\")";
    return TRUE;
}


/*************************************************************************
|*
|*    SvBOOL::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvBOOL::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        BOOL bOk = TRUE;
        BOOL bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsBool() )
            {
                *this = pTok->GetBool();

                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        else
            *this = TRUE; //Defaultaktion ist auf TRUE setzen
        if( bOk )
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvBOOL::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvBOOL::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    if( nVal )
        rOutStm << pName->GetName().GetBuffer();
    else
        rOutStm << pName->GetName().GetBuffer() << "(FALSE)";
    return TRUE;
}

/*************************************************************************
|*
|*    SvBOOL::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
ByteString SvBOOL::GetSvIdlString( SvStringHashEntry * pName )
{
    if( nVal )
        return pName->GetName();
    else
    {
        ByteString aTmp( pName->GetName() );
        aTmp += "(FALSE)";
        return aTmp;
    }
}


/************************************************************************/
/*************************************************************************
|*
|*    SvIdentifier::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvIdentifier::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        BOOL bOk = TRUE;
        BOOL bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsIdentifier() )
            {
                *(ByteString *)this = pTok->GetString();
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvIdentifier::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvIdentifier::WriteSvIdl( SvStringHashEntry * pName,
                               SvStream & rOutStm,
                               USHORT /*nTab */ )
{
    rOutStm << pName->GetName().GetBuffer() << '(';
    rOutStm << GetBuffer() << ')';
    return TRUE;
}

SvStream& operator << (SvStream & rStm, const SvIdentifier & r )
{
    rStm.WriteByteString( r );
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvIdentifier & r )
{
    rStm.ReadByteString( r );
    return rStm;
}


/************************************************************************/
/*************************************************************************
|*
|*    SvNumberIdentifier::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvStringHashEntry * pName,
                                    SvTokenStream & rInStm )
{
    if( SvIdentifier::ReadSvIdl( pName, rInStm ) )
    {
        ULONG n;
        if( rBase.FindId( *this, &n ) )
        {
            nValue = n;
            return TRUE;
        }
        else
        {
            ByteString aStr ("no value for identifier <");
            aStr += *this;
            aStr += "> ";
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvNumberIdentifier::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
    {
        ULONG n;
        if( rBase.FindId( pTok->GetString(), &n ) )
        {
            *(ByteString *)this = pTok->GetString();
            nValue = n;
            return TRUE;
        }
        else
        {
            ByteString aStr ("no value for identifier <");
            aStr += *this;
            aStr += "> ";
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvStream& operator << ( SvNumberIdentifier )
|*    SvStream& operator >> ( SvNumberIdentifier )
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
SvStream& operator << (SvStream & rStm, const SvNumberIdentifier & r )
{
    rStm << (SvIdentifier &)r;
    SvPersistStream::WriteCompressed( rStm, r.nValue );
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvNumberIdentifier & r )
{
    rStm >> (SvIdentifier &)r;
    r.nValue = SvPersistStream::ReadCompressed( rStm );
    return rStm;
}


/************************************************************************/
/*************************************************************************
|*
|*    SvString::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvString::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        BOOL bOk = TRUE;
        BOOL bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                *(ByteString *)this = pTok->GetString();
                rInStm.GetToken_Next();
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvString::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvString::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                           USHORT /*nTab */ )
{
    rOutStm << pName->GetName().GetBuffer() << "(\"";
    rOutStm << GetBuffer() << "\")";
    return TRUE;
}

SvStream& operator << (SvStream & rStm, const SvString & r )
{
    rStm.WriteByteString( r );
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvString & r )
{
    rStm.ReadByteString( r );
    return rStm;
}


/*************************************************************************
|*
|*    SvHelpText::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvHelpText::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    return SvString::ReadSvIdl( SvHash_HelpText(), rInStm );
}

/*************************************************************************
|*
|*    SvHelpText::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvHelpText::WriteSvIdl( SvIdlDataBase &, SvStream & rOutStm, USHORT nTab )
{
    return SvString::WriteSvIdl( SvHash_HelpText(), rOutStm, nTab );
}

#ifdef sdvnmsd
/*************************************************************************
|*
|*    SvHelpContext::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvHelpContext::ReadSvIdl( RangeUniqueIdContainer & rCont,
                               SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    long n = 0;

    if( ReadRangeSvIdl( SvHash_HelpContext(), rInStm, 0, 0xFFFFFFFF, &n ) )
    {
        if( n == 0 )
        {
            //if( rCont.CreateId( &aId ) )
            // erst beim Schreiben anlegen
            bSet = TRUE;
            return TRUE;
        }
        else
        {
            if( (UINT32)n <= rCont.GetMax() && (UINT32)n >= rCont.GetMin() )
            {
                aId = UniqueIdContainer::CreateFreeId( (UINT32)n );
                bSet = TRUE;
                return TRUE;
            }
            else if( rCont.CreateId( (UINT32)n, &aId ) )
            {
                bSet = TRUE;
                return TRUE;
            }
        }
    }

    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvHelpContext::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvHelpContext::WriteSvIdl( RangeUniqueIdContainer & rCont,
                                SvStream & rOutStm, USHORT )
{
    if( IsSet() && GetId() == 0 )
        if( !rCont.CreateId( &aId ) )
            return FALSE;
    rOutStm << (const char *)SvHash_HelpContext()->GetName() << '('
            << (const char *)ByteString( GetId() )  << ')';
    return TRUE;
}
#endif

/************************************************************************/
/*************************************************************************
|*
|*    SvUUId::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUUId::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_uuid() ) )
    {
        BOOL bOk = TRUE;
        BOOL bBraket = rInStm.Read( '(' );
        if( bBraket || rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                pTok = rInStm.GetToken_Next();
                bOk = MakeId( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
            }
            if( bOk && bBraket )
                bOk = rInStm.Read( ')' );
        }
        if( bOk )
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvMetaObject::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvUUId::WriteSvIdl( SvStream & rOutStm )
{
    // Global Id schreiben
    rOutStm << SvHash_uuid()->GetName().GetBuffer() << "(\"";
    rOutStm << ByteString( GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << "\")";
    return TRUE;
}


/************************************************************************/
/*************************************************************************
|*
|*    SvVersion::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvVersion::ReadSvIdl( SvTokenStream & rInStm )
{
    long n;

    UINT32 nTokPos = rInStm.Tell();
    if( ReadRangeSvIdl( SvHash_Version(), rInStm, 0 , 0xFFFF, &n ) )
    {
        nMajorVersion = (USHORT)n;
        if( rInStm.Read( '.' ) )
        {
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() && pTok->GetNumber() <= 0xFFFF )
            {
                nMinorVersion = (USHORT)pTok->GetNumber();
                return TRUE;
            }
        }
        else
            return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

/*************************************************************************
|*
|*    SvVersion::WriteSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvVersion::WriteSvIdl( SvStream & rOutStm )
{
    rOutStm << SvHash_Version()->GetName().GetBuffer() << '('
            << ByteString::CreateFromInt32( nMajorVersion ).GetBuffer() << '.'
            << ByteString::CreateFromInt32( nMinorVersion ).GetBuffer() << ')';
    return TRUE;
}
#endif //IDL_COMPILER


