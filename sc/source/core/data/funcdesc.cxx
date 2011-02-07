/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <numeric>
#include <boost/scoped_ptr.hpp>

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
    void FreeResource() { Resource::FreeResource(); }
public:
    ScResourcePublisher( const ScResId& rId ) : Resource( rId ) {}
    ~ScResourcePublisher() { FreeResource(); }
    bool IsAvailableRes( const ResId& rId ) const
                        { return Resource::IsAvailableRes( rId ); }

};

//========================================================================
// class ScFuncDesc:
//========================================================================

ScFuncDesc::ScFuncDesc() :
        pFuncName       (NULL),
        pFuncDesc       (NULL),
        ppDefArgNames   (NULL),
        ppDefArgDescs   (NULL),
        pDefArgFlags    (NULL),
        nFIndex         (0),
        nCategory       (0),
        nArgCount       (0),
        nHelpId         (0),
        bIncomplete     (false),
        bHasSuppressedArgs(false)
{}

ScFuncDesc::~ScFuncDesc()
{
    Clear();
}

void ScFuncDesc::Clear()
{
    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= VAR_ARGS) nArgs -= VAR_ARGS-1;
    if (nArgs)
    {
        for (sal_uInt16 i=0; i<nArgs; i++ )
        {
            delete ppDefArgNames[i];
            delete ppDefArgDescs[i];
        }
        delete [] ppDefArgNames;
        delete [] ppDefArgDescs;
        delete [] pDefArgFlags;
    }
    nArgCount = 0;
    ppDefArgNames = NULL;
    ppDefArgDescs = NULL;
    pDefArgFlags = NULL;

    delete pFuncName;
    pFuncName = NULL;

    delete pFuncDesc;
    pFuncDesc = NULL;

    nFIndex = 0;
    nCategory = 0;
    nHelpId = 0;
    bIncomplete = false;
    bHasSuppressedArgs = false;
}

::rtl::OUString ScFuncDesc::GetParamList() const
{
    ::rtl::OUString sep(ScCompiler::GetNativeSymbol(ocSep));

    ::rtl::OUStringBuffer aSig;

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
                    aSig.append(*(ppDefArgNames[i]));
                    if ( i != nArgCount-1 )
                    {
                        aSig.append(sep);
                        aSig.appendAscii( " " );
                    }
                }
            }
            // If only suppressed parameters follow the last added parameter,
            // remove one "; "
            if (nLastSuppressed < nArgCount && nLastAdded < nLastSuppressed &&
                    aSig.getLength() >= 2)
                aSig.setLength(aSig.getLength() - 2);
        }
        else
        {
            sal_uInt16 nFix = nArgCount - VAR_ARGS;
            for ( sal_uInt16 nArg = 0; nArg < nFix; nArg++ )
            {
                if (!pDefArgFlags[nArg].bSuppress)
                {
                    aSig.append(*(ppDefArgNames[nArg]));
                    aSig.append(sep);
                    aSig.appendAscii( " " );
                }
            }
            /* NOTE: Currently there are no suppressed var args parameters. If
             * there were, we'd have to cope with it here and above for the fix
             * parameters. For now parameters are always added, so no special
             * treatment of a trailing "; " necessary. */
            aSig.append(*(ppDefArgNames[nFix]));
            aSig.append(sal_Unicode('1'));
            aSig.append(sep);
            aSig.append(sal_Unicode(' '));
            aSig.append(*(ppDefArgNames[nFix]));
            aSig.append(sal_Unicode('2'));
            aSig.append(sep);
            aSig.appendAscii(" ... ");
        }
    }

    return aSig.makeStringAndClear();
}

::rtl::OUString ScFuncDesc::getSignature() const
{
    ::rtl::OUStringBuffer aSig;

    if(pFuncName)
    {
        aSig.append(*pFuncName);

        ::rtl::OUString aParamList = GetParamList();
        if( aParamList.getLength() )
        {
            aSig.appendAscii( "( " );
            aSig.append(aParamList);
            // U+00A0 (NBSP) prevents automatic line break
            aSig.append( static_cast< sal_Unicode >(0xA0) );
            aSig.appendAscii( ")" );
        }
        else
            aSig.appendAscii( "()" );
    }
    return aSig.makeStringAndClear();
}

