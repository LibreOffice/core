/*************************************************************************
 *
 *  $RCSfile: unorefmk.cxx,v $
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
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::rtl;

/******************************************************************
 * SwXReferenceMark
 ******************************************************************/

/* -----------------------------13.03.00 12:15--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXReferenceMark::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXReferenceMark::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXReferenceMark::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXReferenceMark");
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXReferenceMark::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.ReferenceMark")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent");
}
/* -----------------------------06.04.00 16:41--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXReferenceMark::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.ReferenceMark");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    return aRet;
}
/*-- 11.12.98 10:28:32---------------------------------------------------

  -----------------------------------------------------------------------*/
TYPEINIT1(SwXReferenceMark, SwClient);

SwXReferenceMark::SwXReferenceMark(SwDoc* pDc, const SwFmtRefMark* pRefMark) :
    aLstnrCntnr( (text::XTextContent*)this),
    pDoc(pDc),
    pMark(pRefMark),
    m_bIsDescriptor(0 == pRefMark)
{
    if(pRefMark)
        sMarkName = pRefMark->GetRefName();
    if(pDoc)
        pDoc->GetUnoCallBack()->Add(this);
}
/*-- 11.12.98 10:28:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXReferenceMark::~SwXReferenceMark()
{

}
/* -----------------03.11.99 14:14-------------------

 --------------------------------------------------*/
void SwXReferenceMark::InsertRefMark(SwPaM& rPam, SwDoc* pDoc)
{
    UnoActionContext aCont(pDoc);
    SwTxtAttr* pTxtAttr = 0;
    SwFmtRefMark aRefMark(sMarkName);
    SfxItemSet  aSet(pDoc->GetAttrPool(), RES_TXTATR_REFMARK, RES_TXTATR_REFMARK, 0L);
    aSet.Put(aRefMark);
    sal_Bool bMark = *rPam.GetPoint() != *rPam.GetMark();
    SwXTextCursor::SetCrsrAttr(rPam, aSet);

    if(bMark && *rPam.GetPoint() > *rPam.GetMark())
        rPam.Exchange();
    SwUnoCrsr* pCrsr = pDoc->CreateUnoCrsr( *rPam.Start() );
    if(!bMark)
    {
        pCrsr->SetMark();
        pCrsr->Left(1);
    }
    pTxtAttr = pCrsr->GetNode()->GetTxtNode()->GetTxtAttr(pCrsr->GetPoint()->nContent, RES_TXTATR_REFMARK);
    delete pCrsr;
    if(pTxtAttr)
    {
        pMark = &pTxtAttr->GetRefMark();
    }
    pDoc->GetUnoCallBack()->Add(this);
}

/* -----------------18.02.99 13:33-------------------
 *
 * --------------------------------------------------*/
