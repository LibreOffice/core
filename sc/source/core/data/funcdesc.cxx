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

#include "funcdesc.hxx"

#include "addincol.hxx"
#include "appoptio.hxx"
#include "callform.hxx"
#include "compiler.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "scmod.hxx"
#include "scresid.hxx"

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <unotools/collatorwrapper.hxx>

#include <memory>
#include <numeric>

class ScFuncRes : public Resource
{
public:
    ScFuncRes( ResId&, ScFuncDesc*, bool & rbSuppressed );

private:
    sal_uInt16 GetNum();
};

class ScResourcePublisher : public Resource
{
private:
    using Resource::FreeResource;
public:
    explicit ScResourcePublisher( const ScResId& rId ) : Resource( rId ) {}
    ~ScResourcePublisher() { FreeResource(); }
    using Resource::IsAvailableRes;
};

// class ScFuncDesc:

ScFuncDesc::ScFuncDesc() :
        pFuncName       (nullptr),
        pFuncDesc       (nullptr),
        pDefArgFlags    (nullptr),
        nFIndex         (0),
        nCategory       (0),
        nArgCount       (0),
        nVarArgsStart   (0),
        bIncomplete     (false),
        bHasSuppressedArgs(false),
        mbHidden        (false)
{}

ScFuncDesc::~ScFuncDesc()
{
    Clear();
}

void ScFuncDesc::Clear()
{
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    if (nArgs)
    {
        delete [] pDefArgFlags;
    }
    nArgCount = 0;
    nVarArgsStart = 0;
    maDefArgNames.clear();
    maDefArgDescs.clear();
    pDefArgFlags = nullptr;

    delete pFuncName;
    pFuncName = nullptr;

    delete pFuncDesc;
    pFuncDesc = nullptr;

    nFIndex = 0;
    nCategory = 0;
    sHelpId.clear();
    bIncomplete = false;
    bHasSuppressedArgs = false;
    mbHidden = false;
}

