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


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <tools/debug.hxx>
#include <database.hxx>
#include <globals.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

SvIdlDataBase::SvIdlDataBase( const SvCommand& rCmd )
    : bExport( false )
    , nUniqueId( 0 )
    , nVerbosity( rCmd.nVerbosity )
    , pIdTable( nullptr )
{
    sSlotMapFile = rCmd.aSlotMapFile;
}

SvIdlDataBase::~SvIdlDataBase()
{
    aIdFileList.clear();

    delete pIdTable;
}

#define ADD_TYPE( Name, OdlName, ParserChar, CName, BasName, BasPost )            \
    aTypeList.push_back( new SvMetaType( SvHash_##Name()->GetName(),   \
                     BasName, OdlName, ParserChar, CName, BasName, BasPost ) );

SvMetaTypeMemberList & SvIdlDataBase::GetTypeList()
{
    if( aTypeList.empty() )
    { // fill initially
        aTypeList.push_back( new SvMetaTypeString() );
        aTypeList.push_back( new SvMetaTypevoid() );

        // MI: IDispatch::Invoke can not unsigned
        ADD_TYPE( UINT16,    "long", 'h', "unsigned short", "Long", "&" );
        ADD_TYPE( INT16,     "short", 'h', "short", "Integer", "%" );
        ADD_TYPE( UINT32,    "long", 'l', "unsigned long", "Long", "&" );
        ADD_TYPE( INT32,     "long", 'l', "long", "Long", "&" );
        ADD_TYPE( int,       "int", 'i', "int", "Integer", "%" );
        ADD_TYPE( BOOL,      "boolean", 'b', "unsigned char", "Boolean", "" );
        ADD_TYPE( char,      "char", 'c', "char", "Integer", "%" );
        ADD_TYPE( BYTE,      "char", 'c', "unsigned char", "Integer", "%" );
        ADD_TYPE( float,     "float", 'f', "float", "Single", "!" );
        ADD_TYPE( double,    "double", 'F', "double", "Double", "#" );
        ADD_TYPE( SbxObject, "VARIANT", 'o', "C_Object", "Object", "" );

        // Attention! When adding types all binary data bases get incompatible

    }
    return aTypeList;
}

SvMetaModule * SvIdlDataBase::GetModule( const OString& rName )
{
    for( sal_uLong n = 0; n < aModuleList.size(); n++ )
        if( aModuleList[n]->GetName().getString().equals(rName) )
            return aModuleList[n];
    return nullptr;
}

void SvIdlDataBase::SetError( const OString& rError, SvToken& rTok )
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