void SwXReferenceMark::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    if(!m_bIsDescriptor)
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    SwXTextCursor* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (SwXTextCursor*)xRangeTunnel->getSomething(
                                SwXTextCursor::getUnoTunnelId());
    }
    SwDoc* pDocument = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDocument)
    {
        SwUnoInternalPaM aPam(*pDocument);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aPam, xTextRange);
        InsertRefMark(aPam, pDocument);
        m_bIsDescriptor = sal_False;
        pDoc = pDocument;
        pDoc->GetUnoCallBack()->Add(this);
    }
    else
        throw lang::IllegalArgumentException();
}
/*-- 11.12.98 10:28:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::attach(const uno::Reference< text::XTextRange > & xTextRange)
                throw( lang::IllegalArgumentException, uno::RuntimeException )
{
}
/*-- 11.12.98 10:28:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXReferenceMark::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< text::XTextRange >  xRet;
    if(IsValid())
    {
        const SwFmtRefMark* pNewMark = pDoc->GetRefMark(sMarkName);
        if(pNewMark && pNewMark == pMark)
        {
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                SwPaM* pPam  = pTxtMark->GetEnd() ?
                                new SwPaM( rTxtNode, *pTxtMark->GetEnd(),
                                    rTxtNode, *pTxtMark->GetStart()) :
                                new SwPaM(  rTxtNode, *pTxtMark->GetStart());


                xRet = ::CreateTextRangeFromPosition(pDoc,
                        *pPam->Start(), pPam->End());
                delete pPam;
            }
        }
    }
    return xRet;
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(IsValid())
    {
        const SwFmtRefMark* pNewMark = pDoc->GetRefMark(sMarkName);
        if(pNewMark && pNewMark == pMark)
        {
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                SwPaM* pPam  = pTxtMark->GetEnd() ?
                                new SwPaM( rTxtNode, *pTxtMark->GetEnd(),
                                    rTxtNode, *pTxtMark->GetStart()) :
                                new SwPaM(  rTxtNode, *pTxtMark->GetStart());

                if(pPam->HasMark())
                    pDoc->DeleteAndJoin(*pPam);
                else
                {
                    SwCursor aCrsr(*pPam->Start());
                    aCrsr.SetMark();
                    aCrsr.LeftRight(sal_False, 1);
                    pDoc->DeleteAndJoin(aCrsr);
                }
                delete pPam;
            }
        }
    }
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/*-- 11.12.98 10:28:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 10:28:36---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXReferenceMark::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(!IsValid() || !pDoc->GetRefMark(sMarkName))
    {
        throw uno::RuntimeException();
    }
    return sMarkName;
}
/*-- 11.12.98 10:28:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::setName(const OUString& Name_) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if(m_bIsDescriptor)
        sMarkName = String(Name_);
    else
    {
        String sNewName(Name_);
        if(!IsValid() || !pDoc->GetRefMark(sMarkName) || pDoc->GetRefMark(sNewName))
        {
            throw uno::RuntimeException();
        }
        const SwFmtRefMark* pCurMark = pDoc->GetRefMark(sMarkName);
        if(sNewName != sMarkName && pCurMark && pCurMark == pMark)
        {
            SwDoc* pTempDoc = pDoc;
            UnoActionContext aCont(pTempDoc);
            const SwTxtRefMark* pTxtMark = pMark->GetTxtRefMark();
            if(pTxtMark &&
                &pTxtMark->GetTxtNode().GetNodes() == &pTempDoc->GetNodes())
            {
                SwTxtNode& rTxtNode = (SwTxtNode&)pTxtMark->GetTxtNode();
                SwPaM* pPam  = pTxtMark->GetEnd() ?
                                new SwPaM( rTxtNode, *pTxtMark->GetEnd(),
                                    rTxtNode, *pTxtMark->GetStart()) :
                                new SwPaM(  rTxtNode, *pTxtMark->GetStart());

                //kill the old reference mark
                if(pPam->HasMark())
                    pTempDoc->DeleteAndJoin(*pPam);
                else
                {
                    SwCursor aCrsr(*pPam->Start());
                    aCrsr.SetMark();
                    aCrsr.LeftRight(sal_False, 1);
                    pDoc->DeleteAndJoin(aCrsr);
                }
                sMarkName = sNewName;
                //create a new one
                InsertRefMark(*pPam, pTempDoc);
                delete pPam;
                pDoc = pTempDoc;
            }
        }
    }
}
/* -----------------------------07.01.00 12:51--------------------------------

 ---------------------------------------------------------------------------*/
void    SwXReferenceMark::Invalidate()
{
    if(GetRegisteredIn())
    {
        ((SwModify*)GetRegisteredIn())->Remove(this);
        aLstnrCntnr.Disposing();
        pDoc = 0;
        pMark = 0;
    }
}
/*-- 11.12.98 10:28:37---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXReferenceMark::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify( this, pOld, pNew );
    if(!GetRegisteredIn())
        aLstnrCntnr.Disposing();
}

/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference< XPropertySetInfo > SwXReferenceMark::getPropertySetInfo(  ) throw(RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >  xRef =
        SfxItemPropertySet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_PARAGRAPH_EXTENSIONS)).
            getPropertySetInfo();
    return xRef;
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::setPropertyValue(
    const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    throw IllegalArgumentException();
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXReferenceMark::getPropertyValue( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aRet;
    if(!SwXParagraph::getDefaultTextContentValue(aRet, rPropertyName))
        throw UnknownPropertyException();
    return aRet;
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addPropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
            throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removePropertyChangeListener(
    const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
            throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 12.09.00 12:58:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::addVetoableChangeListener( const OUString& PropertyName,
    const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*-- 12.09.00 12:58:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXReferenceMark::removeVetoableChangeListener(
    const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.4  2000/09/18 16:04:34  willem.vandorp
    OpenOffice header added.

    Revision 1.3  2000/09/12 11:42:59  os
    #78682# support of service TextContent

    Revision 1.2  2000/09/11 09:57:06  os
    TYPEINFO

    Revision 1.1  2000/05/04 15:14:14  os
    reduce size of unoobj.cxx


------------------------------------------------------------------------*/

