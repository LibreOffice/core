/*************************************************************************
 *
 *  $RCSfile: rscdef.hxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rscdef.hxx,v 1.1.1.1 2000-09-18 16:42:54 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.10  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.9  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.8  2000/07/11 17:00:08  th
    Unicode

    Revision 1.7  1997/08/27 18:18:14  MM
    neue Headerstruktur

**************************************************************************/

#ifndef _RSCDEF_HXX
#define _RSCDEF_HXX

#ifndef _TOOLS_UNQIDX_HXX
#include <tools/unqidx.hxx>
#endif
#ifndef _RSCTREE_HXX
#include <rsctree.hxx>
#endif

/****************** C L A S S E S ****************************************/
class RscExpression;
class RscFileTab;
class RscDefine;

/*********** R s c E x p r e s s i o n ***********************************/
#define RSCEXP_LONG     0
#define RSCEXP_EXP      1
#define RSCEXP_DEF      2
#define RSCEXP_NOTHING  3

class RscExpType
{
public:
    union {
        RscExpression * pExp;
        RscDefine     * pDef;
        struct {
             short           nHi;
             USHORT          nLo;
        } aLong;
    } aExp;
    char cType;
    char cUnused;
    BOOL IsNumber()    const { return( RSCEXP_LONG     == cType ); }
    BOOL IsExpression()const { return( RSCEXP_EXP      == cType ); }
    BOOL IsDefinition()const { return( RSCEXP_DEF      == cType ); }
    BOOL IsNothing()   const { return( RSCEXP_NOTHING  == cType ); }
    void SetLong( long lValue ){
            aExp.aLong.nHi = (short)(lValue >> 16);
            aExp.aLong.nLo = (USHORT)lValue;
            cType = RSCEXP_LONG;
         }
    long GetLong() const{
             return aExp.aLong.nLo |
                    ((long)aExp.aLong.nHi << 16);
         }
    BOOL Evaluate( long * pValue ) const;
    void GetMacro( ByteString & ) const;
};

/*********** R s c I d ***************************************************/
class RscId
{
    static      BOOL bNames;// FALSE, bei den Namenoperation nur Zahlen
public:
    RscExpType  aExp;       // Zahl, Define oder Ausdruck
    long    GetNumber() const;
    void    Create( const RscExpType & rExpType );
    void    Create(){ aExp.cType = RSCEXP_NOTHING; }

            RscId() { Create();                         }

            RscId( RscDefine * pEle );
            RscId( long lNumber )
                    { aExp.SetLong( lNumber );          }

            RscId( const RscExpType & rExpType )
                    { Create( rExpType );               }

            void Destroy();

            ~RscId(){
                Destroy();
            }

            RscId( const RscId& rRscId );

            RscId& operator = ( const RscId& rRscId );

    static BOOL    IsSetNames();
    static void    SetNames( BOOL bSet = TRUE );
    operator       long() const;   // Gibt Nummer zurueck
    ByteString     GetName()  const;   // Gibt den Namen des Defines zurueck
    ByteString     GetMacro()  const;  // Gibt das Macro zurueck
    BOOL    operator <  ( const RscId& rRscId ) const;
    BOOL    operator >  ( const RscId& rRscId ) const;
    BOOL    operator == ( const RscId& rRscId ) const;
    BOOL    operator <=  ( const RscId& rRscId ) const
            { return !(operator > ( rRscId )); }
    BOOL    operator >= ( const RscId& rRscId ) const
            { return !(operator < ( rRscId )); }
    BOOL    IsId() const { return !aExp.IsNothing(); }
};

/*********** R s c D e f i n e *******************************************/
class RscDefine : public StringNode
{
friend class RscFileTab;
friend class RscDefineList;
friend class RscDefTree;
friend class RscExpression;
friend class RscId;
    ULONG           lFileKey;   // zu welcher Datei gehoert das Define
    USHORT          nRefCount;  // Wieviele Referenzen auf dieses Objekt
    long            lId;        // Identifier
    RscExpression * pExp;       // Ausdruck
protected:

            RscDefine( ULONG lFileKey, const ByteString & rDefName,
                       long lDefId );
            RscDefine( ULONG lFileKey, const ByteString & rDefName,
                       RscExpression * pExpression );
            ~RscDefine();
    void    IncRef(){ nRefCount++; }
    USHORT  GetRefCount() const    { return nRefCount; }
    void    DecRef();
    void    DefineToNumber();
    void    SetName( const ByteString & rNewName ){ aName = rNewName; }
    void    ChangeMacro( RscExpression * pExpression );
    void    ChangeMacro( long lIdentifier );
public:
    RscDefine * Search( const char * );
    ULONG       GetFileKey() const { return lFileKey; }
    BOOL        Evaluate();
    long        GetNumber() const  { return lId;      }
    ByteString  GetMacro();
};

