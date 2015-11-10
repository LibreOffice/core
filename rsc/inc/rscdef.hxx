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
#ifndef INCLUDED_RSC_INC_RSCDEF_HXX
#define INCLUDED_RSC_INC_RSCDEF_HXX

#include <sal/config.h>

#include <tools/solar.h>
#include <tools/unqidx.hxx>
#include <rsctree.hxx>
#include <rtl/strbuf.hxx>
#include <vector>

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
    union
    {
        RscExpression * pExp;
        RscDefine     * pDef;
        struct
        {
             short           nHi;
             unsigned short  nLo;
        } aLong;
    } aExp;
    char cType;
    bool cUnused;
    bool IsNumber()    const { return( RSCEXP_LONG     == cType ); }
    bool IsExpression()const { return( RSCEXP_EXP      == cType ); }
    bool IsDefinition()const { return( RSCEXP_DEF      == cType ); }
    bool IsNothing()   const { return( RSCEXP_NOTHING  == cType ); }
    void SetLong( sal_Int32 lValue )
             {
                 aExp.aLong.nHi = (short)(lValue >> 16);
                 aExp.aLong.nLo = (unsigned short)lValue;
                 cType = RSCEXP_LONG;
             }
    sal_Int32 GetLong() const
                  {
                      return aExp.aLong.nLo |
                          ((sal_uInt32)aExp.aLong.nHi << 16);
                  }
    bool Evaluate( sal_Int32 * pValue ) const;
    void AppendMacro( OStringBuffer & ) const;
};

/*********** R s c I d ***************************************************/
class RscId
{
    static      bool bNames;// if false, only count name operation
public:
    RscExpType  aExp;       // number, define or expression
    sal_Int32   GetNumber() const;
    void    Create( const RscExpType & rExpType );
    void    Create(){ aExp.cType = RSCEXP_NOTHING; }

            RscId() { Create();                         }

            RscId( RscDefine * pEle );
            RscId( sal_Int32 lNumber )
            {
                aExp.cUnused = false;
                aExp.SetLong( lNumber );
            }

            RscId( const RscExpType & rExpType )
                    { Create( rExpType );               }

            void Destroy();

            ~RscId() { Destroy(); }

            RscId( const RscId& rRscId );

            RscId& operator = ( const RscId& rRscId );

    static void SetNames( bool bSet = true );
    operator sal_Int32() const;   // returns the number
    OString GetName()  const;   // returns the define
    bool    operator <  ( const RscId& rRscId ) const;
    bool    operator >  ( const RscId& rRscId ) const;
    bool    operator == ( const RscId& rRscId ) const;
    bool    operator <=  ( const RscId& rRscId ) const
                { return !(operator > ( rRscId )); }
    bool    operator >= ( const RscId& rRscId ) const
                { return !(operator < ( rRscId )); }
    bool    IsId() const { return !aExp.IsNothing(); }
};

/*********** R s c D e f i n e *******************************************/
class RscDefine : public StringNode
{
friend class RscFileTab;
friend class RscDefineList;
friend class RscDefTree;
friend class RscExpression;
friend class RscId;
    sal_uLong   lFileKey;   // file the define belongs to
    sal_uInt32  nRefCount;  // reference count to this object
    sal_Int32   lId;        // identifier
    RscExpression * pExp;       // expression
protected:

                RscDefine( sal_uLong lFileKey, const OString& rDefName,
                           sal_Int32 lDefId );
                RscDefine( sal_uLong lFileKey, const OString& rDefName,
                           RscExpression * pExpression );
                virtual ~RscDefine();
    void        IncRef(){ nRefCount++; }
    void        DecRef();
    void        DefineToNumber();
    void        SetName(const OString& rNewName) { m_aName = rNewName; }

    using StringNode::Search;
public:
    RscDefine * Search( const char * );
    sal_uLong   GetFileKey() const { return lFileKey; }
    bool        Evaluate();
    sal_Int32   GetNumber() const  { return lId;      }
    OString     GetMacro();
};

typedef ::std::vector< RscDefine* > RscSubDefList;

