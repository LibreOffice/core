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


// overall program includes
#include <rscdef.hxx>

#include <limits.h>

bool RscId::bNames = true;

void RscId::SetNames( bool bSet )
{
    bNames = bSet;
}

sal_Int32 RscId::GetNumber() const
{
    sal_Int32 lVal;
    aExp.Evaluate( &lVal );
    return lVal;
}

void RscId::Create( const RscExpType & rExpType )
{
    aExp = rExpType;
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->IncRef();
    else if( aExp.IsExpression() )
    {
        sal_Int32 lValue;

        aExp.Evaluate( &lValue );
        aExp.SetLong( lValue );
    }
}

void RscId::Destroy()
{
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->DecRef();
    aExp.cType = RSCEXP_NOTHING;
}

RscId::RscId( const RscId& rRscId )
{
    aExp = rRscId.aExp;
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->IncRef();
}

RscId::RscId( RscDefine * pDef )
{
    RscExpType aExpType;

    aExpType.aExp.pDef = pDef;
    aExpType.cType = RSCEXP_DEF;
    aExpType.cUnused = false;
    Create( aExpType );
}

RscId& RscId::operator = ( const RscId& rRscId )
{
    if( rRscId.aExp.IsDefinition() )
        rRscId.aExp.aExp.pDef->IncRef();
    Destroy();
    aExp = rRscId.aExp;
    return *this;
}

bool RscId::operator == ( const RscId& rRscId ) const
{
    return GetNumber() == rRscId.GetNumber();
}

bool RscId::operator < ( const RscId& rRscId ) const
{
    return GetNumber() < rRscId.GetNumber();
}

bool RscId::operator > ( const RscId& rRscId ) const
{
    return GetNumber() > rRscId.GetNumber();
}

OString RscId::GetName() const
{
    OStringBuffer aStr;

    if ( !aExp.IsNothing() )
    {
        if( bNames )
            aExp.AppendMacro(aStr);
        else
            aStr.append(GetNumber());
    }

    return aStr.makeStringAndClear();
}

RscDefine::RscDefine( RscFileTab::Index lKey, const OString& rDefName, sal_Int32 lDefId )
    : m_aName( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    lId       = lDefId;
    pExp      = nullptr;
}

RscDefine::RscDefine( RscFileTab::Index lKey, const OString& rDefName,
                      RscExpression * pExpression  )
    : lId(0), m_aName( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    pExpression->Evaluate( &lId );
    pExp      = pExpression;
}

RscDefine::~RscDefine()
{
    delete pExp;
    if( nRefCount )
        RscExit( 14 );
}

void RscDefine::DecRef()
{
    nRefCount--;
    if( 0 == nRefCount )
    {
        delete this;
    }
}

void RscDefine::DefineToNumber()
{
    delete pExp;
    pExp = nullptr;
    m_aName = OString::number(lId);
}

void RscDefine::Evaluate()
{
    if( pExp )
        pExp->Evaluate( &lId );
}

RscDefine * RscDefine::Search( const char * pSearch ) const
{
    return static_cast<RscDefine *>(NameNode::Search( static_cast<const void *>(pSearch) ));
}

COMPARE RscDefine::Compare( const NameNode * pSearch ) const
{
    int nCmp = strcmp( m_aName.getStr(),
                       static_cast<const RscDefine *>(pSearch)->m_aName.getStr() );
    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

// pSearch is a pointer to const char *
COMPARE RscDefine::Compare( const void * pSearch ) const
{
    int nCmp = strcmp( m_aName.getStr(), static_cast<const char *>(pSearch) );

    if( nCmp < 0 )
        return LESS;
    else if( nCmp > 0 )
        return GREATER;
    else
        return EQUAL;
}

RscDefine * RscDefineList::New( RscFileTab::Index lFileKey, const OString& rDefName,
                                sal_Int32 lDefId, size_t lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, lDefId );
    pDef->IncRef();
    if ( lPos < maList.size() )
    {
        RscSubDefList::iterator it = maList.begin();
        ::std::advance( it, lPos );
        maList.insert( it, pDef );
    }
    else
    {
        maList.push_back( pDef );
    }
    return pDef;
}

RscDefine * RscDefineList::New( RscFileTab::Index lFileKey, const OString& rDefName,
                                RscExpression * pExpression, size_t lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, pExpression );
    pDef->IncRef();
    if ( lPos < maList.size() )
    {
        RscSubDefList::iterator it = maList.begin();
        ::std::advance( it, lPos );
        maList.insert( it, pDef );
    }
    else
    {
        maList.push_back( pDef );
    }
    return pDef;
}

bool RscDefineList::Remove()
{
    if ( maList.empty() )
        return false;

    maList[ 0 ]->DefineToNumber();
    maList[ 0 ]->DecRef();
    maList.erase( maList.begin() );
    return true;
}

bool RscExpType::Evaluate( sal_Int32 * plValue ) const
{
    if( IsDefinition() )
    {
        aExp.pDef->Evaluate();
        // ignore potential errors
        *plValue = aExp.pDef->GetNumber();
    }
    else if( IsExpression() )
        return aExp.pExp->Evaluate( plValue );
    else if( IsNothing() )
        *plValue = 0;
    else
        *plValue = GetLong();

    return true;
}