::rtl::OUString ScFuncDesc::getFormula( const ::std::vector< ::rtl::OUString >& _aArguments ) const
{
    ::rtl::OUString sep = ScCompiler::GetNativeSymbol(ocSep);

    ::rtl::OUStringBuffer aFormula;

    if(pFuncName)
    {
        aFormula.append( *pFuncName );

        aFormula.appendAscii( "(" );
        ::std::vector< ::rtl::OUString >::const_iterator aIter = _aArguments.begin();
        ::std::vector< ::rtl::OUString >::const_iterator aEnd = _aArguments.end();

        if ( nArgCount > 0 && aIter != aEnd )
        {
            bool bLastArg = ( aIter->getLength() == 0 );

            while( aIter != aEnd && !bLastArg )
            {
                aFormula.append( *(aIter) );
                if ( aIter != (aEnd-1) )
                {
                    bLastArg = !( (aIter+1)->getLength() > 0 );
                    if ( !bLastArg )
                        aFormula.append( sep );
                }

                ++aIter;
            }
        }

        aFormula.appendAscii( ")" );
    }
    return aFormula.makeStringAndClear();
}

sal_uInt16 ScFuncDesc::GetSuppressedArgCount() const
{
    if (!bHasSuppressedArgs || !pDefArgFlags)
        return nArgCount;

    sal_uInt16 nArgs = nArgCount;
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    sal_uInt16 nCount = nArgs;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (pDefArgFlags[i].bSuppress)
            --nCount;
    }
    if (nArgCount >= VAR_ARGS)
        nCount += VAR_ARGS - 1;
    return nCount;
}

::rtl::OUString ScFuncDesc::getFunctionName() const
{
    ::rtl::OUString sRet;
    if ( pFuncName )
        sRet = *pFuncName;
    return sRet;
}

const formula::IFunctionCategory* ScFuncDesc::getCategory() const
{
    return ScGlobal::GetStarCalcFunctionMgr()->getCategory(nCategory);
}

::rtl::OUString ScFuncDesc::getDescription() const
{
    ::rtl::OUString sRet;
    if ( pFuncDesc )
        sRet = *pFuncDesc;
    return sRet;
}

xub_StrLen ScFuncDesc::getSuppressedArgumentCount() const
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
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    for (sal_uInt16 i=0; i < nArgs; ++i)
    {
        if (!pDefArgFlags[i].bSuppress)
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
        ::rtl::OUString aIntName(rAddIns.FindFunction( *pFuncName, true ));         // pFuncName is upper-case

        if ( aIntName.getLength() )
        {
            // GetFuncData with bComplete=true loads the component and updates
            // the global function list if needed.

            rAddIns.GetFuncData( aIntName, true );
        }

        if ( bIncomplete )
        {
            DBG_ERRORFILE( "couldn't initialize add-in function" );
            const_cast<ScFuncDesc*>(this)->bIncomplete = false;         // even if there was an error, don't try again
        }
    }
}

long ScFuncDesc::getHelpId() const
{
    return (long)nHelpId;
}

sal_uInt32 ScFuncDesc::getParameterCount() const
{
    return nArgCount;
}

::rtl::OUString ScFuncDesc::getParameterName(sal_uInt32 _nPos) const
{
    return *(ppDefArgNames[_nPos]);
}

::rtl::OUString ScFuncDesc::getParameterDescription(sal_uInt32 _nPos) const
{
    return *(ppDefArgDescs[_nPos]);
}

bool ScFuncDesc::isParameterOptional(sal_uInt32 _nPos) const
{
    return pDefArgFlags[_nPos].bOptional;
}

bool ScFuncDesc::compareByName(const ScFuncDesc* a, const ScFuncDesc* b)
{
    return (ScGlobal::GetCaseCollator()->compareString(*a->pFuncName, *b->pFuncName ) == COMPARE_LESS);
}

//===================================================================
// class ScFunctionList:
//===================================================================