DECLARE_LIST( RscSubDefList, RscDefine * )

class RscDefineList : public RscSubDefList {
friend class RscFile;
friend class RscFileTab;
private:
                // pExpression wird auf jedenfall Eigentum der Liste
    RscDefine * New( ULONG lFileKey, const ByteString & rDefName,
                     long lDefId, ULONG lPos );
    RscDefine * New( ULONG lFileKey, const ByteString & rDefName,
                     RscExpression * pExpression, ULONG lPos );
    BOOL        Befor( const RscDefine * pFree, const RscDefine * pDepend );
    BOOL        Remove( RscDefine * pDef );
    BOOL        Remove( ULONG nIndex );
    BOOL        Remove();
public:
    void        WriteAll( FILE * fOutput );
};

/*********** R s c E x p r e s s i o n ***********************************/
class RscExpression {
friend class RscFileTab;
    char        cOperation;
    RscExpType  aLeftExp;
    RscExpType  aRightExp;
public:
                RscExpression( RscExpType aLE, char cOp,
                               RscExpType aRE );
                ~RscExpression();
    BOOL        Evaluate( long * pValue );
    ByteString  GetMacro();
};

/********************** R S C F I L E ************************************/
class RscDepend {
    ULONG            lKey;
public:
            RscDepend( ULONG lIncKey ){ lKey = lIncKey; };
    ULONG   GetFileKey(){ return lKey; }
};
DECLARE_LIST( RscDependList, RscDepend * )

// Tabelle die alle Dateinamen enthaelt
class RscFile : public RscDependList
{
friend class RscFileTab;
    BOOL            bIncFile;   // Ist es eine Include-Datei
public:
    BOOL            bLoaded;    // Ist die Datei geladen
    BOOL            bScanned;   // Wurde Datei nach Inclide abgesucht
    BOOL            bDirty;     // Dirty-Flag
    ByteString      aFileName;  // Name der Datei
    ByteString      aPathName;  // Pfad und Name der Datei
    RscDefineList   aDefLst;    // Liste der Defines

                    RscFile();
                    ~RscFile();
    BOOL            InsertDependFile( ULONG lDepFile, ULONG lPos );
    void            RemoveDependFile( ULONG lDepFile );
    BOOL            Depend( ULONG lDepend, ULONG lFree );
    void            SetIncFlag(){ bIncFile = TRUE; };
    BOOL            IsIncFile(){  return bIncFile; };
};

DECLARE_UNIQUEINDEX( RscSubFileTab, RscFile * )
#define NOFILE_INDEX UNIQUEINDEX_ENTRY_NOTFOUND

class RscDefTree {
    RscDefine * pDefRoot;
public:
    static BOOL Evaluate( RscDefine * pDef );
                RscDefTree(){ pDefRoot = NULL; }
                ~RscDefTree();
    void        Remove();
    BOOL        Evaluate();
    RscDefine * Search( const char * pName );
    void        Insert( RscDefine * pDef );
    void        Remove( RscDefine * pDef );
};

class RscFileTab : public RscSubFileTab {
    RscDefTree aDefTree;
    ULONG       Find( const ByteString & rName );
public:
                RscFileTab();
                ~RscFileTab();

    RscDefine * FindDef( const char * );
    RscDefine * FindDef( const ByteString& rStr ) { return FindDef( rStr.GetBuffer() ); }
    RscDefine * FindDef( ULONG lKey, const ByteString & );

    BOOL        Depend( ULONG lDepend, ULONG lFree );
    BOOL        TestDef( ULONG lFileKey, ULONG lPos,
                         const RscDefine * pDefDec );
    BOOL        TestDef( ULONG lFileKey, ULONG lPos,
                         const RscExpression * pExpDec );

    RscDefine * NewDef( ULONG lKey, const ByteString & rDefName,
                        long lId, ULONG lPos );
    RscDefine * NewDef( ULONG lKey, const ByteString & rDefName,
                        RscExpression *, ULONG lPos );

    BOOL        ChangeDef( const ByteString & rDefName, long lId );
    BOOL        ChangeDef( const ByteString & rDefName, RscExpression * );

    BOOL IsDefUsed( const ByteString & );
    void DeleteDef( const ByteString & );
    BOOL ChangeDefName( const ByteString & rDefName,
                        const ByteString & rNewName );

           // Alle Defines die in dieser Datei Definiert sind loeschen
    void   DeleteFileContext( ULONG lKey );
    void   DeleteFile( ULONG lKey );
    ULONG  NewCodeFile( const ByteString & rName );
    ULONG  NewIncFile( const ByteString & rName, const ByteString & rPath );
    RscFile * GetFile( ULONG lFileKey ){ return Get( lFileKey ); }
};

#endif // _RSCDEF_HXX
