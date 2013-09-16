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
    : bExport( sal_False )
    , nUniqueId( 0 )
    , nVerbosity( rCmd.nVerbosity )
    , aPersStream( *IDLAPP->pClassMgr, NULL )
    , pIdTable( NULL )
{
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
    return NULL;
}

#define DATABASE_SIGNATURE  (sal_uInt32)0x13B799F2
#define DATABASE_VER 0x0006
sal_Bool SvIdlDataBase::IsBinaryFormat( SvStream & rStm )
{
    sal_uInt32  nSig = 0;
    sal_uLong   nPos = rStm.Tell();
    rStm >> nSig;
    rStm.Seek( nPos );

    return nSig == DATABASE_SIGNATURE;
}

void SvIdlDataBase::Load( SvStream & rStm )
{
    DBG_ASSERT( aTypeList.empty(), "type list already initialized" );
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );

    sal_uInt16  nVersion = 0;
    sal_uInt32  nSig = 0;

    aPStm >> nSig;
    aPStm >> nVersion;
    if( nSig != DATABASE_SIGNATURE )
    {
        aPStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        return;
    }
    if( nVersion != DATABASE_VER )
    {
        aPStm.SetError( SVSTREAM_WRONGVERSION );
        return;
    }
    aPStm >> aClassList;
    aPStm >> aTypeList;
    aPStm >> aAttrList;
    aPStm >> aModuleList;
    aPStm >> nUniqueId;

    if( aPStm.IsEof() )
        aPStm.SetError( SVSTREAM_GENERALERROR );
}

void SvIdlDataBase::Save( SvStream & rStm, sal_uInt32 nFlags )
{
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );
    aPStm.SetContextFlags( nFlags );

    aPStm << (sal_uInt32)DATABASE_SIGNATURE;
    aPStm << (sal_uInt16)DATABASE_VER;

    sal_Bool bOnlyStreamedObjs = sal_False;
    if( nFlags & IDL_WRITE_CALLING )
        bOnlyStreamedObjs = sal_True;

    if( bOnlyStreamedObjs )
    {
        SvMetaClassMemberList aList;
        for( sal_uLong n = 0; n < GetModuleList().size(); n++ )
        {
            SvMetaModule * pModule = GetModuleList()[n];
            if( !pModule->IsImported() )
                aList.insert( pModule->GetClassList() );
        }
        aPStm << aList;
    }
    else
        aPStm << aClassList;

    aTypeList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aAttrList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aModuleList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aPStm << nUniqueId;
}

void SvIdlDataBase::SetError( const OString& rError, SvToken * pTok )
{
    if( pTok->GetLine() > 10000 )
        aError.SetText( "line count overflow" );

    if( aError.nLine < pTok->GetLine()
      || (aError.nLine == pTok->GetLine() && aError.nColumn < pTok->GetColumn()) )
    {
        aError = SvIdlError( pTok->GetLine(), pTok->GetColumn() );
        aError.SetText( rError );
    }
}

void SvIdlDataBase::Push( SvMetaObject * pObj )
{
    GetStack().Push( pObj );
}

