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

#ifndef SC_FUNCDESC_HXX
#define SC_FUNCDESC_HXX

/* Function descriptions for function wizard / autopilot / most recent used
 * list et al. Separated from the global.hxx lump, implementation still in
 * global.cxx
 */

#include <tools/list.hxx>
#include <tools/string.hxx>
#include <formula/IFunctionDescription.hxx>

#define MAX_FUNCCAT 12  /* maximum number of categories for functions */

class ScFuncDesc : public formula::IFunctionDescription
{
public:

    virtual ::rtl::OUString getFunctionName() const ;
    virtual const formula::IFunctionCategory* getCategory() const ;
    virtual ::rtl::OUString getDescription() const ;
    // GetSuppressedArgCount
    virtual xub_StrLen getSuppressedArgumentCount() const ;
    /** Returns the function signature with parameters from the passed string array. */
    virtual ::rtl::OUString getFormula(const ::std::vector< ::rtl::OUString >& _aArguments) const ;
    // GetVisibleArgMapping
    /** Returns mapping from visible arguments to real arguments, e.g. if of 4
        parameters the second one is suppressed {0,2,3}. For VAR_ARGS
        parameters only one element is added to the end of the sequence. */
    virtual void fillVisibleArgumentMapping(::std::vector<USHORT>& _rArguments) const ;
    virtual void initArgumentInfo()  const;
    virtual ::rtl::OUString getSignature() const ;
    virtual long getHelpId() const ;

    // parameter
    virtual sal_uInt32 getParameterCount() const ;
    virtual ::rtl::OUString getParameterName(sal_uInt32 _nPos) const ;
    virtual ::rtl::OUString getParameterDescription(sal_uInt32 _nPos) const ;
    virtual bool isParameterOptional(sal_uInt32 _nPos) const ;

    struct ParameterFlags
    {
        bool    bOptional   :1;     // Parameter is optional
        bool    bSuppress   :1;     // Suppress parameter in UI because not implemented yet

        ParameterFlags() : bOptional(false), bSuppress(false) {}
    };


    ScFuncDesc();
    virtual ~ScFuncDesc();

    void        Clear();

    /** Returns a semicolon separated list of all parameter names. */
    String  GetParamList        () const;
    /** Returns the full function signature: "FUNCTIONNAME( parameter list )". */
    String  GetSignature        () const;



    /** Returns the number of non-suppressed arguments. In case there are
        variable arguments the number of fixed non-suppressed arguments plus
        VAR_ARGS, same as for nArgCount (variable arguments can't be
        suppressed). */
    USHORT  GetSuppressedArgCount() const;

    String          *pFuncName;              // Function name
    String          *pFuncDesc;              // Description of function
    String         **ppDefArgNames;          // Parameter name(s)
    String         **ppDefArgDescs;          // Description(s) of parameter(s)
    ParameterFlags  *pDefArgFlags;           // Flags for each parameter
    USHORT           nFIndex;                // Unique function index
    USHORT           nCategory;              // Function category
    USHORT           nArgCount;              // All parameter count, suppressed and unsuppressed
    USHORT           nHelpId;                // HelpID of function
    bool             bIncomplete         :1; // Incomplete argument info (set for add-in info from configuration)
    bool             bHasSuppressedArgs  :1; // Whether there is any suppressed parameter.
};

//============================================================================

class ScFunctionList
{
public:
    ScFunctionList();
    ~ScFunctionList();

    ULONG           GetCount() const
                    { return aFunctionList.Count(); }

    const ScFuncDesc*   First()
                        { return (const ScFuncDesc*) aFunctionList.First(); }

    const ScFuncDesc*   Next()
                        { return (const ScFuncDesc*) aFunctionList.Next(); }

    const ScFuncDesc*   GetFunction( ULONG nIndex ) const
                    { return (const ScFuncDesc*) aFunctionList.GetObject( nIndex ); }

    xub_StrLen      GetMaxFuncNameLen() const
                    { return nMaxFuncNameLen; }

private:
    List        aFunctionList;
    xub_StrLen  nMaxFuncNameLen;
};

//============================================================================
class ScFunctionCategory : public formula::IFunctionCategory
{
    ScFunctionMgr* m_pMgr;
    List* m_pCategory;
    mutable ::rtl::OUString m_sName;
    sal_uInt32 m_nCategory;
public:
    ScFunctionCategory(ScFunctionMgr* _pMgr,List* _pCategory,sal_uInt32 _nCategory) : m_pMgr(_pMgr),m_pCategory(_pCategory),m_nCategory(_nCategory){}
    virtual ~ScFunctionCategory(){}
    virtual sal_uInt32                          getCount() const;
    virtual const formula::IFunctionManager*        getFunctionManager() const;
    virtual const formula::IFunctionDescription*    getFunction(sal_uInt32 _nPos) const;
    virtual sal_uInt32                          getNumber() const;
    virtual ::rtl::OUString                     getName() const;
};
//============================================================================
#define SC_FUNCGROUP_COUNT  ID_FUNCTION_GRP_ADDINS
class ScFunctionMgr : public formula::IFunctionManager
{
public:
            ScFunctionMgr();
    virtual ~ScFunctionMgr();

    static String       GetCategoryName(sal_uInt32 _nCategoryNumber );

    const ScFuncDesc*   Get( const String& rFName ) const;
    const ScFuncDesc*   Get( USHORT nFIndex ) const;
    const ScFuncDesc*   First( USHORT nCategory = 0 ) const;
    const ScFuncDesc*   Next() const;

    // formula::IFunctionManager
    virtual sal_uInt32                              getCount() const;
    virtual const formula::IFunctionCategory*       getCategory(sal_uInt32 nPos) const;
    virtual void                                    fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const;
    virtual const formula::IFunctionDescription*    getFunctionByName(const ::rtl::OUString& _sFunctionName) const;
    virtual sal_Unicode                             getSingleToken(const formula::IFunctionManager::EToken _eToken) const;
private:
    ScFunctionList* pFuncList;
    List*           aCatLists[MAX_FUNCCAT];
    mutable List*   pCurCatList;
};

//============================================================================
#endif // SC_FUNCDESC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
