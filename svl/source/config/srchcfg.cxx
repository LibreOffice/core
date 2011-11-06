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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include <svl/srchcfg.hxx>
#include <svl/svarray.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/debug.hxx>
#include <unotools/configpathes.hxx>

//-----------------------------------------------------------------------------
using namespace utl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

#define C2U(cChar) OUString::createFromAscii(cChar)

//-----------------------------------------------------------------------------
typedef SvxSearchEngineData* SvxSearchEngineDataPtr;
SV_DECL_PTRARR_DEL(SvxSearchEngineArr, SvxSearchEngineDataPtr, 2, 2)
SV_IMPL_PTRARR(SvxSearchEngineArr, SvxSearchEngineDataPtr);
//-----------------------------------------------------------------------------
struct SvxSearchConfig_Impl
{
    SvxSearchEngineArr   aEngineArr;
};
/* -----------------------------19.03.01 14:00--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SvxSearchEngineData::operator==(const SvxSearchEngineData& rData)
{
    return sEngineName      == rData.sEngineName     &&
            sAndPrefix       == rData.sAndPrefix      &&
            sAndSuffix       == rData.sAndSuffix      &&
            sAndSeparator    == rData.sAndSeparator   &&
            nAndCaseMatch    == rData.nAndCaseMatch   &&
            sOrPrefix        == rData.sOrPrefix       &&
            sOrSuffix        == rData.sOrSuffix       &&
            sOrSeparator     == rData.sOrSeparator    &&
            nOrCaseMatch     == rData.nOrCaseMatch    &&
            sExactPrefix     == rData.sExactPrefix    &&
            sExactSuffix     == rData.sExactSuffix    &&
            sExactSeparator  == rData.sExactSeparator &&
            nExactCaseMatch  == rData.nExactCaseMatch;
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence<OUString>& lcl_GetSearchPropertyNames_Impl()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        aNames.realloc(12);
        OUString* pNames = aNames.getArray();
        pNames[0] = C2U("And/ooInetPrefix");
        pNames[1] = C2U("And/ooInetSuffix");
        pNames[2] = C2U("And/ooInetSeparator");
        pNames[3] = C2U("And/ooInetCaseMatch");
        pNames[4] = C2U("Or/ooInetPrefix");
        pNames[5] = C2U("Or/ooInetSuffix");
        pNames[6] = C2U("Or/ooInetSeparator");
        pNames[7] = C2U("Or/ooInetCaseMatch");
        pNames[8] = C2U("Exact/ooInetPrefix");
        pNames[9] = C2U("Exact/ooInetSuffix");
        pNames[10] = C2U("Exact/ooInetSeparator");
        pNames[11] = C2U("Exact/ooInetCaseMatch");
    }
    return aNames;
}
// ---------------------------------------------------------------------------
SvxSearchConfig::SvxSearchConfig(sal_Bool bEnableNotify) :
    utl::ConfigItem(C2U("Inet/SearchEngines"), CONFIG_MODE_DELAYED_UPDATE),
    pImpl(new SvxSearchConfig_Impl)
{
    if(bEnableNotify)
    {
        //request notifications from the node
        Sequence<OUString> aEnable(1);
        EnableNotification(aEnable);
    }
    Load();
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
SvxSearchConfig::~SvxSearchConfig()
{
    delete pImpl;
}
/* -----------------------------17.01.01 09:57--------------------------------

 ---------------------------------------------------------------------------*/
void SvxSearchConfig::Load()
{
    pImpl->aEngineArr.DeleteAndDestroy(0, pImpl->aEngineArr.Count());
    Sequence<OUString> aNodeNames = GetNodeNames(OUString());
    const OUString* pNodeNames = aNodeNames.getConstArray();
    for(sal_Int32 nNode = 0; nNode < aNodeNames.getLength(); nNode++)
    {
        SvxSearchEngineDataPtr pNew = new SvxSearchEngineData;
        pNew->sEngineName = pNodeNames[nNode];
        const Sequence<OUString>& rPropNames = lcl_GetSearchPropertyNames_Impl();
        const OUString* pPropNames = rPropNames.getConstArray();
        Sequence<OUString> aPropertyNames(rPropNames.getLength());
        OUString* pPropertyNames = aPropertyNames.getArray();
        const OUString sSlash(C2U("/"));
        sal_Int32 nProp;
        for(nProp = 0; nProp < rPropNames.getLength(); nProp++)
        {
            pPropertyNames[nProp] = wrapConfigurationElementName(pNodeNames[nNode]);
            pPropertyNames[nProp] += sSlash;
            pPropertyNames[nProp] += pPropNames[nProp];
        }
        Sequence<Any> aValues = GetProperties(aPropertyNames);
        const Any* pValues = aValues.getConstArray();
        for(nProp = 0; nProp < rPropNames.getLength(); nProp++)
        {
            switch(nProp)
            {
                case 0 : pValues[nProp] >>= pNew->sAndPrefix;      break;
                case 1 : pValues[nProp] >>= pNew->sAndSuffix;      break;
                case 2 : pValues[nProp] >>= pNew->sAndSeparator;   break;
                case 3 : pValues[nProp] >>= pNew->nAndCaseMatch;   break;

                case 4 : pValues[nProp] >>= pNew->sOrPrefix;       break;
                case 5 : pValues[nProp] >>= pNew->sOrSuffix;       break;
                case 6 : pValues[nProp] >>= pNew->sOrSeparator;    break;
                case 7 : pValues[nProp] >>= pNew->nOrCaseMatch;    break;

                case 8 : pValues[nProp] >>= pNew->sExactPrefix;    break;
                case 9 : pValues[nProp] >>= pNew->sExactSuffix;    break;
                case 10: pValues[nProp] >>= pNew->sExactSeparator; break;
                case 11: pValues[nProp] >>= pNew->nExactCaseMatch; break;
            }
        }
        pImpl->aEngineArr.Insert(pNew, pImpl->aEngineArr.Count());
    }
}
/* -----------------------------17.01.01 09:57--------------------------------

 ---------------------------------------------------------------------------*/
