/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asiancfg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:28:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SVX_ASIANCFG_HXX
#include <asiancfg.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//-----------------------------------------------------------------------------
using namespace utl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

#define C2U(cChar) OUString::createFromAscii(cChar)
const sal_Char sStartEndCharacters[] = "StartEndCharacters";
const sal_Char sStartCharacters[] = "StartCharacters";
const sal_Char sEndCharacters[] = "EndCharacters";

//-----------------------------------------------------------------------------
struct SvxForbiddenStruct_Impl
{
    Locale      aLocale;
    OUString    sStartChars;
    OUString    sEndChars;
};
//-----------------------------------------------------------------------------
typedef SvxForbiddenStruct_Impl* SvxForbiddenStruct_ImplPtr;
SV_DECL_PTRARR_DEL(SvxForbiddenStructArr, SvxForbiddenStruct_ImplPtr, 2, 2);
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
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
SvxAsianConfig::~SvxAsianConfig()
{
    delete pImpl;
}
/* -----------------------------17.01.01 09:57--------------------------------

 ---------------------------------------------------------------------------*/
void SvxAsianConfig::Load()
{
    Sequence<Any> aValues = GetProperties(lcl_GetPropertyNames());
    const Any* pValues = aValues.getConstArray();
    if(pValues[0].hasValue())
        pImpl->bKerningWesternTextOnly = *(sal_Bool*) pValues[0].getValue();
    pValues[1] >>= pImpl->nCharDistanceCompression;

    pImpl->aForbiddenArr.DeleteAndDestroy(0, pImpl->aForbiddenArr.Count());
    OUString sPropPrefix(C2U(sStartEndCharacters));
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
/* -----------------------------17.01.01 09:57--------------------------------

 ---------------------------------------------------------------------------*/
void    SvxAsianConfig::Notify( const Sequence<OUString>& rPropertyNames)
{
    Load();
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
void SvxAsianConfig::Commit()
{
    Sequence<Any> aValues(2);
    Any* pValues = aValues.getArray();
    pValues[0].setValue(&pImpl->bKerningWesternTextOnly, ::getBooleanCppuType());
    pValues[1] <<= pImpl->nCharDistanceCompression;
    PutProperties(lcl_GetPropertyNames(), aValues);


    OUString sNode(C2U(sStartEndCharacters));
    if(!pImpl->aForbiddenArr.Count())
        ClearNodeSet(sNode);
    else
    {
        Sequence<PropertyValue> aSetValues(2 * pImpl->aForbiddenArr.Count());
        PropertyValue* pSetValues = aSetValues.getArray();
        sal_Int32 nSetValue = 0;
        const OUString sStartChars(C2U(sStartCharacters));
        const OUString sEndChars(C2U(sEndCharacters));
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
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    SvxAsianConfig::IsKerningWesternTextOnly() const
{
    return pImpl->bKerningWesternTextOnly;
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
void        SvxAsianConfig::SetKerningWesternTextOnly(sal_Bool bSet)
{
    pImpl->bKerningWesternTextOnly = bSet;
    SetModified();
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16   SvxAsianConfig::GetCharDistanceCompression() const
{
    return pImpl->nCharDistanceCompression;
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
void        SvxAsianConfig::SetCharDistanceCompression(sal_Int16 nSet)
{
    DBG_ASSERT(nSet >= 0 && nSet < 3, "compression value illegal");
    SetModified();
    pImpl->nCharDistanceCompression = nSet;
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
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
