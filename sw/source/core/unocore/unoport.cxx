/*************************************************************************
 *
 *  $RCSfile: unoport.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:28 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
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
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

//TODO: new Interface & new uno::Exception for protected content
#define EXCEPT_ON_PROTECTION(rUnoCrsr)  \
    if((rUnoCrsr).HasReadonlySel()) \
        throw uno::RuntimeException();

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
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr, uno::Reference< XText >  xParent,
        SwTextPortionType eType) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    aLstnrCntnr( (XTextRange*)this),
    pFmtFld(0),
    xParentText(xParent),
    ePortionType(eType),
    pFrameFmt(0),
    aFrameDepend(this, 0),
    bIsCollapsed(FALSE),
    nControlChar(0)
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
SwXTextPortion::SwXTextPortion(const SwUnoCrsr* pPortionCrsr, uno::Reference< XText >  xParent,
                        SwFrmFmt& rFmt ) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_CURSOR)),
    aLstnrCntnr( (XTextRange*)this),
    pFrameFmt(&rFmt),
    xParentText(xParent),
    ePortionType(PORTION_FRAME),
    pFmtFld(0),
    aFrameDepend(this, &rFmt),
    bIsCollapsed(FALSE),
    nControlChar(0)
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
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XText >  SwXTextPortion::getText(void) throw( uno::RuntimeException )
{
    return xParentText;
}
/*-- 11.12.98 09:56:56---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextPortion::getStart(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->Start());
        uno::Reference< XText >  xParent = getText();
        xRet = new SwXTextRange(aPam, xParent);
    }
    else
        throw uno::RuntimeException();
    return xRet;
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextPortion::getEnd(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  xRet;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        SwPaM aPam(*pUnoCrsr->End());
        uno::Reference< XText >  xParent = getText();
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
    SwUnoCrsr* pUnoCrsr = ((SwXTextCursor*)this)->GetCrsr();
    EXCEPT_ON_PROTECTION(*pUnoCrsr)
    if(pUnoCrsr)
        SwXTextCursor::SetString(*pUnoCrsr, aString);
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 09:56:57---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXTextPortion::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef;
    if(!xRef.is())
    {
        uno::Reference< beans::XPropertySetInfo >  xInfo = aPropSet.getPropertySetInfo();
        // PropertySetInfo verlaengern!
        const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
        xRef = new SfxExtItemPropertySetInfo(
            aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXTPORTION_EXTENSIONS),
            aPropSeq );
    }
    return xRef;

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
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXTextPortion::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(pUnoCrsr)
    {
        if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_TEXT_PORTION_TYPE))
        {
            OUString sRet;
            switch (ePortionType)
            {
                case PORTION_TEXT:          sRet = C2U("Text");break;
                case PORTION_FIELD:         sRet = C2U("TextField");break;
                case PORTION_FRAME:         sRet = C2U("Frame");break;
                case PORTION_FOOTNOTE:      sRet = C2U("Footnote");break;
                case PORTION_CONTROL_CHAR:  sRet = C2U("ControlCharacter");break;
                case PORTION_REFMARK_START:
                case PORTION_REFMARK_END:       sRet = C2U(UNO_NAME_REFERENCE_MARK);break;
                case PORTION_TOXMARK_START:
                case PORTION_TOXMARK_END:       sRet = C2U(UNO_NAME_DOCUMENT_INDEX_MARK);break;
                case PORTION_BOOKMARK_START :
                case PORTION_BOOKMARK_END :  sRet = C2U(UNO_NAME_BOOKMARK);break;
            }
            aAny <<= sRet;
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_CONTROL_CHARACTER))
        {
            if(PORTION_CONTROL_CHAR == ePortionType)
                aAny <<= (sal_Int16) nControlChar;
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_DOCUMENT_INDEX_MARK))
            aAny <<= xTOXMark;
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_REFERENCE_MARK))
            aAny <<= xRefMark;
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_BOOKMARK))
            aAny <<= xBookmark;
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_FOOTNOTE))
            aAny <<= xFootnote;
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_IS_COLLAPSED))
        {
            BOOL bStart = TRUE, bPut = TRUE;
            switch (ePortionType)
            {
                case PORTION_REFMARK_START:
                case PORTION_BOOKMARK_START :
                case PORTION_TOXMARK_START:
                case PORTION_REFMARK_END:
                case PORTION_TOXMARK_END:
                case PORTION_BOOKMARK_END :  ;
                    aAny.setValue(&bIsCollapsed, ::getBooleanCppuType());
                break;
                default:
                    bPut = FALSE;
            }
        }
        else if(COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_IS_START))
        {
            BOOL bStart = TRUE, bPut = TRUE;
            switch (ePortionType)
            {
                case PORTION_REFMARK_START:
                case PORTION_BOOKMARK_START :
                case PORTION_TOXMARK_START:
                break;
                case PORTION_REFMARK_END:
                case PORTION_TOXMARK_END:
                case PORTION_BOOKMARK_END :  bStart = FALSE;
                break;
                default:
                    bPut = FALSE;
            }
            if(bPut)
                aAny.setValue(&bStart, ::getBooleanCppuType());
        }
        else
            aAny = SwXTextCursor::GetPropertyValue(*pUnoCrsr, aPropSet, rPropertyName);
    }
    else
        throw uno::RuntimeException();
    return aAny;
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removePropertyChangeListener(const OUString& PropertyName, const uno::Reference< beans::XPropertyChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:58---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 09:56:59---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener > & aListener) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
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
        eRet = SwXTextCursor::GetPropertyState(*pUnoCrsr, aPropSet, rPropertyName);
    else
        throw RuntimeException();
    return eRet;
}
/*-- 08.03.99 09:41:47---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SwXTextPortion::getPropertyStates(
        const uno::Sequence< OUString >& PropertyNames)
        throw( beans::UnknownPropertyException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwUnoCrsr* pUnoCrsr = ((SwXTextPortion*)this)->GetCrsr();
    if(!pUnoCrsr)
        throw RuntimeException();
    return SwXTextCursor::GetPropertyStates(*pUnoCrsr, aPropSet, PropertyNames);
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
OUString SwXTextPortion::getPresentation(sal_Bool bShowCommand) throw( uno::RuntimeException )
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
void SwXTextPortion::attach(const uno::Reference< XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException )
{

}
/*-- 11.12.98 09:57:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextPortion::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  aRet;
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
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 09:57:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextPortion::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextPortion::createContentEnumeration(const OUString& aServiceName)
        throw( uno::RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    uno::Reference< container::XEnumeration >  xRet = new SwXParaFrameEnumeration(*pUnoCrsr, PARAFRAME_PORTION_CHAR, pFrameFmt);
    return xRet;

}
/* -----------------24.03.99 13:30-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getAvailableServiceNames(void) throw( uno::RuntimeException )
{
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
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();

    sal_Bool bRet = sal_False;
    if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextPortion"))
        bRet = sal_True;
    else if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextField"))
        bRet = 0 != GetFldFmt();
    else
    {
        SwFrmFmt* pCurFrmFmt = pFrameFmt;
        if(!pCurFrmFmt && !pUnoCrsr->HasMark() && pUnoCrsr->GetCntntNode()->Len())
        {
            // hier die zeichengebundenen am Cursor suchen - hier kann es nur einen geben
            SwTxtAttr* pTxtAttr = pUnoCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                            pUnoCrsr->GetPoint()->nContent, RES_TXTATR_FLYCNT);
            DBG_ASSERT(pTxtAttr, "Hier muss doch ein Fly-Attribut zu finden sein!")
            if(pTxtAttr)
            {
                const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
                pCurFrmFmt = rFlyCnt.GetFrmFmt();
            }
        }
        if(pCurFrmFmt)
        {
            const SwNodeIndex* pIdx = pCurFrmFmt->GetCntnt().GetCntntIdx();
            const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];

            if((!pNd->IsNoTxtNode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame"))||
                (pNd->IsGrfNode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject")) ||
                (pNd->IsOLENode() && COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")))
                bRet = sal_True;
        }
    }
    return bRet;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXTextPortion::getSupportedServiceNames(void)
                                                throw( uno::RuntimeException )
{
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(!pUnoCrsr)
        throw uno::RuntimeException();
    sal_Bool bField = 0 != GetFldFmt();
    sal_uInt16 nCount = bField ? 2 : 1;
    uno::Sequence< OUString > aRet(nCount);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.TextPortion");
    if(bField)
        pArray[1] = C2U("com.sun.star.text.TextField");
    else
    {
        SwFrmFmt* pCurFrmFmt = pFrameFmt;
        if(!pCurFrmFmt && !pUnoCrsr->HasMark())
        {
            // hier die zeichengebundenen am Cursor suchen - hier kann es nur einen geben
            SwTxtAttr* pTxtAttr = pUnoCrsr->GetNode()->GetTxtNode()->GetTxtAttr(
                            pUnoCrsr->GetPoint()->nContent, RES_TXTATR_FLYCNT);
            // if any - it could also be an empty paragraph
            if(pTxtAttr)
            {
                const SwFmtFlyCnt& rFlyCnt = pTxtAttr->GetFlyCnt();
                pCurFrmFmt = rFlyCnt.GetFrmFmt();
            }
        }
        if(pCurFrmFmt)
        {
            aRet.realloc(2);
            pArray = aRet.getArray();
            const SwNodeIndex* pIdx = pCurFrmFmt->GetCntnt().GetCntntIdx();
            const SwNode* pNd = GetCrsr()->GetDoc()->GetNodes()[ pIdx->GetIndex() + 1 ];
            if(!pNd->IsNoTxtNode())
                pArray[1] = C2U("com.sun.star.text.TextFrame");
            else if(pNd->IsGrfNode())
                pArray[1] = C2U("com.sun.star.text.TextGraphicObject");
            else
                pArray[1] = C2U("com.sun.star.text.TextEmbeddedObject");
        }
    }
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
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.6  2000/09/18 16:04:34  willem.vandorp
    OpenOffice header added.

    Revision 1.5  2000/09/07 07:57:19  os
    new properties: IsCollapsed, IsStart

    Revision 1.4  2000/09/05 15:20:12  os
    footnote property at SwTextPortion

    Revision 1.3  2000/09/04 12:50:47  os
    TextPortionEnumeration almost completed

    Revision 1.2  2000/09/01 14:31:57  os
    properties added

    Revision 1.1  2000/05/04 15:14:08  os
    reduce size of unoobj.cxx


------------------------------------------------------------------------*/


