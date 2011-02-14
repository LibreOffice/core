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
#include <stdlib.h>
#include <tools/fsys.hxx>
#include <tools/debug.hxx>
#include <database.hxx>
#include <globals.hxx>

/****************** SvIdlDataBase ****************************************/
/*************************************************************************
|*    SvIdlDataBase::SvIdlDataBase()
|*
|*    Beschreibung
*************************************************************************/
/*
void PrimeNumber(){
    sal_uInt16 i, n;
    for( i = 5001; i < 5500; i += 2 ){
        for( n = 2; n < i && ((i % n) != 0); n++ );
        if( n == i ){
            printf( "\nPrimzahl: %d\n", i );
            return;
        }
    }
}
*/

SvIdlDataBase::SvIdlDataBase( const SvCommand& rCmd )
    : bExport( sal_False )
    , nUniqueId( 0 )
    , nVerbosity( rCmd.nVerbosity )
    , bIsModified( sal_False )
    , aPersStream( *IDLAPP->pClassMgr, NULL )
    , pIdTable( NULL )
{
    //PrimeNumber();
}

/*************************************************************************
|*    SvIdlDataBase::~SvIdlDataBase()
|*
|*    Beschreibung
*************************************************************************/
SvIdlDataBase::~SvIdlDataBase()
{
    String * pStr = aIdFileList.First();
    while( pStr )
    {
        delete pStr;
        pStr = aIdFileList.Next();
    }
    delete pIdTable;
}

/*************************************************************************
|*    SvIdlDataBase::GetTypeList()
|*
|*    Beschreibung
*************************************************************************/
#define ADD_TYPE( Name, OdlName, ParserChar, CName, BasName, BasPost )            \
    aTypeList.Append( new SvMetaType( SvHash_##Name()->GetName(),   \
                     BasName, OdlName, ParserChar, CName, BasName, BasPost ) );

SvMetaTypeMemberList & SvIdlDataBase::GetTypeList()
{
    if( aTypeList.Count() == 0 )
    { // Initial fuellen
        aTypeList.Append( new SvMetaTypeString() );
        aTypeList.Append( new SvMetaTypevoid() );

        // MI: IDispatch::Invoke kann keine unsigned
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


        // A c h t u n g !!!, bei hinzufuegen von Typen werden alle
        // bin�ren Datenbasen inkompatibel

    /*
        // So tun als ob die Init-Daten auf einem Stream gelesen wurden
        SvMemoryStream aStm;
        aPersStream.SetStream( &aStm );
        // Alle Init-Daten Streamen
        aPersStream << aTypeList;
        // Nur die Id-Zuordnung merken
        aPersStream.SetStream( NULL );
    */
    }
    return aTypeList;
}

/*************************************************************************
|*
|*    SvIdlDataBase::GetModuleInfo()
|*
|*    Beschreibung
|*    Ersterstellung    MM 13.12.94
|*    Letzte Aenderung  MM 13.12.94
|*
*************************************************************************/
SvMetaModule * SvIdlDataBase::GetModule( const ByteString & rName )
{
    for( sal_uLong n = 0; n < aModuleList.Count(); n++ )
        if( aModuleList.GetObject( n )->GetName() == rName )
            return aModuleList.GetObject( n );
    return NULL;
}

/*************************************************************************
|*
|*    SvIdlDataBase::IsBinaryFormat()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    SvIdlDataBase::Load()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvIdlDataBase::Load( SvStream & rStm )
{
    DBG_ASSERT( aTypeList.Count() == 0, "type list already initialized" );
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

/*************************************************************************
|*    SvIdlDataBase::Save()
|*
|*    Beschreibung
*************************************************************************/
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
        for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
        {
            SvMetaModule * pModule = GetModuleList().GetObject( n );
            if( !pModule->IsImported() )
                aList.Append( pModule->GetClassList() );
        }
        aPStm << aList;
    }
    else
        aPStm << aClassList;

    //aClassList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aTypeList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aAttrList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aModuleList.WriteObjects( aPStm, bOnlyStreamedObjs );
    aPStm << nUniqueId;
}