OUString ScFuncDesc::GetParamList() const
{
    OUString sep(ScCompiler::GetNativeSymbol(ocSep));

    OUStringBuffer aSig;

    if ( nArgCount > 0 )
    {
        if ( nArgCount < VAR_ARGS )
        {
            sal_uInt16 nLastSuppressed = nArgCount;
            sal_uInt16 nLastAdded = nArgCount;
            for ( sal_uInt16 i=0; i<nArgCount; i++ )
            {
                if (pDefArgFlags[i].bSuppress)
                    nLastSuppressed = i;
                else
                {
                    nLastAdded = i;
                    aSig.append(maDefArgNames[i]);
                    if ( i != nArgCount-1 )
                    {
                        aSig.append(sep);
                        aSig.append( " " );
                    }
                }
            }
            // If only suppressed parameters follow the last added parameter,
            // remove one "; "
            if (nLastSuppressed < nArgCount && nLastAdded < nLastSuppressed &&
                    aSig.getLength() >= 2)
                aSig.setLength(aSig.getLength() - 2);
        }
        else if ( nArgCount < PAIRED_VAR_ARGS)
        {
            for ( sal_uInt16 nArg = 0; nArg < nVarArgsStart; nArg++ )
            {
                if (!pDefArgFlags[nArg].bSuppress)
                {
                    aSig.append(maDefArgNames[nArg]);
                    aSig.append(sep);
                    aSig.append( " " );
                }
            }
            /* NOTE: Currently there are no suppressed var args parameters. If
             * there were, we'd have to cope with it here and above for the fix
             * parameters. For now parameters are always added, so no special
             * treatment of a trailing "; " necessary. */
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append(' ');
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append(" ... ");
        }
        else
        {
            for ( sal_uInt16 nArg = 0; nArg < nVarArgsStart; nArg++ )
            {
                if (!pDefArgFlags[nArg].bSuppress)
                {
                    aSig.append(maDefArgNames[nArg]);
                    aSig.append(sep);
                    aSig.append( " " );
                }
            }

            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append(maDefArgNames[nVarArgsStart+1]);
            aSig.append('1');
            aSig.append(sep);
            aSig.append( " " );
            aSig.append(maDefArgNames[nVarArgsStart]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append(maDefArgNames[nVarArgsStart+1]);
            aSig.append('2');
            aSig.append(sep);
            aSig.append( " ... " );
        }
    }

    return aSig.makeStringAndClear();
}

OUString ScFuncDesc::getSignature() const
{
    OUStringBuffer aSig;

    if(pFuncName)
    {
        aSig.append(*pFuncName);

        OUString aParamList = GetParamList();
        if( !aParamList.isEmpty() )
        {
            aSig.append( "( " );
            aSig.append(aParamList);
            // U+00A0 (NBSP) prevents automatic line break
            aSig.append( static_cast< sal_Unicode >(0xA0) );
            aSig.append( ")" );
        }
        else
            aSig.append( "()" );
    }
    return aSig.makeStringAndClear();
}

OUString ScFuncDesc::getFormula( const ::std::vector< OUString >& _aArguments ) const
{
    OUString sep = ScCompiler::GetNativeSymbol(ocSep);

    OUStringBuffer aFormula;

    if(pFuncName)
    {
        aFormula.append( *pFuncName );

        aFormula.append( "(" );
        if ( nArgCount > 0 && !_aArguments.empty() && !_aArguments[0].isEmpty())
        {
            ::std::vector< OUString >::const_iterator aIter = _aArguments.begin();
            ::std::vector< OUString >::const_iterator aEnd = _aArguments.end();

            aFormula.append( *aIter );
            ++aIter;
            while( aIter != aEnd && !aIter->isEmpty() )
            {
                aFormula.append( sep );
                aFormula.append( *aIter );
                ++aIter;
            }
        }

        aFormula.append( ")" );
    }
    return aFormula.makeStringAndClear();
}

sal_uInt16 ScFuncDesc::GetSuppressedArgCount() const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
        return nArgCount;

    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    sal_uInt16 nCount = nArgs;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (pDefArgFlags[i].bSuppress)
            --nCount;
    }
    if (nArgCount >= PAIRED_VAR_ARGS)
        nCount += PAIRED_VAR_ARGS - 2;
    else if (nArgCount >= VAR_ARGS)
        nCount += VAR_ARGS - 1;
    return nCount;
}

OUString ScFuncDesc::getFunctionName() const
{
    OUString sRet;
    if ( pFuncName )
        sRet = *pFuncName;
    return sRet;
}

const formula::IFunctionCategory* ScFuncDesc::getCategory() const
{
    return ScGlobal::GetStarCalcFunctionMgr()->getCategory(nCategory);
}

OUString ScFuncDesc::getDescription() const
{
    OUString sRet;
    if ( pFuncDesc )
        sRet = *pFuncDesc;
    return sRet;
}

sal_Int32 ScFuncDesc::getSuppressedArgumentCount() const
{
    return GetSuppressedArgCount();
}

void ScFuncDesc::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
    {
        _rArguments.resize( nArgCount);
        ::std::vector<sal_uInt16>::iterator iter = _rArguments.begin();
        sal_uInt16 value = 0;
        while (iter != _rArguments.end())
            *iter++ = value++;
    }

    _rArguments.reserve( nArgCount);
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= PAIRED_VAR_ARGS)
        nArgs -= PAIRED_VAR_ARGS - 2;
    else if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (!pDefArgFlags || !pDefArgFlags[i].bSuppress)
            _rArguments.push_back(i);
    }
}

void ScFuncDesc::initArgumentInfo()  const
{
    // get the full argument description
    // (add-in has to be instantiated to get the type information)

    if ( bIncomplete && pFuncName )
    {
        ScUnoAddInCollection& rAddIns = *ScGlobal::GetAddInCollection();
        OUString aIntName(rAddIns.FindFunction( *pFuncName, true ));         // pFuncName is upper-case

        if ( !aIntName.isEmpty() )
        {
            // GetFuncData with bComplete=true loads the component and updates
            // the global function list if needed.

            rAddIns.GetFuncData( aIntName, true );
        }

        if ( bIncomplete )
        {
            OSL_FAIL( "couldn't initialize add-in function" );
            const_cast<ScFuncDesc*>(this)->bIncomplete = false;         // even if there was an error, don't try again
        }
    }
}