class RscDefineList
{
friend class RscFile;
friend class RscFileTab;
private:
    RscSubDefList   maList;
                // pExpression always belongs to the list
    RscDefine * New( sal_uLong lFileKey, const OString& rDefName,
                     sal_Int32 lDefId, size_t lPos );
    RscDefine * New( sal_uLong lFileKey, const OString& rDefName,
                     RscExpression * pExpression, size_t lPos );
    bool        Remove();
    size_t      GetPos( RscDefine* item )
                    {
                        for ( size_t i = 0, n = maList.size(); i < n; ++i )
                            if ( maList[ i ] == item )
                                return i;
                        return size_t(-1);
                    }
public:
    void        WriteAll( FILE * fOutput );
};

/*********** R s c E x p r e s s i o n ***********************************/
class RscExpression
{
friend class RscFileTab;
    char        cOperation;
    RscExpType  aLeftExp;
    RscExpType  aRightExp;
public:
                RscExpression( RscExpType aLE, char cOp,
                               RscExpType aRE );
                ~RscExpression();
    bool        Evaluate( sal_Int32 * pValue );
    OString     GetMacro();
};

/********************** R S C F I L E ************************************/
class RscDepend
{
    sal_uLong   lKey;
public:
                RscDepend( sal_uLong lIncKey ){ lKey = lIncKey; };
    sal_uLong   GetFileKey(){ return lKey; }
};

typedef ::std::vector< RscDepend* > RscDependList;

// table containing al file names
class RscFile
{
friend class RscFileTab;
    bool            bIncFile;   // whether it is an include file
public:
    bool            bLoaded;    // whether the file is loaded
    bool            bScanned;   // whether the file searches for include
    bool            bDirty;     // dirty-flag
    OString         aFileName;  // file name
    OString         aPathName;  // file path and name
    RscDefineList   aDefLst;    // list of defines
    RscDependList   aDepLst;    // list of depend

                    RscFile();
                    ~RscFile();
    bool            InsertDependFile( sal_uLong lDepFile, size_t lPos );
    bool            Depend( sal_uLong lDepend, sal_uLong lFree );
    void            SetIncFlag(){ bIncFile = true; };
    bool            IsIncFile(){  return bIncFile; };
};

#define NOFILE_INDEX UNIQUEINDEX_ENTRY_NOTFOUND

class RscDefTree
{
    RscDefine * pDefRoot;
public:
    static bool Evaluate( RscDefine * pDef );
                RscDefTree(){ pDefRoot = nullptr; }
                ~RscDefTree();
    void        Remove();
    RscDefine * Search( const char * pName );
    void        Insert( RscDefine * pDef );
    void        Remove( RscDefine * pDef );
};

class RscFileTab : public UniqueIndex<RscFile>
{
    RscDefTree aDefTree;
    sal_uLong       Find(const OString& rName);
public:
                RscFileTab();
                ~RscFileTab();

    RscDefine * FindDef( const char * );
    RscDefine * FindDef(const OString& rStr)
                {
                    return FindDef(rStr.getStr());
                }

    bool        Depend( sal_uLong lDepend, sal_uLong lFree );
    bool        TestDef( sal_uLong lFileKey, size_t lPos,
                         const RscDefine * pDefDec );
    bool        TestDef( sal_uLong lFileKey, size_t lPos,
                         const RscExpression * pExpDec );

    RscDefine * NewDef( sal_uLong lKey, const OString& rDefName,
                        sal_Int32 lId, sal_uLong lPos );
    RscDefine * NewDef( sal_uLong lKey, const OString& rDefName,
                        RscExpression *, sal_uLong lPos );

           // deletes all defines defined in this file
    void        DeleteFileContext( sal_uLong lKey );
    sal_uLong   NewCodeFile(const OString& rName);
    sal_uLong   NewIncFile(const OString& rName, const OString& rPath);
    RscFile *   GetFile( sal_uLong lFileKey ){ return Get( lFileKey ); }
};

#endif // INCLUDED_RSC_INC_RSCDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