void    SvxSearchConfig::Notify( const Sequence<OUString>& )
{
    Load();
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
void SvxSearchConfig::Commit()
{
    OUString sNode;
    if(!pImpl->aEngineArr.Count())
        ClearNodeSet(sNode);
    else
    {
        Sequence<PropertyValue> aSetValues(12 * pImpl->aEngineArr.Count());
        PropertyValue* pSetValues = aSetValues.getArray();

        const Sequence<OUString>& rPropNames = lcl_GetSearchPropertyNames_Impl();
        const OUString* pPropNames = rPropNames.getConstArray();
        const OUString sSlash(C2U("/"));
        for(sal_uInt16 i = 0; i < pImpl->aEngineArr.Count(); i++)
        {
            SvxSearchEngineDataPtr pSave = pImpl->aEngineArr[i];
            for(sal_Int16 nProp = 0; nProp < rPropNames.getLength(); nProp++)
            {
                OUString sTmpName = sSlash;
                sTmpName += wrapConfigurationElementName(pSave->sEngineName);
                sTmpName += sSlash;
                sTmpName += pPropNames[nProp];
                pSetValues[nProp].Name = sTmpName;
                switch(nProp)
                {
                    case 0 : pSetValues[nProp].Value <<= pSave->sAndPrefix;      break;
                    case 1 : pSetValues[nProp].Value <<= pSave->sAndSuffix;      break;
                    case 2 : pSetValues[nProp].Value <<= pSave->sAndSeparator;   break;
                    case 3 : pSetValues[nProp].Value <<= pSave->nAndCaseMatch;   break;

                    case 4 : pSetValues[nProp].Value <<= pSave->sOrPrefix;       break;
                    case 5 : pSetValues[nProp].Value <<= pSave->sOrSuffix;       break;
                    case 6 : pSetValues[nProp].Value <<= pSave->sOrSeparator;    break;
                    case 7 : pSetValues[nProp].Value <<= pSave->nOrCaseMatch;    break;

                    case 8 : pSetValues[nProp].Value <<= pSave->sExactPrefix;    break;
                    case 9 : pSetValues[nProp].Value <<= pSave->sExactSuffix;    break;
                    case 10: pSetValues[nProp].Value <<= pSave->sExactSeparator; break;
                    case 11: pSetValues[nProp].Value <<= pSave->nExactCaseMatch; break;
                }
            }
            pSetValues+= 12;
        }
        ReplaceSetProperties(sNode, aSetValues);
    }
}
/* -----------------------------19.03.01 10:02--------------------------------

 ---------------------------------------------------------------------------*/
sal_uInt16 SvxSearchConfig::Count()
{
    return pImpl->aEngineArr.Count();
}
/* -----------------------------19.03.01 10:02--------------------------------

 ---------------------------------------------------------------------------*/
const SvxSearchEngineData&  SvxSearchConfig::GetData(sal_uInt16 nPos)
{
    DBG_ASSERT(nPos < pImpl->aEngineArr.Count(), "wrong array index");
    return *pImpl->aEngineArr[nPos];
}
/* -----------------------------19.03.01 10:38--------------------------------

 ---------------------------------------------------------------------------*/
const SvxSearchEngineData*  SvxSearchConfig::GetData(const rtl::OUString& rEngineName)
{
    for(sal_uInt16 nPos = 0; nPos < pImpl->aEngineArr.Count(); nPos++)
    {
        if(pImpl->aEngineArr[nPos]->sEngineName == rEngineName)
            return pImpl->aEngineArr[nPos];
    }
    return 0;
}
/* -----------------------------19.03.01 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void  SvxSearchConfig::SetData(const SvxSearchEngineData& rData)
{
    for(sal_uInt16 nPos = 0; nPos < pImpl->aEngineArr.Count(); nPos++)
    {
        if(pImpl->aEngineArr[nPos]->sEngineName == rData.sEngineName)
        {
            if((*pImpl->aEngineArr[nPos]) == rData)
                return;
            pImpl->aEngineArr.DeleteAndDestroy(nPos, 1);
            break;
        }
    }
    SvxSearchEngineDataPtr pInsert = new SvxSearchEngineData(rData);
    pImpl->aEngineArr.Insert(pInsert, pImpl->aEngineArr.Count());
    SetModified();
}
/* -----------------------------19.03.01 10:38--------------------------------

 ---------------------------------------------------------------------------*/
void SvxSearchConfig::RemoveData(const rtl::OUString& rEngineName)
{
    for(sal_uInt16 nPos = 0; nPos < pImpl->aEngineArr.Count(); nPos++)
    {
        if(pImpl->aEngineArr[nPos]->sEngineName == rEngineName)
        {
            pImpl->aEngineArr.DeleteAndDestroy(nPos, 1);
            SetModified();
            return ;
        }
    }
}

