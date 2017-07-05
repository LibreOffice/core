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

#include "FunctionHelper.hxx"

#include <osl/diagnose.h>
#include <formula/funcvarargs.h>


namespace rptui
{

    using namespace ::com::sun::star;

FunctionManager::FunctionManager(const uno::Reference< report::meta::XFunctionManager>& _xMgr)
: m_xMgr(_xMgr)
{
}
FunctionManager::~FunctionManager()
{
}
sal_Unicode FunctionManager::getSingleToken(const formula::IFunctionManager::EToken _eToken) const
{
    switch(_eToken)
    {
        case eOk:
            return '(';
        case eClose:
            return ')';
        case eSep:
            return ';';
        case eArrayOpen:
            return '{';
        case eArrayClose:
            return '}';
    }
    return 0;
}

sal_uInt32 FunctionManager::getCount() const
{
    return m_xMgr->getCount();
}

const formula::IFunctionCategory* FunctionManager::getCategory(sal_uInt32 _nPos) const
{
    if ( _nPos >= m_aCategoryIndex.size() )
    {
        uno::Reference< report::meta::XFunctionCategory> xCategory = m_xMgr->getCategory(_nPos);
        std::shared_ptr< FunctionCategory > pCategory(new FunctionCategory(this,_nPos + 1,xCategory));
        m_aCategoryIndex.push_back( m_aCategories.insert(TCategoriesMap::value_type(xCategory->getName(),pCategory)).first );
    }
    return m_aCategoryIndex[_nPos]->second.get();
}

void FunctionManager::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& /*_rLastRUFunctions*/) const
{
}

std::shared_ptr< FunctionDescription > FunctionManager::get(const uno::Reference< report::meta::XFunctionDescription>& _xFunctionDescription) const
{
    std::shared_ptr< FunctionDescription > pDesc;
    if ( _xFunctionDescription.is() )
    {
        const OUString sFunctionName = _xFunctionDescription->getName();
        TFunctionsMap::const_iterator aFunctionFind = m_aFunctions.find(sFunctionName);
        if ( aFunctionFind == m_aFunctions.end() )
        {
            const uno::Reference< report::meta::XFunctionCategory> xCategory = _xFunctionDescription->getCategory();
            const OUString sCategoryName = xCategory->getName();
            TCategoriesMap::iterator aCategoryFind = m_aCategories.find(sCategoryName);
            if ( aCategoryFind == m_aCategories.end() )
            {
                aCategoryFind = m_aCategories.insert(TCategoriesMap::value_type(sCategoryName,std::make_shared< FunctionCategory > (this,xCategory->getNumber() + 1,xCategory))).first;
                m_aCategoryIndex.push_back( aCategoryFind );
            }
            aFunctionFind = m_aFunctions.insert(TFunctionsMap::value_type(sFunctionName,std::make_shared<FunctionDescription>(aCategoryFind->second.get(),_xFunctionDescription))).first;
        }
        pDesc = aFunctionFind->second;
    }
    return pDesc;
}

FunctionCategory::FunctionCategory(const FunctionManager* _pFMgr,sal_uInt32 _nPos,const uno::Reference< report::meta::XFunctionCategory>& _xCategory)
: m_xCategory(_xCategory)
,m_nFunctionCount(_xCategory->getCount())
, m_nNumber(_nPos)
,m_pFunctionManager(_pFMgr)
{
}

sal_uInt32 FunctionCategory::getCount() const
{
    return m_nFunctionCount;
}

const formula::IFunctionDescription* FunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    if ( _nPos >= m_aFunctions.size() && _nPos < m_nFunctionCount )
    {
        uno::Reference< report::meta::XFunctionDescription> xFunctionDescription = m_xCategory->getFunction(_nPos);
        std::shared_ptr< FunctionDescription > pFunction = m_pFunctionManager->get(xFunctionDescription);
        m_aFunctions.push_back( pFunction );
    }
    return m_aFunctions[_nPos].get();
}

sal_uInt32 FunctionCategory::getNumber() const
{
    return m_nNumber;
}

