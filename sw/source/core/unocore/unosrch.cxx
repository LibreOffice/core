/*************************************************************************
 *
 *  $RCSfile: unosrch.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:29 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _UNOSRCH_HXX
#include <unosrch.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
/******************************************************************************
 *
 ******************************************************************************/

/* -----------------23.06.99 12:19-------------------

 --------------------------------------------------*/
class SwSearchProperties_Impl
{
    beans::PropertyValue**          pValueArr; //
    sal_uInt16                      nArrLen;
public:
    SwSearchProperties_Impl();
    ~SwSearchProperties_Impl();

    void    SetProperties(const Sequence< beans::PropertyValue >& aSearchAttribs)
        throw( beans::UnknownPropertyException, lang::IllegalArgumentException, RuntimeException );
    const Sequence< beans::PropertyValue > GetProperties() const;

    void    FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const;
    sal_Bool    HasAttributes() const;
};
/* -----------------23.06.99 13:08-------------------

 --------------------------------------------------*/
SwSearchProperties_Impl::SwSearchProperties_Impl() :
    nArrLen(0)
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    while(pMap->nWID)
    {
        if(pMap->nWID < RES_FRMATR_END)
            nArrLen++;
        pMap++;
    }
    pValueArr = new beans::PropertyValue*[nArrLen];
    *pValueArr = new beans::PropertyValue[nArrLen];
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        pValueArr[i] = 0;
}
/* -----------------23.06.99 13:08-------------------

 --------------------------------------------------*/
SwSearchProperties_Impl::~SwSearchProperties_Impl()
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        delete pValueArr[i];
    delete pValueArr;
}
/* -----------------23.06.99 13:09-------------------

 --------------------------------------------------*/
void    SwSearchProperties_Impl::SetProperties(const Sequence< beans::PropertyValue >& aSearchAttribs)
                throw( beans::UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    long nLen = aSearchAttribs.getLength();
    const beans::PropertyValue* pProps = aSearchAttribs.getConstArray();
    //delete all existing values
    for(long i = 0; i < nArrLen; i++)
    {
        delete pValueArr[i];
        pValueArr[i] = 0;
    }

    for(i = 0; i < nLen; i++)
    {
        String sPropertyName(pProps[i].Name);
        sal_uInt16 nIndex = 0;
        const SfxItemPropertyMap* pTempMap = pMap;
        while(pTempMap->nWID && !sPropertyName.EqualsAscii(pTempMap->pName))
        {
            pTempMap++;
            nIndex++;
        }
        if(!pTempMap->nWID)
            throw beans::UnknownPropertyException();
        pValueArr[nIndex] = new beans::PropertyValue(pProps[i]);
    }
}
/* -----------------23.06.99 13:08-------------------

 --------------------------------------------------*/
const Sequence< beans::PropertyValue > SwSearchProperties_Impl::GetProperties() const
{
    sal_uInt16 nPropCount = 0;
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            nPropCount++;

    Sequence< beans::PropertyValue > aRet(nPropCount);
    beans::PropertyValue* pProps = aRet.getArray();
    nPropCount = 0;
    for(i = 0; i < nArrLen; i++)
    {
        if(pValueArr[i])
        {
            pProps[nPropCount] = *(pValueArr[i]);
        }
        nPropCount++;
    }
    return aRet;
}
/* -----------------23.06.99 13:06-------------------

 --------------------------------------------------*/
