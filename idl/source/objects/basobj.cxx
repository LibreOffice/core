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
#include "precompiled_idl.hxx"

#include <ctype.h>
#include <stdio.h>

#include <tools/debug.hxx>

#include <basobj.hxx>
#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaObject, SvRttiBase )
SvMetaObject::SvMetaObject()
{
}

void SvMetaObject::Load( SvPersistStream & )
{
}

void SvMetaObject::Save( SvPersistStream & )
{
}

#ifdef IDL_COMPILER
void SvMetaObject::WriteTab( SvStream & rOutStm, sal_uInt16 nTab )
{
    while( nTab-- )
        rOutStm << "    ";
}

void SvMetaObject::WriteStars( SvStream & rOutStm )
{
    rOutStm << '/';
    for( int i = 6; i > 0; i-- )
        rOutStm << "**********";
    rOutStm << '/' << endl;
}

sal_Bool SvMetaObject::TestAndSeekSpaceOnly( SvStream & rOutStm, sal_uLong nBegPos )
{
    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.Seek( nBegPos );
    sal_Bool bOnlySpace = sal_True;
    while( bOnlySpace && rOutStm.Tell() < nPos )
    {
        char c;
        rOutStm >> c;
        if( !isspace( c ) )
            bOnlySpace = sal_False;
    }
    if( bOnlySpace )
        // nothing written
        rOutStm.Seek( nBegPos );
    else
        rOutStm.Seek( nPos );
    return bOnlySpace;
}

void SvMetaObject::Back2Delemitter( SvStream & rOutStm )
{
    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.SeekRel( -1 );
    char c = 0;
    rOutStm >> c;

    while( isspace( c ) && rOutStm.Tell() != 1 )
    {
        rOutStm.SeekRel( -2 );
        rOutStm >> c;
    }

    if( c == ';' || c == ',' )
        rOutStm.SeekRel( -1 );
    else
        rOutStm.Seek( nPos );
}

sal_Bool SvMetaObject::ReadSvIdl( SvIdlDataBase &, SvTokenStream & )
{
    return sal_False;
}

void SvMetaObject::WriteSvIdl( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */ )
{
}

void SvMetaObject::Write( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */,
                             WriteType, WriteAttribute )
{
}

void SvMetaObject::WriteCxx( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */ )
{
}

void SvMetaObject::WriteHxx( SvIdlDataBase &, SvStream &, sal_uInt16 /*nTab */ )
{
}

#endif

SV_IMPL_META_FACTORY1( SvMetaName, SvMetaObject );
SvMetaName::SvMetaName()
{
}

void SvMetaName::Load( SvPersistStream & rStm )
{
    SvMetaObject::Load( rStm );
    sal_uInt8 nMask;
    rStm >> nMask;

    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 )  rStm >> aName;
    if( nMask & 0x02 )  rStm >> aHelpContext;
    if( nMask & 0x04 )  rStm >> aHelpText;
    if( nMask & 0x08 )  rStm >> aConfigName;
    if( nMask & 0x10 )  rStm >> aDescription;
}

void SvMetaName::Save( SvPersistStream & rStm )
{
    SvMetaObject::Save( rStm );
    sal_uInt8 nMask = 0;
    if( aName.IsSet() )         nMask |= 0x01;
    if( aHelpContext.IsSet() )  nMask |= 0x02;
    if( aHelpText.IsSet() )     nMask |= 0x04;
    if( aConfigName.IsSet() )   nMask |= 0x08;
    if( aDescription.IsSet() )  nMask |= 0x10;

    rStm << nMask;
    if( nMask & 0x01 ) rStm << aName;
    if( nMask & 0x02 ) rStm << aHelpContext;
    if( nMask & 0x04 ) rStm << aHelpText;
    if( nMask & 0x08 ) rStm << aConfigName;
    if( nMask & 0x10 ) rStm << aDescription;
}

sal_Bool SvMetaName::SetName( const ByteString & rName, SvIdlDataBase * )
{
    aName = rName;
    return sal_True;
}

#ifdef IDL_COMPILER
sal_Bool SvMetaName::ReadNameSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    // read module name
    if( pTok->IsIdentifier() )
        if( SetName( pTok->GetString(), &rBase ) )
            return sal_True;

    rInStm.Seek( nTokPos );
    return sal_False;
}

void SvMetaName::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    if( aName.ReadSvIdl( SvHash_Name(), rInStm ) )
    {
        if( !SetName( aName, &rBase ) )
            rInStm.Seek( nTokPos );
    }
    aHelpContext.ReadSvIdl( rBase, SvHash_HelpContext(), rInStm );
    aHelpText.ReadSvIdl( rBase, rInStm );
    aConfigName.ReadSvIdl( SvHash_ConfigName(), rInStm );
    aDescription.ReadSvIdl( SvHash_Description(), rInStm );
}