/*************************************************************************
|*    SvIdlDataBase::SetError()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::SetError( const ByteString & rError, SvToken * pTok )
{
    if( pTok->GetLine() > 10000 )
        aError.SetText( "hgchcg" );

    if( aError.nLine < pTok->GetLine()
      || (aError.nLine == pTok->GetLine() && aError.nColumn < pTok->GetColumn()) )
    {
        aError = SvIdlError( pTok->GetLine(), pTok->GetColumn() );
        aError.SetText( rError );
    }
}

/*************************************************************************
|*    SvIdlDataBase::Push()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::Push( SvMetaObject * pObj )
{
    GetStack().Push( pObj );
}

#ifdef IDL_COMPILER
/*************************************************************************
|*
|*    SvIdlDataBase::FindId()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvIdlDataBase::FindId( const ByteString & rIdName, sal_uLong * pVal )
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

/*************************************************************************
|*
|*    SvIdlDataBase::InsertId()
|*
|*    Beschreibung
|*
*************************************************************************/
sal_Bool SvIdlDataBase::InsertId( const ByteString & rIdName, sal_uLong nVal )
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

/*************************************************************************
|*    SvIdlDataBase::ReadIdFile()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlDataBase::ReadIdFile( const String & rFileName )
{
    DirEntry aFullName( rFileName );
    aFullName.Find( GetPath() );

    String * pIdFile = aIdFileList.First();
    while( pIdFile )
    {
        if( *pIdFile == rFileName )
            return sal_True; // schon eingelesen
        pIdFile = aIdFileList.Next();
    }

    aIdFileList.Insert( new String( rFileName ), LIST_APPEND );

    SvTokenStream aTokStm( aFullName.GetFull() );
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
                    ByteString aDefName;
                    if( pTok->IsIdentifier() )
                        aDefName = pTok->GetString();
                    else
                    {
                        ByteString aStr( "unexpected token after define" );
                        // Fehler setzen
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
                                ByteString aStr( "unknown operator '" );
                                aStr += pTok->GetChar();
                                aStr += "'in define";
                                // Fehler setzen
                                SetError( aStr, pTok );
                                WriteError( aTokStm );
                                return sal_False;
                            }
                            if( pTok->GetChar() != '+'
                              && pTok->GetChar() != '('
                              && pTok->GetChar() != ')' )
                                // nur + erlaubt, Klammern spielen kein Rolle,
                                // da + komutativ ist
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
                            ByteString aStr = "hash table overflow: ";
                            SetError( aStr, pTok );
                            WriteError( aTokStm );
                            return sal_False;
                        }
                    }
                }
                else if( pTok->Is( SvHash_include() ) )
                {
                    pTok = aTokStm.GetToken_Next();
                    ByteString aName;
                    if( pTok->IsString() )
                        aName = pTok->GetString();
                    else if( pTok->IsChar() && pTok->GetChar() == '<' )
                    {
                        pTok = aTokStm.GetToken_Next();
                        while( !pTok->IsEof()
                          && !(pTok->IsChar() && pTok->GetChar() == '>') )
                        {
                            aName += pTok->GetTokenAsString();
                            pTok = aTokStm.GetToken_Next();
                        }
                        if( pTok->IsEof() )
                        {
                            ByteString aStr( "unexpected eof in #include" );
                            // Fehler setzen
                            SetError( aStr, pTok );
                            WriteError( aTokStm );
                            return sal_False;
                        }
                    }
                    if( !ReadIdFile( String::CreateFromAscii(aName.GetBuffer()) ) )
                    {
                        ByteString aStr = "cannot read file: ";
                        aStr += aName;
                        SetError( aStr, pTok );
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

/*************************************************************************
|*    SvIdlDataBase::FindType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvIdlDataBase::FindType( const SvMetaType * pPType,
                                    SvMetaTypeMemberList & rList )
{
    SvMetaType * pType = rList.First();
    while( pType && pPType != pType )
        pType = rList.Next();
    return pType;
}

SvMetaType * SvIdlDataBase::FindType( const ByteString & rName )
{
    SvMetaType * pType = aTypeList.First();
    while( pType && rName != pType->GetName() )
        pType = aTypeList.Next();
    return pType;
}

/*************************************************************************
|*    SvIdlDataBase::ReadKnownType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    sal_Bool bIn    = sal_False;
    sal_Bool bOut   = sal_False;
    int nCall0  = CALL_VALUE;
    int nCall1  = CALL_VALUE;
    sal_Bool bSet   = sal_False; //irgent ein Attribut gesetzt

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->HasHash() )
    {
        sal_uInt32 nBeginPos = 0; // kann mit Tell nicht vorkommen
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
/*
    SvMetaTypeList aTmpTypeList;
    if( FillTypeList( aTmpTypeList, pTok ) )
*/
    if( pTok->IsIdentifier() )
    {
        ByteString aName = pTok->GetString();
        SvMetaTypeMemberList & rList = GetTypeList();
        SvMetaType * pType = rList.First();
        while( pType )
        {
            if( pType->GetName() == aName )
                break;
            pType = rList.Next();
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
            /*
            SvMetaType * pMetaType = aTmpTypeList.First();
            while( pMetaType )
            {
                if( pMetaType->GetIn() == bIn
                  && pMetaType->GetOut() == bOut
                  && pMetaType->GetCall0() == nCall0
                  && pMetaType->GetCall1() == nCall1 )
                {
                    return pMetaType;
                }
                pMetaType = aTmpTypeList.Next();
            }
            */
            //SvMetaType * pType = aTmpTypeList.First();
            if( !bSet )
                // Ist genau dieser Typ
                return pType;

            DBG_ASSERT( aTmpTypeList.First(), "mindestens ein Element" );
            SvMetaTypeRef xType = new SvMetaType( pType->GetName(), 'h', "dummy" );
            xType->SetRef( pType );
            xType->SetIn( bIn );
            xType->SetOut( bOut );
            xType->SetCall0( nCall0 );
            xType->SetCall1( nCall1 );

            aTmpTypeList.Append( xType );
            return xType;
        }
    }
    rInStm.Seek( nTokPos );
    return NULL;
}