ScFunctionList::ScFunctionList() :
        nMaxFuncNameLen ( 0 )
{
    ScFuncDesc* pDesc = NULL;
    xub_StrLen nStrLen = 0;
    FuncCollection* pFuncColl;
    ::std::list<ScFuncDesc*> tmpFuncList;
    sal_uInt16 nDescBlock[] =
    {
        RID_SC_FUNCTION_DESCRIPTIONS1,
        RID_SC_FUNCTION_DESCRIPTIONS2
    };

    for (sal_uInt16 k = 0; k < SAL_N_ELEMENTS(nDescBlock); ++k)
    {
        ::std::auto_ptr<ScResourcePublisher> pBlock( new ScResourcePublisher( ScResId( nDescBlock[k] ) ) );
        // Browse for all possible OpCodes. This is not the fastest method, but
        // otherwise the sub resources within the resource blocks and the
        // resource blocks themselfs would had to be ordered according to
        // OpCodes, which is utopian..
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

    sal_uInt16 nNextId = SC_OPCODE_LAST_OPCODE_ID + 1; // FuncID for AddIn functions

    // Interpretation of AddIn list
    ::rtl::OUString aDefArgNameValue   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("value"));
    ::rtl::OUString aDefArgNameString  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("string"));
    ::rtl::OUString aDefArgNameValues  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("values"));
    ::rtl::OUString aDefArgNameStrings = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("strings"));
    ::rtl::OUString aDefArgNameCells   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cells"));
    ::rtl::OUString aDefArgNameNone    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("none"));
    ::rtl::OUString aDefArgDescValue   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a value"));
    ::rtl::OUString aDefArgDescString  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("a string"));
    ::rtl::OUString aDefArgDescValues  = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("array of values"));
    ::rtl::OUString aDefArgDescStrings = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("array of strings"));
    ::rtl::OUString aDefArgDescCells   = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("range of cells"));
    ::rtl::OUString aDefArgDescNone    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("none"));

    ::rtl::OUString aArgName, aArgDesc;
    pFuncColl = ScGlobal::GetFuncCollection();
    for (sal_uInt16 i = 0; i < pFuncColl->GetCount(); ++i)
    {
        pDesc = new ScFuncDesc;
        FuncData *pAddInFuncData = (FuncData*)pFuncColl->At(i);
        sal_uInt16 nArgs = pAddInFuncData->GetParamCount() - 1;
        pAddInFuncData->getParamDesc( aArgName, aArgDesc, 0 );
        pDesc->nFIndex     = nNextId++; //  ??? OpCode vergeben
        pDesc->nCategory   = ID_FUNCTION_GRP_ADDINS;
        pDesc->pFuncName   = new ::rtl::OUString(pAddInFuncData->GetInternalName());
        pDesc->pFuncName->toAsciiUpperCase();

        ::rtl::OUStringBuffer aBuf(aArgDesc);
        aBuf.append(sal_Unicode('\n'));
        aBuf.appendAscii("( AddIn: ");
        aBuf.append(pAddInFuncData->GetModuleName());
        aBuf.appendAscii(" )");
        pDesc->pFuncDesc = new ::rtl::OUString(aBuf.makeStringAndClear());

        pDesc->nArgCount   = nArgs;
        if (nArgs)
        {
            pDesc->pDefArgFlags  = new ScFuncDesc::ParameterFlags[nArgs];
            pDesc->ppDefArgNames = new ::rtl::OUString*[nArgs];
            pDesc->ppDefArgDescs = new ::rtl::OUString*[nArgs];
            for (sal_uInt16 j = 0; j < nArgs; ++j)
            {
                pDesc->pDefArgFlags[j].bOptional = false;
                pDesc->pDefArgFlags[j].bSuppress = false;
                pAddInFuncData->getParamDesc( aArgName, aArgDesc, j+1 );
                if ( aArgName.getLength() )
                    pDesc->ppDefArgNames[j] = new ::rtl::OUString( aArgName );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameValue );
                            break;
                        case PTR_STRING:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameCells );
                            break;
                        default:
                            pDesc->ppDefArgNames[j] = new ::rtl::OUString( aDefArgNameNone );
                            break;
                    }
                }
                if ( aArgDesc.getLength() )
                    pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aArgDesc );
                else
                {
                    switch (pAddInFuncData->GetParamType(j+1))
                    {
                        case PTR_DOUBLE:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescValue );
                            break;
                        case PTR_STRING:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescString );
                            break;
                        case PTR_DOUBLE_ARR:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescValues );
                            break;
                        case PTR_STRING_ARR:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescStrings );
                            break;
                        case PTR_CELL_ARR:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescCells );
                            break;
                        default:
                            pDesc->ppDefArgDescs[j] = new ::rtl::OUString( aDefArgDescNone );
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
    const ScFuncDesc* pDesc = NULL;
    aFunctionListIter = aFunctionList.begin();
    if(aFunctionListIter != aFunctionList.end())
        pDesc = *aFunctionListIter;

    return pDesc;
}