void RscExpType::AppendMacro(OStringBuffer& rStr) const
{
    if( IsDefinition() )
        rStr.append(aExp.pDef->GetName());
    else if( IsExpression() )
        rStr.append(aExp.pExp->GetMacro());
    else if( IsNumber() )
        rStr.append(GetLong());
}


RscExpression::RscExpression( RscExpType aLE, char cOp, RscExpType aRE )
{
    aLeftExp   = aLE;
    cOperation = cOp;
    aRightExp  = aRE;
    if( aLeftExp.IsDefinition() )
        aLeftExp.aExp.pDef->IncRef();
    if( aRightExp.IsDefinition() )
        aRightExp.aExp.pDef->IncRef();
}

RscExpression::~RscExpression()
{
    if( aLeftExp.IsDefinition() )
        aLeftExp.aExp.pDef->DecRef();
    else if( aLeftExp.IsExpression() )
        delete aLeftExp.aExp.pExp;

    if( aRightExp.IsDefinition() )
        aRightExp.aExp.pDef->DecRef();
    else if( aRightExp.IsExpression() )
        delete aRightExp.aExp.pExp;
}

bool RscExpression::Evaluate( sal_Int32 * plValue )
{
    sal_Int32 lLeft;
    sal_Int32 lRight;

    // interpret left and right branches
    if( aLeftExp.Evaluate( &lLeft ) && aRightExp.Evaluate( &lRight ) )
    {
        if( cOperation == '&' )
            *plValue = lLeft & lRight;
        else if( cOperation == '|' )
            *plValue = lLeft | lRight;
        else if( cOperation == '+' )
            *plValue = lLeft + lRight;
        else if( cOperation == '-' )
            *plValue = lLeft - lRight;
        else if( cOperation == '*' )
            *plValue = lLeft * lRight;
        else if( cOperation == 'r' )
            *plValue = lLeft >> lRight;
        else if( cOperation == 'l' )
            *plValue = lLeft << lRight;
        else
        {
            if( 0 == lRight )
                return false;
            *plValue = lLeft / lRight;
        }
        return true;
    }
    return false;
}

OString RscExpression::GetMacro()
{
    OStringBuffer aLeft;

    // output optimization
    if( aLeftExp.IsNothing() )
    {
        if ( '-' == cOperation )
        {
            aLeft.append('(');
            aLeft.append('-');
        }
        aRightExp.AppendMacro(aLeft);
        if( '-' == cOperation )
        {
            aLeft.append(')');
        }
    }
    else if( aRightExp.IsNothing() )
        aLeftExp.AppendMacro(aLeft);
    else
    {
        aLeft.append('(');
        // interpret left branch
        aLeftExp.AppendMacro(aLeft);

        aLeft.append(cOperation);

        aLeft.append('(');
        // interpret right branch
        aRightExp.AppendMacro(aLeft);
        aLeft.append(')');

        aLeft.append(')');
    }

    return aLeft.makeStringAndClear();
}

RscFile::RscFile()
{
    bLoaded  = false;
    bIncFile = false;
    bScanned = false;
}

RscFile::~RscFile()
{
    for ( size_t i = 0, n = aDepLst.size(); i < n; ++i )
        delete aDepLst[ i ];
    aDepLst.clear();

    // from back to front is better because of dependencies
    // objects are destroyed when reference counter is NULL
    while( aDefLst.Remove() ) ;
}

bool RscFile::Depend( RscFileTab::Index lDepend, RscFileTab::Index lFree )
{
    for ( size_t i = aDepLst.size(); i > 0; )
    {
        RscDepend * pDep = aDepLst[ --i ];
        if( pDep->GetFileKey() == lDepend )
        {
            for ( size_t j = i ? --i : 0; j > 0; )
            {
                pDep = aDepLst[ --j ];
                if( pDep->GetFileKey() == lFree )
                    return true;
            }
            return false;
        }
    }
    return true;
}

void RscFile::InsertDependFile( RscFileTab::Index lIncFile )
{
    for ( size_t i = 0, n = aDepLst.size(); i < n; ++i )
    {
        RscDepend* pDep = aDepLst[ i ];
        if( pDep->GetFileKey() == lIncFile )
            return;
    }

    aDepLst.push_back( new RscDepend( lIncFile ) );
}

RscDefTree::~RscDefTree()
{
    Remove();
}

void RscDefTree::Remove()
{
    while( pDefRoot )
    {
        RscDefine * pDef = pDefRoot;
        pDefRoot = static_cast<RscDefine *>(pDefRoot->Remove( pDefRoot ));
        pDef->DecRef();
    }
}

RscDefine * RscDefTree::Search( const char * pName )
{
    if( pDefRoot )
        return pDefRoot->Search( pName );
    return nullptr;
}

void RscDefTree::Insert( RscDefine * pDef )
{
    if( pDefRoot )
        pDefRoot->Insert( pDef );
    else
        pDefRoot = pDef;
    pDef->IncRef();
}

