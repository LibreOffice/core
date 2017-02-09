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
        return aExp.aLong.nLo | ((sal_uInt32)aExp.aLong.nHi << 16);
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

typedef ::std::vector< RscDefine* > RscSubDefList;

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
class RscDepend;

typedef ::std::vector< RscDepend* > RscDependList;

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

class RscFile;

class RscFileTab : public UniqueIndex<RscFile>
{
public:
    using UniqueIndex<RscFile>::Index;
    using UniqueIndex<RscFile>::IndexNotFound;

private:
    RscDefTree aDefTree;
    Index       Find(const OString& rName);
public:
                RscFileTab();
                ~RscFileTab();

    RscDefine * FindDef( const char * );
    RscDefine * FindDef(const OString& rStr)
                {
                    return FindDef(rStr.getStr());
                }

    bool        Depend( Index lDepend, Index lFree );
    bool        TestDef( Index lFileKey, size_t lPos,
                         const RscDefine * pDefDec );
    bool        TestDef( Index lFileKey, size_t lPos,
                         const RscExpression * pExpDec );

    RscDefine * NewDef( Index lKey, const OString& rDefName,
                        sal_Int32 lId );
    RscDefine * NewDef( Index lKey, const OString& rDefName,
                        RscExpression * );

           // deletes all defines defined in this file
    void        DeleteFileContext( Index lKey );
    Index       NewCodeFile(const OString& rName);
    Index       NewIncFile(const OString& rName, const OString& rPath);
    RscFile *   GetFile( Index lFileKey ){ return Get( lFileKey ); }
};

class RscDepend
{
    RscFileTab::Index   lKey;
public:
                RscDepend( RscFileTab::Index lIncKey ){ lKey = lIncKey; };
    RscFileTab::Index   GetFileKey(){ return lKey; }
};

class RscDefineList
{
friend class RscFile;
friend class RscFileTab;
private:
    RscSubDefList   maList;
                // pExpression always belongs to the list
    RscDefine * New( RscFileTab::Index lFileKey, const OString& rDefName,
                     sal_Int32 lDefId, size_t lPos );
    RscDefine * New( RscFileTab::Index lFileKey, const OString& rDefName,
                     RscExpression * pExpression, size_t lPos );
    bool        Remove();
    size_t      GetPos( RscDefine* item )
                    {
                        for ( size_t i = 0, n = maList.size(); i < n; ++i )
                            if ( maList[ i ] == item )
                                return i;
                        return size_t(-1);
                    }
};

// table containing all file names
class RscFile
{
friend class RscFileTab;
    bool            bIncFile;   // whether it is an include file
public:
    bool            bLoaded;    // whether the file is loaded
    bool            bScanned;   // whether the file searches for include
    OString         aFileName;  // file name
    OString         aPathName;  // file path and name
    RscDefineList   aDefLst;    // list of defines
    RscDependList   aDepLst;    // list of depend

                    RscFile();
                    ~RscFile();
    void            InsertDependFile( RscFileTab::Index lDepFile );
    bool            Depend( RscFileTab::Index lDepend, RscFileTab::Index lFree );
    void            SetIncFlag(){ bIncFile = true; };
    bool            IsIncFile(){  return bIncFile; };
};

/*********** R s c D e f i n e *******************************************/
class RscDefine : public NameNode
{
friend class RscFileTab;
friend class RscDefineList;
friend class RscDefTree;
friend class RscExpression;
friend class RscId;

    RscFileTab::Index lFileKey;   // file the define belongs to
    sal_uInt32  nRefCount;  // reference count to this object
    sal_Int32   lId;        // identifier
    RscExpression * pExp;   // expression
    OString     m_aName;

    virtual COMPARE Compare( const NameNode * ) const override;
    virtual COMPARE Compare( const void * ) const override;

protected:

    RscDefine( RscFileTab::Index lFileKey, const OString& rDefName,
                           sal_Int32 lDefId );
    RscDefine( RscFileTab::Index lFileKey, const OString& rDefName,
                           RscExpression * pExpression );
    virtual ~RscDefine() override;

    void          IncRef() { nRefCount++; }
    void          DecRef();
    void          DefineToNumber();

public:
    RscFileTab::Index GetFileKey() const { return lFileKey; }
    void           Evaluate();
    sal_Int32      GetNumber() const  { return lId;      }
    RscDefine*     Search( const char * ) const;
    const OString& GetName() const { return m_aName; }
};


#endif // INCLUDED_RSC_INC_RSCDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
