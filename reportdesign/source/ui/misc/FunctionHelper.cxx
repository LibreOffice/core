/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#include "precompiled_rptui.hxx"
#include "FunctionHelper.hxx"
#include <tools/debug.hxx>

// =============================================================================
namespace rptui
{
// =============================================================================
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
            return sal_Unicode('(');
        case eClose:
            return sal_Unicode(')');
        case eSep:
            return sal_Unicode(';');
        case eArrayOpen:
            return sal_Unicode('{');
        case eArrayClose:
            return sal_Unicode('}');
    } // switch(_eToken)
    return 0;
}
// -----------------------------------------------------------------------------
sal_uInt32 FunctionManager::getCount() const
{
    return m_xMgr->getCount();
}
// -----------------------------------------------------------------------------
const formula::IFunctionCategory* FunctionManager::getCategory(sal_uInt32 _nPos) const
{
    if ( _nPos >= m_aCategoryIndex.size() )
    {
        uno::Reference< report::meta::XFunctionCategory> xCategory = m_xMgr->getCategory(_nPos);
        ::boost::shared_ptr< FunctionCategory > pCategory(new FunctionCategory(this,_nPos + 1,xCategory));
        m_aCategoryIndex.push_back( m_aCategories.insert(TCategoriesMap::value_type(xCategory->getName(),pCategory)).first );
    }
    return m_aCategoryIndex[_nPos]->second.get();
}
// -----------------------------------------------------------------------------
const formula::IFunctionDescription* FunctionManager::getFunctionByName(const ::rtl::OUString& _sFunctionName) const
{
    const formula::IFunctionDescription* pDesc = NULL;
    try
    {
        pDesc = get(m_xMgr->getFunctionByName(_sFunctionName)).get();
    }
    catch(uno::Exception&)
    {
    }
    return pDesc;
}
// -----------------------------------------------------------------------------
void FunctionManager::fillLastRecentlyUsedFunctions(::std::vector< const formula::IFunctionDescription*>& /*_rLastRUFunctions*/) const
{
    //const sal_uInt32 nCount = getCount();
    //for(sal_uInt32 i = 0 ; i < nCount ; ++i)
    //{
    //    const formula::IFunctionCategory* pCategory = getCategory(
    //}
}
// -----------------------------------------------------------------------------
::boost::shared_ptr< FunctionDescription > FunctionManager::get(const uno::Reference< report::meta::XFunctionDescription>& _xFunctionDescription) const
{
    ::boost::shared_ptr< FunctionDescription > pDesc;
    if ( _xFunctionDescription.is() )
    {
        const ::rtl::OUString sFunctionName = _xFunctionDescription->getName();
        TFunctionsMap::const_iterator aFunctionFind = m_aFunctions.find(sFunctionName);
        if ( aFunctionFind == m_aFunctions.end() )
        {
            const uno::Reference< report::meta::XFunctionCategory> xCategory = _xFunctionDescription->getCategory();
            const ::rtl::OUString sCategoryName = xCategory->getName();
            TCategoriesMap::iterator aCategoryFind = m_aCategories.find(sCategoryName);
            if ( aCategoryFind == m_aCategories.end() )
            {
                aCategoryFind = m_aCategories.insert(TCategoriesMap::value_type(sCategoryName,::boost::shared_ptr< FunctionCategory > (new FunctionCategory(this,xCategory->getNumber() + 1,xCategory)))).first;
                m_aCategoryIndex.push_back( aCategoryFind );
            }
            aFunctionFind = m_aFunctions.insert(TFunctionsMap::value_type(sFunctionName,::boost::shared_ptr<FunctionDescription>(new FunctionDescription(aCategoryFind->second.get(),_xFunctionDescription)))).first;
        } // if ( aFind == m_aFunctions.end() )
        pDesc = aFunctionFind->second;
    } // if ( _xFunctionDescription.is() )
    return pDesc;
}
// -----------------------------------------------------------------------------
FunctionCategory::FunctionCategory(const FunctionManager* _pFMgr,sal_uInt32 _nPos,const uno::Reference< report::meta::XFunctionCategory>& _xCategory)
: m_xCategory(_xCategory)
,m_nFunctionCount(_xCategory->getCount())
, m_nNumber(_nPos)
,m_pFunctionManager(_pFMgr)
{
}
// -----------------------------------------------------------------------------
sal_uInt32 FunctionCategory::getCount() const
{
    return m_nFunctionCount;
}
// -----------------------------------------------------------------------------
const formula::IFunctionDescription* FunctionCategory::getFunction(sal_uInt32 _nPos) const
{
    if ( _nPos >= m_aFunctions.size() && _nPos < m_nFunctionCount )
    {
        uno::Reference< report::meta::XFunctionDescription> xFunctionDescription = m_xCategory->getFunction(_nPos);
        ::boost::shared_ptr< FunctionDescription > pFunction = m_pFunctionManager->get(xFunctionDescription);
        m_aFunctions.push_back( pFunction );
    }
    return m_aFunctions[_nPos].get();
}
// -----------------------------------------------------------------------------
sal_uInt32 FunctionCategory::getNumber() const
{
    return m_nNumber;
}
// -----------------------------------------------------------------------------
const formula::IFunctionManager* FunctionCategory::getFunctionManager() const
{
    return m_pFunctionManager;
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionCategory::getName() const
{
    return m_xCategory->getName();
}
// -----------------------------------------------------------------------------
FunctionDescription::FunctionDescription(const formula::IFunctionCategory* _pFunctionCategory,const uno::Reference< report::meta::XFunctionDescription>& _xFunctionDescription)
: m_xFunctionDescription(_xFunctionDescription)
, m_pFunctionCategory(_pFunctionCategory)
{
    m_aParameter = m_xFunctionDescription->getArguments();
}
::rtl::OUString FunctionDescription::getFunctionName() const
{
    return m_xFunctionDescription->getName();
}
// -----------------------------------------------------------------------------
const formula::IFunctionCategory* FunctionDescription::getCategory() const
{
    return m_pFunctionCategory;
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionDescription::getDescription() const
{
    return m_xFunctionDescription->getDescription();
}
// -----------------------------------------------------------------------------
xub_StrLen FunctionDescription::getSuppressedArgumentCount() const
{
    return static_cast<xub_StrLen>(m_aParameter.getLength());
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionDescription::getFormula(const ::std::vector< ::rtl::OUString >& _aArguments) const
{
    ::rtl::OUString sFormula;
    try
    {
        const ::rtl::OUString *pArguments = _aArguments.empty() ? 0 : &_aArguments[0];
        sFormula = m_xFunctionDescription->createFormula(uno::Sequence< ::rtl::OUString >(pArguments, _aArguments.size()));
    }
    catch(const uno::Exception&)
    {
        DBG_ERROR("Exception caught!");
    }
    return sFormula;
}
// -----------------------------------------------------------------------------
void FunctionDescription::fillVisibleArgumentMapping(::std::vector<sal_uInt16>& _rArguments) const
{
    const sal_Int32 nCount = m_aParameter.getLength();
    for(sal_uInt16 i = 0;i < nCount; ++i)
    {
        _rArguments.push_back(i);
    }
}
// -----------------------------------------------------------------------------
void FunctionDescription::initArgumentInfo()  const
{
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionDescription::getSignature() const
{
    return m_xFunctionDescription->getSignature();
}
// -----------------------------------------------------------------------------
rtl::OString FunctionDescription::getHelpId() const
{
    return rtl::OString();
}
// -----------------------------------------------------------------------------
sal_uInt32 FunctionDescription::getParameterCount() const
{
    return m_aParameter.getLength();
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionDescription::getParameterName(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].Name;
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
::rtl::OUString FunctionDescription::getParameterDescription(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].Description;
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
bool FunctionDescription::isParameterOptional(sal_uInt32 _nPos) const
{
    if ( _nPos < static_cast<sal_uInt32>(m_aParameter.getLength()) )
        return m_aParameter[_nPos].IsOptional;
    return false;
}
// -----------------------------------------------------------------------------
// =============================================================================
} // rptui
// =============================================================================
