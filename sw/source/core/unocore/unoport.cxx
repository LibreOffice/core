/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoport.cxx,v $
 *
 *  $Revision: 1.41 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:32:26 $
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
#include "precompiled_sw.hxx"


#include <cmdid.h>
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _UNOCRSRHELPER_HXX
#include <unocrsrhelper.hxx>
#endif
#ifndef _UNOPORT_HXX
#include <unoport.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
//#ifndef _COM_SUN_STAR_BEANS_SETPROPERTYTOLERANTFAILED_HPP_
//#include <com/sun/star/beans/SetPropertyTolerantFailed.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_GETPROPERTYTOLERANTRESULT_HPP_
//#include <com/sun/star/beans/GetPropertyTolerantResult.hpp>
//#endif
//#ifndef _COM_SUN_STAR_BEANS_TOLERANTPROPERTYSETRESULTTYPE_HPP_
//#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
//#endif

using namespace ::com::sun::star;
using ::rtl::OUString;


/******************************************************************
 * SwXTextPortion
 ******************************************************************/

/*-- 11.12.98 09:56:52---------------------------------------------------

  -----------------------------------------------------------------------*/
SwFmtFld*   SwXTextPortion::GetFldFmt(sal_Bool bInit)
{
    SwFmtFld* pRet = 0;
    // initial wird es immer gesucht, danach nur noch, wenn es bereits existierte
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr && (bInit || pFmtFld))
    {
        SwTxtNode *pNode = pUnoCrsr->GetPoint()->nNode.GetNode().GetTxtNode();
        SwTxtFld *pTxtFld = 0;
        if( pNode )
            pTxtFld = pNode->GetTxtFld( pUnoCrsr->Start()->nContent );
        if(pTxtFld)
            pFmtFld = pRet = (SwFmtFld*)&pTxtFld->GetFld();
    }
    return pRet;
}
/*-- 11.12.98 09:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr,
        uno::Reference< text::XText > & rParent,
        SwTextPortionType eType) :
    aLstnrCntnr( (text::XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(
                (PORTION_REDLINE_START == eType ||
                 PORTION_REDLINE_END   == eType) ?
                    PROPERTY_MAP_REDLINE_PORTION : PROPERTY_MAP_TEXTPORTION_EXTENSIONS)),
    xParentText(rParent),
    pRubyText(0),
    pRubyStyle(0),
    pRubyAdjust(0),
    pRubyIsAbove(0),
    pFmtFld(0),
    aFrameDepend(this, 0),
    pFrameFmt(0),
    ePortionType(eType),
    nControlChar(0),
    bIsCollapsed(FALSE)
{
    SwUnoCrsr* pUnoCrsr = pPortionCrsr->GetDoc()->CreateUnoCrsr(*pPortionCrsr->GetPoint());
    if(pPortionCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pPortionCrsr->GetMark();
    }
    pUnoCrsr->Add(this);
    // erst nach ->Add()
    if(ePortionType == PORTION_FIELD)
        GetFldFmt(sal_True);
//  else if(ePortionType == PORTION_FRAME)
//      ...;
}
/* -----------------24.03.99 16:30-------------------
 *
 * --------------------------------------------------*/
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr, uno::Reference< text::XText > & rParent,
                        SwFrmFmt& rFmt ) :
    aLstnrCntnr( (text::XTextRange*)this),
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXTPORTION_EXTENSIONS)),
    xParentText(rParent),
    pRubyText(0),
    pRubyStyle(0),
    pRubyAdjust(0),
    pRubyIsAbove(0),
    pFmtFld(0),
    aFrameDepend(this, &rFmt),
    pFrameFmt(&rFmt),
    ePortionType(PORTION_FRAME),
    nControlChar(0),
    bIsCollapsed(FALSE)
{
    SwUnoCrsr* pUnoCrsr = pPortionCrsr->GetDoc()->CreateUnoCrsr(*pPortionCrsr->GetPoint());
    if(pPortionCrsr->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pPortionCrsr->GetMark();
    }
    pUnoCrsr->Add(this);
}

