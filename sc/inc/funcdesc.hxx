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

#ifndef INCLUDED_SC_INC_FUNCDESC_HXX
#define INCLUDED_SC_INC_FUNCDESC_HXX

/* Function descriptions for function wizard / autopilot */

#include "scfuncs.hrc"

#include <formula/IFunctionDescription.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <map>

#define MAX_FUNCCAT 12  /* maximum number of categories for functions */
#define LRU_MAX 10 /* maximal number of last recently used functions */

class ScFuncDesc;
class ScFunctionList;
class ScFunctionCategory;
class ScFunctionMgr;

/**
  Stores and generates human readable descriptions for spreadsheet-functions,
  e.g.\ functions used in formulas in calc
*/
class ScFuncDesc : public formula::IFunctionDescription
{
public:
    ScFuncDesc();
    virtual ~ScFuncDesc();

    /**
      Clears the object

      Deletes all objects referenced by the pointers in the class,
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
    virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const override ;

    /**
      Returns the category of the function

      @return    the category of the function
    */
    virtual const formula::IFunctionCategory* getCategory() const override ;

    /**
      Returns the description of the function

      @return    the description of the function, or an empty OUString if there is no description
    */
    virtual OUString getDescription() const override ;

    /**
      Returns the function signature with parameters from the passed string array.

      @return    function signature with parameters
    */
    virtual OUString getFormula(const ::std::vector< OUString >& _aArguments) const override ;

    /**
      Returns the name of the function

      @return    the name of the function, or an empty OUString if there is no name
    */
    virtual OUString getFunctionName() const override ;

    /**
      Returns the help id of the function

      @return   help id of the function
    */
    virtual OString getHelpId() const override ;

    /** Returns whether function is hidden and not offered in the Function
        Wizard unless used in an expression.

        @return flag whether function is hidden
     */
    virtual bool isHidden() const override;

    /**
      Returns number of arguments

      @return   number of arguments
    */
    virtual sal_uInt32 getParameterCount() const override ;

    /**
      Returns start of variable arguments

      @return   start of variable arguments
    */
    virtual sal_uInt32 getVarArgsStart() const override ;

    /**
      Returns description of parameter at given position

      @param _nPos
      Position of the parameter

      @return   OUString description of the parameter
    */
    virtual OUString getParameterDescription(sal_uInt32 _nPos) const override ;

    /**
      Returns name of parameter at given position

      @param _nPos
      Position of the parameter

      @return   OUString name of the parameter
    */
    virtual OUString getParameterName(sal_uInt32 _nPos) const override ;

    /**
      Returns list of all parameter names

      @return OUString containing separated list of all parameter names
    */
    OUString GetParamList() const;

    /**
      Returns the full function signature

      @return   OUString of the form "FUNCTIONNAME( parameter list )"
    */
    virtual OUString getSignature() const override ;

    /**
      Returns the number of non-suppressed arguments

      In case there are variable arguments the number of fixed non-suppressed
      arguments plus VAR_ARGS, same as for nArgCount (variable arguments can't
      be suppressed). The two functions are equal apart from return type and
      name.

      @return    number of non-suppressed arguments
    */
    sal_uInt16  GetSuppressedArgCount() const;
    virtual sal_Int32 getSuppressedArgumentCount() const override ;

    /**
      Requests function data from AddInCollection

      Logs error message on failure for debugging purposes
    */
    virtual void initArgumentInfo()  const override;

    /**
      Returns true if parameter at given position is optional

      @param _nPos
      Position of the parameter

      @return   true if optional, false if not optional
    */
    virtual bool isParameterOptional(sal_uInt32 _nPos) const override ;

    /**
      Compares functions by name, respecting special characters

      @param a
      pointer to first function descriptor

      @param b
      pointer to second function descriptor

      @return "(a < b)"
    */
    static bool compareByName(const ScFuncDesc* a, const ScFuncDesc* b);

    /**
      Stores whether a parameter is optional or suppressed
    */
    struct ParameterFlags
    {
        bool    bOptional   :1;     /**< Parameter is optional */
        bool    bSuppress   :1;     /**< Suppress parameter in UI because not implemented yet */

        ParameterFlags() : bOptional(false), bSuppress(false) {}
    };

    OUString      *pFuncName;              /**< Function name */
    OUString      *pFuncDesc;              /**< Description of function */
    std::vector<OUString> maDefArgNames;          /**< Parameter name(s) */
    std::vector<OUString> maDefArgDescs;          /**< Description(s) of parameter(s) */
    ParameterFlags       *pDefArgFlags;           /**< Flags for each parameter */
    sal_uInt16            nFIndex;                /**< Unique function index */
    sal_uInt16            nCategory;              /**< Function category */
    sal_uInt16            nArgCount;              /**< All parameter count, suppressed and unsuppressed */
    sal_uInt16            nVarArgsStart;          /**< Start of variable arguments, for numbering */
    OString          sHelpId;                /**< HelpId of function */
    bool                  bIncomplete         :1; /**< Incomplete argument info (set for add-in info from configuration) */
    bool                  bHasSuppressedArgs  :1; /**< Whether there is any suppressed parameter. */
    bool                  mbHidden            :1; /**< Whether function is hidden */
};

