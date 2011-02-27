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

#include <svl/asiancfg.hxx>
#include <svl/svarray.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/debug.hxx>

//-----------------------------------------------------------------------------
using namespace utl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

using ::rtl::OUString;

#define C2U(cChar) OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))

//-----------------------------------------------------------------------------
struct SvxForbiddenStruct_Impl
{
    Locale      aLocale;
    OUString    sStartChars;
    OUString    sEndChars;
};
//-----------------------------------------------------------------------------
typedef SvxForbiddenStruct_Impl* SvxForbiddenStruct_ImplPtr;
SV_DECL_PTRARR_DEL(SvxForbiddenStructArr, SvxForbiddenStruct_ImplPtr, 2, 2)
SV_IMPL_PTRARR(SvxForbiddenStructArr, SvxForbiddenStruct_ImplPtr);
//-----------------------------------------------------------------------------
struct SvxAsianConfig_Impl
{
    sal_Bool    bKerningWesternTextOnly;
    sal_Int16   nCharDistanceCompression;

    SvxForbiddenStructArr   aForbiddenArr;

    SvxAsianConfig_Impl() :
        bKerningWesternTextOnly(sal_True),
        nCharDistanceCompression(0) {}
};

Sequence<OUString> lcl_GetPropertyNames()
{
    Sequence<OUString> aNames(2);
    OUString* pNames = aNames.getArray();
    pNames[0] = C2U("IsKerningWesternTextOnly");
    pNames[1] = C2U("CompressCharacterDistance");
    return aNames;;
}
// ---------------------------------------------------------------------------
SvxAsianConfig::SvxAsianConfig(sal_Bool bEnableNotify) :
    utl::ConfigItem(C2U("Office.Common/AsianLayout")),
    pImpl(new SvxAsianConfig_Impl)
{
    if(bEnableNotify)
        EnableNotification(lcl_GetPropertyNames());
    Load();
}

SvxAsianConfig::~SvxAsianConfig()
{
    delete pImpl;
}

void SvxAsianConfig::Load()
{
    Sequence<Any> aValues = GetProperties(lcl_GetPropertyNames());
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        pImpl->bKerningWesternTextOnly = *(sal_Bool*) pValues[0].getValue();
    pValues[1] >>= pImpl->nCharDistanceCompression;

    pImpl->aForbiddenArr.DeleteAndDestroy(0, pImpl->aForbiddenArr.Count());
    OUString sPropPrefix(C2U("StartEndCharacters"));
    Sequence<OUString> aNodes = GetNodeNames(sPropPrefix);

    Sequence<OUString> aPropNames(aNodes.getLength() * 2);
    OUString* pNames = aPropNames.getArray();
    sal_Int32 nName = 0;
    sPropPrefix += C2U("/");
    sal_Int32 nNode;
    const OUString* pNodes = aNodes.getConstArray();
    for(nNode = 0; nNode < aNodes.getLength(); nNode++)
    {
        OUString sStart(sPropPrefix);
        sStart += pNodes[nNode];
        sStart += C2U("/");
        pNames[nName] = sStart;     pNames[nName++] += C2U("StartCharacters");
        pNames[nName] = sStart;     pNames[nName++] += C2U("EndCharacters");
    }
    Sequence<Any> aNodeValues = GetProperties(aPropNames);
    const Any* pNodeValues = aNodeValues.getConstArray();
    nName = 0;
    for(nNode = 0; nNode < aNodes.getLength(); nNode++)
    {
        SvxForbiddenStruct_ImplPtr pInsert = new SvxForbiddenStruct_Impl;
        pInsert->aLocale.Language = pNodes[nNode].copy(0, 2);
        DBG_ASSERT(pInsert->aLocale.Language.getLength(), "illegal language");
        pInsert->aLocale.Country = pNodes[nNode].copy(3, 2);

        pNodeValues[nName++] >>= pInsert->sStartChars;
        pNodeValues[nName++] >>= pInsert->sEndChars;
        pImpl->aForbiddenArr.Insert(pInsert, pImpl->aForbiddenArr.Count());
    }
}

void    SvxAsianConfig::Notify( const Sequence<OUString>& )
{
    Load();
}

