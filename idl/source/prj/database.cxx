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

#include <sal/config.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string_view>

#include <database.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>


SvParseException::SvParseException( SvTokenStream const & rInStm, const OString& rError )
{
    SvToken& rTok = rInStm.GetToken();
    aError = SvIdlError( rTok.GetLine(), rTok.GetColumn() );
    aError.SetText( rError );
};

SvParseException::SvParseException( const OString& rError, SvToken const & rTok )
{
    aError = SvIdlError( rTok.GetLine(), rTok.GetColumn() );
    aError.SetText( rError );
};


SvIdlDataBase::SvIdlDataBase( const SvCommand& rCmd )
    : bExport( false )
    , nUniqueId( 0 )
    , nVerbosity( rCmd.nVerbosity )
{
    sSlotMapFile = rCmd.aSlotMapFile;
}

SvIdlDataBase::~SvIdlDataBase()
{
    aIdFileList.clear();
}

#define ADD_TYPE( Name )            \
    aTypeList.push_back( new SvMetaType( SvHash_##Name()->GetName() ) );

SvRefMemberList<SvMetaType *>& SvIdlDataBase::GetTypeList()
{
    if( aTypeList.empty() )
    { // fill initially
        aTypeList.push_back( new SvMetaTypeString() );
        aTypeList.push_back( new SvMetaTypevoid() );

        // MI: IDispatch::Invoke can not unsigned
        ADD_TYPE( UINT16 );
        ADD_TYPE( INT16 );
        ADD_TYPE( UINT32 );
        ADD_TYPE( INT32 );
        ADD_TYPE( BOOL );
        ADD_TYPE( BYTE );
        ADD_TYPE( float );
        ADD_TYPE( double );
        ADD_TYPE( SbxObject );

        // Attention! When adding types all binary data bases get incompatible

    }
    return aTypeList;
}

void SvIdlDataBase::SetError( const OString& rError, SvToken const & rTok )
{
    if( rTok.GetLine() > 10000 )
        aError.SetText( "line count overflow" );

    if( aError.nLine < rTok.GetLine()
      || (aError.nLine == rTok.GetLine() && aError.nColumn < rTok.GetColumn()) )
    {
        aError = SvIdlError( rTok.GetLine(), rTok.GetColumn() );
        aError.SetText( rError );
    }
}

void SvIdlDataBase::SetAndWriteError( SvTokenStream & rInStm, const OString& rError )
{
    SetError( rError, rInStm.GetToken() );
    WriteError( rInStm );
}

void SvIdlDataBase::Push( SvMetaObject * pObj )
{
    GetStack().push_back( pObj );
}

bool SvIdlDataBase::FindId( const OString& rIdName, sal_uLong * pVal )
{
    if( pIdTable )
    {
        sal_uInt32 nHash;
        if( pIdTable->Test( rIdName, &nHash ) )
        {
            *pVal = pIdTable->Get( nHash )->GetValue();
            return true;
        }
    }
    return false;
}

void SvIdlDataBase::InsertId( const OString& rIdName, sal_uLong nVal )
{
    if( !pIdTable )
        pIdTable.reset( new SvStringHashTable );

    sal_uInt32 nHash;
    pIdTable->Insert( rIdName, &nHash )->SetValue( nVal );
}

bool SvIdlDataBase::ReadIdFile( std::string_view rOFileName )
{
    OUString rFileName = OStringToOUString(rOFileName, RTL_TEXTENCODING_ASCII_US);
    OUString aFullName;
    osl::File::searchFileURL( rFileName, GetPath(), aFullName);
    osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );

    for ( size_t i = 0, n = aIdFileList.size(); i < n; ++i )
        if ( aIdFileList[ i ] == rFileName )
            return true;

    aIdFileList.push_back( rFileName );
    AddDepFile( aFullName );
    SvTokenStream aTokStm( aFullName );
    if( aTokStm.GetStream().GetError() != ERRCODE_NONE )
        return false;

    SvToken& rTok = aTokStm.GetToken_Next();

    while( !rTok.IsEof() )
    {
        if( rTok.IsChar() && rTok.GetChar() == '#' )
        {
            rTok = aTokStm.GetToken_Next();
            if( rTok.Is( SvHash_define() ) )
            {
                rTok = aTokStm.GetToken_Next();
                OString aDefName;
                if( !rTok.IsIdentifier() )
                    throw SvParseException( "unexpected token after define", rTok );
                aDefName = rTok.GetString();

                sal_uLong nVal = 0;
                bool bOk = true;
                while( bOk )
                {
                    rTok = aTokStm.GetToken_Next();
                    if (rTok.GetTokenAsString().startsWith("TypedWhichId"))
                    {
                        rTok = aTokStm.GetToken_Next();
                        if( !rTok.IsChar() || rTok.GetChar() != '<')
                            throw SvParseException( "expected '<'", rTok );
                        rTok = aTokStm.GetToken_Next();
                        if( !rTok.IsIdentifier() )
                            throw SvParseException( "expected identifier", rTok );
                        rTok = aTokStm.GetToken_Next();
                        if( !rTok.IsChar() || rTok.GetChar() != '>')
                            throw SvParseException( "expected '<'", rTok );
                        rTok = aTokStm.GetToken_Next();
                    }
                    else if( rTok.IsIdentifier() )
                    {
                        sal_uLong n;
                        if( FindId( rTok.GetString(), &n ) )
                            nVal += n;
                        else
                            bOk = false;
                    }
                    else if( rTok.IsChar() )
                    {
                        if( rTok.GetChar() == '-'
                          || rTok.GetChar() == '/'
                          || rTok.GetChar() == '*'
                          || rTok.GetChar() == '&'
                          || rTok.GetChar() == '|'
                          || rTok.GetChar() == '^'
                          || rTok.GetChar() == '~' )
                        {
                            throw SvParseException( "unknown operator '" + OStringChar(rTok.GetChar()) + "'in define", rTok );
                        }
                        if( rTok.GetChar() != '+'
                          && rTok.GetChar() != '('
                          && rTok.GetChar() != ')' )
                            // only + is allowed, parentheses are immaterial
                            // because + is commutative
                            break;
                    }
                    else if( rTok.IsInteger() )
                    {
                        nVal += rTok.GetNumber();
                    }
                    else
                        break;
                }
                if( bOk )
                {
                    InsertId( aDefName, nVal );
                }
            }
            else if( rTok.Is( SvHash_include() ) )
            {
                rTok = aTokStm.GetToken_Next();
                OStringBuffer aNameBuf(128);
                if( rTok.IsString() )
                    aNameBuf.append(rTok.GetString());
                else if( rTok.IsChar() && rTok.GetChar() == '<' )
                {
                    rTok = aTokStm.GetToken_Next();
                    while( !rTok.IsEof()
                      && !(rTok.IsChar() && rTok.GetChar() == '>') )
                    {
                        aNameBuf.append(rTok.GetTokenAsString());
                        rTok = aTokStm.GetToken_Next();
                    }
                    if( rTok.IsEof() )
                    {
                        throw SvParseException("unexpected eof in #include", rTok);
                    }
                }
                OString aName(aNameBuf.makeStringAndClear());
                if (aName == "sfx2/groupid.hxx")
                {
                    // contains C++ code which we cannot parse
                    // we special-case this by defining a macro internally in...
                }
                else if (aName == "svl/typedwhich.hxx")
                {
                    // contains C++ code which we cannot parse
                }
                else if (!ReadIdFile(aName))
                {
                    throw SvParseException("cannot read file: " + aName, rTok);
                }
            }
        }
        else
            rTok = aTokStm.GetToken_Next();
    }
    return true;
}

SvMetaType * SvIdlDataBase::FindType( const SvMetaType * pPType,
                                    SvRefMemberList<SvMetaType *>& rList )
{
    for (auto const& elem : rList)
        if( elem == pPType )
            return elem;
    return nullptr;
}

SvMetaType * SvIdlDataBase::FindType( std::string_view rName )
{
    for (auto const& elem : aTypeList)
        if( rName == elem->GetName() )
            return elem;
    return nullptr;
}

SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.IsIdentifier() )
    {
        const OString& aName = rTok.GetString();
        for( const auto& aType : GetTypeList() )
        {
            if( aType->GetName() == aName )
            {
                return aType;
            }
        }
    }
    rInStm.Seek( nTokPos );
    return nullptr;
}