/*************************************************************************
|*
|*    SvIdlDataBase::ReadKnownAttr()
|*
|*    Beschreibung
|*
*************************************************************************/
SvMetaAttribute * SvIdlDataBase::ReadKnownAttr
(
    SvTokenStream & rInStm,
    SvMetaType *    pType   /* Wenn der pType == NULL, dann muss der Typ
                               noch gelesen werden. */
)
{
    sal_uInt32  nTokPos = rInStm.Tell();

    if( !pType )
        pType = ReadKnownType( rInStm );

    if( pType )
    {
        // Wenn wir Slots auf die Wiese stellen, d"urfen wir nicht voraussetzen,
        // da\s jeder Slot einen anderen Namen hat!
/*
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsIdentifier() )
            for( sal_uLong n = 0; n < aAttrList.Count(); n++ )
            {
                SvMetaAttribute * pAttr = aAttrList.GetObject( n );
                if( pAttr->GetName() == pTok->GetString() )
                    return pAttr;
            }
*/
    }
    else
    {
        // sonst SlotId?
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsIdentifier() )
        {
            sal_uLong n;
            if( FindId( pTok->GetString(), &n ) )
            {
                for( sal_uLong i = 0; i < aAttrList.Count(); i++ )
                {
                    SvMetaAttribute * pAttr = aAttrList.GetObject( i );
                    if( pAttr->GetSlotId() == pTok->GetString() )
                        return pAttr;
                }
            }

            ByteString aStr( "Nicht gefunden : " );
            aStr += pTok->GetString();
            DBG_ERROR( aStr.GetBuffer() );
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
    if( FindId( rId, &n ) )
    {
        for( sal_uLong i = 0; i < aAttrList.Count(); i++ )
        {
            SvMetaAttribute * pAttr = aAttrList.GetObject( i );
            if( pAttr->GetSlotId() == rId )
                return pAttr;
        }
    }

    return NULL;
}

/*************************************************************************
|*    SvIdlDataBase::ReadKnownClass()
|*
|*    Beschreibung
*************************************************************************/
SvMetaClass * SvIdlDataBase::ReadKnownClass( SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( pClass->GetName() == pTok->GetString() )
                return pClass;
        }

    rInStm.Seek( nTokPos );
    return NULL;
}

