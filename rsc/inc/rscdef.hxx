/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rscdef.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _RSCDEF_HXX
#define _RSCDEF_HXX

#ifndef _TOOLS_UNQIDX_HXX
#include <tools/unqidx.hxx>
#endif
#include <rsctree.hxx>

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
             unsigned short  nLo;
        } aLong;
    } aExp;
    char cType;
    char cUnused;
    BOOL IsNumber()    const { return( RSCEXP_LONG     == cType ); }
    BOOL IsExpression()const { return( RSCEXP_EXP      == cType ); }
    BOOL IsDefinition()const { return( RSCEXP_DEF      == cType ); }
    BOOL IsNothing()   const { return( RSCEXP_NOTHING  == cType ); }
    void SetLong( INT32 lValue ){
            aExp.aLong.nHi = (short)(lValue >> 16);
            aExp.aLong.nLo = (unsigned short)lValue;
            cType = RSCEXP_LONG;
         }
    INT32 GetLong() const{
             return aExp.aLong.nLo |
                    ((INT32)aExp.aLong.nHi << 16);
         }
    BOOL Evaluate( INT32 * pValue ) const;
    void GetMacro( ByteString & ) const;
};

/*********** R s c I d ***************************************************/
class RscId
{
    static      BOOL bNames;// FALSE, bei den Namenoperation nur Zahlen
public:
    RscExpType  aExp;       // Zahl, Define oder Ausdruck
    INT32   GetNumber() const;
    void    Create( const RscExpType & rExpType );
    void    Create(){ aExp.cType = RSCEXP_NOTHING; }

            RscId() { Create();                         }

            RscId( RscDefine * pEle );
            RscId( INT32 lNumber )
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
    operator         INT32() const;   // Gibt Nummer zurueck
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
    sal_uInt32          nRefCount;  // Wieviele Referenzen auf dieses Objekt
    INT32           lId;        // Identifier
    RscExpression * pExp;       // Ausdruck
protected:

            RscDefine( ULONG lFileKey, const ByteString & rDefName,
                       INT32 lDefId );
            RscDefine( ULONG lFileKey, const ByteString & rDefName,
                       RscExpression * pExpression );
            ~RscDefine();
    void    IncRef(){ nRefCount++; }
    sal_uInt32  GetRefCount() const    { return nRefCount; }
    void    DecRef();
    void    DefineToNumber();
    void    SetName( const ByteString & rNewName ){ aName = rNewName; }
    void    ChangeMacro( RscExpression * pExpression );
    void    ChangeMacro( INT32 lIdentifier );

    using StringNode::Search;
public:
    RscDefine * Search( const char * );
    ULONG       GetFileKey() const { return lFileKey; }
    BOOL        Evaluate();
    INT32       GetNumber() const  { return lId;      }
    ByteString  GetMacro();
};

DECLARE_LIST( RscSubDefList, RscDefine * )

class RscDefineList : public RscSubDefList {
friend class RscFile;
friend class RscFileTab;
private:
                // pExpression wird auf jedenfall Eigentum der Liste
    RscDefine * New( ULONG lFileKey, const ByteString & rDefName,
                     INT32 lDefId, ULONG lPos );
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
    BOOL        Evaluate( INT32 * pValue );
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
                        INT32 lId, ULONG lPos );
    RscDefine * NewDef( ULONG lKey, const ByteString & rDefName,
                        RscExpression *, ULONG lPos );

    BOOL        ChangeDef( const ByteString & rDefName, INT32 lId );
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