SvMetaAttribute * SvIdlDataBase::ReadKnownAttr
(
    SvTokenStream & rInStm,
    SvMetaType *    pType   /* If pType == NULL, then the type has
                               still to be read. */
)
{
    sal_uInt32  nTokPos = rInStm.Tell();

    if( !pType )
        pType = ReadKnownType( rInStm );

    if( !pType )
    {
        // otherwise SlotId?
        SvToken& rTok = rInStm.GetToken_Next();
        if( rTok.IsIdentifier() )
        {
            sal_uLong n;
            if( FindId( rTok.GetString(), &n ) )
            {
                for( sal_uLong i = 0; i < aSlotList.size(); i++ )
                {
                    SvMetaSlot * pSlot = aSlotList[i];
                    if( pSlot->GetSlotId().getString() == rTok.GetString() )
                        return pSlot;
                }
            }

            OStringBuffer aStr("Not found : ");
            aStr.append(rTok.GetString());
            OSL_FAIL(aStr.getStr());
        }
    }

    rInStm.Seek( nTokPos );
    return nullptr;
}

SvMetaAttribute* SvIdlDataBase::FindKnownAttr( const SvIdentifier& rId )
{
    sal_uLong n;
    if( FindId( rId.getString(), &n ) )
    {
        for( sal_uLong i = 0; i < aSlotList.size(); i++ )
        {
            SvMetaSlot * pSlot = aSlotList[i];
            if( pSlot->GetSlotId().getString() == rId.getString() )
                return pSlot;
        }
    }

    return nullptr;
}