void SvxAsianConfig::Commit()
{
    Sequence<Any> aValues(2);
    Any* pValues = aValues.getArray();
    pValues[0].setValue(&pImpl->bKerningWesternTextOnly, ::getBooleanCppuType());
    pValues[1] <<= pImpl->nCharDistanceCompression;
    PutProperties(lcl_GetPropertyNames(), aValues);


    OUString sNode(C2U("StartEndCharacters"));
    if(!pImpl->aForbiddenArr.Count())
        ClearNodeSet(sNode);
    else
    {
        Sequence<PropertyValue> aSetValues(2 * pImpl->aForbiddenArr.Count());
        PropertyValue* pSetValues = aSetValues.getArray();
        sal_Int32 nSetValue = 0;
        const OUString sStartChars(C2U("StartCharacters"));
        const OUString sEndChars(C2U("EndCharacters"));
        for(sal_uInt16 i = 0; i < pImpl->aForbiddenArr.Count(); i++)
        {
            OUString sPrefix(sNode);
            sPrefix += C2U("/");
            sPrefix += pImpl->aForbiddenArr[i]->aLocale.Language;
            DBG_ASSERT(pImpl->aForbiddenArr[i]->aLocale.Language.getLength(), "illegal language");
            sPrefix += C2U("-");
            sPrefix += pImpl->aForbiddenArr[i]->aLocale.Country;
            sPrefix += C2U("/");
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sStartChars;
            pSetValues[nSetValue++].Value <<= pImpl->aForbiddenArr[i]->sStartChars;
            pSetValues[nSetValue].Name = sPrefix; pSetValues[nSetValue].Name += sEndChars;
            pSetValues[nSetValue++].Value <<= pImpl->aForbiddenArr[i]->sEndChars;
        }
        ReplaceSetProperties(sNode, aSetValues);
    }
}

sal_Bool    SvxAsianConfig::IsKerningWesternTextOnly() const
{
    return pImpl->bKerningWesternTextOnly;
}

void        SvxAsianConfig::SetKerningWesternTextOnly(sal_Bool bSet)
{
    pImpl->bKerningWesternTextOnly = bSet;
    SetModified();
}

sal_Int16   SvxAsianConfig::GetCharDistanceCompression() const
{
    return pImpl->nCharDistanceCompression;
}

void        SvxAsianConfig::SetCharDistanceCompression(sal_Int16 nSet)
{
    DBG_ASSERT(nSet >= 0 && nSet < 3, "compression value illegal");
    SetModified();
    pImpl->nCharDistanceCompression = nSet;
}

uno::Sequence<lang::Locale> SvxAsianConfig::GetStartEndCharLocales()
{
    Sequence<Locale> aRet(pImpl->aForbiddenArr.Count());
    Locale* pRet = aRet.getArray();
    for(sal_uInt16 i = 0; i < pImpl->aForbiddenArr.Count(); i++)
    {
        pRet[i] = pImpl->aForbiddenArr[i]->aLocale;
    }
    return aRet;
}

sal_Bool    SvxAsianConfig::GetStartEndChars( const Locale& rLocale,
                                    OUString& rStartChars,
                                    OUString& rEndChars )
{
    for(sal_uInt16 i = 0; i < pImpl->aForbiddenArr.Count(); i++)
    {
        if(rLocale.Language == pImpl->aForbiddenArr[i]->aLocale.Language &&
            rLocale.Country == pImpl->aForbiddenArr[i]->aLocale.Country)
        {
            rStartChars = pImpl->aForbiddenArr[i]->sStartChars;
            rEndChars = pImpl->aForbiddenArr[i]->sEndChars;
            return sal_True;
        }
    }
    return sal_False;
}

void SvxAsianConfig::SetStartEndChars( const Locale& rLocale,
                                    const OUString* pStartChars,
                                    const OUString* pEndChars )
{
    sal_Bool bFound = sal_False;
    for(sal_uInt16 i = 0; i < pImpl->aForbiddenArr.Count(); i++)
    {
        if(rLocale.Language == pImpl->aForbiddenArr[i]->aLocale.Language &&
            rLocale.Country == pImpl->aForbiddenArr[i]->aLocale.Country)
        {
            if(pStartChars && pEndChars)
            {
                pImpl->aForbiddenArr[i]->sStartChars = *pStartChars;
                pImpl->aForbiddenArr[i]->sEndChars = *pEndChars;
            }
            else
                pImpl->aForbiddenArr.DeleteAndDestroy(i, 1);
            bFound = sal_True;
        }
    }
    if(!bFound && pStartChars && pEndChars)
    {
        SvxForbiddenStruct_ImplPtr pInsert = new SvxForbiddenStruct_Impl;
        pInsert->aLocale = rLocale;
        pInsert->sStartChars = *pStartChars;
        pInsert->sEndChars = *pEndChars;
        pImpl->aForbiddenArr.Insert(pInsert, pImpl->aForbiddenArr.Count());
    }
#ifdef DBG_UTIL
    else if(!bFound)
        DBG_ERROR("attempt to clear unavailable data");
#endif
    SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