/*-- 11.12.98 09:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextPortion::~SwXTextPortion()
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
    delete pRubyText;
    delete pRubyStyle;
    delete pRubyAdjust;
    delete pRubyIsAbove;
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText >  SwXTextPortion::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< text::XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< text::XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextPortion::getString(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String aTxt;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        //TextPortions liegen immer innerhalb eines Absatzes
        SwTxtNode* pTxtNd = pUnoCrsr->GetNode()->GetTxtNode();
        if( pTxtNd )
        {
            xub_StrLen nStt = pUnoCrsr->Start()->nContent.GetIndex();
            aTxt = pTxtNd->GetExpandTxt( nStt,
                    pUnoCrsr->End()->nContent.GetIndex() - nStt );
        }
    }
    else
        throw uno::RuntimeException();
    return aTxt;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setString(const OUString& aString) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
        SwXTextCursor::SetString(*pUnoCrsr, aString);
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    //! PropertySetInfo for text portion extensions
    static uno::Reference< beans::XPropertySetInfo >
            xTxtPorExtRef = SfxItemPropertySet( aSwMapProvider.GetPropertyMap(
                    PROPERTY_MAP_TEXTPORTION_EXTENSIONS) ).getPropertySetInfo();
    //! PropertySetInfo for redline portions
    static uno::Reference< beans::XPropertySetInfo >
            xRedlPorRef = SfxItemPropertySet( aSwMapProvider.GetPropertyMap(
                    PROPERTY_MAP_REDLINE_PORTION) ).getPropertySetInfo();

    return (PORTION_REDLINE_START == ePortionType ||
            PORTION_REDLINE_END   == ePortionType) ? xRedlPorRef : xTxtPorExtRef;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName, aValue);
    }
    else
        throw uno::RuntimeException();
}
/*-- 04.11.03 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::GetPropertyValue(
        uno::Any &rVal,
        const SfxItemPropertyMap *pEntry,
        SwUnoCrsr *pUnoCrsr,
        SfxItemSet *&pSet )
{
    DBG_ASSERT( pUnoCrsr, "UNO cursor missing" );
    if (!pUnoCrsr)
        return;
    if (pEntry && pUnoCrsr)
    {
        switch(pEntry->nWID)
        {
            case FN_UNO_TEXT_PORTION_TYPE:
            {
                const char* pRet;
                switch (ePortionType)
                {
                case PORTION_TEXT:          pRet = "Text";break;
                case PORTION_FIELD:         pRet = "TextField";break;
                case PORTION_FRAME:         pRet = "Frame";break;
                case PORTION_FOOTNOTE:      pRet = "Footnote";break;
                case PORTION_CONTROL_CHAR:  pRet = "ControlCharacter";break;
                case PORTION_REFMARK_START:
                case PORTION_REFMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_REFERENCE_MARK);break;
                case PORTION_TOXMARK_START:
                case PORTION_TOXMARK_END:   pRet = SW_PROP_NAME_STR(UNO_NAME_DOCUMENT_INDEX_MARK);break;
                case PORTION_BOOKMARK_START :
                case PORTION_BOOKMARK_END : pRet = SW_PROP_NAME_STR(UNO_NAME_BOOKMARK);break;
                case PORTION_REDLINE_START:
                case PORTION_REDLINE_END:   pRet = "Redline";break;
                case PORTION_RUBY_START:
                case PORTION_RUBY_END:      pRet = "Ruby";break;
                case PORTION_SOFT_PAGEBREAK:pRet = "SoftPageBreak";break;
                default:
                    pRet = 0;
                }

                OUString sRet;
                if( pRet )
                    sRet = C2U( pRet );
                rVal <<= sRet;
            }
            break;
            case FN_UNO_CONTROL_CHARACTER:
            {
                if(PORTION_CONTROL_CHAR == ePortionType)
                    rVal <<= (sal_Int16) nControlChar;
            }
            break;
            case FN_UNO_DOCUMENT_INDEX_MARK:
                rVal <<= xTOXMark;
            break;
            case FN_UNO_REFERENCE_MARK:
                rVal <<= xRefMark;
            break;
            case FN_UNO_BOOKMARK:
                rVal <<= xBookmark;
            break;
            case FN_UNO_FOOTNOTE:
                rVal <<= xFootnote;
            break;
            case FN_UNO_IS_COLLAPSED:
            {
                BOOL bPut = TRUE;
                switch (ePortionType)
                {
                    case PORTION_REFMARK_START:
                    case PORTION_BOOKMARK_START :
                    case PORTION_TOXMARK_START:
                    case PORTION_REFMARK_END:
                    case PORTION_TOXMARK_END:
                    case PORTION_BOOKMARK_END :
                    case PORTION_REDLINE_START :
                    case PORTION_REDLINE_END :
                    case PORTION_RUBY_START:
                    case PORTION_RUBY_END:
                        rVal.setValue(&bIsCollapsed, ::getBooleanCppuType());
                    break;
                    default:
                        bPut = FALSE;
                }
            }
            break;
            case FN_UNO_IS_START:
            {
                BOOL bStart = TRUE, bPut = TRUE;
                switch (ePortionType)
                {
                    case PORTION_REFMARK_START:
                    case PORTION_BOOKMARK_START:
                    case PORTION_TOXMARK_START:
                    case PORTION_REDLINE_START:
                    case PORTION_RUBY_START:
                    break;

                    case PORTION_REFMARK_END:
                    case PORTION_TOXMARK_END:
                    case PORTION_BOOKMARK_END:
                    case PORTION_REDLINE_END:
                    case PORTION_RUBY_END:
                        bStart = FALSE;
                    break;
                    default:
                        bPut = FALSE;
                }
                if(bPut)
                    rVal.setValue(&bStart, ::getBooleanCppuType());
            }
            break;
            case RES_TXTATR_CJK_RUBY:
            {
                uno::Any* pToSet = 0;
                switch(pEntry->nMemberId)
                {
                    case MID_RUBY_TEXT :    pToSet = pRubyText;     break;
                    case MID_RUBY_ADJUST :  pToSet = pRubyAdjust;   break;
                    case MID_RUBY_CHARSTYLE:pToSet = pRubyStyle;    break;
                    case MID_RUBY_ABOVE :   pToSet = pRubyIsAbove;  break;
                }
                if(pToSet)
                    rVal = *pToSet;
            }
            break;
            default:
                beans::PropertyState eTemp;
                BOOL bDone = SwUnoCursorHelper::getCrsrPropertyValue(
                                    pEntry, *pUnoCrsr, &(rVal), eTemp );
                if(!bDone)
                {
                    if(!pSet)
                    {
                        pSet = new SfxItemSet(pUnoCrsr->GetDoc()->GetAttrPool(),
                            RES_CHRATR_BEGIN, RES_FRMATR_END - 1,
                            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                            RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                            0L);
                        SwXTextCursor::GetCrsrAttr(*pUnoCrsr, *pSet);
                    }
                    rVal = aPropSet.getPropertyValue(*pEntry, *pSet);
                }
        }
    }
}


uno::Sequence< uno::Any > SAL_CALL SwXTextPortion::GetPropertyValues_Impl(
        const uno::Sequence< OUString >& rPropertyNames )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    sal_Int32 nLength = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    uno::Sequence< uno::Any > aValues(rPropertyNames.getLength());
    uno::Any *pValues = aValues.getArray();
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SfxItemSet *pSet = 0;
        // get startting pount fo the look-up, either the provided one or else
        // from the beginning of the map
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        for(sal_Int32 nProp = 0; nProp < nLength; nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if(pMap)
            {
                GetPropertyValue( pValues[nProp], pMap, pUnoCrsr, pSet );
                pMap++;
            }
            else
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
        }
        delete pSet;
    }
    else
        throw uno::RuntimeException();
    return aValues;
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortion::getPropertyValue(
    const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< ::rtl::OUString > aPropertyNames(1);
    aPropertyNames.getArray()[0] = rPropertyName;
    return GetPropertyValues_Impl(aPropertyNames).getConstArray()[0];
}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextPortion::SetPropertyValues_Impl(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        const OUString* pPropertyNames = rPropertyNames.getConstArray();
        const uno::Any* pValues = rValues.getConstArray();
        const SfxItemPropertyMap*   pMap = aPropSet.getPropertyMap();
        OUString sTmp;
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength(); nProp++)
        {
            pMap = SfxItemPropertyMap::GetByName(pMap, pPropertyNames[nProp]);
            if (!pMap)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );
            if ( pMap->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + pPropertyNames[nProp], static_cast < cppu::OWeakObject * > ( this ) );

            SwXTextCursor::SetPropertyValue( *pUnoCrsr, aPropSet, sTmp, pValues[nProp], pMap);
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXTextPortion::setPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames,
    const uno::Sequence< uno::Any >& rValues )
        throw(beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // workaround for bad designed API
    try
    {
        SetPropertyValues_Impl( rPropertyNames, rValues );
    }
    catch (beans::UnknownPropertyException &rException)
    {
        // wrap the original (here not allowed) exception in
        // a lang::WrappedTargetException that gets thrown instead.
        lang::WrappedTargetException aWExc;
        aWExc.TargetException <<= rException;
        throw aWExc;
    }
}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< uno::Any > SwXTextPortion::getPropertyValues(
    const uno::Sequence< OUString >& rPropertyNames )
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< uno::Any > aValues;

    // workaround for bad designed API
    try
    {
        aValues = GetPropertyValues_Impl( rPropertyNames );
    }
    catch (beans::UnknownPropertyException &)
    {
        throw uno::RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property exception caught" ) ), static_cast < cppu::OWeakObject * > ( this ) );
    }
    catch (lang::WrappedTargetException &)
    {
        throw uno::RuntimeException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "WrappedTargetException caught" ) ), static_cast < cppu::OWeakObject * > ( this ) );
    }

    return aValues;
}
/* -----------------------------29.09.03 11:44--------------------------------

 ---------------------------------------------------------------------------*/