void SwSearchProperties_Impl::FillItemSet(SfxItemSet& rSet, sal_Bool bIsValueSearch) const
{
    const SfxItemPropertyMap* pMap = aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR);
    //

    SfxPoolItem* pBoxItem = 0,
    *pBreakItem = 0,
    *pAutoKernItem  = 0,
    *pWLineItem   = 0,
    *pTabItem  = 0,
    *pSplitItem  = 0,
    *pRegItem  = 0,
    *pLineSpaceItem  = 0,
    *pLineNumItem  = 0,
    *pKeepItem  = 0,
    *pLRItem  = 0,
    *pULItem  = 0,
    *pBackItem  = 0,
    *pAdjItem  = 0,
    *pDescItem  = 0,
    *pInetItem  = 0,
    *pDropItem  = 0,
    *pWeightItem  = 0,
    *pULineItem  = 0,
    *pCharFmtItem  = 0,
    *pShadItem  = 0,
    *pPostItem  = 0,
    *pNHyphItem  = 0,
    *pLangItem  = 0,
    *pKernItem  = 0,
    *pFontSizeItem  = 0,
    *pFontItem  = 0,
    *pBlinkItem  = 0,
    *pEscItem  = 0,
    *pCrossedOutItem  = 0,
    *pContourItem  = 0,
    *pCharColorItem  = 0,
    *pCasemapItem  = 0,
    *pBrushItem  = 0;

    for(sal_uInt16 i = 0; i < nArrLen; i++)
    {
        if(pValueArr[i])
        {
            const SfxItemPropertyMap*   pTempMap =  pMap + i;
            SfxPoolItem* pTempItem = 0;
            switch(pTempMap->nWID)
            {
                case  RES_BOX:
                    if(!pBoxItem)
                        pBoxItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBoxItem;
                break;
                case  RES_BREAK:
                    if(!pBreakItem)
                        pBreakItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBreakItem;
                break;
                case  RES_CHRATR_AUTOKERN:
                    if(!pAutoKernItem)
                        pAutoKernItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pAutoKernItem;
                    break;
                case  RES_CHRATR_BACKGROUND:
                    if(!pBrushItem)
                        pBrushItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBrushItem;
                break;
                case  RES_CHRATR_CASEMAP:
                    if(!pCasemapItem)
                        pCasemapItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCasemapItem;
                break;
                case  RES_CHRATR_COLOR:
                    if(!pCharColorItem)
                        pCharColorItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCharColorItem;
                break;
                case  RES_CHRATR_CONTOUR:
                    if(!pContourItem)
                        pContourItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pContourItem;
                break;
                case  RES_CHRATR_CROSSEDOUT:
                    if(!pCrossedOutItem)
                        pCrossedOutItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCrossedOutItem;
                break;
                case  RES_CHRATR_ESCAPEMENT:
                    if(!pEscItem)
                        pEscItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pEscItem;
                break;
                case  RES_CHRATR_BLINK:
                    if(!pBlinkItem)
                        pBlinkItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBlinkItem;
                break;
                case  RES_CHRATR_FONT:
                    if(!pFontItem)
                        pFontItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pFontItem;
                break;
                case  RES_CHRATR_FONTSIZE:
                    if(!pFontSizeItem)
                        pFontSizeItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pFontSizeItem;
                break;
                case  RES_CHRATR_KERNING:
                    if(!pKernItem)
                        pKernItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pKernItem;
                break;
                case  RES_CHRATR_LANGUAGE:
                    if(!pLangItem)
                        pLangItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLangItem;
                break;
                case  RES_CHRATR_NOHYPHEN:
                    if(!pNHyphItem)
                        pNHyphItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pNHyphItem;
                break;
                case  RES_CHRATR_POSTURE:
                    if(!pPostItem)
                        pPostItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pPostItem;
                break;
                case  RES_CHRATR_SHADOWED:
                    if(!pShadItem)
                        pShadItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pShadItem;
                break;
                case  RES_TXTATR_CHARFMT:
                    if(!pCharFmtItem)
                        pCharFmtItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pCharFmtItem;
                break;
                case  RES_CHRATR_UNDERLINE:
                    if(!pULineItem)
                        pULineItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pULineItem;
                break;
                case  RES_CHRATR_WEIGHT:
                    if(!pWeightItem)
                        pWeightItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pWeightItem;
                break;
                case  RES_PARATR_DROP:
                    if(!pDropItem)
                        pDropItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pDropItem;
                break;
                case  RES_TXTATR_INETFMT:
                    if(!pInetItem)
                        pInetItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pInetItem;
                break;
                case  RES_PAGEDESC:
                    if(!pDescItem)
                        pDescItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pDescItem;
                break;
                case  RES_PARATR_ADJUST:
                    if(!pAdjItem)
                        pAdjItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pAdjItem;
                break;
                case  RES_BACKGROUND:
                    if(!pBackItem)
                        pBackItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pBackItem;
                break;
                case  RES_UL_SPACE:
                    if(!pULItem)
                        pULItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pULItem;
                break;
                case  RES_LR_SPACE:
                    if(!pLRItem)
                        pLRItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLRItem;
                break;
                case  RES_KEEP:
                    if(!pKeepItem)
                        pKeepItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pKeepItem;
                break;
                case  RES_LINENUMBER:
                    if(!pLineNumItem)
                        pLineNumItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLineNumItem;
                break;
                case  RES_PARATR_LINESPACING:
                    if(!pLineSpaceItem)
                        pLineSpaceItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pLineSpaceItem;
                break;
                case  RES_PARATR_REGISTER:
                    if(!pRegItem)
                        pRegItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pRegItem;
                break;
                case  RES_PARATR_SPLIT:
                    if(!pSplitItem)
                        pSplitItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pSplitItem;
                break;
                case  RES_PARATR_TABSTOP:
                    if(!pTabItem)
                        pTabItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pTabItem;
                break;
                case  RES_CHRATR_WORDLINEMODE:
                    if(!pWLineItem)
                        pWLineItem = rSet.GetPool()->GetDefaultItem(pTempMap->nWID).Clone();
                    pTempItem = pWLineItem;
                break;
            }
            if(pTempItem)
            {
                if(bIsValueSearch)
                {
                    pTempItem->PutValue(pValueArr[i]->Value, pTempMap->nMemberId);
                    rSet.Put(*pTempItem);
                }
                else
                    rSet.InvalidateItem( pTempItem->Which() );
            }
        }
    }
    delete pBoxItem;
    delete pBreakItem;
    delete pBreakItem ;
    delete pAutoKernItem ;
    delete pWLineItem;
    delete pTabItem;
    delete pSplitItem;
    delete pRegItem;
    delete pLineSpaceItem ;
    delete pLineNumItem  ;
    delete pKeepItem;
    delete pLRItem  ;
    delete pULItem  ;
    delete pBackItem;
    delete pAdjItem;
    delete pDescItem;
    delete pInetItem;
    delete pDropItem;
    delete pWeightItem;
    delete pULineItem;
    delete pCharFmtItem  ;
    delete pShadItem;
    delete pPostItem;
    delete pNHyphItem;
    delete pLangItem;
    delete pKernItem;
    delete pFontSizeItem ;
    delete pFontItem;
    delete pBlinkItem;
    delete pEscItem;
    delete pCrossedOutItem;
    delete pContourItem  ;
    delete pCharColorItem;
    delete pCasemapItem  ;
    delete pBrushItem  ;
}
/* -----------------23.06.99 14:18-------------------

 --------------------------------------------------*/