OString ScFuncDesc::getHelpId() const
{
    return sHelpId;
}

bool ScFuncDesc::isHidden() const
{
    return mbHidden;
}

sal_uInt32 ScFuncDesc::getParameterCount() const
{
    return nArgCount;
}

sal_uInt32 ScFuncDesc::getVarArgsStart() const
{
    return nVarArgsStart;
}

OUString ScFuncDesc::getParameterName(sal_uInt32 _nPos) const
{
    return maDefArgNames[_nPos];
}

OUString ScFuncDesc::getParameterDescription(sal_uInt32 _nPos) const
{
    return maDefArgDescs[_nPos];
}

bool ScFuncDesc::isParameterOptional(sal_uInt32 _nPos) const
{
    return pDefArgFlags[_nPos].bOptional;
}

bool ScFuncDesc::compareByName(const ScFuncDesc* a, const ScFuncDesc* b)
{
    return (ScGlobal::GetCaseCollator()->compareString(*a->pFuncName, *b->pFuncName ) < 0);
}

// class ScFunctionList:

ScFunctionList::ScFunctionList() :
        nMaxFuncNameLen ( 0 )
{
    ScFuncDesc* pDesc = nullptr;
    sal_Int32 nStrLen = 0;
    ::std::list<ScFuncDesc*> tmpFuncList;
    sal_uInt16 nDescBlock[] =
    {
        RID_SC_FUNCTION_DESCRIPTIONS1,
        RID_SC_FUNCTION_DESCRIPTIONS2
    };

    for (sal_uInt16 k : nDescBlock)
    {
        std::unique_ptr<ScResourcePublisher> pBlock( new ScResourcePublisher( ScResId( k ) ) );
        // Browse for all possible OpCodes. This is not the fastest method, but
        // otherwise the sub resources within the resource blocks and the
        // resource blocks themselves would had to be ordered according to
        // OpCodes, which is utopian...
        for (sal_uInt16 i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; ++i)
        {
            ScResId aRes(i);
            aRes.SetRT(RSC_RESOURCE);
            // Sub resource of OpCode available?
            if (pBlock->IsAvailableRes(aRes))
            {
                pDesc = new ScFuncDesc;
                bool bSuppressed = false;
                ScFuncRes aSubRes( aRes, pDesc, bSuppressed);
                // Instead of dealing with this exceptional case at 1001 places
                // we simply don't add an entirely suppressed function to the
                // list and delete it.
                if (bSuppressed)
                    delete pDesc;
                else
                {
                    pDesc->nFIndex = i;
                    tmpFuncList.push_back(pDesc);

                    nStrLen = (*(pDesc->pFuncName)).getLength();
                    if (nStrLen > nMaxFuncNameLen)
                        nMaxFuncNameLen = nStrLen;
                }
            }
        }
    }

    // legacy binary AddIn functions

    sal_uInt16 nNextId = SC_OPCODE_LAST_OPCODE_ID + 1; // FuncID for AddIn functions

    // Interpretation of AddIn list
    OUString aDefArgNameValue   = "value";
    OUString aDefArgNameString  = "string";
    OUString aDefArgNameValues  = "values";
    OUString aDefArgNameStrings = "strings";
    OUString aDefArgNameCells   = "cells";
    OUString aDefArgNameNone    = "none";
    OUString aDefArgDescValue   = "a value";
    OUString aDefArgDescString  = "a string";
    OUString aDefArgDescValues  = "array of values";
    OUString aDefArgDescStrings = "array of strings";
    OUString aDefArgDescCells   = "range of cells";
    OUString aDefArgDescNone    = "none";

    OUString aArgName, aArgDesc;
    const LegacyFuncCollection& rLegacyFuncColl = *ScGlobal::GetLegacyFuncCollection();
    LegacyFuncCollection::const_iterator it = rLegacyFuncColl.begin(), itEnd = rLegacyFuncColl.end();
    for (; it != itEnd; ++it)
    {
        const LegacyFuncData *const pLegacyFuncData = it->second.get();
        pDesc = new ScFuncDesc;
        sal_uInt16 nArgs = pLegacyFuncData->GetParamCount() - 1;
        pLegacyFuncData->getParamDesc( aArgName, aArgDesc, 0 );
        pDesc->nFIndex     = nNextId++; //  ??? OpCode vergeben
        pDesc->nCategory   = ID_FUNCTION_GRP_ADDINS;
        pDesc->pFuncName   = new OUString(pLegacyFuncData->GetInternalName().toAsciiUpperCase());

        OUStringBuffer aBuf(aArgDesc);
        aBuf.append('\n');
        aBuf.append("( AddIn: ");
        aBuf.append(pLegacyFuncData->GetModuleName());
        aBuf.append(" )");
        pDesc->pFuncDesc = new OUString(aBuf.makeStringAndClear());

        pDesc->nArgCount   = nArgs;
        if (nArgs)
        {
            pDesc->maDefArgNames.clear();
            pDesc->maDefArgNames.resize(nArgs);
            pDesc->maDefArgDescs.clear();
            pDesc->maDefArgDescs.resize(nArgs);
            pDesc->pDefArgFlags  = new ScFuncDesc::ParameterFlags[nArgs];
            for (sal_uInt16 j = 0; j < nArgs; ++j)
            {
                pDesc->pDefArgFlags[j].bOptional = false;
                pDesc->pDefArgFlags[j].bSuppress = false;
                pLegacyFuncData->getParamDesc( aArgName, aArgDesc, j+1 );
                if ( !aArgName.isEmpty() )
                    pDesc->maDefArgNames[j] = aArgName;
                else
                {
                    switch (pLegacyFuncData->GetParamType(j+1))
                    {
                        case ParamType::PTR_DOUBLE:
                            pDesc->maDefArgNames[j] = aDefArgNameValue;
                            break;
                        case ParamType::PTR_STRING:
                            pDesc->maDefArgNames[j] = aDefArgNameString;
                            break;
                        case ParamType::PTR_DOUBLE_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameValues;
                            break;
                        case ParamType::PTR_STRING_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameStrings;
                            break;
                        case ParamType::PTR_CELL_ARR:
                            pDesc->maDefArgNames[j] = aDefArgNameCells;
                            break;
                        default:
                            pDesc->maDefArgNames[j] = aDefArgNameNone;
                            break;
                    }
                }
                if ( !aArgDesc.isEmpty() )
                    pDesc->maDefArgDescs[j] = aArgDesc;
                else
                {
                    switch (pLegacyFuncData->GetParamType(j+1))
                    {
                        case ParamType::PTR_DOUBLE:
                            pDesc->maDefArgDescs[j] = aDefArgDescValue;
                            break;
                        case ParamType::PTR_STRING:
                            pDesc->maDefArgDescs[j] = aDefArgDescString;
                            break;
                        case ParamType::PTR_DOUBLE_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescValues;
                            break;
                        case ParamType::PTR_STRING_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescStrings;
                            break;
                        case ParamType::PTR_CELL_ARR:
                            pDesc->maDefArgDescs[j] = aDefArgDescCells;
                            break;
                        default:
                            pDesc->maDefArgDescs[j] = aDefArgDescNone;
                            break;
                    }
                }
            }
        }

        tmpFuncList.push_back(pDesc);
        nStrLen = (*(pDesc->pFuncName)).getLength();
        if ( nStrLen > nMaxFuncNameLen)
            nMaxFuncNameLen = nStrLen;
    }

    // StarOne AddIns

    ScUnoAddInCollection* pUnoAddIns = ScGlobal::GetAddInCollection();
    long nUnoCount = pUnoAddIns->GetFuncCount();
    for (long nFunc=0; nFunc<nUnoCount; nFunc++)
    {
        pDesc = new ScFuncDesc;
        pDesc->nFIndex = nNextId++;

        if ( pUnoAddIns->FillFunctionDesc( nFunc, *pDesc ) )
        {
            tmpFuncList.push_back(pDesc);
            nStrLen = (*(pDesc->pFuncName)).getLength();
            if (nStrLen > nMaxFuncNameLen)
                nMaxFuncNameLen = nStrLen;
        }
        else
            delete pDesc;
    }

    //Move list to vector for better random access performance
    ::std::vector<const ScFuncDesc*> tmp(tmpFuncList.begin(), tmpFuncList.end());
    tmpFuncList.clear();
    aFunctionList.swap(tmp);

    //Initialize iterator
    aFunctionListIter = aFunctionList.end();
}