const ScFuncDesc* ScFunctionList::Next()
{
    const ScFuncDesc* pDesc = NULL;
    if(aFunctionListIter != aFunctionList.end())
    {
        if((++aFunctionListIter) != aFunctionList.end())
            pDesc = *aFunctionListIter;
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionList::GetFunction( sal_uInt32 nIndex ) const
{
    const ScFuncDesc* pDesc = NULL;
    if(nIndex < aFunctionList.size())
        pDesc = aFunctionList.at(nIndex);

    return pDesc;
}

//===================================================================
// class ScFunctionCategory:
//===================================================================

sal_uInt32 ScFunctionCategory::getCount() const
{
    return m_pCategory->size();
}

const formula::IFunctionManager* ScFunctionCategory::getFunctionManager() const
{
    return m_pMgr;
}

::rtl::OUString ScFunctionCategory::getName() const
{
    if ( !m_sName.getLength() )
        m_sName = ScFunctionMgr::GetCategoryName(m_nCategory+1);
    return m_sName;
}

const formula::IFunctionDescription* ScFunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    const ScFuncDesc* pDesc = NULL;
    if(_nPos < m_pCategory->size())
        pDesc = m_pCategory->at(_nPos);
    return pDesc;
}

sal_uInt32 ScFunctionCategory::getNumber() const
{
    return m_nCategory;
}

//========================================================================
// class ScFunctionMgr:
//========================================================================

ScFunctionMgr::ScFunctionMgr() :
    pFuncList( ScGlobal::GetStarCalcFunctionList() )
{
    DBG_ASSERT( pFuncList, "Functionlist not found." );
    sal_uInt32 catCount[MAX_FUNCCAT] = {0};

    aCatLists[0] = new ::std::vector<const ScFuncDesc*>();
    aCatLists[0]->reserve(pFuncList->GetCount());

    // Retrieve all functions, store in cumulative ("All") category, and count
    // number of functions in each category
    for(const ScFuncDesc* pDesc = pFuncList->First(); pDesc; pDesc = pFuncList->Next())
    {
        DBG_ASSERT((pDesc->nCategory) < MAX_FUNCCAT, "Unknown category");
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
    for (sal_uInt16 i = 0; i < MAX_FUNCCAT; ++i)
        delete aCatLists[i];
}

const ScFuncDesc* ScFunctionMgr::Get( const ::rtl::OUString& rFName ) const
{
    const ScFuncDesc* pDesc = NULL;
    if (rFName.getLength() <= pFuncList->GetMaxFuncNameLen())
    {
        ::boost::scoped_ptr<ScFuncDesc> dummy(new ScFuncDesc);
        dummy->pFuncName = new ::rtl::OUString(rFName);
        ::std::vector<const ScFuncDesc*>::iterator lower =
            ::std::lower_bound(aCatLists[0]->begin(), aCatLists[0]->end(),
                        static_cast<const ScFuncDesc*>(dummy.get()), ScFuncDesc::compareByName);

        if(rFName.equalsIgnoreAsciiCase(*(*lower)->pFuncName))
            pDesc = *lower;
    }
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::Get( sal_uInt16 nFIndex ) const
{
    const ScFuncDesc* pDesc;
    for (pDesc = First(0); pDesc; pDesc = Next())
        if (pDesc->nFIndex == nFIndex)
            break;
    return pDesc;
}

const ScFuncDesc* ScFunctionMgr::First( sal_uInt16 nCategory ) const
{
    DBG_ASSERT( nCategory < MAX_FUNCCAT, "Unbekannte Kategorie" );
    const ScFuncDesc* pDesc = NULL;
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
    const ScFuncDesc* pDesc = NULL;
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
    formula::IFunctionCategory* pRet = NULL;
    if ( nCategory < (MAX_FUNCCAT-1) )
    {
         pRet = new ScFunctionCategory(const_cast<ScFunctionMgr*>(this),aCatLists[nCategory+1],nCategory); // aCatLists[0] is "all"
    }
    return pRet;
}

const formula::IFunctionDescription* ScFunctionMgr::getFunctionByName(const ::rtl::OUString& _sFunctionName) const
{
    return Get(_sFunctionName);
}

void ScFunctionMgr::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const
{
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = Min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
    sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();

    if ( pLRUListIds )
    {
        for (sal_uInt16 i = 0; i < nLRUFuncCount; ++i)
            _rLastRUFunctions.push_back( Get( pLRUListIds[i] ) );
    }
}

::rtl::OUString ScFunctionMgr::GetCategoryName(sal_uInt32 _nCategoryNumber )
{
    if ( _nCategoryNumber > SC_FUNCGROUP_COUNT )
    {
        DBG_ERROR("Invalid category number!");
        return ::rtl::OUString();
    }

    ::std::auto_ptr<ScResourcePublisher> pCategories( new ScResourcePublisher( ScResId( RID_FUNCTION_CATEGORIES ) ) );
    return ResId::toString(ScResId(static_cast<sal_uInt16>(_nCategoryNumber)));
}

sal_Unicode ScFunctionMgr::getSingleToken(const formula::IFunctionManager::EToken _eToken) const
{
    switch(_eToken)
    {
        case eOk:
            return ScCompiler::GetNativeSymbol(ocOpen).GetChar(0);
        case eClose:
            return ScCompiler::GetNativeSymbol(ocClose).GetChar(0);
        case eSep:
            return ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
        case eArrayOpen:
            return ScCompiler::GetNativeSymbol(ocArrayOpen).GetChar(0);
        case eArrayClose:
            return ScCompiler::GetNativeSymbol(ocArrayClose).GetChar(0);
    }
    return 0;
}

//========================================================================
// class ScFuncRes:
//========================================================================

ScFuncRes::ScFuncRes( ResId &aRes, ScFuncDesc* pDesc, bool & rbSuppressed )
 : Resource(aRes)
{
    rbSuppressed = (bool)GetNum();
    pDesc->nCategory = GetNum();
    pDesc->nHelpId = GetNum() + 32768; //! Hack, see scfuncs.src
    pDesc->nArgCount = GetNum();
    sal_uInt16 nArgs = pDesc->nArgCount;
    if (nArgs >= VAR_ARGS)
        nArgs -= VAR_ARGS - 1;
    if (nArgs)
    {
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
            DBG_ERROR3( "ScFuncRes: suppressed parameters count mismatch on OpCode %u: suppressed %d > params %d",
                    aRes.GetId(), (int)nSuppressed, (int)nArgs);
            nSuppressed = nArgs;    // sanitize
        }
        for (sal_uInt16 i = 0; i < nSuppressed; ++i)
        {
            sal_uInt16 nParam = GetNum();
            if (nParam < nArgs)
            {
                if (pDesc->nArgCount >= VAR_ARGS && nParam == nArgs-1)
                {
                    DBG_ERROR3( "ScFuncRes: VAR_ARGS parameters can't be suppressed, on OpCode %u: param %d == arg %d-1",
                            aRes.GetId(), (int)nParam, (int)nArgs);
                }
                else
                {
                    pDesc->pDefArgFlags[nParam].bSuppress = true;
                    pDesc->bHasSuppressedArgs = true;
                }
            }
            else
            {
                DBG_ERROR3( "ScFuncRes: suppressed parameter exceeds count on OpCode %u: param %d >= args %d",
                        aRes.GetId(), (int)nParam, (int)nArgs);
            }
        }
    }

    pDesc->pFuncName = new ::rtl::OUString( ScCompiler::GetNativeSymbol( static_cast<OpCode>( aRes.GetId())));
    pDesc->pFuncDesc = new ::rtl::OUString( ResId::toString(ScResId(1)));

    if (nArgs)
    {
        pDesc->ppDefArgNames = new ::rtl::OUString*[nArgs];
        pDesc->ppDefArgDescs = new ::rtl::OUString*[nArgs];
        for (sal_uInt16 i = 0; i < nArgs; ++i)
        {
            pDesc->ppDefArgNames[i] = new ::rtl::OUString(ResId::toString(ScResId(2*(i+1)  )));
            pDesc->ppDefArgDescs[i] = new ::rtl::OUString(ResId::toString(ScResId(2*(i+1)+1)));
        }
    }

    FreeResource();
}

sal_uInt16 ScFuncRes::GetNum()
{
    return ReadShortRes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