/* disabled for #i46921#

uno::Sequence< SetPropertyTolerantFailed > SAL_CALL SwXTextPortion::setPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames,
        const uno::Sequence< uno::Any >& rValues )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if (rPropertyNames.getLength() != rValues.getLength())
        throw lang::IllegalArgumentException();
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if (!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    sal_Int32 nVals = rValues.getLength();
    const uno::Any *pValue = rValues.getConstArray();

    sal_Int32 nFailed = 0;
    uno::Sequence< SetPropertyTolerantFailed > aFailed( nProps );
    SetPropertyTolerantFailed *pFailed = aFailed.getArray();

    // get entry to start with
    const SfxItemPropertyMap*   pStartEntry = aPropSet.getPropertyMap();

    OUString sTmp;
    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        try
        {
            pFailed[ nFailed ].Name    = pProp[i];

            const SfxItemPropertyMap *pEntry =
                    SfxItemPropertyMap::GetByName( pStartEntry, pProp[i] );
            if (!pEntry)
                pFailed[ nFailed++ ].Result  = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            else
            {
                // set property value
                // (compare to SwXTextPortion::setPropertyValues)
                if (pEntry->nFlags & beans::PropertyAttribute::READONLY)
                    pFailed[ nFailed++ ].Result  = TolerantPropertySetResultType::PROPERTY_VETO;
                else
                {
                    SwXTextCursor::SetPropertyValue(
                                *pUnoCrsr, aPropSet, sTmp, pValue[i], pEntry );
                }

                // continue with search for next property after current entry
                // (property map and sequence of property names are sorted!)
                pStartEntry = ++pEntry;
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            pFailed[ nFailed++ ].Result = TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }

    aFailed.realloc( nFailed );
    return aFailed;
}


uno::Sequence< GetPropertyTolerantResult > SAL_CALL SwXTextPortion::getPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    uno::Sequence< GetDirectPropertyTolerantResult > aTmpRes(
            GetPropertyValuesTolerant_Impl( rPropertyNames, sal_False ) );
    const GetDirectPropertyTolerantResult *pTmpRes = aTmpRes.getConstArray();

    // copy temporary result to final result type
    sal_Int32 nLen = aTmpRes.getLength();
    uno::Sequence< GetPropertyTolerantResult > aRes( nLen );
    GetPropertyTolerantResult *pRes = aRes.getArray();
    for (sal_Int32 i = 0;  i < nLen;  i++)
        *pRes++ = *pTmpRes++;
    return aRes;
}


uno::Sequence< GetDirectPropertyTolerantResult > SAL_CALL SwXTextPortion::getDirectPropertyValuesTolerant(
        const uno::Sequence< OUString >& rPropertyNames )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    return GetPropertyValuesTolerant_Impl( rPropertyNames, sal_True );
}


uno::Sequence< GetDirectPropertyTolerantResult > SAL_CALL SwXTextPortion::GetPropertyValuesTolerant_Impl(
        const uno::Sequence< OUString >& rPropertyNames,
        sal_Bool bDirectValuesOnly )
    throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Int32 nProps = rPropertyNames.getLength();
    const OUString *pProp = rPropertyNames.getConstArray();

    uno::Sequence< GetDirectPropertyTolerantResult > aResult( nProps );
    GetDirectPropertyTolerantResult *pResult = aResult.getArray();
    sal_Int32 nIdx = 0;

    SfxItemSet *pSet = 0;

    // get entry to start with
    const SfxItemPropertyMap*   pStartEntry = aPropSet.getPropertyMap();

    for (sal_Int32 i = 0;  i < nProps;  ++i)
    {
        DBG_ASSERT( nIdx < nProps, "index out ouf bounds" );
        beans::GetDirectPropertyTolerantResult &rResult = pResult[nIdx];

        try
        {
            rResult.Name = pProp[i];

            const SfxItemPropertyMap *pEntry =
                    SfxItemPropertyMap::GetByName( pStartEntry, pProp[i] );
            if (!pEntry)     // property available?
                rResult.Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
            else
            {
                // get property state
                // (compare to SwXTextPortion::getPropertyState)
                beans::PropertyState eState;
                if (GetTextPortionType() == PORTION_RUBY_START &&
                    !pProp[i].compareToAscii( RTL_CONSTASCII_STRINGPARAM( "Ruby" ) ))
                    eState = beans::PropertyState_DIRECT_VALUE;
                else
                    eState = SwXTextCursor::GetPropertyState( *pUnoCrsr, aPropSet, pProp[i] );
                rResult.State  = eState;

//                if (bDirectValuesOnly  &&  beans::PropertyState_DIRECT_VALUE != eState)
//                    rResult.Result = TolerantPropertySetResultType::NO_DIRECT_VALUE;
//                else
                rResult.Result = TolerantPropertySetResultType::UNKNOWN_FAILURE;
                if (!bDirectValuesOnly  ||  beans::PropertyState_DIRECT_VALUE == eState)
                {
                    // get property value
                    // (compare to SwXTextPortion::getPropertyValue(s))
                    GetPropertyValue( rResult.Value, pEntry, pUnoCrsr, pSet );
                    rResult.Result = TolerantPropertySetResultType::SUCCESS;

                    nIdx++;
                }
                // this assertion should never occur!
                DBG_ASSERT( nIdx < 1  ||  pResult[nIdx - 1].Result != TolerantPropertySetResultType::UNKNOWN_FAILURE,
                        "unknown failure while retrieving property" );

                // continue with search for next property after current entry
                // (property map and sequence of property names are sorted!)
                pStartEntry = ++pEntry;
            }
        }
        catch (beans::UnknownPropertyException &)
        {
            // should not occur because property was searched for before
            DBG_ERROR( "unexpected exception catched" );
            rResult.Result = TolerantPropertySetResultType::UNKNOWN_PROPERTY;
        }
        catch (lang::IllegalArgumentException &)
        {
            rResult.Result = TolerantPropertySetResultType::ILLEGAL_ARGUMENT;
        }
        catch (beans::PropertyVetoException &)
        {
            rResult.Result = TolerantPropertySetResultType::PROPERTY_VETO;
        }
        catch (lang::WrappedTargetException &)
        {
            rResult.Result = TolerantPropertySetResultType::WRAPPED_TARGET;
        }
    }
    delete pSet;

    // resize to actually used size
    aResult.realloc( nIdx );

    return aResult;
}


*/


