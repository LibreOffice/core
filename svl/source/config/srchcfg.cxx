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
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

using ::rtl::OUString;

#define C2U(cChar) OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

//-----------------------------------------------------------------------------
typedef SvxSearchEngineData* SvxSearchEngineDataPtr;
SV_DECL_PTRARR_DEL(SvxSearchEngineArr, SvxSearchEngineDataPtr, 2, 2)
SV_IMPL_PTRARR(SvxSearchEngineArr, SvxSearchEngineDataPtr);
//-----------------------------------------------------------------------------
struct SvxSearchConfig_Impl
{
    SvxSearchEngineArr   aEngineArr;
};

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

SvxSearchConfig::~SvxSearchConfig()
{
    delete pImpl;
}

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

void    SvxSearchConfig::Notify( const Sequence<OUString>& )
{
    Load();
}

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

sal_uInt16 SvxSearchConfig::Count()
{
    return pImpl->aEngineArr.Count();
}

const SvxSearchEngineData&  SvxSearchConfig::GetData(sal_uInt16 nPos)
{
    DBG_ASSERT(nPos < pImpl->aEngineArr.Count(), "wrong array index");
    return *pImpl->aEngineArr[nPos];
}

const SvxSearchEngineData*  SvxSearchConfig::GetData(const rtl::OUString& rEngineName)
{
    for(sal_uInt16 nPos = 0; nPos < pImpl->aEngineArr.Count(); nPos++)
    {
        if(pImpl->aEngineArr[nPos]->sEngineName == rEngineName)
            return pImpl->aEngineArr[nPos];
    }
    return 0;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