void SvMetaName::DoReadContextSvIdl( SvIdlDataBase & rBase,
                                   SvTokenStream & rInStm, char cDel )
{
    sal_uInt32 nBeginPos = 0; // can not happen with Tell
    while( nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        ReadContextSvIdl( rBase, rInStm );
        if( cDel == '\0' )
            rInStm.ReadDelemiter();
        else
            rInStm.Read( cDel );
    }
}

void SvMetaName::ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & )
{
}

sal_Bool SvMetaName::Test( SvIdlDataBase &, SvTokenStream & )
{
    return sal_True;
}

void SvMetaName::WriteContextSvIdl( SvIdlDataBase &, SvStream &, sal_uInt16 )
{
}

void SvMetaName::WriteDescription( SvStream & rOutStm )
{
    rOutStm << "<DESCRIPTION>" << endl;

    ByteString aDesc( GetDescription() );
    sal_uInt16 nPos = aDesc.Search( '\n' );
    while ( nPos != STRING_NOTFOUND )
    {
        rOutStm << aDesc.Copy( 0, nPos ).GetBuffer() << endl;
        aDesc.Erase(0,nPos+1);
        nPos = aDesc.Search( '\n' );
    }

    rOutStm << aDesc.GetBuffer() << endl << "</DESCRIPTION>" << endl;
}

void SvMetaName::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm,
                                       sal_uInt16 nTab )
{
    if( aHelpContext.IsSet() || aHelpText.IsSet() || aConfigName.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "// class SvMetaName" << endl;
    }
    if( aHelpContext.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aHelpContext.WriteSvIdl( SvHash_HelpContext(), rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    if( aHelpText.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aHelpText.WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    if( aConfigName.IsSet() )
    {
        WriteTab( rOutStm, nTab );
        aConfigName.WriteSvIdl( SvHash_ConfigName(), rOutStm, nTab );
        rOutStm << ';' << endl;
    }
}

sal_Bool SvMetaName::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();
    sal_Bool bOk = sal_True;
    if( rInStm.Read( '[' ) )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            ReadAttributesSvIdl( rBase, rInStm );
            rInStm.ReadDelemiter();
        }
        bOk = rInStm.Read( ']' );
    }

    if( bOk )
    {
        if( rInStm.Read( '{' ) )
        {
            DoReadContextSvIdl( rBase, rInStm );
            bOk = rInStm.Read( '}' );
        }
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaName::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    sal_uLong nBeginPos = rOutStm.Tell();
    WriteTab( rOutStm, nTab );
    rOutStm << '[' << endl;
    sal_uLong nOldPos = rOutStm.Tell();
    WriteAttributesSvIdl( rBase, rOutStm, nTab +1 );

    // write no empty brackets
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        WriteTab( rOutStm, nTab );
        rOutStm << ']';
        nBeginPos = rOutStm.Tell();
          rOutStm << endl;
    }

    WriteTab( rOutStm, nTab );
    rOutStm << '{' << endl;
    nOldPos = rOutStm.Tell();
    WriteContextSvIdl( rBase, rOutStm, nTab +1 );

    // write no empty brackets
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        WriteTab( rOutStm, nTab );
        rOutStm << '}';
    }
}

void SvMetaName::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                           sal_uInt16 nTab,
                         WriteType nT, WriteAttribute nA )
{
    sal_uLong nBeginPos = rOutStm.Tell();
    WriteTab( rOutStm, nTab );
    rOutStm << '[' << endl;
    sal_uLong nOldPos = rOutStm.Tell();
    WriteAttributes( rBase, rOutStm, nTab +1, nT, nA );

    // write no empty brackets
    sal_uLong nPos = rOutStm.Tell();
    rOutStm.Seek( nOldPos );
    sal_Bool bOnlySpace = sal_True;
    while( bOnlySpace && rOutStm.Tell() < nPos )
    {
        char c;
        rOutStm >> c;
        if( !isspace( c ) )
            bOnlySpace = sal_False;
    }
    if( bOnlySpace )
        // nothing written
        rOutStm.Seek( nBeginPos );
    else
    {
        rOutStm.Seek( nPos );
        WriteTab( rOutStm, nTab );
        rOutStm << ']' << endl;
    }
}

void SvMetaName::WriteAttributes( SvIdlDataBase &, SvStream & rOutStm,
                                sal_uInt16 nTab,
                                 WriteType, WriteAttribute )
{
    if( GetHelpText().IsSet() || GetHelpContext().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "// class SvMetaName" << endl;
    }
    if( GetHelpText().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "helpstring(\"" << GetHelpText().GetBuffer() << "\")," << endl;
    }
    if( GetHelpContext().IsSet() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "helpcontext("
                << ByteString::CreateFromInt64(
                    GetHelpContext().GetValue() ).GetBuffer()
                << ")," << endl;
    }
}