/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/* -----------------------------02.04.01 11:44--------------------------------

 ---------------------------------------------------------------------------*/
void SwXTextPortion::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /*aPropertyNames*/,
    const uno::Reference< beans::XPropertiesChangeListener >& /*xListener*/ )
        throw(uno::RuntimeException)
{}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addPropertyChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 08.03.99 09:41:43---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXTextPortion::getPropertyState(const OUString& rPropertyName)
            throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    beans::PropertyState eRet = beans::PropertyState_DEFAULT_VALUE;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(GetTextPortionType() == PORTION_RUBY_START &&
           !rPropertyName.compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
            eRet = beans::PropertyState_DIRECT_VALUE;
        else
            eRet = SwXTextCursor::GetPropertyState(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return eRet;
}
/*-- 08.03.99 09:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& rPropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    uno::Sequence< beans::PropertyState > aRet = SwXTextCursor::GetPropertyStates(*pUnoCrsr, aPropSet, rPropertyNames, SW_PROPERTY_STATE_CALLER_SWX_TEXT_PORTION);

    if(GetTextPortionType() == PORTION_RUBY_START)
    {
        const OUString* pNames = rPropertyNames.getConstArray();
        beans::PropertyState* pStates = aRet.getArray();
        for(sal_Int32 nProp = 0; nProp < rPropertyNames.getLength();nProp++)
        {
           if(!pNames[nProp].compareToAscii( RTL_CONSTASCII_STRINGPARAM("Ruby") ))
                pStates[nProp] = beans::PropertyState_DIRECT_VALUE;
        }
    }
    return aRet;
}
/*-- 08.03.99 09:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::setPropertyToDefault(const OUString& rPropertyName)
                throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwXTextCursor::SetPropertyToDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
}
/*-- 08.03.99 09:41:48---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortion::getPropertyDefault(const OUString& rPropertyName)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        aRet = SwXTextCursor::GetPropertyDefault(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXTextPortion::getPresentation(sal_Bool /*bShowCommand*/) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    String sRet;
    SwFmtFld* pFmt = 0;
    if(pUnoCrsr && 0 != (pFmt = GetFldFmt()))
    {
        const SwField* pField = pFmt->GetFld();
        DBG_ERROR("bShowCommand auswerten!")
        sRet = pField->Expand();
    }
    return sRet;
}
/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::attach(const uno::Reference< text::XTextRange > & /*xTextRange*/)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // SwXTextPortion cannot be created at the factory therefore
    // they cannot be attached
    throw uno::RuntimeException();
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextPortion::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  aRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
        aRet = new SwXTextRange(*pUnoCrsr, xParentText);
    else
        throw uno::RuntimeException();
    return aRet;
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        setString(aEmptyStr);
        pUnoCrsr->Remove(this);
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 09:57:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& /*aServiceName*/)
        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    uno::Reference< container::XEnumeration >  xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_CHAR, pFrameFmt);
    return xRet;

}
/* -----------------------------04.07.01 08:52--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXTextPortion::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------04.07.01 08:52--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SwXTextPortion::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/* -----------------25.03.99 10:30-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextPortion::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextPortion");
}
/* -----------------25.03.99 10:30-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextPortion::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Bool bRet = sal_False;
    if(!rServiceName.compareToAscii("com.sun.star.text.TextPortion") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.CharacterPropertiesComplex") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphProperties") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesAsian") ||
            !rServiceName.compareToAscii("com.sun.star.style.ParagraphPropertiesComplex"))
        bRet = sal_True;
    else if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextField"))
        bRet = 0 != GetFldFmt();

    return bRet;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames(void)
                                                throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    sal_Bool bField = 0 != GetFldFmt();
    sal_uInt16 nCount = bField ? 8 : 7;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortion");
    pArray[1] = C2U("com.sun.star.style.CharacterProperties");
    pArray[2] = C2U("com.sun.star.style.CharacterPropertiesAsian");
    pArray[3] = C2U("com.sun.star.style.CharacterPropertiesComplex");
    pArray[4] = C2U("com.sun.star.style.ParagraphProperties");
    pArray[5] = C2U("com.sun.star.style.ParagraphPropertiesAsian");
    pArray[6] = C2U("com.sun.star.style.ParagraphPropertiesComplex");
    if(bField)
        pArray[7] = C2U("com.sun.star.text.TextField");
    return aRet;
}
/*-- 11.12.98 09:57:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!aFrameDepend.GetRegisteredIn())
        pFrameFmt = 0;
}
/* -----------------------------19.02.01 10:52--------------------------------

 ---------------------------------------------------------------------------*/
SwXRubyPortion::SwXRubyPortion(const SwUnoCrsr* pPortionCrsr,
                    SwTxtRuby& rAttr,
                    uno::Reference< text::XText > & rParent,
                    sal_Bool bEnd   ) :
        SwXTextPortion(pPortionCrsr, rParent, bEnd ? PORTION_RUBY_END : PORTION_RUBY_START  )
{
    if(!bEnd)
    {
        const SfxPoolItem& rItem = rAttr.GetAttr();
        pRubyText = new uno::Any;
        pRubyStyle = new uno::Any;
        pRubyAdjust = new uno::Any;
        pRubyIsAbove = new uno::Any;
        rItem.QueryValue(*pRubyText, MID_RUBY_TEXT);
        rItem.QueryValue(*pRubyStyle, MID_RUBY_CHARSTYLE);
        rItem.QueryValue(*pRubyAdjust, MID_RUBY_ADJUST);
        rItem.QueryValue(*pRubyIsAbove, MID_RUBY_ABOVE);
    }
}
/* -----------------------------19.02.01 10:52--------------------------------

 ---------------------------------------------------------------------------*/
SwXRubyPortion::~SwXRubyPortion()
{
}

