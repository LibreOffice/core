/*************************************************************************
 *
 *  $RCSfile: rscinst.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:54 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscinst.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.14  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.13  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.12  2000/07/11 17:00:28  th
    Unicode

    Revision 1.11  1999/09/07 13:30:02  mm
    UniCode

    Revision 1.10  1997/08/27 18:18:06  MM
    neue Headerstruktur

**************************************************************************/

#ifndef _RSCINST_HXX
#define _RSCINST_HXX

#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCERROR_H
#include <rscerror.h>
#endif
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

/******************* C l a s s e s   F o r w a r d s *********************/
class RscTypCont;
class RscCmdLine;
class REResourceList;
class RscTop;
class RscInstCopy;

/******************* F u n c t i o n   F o r w a r d s *******************/
HASHID HashId( const char * );        // Gibt zu einem String eine HashId
const char * GetHashString( HASHID ); // Gibt zu einer HASHID einen String
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
    ERRTYPE     SetPosEle( USHORT nPos, RscInstCopy & rInst );
    ERRTYPE     SetPosRscId( USHORT nPos, const RscId & rId );
    SUBINFO_STRUCT  GetInfoEle( USHORT nPos );
    USHORT      GetCount();
    RscInst     GetElement( RscTop * pClass, const RscId & rName );
    RscInst     GetPosEle( USHORT nPos );
    ERRTYPE     MovePosEle( USHORT nDestPos, USHORT nSourcePos );
    ERRTYPE     DeleteElement( RscTop * pClass, const RscId & rName );
    ERRTYPE     DeletePosEle( USHORT nPos );

    ERRTYPE     SetVar( HASHID nVarName, RscInstCopy & rInst );
    ERRTYPE     SetConst( HASHID nVarName, HASHID nConstId );
    ERRTYPE     SetBool( HASHID nVarName, BOOL );

    // Hack fuer X, Y, Width, Height
    static ERRTYPE SetCorrectValues( RSCINST & rInst, RSCINST & rVarInst,
                                    long lValue, USHORT nTupelIdx );
    ERRTYPE     SetNumber( HASHID nVarName, long );

    ERRTYPE     SetString( HASHID nVarName, const char * );
    ERRTYPE     SetConst( HASHID nConstId );
    ERRTYPE     SetBool( BOOL );
    ERRTYPE     SetNumber( long );
    ERRTYPE     SetString( const char * );
    ERRTYPE     SetRef( const RscId & rRscId );
    ERRTYPE     SetDefault( HASHID nVarName );

    RscInst     GetVar( HASHID nVarName );
    HASHID      GetConst( HASHID nVarName = HASH_NONAME );
    USHORT      GetConstPos( HASHID nVarName = HASH_NONAME );
    BOOL        GetBool( HASHID nVarName = HASH_NONAME );

    // Hack fuer X, Y, Width, Height
    static long GetCorrectValues( RSCINST & rInst, RSCINST & rVarInst,
                                    USHORT nTupelIdx );
    long        GetNumber( HASHID nVarName = HASH_NONAME );

    const char *GetString( HASHID nVarName = HASH_NONAME );
    RscId       GetRef();
    BOOL        IsDefault( HASHID nVarName );
    BOOL        IsConsistent( RscInconsList * pList );

    HASHID      GetClassEnum( HASHID nVarName, USHORT nPos );
    HASHID      GetClassEnum( USHORT nPos );
    RscTop *    GetClassType(){ return aInst.pClass; };
    HASHID      GetClassName();
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
    void        SetLanguage( HASHID nId );
    HASHID      GetLanguage() const;

    ByteString  GetPath() const;
    void        SetPath( const ByteString & rPath );
                // Konvertiert einen Namen in einen Typ
    RscTop*     GetClassType( HASHID nClassName );
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
    ERRTYPE     NewDef( const ByteString & rMacroName, long lValue,
                        ULONG lPos );
    ERRTYPE     NewDef( const ByteString & rMacroName, const ByteString & rMacro,
                        ULONG lPos );
    ERRTYPE     ChangeDef( const ByteString & rMacroName, long lValue );
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