OUString FunctionCategory::getName() const
{
    return m_xCategory->getName();
}

FunctionDescription::FunctionDescription(const formula::IFunctionCategory* _pFunctionCategory,const uno::Reference< report::meta::XFunctionDescription>& _xFunctionDescription)
: m_xFunctionDescription(_xFunctionDescription)
, m_pFunctionCategory(_pFunctionCategory)
{
    m_aParameter = m_xFunctionDescription->getArguments();
}
OUString FunctionDescription::getFunctionName() const
{
    return m_xFunctionDescription->getName();
}

const formula::IFunctionCategory* FunctionDescription::getCategory() const
{
    return m_pFunctionCategory;
}

OUString FunctionDescription::getDescription() const
{
    return m_xFunctionDescription->getDescription();
}

sal_Int32 FunctionDescription::getSuppressedArgumentCount() const
{
    return m_aParameter.getLength();
}

OUString FunctionDescription::getFormula(const ::std::vector< OUString >& _aArguments) const
{
    OUString sFormula;
    try
    {
        sFormula = m_xFunctionDescription->createFormula(uno::Sequence< OUString >(_aArguments.data(), _aArguments.size()));
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("Exception caught!");
    }
    return sFormula;
}

void FunctionDescription::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    const sal_Int32 nCount = m_aParameter.getLength();
    for(sal_Int32 i = 0;i < nCount; ++i)
    {
        _rArguments.push_back(i);
    }
}

void FunctionDescription::initArgumentInfo()  const
{
}

OUString FunctionDescription::getSignature() const
{
    return m_xFunctionDescription->getSignature();
}

OString FunctionDescription::getHelpId() const
{
    return OString();
}

bool FunctionDescription::isHidden() const
{
    return false;
}

sal_uInt32 FunctionDescription::getParameterCount() const
{
    return m_aParameter.getLength();
}

sal_uInt32 FunctionDescription::getVarArgsStart() const
{
    /* XXX there are no variable number of arguments, are there? Nevertheless
     * consider the varargs handling of the Function Wizard and return a value
     * within the bounds of parameters. */
    // Don't use defines/constants that could change in future, parameter count
    // could be part of an implicit stable API.
    // offapi/com/sun/star/report/meta/XFunctionDescription.idl doesn't tell.
    const sal_uInt32 nVarArgs30 = 30;           // ugly hard coded old VAR_ARGS of formula::ParaWin
    const sal_uInt32 nPairedVarArgs60 = 60;     // ugly hard coded old PAIRED_VAR_ARGS of formula::ParaWin
    const sal_uInt32 nVarArgs255 = 255;         // ugly hard coded new VAR_ARGS of formula::ParaWin
    const sal_uInt32 nPairedVarArgs510 = 510;   // ugly hard coded new PAIRED_VAR_ARGS of formula::ParaWin
    sal_uInt32 nLen = m_aParameter.getLength();
    // If the value of VAR_ARGS changes then adapt *and* maintain implicit API
    // stability, ie. old code using the old VAR_ARGS and PAIRED_VAR_ARGS
    // values must still be handled. It is *not* sufficient to simply change
    // the values here.
    static_assert(nVarArgs255 == VAR_ARGS && nPairedVarArgs510 == PAIRED_VAR_ARGS,
            "VAR_ARGS or PAIRED_VAR_ARGS has unexpected value");
    if (nLen >= nPairedVarArgs510)
        nLen -= nPairedVarArgs510;
    else if (nLen >= nVarArgs255)
        nLen -= nVarArgs255;
    else if (nLen >= nPairedVarArgs60)
        nLen -= nPairedVarArgs60;
    else if (nLen >= nVarArgs30)
        nLen -= nVarArgs30;
    return nLen ? nLen - 1 : 0;
}

OUString FunctionDescription::getParameterName(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].Name;
    return OUString();
}

OUString FunctionDescription::getParameterDescription(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].Description;
    return OUString();
}

bool FunctionDescription::isParameterOptional(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].IsOptional;
    return false;
}


} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