SvMetaClass * SvIdlDataBase::ReadKnownClass( SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken& rTok = rInStm.GetToken_Next();

    if( rTok.IsIdentifier() )
    {
        SvMetaClass* p = FindKnownClass(rTok.GetString());
        if (p)
            return p;
    }

    rInStm.Seek( nTokPos );
    return nullptr;
}

SvMetaClass * SvIdlDataBase::FindKnownClass( std::string_view aName )
{
    for( sal_uLong n = 0; n < aClassList.size(); n++ )
    {
        SvMetaClass * pClass = aClassList[n];
        if( pClass->GetName() == aName )
            return pClass;
    }
    return nullptr;
}

void SvIdlDataBase::Write(const OString& rText) const
{
    if( nVerbosity != 0 )
        fprintf( stdout, "%s", rText.getStr() );
}

void SvIdlDataBase::WriteError( SvTokenStream & rInStm )
{
    // error treatment
    OUString aFileName( rInStm.GetFileName() );
    OStringBuffer aErrorText;
    sal_uLong   nRow = 0, nColumn = 0;

    rInStm.SeekToMax();
    SvToken& rTok = rInStm.GetToken();

    // error position
    nRow    = rTok.GetLine();
    nColumn = rTok.GetColumn();

    if( aError.IsError() )
    { // error set
        // search error token
        // error text
        if( !aError.GetText().isEmpty() )
        {
            aErrorText.append("may be <");
            aErrorText.append(aError.GetText());
        }
        SvToken * pPrevTok = nullptr;
        while( &rTok != pPrevTok )
        {
            pPrevTok = &rTok;
            if( rTok.GetLine() == aError.nLine
              && rTok.GetColumn() == aError.nColumn )
                break;
            rTok = rInStm.GetToken_PrevAll();
        }

        // error position
        aErrorText.append("> at ( ");
        aErrorText.append(static_cast<sal_Int64>(aError.nLine));
        aErrorText.append(", ");
        aErrorText.append(static_cast<sal_Int64>(aError.nColumn));
        aErrorText.append(" )");

        // reset error
        aError = SvIdlError();
    }

    // error treatment
    fprintf( stderr, "\n%s --- %s: ( %" SAL_PRIuUINTPTR ", %" SAL_PRIuUINTPTR " )\n",
             OUStringToOString(aFileName, RTL_TEXTENCODING_UTF8).getStr(),
             "error", nRow, nColumn );

    if( !aErrorText.isEmpty() )
    { // error set
        fprintf( stderr, "\t%s\n", aErrorText.getStr() );
    }

    // look for identifier close by
    if( !rTok.IsIdentifier() )
    {
        rInStm.GetToken_PrevAll();
        rTok = rInStm.GetToken();
    }
    if( rTok.IsIdentifier() )
    {
        OString aN = GetIdlApp().pHashTable->GetNearString( rTok.GetString() );
        if( !aN.isEmpty() )
            fprintf( stderr, "%s versus %s\n", rTok.GetString().getStr(), aN.getStr() );
    }
}