void SvIdlDataBase::Push( SvMetaObject * pObj )
{
    GetStack().Push( pObj );
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

bool SvIdlDataBase::InsertId( const OString& rIdName, sal_uLong nVal )
{
    if( !pIdTable )
        pIdTable = new SvStringHashTable( 20003 );

    sal_uInt32 nHash;
    if( pIdTable->Insert( rIdName, &nHash ) )
    {
        pIdTable->Get( nHash )->SetValue( nVal );
        return true;
    }
    return false;
}

bool SvIdlDataBase::ReadIdFile( const OUString & rFileName )
{
    OUString aFullName;
    osl::File::searchFileURL( rFileName, GetPath(), aFullName);
    osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );

    for ( size_t i = 0, n = aIdFileList.size(); i < n; ++i )
        if ( aIdFileList[ i ] == rFileName )
            return true;

    aIdFileList.push_back( rFileName );
    this->AddDepFile( aFullName );
    SvTokenStream aTokStm( aFullName );
    if( aTokStm.GetStream().GetError() == SVSTREAM_OK )
    {
        SvToken * pTok = aTokStm.GetToken_Next();

        while( !pTok->IsEof() )
        {
            if( pTok->IsChar() && pTok->GetChar() == '#' )
            {
                pTok = aTokStm.GetToken_Next();
                if( pTok->Is( SvHash_define() ) )
                {
                    pTok = aTokStm.GetToken_Next();
                    OString aDefName;
                    if( pTok->IsIdentifier() )
                        aDefName = pTok->GetString();
                    else
                    {
                        OString aStr("unexpected token after define");
                        // set error
                        SetError( aStr, *pTok );
                        WriteError( aTokStm );
                        return false;
                    }

                    sal_uLong nVal = 0;
                    bool bOk = true;
                    while( bOk )
                    {
                        pTok = aTokStm.GetToken_Next();
                        if( pTok->IsIdentifier() )
                        {
                            sal_uLong n;
                            if( FindId( pTok->GetString(), &n ) )
                                nVal += n;
                            else
                                bOk = false;
                        }
                        else if( pTok->IsChar() )
                        {
                            if( pTok->GetChar() == '-'
                              || pTok->GetChar() == '/'
                              || pTok->GetChar() == '*'
                              || pTok->GetChar() == '&'
                              || pTok->GetChar() == '|'
                              || pTok->GetChar() == '^'
                              || pTok->GetChar() == '~' )
                            {
                                OStringBuffer aStr("unknown operator '");
                                aStr.append(pTok->GetChar());
                                aStr.append("'in define");
                                // set error
                                SetError( aStr.makeStringAndClear(), *pTok );
                                WriteError( aTokStm );
                                return false;
                            }
                            if( pTok->GetChar() != '+'
                              && pTok->GetChar() != '('
                              && pTok->GetChar() != ')' )
                                // only + is allowed, parentheses are immaterial
                                // because + is commutative
                                break;
                        }
                        else if( pTok->IsInteger() )
                        {
                            nVal += pTok->GetNumber();
                        }
                        else
                            break;
                    }
                    if( bOk )
                    {
                        if( !InsertId( aDefName, nVal ) )
                        {
                            OString aStr("hash table overflow: ");
                            SetError( aStr, *pTok );
                            WriteError( aTokStm );
                            return false;
                        }
                    }
                }
                else if( pTok->Is( SvHash_include() ) )
                {
                    pTok = aTokStm.GetToken_Next();
                    OStringBuffer aName;
                    if( pTok->IsString() )
                        aName.append(pTok->GetString());
                    else if( pTok->IsChar() && pTok->GetChar() == '<' )
                    {
                        pTok = aTokStm.GetToken_Next();
                        while( !pTok->IsEof()
                          && !(pTok->IsChar() && pTok->GetChar() == '>') )
                        {
                            aName.append(pTok->GetTokenAsString());
                            pTok = aTokStm.GetToken_Next();
                        }
                        if( pTok->IsEof() )
                        {
                            OString aStr("unexpected eof in #include");
                            // set error
                            SetError(aStr, *pTok);
                            WriteError( aTokStm );
                            return false;
                        }
                    }
                    if (!ReadIdFile(OStringToOUString(aName.toString(),
                        RTL_TEXTENCODING_ASCII_US)))
                    {
                        OStringBuffer aStr("cannot read file: ");
                        aStr.append(aName.makeStringAndClear());
                        SetError(aStr.makeStringAndClear(), *pTok);
                        WriteError( aTokStm );
                        return false;
                    }
                }
            }
            else
                pTok = aTokStm.GetToken_Next();
        }
    }
    else
        return false;
    return true;
}

SvMetaType * SvIdlDataBase::FindType( const SvMetaType * pPType,
                                    SvMetaTypeMemberList & rList )
{
    for( SvMetaTypeMemberList::const_iterator it = rList.begin(); it != rList.end(); ++it )
        if( *it == pPType )
            return *it;
    return nullptr;
}

SvMetaType * SvIdlDataBase::FindType( const OString& rName )
{
    for( SvMetaTypeMemberList::const_iterator it = aTypeList.begin(); it != aTypeList.end(); ++it )
        if( rName.equals((*it)->GetName().getString()) )
            return *it;
    return nullptr;
}

SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    bool bIn    = false;
    bool bOut   = false;
    int nCall0  = CALL_VALUE;
    int nCall1  = CALL_VALUE;
    bool bSet   = false; // any attribute set

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->HasHash() )
    {
        sal_uInt32 nBeginPos = 0; // can not happen with Tell
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            if( pTok->Is( SvHash_in() ) )
            {
                bIn  = true;
                pTok = rInStm.GetToken_Next();
                bSet = true;
            }
            if( pTok->Is( SvHash_out() ) )
            {
                bOut = true;
                pTok = rInStm.GetToken_Next();
                bSet = true;
            }
            if( pTok->Is( SvHash_inout() ) )
            {
                bIn  = true;
                bOut = true;
                pTok = rInStm.GetToken_Next();
                bSet = true;
            }
        }
    }

    if( pTok->IsIdentifier() )
    {
        OString aName = pTok->GetString();
        SvMetaTypeMemberList & rList = GetTypeList();
        SvMetaTypeMemberList::const_iterator it = rList.begin();
        SvMetaType * pType = nullptr;
        while( it != rList.end() )
        {
            if( (*it)->GetName().getString().equals(aName) )
            {
                pType = *it;
                break;
            }
            ++it;
        }
        if( pType )
        {
            pTok = &rInStm.GetToken();
            if( pTok->IsChar() )
            {
                if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                {
                    nCall0 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                        CALL_POINTER;
                    rInStm.GetToken_Next();
                    pTok = &rInStm.GetToken();
                    if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                    {
                        nCall1 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                            CALL_POINTER;
                        rInStm.GetToken_Next();
                    }
                    bSet = true;
                }
            }

            if( !bSet )
                // is exactly this type
                return pType;

            DBG_ASSERT( aTmpTypeList.front(), "mindestens ein Element" );
            SvMetaTypeRef xType = new SvMetaType( pType->GetName().getString(), 'h', "dummy" );
            xType->SetRef( pType );
            xType->SetIn( bIn );
            xType->SetOut( bOut );
            xType->SetCall0( nCall0 );
            xType->SetCall1( nCall1 );

            aTmpTypeList.push_back( xType );
            return xType;
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
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsIdentifier() )
        {
            sal_uLong n;
            if( FindId( pTok->GetString(), &n ) )
            {
                for( sal_uLong i = 0; i < aAttrList.size(); i++ )
                {
                    SvMetaAttribute * pAttr = aAttrList[i];
                    if( pAttr->GetSlotId().getString().equals(pTok->GetString()) )
                        return pAttr;
                }
            }

            OStringBuffer aStr("Nicht gefunden : ");
            aStr.append(pTok->GetString());
            OSL_FAIL(aStr.getStr());
        }
    }

    rInStm.Seek( nTokPos );
    return nullptr;
}

SvMetaAttribute* SvIdlDataBase::SearchKnownAttr
(
    const SvIdentifier& rId
)
{
    sal_uLong n;
    if( FindId( rId.getString(), &n ) )
    {
        for( sal_uLong i = 0; i < aAttrList.size(); i++ )
        {
            SvMetaAttribute * pAttr = aAttrList[i];
            if( pAttr->GetSlotId().getString() == rId.getString() )
                return pAttr;
        }
    }

    return nullptr;
}

SvMetaClass * SvIdlDataBase::ReadKnownClass( SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
        for( sal_uLong n = 0; n < aClassList.size(); n++ )
        {
            SvMetaClass * pClass = aClassList[n];
            if( pClass->GetName().getString().equals(pTok->GetString()) )
                return pClass;
        }

    rInStm.Seek( nTokPos );
    return nullptr;
}

void SvIdlDataBase::Write(const OString& rText)
{
    if( nVerbosity != 0 )
        fprintf( stdout, "%s", rText.getStr() );
}

void SvIdlDataBase::WriteError( const OString& rErrWrn,
                                const OString& rFileName,
                                const OString& rErrorText,
                                sal_uLong nRow, sal_uLong nColumn )
{
    // error treatment
    fprintf( stderr, "\n%s --- %s: ( %" SAL_PRIuUINTPTR ", %" SAL_PRIuUINTPTR " )\n",
             rFileName.getStr(), rErrWrn.getStr(), nRow, nColumn );

    if( !rErrorText.isEmpty() )
    { // error set
        fprintf( stderr, "\t%s\n", rErrorText.getStr() );
    }
}

