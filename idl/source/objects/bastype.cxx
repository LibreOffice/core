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
#include "precompiled_idl.hxx"

#include <tools/debug.hxx>

#include <limits.h>
#include <ctype.h>
#include <bastype.hxx>
#include <lex.hxx>
#include <globals.hxx>
#include <hash.hxx>
#include <database.hxx>

#ifdef IDL_COMPILER
/************************************************************************/
static sal_Bool ReadRangeSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm,
                            sal_uLong nMin, sal_uLong nMax, sal_uLong* pValue )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();
    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_False;
        if( rInStm.Read( '=' ) )
        {
            pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() )
            {
                sal_uLong n = pTok->GetNumber();
                if ( n >= nMin && n <= nMax )
                {
                    *pValue = n;
                    bOk = sal_True;
                }
            }
        }

        if( bOk )
            return sal_True;
    }

    rInStm.Seek( nTokPos );
    return sal_False;
}
#endif

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
sal_uInt32 SvUINT32::Read( SvStream & rStm )
{
    return SvPersistStream::ReadCompressed( rStm );
}

void SvUINT32::Write( SvStream & rStm, sal_uInt32 nVal )
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
    sal_uInt8 n = rb.nVal;
    if( rb.bSet )
        n |= 0x02;
    rStm << n;
    return rStm;
}
SvStream& operator >> (SvStream & rStm, SvBOOL & rb )
{
    sal_uInt8 n;
    rStm >> n;
    rb.nVal = (n & 0x01) ? sal_True : sal_False;
    rb.bSet = (n & 0x02) ? sal_True : sal_False;
    if( n & ~0x03 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "format error" );
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

        int n = r.GetMajorVersion() << 4;
        n |= r.GetMinorVersion();
        rStm << (sal_uInt8)n;
    }
    else
    {
        rStm << (sal_uInt8)0;
        rStm << r.GetMajorVersion();
        rStm << r.GetMinorVersion();
    }
    return rStm;
}

SvStream& operator >> (SvStream & rStm, SvVersion & r )
{
    sal_uInt8 n;
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
|*    SvBOOL::ReadSvIdl()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
sal_Bool SvBOOL::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
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
            *this = sal_True; //Defaultaktion ist auf sal_True setzen
        if( bOk )
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

/*************************************************************************
|*
|*    SvBOOL::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvBOOL::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm )
{
    if( nVal )
        rOutStm << pName->GetName().GetBuffer();
    else
        rOutStm << pName->GetName().GetBuffer() << "(FALSE)";
    return sal_True;
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
sal_Bool SvIdentifier::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
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
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

/*************************************************************************
|*
|*    SvIdentifier::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvIdentifier::WriteSvIdl( SvStringHashEntry * pName,
                               SvStream & rOutStm,
                               sal_uInt16 /*nTab */ )
{
    rOutStm << pName->GetName().GetBuffer() << '(';
    rOutStm << GetBuffer() << ')';
    return sal_True;
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
sal_Bool SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvStringHashEntry * pName,
                                    SvTokenStream & rInStm )
{
    if( SvIdentifier::ReadSvIdl( pName, rInStm ) )
    {
        sal_uLong n;
        if( rBase.FindId( *this, &n ) )
        {
            nValue = n;
            return sal_True;
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
    return sal_False;
}

/*************************************************************************
|*
|*    SvNumberIdentifier::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvNumberIdentifier::ReadSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
    {
        sal_uLong n;
        if( rBase.FindId( pTok->GetString(), &n ) )
        {
            *(ByteString *)this = pTok->GetString();
            nValue = n;
            return sal_True;
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
    return sal_False;
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
sal_Bool SvString::ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( pName ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
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
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
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
sal_Bool SvString::WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                           sal_uInt16 /*nTab */ )
{
    rOutStm << pName->GetName().GetBuffer() << "(\"";
    rOutStm << GetBuffer() << "\")";
    return sal_True;
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
sal_Bool SvHelpText::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
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
sal_Bool SvHelpText::WriteSvIdl( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16 nTab )
{
    return SvString::WriteSvIdl( SvHash_HelpText(), rOutStm, nTab );
}

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
sal_Bool SvUUId::ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_uuid() ) )
    {
        sal_Bool bOk = sal_True;
        sal_Bool bBraket = rInStm.Read( '(' );
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
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
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
sal_Bool SvUUId::WriteSvIdl( SvStream & rOutStm )
{
    // Global Id schreiben
    rOutStm << SvHash_uuid()->GetName().GetBuffer() << "(\"";
    rOutStm << ByteString( GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << "\")";
    return sal_True;
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
sal_Bool SvVersion::ReadSvIdl( SvTokenStream & rInStm )
{
    sal_uLong n = 0;

    sal_uInt32 nTokPos = rInStm.Tell();
    if( ReadRangeSvIdl( SvHash_Version(), rInStm, 0 , 0xFFFF, &n ) )
    {
        nMajorVersion = (sal_uInt16)n;
        if( rInStm.Read( '.' ) )
        {
            SvToken * pTok = rInStm.GetToken_Next();
            if( pTok->IsInteger() && pTok->GetNumber() <= 0xFFFF )
            {
                nMinorVersion = (sal_uInt16)pTok->GetNumber();
                return sal_True;
            }
        }
        else
            return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
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
sal_Bool SvVersion::WriteSvIdl( SvStream & rOutStm )
{
    rOutStm << SvHash_Version()->GetName().GetBuffer() << '('
            << ByteString::CreateFromInt32( nMajorVersion ).GetBuffer() << '.'
            << ByteString::CreateFromInt32( nMinorVersion ).GetBuffer() << ')';
    return sal_True;
}
#endif //IDL_COMPILER