void RscDefTree::Remove( RscDefine * pDef )
{
    if( pDefRoot )
    {
        // in case pDef == pDefRoot
        pDefRoot = static_cast<RscDefine *>(pDefRoot->Remove( pDef ));
    }
    pDef->DecRef();
}

bool RscDefTree::Evaluate( RscDefine * pDef )
{
    if( pDef )
    {
        if( !Evaluate( static_cast<RscDefine *>(pDef->Left()) ) )
            return false;
        if( !Evaluate( static_cast<RscDefine *>(pDef->Right()) ) )
            return false;
    }
    return true;
}

RscFileTab::RscFileTab()
{
}

RscFileTab::~RscFileTab()
{

    aDefTree.Remove();

    Index aIndex = LastIndex();
    while( aIndex != IndexNotFound )
    {
        delete Remove( aIndex );
        aIndex = LastIndex();
    };
}

RscFileTab::Index RscFileTab::Find( const OString& rName )
{
    Index aIndex = FirstIndex();
    while( aIndex != IndexNotFound && (Get(aIndex)->aFileName != rName) )
        aIndex = NextIndex(aIndex);

    return aIndex;
}

RscDefine * RscFileTab::FindDef( const char * pName )
{
    return aDefTree.Search( pName );
}

/* This method gives back true when lDepend
   exists and is behind lFree, or when lDepend does not exist. */
bool RscFileTab::Depend( Index lDepend, Index lFree )
{
    if( lDepend == lFree )
        return true;

    Index aIndex = FirstIndex();
    while( aIndex != IndexNotFound )
    {
        RscFile * pFile = Get(aIndex);
        if( !pFile->IsIncFile() )
        {
            if( !pFile->Depend( lDepend, lFree ) )
                return false;
        }
        aIndex = NextIndex(aIndex);
    }

    return true;
}

bool RscFileTab::TestDef( Index lFileKey, size_t lPos,
                          const RscDefine * pDefDec )
{
    if( lFileKey == pDefDec->GetFileKey() )
    {
        RscFile * pFile = GetFile( pDefDec->GetFileKey() );
        if( pFile && (lPos <= pFile->aDefLst.GetPos( const_cast<RscDefine *>(pDefDec) ))
            && (lPos != ULONG_MAX ) )
        {
            return false;
        }
    }
    else if( !Depend( lFileKey, pDefDec->GetFileKey() ) )
        return false;

    return TestDef( lFileKey, lPos, pDefDec->pExp );
}

bool RscFileTab::TestDef( Index lFileKey, size_t lPos,
                          const RscExpression * pExpDec )
{
    if( !pExpDec )
        return true;

    if( pExpDec->aLeftExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pExp ) )
            return false;

    if( pExpDec->aLeftExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pDef ) )
            return false;

    if( pExpDec->aRightExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pExp ) )
            return false;

    if( pExpDec->aRightExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pDef ) )
            return false;

    return true;
}

RscDefine * RscFileTab::NewDef( Index lFileKey, const OString& rDefName,
                                sal_Int32 lId )
{
    RscDefine * pDef = FindDef( rDefName );

    if( !pDef )
    {
        RscFile * pFile = GetFile( lFileKey );

        if( pFile )
        {
            pDef = pFile->aDefLst.New( lFileKey, rDefName, lId, ULONG_MAX );
            aDefTree.Insert( pDef );
        }
    }
    else
        pDef = nullptr;

    return pDef;
}

RscDefine * RscFileTab::NewDef( Index lFileKey, const OString& rDefName,
                                RscExpression * pExp )
{
    RscDefine * pDef = FindDef( rDefName );

    if( !pDef )
    {
        // are macros in expressions defined?
        if( TestDef( lFileKey, ULONG_MAX, pExp ) )
        {
            RscFile * pFile = GetFile( lFileKey );

            if( pFile )
            {
                pDef = pFile->aDefLst.New( lFileKey, rDefName, pExp, ULONG_MAX );
                aDefTree.Insert( pDef );
            }
        }
    }
    else
        pDef = nullptr;

    if( !pDef )
    {
        // pExp is always owned and must be deleted after used
        delete pExp;
    }
    return pDef;
}

void RscFileTab::DeleteFileContext( Index lFileKey )
{
    RscFile* pFName = GetFile( lFileKey );
    if( pFName )
    {
        for ( size_t i = 0, n = pFName->aDefLst.maList.size(); i < n; ++i )
        {
            RscDefine * pDef = pFName->aDefLst.maList[ i ];
            aDefTree.Remove( pDef );
        };

        while( pFName->aDefLst.Remove() ) ;
    }
}

RscFileTab::Index RscFileTab::NewCodeFile( const OString& rName )
{
    Index lKey = Find( rName );
    if( lKey == IndexNotFound )
    {
        RscFile * pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rName;
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey );
    }
    return lKey;
}

RscFileTab::Index RscFileTab::NewIncFile(const OString& rName,
                                    const OString& rPath)
{
    Index lKey = Find( rName );
    if( lKey == IndexNotFound )
    {
        RscFile * pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rPath;
        pFName->SetIncFlag();
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey );
    }
    return lKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