void SvIdlDataBase::WriteError( SvTokenStream & rInStm )
{
    // error treatment
    OUString aFileName( rInStm.GetFileName() );
    OStringBuffer aErrorText;
    sal_uLong   nRow = 0, nColumn = 0;

    rInStm.SeekEnd();
    SvToken *pTok = &rInStm.GetToken();

    // error position
    nRow    = pTok->GetLine();
    nColumn = pTok->GetColumn();

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
        while( pTok != pPrevTok )
        {
            pPrevTok = pTok;
            if( pTok->GetLine() == aError.nLine
              && pTok->GetColumn() == aError.nColumn )
                break;
            pTok = rInStm.GetToken_PrevAll();
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

    WriteError("error", OUStringToOString(aFileName,
        RTL_TEXTENCODING_UTF8), aErrorText.makeStringAndClear(), nRow, nColumn);

    DBG_ASSERT( pTok, "token must be found" );
    if( !pTok )
        return;

    // look for identifier close by
    if( !pTok->IsIdentifier() )
    {
        rInStm.GetToken_PrevAll();
        pTok = &rInStm.GetToken();
    }
    if( pTok && pTok->IsIdentifier() )
    {
        OString aN = GetIdlApp().pHashTable->GetNearString( pTok->GetString() );
        if( !aN.isEmpty() )
            fprintf( stderr, "%s versus %s\n", pTok->GetString().getStr(), aN.getStr() );
    }
}

SvIdlWorkingBase::SvIdlWorkingBase(const SvCommand& rCmd) : SvIdlDataBase(rCmd)
{
}

bool SvIdlWorkingBase::ReadSvIdl( SvTokenStream & rInStm, bool bImported, const OUString & rPath )
{
    aPath = rPath; // only valid for this iteration
    bool bOk = true;
    SvToken * pTok = &rInStm.GetToken();
    // only one import at the very beginning
    if( pTok->Is( SvHash_import() ) )
    {
        rInStm.GetToken_Next();
        bOk = rInStm.Read( '(' ); // optional
        pTok = bOk ? rInStm.GetToken_Next() : nullptr;
        if( pTok && pTok->IsString() )
        {
            OUString aFullName;
            if( osl::FileBase::E_None == osl::File::searchFileURL(
                OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US),
                rPath,
                aFullName) )
            {
                osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );
                this->AddDepFile(aFullName);
                SvFileStream aStm( aFullName, STREAM_STD_READ | StreamMode::NOCREATE );
                SvTokenStream aTokStm( aStm, aFullName );
                bOk = ReadSvIdl( aTokStm, true, rPath );
            }
            else
                bOk = false;
        }
        else
            bOk = false;
    }

    sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell

    while( bOk && nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        pTok = &rInStm.GetToken();
        if( pTok->IsEof() )
            return true;
        if( pTok->IsEmpty() )
            bOk = false;

        // only one import at the very beginning
        if( pTok->Is( SvHash_module() ) )
        {
            tools::SvRef<SvMetaModule> aModule = new SvMetaModule( bImported );
            if( aModule->ReadSvIdl( *this, rInStm ) )
                GetModuleList().push_back( aModule );
            else
                bOk = false;
        }
        else
            bOk = false;
    }
    if( !bOk || !pTok->IsEof() )
    {
         // error treatment
         WriteError( rInStm );
         return false;
    }
    return true;
}

bool SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return false;

    // reset all tmp variables for writing
    WriteReset();
    SvMemoryStream aTmpStm( 256000, 256000 );
    sal_uLong n;
    for( n = 0; n < GetModuleList().size(); n++ )
    {
        SvMetaModule * pModule = GetModuleList()[n];
        if( !pModule->IsImported() )
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

void SvIdlDataBase::StartNewFile( const OUString& rName )
{
    bExport = ( aExportFile.equalsIgnoreAsciiCase( rName ) );
}

void SvIdlDataBase::AppendAttr( SvMetaAttribute *pAttr )
{
    aAttrList.push_back( pAttr );
    if ( bExport )
        pAttr->SetNewAttribute( true );
}

void SvIdlDataBase::AddDepFile(OUString const& rFileName)
{
    m_DepFiles.insert(rFileName);
}

struct WriteDep
{
    SvFileStream & m_rStream;
    explicit WriteDep(SvFileStream & rStream) : m_rStream(rStream) { }
    void operator() (OUString const& rItem)
    {
        m_rStream.WriteCharPtr( " \\\n " );
        m_rStream.WriteCharPtr( OUStringToOString(rItem, RTL_TEXTENCODING_UTF8).getStr() );
    }
};

// write a dummy target for one included file, so the incremental build does
// not break with "No rule to make target" if the included file is removed
struct WriteDummy
{
    SvFileStream & m_rStream;
    explicit WriteDummy(SvFileStream & rStream) : m_rStream(rStream) { }
    void operator() (OUString const& rItem)
    {
        m_rStream.WriteCharPtr( OUStringToOString(rItem, RTL_TEXTENCODING_UTF8).getStr() );
        m_rStream.WriteCharPtr( " :\n\n" );
    }
};

bool SvIdlDataBase::WriteDepFile(
        SvFileStream & rStream, OUString const& rTarget)
{
    rStream.WriteCharPtr( OUStringToOString(rTarget, RTL_TEXTENCODING_UTF8).getStr() );
    rStream.WriteCharPtr( " :" );
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDep(rStream));
    rStream.WriteCharPtr( "\n\n" );
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDummy(rStream));
    return rStream.GetError() == SVSTREAM_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