void SvMetaName::WriteContext( SvIdlDataBase &, SvStream &,
                                sal_uInt16,
                                 WriteType, WriteAttribute )
{
}
#endif // IDL_COMPILER

SV_IMPL_META_FACTORY1( SvMetaReference, SvMetaName );

SvMetaReference::SvMetaReference()
{
}

void SvMetaReference::Load( SvPersistStream & rStm )
{
    SvMetaName::Load( rStm );

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x2 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 )
    {
        SvMetaReference * pRef;
        rStm >> pRef;
        aRef = pRef;
    }
}

void SvMetaReference::Save( SvPersistStream & rStm )
{
    SvMetaName::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( aRef.Is() )
        nMask |= 0x01;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aRef;
}

SV_IMPL_META_FACTORY1( SvMetaExtern, SvMetaReference );

SvMetaExtern::SvMetaExtern()
    : pModule( NULL )
    , bReadUUId( sal_False )
    , bReadVersion( sal_False )
{
}

void SvMetaExtern::Load( SvPersistStream & rStm )
{
    SvMetaReference::Load( rStm );

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> pModule;
    if( nMask & 0x02 ) rStm >> aUUId;
    if( nMask & 0x04 ) rStm >> aVersion;
    if( nMask & 0x08 ) bReadUUId = sal_True;
    if( nMask & 0x10 ) bReadVersion = sal_True;
}

void SvMetaExtern::Save( SvPersistStream & rStm )
{
    SvMetaReference::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( pModule )                   nMask |= 0x01;
    if( aUUId != SvGlobalName() )   nMask |= 0x02;
    if( aVersion != SvVersion() )   nMask |= 0x04;
    if( bReadUUId )                 nMask |= 0x08;
    if( bReadVersion )              nMask |= 0x10;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << pModule;
    if( nMask & 0x02 ) rStm << aUUId;
    if( nMask & 0x04 ) rStm << aVersion;
}

SvMetaModule * SvMetaExtern::GetModule() const
{
    DBG_ASSERT( pModule != NULL, "module not set" );
    return pModule;
}

const SvGlobalName & SvMetaExtern::GetUUId() const
{
#ifdef IDL_COMPILER
    if( aUUId == SvGlobalName() )
        GetModule()->FillNextName( &((SvMetaExtern *)this)->aUUId );
#endif
    return aUUId;
}

#ifdef IDL_COMPILER
void SvMetaExtern::SetModule( SvIdlDataBase & rBase )
{
    pModule = (SvMetaModule *)rBase.GetStack().Get( TYPE( SvMetaModule ) );
}

void SvMetaExtern::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaReference::ReadAttributesSvIdl( rBase, rInStm );
    if( aUUId.ReadSvIdl( rBase, rInStm ) )
        bReadUUId = sal_True;
    if( aVersion.ReadSvIdl( rInStm ) )
        bReadVersion = sal_True;
}

void SvMetaExtern::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                         SvStream & rOutStm, sal_uInt16 nTab )
{
    SvMetaReference::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    if( bReadUUId || bReadVersion )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "// class SvMetaExtern" << endl;

        if( bReadUUId )
        {
            WriteTab( rOutStm, nTab );
            aUUId.WriteSvIdl( rOutStm );
            rOutStm << ';' << endl;
        }
        if( bReadVersion )
        {
            WriteTab( rOutStm, nTab );
            aVersion.WriteSvIdl( rOutStm );
            rOutStm << ';' << endl;
        }
    }
}

sal_Bool SvMetaExtern::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    SetModule( rBase );
    GetUUId(); // id gets created
    return SvMetaReference::ReadSvIdl( rBase, rInStm );
}

void SvMetaExtern::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               sal_uInt16 nTab )
{
    SvMetaReference::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaExtern::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                        sal_uInt16 nTab,
                         WriteType nT, WriteAttribute nA )
{
    SvMetaReference::Write( rBase, rOutStm, nTab, nT, nA );
}

void SvMetaExtern::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                     sal_uInt16 nTab,
                                     WriteType nT, WriteAttribute nA )
{
    SvMetaReference::WriteAttributes( rBase, rOutStm, nTab, nT, nA );

    WriteTab( rOutStm, nTab );
    rOutStm << "// class SvMetaExtern" << endl;
    WriteTab( rOutStm, nTab );
    rOutStm << "uuid(" << ByteString( GetUUId().GetHexName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << ")," << endl;
    WriteTab( rOutStm, nTab );
    rOutStm << "version(" << ByteString::CreateFromInt32( aVersion.GetMajorVersion() ).GetBuffer() << '.'
             << ByteString::CreateFromInt32( aVersion.GetMinorVersion() ).GetBuffer() << ")," << endl;
}

#endif // IDL_COMPILER


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