sal_Bool    SwSearchProperties_Impl::HasAttributes() const
{
    for(sal_uInt16 i = 0; i < nArrLen; i++)
        if(pValueArr[i])
            return sal_True;
    return sal_False;
}

/*-- 14.12.98 13:07:10    ---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSearch::SwXTextSearch() :
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_SEARCH)),
    pSearchProperties( new SwSearchProperties_Impl),
    pReplaceProperties( new SwSearchProperties_Impl),
    bIsValueSearch(sal_True),
    bAll(sal_False),
    bWord(sal_False),
    bBack(sal_False),
    bExpr(sal_False),
    bCase(sal_False),
    bStyles(sal_False),
    bSimilarity(sal_False),
    bLevRelax(sal_False),
    nLevExchange(2),
    nLevAdd(2),
    nLevRemove(2)
{
}
/*-- 14.12.98 13:07:12    ---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextSearch::~SwXTextSearch()
{
    delete pSearchProperties;
    delete pReplaceProperties;
}
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence< sal_Int8 > & SwXTextSearch::getUnoTunnelId()
{
    static Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextSearch::getSomething( const Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/*-- 14.12.98 13:07:12---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextSearch::getSearchString(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return sSearchText;
}
/*-- 14.12.98 13:07:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setSearchString(const OUString& rString)
                                        throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sSearchText = String(rString);
}
/*-- 14.12.98 13:07:12---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextSearch::getReplaceString(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return sReplaceText;
}
/*-- 14.12.98 13:07:12---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setReplaceString(const OUString& rReplaceString) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sReplaceText = String(rReplaceString);
}
/*-- 14.12.98 13:07:13---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< beans::XPropertySetInfo >  SwXTextSearch::getPropertySetInfo(void) throw( RuntimeException )
{
    static Reference< beans::XPropertySetInfo >  aRef = new SfxItemPropertySetInfo(_pMap);
    return aRef;
}
/*-- 14.12.98 13:07:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setPropertyValue(const OUString& rPropertyName, const Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
        lang::IllegalArgumentException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    if(pMap)
    {
        sal_Bool bVal = FALSE;
        if(aValue.getValueType() == ::getBooleanCppuType())
            bVal = *(sal_Bool*)aValue.getValue();
        switch(pMap->nWID)
        {
            case WID_SEARCH_ALL :           bAll        = bVal; break;
            case WID_WORDS:                 bWord       = bVal; break;
            case WID_BACKWARDS :            bBack       = bVal; break;
            case WID_REGULAR_EXPRESSION :   bExpr       = bVal; break;
            case WID_CASE_SENSITIVE  :      bCase       = bVal; break;
            //case WID_IN_SELECTION  :      bInSel      = bVal; break;
            case WID_STYLES          :      bStyles     = bVal; break;
            case WID_SIMILARITY      :      bSimilarity = bVal; break;
            case WID_SIMILARITY_RELAX:      bLevRelax   = bVal; break;
            case WID_SIMILARITY_EXCHANGE:   aValue >>= nLevExchange; break;
            case WID_SIMILARITY_ADD:        aValue >>= nLevAdd; break;
            case WID_SIMILARITY_REMOVE :    aValue >>= nLevRemove;break;
            break;
        };
    }
}
/*-- 14.12.98 13:07:13---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXTextSearch::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Any aRet;

    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    sal_Bool bSet = sal_False;
    sal_Int16 nSet = 0;
    if(pMap)
        switch(pMap->nWID)
        {
            case WID_SEARCH_ALL :           bSet = bAll; goto SET_BOOL;
            case WID_WORDS:                 bSet = bWord; goto SET_BOOL;
            case WID_BACKWARDS :            bSet = bBack; goto SET_BOOL;
            case WID_REGULAR_EXPRESSION :   bSet = bExpr; goto SET_BOOL;
            case WID_CASE_SENSITIVE  :      bSet = bCase; goto SET_BOOL;
            //case WID_IN_SELECTION  :      bSet = bInSel; goto SET_BOOL;
            case WID_STYLES          :      bSet = bStyles; goto SET_BOOL;
            case WID_SIMILARITY      :      bSet = bSimilarity; goto SET_BOOL;
            case WID_SIMILARITY_RELAX:      bSet = bLevRelax;
SET_BOOL:
            aRet.setValue(&bSet, ::getBooleanCppuType());
            break;
            case WID_SIMILARITY_EXCHANGE:   nSet = nLevExchange; goto SET_UINT16;
            case WID_SIMILARITY_ADD:        nSet = nLevAdd; goto SET_UINT16;
            case WID_SIMILARITY_REMOVE :    nSet = nLevRemove;
SET_UINT16:
            aRet <<= nSet;
            break;
        };
    return aRet;
}
/*-- 14.12.98 13:07:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::addPropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 13:07:13---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::removePropertyChangeListener(const OUString& PropertyName, const Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 13:07:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::addVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 13:07:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::removeVetoableChangeListener(const OUString& PropertyName, const Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 14.12.98 13:07:14---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextSearch::getValueSearch(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return bIsValueSearch;
}
/*-- 14.12.98 13:07:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setValueSearch(sal_Bool ValueSearch_) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    bIsValueSearch = ValueSearch_;
}
/*-- 14.12.98 13:07:15---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< beans::PropertyValue > SwXTextSearch::getSearchAttributes(void) throw( RuntimeException )
{
    return  pSearchProperties->GetProperties();
}
/*-- 14.12.98 13:07:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setSearchAttributes(const Sequence< beans::PropertyValue >& rSearchAttribs)
    throw( beans::UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    pSearchProperties->SetProperties(rSearchAttribs);
}
/*-- 14.12.98 13:07:16---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< beans::PropertyValue > SwXTextSearch::getReplaceAttributes(void)
    throw( RuntimeException )
{
    return pReplaceProperties->GetProperties();
}
/*-- 14.12.98 13:07:17---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextSearch::setReplaceAttributes(const Sequence< beans::PropertyValue >& rReplaceAttribs)
    throw( beans::UnknownPropertyException, lang::IllegalArgumentException, RuntimeException )
{
    pReplaceProperties->SetProperties(rReplaceAttribs);
}
/* -----------------23.06.99 14:13-------------------

 --------------------------------------------------*/
