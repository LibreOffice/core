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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FUNCTIONHELPER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FUNCTIONHELPER_HXX

#include <formula/IFunctionDescription.hxx>
#include <com/sun/star/report/meta/XFunctionManager.hpp>
#include <com/sun/star/report/meta/XFunctionCategory.hpp>
#include <com/sun/star/report/meta/XFunctionDescription.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace rptui
{

class FunctionCategory;
class FunctionDescription;

class FunctionManager : public formula::IFunctionManager
{
    typedef std::map< OUString, ::boost::shared_ptr< FunctionDescription > > TFunctionsMap;
    typedef std::map< OUString, ::boost::shared_ptr< FunctionCategory > > TCategoriesMap;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionManager> m_xMgr;
    mutable TCategoriesMap  m_aCategories;
    mutable ::std::vector< TCategoriesMap::iterator > m_aCategoryIndex;
    mutable TFunctionsMap   m_aFunctions;
public:
            FunctionManager(const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionManager>& _xMgr);
    virtual ~FunctionManager();
    virtual sal_uInt32                              getCount() const SAL_OVERRIDE;
    virtual const formula::IFunctionCategory*       getCategory(sal_uInt32 nPos) const SAL_OVERRIDE;
    virtual void                                    fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const SAL_OVERRIDE;
    virtual sal_Unicode                             getSingleToken(const EToken _eToken) const SAL_OVERRIDE;

    ::boost::shared_ptr< FunctionDescription >      get(const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription>& _xFunctionDescription) const;
};

class FunctionDescription : public formula::IFunctionDescription
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FunctionArgument > m_aParameter;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription> m_xFunctionDescription;
    const formula::IFunctionCategory* m_pFunctionCategory;
public:
    FunctionDescription(const formula::IFunctionCategory* _pFunctionCategory,const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription>& _xFunctionDescription);
    virtual ~FunctionDescription(){}

    virtual OUString getFunctionName() const SAL_OVERRIDE ;
    virtual const formula::IFunctionCategory* getCategory() const SAL_OVERRIDE ;
    virtual OUString getDescription() const SAL_OVERRIDE ;
    virtual sal_Int32 getSuppressedArgumentCount() const SAL_OVERRIDE ;
    virtual OUString getFormula(const ::std::vector< OUString >& _aArguments) const SAL_OVERRIDE ;
    virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const SAL_OVERRIDE ;
    virtual void initArgumentInfo()  const SAL_OVERRIDE;
    virtual OUString getSignature() const SAL_OVERRIDE ;
    virtual OString getHelpId() const SAL_OVERRIDE ;
    virtual sal_uInt32 getParameterCount() const SAL_OVERRIDE ;
    virtual OUString getParameterName(sal_uInt32 _nPos) const SAL_OVERRIDE ;
    virtual OUString getParameterDescription(sal_uInt32 _nPos) const SAL_OVERRIDE ;
    virtual bool isParameterOptional(sal_uInt32 _nPos) const SAL_OVERRIDE ;
};

class FunctionCategory : public formula::IFunctionCategory
{
    mutable ::std::vector< ::boost::shared_ptr< FunctionDescription > > m_aFunctions;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionCategory> m_xCategory;
    sal_uInt32 m_nFunctionCount;
    sal_uInt32 m_nNumber;
    const FunctionManager* m_pFunctionManager;
public:
    FunctionCategory(const FunctionManager* _pFMgr,sal_uInt32 _nPos,const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionCategory>& _xCategory);
    virtual ~FunctionCategory() {}

    virtual sal_uInt32                              getCount() const SAL_OVERRIDE;
    virtual const formula::IFunctionDescription*    getFunction(sal_uInt32 _nPos) const SAL_OVERRIDE;
    virtual sal_uInt32                              getNumber() const SAL_OVERRIDE;
    virtual OUString                                getName() const SAL_OVERRIDE;
};

} // rptui


#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FUNCTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
