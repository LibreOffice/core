/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscinst.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _RSCINST_HXX
#define _RSCINST_HXX

#include <rscall.h>
#include <rscerror.h>
#include <rsctools.hxx>

/******************* C l a s s e s   F o r w a r d s *********************/
class RscTypCont;
class RscCmdLine;
class REResourceList;
class RscTop;
class RscInstCopy;

/******************* F u n c t i o n   F o r w a r d s *******************/
Atom HashId( const char * );          // Gibt zu einem String eine HashId
const char * GetHashString( Atom ); // Gibt zu einer Atom einen String
                                      // NULL, wenn kein Eintrag vorhanden

/******************* S t r u c t s ***************************************/

/****************** C L A S S E S ****************************************/
class RscInst
{
    void            MakeCopy( RSCINST aCopyInst );
    static RSCINST  GetSysLangInst( RSCINST & rInst );
public:
    RSCINST     aInst;

                RscInst();
                RscInst( const RscInst & rInst );
                RscInst( RSCINST aTmpI );
    RscInst&    operator =  ( const RscInst& rRscInst );
                ~RscInst();
    void        OverWrite( RscInstCopy & rInst );
    BOOL        IsInst() const { return( aInst.IsInst() ); }

                // Listen Methoden
    ERRTYPE     SetElement( const RscId & rName, RscInstCopy & rInst );
    ERRTYPE     SetPosEle( sal_uInt32 nPos, RscInstCopy & rInst );
    ERRTYPE     SetPosRscId( sal_uInt32 nPos, const RscId & rId );
    SUBINFO_STRUCT  GetInfoEle( sal_uInt32 nPos );
    sal_uInt32      GetCount();
    RscInst     GetElement( RscTop * pClass, const RscId & rName );
    RscInst     GetPosEle( sal_uInt32 nPos );
    ERRTYPE     MovePosEle( sal_uInt32 nDestPos, sal_uInt32 nSourcePos );
    ERRTYPE     DeleteElement( RscTop * pClass, const RscId & rName );
    ERRTYPE     DeletePosEle( sal_uInt32 nPos );

    ERRTYPE     SetVar( Atom nVarName, RscInstCopy & rInst );
    ERRTYPE     SetConst( Atom nVarName, Atom nConstId );
    ERRTYPE     SetBool( Atom nVarName, BOOL );

    // Hack fuer X, Y, Width, Height
    static ERRTYPE SetCorrectValues( RSCINST & rInst, RSCINST & rVarInst,
                                    INT32 lValue, sal_uInt32 nTupelIdx );
    ERRTYPE     SetNumber( Atom nVarName, INT32 );

    ERRTYPE     SetString( Atom nVarName, const char * );
    ERRTYPE     SetConst( Atom nConstId );
    ERRTYPE     SetBool( BOOL );
    ERRTYPE     SetNumber( INT32 );
    ERRTYPE     SetString( const char * );
    ERRTYPE     SetRef( const RscId & rRscId );
    ERRTYPE     SetDefault( Atom nVarName );

    RscInst     GetVar( Atom nVarName );
    Atom        GetConst( Atom nVarName = InvalidAtom );
    sal_uInt32      GetConstPos( Atom nVarName = InvalidAtom );
    BOOL        GetBool( Atom nVarName = InvalidAtom );

    // Hack fuer X, Y, Width, Height
    static INT32 GetCorrectValues( RSCINST & rInst, RSCINST & rVarInst,
                                    sal_uInt32 nTupelIdx );
    INT32       GetNumber( Atom nVarName = InvalidAtom );

    const char *GetString( Atom nVarName = InvalidAtom );
    RscId       GetRef();
    BOOL        IsDefault( Atom nVarName );
    BOOL        IsConsistent( RscInconsList * pList );

    Atom        GetClassEnum( Atom nVarName, sal_uInt32 nPos );
    Atom        GetClassEnum( sal_uInt32 nPos );
    RscTop *    GetClassType(){ return aInst.pClass; };
    Atom        GetClassName();
    void        EnumClassVariables( void * pData, VarEnumCallbackProc ) const;
    ERRTYPE     WriteRc( RscWriteRc & aMem );
};

class RscInstCopy : public RscInst {
    void        MakeCopy( RSCINST aCopyInst );
public:
                RscInstCopy();
                RscInstCopy( const RscInstCopy & rInst );
                RscInstCopy( const RscInst & rInst );
                RscInstCopy( RSCINST aTmpI );
                RscInstCopy( RscTop * pNewType, const RscInst & rInst );
    RscInstCopy&    operator =  ( const RscInstCopy & rRscInst );
    RscInstCopy&    operator =  ( const RscInst & rRscInst );
                ~RscInstCopy();
};