/*************************************************************************
|*    SvIdlDataBase::Write()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::Write( const ByteString & rText )
{
    if( nVerbosity != 0 )
        fprintf( stdout, "%s", rText.GetBuffer() );
}

/*************************************************************************
|*    SvIdlDataBase::WriteError()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::WriteError( const ByteString & rErrWrn,
                                const ByteString & rFileName,
                                const ByteString & rErrorText,
                                sal_uLong nRow, sal_uLong nColumn ) const
{
    //Fehlerbehandlung
    fprintf( stderr, "\n%s --- %s: ( %ld, %ld )\n",
             rFileName.GetBuffer(), rErrWrn.GetBuffer(), nRow, nColumn );

    if( rErrorText.Len() )
    { // Fehler gesetzt
        fprintf( stderr, "\t%s\n", rErrorText.GetBuffer() );
    }
}

/*************************************************************************
|*    SvIdlDataBase::WriteError()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::WriteError( SvTokenStream & rInStm )
{
    //Fehlerbehandlung
    String aFileName( rInStm.GetFileName() );
    ByteString aErrorText;
    sal_uLong   nRow = 0, nColumn = 0;

    rInStm.SeekEnd();
    SvToken *pTok = rInStm.GetToken();

    // Fehlerposition
    nRow    = pTok->GetLine();
    nColumn = pTok->GetColumn();

    if( aError.IsError() )
    { // Fehler gesetzt
        // Fehler Token suchen
        // Fehlertext
        if( aError.GetText().Len() )
        {
            aErrorText = "may be <";
            aErrorText += aError.GetText();
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

        // Fehlerposition
        aErrorText += "> at ( ";
        aErrorText += ByteString::CreateFromInt64(aError.nLine);
        aErrorText += ", ";
        aErrorText += ByteString::CreateFromInt64(aError.nColumn);
        aErrorText += " )";

        // Fehler zuruecksetzen
        aError = SvIdlError();
    }

    WriteError( "error", ByteString( aFileName, RTL_TEXTENCODING_UTF8 ), aErrorText, nRow, nColumn );

    DBG_ASSERT( pTok, "token must be found" );
    if( !pTok )
        return;

    // Identifier in der Naehe suchen
    if( !pTok->IsIdentifier() )
    {
        rInStm.GetToken_PrevAll();
        pTok = rInStm.GetToken();
    }
    if( pTok && pTok->IsIdentifier() )
    {
        ByteString aN = IDLAPP->pHashTable->GetNearString( pTok->GetString() );
        if( aN.Len() )
            fprintf( stderr, "%s versus %s\n", pTok->GetString().GetBuffer(), aN.GetBuffer() );
    }
}

/****************** SvIdlWorkingBase ****************************************/
/*************************************************************************
|*    SvIdlWorkingBase::SvIdlWorkingBase()
|*
|*    Beschreibung
*************************************************************************/
SvIdlWorkingBase::SvIdlWorkingBase(const SvCommand& rCmd) : SvIdlDataBase(rCmd)
{
}