void    SwXTextSearch::FillSearchItemSet(SfxItemSet& rSet) const
{
    pSearchProperties->FillItemSet(rSet, bIsValueSearch);
}
/* -----------------23.06.99 14:14-------------------

 --------------------------------------------------*/
void    SwXTextSearch::FillReplaceItemSet(SfxItemSet& rSet) const
{
    pReplaceProperties->FillItemSet(rSet, bIsValueSearch);
}
/* -----------------23.06.99 14:17-------------------

 --------------------------------------------------*/
sal_Bool    SwXTextSearch::HasSearchAttributes() const
{
    return pSearchProperties->HasAttributes();
}
/* -----------------23.06.99 14:17-------------------

 --------------------------------------------------*/
sal_Bool    SwXTextSearch::HasReplaceAttributes() const
{
    return pReplaceProperties->HasAttributes();
}
/* -----------------------------19.04.00 14:43--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXTextSearch::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextSearch");
}
/* -----------------------------19.04.00 14:43--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXTextSearch::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.util.SearchDescriptor") == rServiceName ||
            C2U("com.sun.star.util.ReplaceDescriptor") == rServiceName;
}
/* -----------------------------19.04.00 14:43--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXTextSearch::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.util.SearchDescriptor");
    pArray[1] = C2U("com.sun.star.util.ReplaceDescriptor");
    return aRet;
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.38  2000/09/18 16:04:35  willem.vandorp
    OpenOffice header added.

    Revision 1.37  2000/06/13 13:33:29  os
    #75482# ServiceInfo of SwXTextSearch completed

    Revision 1.36  2000/05/16 09:14:55  os
    project usr removed

    Revision 1.35  2000/04/19 13:35:31  os
    UNICODE

    Revision 1.34  2000/04/11 08:31:04  os
    UNICODE

    Revision 1.33  2000/03/27 10:21:10  os
    UNO III

    Revision 1.32  2000/03/21 15:42:25  os
    UNOIII

    Revision 1.31  2000/02/11 14:35:55  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.30  1999/11/19 16:40:19  os
    modules renamed

    Revision 1.29  1999/06/24 07:19:34  OS
    #67048# util::XPropertyReplace implemented


      Rev 1.28   24 Jun 1999 09:19:34   OS
   #67048# util::XPropertyReplace implemented

      Rev 1.27   22 Apr 1999 16:13:56   OS
   #65194# throw -> throw; #65124# not impl. nur noch warning; EventListener

      Rev 1.26   15 Mar 1999 14:37:50   OS
   #62845# Makro fuer ServiceInfo jetzt auch fuer OS/2

      Rev 1.25   12 Mar 1999 09:41:28   OS
   #62845# lang::XServiceInfo impl.

      Rev 1.24   09 Mar 1999 12:41:26   OS
   #62008# Solar-Mutex

      Rev 1.23   04 Mar 1999 11:43:44   OS
   #62459# Searchable funktioniert wieder

      Rev 1.22   18 Feb 1999 13:44:30   HR
   Typo in Makro

      Rev 1.21   28 Jan 1999 16:45:00   OS
   #56371# keine Objekte fuer DEBUG anlegen

      Rev 1.20   22 Jan 1999 15:09:20   OS
   #56371# Draw wieder verfuegbar

      Rev 1.19   15 Dec 1998 10:10:02   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.18   10 Dec 1998 15:53:36   OS
   #56371# TF_ONE51 Zwischenstand

      Rev 1.17   10 Jul 1998 18:09:02   OS
   PropertySetInfo und IdlClass static

      Rev 1.16   26 Jun 1998 18:17:12   OS
   includes

      Rev 1.15   25 Jun 1998 11:14:56   OS
   PreopertyMaps nur noch vom PropertyMapProvider

      Rev 1.14   23 Jun 1998 10:22:50   OS
   search/replace funktioniert wieder

      Rev 1.13   18 Jun 1998 13:22:44   OS
   include-Umstellung 396c

      Rev 1.12   17 Jun 1998 18:14:42   OS
   search()/replace() nicht mehr am util::XSearchable

      Rev 1.11   15 Jun 1998 14:04:58   OS
   SwXContainer vergessen

      Rev 1.10   15 Jun 1998 10:53:54   OS
   Chg Container 396

      Rev 1.9   05 Jun 1998 14:28:52   TJ
   wieder eine Klammer

      Rev 1.8   04 Jun 1998 09:40:04   OS
// automatisch auskommentiert - [getIdlClass(es) or queryInterface] - Bitte XTypeProvider benutzen!
//   getIdlClasses


      Rev 1.7   22 May 1998 15:10:50   OS
   include

      Rev 1.6   18 May 1998 12:20:36   OS
   Container fuer Suchergebnis

      Rev 1.5   14 May 1998 17:48:52   OS
   wiedererweckt

      Rev 1.4   04 Feb 1998 17:27:14   OS
   uebersetzbar

      Rev 1.3   30 Jan 1998 15:10:46   OS
   wieder uebersetzbar

      Rev 1.2   19 Jan 1998 14:58:04   OS
   wieder uebersetzbar

      Rev 1.1   08 Jan 1998 12:56:50   OS
   UsrUik -> Uik

      Rev 1.0   05 Jan 1998 11:03:20   OS
   Initial revision.

------------------------------------------------------------------------*/