SvIdlWorkingBase::SvIdlWorkingBase(const SvCommand& rCmd) : SvIdlDataBase(rCmd)
{
}


bool SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != ERRCODE_NONE )
        return false;

    // reset all tmp variables for writing
    WriteReset();
    SvMemoryStream aTmpStm( 256000, 256000 );
    sal_uLong n;
    for( n = 0; n < GetModuleList().size(); n++ )
    {
        SvMetaModule * pModule = GetModuleList()[n];
        pModule->WriteSfx( *this, aTmpStm );
        aTmpStm.Seek( 0 );
    }
    for( n = 0; n < aUsedTypes.size(); n++ )
    {
        SvMetaType * pType = aUsedTypes[n];
        pType->WriteSfx( *this, rOutStm );
    }
    aUsedTypes.clear();
    rOutStm.WriteStream( aTmpStm );
    return true;
}

void SvIdlDataBase::StartNewFile( std::u16string_view rName )
{
    bExport = aExportFile.equalsIgnoreAsciiCase( rName );
    assert ( !bExport );
}

void SvIdlDataBase::AppendSlot( SvMetaSlot *pSlot )
{
    aSlotList.push_back( pSlot );
    assert ( !bExport );
}

void SvIdlDataBase::AddDepFile(OUString const& rFileName)
{
    m_DepFiles.insert(rFileName);
}

namespace {

struct WriteDep
{
    SvFileStream & m_rStream;
    explicit WriteDep(SvFileStream & rStream) : m_rStream(rStream) { }
    void operator() (std::u16string_view rItem)
    {
        m_rStream.WriteCharPtr( " \\\n " );
        m_rStream.WriteOString( OUStringToOString(rItem, RTL_TEXTENCODING_UTF8) );
    }
};

// write a dummy target for one included file, so the incremental build does
// not break with "No rule to make target" if the included file is removed
struct WriteDummy
{
    SvFileStream & m_rStream;
    explicit WriteDummy(SvFileStream & rStream) : m_rStream(rStream) { }
    void operator() (std::u16string_view rItem)
    {
        m_rStream.WriteOString( OUStringToOString(rItem, RTL_TEXTENCODING_UTF8) );
        m_rStream.WriteCharPtr( " :\n\n" );
    }
};

}

void SvIdlDataBase::WriteDepFile(
        SvFileStream & rStream, std::u16string_view rTarget)
{
    rStream.WriteOString( OUStringToOString(rTarget, RTL_TEXTENCODING_UTF8) );
    rStream.WriteCharPtr( " :" );
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDep(rStream));
    rStream.WriteCharPtr( "\n\n" );
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDummy(rStream));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