sal_Bool SvIdlDataBase::FindId( const OString& rIdName, sal_uLong * pVal )
{
    if( pIdTable )
    {
        sal_uInt32 nHash;
        if( pIdTable->Test( rIdName, &nHash ) )
        {
            *pVal = pIdTable->Get( nHash )->GetValue();
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool SvIdlDataBase::InsertId( const OString& rIdName, sal_uLong nVal )
{
    if( !pIdTable )
        pIdTable = new SvStringHashTable( 20003 );

    sal_uInt32 nHash;
    if( pIdTable->Insert( rIdName, &nHash ) )
    {
        pIdTable->Get( nHash )->SetValue( nVal );
        return sal_True;
    }
    return sal_False;
}

sal_Bool SvIdlDataBase::ReadIdFile( const OUString & rFileName )
{
    OUString aFullName;
    osl::File::searchFileURL( rFileName, GetPath(), aFullName);
    osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );

    for ( size_t i = 0, n = aIdFileList.size(); i < n; ++i )
        if ( aIdFileList[ i ] == rFileName )
            return sal_True;

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
                        OString aStr(RTL_CONSTASCII_STRINGPARAM(
                            "unexpected token after define"));
                        // set error
                        SetError( aStr, pTok );
                        WriteError( aTokStm );
                        return sal_False;
                    }

                    sal_uLong nVal = 0;
                    sal_Bool bOk = sal_True;
                    while( bOk )
                    {
                        pTok = aTokStm.GetToken_Next();
                        if( pTok->IsIdentifier() )
                        {
                            sal_uLong n;
                            if( FindId( pTok->GetString(), &n ) )
                                nVal += n;
                            else
                                bOk = sal_False;
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
                                SetError( aStr.makeStringAndClear(), pTok );
                                WriteError( aTokStm );
                                return sal_False;
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
                            OString aStr(RTL_CONSTASCII_STRINGPARAM("hash table overflow: "));
                            SetError( aStr, pTok );
                            WriteError( aTokStm );
                            return sal_False;
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
                            OString aStr(RTL_CONSTASCII_STRINGPARAM(
                                "unexpected eof in #include"));
                            // set error
                            SetError(aStr, pTok);
                            WriteError( aTokStm );
                            return sal_False;
                        }
                    }
                    if (!ReadIdFile(OStringToOUString(aName.toString(),
                        RTL_TEXTENCODING_ASCII_US)))
                    {
                        OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                            "cannot read file: "));
                        aStr.append(aName.makeStringAndClear());
                        SetError(aStr.makeStringAndClear(), pTok);
                        WriteError( aTokStm );
                        return sal_False;
                    }
                }
            }
            else
                pTok = aTokStm.GetToken_Next();
        }
    }
    else
        return sal_False;
    return sal_True;
}

SvMetaType * SvIdlDataBase::FindType( const SvMetaType * pPType,
                                    SvMetaTypeMemberList & rList )
{
    for( SvMetaTypeMemberList::const_iterator it = rList.begin(); it != rList.end(); ++it )
        if( *it == pPType )
            return *it;
    return NULL;
}

SvMetaType * SvIdlDataBase::FindType( const OString& rName )
{
    for( SvMetaTypeMemberList::const_iterator it = aTypeList.begin(); it != aTypeList.end(); ++it )
        if( rName.equals((*it)->GetName().getString()) )
            return *it;
    return NULL;
}

SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    sal_Bool bIn    = sal_False;
    sal_Bool bOut   = sal_False;
    int nCall0  = CALL_VALUE;
    int nCall1  = CALL_VALUE;
    sal_Bool bSet   = sal_False; // any attribute set

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
                bIn  = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
            if( pTok->Is( SvHash_out() ) )
            {
                bOut = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
            if( pTok->Is( SvHash_inout() ) )
            {
                bIn  = sal_True;
                bOut = sal_True;
                pTok = rInStm.GetToken_Next();
                bSet = sal_True;
            }
        }
    }

    if( pTok->IsIdentifier() )
    {
        OString aName = pTok->GetString();
        SvMetaTypeMemberList & rList = GetTypeList();
        SvMetaTypeMemberList::const_iterator it = rList.begin();
        SvMetaType * pType = NULL;
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
            pTok = rInStm.GetToken();
            if( pTok->IsChar() )
            {
                if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                {
                    nCall0 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                        CALL_POINTER;
                    rInStm.GetToken_Next();
                    pTok = rInStm.GetToken();
                    if( pTok->GetChar() == '&' || pTok->GetChar() == '*' )
                    {
                        nCall1 = (pTok->GetChar() == '&') ? CALL_REFERENCE :
                                                            CALL_POINTER;
                        rInStm.GetToken_Next();
                    }
                    bSet = sal_True;
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
    return NULL;
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
    return NULL;
}

SvMetaAttribute* SvIdlDataBase::SearchKnownAttr
(
    const SvNumberIdentifier& rId
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

    return NULL;
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
    return NULL;
}

void SvIdlDataBase::Write(const OString& rText)
{
    if( nVerbosity != 0 )
        fprintf( stdout, "%s", rText.getStr() );
}

void SvIdlDataBase::WriteError( const OString& rErrWrn,
                                const OString& rFileName,
                                const OString& rErrorText,
                                sal_uLong nRow, sal_uLong nColumn ) const
{
    // error treatment
    fprintf( stderr, "\n%s --- %s: ( %ld, %ld )\n",
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
    SvToken *pTok = rInStm.GetToken();

    // error position
    nRow    = pTok->GetLine();
    nColumn = pTok->GetColumn();

    if( aError.IsError() )
    { // error set
        // search error token
        // error text
        if( !aError.GetText().isEmpty() )
        {
            aErrorText.append(RTL_CONSTASCII_STRINGPARAM("may be <"));
            aErrorText.append(aError.GetText());
        }
        SvToken * pPrevTok = NULL;
        while( pTok != pPrevTok )
        {
            pPrevTok = pTok;
            if( pTok->GetLine() == aError.nLine
              && pTok->GetColumn() == aError.nColumn )
                break;
            pTok = rInStm.GetToken_PrevAll();
        }

        // error position
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM("> at ( "));
        aErrorText.append(static_cast<sal_Int64>(aError.nLine));
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM(", "));
        aErrorText.append(static_cast<sal_Int64>(aError.nColumn));
        aErrorText.append(RTL_CONSTASCII_STRINGPARAM(" )"));

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
        pTok = rInStm.GetToken();
    }
    if( pTok && pTok->IsIdentifier() )
    {
        OString aN = IDLAPP->pHashTable->GetNearString( pTok->GetString() );
        if( !aN.isEmpty() )
            fprintf( stderr, "%s versus %s\n", pTok->GetString().getStr(), aN.getStr() );
    }
}

SvIdlWorkingBase::SvIdlWorkingBase(const SvCommand& rCmd) : SvIdlDataBase(rCmd)
{
}

sal_Bool SvIdlWorkingBase::ReadSvIdl( SvTokenStream & rInStm, sal_Bool bImported, const OUString & rPath )
{
    aPath = rPath; // only valid for this iteration
    SvToken * pTok;
    sal_Bool bOk = sal_True;
        pTok = rInStm.GetToken();
        // only one import at the very beginning
        if( pTok->Is( SvHash_import() ) )
        {
            rInStm.GetToken_Next();
            rInStm.Read( '(' ); // optional
            pTok = rInStm.GetToken_Next();
            if( pTok->IsString() )
            {
                OUString aFullName;
                if( osl::FileBase::E_None == osl::File::searchFileURL(
                    OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US),
                    rPath,
                    aFullName) )
                {
                    osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );
                    this->AddDepFile(aFullName);
                    SvFileStream aStm( aFullName, STREAM_STD_READ | STREAM_NOCREATE );
                    Load( aStm );
                    if( aStm.GetError() != SVSTREAM_OK )
                    {
                        if( aStm.GetError() == SVSTREAM_WRONGVERSION )
                        {
                            OStringBuffer aStr("wrong version, file ");
                            aStr.append(OUStringToOString( aFullName, RTL_TEXTENCODING_UTF8));
                            SetError(aStr.makeStringAndClear(), pTok);
                            WriteError( rInStm );
                            bOk = sal_False;
                        }
                        else
                        {
                            aStm.Seek( 0 );
                            aStm.ResetError();
                            SvTokenStream aTokStm( aStm, aFullName );
                            bOk = ReadSvIdl( aTokStm, sal_True, rPath );
                        }
                    }
                }
                else
                    bOk = sal_False;
            }
            else
                bOk = sal_False;
        }

    sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell

    while( bOk && nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        pTok = rInStm.GetToken();
        if( pTok->IsEof() )
            return sal_True;
        if( pTok->IsEmpty() )
            bOk = sal_False;

        // only one import at the very beginning
        if( pTok->Is( SvHash_module() ) )
        {
            SvMetaModuleRef aModule = new SvMetaModule( rInStm.GetFileName(), bImported );
            if( aModule->ReadSvIdl( *this, rInStm ) )
                GetModuleList().push_back( aModule );
            else
                bOk = sal_False;
        }
        else
            bOk = sal_False;
    }
    if( !bOk || !pTok->IsEof() )
    {
         // error treatment
         WriteError( rInStm );
         return sal_False;
    }
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSvIdl( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    SvStringHashList aList;
    if( GetIdTable() )
    {
        GetIdTable()->FillHashList( &aList );
        for ( size_t i = 0, n = aList.size(); i < n; ++i )
        {
            SvStringHashEntry* pEntry = aList[ i ];
            rOutStm << "#define " << pEntry->GetName().getStr()
                    << '\t'
                    << OString::number(pEntry->GetValue()).getStr()
                    << endl;
        }
    }

    for( sal_uLong n = 0; n < GetModuleList().size(); n++ )
    {
        SvMetaModule * pModule = GetModuleList()[n];
        pModule->WriteSvIdl( *this, rOutStm, 0 );
    }
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

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
    rOutStm << aTmpStm;
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteHelpIds( SvStream& rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    HelpIdTable aIdTable;
    sal_uLong n;
    for( n = 0; n < GetModuleList().size(); n++ )
    {
        SvMetaModule * pModule = GetModuleList()[n];
        pModule->WriteHelpIds( *this, rOutStm, aIdTable );
    }

    const SvMetaAttributeMemberList & rAttrList = GetAttrList();
    for( n = 0; n < rAttrList.size(); n++ )
    {
        SvMetaAttribute * pAttr = rAttrList[n];
        pAttr->WriteHelpId( *this, rOutStm, aIdTable );
    }

    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteSfxItem( SvStream & )
{
    return sal_False;
}

void SvIdlDataBase::StartNewFile( const OUString& rName )
{
    bExport = ( aExportFile.equalsIgnoreAsciiCase( rName ) );
}

void SvIdlDataBase::AppendAttr( SvMetaAttribute *pAttr )
{
    aAttrList.push_back( pAttr );
    if ( bExport )
        pAttr->SetNewAttribute( sal_True );
}

sal_Bool SvIdlWorkingBase::WriteCSV( SvStream& rStrm )
{
    SvMetaAttributeMemberList &rList = GetAttrList();
    sal_uLong nCount = rList.size();
    for ( sal_uLong n=0; n<nCount; n++ )
    {
        if ( rList[n]->IsNewAttribute() )
        {
            rList[n]->WriteCSV( *this, rStrm );
        }
    }

    if ( rStrm.GetError() != SVSTREAM_OK )
        return sal_False;
    else
        return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteDocumentation( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    for( sal_uLong n = 0; n < GetModuleList().size(); n++ )
    {
        SvMetaModule * pModule = GetModuleList()[n];
        if( !pModule->IsImported() )
            pModule->Write( *this, rOutStm, 0, WRITE_DOCU );
    }
    return sal_True;
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
        m_rStream << " \\\n ";
        m_rStream << OUStringToOString(rItem, RTL_TEXTENCODING_UTF8).getStr();
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
        m_rStream << OUStringToOString(rItem, RTL_TEXTENCODING_UTF8).getStr();
        m_rStream << " :\n\n";
    }
};

bool SvIdlDataBase::WriteDepFile(
        SvFileStream & rStream, OUString const& rTarget)
{
    rStream << OUStringToOString(rTarget, RTL_TEXTENCODING_UTF8).getStr();
    rStream << " :";
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDep(rStream));
    rStream << "\n\n";
    ::std::for_each(m_DepFiles.begin(), m_DepFiles.end(), WriteDummy(rStream));
    return rStream.GetError() == SVSTREAM_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