/*************************************************************************
|*    SvIdlWorkingBase::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlWorkingBase::ReadSvIdl( SvTokenStream & rInStm, sal_Bool bImported, const String & rPath )
{
    aPath = rPath; // nur fuer den durchlauf gueltig
    SvToken * pTok;
    sal_Bool bOk = sal_True;
        pTok = rInStm.GetToken();
        // nur ein import ganz am Anfang
        if( pTok->Is( SvHash_import() ) )
        {
            rInStm.GetToken_Next();
            rInStm.Read( '(' ); // optional
            pTok = rInStm.GetToken_Next();
            if( pTok->IsString() )
            {
                DirEntry aFullName( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
                if( aFullName.Find( rPath ) )
                {
                    SvFileStream aStm( aFullName.GetFull(),
                                        STREAM_STD_READ | STREAM_NOCREATE );
                    Load( aStm );
                    if( aStm.GetError() != SVSTREAM_OK )
                    {
                        if( aStm.GetError() == SVSTREAM_WRONGVERSION )
                        {
                            ByteString aStr( "wrong version, file " );
                            aStr += ByteString( aFullName.GetFull(), RTL_TEXTENCODING_UTF8 );
                            SetError( aStr, pTok );
                            WriteError( rInStm );
                            bOk = sal_False;
                        }
                        else
                        {
                            aStm.Seek( 0 );
                            aStm.ResetError();
                            SvTokenStream aTokStm( aStm, aFullName.GetFull() );
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

    sal_uInt32 nBeginPos = 0xFFFFFFFF; // kann mit Tell nicht vorkommen

    while( bOk && nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        pTok = rInStm.GetToken();
        if( pTok->IsEof() )
            return sal_True;
        if( pTok->IsEmpty() )
            bOk = sal_False;

        // nur ein import ganz am Anfang
        /*else */if( pTok->Is( SvHash_module() ) )
        {
            SvMetaModuleRef aModule = new SvMetaModule( rInStm.GetFileName(), bImported );
            if( aModule->ReadSvIdl( *this, rInStm ) )
                GetModuleList().Append( aModule );
            else
                bOk = sal_False;
        }
        else
            bOk = sal_False;
    }
    if( !bOk || !pTok->IsEof() )
    {
         //Fehlerbehandlung
         WriteError( rInStm );
         return sal_False;
    }
    return sal_True;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlWorkingBase::WriteSvIdl( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    SvStringHashList aList;
    if( GetIdTable() )
    {
        GetIdTable()->FillHashList( &aList );
        SvStringHashEntry * pEntry = aList.First();
        while( pEntry )
        {
            rOutStm << "#define " << pEntry->GetName().GetBuffer()
                    << '\t'
                    << ByteString::CreateFromInt64(
                        pEntry->GetValue() ).GetBuffer()
                    << endl;
            pEntry = aList.Next();
        }
    }

    for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        //if( !pModule->IsImported() )
            pModule->WriteSvIdl( *this, rOutStm, 0 );
    }
    return sal_True;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSfx()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    // alle Tmp-Variablen fuer das Schreiben zuruecksetzen
    WriteReset();
    SvMemoryStream aTmpStm( 256000, 256000 );
    sal_uLong n;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->WriteSfx( *this, aTmpStm );
        aTmpStm.Seek( 0 );
    }
    for( n = 0; n < aUsedTypes.Count(); n++ )
    {
        SvMetaType * pType = aUsedTypes.GetObject( n );
        pType->WriteSfx( *this, rOutStm );
    }
    aUsedTypes.Clear();
    rOutStm << aTmpStm;
    return sal_True;
}

sal_Bool SvIdlWorkingBase::WriteHelpIds( SvStream& rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    Table aIdTable;
    sal_uLong n;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        //if( !pModule->IsImported() )
            pModule->WriteHelpIds( *this, rOutStm, &aIdTable );
    }

    const SvMetaAttributeMemberList & rAttrList = GetAttrList();
    for( n = 0; n < rAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = rAttrList.GetObject( n );
        pAttr->WriteHelpId( *this, rOutStm, &aIdTable );
    }

    return sal_True;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSfxItem()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlWorkingBase::WriteSfxItem( SvStream & )
{
    return sal_False;
}

void SvIdlDataBase::StartNewFile( const String& rName )
{
    bExport = ( aExportFile.EqualsIgnoreCaseAscii( rName ) );
}

void SvIdlDataBase::AppendAttr( SvMetaAttribute *pAttr )
{
    aAttrList.Append( pAttr );
    if ( bExport )
        pAttr->SetNewAttribute( sal_True );
}

sal_Bool SvIdlWorkingBase::WriteCSV( SvStream& rStrm )
{
    SvMetaAttributeMemberList &rList = GetAttrList();
    sal_uLong nCount = rList.Count();
    for ( sal_uLong n=0; n<nCount; n++ )
    {
        if ( rList.GetObject(n)->IsNewAttribute() )
        {
            rList.GetObject(n)->WriteCSV( *this, rStrm );
        }
    }

    if ( rStrm.GetError() != SVSTREAM_OK )
        return sal_False;
    else
        return sal_True;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteDocumentation()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SvIdlWorkingBase::WriteDocumentation( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return sal_False;

    for( sal_uLong n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->Write( *this, rOutStm, 0, WRITE_DOCU );
    }
    return sal_True;
}



#endif // IDL_COMPILER

