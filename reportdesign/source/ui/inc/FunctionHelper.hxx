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

#ifndef RPTUI_FUNCTIONHELPER_HXX
#define RPTUI_FUNCTIONHELPER_HXX

#include <formula/IFunctionDescription.hxx>
#include <com/sun/star/report/meta/XFunctionManager.hpp>
#include <com/sun/star/report/meta/XFunctionCategory.hpp>
#include <com/sun/star/report/meta/XFunctionDescription.hpp>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>


namespace rptui
{
//============================================================================
class FunctionCategory;
class FunctionDescription;
//============================================================================
class FunctionManager : public formula::IFunctionManager
{
    DECLARE_STL_USTRINGACCESS_MAP( ::boost::shared_ptr< FunctionDescription >,  TFunctionsMap);
    DECLARE_STL_USTRINGACCESS_MAP( ::boost::shared_ptr< FunctionCategory > ,    TCategoriesMap);
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionManager> m_xMgr;
    mutable TCategoriesMap  m_aCategories;
    mutable ::std::vector< TCategoriesMap::iterator > m_aCategoryIndex;
    mutable TFunctionsMap   m_aFunctions;
public:
            FunctionManager(const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionManager>& _xMgr);
    virtual ~FunctionManager();
    virtual sal_uInt32                              getCount() const;
    virtual const formula::IFunctionCategory*       getCategory(sal_uInt32 nPos) const;
    virtual void                                    fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& _rLastRUFunctions) const;
    virtual const formula::IFunctionDescription*    getFunctionByName(const ::rtl::OUString& _sFunctionName) const;
    virtual sal_Unicode                       getSingleToken(const EToken _eToken) const;

    ::boost::shared_ptr< FunctionDescription >      get(const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription>& _xFunctionDescription) const;
};
//============================================================================
class FunctionDescription : public formula::IFunctionDescription
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FunctionArgument > m_aParameter;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription> m_xFunctionDescription;
    const formula::IFunctionCategory* m_pFunctionCategory;
public:
    FunctionDescription(const formula::IFunctionCategory* _pFunctionCategory,const ::com::sun::star::uno::Reference< ::com::sun::star::report::meta::XFunctionDescription>& _xFunctionDescription);
    virtual ~FunctionDescription(){}

    virtual ::rtl::OUString getFunctionName() const ;
    virtual const formula::IFunctionCategory* getCategory() const ;
    virtual ::rtl::OUString getDescription() const ;
    virtual xub_StrLen getSuppressedArgumentCount() const ;
    virtual ::rtl::OUString getFormula(const ::std::vector< ::rtl::OUString >& _aArguments) const ;
    virtual void fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const ;
    virtual void initArgumentInfo()  const;
    virtual ::rtl::OUString getSignature() const ;
    virtual rtl::OString getHelpId() const ;
    virtual sal_uInt32 getParameterCount() const ;
    virtual ::rtl::OUString getParameterName(sal_uInt32 _nPos) const ;
    virtual ::rtl::OUString getParameterDescription(sal_uInt32 _nPos) const ;
    virtual bool isParameterOptional(sal_uInt32 _nPos) const ;
};
//============================================================================
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

    virtual sal_uInt32                              getCount() const;
    virtual const formula::IFunctionDescription*    getFunction(sal_uInt32 _nPos) const;
    virtual sal_uInt32                              getNumber() const;
    virtual const formula::IFunctionManager*        getFunctionManager() const;
    virtual ::rtl::OUString                         getName() const;
};
// =============================================================================
} // rptui
// =============================================================================

#endif //RPTUI_FUNCTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
