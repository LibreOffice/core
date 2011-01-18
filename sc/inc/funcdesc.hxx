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

#include "scfuncs.hrc"

#include <tools/list.hxx>
#include <formula/IFunctionDescription.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>

#define MAX_FUNCCAT 12  /* maximum number of categories for functions */

class ScFuncDesc;
class ScFunctionList;
class ScFunctionCategory;
class ScFunctionMgr;

/**
  Stores and generates human readable descriptions for spreadsheet-functions,
  e.g. functions used in formulas in calc
*/
class ScFuncDesc : public formula::IFunctionDescription
{
public:
    ScFuncDesc();
    virtual ~ScFuncDesc();

    /**
      Clears the object

      Deletes all objets referenced by the pointers in the class,
      sets pointers to NULL, and all numerical variables to 0
    */
    void Clear();

    /**
      Fills a mapping with indexes for non-suppressed arguments

      Fills mapping from visible arguments to real arguments, e.g. if of 4
      parameters the second one is suppressed {0,2,3}. For VAR_ARGS
      parameters only one element is added to the end of the sequence.

      @param _rArgumens
      Vector, which the indices are written to
    */
    virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const ;

    /**
      Returns the category of the function

      @return    the category of the function
    */
    virtual const formula::IFunctionCategory* getCategory() const ;

    /**
      Returns the description of the function

      @return    the description of the function, or an empty OUString if there is no description
    */
    virtual ::rtl::OUString getDescription() const ;

    /**
      Returns the function signature with parameters from the passed string array.

      @return    function signature with parameters
    */
    virtual ::rtl::OUString getFormula(const ::std::vector< ::rtl::OUString >& _aArguments) const ;

    /**
      Returns the name of the function

      @return    the name of the function, or an empty OUString if there is no name
    */
    virtual ::rtl::OUString getFunctionName() const ;

    /**
      Returns the help id of the function

      @return   help id of the function
    */
    virtual long getHelpId() const ;

    /**
      Returns number of arguments

      @return   help id of the function
    */
    virtual sal_uInt32 getParameterCount() const ;

    /**
      Returns description of parameter at given position

      @param _nPos
      Position of the parameter

      @return   OUString description of the parameter
    */
    virtual ::rtl::OUString getParameterDescription(sal_uInt32 _nPos) const ;

    /**
      Returns name of parameter at given position

      @param _nPos
      Position of the parameter

      @return   OUString name of the parameter
    */
    virtual ::rtl::OUString getParameterName(sal_uInt32 _nPos) const ;

    /**
      Returns list of all parameter names

      @return OUString containing separated list of all parameter names
    */
    ::rtl::OUString GetParamList() const;

    /**
      Returns the full function signature

      @return   OUString of the form "FUNCTIONNAME( parameter list )"
    */
    virtual ::rtl::OUString getSignature() const ;

    /**
      Returns the number of non-suppressed arguments

      In case there are variable arguments the number of fixed non-suppressed
      arguments plus VAR_ARGS, same as for nArgCount (variable arguments can't
      be suppressed). The two functions are equal apart from return type and
      name.

      @return    number of non-suppressed arguments
    */
    sal_uInt16  GetSuppressedArgCount() const;
    virtual xub_StrLen getSuppressedArgumentCount() const ;

    /**
      Requests function data from AddInCollection

      Logs error message on failure for debugging purposes
    */
    virtual void initArgumentInfo()  const;

    /**
      Returns true if parameter at given position is optional

      @param _nPos
      Position of the parameter

      @return   true if optional, false if not optional
    */
    virtual bool isParameterOptional(sal_uInt32 _nPos) const ;


    /**
      Stores whether a parameter is optional or suppressed
    */
    struct ParameterFlags
    {
        bool    bOptional   :1;     // Parameter is optional
        bool    bSuppress   :1;     // Suppress parameter in UI because not implemented yet

        ParameterFlags() : bOptional(false), bSuppress(false) {}
    };



    ::rtl::OUString      *pFuncName;              // Function name
    ::rtl::OUString      *pFuncDesc;              // Description of function
    ::rtl::OUString     **ppDefArgNames;          // Parameter name(s)
    ::rtl::OUString     **ppDefArgDescs;          // Description(s) of parameter(s)
    ParameterFlags       *pDefArgFlags;           // Flags for each parameter
    sal_uInt16            nFIndex;                // Unique function index
    sal_uInt16            nCategory;              // Function category
    sal_uInt16            nArgCount;              // All parameter count, suppressed and unsuppressed
    sal_uInt16            nHelpId;                // HelpId of function
    bool                  bIncomplete         :1; // Incomplete argument info (set for add-in info from configuration)
    bool                  bHasSuppressedArgs  :1; // Whether there is any suppressed parameter.
};



//============================================================================

class ScFunctionList
{
public:
    ScFunctionList();
    ~ScFunctionList();

    sal_uInt32           GetCount() const
                    { return aFunctionList.Count(); }

    const ScFuncDesc*   First()
                        { return (const ScFuncDesc*) aFunctionList.First(); }

    const ScFuncDesc*   Next()
                        { return (const ScFuncDesc*) aFunctionList.Next(); }

    const ScFuncDesc* GetFunction( sal_uInt32 nIndex ) const
        { return static_cast<const ScFuncDesc*>(aFunctionList.GetObject(nIndex)); }

    ScFuncDesc* GetFunction( sal_uInt32 nIndex )
        { return static_cast<ScFuncDesc*>(aFunctionList.GetObject(nIndex)); }

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

    static ::rtl::OUString       GetCategoryName(sal_uInt32 _nCategoryNumber );

    const ScFuncDesc*   Get( const ::rtl::OUString& rFName ) const;
    const ScFuncDesc*   Get( sal_uInt16 nFIndex ) const;
    const ScFuncDesc*   First( sal_uInt16 nCategory = 0 ) const;
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