class RscDataBase
{
friend class RscHrc;
friend class RscSrc;
friend class RscInst;

    RscCmdLine  * pCmdLine;
    RscTypCont  * pTC;
    LanguageType   nLangType;
public:
                RscDataBase( RscError * );
                ~RscDataBase();

//  void        SetLanguage( LanguageType nTyp ) { nLangType = nTyp; }
    void        SetLanguage( Atom nId );
    Atom        GetLanguage() const;

    ByteString  GetPath() const;
    void        SetPath( const ByteString & rPath );
                // Konvertiert einen Namen in einen Typ
    RscTop*     GetClassType( Atom nClassName );
                // Instanz einer Klasse erzeugen
    BOOL        MakeConsistent( RscInconsList * pList );
                // Array mit Dateinamen
    RscFileTab* GetFileTab();
                // Eine Dateinamen-Instanz holen
    RscFile *   GetFileStruct( ULONG lKey );

    ULONG       AddSrcFile( const ByteString & rFileName );
    ULONG       AddHrcFile( const ByteString & rFileName );
                // Traegt die Include-Dateien in die Abhaengigkeitsliste
                // von lFileKey ein
    void        ScanForIncFiles( ULONG lFileKey );
    void        RemoveFile( ULONG lKey );

                // Suche ueber alle DEFINES
    RscDefine * FindDef( const ByteString & rName );
    ULONG       GetFileKey( const ByteString & rFileName );
};

class RscHrc
{
protected:
    ULONG         lFileKey; // Index der Instanz
    RscDataBase * pDataBase;// Datenbasis
public:

                // Kompilerinstanz erzeugen
                RscHrc( RscDataBase * pDBase, ULONG lKey );
                ~RscHrc();

                // Daten von Datei uebernehmen
    ERRTYPE     ReadFile();

    ULONG       GetFileKey() const { return lFileKey; }

    void        SetDirty( BOOL bSet );
    BOOL        IsDirty();
    void        SetPathName( const ByteString & );
    ByteString  GetPathName();
    void        SetFileName( const ByteString & );
    ByteString  GetFileName();

                //Depend-Datei anhaengen
    void        InsertDepend( ULONG lKey, ULONG lPos );

                // DefineList holen
    RscDefineList * GetDefineList();
                // Suche ueber all DEFINES im Zugriff
    RscDefine * FindDef( const ByteString & rName );
    ERRTYPE     NewDef( const ByteString & rMacroName, INT32 lValue,
                        ULONG lPos );
    ERRTYPE     NewDef( const ByteString & rMacroName, const ByteString & rMacro,
                        ULONG lPos );
    ERRTYPE     ChangeDef( const ByteString & rMacroName, INT32 lValue );
    ERRTYPE     ChangeDef( const ByteString & rMacroName,
                           const ByteString & rMacro );
    BOOL        IsDefUsed( const ByteString & );
    void        DeleteDef( const ByteString & rMacroName );
    ERRTYPE     ChangeDefName( const ByteString & rMacroName,
                               const ByteString & rNewName );

                // Dateinamen-Instanz holen
    RscFile *   GetFileStruct();
                //Abhaengigkeitsliste holen holen
    RscDependList *   GetDependList();

                // Datei schreiben
    ERRTYPE     WriteFile();
};

class RscSrc : public RscHrc {
public:
            // Kompilerinstanz erzeugen
            RscSrc( RscDataBase * pDBase, ULONG lKey );
            ~RscSrc();


    RscInstCopy CreateRscInst( RscTop * pClass );
                // Instanz loeschen
    ERRTYPE     DeleteRscInst( RscTop * pClass, const RscId & rInstName );
                // Datenbasis nach Define Veraenderung wiederherstellen
    //Instanz global zur Verfuegung stellen
    ERRTYPE SetRscInst( const RscId & rInstName, RscInstCopy & );
    //Instanz holen
    RscInst GetRscInst( RscTop* pClass, const RscId & rInstName );

    // Namen und Identifier Liste fuellen
    void    FillNameIdList( REResourceList * pList );

            // C++ Resourcekonstruktor schreiben
    ERRTYPE WriteCxxFile( const ByteString &, const ByteString & rHxxName );
    ERRTYPE WriteHxxFile( const ByteString & );     // C++ Klasskopf schreiben
};

#endif //_RSCINST_HXX
