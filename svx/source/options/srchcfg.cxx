/*************************************************************************
 *
 *  $RCSfile: srchcfg.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2001-03-20 10:29:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _SVX_SRCHNCFG_HXX
#include <srchcfg.hxx>
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

#define C2U(cChar) OUString::createFromAscii(cChar)

//-----------------------------------------------------------------------------
typedef SvxSearchEngineData* SvxSearchEngineDataPtr;
SV_DECL_PTRARR_DEL(SvxSearchEngineArr, SvxSearchEngineDataPtr, 2, 2);
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
const Sequence<OUString>& lcl_GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        aNames.realloc(12);
        OUString* pNames = aNames.getArray();
        pNames[0] = C2U("And/Prefix");
        pNames[1] = C2U("And/Suffix");
        pNames[2] = C2U("And/Separator");
        pNames[3] = C2U("And/CaseMatch");
        pNames[4] = C2U("Or/Prefix");
        pNames[5] = C2U("Or/Suffix");
        pNames[6] = C2U("Or/Separator");
        pNames[7] = C2U("Or/CaseMatch");
        pNames[8] = C2U("Exact/Prefix");
        pNames[9] = C2U("Exact/Suffix");
        pNames[10] = C2U("Exact/Separator");
        pNames[11] = C2U("Exact/CaseMatch");
    }
    return aNames;
}
// ---------------------------------------------------------------------------
SvxSearchConfig::SvxSearchConfig(sal_Bool bEnableNotify) :
    utl::ConfigItem(C2U("Inet/Search/SearchEngines"), CONFIG_MODE_DELAYED_UPDATE),
    pImpl(new SvxSearchConfig_Impl)
{
    if(bEnableNotify)
        EnableNotification(lcl_GetPropertyNames());
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
        const Sequence<OUString>& rPropNames = lcl_GetPropertyNames();
        const OUString* pPropNames = rPropNames.getConstArray();
        Sequence<OUString> aPropertyNames(rPropNames.getLength());
        OUString* pPropertyNames = aPropertyNames.getArray();
        const OUString sSlash(C2U("/"));
        sal_Int32 nProp;
        for(nProp = 0; nProp < rPropNames.getLength(); nProp++)
        {
            pPropertyNames[nProp] = pNodeNames[nNode];
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
void    SvxSearchConfig::Notify( const Sequence<OUString>& rPropertyNames)
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
        sal_Int32 nSetValue = 0;

        const Sequence<OUString>& rPropNames = lcl_GetPropertyNames();
        const OUString* pPropNames = rPropNames.getConstArray();
        const OUString sSlash(C2U("/"));
        for(sal_uInt16 i = 0; i < pImpl->aEngineArr.Count(); i++)
        {
            SvxSearchEngineDataPtr pSave = pImpl->aEngineArr[i];
            for(sal_Int16 nProp = 0; nProp < rPropNames.getLength(); nProp++)
            {
                OUString sTmpName = sSlash;
                sTmpName += pSave->sEngineName;
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
    DBG_ASSERT(nPos < pImpl->aEngineArr.Count(), "wrong array index")
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
            return ;
        }
    }
}