/**
  List of spreadsheet functions.
  Generated by retrieving functions from resources, AddIns and StarOne AddIns,
  and storing these in one linked list. Functions can be retrieved by index and
  by iterating through the list, starting at the First element, and retrieving
  the Next elements one by one.

  The length of the longest function name can be retrieved for easier
  processing (i.e printing a function list).
*/
class ScFunctionList
{
public:
    ScFunctionList();
    ~ScFunctionList();

    sal_uInt32 GetCount() const
               { return aFunctionList.size(); }

    const ScFuncDesc* First();

    const ScFuncDesc* Next();

    const ScFuncDesc* GetFunction( sal_uInt32 nIndex ) const;

private:
    ::std::vector<const ScFuncDesc*> aFunctionList; /**< List of functions */
    ::std::vector<const ScFuncDesc*>::iterator aFunctionListIter; /**< position in function list */
    sal_Int32  nMaxFuncNameLen; /**< Length of longest function name */
};

/**
  Category of spreadsheet functions.

  Contains the name, index and function manager of a category,
  as well as a list of functions in the category
*/
class ScFunctionCategory : public formula::IFunctionCategory
{
public:
    ScFunctionCategory(::std::vector<const ScFuncDesc*>* _pCategory,sal_uInt32 _nCategory)
            : m_pCategory(_pCategory),m_nCategory(_nCategory){}
    virtual ~ScFunctionCategory(){}

    /**
      @return count of functions in this category
    */
    virtual sal_uInt32 getCount() const override;

    /**
      Gives the _nPos'th function in this category.

      @param _nPos
      position of function in this category.

      @return function at the _nPos position in this category, null if _nPos out of bounds.
    */
    virtual const formula::IFunctionDescription* getFunction(sal_uInt32 _nPos) const override;

    /**
      @return index number of this category.
    */
    virtual sal_uInt32 getNumber() const override;
    virtual OUString getName() const override;

private:
    ::std::vector<const ScFuncDesc*>* m_pCategory; /**< list of functions in this category */
    mutable OUString m_sName; /**< name of this category */
    sal_uInt32 m_nCategory; /**< index number of this category */
};

#define SC_FUNCGROUP_COUNT  ID_FUNCTION_GRP_ADDINS
/**
  Stores spreadsheet functions in categories, including a cumulative ('All') category and makes them accessible.
*/
class ScFunctionMgr : public formula::IFunctionManager
{
public:
    /**
      Retrieves all calc functions, generates cumulative ('All') category, and the categories.

      The function lists of the categories are sorted by (case insensitive) function name
    */
            ScFunctionMgr();
    virtual ~ScFunctionMgr();

    /**
      Returns name of category.

      @param _nCategoryNumber
      index of category

      @return name of the category specified by _nCategoryNumber, empty string if _nCategoryNumber out of bounds
    */
    static OUString GetCategoryName(sal_uInt32 _nCategoryNumber );

    /**
      Returns function by index.

      Searches for a function with the function index nFIndex.

      @param nFIndex
      index of the function

      @return pointer to function with the index nFIndex, null if no such function was found.
    */
    const ScFuncDesc* Get( sal_uInt16 nFIndex ) const;

    /**
      Returns the first function in category nCategory.

      Selects nCategory as current category and returns first element of this.

      @param nCategory
      index of requested category

      @return pointer to first element in current category, null if nCategory out of bounds
    */
    const ScFuncDesc* First( sal_uInt16 nCategory = 0 ) const;

    /**
      Returns the next function of the current category.

      @return pointer to the next function in current category, null if current category not set.
    */
    const ScFuncDesc* Next() const;

    /**
      @return number of categories, not counting the cumulative category ('All')
    */
    virtual sal_uInt32 getCount() const override;

    /**
      Returns a category.

      Returns an IFunctionCategory object for a category specified by nPos.

      @param nPos
      the index of the category, note that 0 maps to the first category not the cumulative ('All') category.

      @return pointer to an IFunctionCategory object, null if nPos out of bounds.
    */
    virtual const formula::IFunctionCategory* getCategory(sal_uInt32 nPos) const override;

    /**
      Appends the last recently used functions.

      Takes the last recently used functions, but maximal LRU_MAX, and appends them to the given vector _rLastRUFunctions.

      @param _rLastRUFunctions
      a vector of pointer to IFunctionDescription, by reference.
    */
    virtual void fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const override;

    /**
      Maps Etoken to character

      Used for retrieving characters for parantheses and separators.

      @param _eToken
      token for which, the corresponding character is retrieved

      @return character
    */
    virtual sal_Unicode getSingleToken(const formula::IFunctionManager::EToken _eToken) const override;

private:
    ScFunctionList* pFuncList; /**< list of all calc functions */
    std::vector<const ScFuncDesc*>* aCatLists[MAX_FUNCCAT]; /**< array of all categories, 0 is the cumulative ('All') category */
    mutable std::map< sal_uInt32, std::shared_ptr<ScFunctionCategory> > m_aCategories; /**< map of category pos to IFunctionCategory */
    mutable std::vector<const ScFuncDesc*>::iterator pCurCatListIter; /**< position in current category */
    mutable std::vector<const ScFuncDesc*>::iterator pCurCatListEnd; /**< end of current category */
};

#endif // INCLUDED_SC_INC_FUNCDESC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
