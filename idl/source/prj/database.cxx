/*************************************************************************
 *
 *  $RCSfile: database.cxx,v $
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#include <tools/debug.hxx>
#include <database.hxx>
#include <globals.hxx>
#pragma hdrstop


/****************** SvIdlDataBase ****************************************/
/*************************************************************************
|*    SvIdlDataBase::SvIdlDataBase()
|*
|*    Beschreibung
*************************************************************************/
/*
void PrimeNumber(){
    USHORT i, n;
    for( i = 5001; i < 5500; i += 2 ){
        for( n = 2; n < i && ((i % n) != 0); n++ );
        if( n == i ){
            printf( "\nPrimzahl: %d\n", i );
            return;
        }
    }
}
*/

SvIdlDataBase::SvIdlDataBase()
    : bIsModified( FALSE )
    , bExport( FALSE )
    , aPersStream( *IDLAPP->pClassMgr, NULL )
    , pIdTable( NULL )
    , nUniqueId( 0 )
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
|*    SvIdlDataBase::IsModified()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlDataBase::IsModified() const
{
    for( ULONG n = 0; n < aModuleList.Count(); n++ )
        if( aModuleList.GetObject( n )->IsModified() )
            return TRUE;
    //bIsModified;
    return FALSE;
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
        // bin„ren Datenbasen inkompatibel

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
    for( ULONG n = 0; n < aModuleList.Count(); n++ )
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
#define DATABASE_SIGNATURE  (UINT32)0x13B799F2
#define DATABASE_VER 0x0006
BOOL SvIdlDataBase::IsBinaryFormat( SvStream & rStm )
{
    UINT32  nSig = 0;
    ULONG   nPos = rStm.Tell();
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
    DBG_ASSERT( aTypeList.Count() == 0, "type list already initialized" )
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );

    USHORT  nVersion = 0;
    UINT32  nSig = 0;

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
void SvIdlDataBase::Save( SvStream & rStm, UINT32 nFlags )
{
    SvPersistStream aPStm( *IDLAPP->pClassMgr, &rStm );
    aPStm.SetContextFlags( nFlags );

    aPStm << (UINT32)DATABASE_SIGNATURE;
    aPStm << (USHORT)DATABASE_VER;

    BOOL bOnlyStreamedObjs = FALSE;
    if( nFlags & IDL_WRITE_CALLING )
        bOnlyStreamedObjs = TRUE;

    if( bOnlyStreamedObjs )
    {
        SvMetaClassMemberList aList;
        for( ULONG n = 0; n < GetModuleList().Count(); n++ )
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
      || aError.nLine == pTok->GetLine() && aError.nColumn < pTok->GetColumn() )
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
BOOL SvIdlDataBase::FindId( const ByteString & rIdName, ULONG * pVal )
{
    if( pIdTable )
    {
        UINT32 nHash;
        if( pIdTable->Test( rIdName, &nHash ) )
        {
            *pVal = pIdTable->Get( nHash )->GetValue();
            return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*    SvIdlDataBase::InsertId()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvIdlDataBase::InsertId( const ByteString & rIdName, ULONG nVal )
{
    if( !pIdTable )
        pIdTable = new SvStringHashTable( 20003 );

    UINT32 nHash;
    if( pIdTable->Insert( rIdName, &nHash ) )
    {
        pIdTable->Get( nHash )->SetValue( nVal );
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*    SvIdlDataBase::ReadIdFile()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlDataBase::ReadIdFile( const String & rFileName )
{
#ifndef MAC
    DirEntry aFullName( rFileName );
#else
    DirEntry aFullName( rFileName, FSYS_STYLE_UNX );
#endif
    aFullName.Find( GetPath() );

    String * pIdFile = aIdFileList.First();
    while( pIdFile )
    {
        if( *pIdFile == rFileName )
            return TRUE; // schon eingelesen
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
                        return FALSE;
                    }

                    ULONG nVal = 0;
                    BOOL bOk = TRUE;
                    while( bOk )
                    {
                        pTok = aTokStm.GetToken_Next();
                        if( pTok->IsIdentifier() )
                        {
                            ULONG n;
                            if( FindId( pTok->GetString(), &n ) )
                                nVal += n;
                            else
                                bOk = FALSE;
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
                                return FALSE;
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
                            return FALSE;
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
                            return FALSE;
                        }
                    }
                    if( !ReadIdFile( String::CreateFromAscii(aName.GetBuffer()) ) )
                    {
                        ByteString aStr = "cannot read file: ";
                        aStr += aName;
                        SetError( aStr, pTok );
                        WriteError( aTokStm );
                        return FALSE;
                    }
                }
            }
            else
                pTok = aTokStm.GetToken_Next();
        }
    }
    else
        return FALSE;
    return TRUE;
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
|*    SvIdlDataBase::FindName()
|*
|*    Beschreibung
*************************************************************************/
ByteString * SvIdlDataBase::FindName( const ByteString & rName, ByteStringList & rList )
{
    ByteString * pS = rList.First();
    while( pS && *pS != rName )
        pS = rList.Next();
    return pS;
}

/*************************************************************************
|*    SvIdlDataBase::FillTypeList()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlDataBase::FillTypeList( SvMetaTypeList & rOutList,
                                  SvToken * pNameTok )
{
    rOutList.Clear();
//    if( pNameTok->IsIdentifierHash() ) Optimierung spaeter
    if( pNameTok->IsIdentifier() )
    {
        ByteString aName = pNameTok->GetString();
        SvMetaType * pMetaType = GetTypeList().First();
        while( pMetaType )
        {
            if( pMetaType->GetName() == aName )
                rOutList.Insert( pMetaType, LIST_APPEND );
            pMetaType = GetTypeList().Next();
        }
    }
    return rOutList.Count() != 0;
}

/*************************************************************************
|*    SvIdlDataBase::ReadKnownType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvIdlDataBase::ReadKnownType( SvTokenStream & rInStm )
{
    BOOL bIn    = FALSE;
    BOOL bOut   = FALSE;
    int nCall0  = CALL_VALUE;
    int nCall1  = CALL_VALUE;
    BOOL bSet   = FALSE; //irgent ein Attribut gesetzt

    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->HasHash() )
    {
        UINT32 nBeginPos = 0; // kann mit Tell nicht vorkommen
        while( nBeginPos != rInStm.Tell() )
        {
            nBeginPos = rInStm.Tell();
            if( pTok->Is( SvHash_in() ) )
            {
                bIn  = TRUE;
                pTok = rInStm.GetToken_Next();
                bSet = TRUE;
            }
            if( pTok->Is( SvHash_out() ) )
            {
                bOut = TRUE;
                pTok = rInStm.GetToken_Next();
                bSet = TRUE;
            }
            if( pTok->Is( SvHash_inout() ) )
            {
                bIn  = TRUE;
                bOut = TRUE;
                pTok = rInStm.GetToken_Next();
                bSet = TRUE;
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
                    bSet = TRUE;
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

            DBG_ASSERT( aTmpTypeList.First(), "mindestens ein Element" )
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
SvMetaAttribute * SvIdlDataBase::FindAttr( SvMetaAttributeMemberList * pList,
                                      SvMetaAttribute * pAttr ) const
{
    for( ULONG n = 0; n < pList->Count(); n++ )
    {
        SvMetaAttribute * p = pList->GetObject( n );
        if( p->GetName() == pAttr->GetName() )
            return pAttr;
        else if( p->GetSlotId().GetValue() == pAttr->GetSlotId().GetValue() )
        {
            ByteString aStr = "different slot names with same id: ";
            aStr += p->GetName();
            aStr += " and ";
            aStr += pAttr->GetName();
            WriteError( "warning", "*.srs", aStr );
            return FALSE;
        }
    }
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
    UINT32  nTokPos = rInStm.Tell();

    if( !pType )
        pType = ReadKnownType( rInStm );

    if( pType )
    {
        // Wenn wir Slots auf die Wiese stellen, d"urfen wir nicht voraussetzen,
        // da\s jeder Slot einen anderen Namen hat!
/*
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsIdentifier() )
            for( ULONG n = 0; n < aAttrList.Count(); n++ )
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
            ULONG n;
            if( FindId( pTok->GetString(), &n ) )
            {
                for( ULONG n = 0; n < aAttrList.Count(); n++ )
                {
                    SvMetaAttribute * pAttr = aAttrList.GetObject( n );
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
    ULONG n;
    if( FindId( rId, &n ) )
    {
        for( ULONG n = 0; n < aAttrList.Count(); n++ )
        {
            SvMetaAttribute * pAttr = aAttrList.GetObject( n );
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
    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->IsIdentifier() )
        for( ULONG n = 0; n < aClassList.Count(); n++ )
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
#ifndef W31
        fprintf( stdout, "%s", rText.GetBuffer() );
#endif
}

/*************************************************************************
|*    SvIdlDataBase::WriteError()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::WriteError( const ByteString & rErrWrn,
                                const ByteString & rFileName,
                                const ByteString & rErrorText,
                                ULONG nRow, ULONG nColumn ) const
{
    //Fehlerbehandlung
#ifndef W31
    fprintf( stderr, "\n%s --- %s: ( %ld, %ld )\n",
             rFileName.GetBuffer(), rErrWrn.GetBuffer(), nRow, nColumn );

    if( rErrorText.Len() )
    { // Fehler gesetzt
        fprintf( stderr, "\t%s\n", rErrorText.GetBuffer() );
    }
#endif
}

/*************************************************************************
|*    SvIdlDataBase::WriteError()
|*
|*    Beschreibung
*************************************************************************/
void SvIdlDataBase::WriteError( SvTokenStream & rInStm )
{
    //Fehlerbehandlung
#ifndef W31
    String aFileName( rInStm.GetFileName() );
    ByteString aErrorText;
    ULONG   nRow = 0, nColumn = 0;

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
        aErrorText += aError.nLine;
        aErrorText += ", ";
        aErrorText += aError.nColumn;
        aErrorText += " )";

        // Fehler zuruecksetzen
        aError = SvIdlError();
    }

    WriteError( "error", ByteString( aFileName, RTL_TEXTENCODING_UTF8 ), aErrorText, nRow, nColumn );

    DBG_ASSERT( pTok, "token must be found" )
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
#endif
}

/****************** SvIdlWorkingBase ****************************************/
/*************************************************************************
|*    SvIdlWorkingBase::SvIdlWorkingBase()
|*
|*    Beschreibung
*************************************************************************/
SvIdlWorkingBase::SvIdlWorkingBase()
{
}

/*************************************************************************
|*    SvIdlWorkingBase::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::ReadSvIdl( SvTokenStream & rInStm, BOOL bImported, const String & rPath )
{
    aPath = rPath; // nur fuer den durchlauf gueltig
    SvToken * pTok;
    BOOL bOk = TRUE;
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
                            bOk = FALSE;
                        }
                        else
                        {
                            aStm.Seek( 0 );
                            aStm.ResetError();
                            SvTokenStream aTokStm( aStm, aFullName.GetFull() );
                            bOk = ReadSvIdl( aTokStm, TRUE, rPath );
                        }
                    }
                }
                else
                    bOk = FALSE;
            }
            else
                bOk = FALSE;
        }

    UINT32 nBeginPos = 0xFFFFFFFF; // kann mit Tell nicht vorkommen

    while( bOk && nBeginPos != rInStm.Tell() )
    {
        nBeginPos = rInStm.Tell();
        pTok = rInStm.GetToken();
        if( pTok->IsEof() )
            return TRUE;
        if( pTok->IsEmpty() )
            bOk = FALSE;

        // nur ein import ganz am Anfang
        /*else */if( pTok->Is( SvHash_module() ) )
        {
            SvMetaModuleRef aModule = new SvMetaModule( rInStm.GetFileName(), bImported );
            if( aModule->ReadSvIdl( *this, rInStm ) )
                GetModuleList().Append( aModule );
            else
                bOk = FALSE;
        }
        else
            bOk = FALSE;
    }
    if( !bOk || !pTok->IsEof() )
    {
         //Fehlerbehandlung
         WriteError( rInStm );
         return FALSE;
    }
    return TRUE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteSvIdl( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    SvStringHashList aList;
    if( GetIdTable() )
    {
        GetIdTable()->FillHashList( &aList );
        SvStringHashEntry * pEntry = aList.First();
        while( pEntry )
        {
            rOutStm << "#define " << pEntry->GetName().GetBuffer()
                    << '\t' << ByteString( pEntry->GetValue() ).GetBuffer()
                    << endl;
            pEntry = aList.Next();
        }
    }

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        //if( !pModule->IsImported() )
            pModule->WriteSvIdl( *this, rOutStm, 0 );
    }
    return TRUE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSfx()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteSfx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    // alle Tmp-Variablen fuer das Schreiben zuruecksetzen
    WriteReset();
    SvMemoryStream aTmpStm( 256000, 256000 );
    ULONG n;
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
    return TRUE;
}

BOOL SvIdlWorkingBase::WriteHelpIds( SvStream& rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    Table aIdTable;
    ULONG n;
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

    return TRUE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteCHeader()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteCHeader( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    ULONG n;
    for( n = 0; n < GetTypeList().Count(); n++ )
    {
        SvMetaType * pType = GetTypeList().GetObject( n );
        pType->Write( *this, rOutStm, 0, WRITE_C_HEADER );
    }
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
        {
            aModulePrefix = pModule->GetModulePrefix();
            pModule->Write( *this, rOutStm, 0, WRITE_C_HEADER );
        }
    }
    return TRUE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteCSource()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteCSource( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    ULONG n;
    for( n = 0; n < GetTypeList().Count(); n++ )
    {
        SvMetaType * pType = GetTypeList().GetObject( n );
        pType->Write( *this, rOutStm, 0, WRITE_C_SOURCE );
    }
    rOutStm << endl;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
        {
            aModulePrefix = pModule->GetModulePrefix();
            pModule->Write( *this, rOutStm, 0, WRITE_C_SOURCE );
        }
    }
    return TRUE;
}



/*************************************************************************
|*    SvIdlWorkingBase::WriteSfxItem()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteSfxItem( SvStream & rOutStm )
{
/*
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
        {
            const SvMetaItemMemberList & rItemList = pModule->GetItemList();
            for( ULONG i = 0; i < rItemList.Count(); i++ )
            {
                SvMetaItem * pItem = rItemList.GetObject( i );
                if( HAS_BASE( SvMetaItem, pItem ) )
                {
                    // Header
                    ((SvMetaItem*)pItem)->WriteSfxItem( *this, rOutStm, TRUE );
                    // Source
                    ((SvMetaItem*)pItem)->WriteSfxItem( *this, rOutStm, FALSE );
                }
            }
        }
    }
    return TRUE;
*/
    return FALSE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteSbx()
|*
|*    Beschreibung
*************************************************************************/
/*
BOOL SvIdlWorkingBase::WriteSbx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    SvNamePosList   aList;
    SvMemoryStream aTmpStm( 256000, 256000 );
    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->WriteSbx( *this, aTmpStm, aList );
    }
    // Version, steht auch in so2 auto.cxx
    rOutStm << (UINT32)0x1258F170;
    rOutStm << (UINT32)aList.Count();
    ULONG i ;
    for( i = 0; i < aList.Count(); i++ )
    {
        SvNamePos * p = aList.GetObject( i );
        rOutStm << p->aUUId;
        rOutStm << (UINT32)0;
    }
    // Ende der Tabelle
    ULONG nEndPos = rOutStm.Tell();
    rOutStm.Seek( 2 * sizeof( UINT32 ) );
    SvGlobalName aTmpName;
    for( i = 0; i < aList.Count(); i++ )
    {
        SvNamePos * p = aList.GetObject( i );
        // Um an die richtige Position zu gelangen
        rOutStm >> aTmpName;
        // richtigen Offset schreiben
        rOutStm << (UINT32)(nEndPos + p->nStmPos);
    }
    aTmpStm.Seek( 0L );
    rOutStm << aTmpStm;

    return TRUE;
}
*/

/*************************************************************************
|*    SvIdlWorkingBase::WriteOdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteOdl( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->Write( *this, rOutStm, 0, WRITE_ODL );
    }
    return TRUE;
}

/*************************************************************************
|*    OdlWorkingBase::WriteSrc()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteSrc( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    Table aIdTable;
    ULONG n;
    for( n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        //if( !pModule->IsImported() )
            pModule->WriteSrc( *this, rOutStm, &aIdTable );
    }
    const SvMetaAttributeMemberList & rAttrList = GetAttrList();
    for( n = 0; n < rAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = rAttrList.GetObject( n );
        pAttr->WriteSrc( *this, rOutStm, &aIdTable );
    }

    return TRUE;
}

/*************************************************************************
|*    OdlWorkingBase::WriteCxx()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteCxx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->WriteCxx( *this, rOutStm, 0 );
    }

    return TRUE;
}

/*************************************************************************
|*    OdlWorkingBase::WriteHxx()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteHxx( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->WriteHxx( *this, rOutStm, 0 );
    }

    return TRUE;
}

void SvIdlDataBase::StartNewFile( const String& rName )
{
    bExport = ( aExportFile.EqualsIgnoreCaseAscii( rName ) );
}

void SvIdlDataBase::AppendAttr( SvMetaAttribute *pAttr )
{
    aAttrList.Append( pAttr );
    if ( bExport )
        pAttr->SetNewAttribute( TRUE );
}

BOOL SvIdlWorkingBase::WriteCSV( SvStream& rStrm )
{
    SvMetaAttributeMemberList &rList = GetAttrList();
    ULONG nCount = rList.Count();
    for ( ULONG n=0; n<nCount; n++ )
    {
        if ( rList.GetObject(n)->IsNewAttribute() )
        {
            rList.GetObject(n)->WriteCSV( *this, rStrm );
        }
    }

    if ( rStrm.GetError() != SVSTREAM_OK )
        return FALSE;
    else
        return TRUE;
}

/*************************************************************************
|*    SvIdlWorkingBase::WriteDocumentation()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvIdlWorkingBase::WriteDocumentation( SvStream & rOutStm )
{
    if( rOutStm.GetError() != SVSTREAM_OK )
        return FALSE;

    for( ULONG n = 0; n < GetModuleList().Count(); n++ )
    {
        SvMetaModule * pModule = GetModuleList().GetObject( n );
        if( !pModule->IsImported() )
            pModule->Write( *this, rOutStm, 0, WRITE_DOCU );
    }
    return TRUE;
}



#endif // IDL_COMPILER