ScFunctionList::~ScFunctionList()
{
    const ScFuncDesc* pDesc = First();
    while (pDesc)
    {
        delete pDesc;
        pDesc = Next();
    }
}

const ScFuncDesc* ScFunctionList::First()
{
    const ScFuncDesc* pDesc = nullptr;
    aFunctionListIter = aFunctionList.begin();
    if(aFunctionListIter != aFunctionList.end())
        pDesc = *aFunctionListIter;

    return pDesc;
}

const ScFuncDesc* ScFunctionList::Next()
{
    const ScFuncDesc* pDesc = nullptr;
    if(aFunctionListIter != aFunctionList.end())
    {
        if((++aFunctionListIter) != aFunctionList.end())
            pDesc = *aFunctionListIter;
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionList::GetFunction( sal_uInt32 nIndex ) const
{
    const ScFuncDesc* pDesc = nullptr;
    if(nIndex < aFunctionList.size())
        pDesc = aFunctionList.at(nIndex);

    return pDesc;
}

// class ScFunctionCategory:

sal_uInt32 ScFunctionCategory::getCount() const
{
    return m_pCategory->size();
}

OUString ScFunctionCategory::getName() const
{
    if ( m_sName.isEmpty() )
        m_sName = ScFunctionMgr::GetCategoryName(m_nCategory);
    return m_sName;
}

const formula::IFunctionDescription* ScFunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    const ScFuncDesc* pDesc = nullptr;
    if(_nPos < m_pCategory->size())
        pDesc = m_pCategory->at(_nPos);
    return pDesc;
}

sal_uInt32 ScFunctionCategory::getNumber() const
{
    return m_nCategory;
}

// class ScFunctionMgr:

ScFunctionMgr::ScFunctionMgr() :
    pFuncList( ScGlobal::GetStarCalcFunctionList() )
{
    OSL_ENSURE( pFuncList, "Functionlist not found." );
    sal_uInt32 catCount[MAX_FUNCCAT] = {0};

    aCatLists[0] = new ::std::vector<const ScFuncDesc*>();
    aCatLists[0]->reserve(pFuncList->GetCount());

    // Retrieve all functions, store in cumulative ("All") category, and count
    // number of functions in each category
    for(const ScFuncDesc* pDesc = pFuncList->First(); pDesc; pDesc = pFuncList->Next())
    {
        OSL_ENSURE((pDesc->nCategory) < MAX_FUNCCAT, "Unknown category");
        if ((pDesc->nCategory) < MAX_FUNCCAT)
            ++catCount[pDesc->nCategory];
        aCatLists[0]->push_back(pDesc);
    }

    // Sort functions in cumulative category by name
    ::std::sort(aCatLists[0]->begin(), aCatLists[0]->end(), ScFuncDesc::compareByName);

    // Allocate correct amount of space for categories
    for (sal_uInt16 i = 1; i < MAX_FUNCCAT; ++i)
    {
        aCatLists[i] = new ::std::vector<const ScFuncDesc*>();
        aCatLists[i]->reserve(catCount[i]);
    }

    // Fill categories with the corresponding functions (still sorted by name)
    for(::std::vector<const ScFuncDesc*>::iterator iter = aCatLists[0]->begin(); iter!=aCatLists[0]->end(); ++iter)
    {
        if (((*iter)->nCategory) < MAX_FUNCCAT)
            aCatLists[(*iter)->nCategory]->push_back(*iter);
    }

    // Initialize iterators
    pCurCatListIter = aCatLists[0]->end();
    pCurCatListEnd = aCatLists[0]->end();
}

ScFunctionMgr::~ScFunctionMgr()
{
    for (std::vector<const ScFuncDesc*> * pCatList : aCatLists)
        delete pCatList;
}


const ScFuncDesc* ScFunctionMgr::Get( sal_uInt16 nFIndex ) const
{
    const ScFuncDesc* pDesc;
    for (pDesc = First(); pDesc; pDesc = Next())
        if (pDesc->nFIndex == nFIndex)
            break;
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::First( sal_uInt16 nCategory ) const
{
    OSL_ENSURE( nCategory < MAX_FUNCCAT, "Unknown category" );
    const ScFuncDesc* pDesc = nullptr;
    if ( nCategory < MAX_FUNCCAT )
    {
        pCurCatListIter = aCatLists[nCategory]->begin();
        pCurCatListEnd = aCatLists[nCategory]->end();
        pDesc = *pCurCatListIter;
    }
    else
    {
        pCurCatListIter = aCatLists[0]->end();
        pCurCatListEnd = aCatLists[0]->end();
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::Next() const
{
    const ScFuncDesc* pDesc = nullptr;
    if ( pCurCatListIter != pCurCatListEnd )
    {
        if ( (++pCurCatListIter) != pCurCatListEnd )
        {
            pDesc = *pCurCatListIter;
        }
    }
    return pDesc;
}

sal_uInt32 ScFunctionMgr::getCount() const
{
    return MAX_FUNCCAT - 1;
}

const formula::IFunctionCategory* ScFunctionMgr::getCategory(sal_uInt32 nCategory) const
{
    if ( nCategory < (MAX_FUNCCAT-1) )
    {
        if (m_aCategories.find(nCategory) == m_aCategories.end())
            m_aCategories[nCategory].reset(new ScFunctionCategory(aCatLists[nCategory+1],nCategory)); // aCatLists[0] is "all"
        return m_aCategories[nCategory].get();
    }
    return nullptr;
}

void ScFunctionMgr::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = std::min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
    sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();
    _rLastRUFunctions.clear();

    if ( pLRUListIds )
    {
        for (sal_uInt16 i = 0; i < nLRUFuncCount; ++i)
        {
            _rLastRUFunctions.push_back( Get( pLRUListIds[i] ) );
        }
    }
}

OUString ScFunctionMgr::GetCategoryName(sal_uInt32 _nCategoryNumber )
{
    if (_nCategoryNumber >= SC_FUNCGROUP_COUNT)
    {
        OSL_FAIL("Invalid category number!");
        return OUString();
    }

    ResStringArray aStringArray(ScResId(RID_FUNCTION_CATEGORIES));
    return aStringArray.GetString(_nCategoryNumber);
}

sal_Unicode ScFunctionMgr::getSingleToken(const formula::IFunctionManager::EToken _eToken) const
{
    switch(_eToken)
    {
        case eOk:
            return ScCompiler::GetNativeSymbolChar(ocOpen);
        case eClose:
            return ScCompiler::GetNativeSymbolChar(ocClose);
        case eSep:
            return ScCompiler::GetNativeSymbolChar(ocSep);
        case eArrayOpen:
            return ScCompiler::GetNativeSymbolChar(ocArrayOpen);
        case eArrayClose:
            return ScCompiler::GetNativeSymbolChar(ocArrayClose);
    }
    return 0;
}

// class ScFuncRes:

ScFuncRes::ScFuncRes( ResId &aRes, ScFuncDesc* pDesc, bool & rbSuppressed )
 : Resource(aRes)
{
    sal_uInt16 nFunctionFlags = GetNum();
    // Bit 1: entirely suppressed
    // Bit 2: hidden unless used
    rbSuppressed = ((nFunctionFlags & 1) != 0);
    pDesc->mbHidden = ((nFunctionFlags & 2) != 0);
    pDesc->nCategory = GetNum();
    pDesc->sHelpId = ReadByteStringRes();
    pDesc->nArgCount = GetNum();
    sal_uInt16 nArgs = pDesc->nArgCount;
    sal_uInt16 nVarArgsSet = 0;
    if (nArgs >= PAIRED_VAR_ARGS)
    {
        nVarArgsSet = 2;
        nArgs -= PAIRED_VAR_ARGS - nVarArgsSet;
    }
    else if (nArgs >= VAR_ARGS)
    {
        nVarArgsSet = 1;
        nArgs -= VAR_ARGS - nVarArgsSet;
    }
    if (nArgs)
    {
        pDesc->nVarArgsStart = nArgs - nVarArgsSet;
        pDesc->pDefArgFlags = new ScFuncDesc::ParameterFlags[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->pDefArgFlags[i].bOptional = (bool)GetNum();
        }
    }
    // Need to read the value from the resource even if nArgs==0 to advance the
    // resource position pointer, so this can't be in the if(nArgs) block above.
    sal_uInt16 nSuppressed = GetNum();
    if (nSuppressed)
    {
        if (nSuppressed > nArgs)
        {
            SAL_WARN("sc.core", "ScFuncRes: suppressed parameters count mismatch on OpCode " <<
                    aRes.GetId() << ": suppressed " << nSuppressed << " > params " << nArgs);
            nSuppressed = nArgs;    // sanitize
        }
        for (sal_uInt16 i = 0; i < nSuppressed; ++i)
        {
            sal_uInt16 nParam = GetNum();
            if (nParam < nArgs)
            {
                if (pDesc->nArgCount >= PAIRED_VAR_ARGS && nParam >= nArgs-2)
                {
                    SAL_WARN("sc.core", "ScFuncRes: PAIRED_VAR_ARGS parameters can't be suppressed, on OpCode " <<
                            aRes.GetId() << ": param " << nParam << " >= arg " << nArgs << "-2");
                }
                else if (pDesc->nArgCount >= VAR_ARGS && nParam == nArgs-1)
                {
                    SAL_WARN("sc.core", "ScFuncRes: VAR_ARGS parameters can't be suppressed, on OpCode " <<
                            aRes.GetId() << ": param " << nParam << " == arg " << nArgs << "-1");
                }
                else
                {
                    pDesc->pDefArgFlags[nParam].bSuppress = true;
                    pDesc->bHasSuppressedArgs = true;
                }
            }
            else
            {
                SAL_WARN("sc.core", "ScFuncRes: suppressed parameter exceeds count on OpCode " <<
                        aRes.GetId() << ": param " << nParam << " >= args " << nArgs);
            }
        }
    }

    pDesc->pFuncName = new OUString( ScCompiler::GetNativeSymbol( static_cast<OpCode>( aRes.GetId())));
    pDesc->pFuncDesc = new OUString( SC_RESSTR(1) );

    if (nArgs)
    {
        pDesc->maDefArgNames.clear();
        pDesc->maDefArgNames.resize(nArgs);
        pDesc->maDefArgDescs.clear();
        pDesc->maDefArgDescs.resize(nArgs);
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->maDefArgNames[i] = SC_RESSTR(2*(i+1)  );
            pDesc->maDefArgDescs[i] = SC_RESSTR(2*(i+1)+1);
            // If empty and variable number of arguments and last parameter and
            // parameter is optional and the previous is not optional, repeat
            // previous parameter name and description.
            if ((pDesc->maDefArgNames[i].isEmpty() || pDesc->maDefArgDescs[i].isEmpty()) &&
                    nVarArgsSet > 0 && i > nVarArgsSet && (i == nArgs-1 || i == nArgs-2) &&
                    pDesc->pDefArgFlags[i].bOptional)
            {
                sal_uInt16 nPrev = i - nVarArgsSet;
                if (!pDesc->pDefArgFlags[nPrev].bOptional)
                {
                    if (pDesc->maDefArgNames[i].isEmpty())
                        pDesc->maDefArgNames[i] = pDesc->maDefArgNames[nPrev];
                    if (pDesc->maDefArgDescs[i].isEmpty())
                        pDesc->maDefArgDescs[i] = pDesc->maDefArgDescs[nPrev];
                    // This also means that variable arguments start one
                    // parameter set earlier.
                    pDesc->nVarArgsStart -= nVarArgsSet;
                }
            }
        }
    }

    FreeResource();
}

sal_uInt16 ScFuncRes::GetNum()
{
    return ReadShortRes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
