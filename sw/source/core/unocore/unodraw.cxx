/*************************************************************************
 *
 *  $RCSfile: unodraw.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 08:58:54 $
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

#include <swtypes.hxx>
#include <cmdid.h>

#include <unomid.h>
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _ROOTFRM_HXX //autogen
#include <rootfrm.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _CRSTATE_HXX //autogen
#include <crstate.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::rtl;

/* -----------------22.01.99 13:19-------------------
 *
 * --------------------------------------------------*/
class SwShapeDescriptor_Impl
{
    SwFmtHoriOrient*    pHOrient;
    SwFmtVertOrient*    pVOrient;
    SwFmtAnchor*        pAnchor;
    SwFmtSurround*      pSurround;
    SvxULSpaceItem*     pULSpace;
    SvxLRSpaceItem*     pLRSpace;
    uno::Reference< XTextRange >        xTextRange;

public:
    SwShapeDescriptor_Impl() :
     pHOrient(0),
     pVOrient(0),
     pAnchor(0),
     pSurround(0),
     pULSpace(0),
     pLRSpace(0)
     {}

    ~SwShapeDescriptor_Impl()
    {
        delete pHOrient;
        delete pVOrient;
        delete pAnchor;
        delete pSurround;
        delete pULSpace;
        delete pLRSpace;
    }
    SwFmtAnchor*    GetAnchor(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pAnchor)
                pAnchor = new SwFmtAnchor(FLY_IN_CNTNT);
            return pAnchor;
        }
    SwFmtHoriOrient* GetHOrient(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pHOrient)
                pHOrient = new SwFmtHoriOrient();
            return pHOrient;
        }
    SwFmtVertOrient* GetVOrient(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pVOrient)
                pVOrient = new SwFmtVertOrient(0, VERT_TOP);
            return pVOrient;
        }

    SwFmtSurround*  GetSurround(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pSurround)
                pSurround = new SwFmtSurround();
            return pSurround;
        }
    SvxLRSpaceItem* GetLRSpace(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pLRSpace)
                pLRSpace = new SvxLRSpaceItem();
            return pLRSpace;
        }
    SvxULSpaceItem* GetULSpace(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pULSpace)
                pULSpace = new SvxULSpaceItem();
            return pULSpace;
        }
    uno::Reference< XTextRange > &  GetTextRange()
    {
        return xTextRange;
    }
};
/****************************************************************************
    class SwFmDrawPage
****************************************************************************/

/* -----------------28.01.99 12:03-------------------
 *
 * --------------------------------------------------*/
SwFmDrawPage::SwFmDrawPage( SdrPage* pPage ) :
    SvxFmDrawPage( pPage ), pPageView(0)
{
}

/*-- 22.01.99 11:13:07---------------------------------------------------

  -----------------------------------------------------------------------*/
SwFmDrawPage::~SwFmDrawPage()
{
    if(pPageView)
        pView->HidePage( pPageView );
}
/*-- 22.01.99 11:13:07---------------------------------------------------

  -----------------------------------------------------------------------*/
const SdrMarkList&  SwFmDrawPage::PreGroup(const uno::Reference< drawing::XShapes > & xShapes)
{
    _SelectObjectsInView( xShapes, GetPageView() );
    const SdrMarkList& rMarkList = pView->GetMarkList();
    return rMarkList;
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwFmDrawPage::PreUnGroup(const uno::Reference< drawing::XShapeGroup >  xShapeGroup)
{
    uno::Reference< drawing::XShape >  xShape(xShapeGroup, UNO_QUERY);
    _SelectObjectInView( xShape, GetPageView() );
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
/*void  SwFmDrawPage::PostGroup()
{
    pView->GroupMarked();
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
SdrPageView*    SwFmDrawPage::GetPageView()
{
    if(!pPageView)
        pPageView = pView->ShowPage( pPage, Point() );
    return pPageView;
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwFmDrawPage::RemovePageView()
{
    if(pPageView)
        pView->HidePage( pPageView );
    pPageView = 0;
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< uno::XInterface >   SwFmDrawPage::GetInterface( SdrObject* pObj )
{
    uno::Reference< drawing::XShape >  xShape;
    if( pObj )
    {
        //TODO: wenn bereits ein SwXShape am Format angemeldet ist, dann das herausreichen!
        SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
        SwXShape* pxShape = (SwXShape*)SwClientIter( *pFmt ).
                                                First( TYPE( SwXShape ));
        if(pxShape)
        {
            xShape = uno::Reference< drawing::XShape >((cppu::OWeakObject*)pxShape, uno::UNO_QUERY);
            pxShape->queryInterface(::getCppuType((const uno::Reference< drawing::XShape >*)0));
        }
        else
            xShape = _CreateShape( pObj );
    }
    return xShape;
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/
SdrObject* SwFmDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape > & xShape )
{
    //TODO: stimmt das so - kann die Methode weg?
    return SvxFmDrawPage::_CreateSdrObject( xShape );
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< drawing::XShape >  SwFmDrawPage::_CreateShape( SdrObject *pObj ) const
{
    uno::Reference< drawing::XShape >  xShape = SvxFmDrawPage::_CreateShape( pObj );
    uno::Reference< XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    //don't create an SwXShape if it already exists
    SwXShape* pShape = 0;
    if(xShapeTunnel.is())
        pShape = (SwXShape*)xShapeTunnel->getSomething(SwXShape::getUnoTunnelId());
    if(!pShape)
    {
        xShapeTunnel = 0;
        uno::Reference< uno::XInterface > xCreate(xShape, uno::UNO_QUERY);
        xShape = 0;
        uno::Reference< XPropertySet >  xPrSet = new SwXShape( xCreate );
        xShape = uno::Reference< drawing::XShape >(xPrSet, uno::UNO_QUERY);
    }
    return xShape;
}

/****************************************************************************
    class SwXDrawPage
****************************************************************************/
/* -----------------------------06.04.00 13:14--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXDrawPage::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXDrawPage");
}
/* -----------------------------06.04.00 13:14--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXDrawPage::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return C2U("com.sun.star.drawing.DrawPage") == rServiceName;
}
/* -----------------------------06.04.00 13:14--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXDrawPage::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.drawing.DrawPage");
    return aRet;
}
/*-- 22.01.99 11:22:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDrawPage::SwXDrawPage(SwDoc* pDc) :
    pDoc(pDc),
    pDrawPage(0)
{
}
/*-- 22.01.99 11:22:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXDrawPage::~SwXDrawPage()
{
    if(xPageAgg.is())
    {
        uno::Reference< uno::XInterface >  xInt;
        xPageAgg->setDelegator(xInt);
    }
}
/* -----------------------------15.06.00 15:00--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXDrawPage::queryInterface( const Type& aType ) throw(RuntimeException)
{
    Any aRet = SwXDrawPageBaseClass::queryInterface(aType);
    if(!aRet.hasValue())
    {
        aRet = GetSvxPage()->queryAggregation(aType);
    }
    return aRet;
}
/* -----------------------------15.06.00 15:01--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Type > SwXDrawPage::getTypes(  ) throw(RuntimeException)
{
    Sequence< uno::Type > aPageTypes = SwXDrawPageBaseClass::getTypes();
    Sequence< uno::Type > aSvxTypes = GetSvxPage()->getTypes();

    long nIndex = aPageTypes.getLength();
    aPageTypes.realloc(aPageTypes.getLength() + aSvxTypes.getLength() + 1);

    uno::Type* pPageTypes = aPageTypes.getArray();
    const uno::Type* pSvxTypes = aSvxTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos < aSvxTypes.getLength(); nPos++)
    {
        pPageTypes[nIndex++] = pSvxTypes[nPos];
    }
    pPageTypes[nIndex] = ::getCppuType((Reference< ::com::sun::star::form::XFormsSupplier>*)0);
    return aPageTypes;
}
/*-- 22.01.99 11:33:44---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXDrawPage::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->GetDrawModel())
        return 0;
    else
    {
        ((SwXDrawPage*)this)->GetSvxPage();
        return pDrawPage->getCount();
    }
}
/*-- 22.01.99 11:33:46---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXDrawPage::getByIndex(sal_Int32 nIndex)
        throw( IndexOutOfBoundsException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->GetDrawModel())
    {
        throw IndexOutOfBoundsException();
    }
    else
    {
         ((SwXDrawPage*)this)->GetSvxPage();
         return pDrawPage->getByIndex( nIndex);
    }
    return uno::Any();
}
/* -----------------22.01.99 13:13-------------------
 *
 * --------------------------------------------------*/
uno::Type  SwXDrawPage::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<drawing::XShape>*)0);
}
/* -----------------22.01.99 13:13-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXDrawPage::hasElements(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->GetDrawModel())
        return sal_False;
    else
        return ((SwXDrawPage*)this)->GetSvxPage()->hasElements();
}

/* -----------------22.01.99 12:42-------------------
 *
 * --------------------------------------------------*/
void SwXDrawPage::add(const uno::Reference< drawing::XShape > & xShape)
    throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
//  uno::Reference< uno::XInterface >  xInt(xShape, uno::UNO_QUERY);
    uno::Reference< XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pShape = 0;
    SvxShape* pSvxShape = 0;
    if(xShapeTunnel.is())
    {
        pShape = (SwXShape*)xShapeTunnel->getSomething(SwXShape::getUnoTunnelId());
        pSvxShape = (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId());
    }

    if(!pShape)
        throw uno::RuntimeException();
    GetSvxPage()->add(xShape);

    uno::Reference< uno::XAggregation >     xAgg = pShape->GetAggregationInterface();

    DBG_ASSERT(pSvxShape, "warum gibt es hier kein SvxShape?")
    //diese Position ist auf jeden Fall in 1/100 mm
    awt::Point aMM100Pos(pSvxShape->getPosition());

    //jetzt noch die Properties aus dem SwShapeDescriptor_Impl auswerten
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
    SwFmtAnchor aAnchor( FLY_IN_CNTNT );
    if( pDesc )
    {
        if(pDesc->GetSurround())
            aSet.Put( *pDesc->GetSurround());
        //die Items sind schon in Twip gesetzt
        if(pDesc->GetLRSpace())
        {
            aSet.Put(*pDesc->GetLRSpace());
        }
        if(pDesc->GetULSpace())
        {
            aSet.Put(*pDesc->GetULSpace());
        }
        if(pDesc->GetAnchor())
            aAnchor = *pDesc->GetAnchor();

        if(pDesc->GetHOrient())
        {
            if(pDesc->GetHOrient()->GetHoriOrient() == HORI_NONE)
                aMM100Pos.X = TWIP_TO_MM100(pDesc->GetHOrient()->GetPos());
            aSet.Put( *pDesc->GetHOrient() );
        }
        if(pDesc->GetVOrient())
        {
            if(pDesc->GetVOrient()->GetVertOrient() == VERT_NONE)
                aMM100Pos.Y = TWIP_TO_MM100(pDesc->GetVOrient()->GetPos());
            aSet.Put( *pDesc->GetVOrient() );
        }

        if(pDesc->GetSurround())
            aSet.Put( *pDesc->GetSurround());
    }

    pSvxShape->setPosition(aMM100Pos);
    SdrObject* pObj = pSvxShape->GetSdrObject();
    pObj->SetLayer( pDoc->GetHeavenId() );

    SwPaM* pPam = new SwPaM(pDoc->GetNodes().GetEndOfContent());
    SwUnoInternalPaM* pInternalPam = 0;
    uno::Reference< XTextRange >  xRg;
    if( pDesc && (xRg = pDesc->GetTextRange()).is() )
    {
        pInternalPam = new SwUnoInternalPaM(*pDoc);
        if(SwXTextRange::XTextRangeToSwPaM(*pInternalPam, xRg))
        {
            if(FLY_AT_FLY == aAnchor.GetAnchorId() &&
                                !pInternalPam->GetNode()->FindFlyStartNode())
                        aAnchor.SetType(FLY_IN_CNTNT);
        }
        else
            throw uno::RuntimeException();
    }
    else if( aAnchor.GetAnchorId() != FLY_PAGE && pDoc->GetRootFrm() )
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aTmp(MM100_TO_TWIP(aMM100Pos.X), MM100_TO_TWIP(aMM100Pos.Y));
        pDoc->GetRootFrm()->GetCrsrOfst( pPam->GetPoint(), aTmp, &aState );
        aAnchor.SetAnchor( pPam->GetPoint() );

        aSet.Put( SwFmtVertOrient( 0, VERT_TOP ) );
    }
    else
    {
        aAnchor.SetType(FLY_PAGE);
         awt::Size aDescSize(xShape->getSize());

        Point aTmp(MM100_TO_TWIP(aMM100Pos.X), MM100_TO_TWIP(aMM100Pos.Y));
         Rectangle aRect( aTmp, Size(MM100_TO_TWIP(aDescSize.Width),
                                    MM100_TO_TWIP(aDescSize.Height)));
        pObj->SetLogicRect( aRect );
        aSet.Put( SwFmtHoriOrient( aTmp.X(), HORI_NONE, FRAME ) );
        aSet.Put( SwFmtVertOrient( aTmp.Y(), VERT_NONE, FRAME ) );
    }
    aSet.Put(aAnchor);
    SwPaM* pTemp = pInternalPam;
    if ( !pTemp )
        pTemp = pPam;
    UnoActionContext aAction(pDoc);
    pDoc->Insert( *pTemp, *pObj, &aSet );
    SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
    if(pFmt)
        pFmt->Add(pShape);

    delete pPam;
    delete pInternalPam;
}
/* -----------------22.01.99 12:42-------------------
 *
 * --------------------------------------------------*/
void SwXDrawPage::remove(const uno::Reference< drawing::XShape > & xShape) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    GetSvxPage()->remove(xShape);
}
/* -----------------17.02.99 10:38-------------------
 *
 * --------------------------------------------------*/
uno::Reference< drawing::XShapeGroup >  SwXDrawPage::group(const uno::Reference< drawing::XShapes > & xShapes) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Reference< drawing::XShapeGroup >  xRet;
    if(xPageAgg.is())
    {

        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage)//kann das auch Null sein?
        {
            //markieren und MarkList zurueckgeben
            const SdrMarkList& rMarkList = pPage->PreGroup(xShapes);
            if ( rMarkList.GetMarkCount() > 1 )
            {
                sal_Bool bFlyInCnt = sal_False;
                for ( sal_uInt16 i = 0; !bFlyInCnt && i < rMarkList.GetMarkCount(); ++i )
                {
                    const SdrObject *pObj = rMarkList.GetMark( i )->GetObj();
                    if ( FLY_IN_CNTNT == ::FindFrmFmt( (SdrObject*)pObj )->GetAnchor().GetAnchorId() )
                        bFlyInCnt = sal_True;
                }
                if( bFlyInCnt )
                    throw uno::RuntimeException();
                if( !bFlyInCnt )
                {
                    UnoActionContext aContext(pDoc);
                    pDoc->StartUndo( UNDO_START );

                    SwDrawContact* pContact = pDoc->GroupSelection( *pPage->GetDrawView() );
                    pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkList(), FLY_AT_CNTNT/*int eAnchorId*/,
                        sal_True, sal_False );

                    pPage->GetDrawView()->UnmarkAll();
                    if(pContact)
                    {
                        uno::Reference< uno::XInterface >  xInt = pPage->GetInterface( pContact->GetMaster() );
                        uno::Reference< XPropertySet >  xTmp = new SwXShape(xInt);
                        xRet = uno::Reference< drawing::XShapeGroup >(xTmp, UNO_QUERY);
                    }
                    pDoc->EndUndo( UNDO_END );
                }
            }
            pPage->RemovePageView();
        }
    }
    return xRet;
}
/* -----------------17.02.99 10:38-------------------
 *
 * --------------------------------------------------*/
void SwXDrawPage::ungroup(const uno::Reference< drawing::XShapeGroup > & xShapeGroup) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc)
        throw uno::RuntimeException();
    if(xPageAgg.is())
    {
        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage)//kann das auch Null sein?
        {
            pPage->PreUnGroup(xShapeGroup);
            UnoActionContext aContext(pDoc);
            pDoc->StartUndo( UNDO_START );

            pDoc->UnGroupSelection( *pPage->GetDrawView() );
            pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkList(), FLY_AT_CNTNT/*int eAnchorId*/,
                        sal_True, sal_False );
            pDoc->EndUndo( UNDO_END );
        }
        pPage->RemovePageView();
    }
}

/* -----------------05.05.98 17:05-------------------
 *
 * --------------------------------------------------*/
SwFmDrawPage*   SwXDrawPage::GetSvxPage()
{
    if(!xPageAgg.is() && pDoc)
    {
        SdrModel* pModel = pDoc->MakeDrawModel();
        SdrPage* pPage = pModel->GetPage( 0 );

        {
            // waehrend des queryInterface braucht man ein Ref auf das
            // Objekt, sonst wird es geloescht.
            pDrawPage = new SwFmDrawPage(pPage);
            uno::Reference< drawing::XDrawPage >  xPage = pDrawPage;
            uno::Any aAgg = xPage->queryInterface(::getCppuType((uno::Reference< uno::XAggregation >*)0));
            if(aAgg.getValueType() == ::getCppuType((uno::Reference< uno::XAggregation >*)0))
                xPageAgg = *(uno::Reference< uno::XAggregation >*)aAgg.getValue();
        }
        if( xPageAgg.is() )
            xPageAgg->setDelegator( (cppu::OWeakObject*)this );
    }
    return pDrawPage;
}
/****************************************************************************

****************************************************************************/
TYPEINIT1(SwXShape, SwClient);
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXShape::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXShape::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }

    if( xShapeAgg.is() )
    {
        const uno::Type& rTunnelType = ::getCppuType((uno::Reference<XUnoTunnel>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rTunnelType );
        if(aAgg.getValueType() == rTunnelType)
        {
            uno::Reference<XUnoTunnel> xAggTunnel =
                    *(uno::Reference<XUnoTunnel>*)aAgg.getValue();
            if(xAggTunnel.is())
                return xAggTunnel->getSomething(rId);
        }
    }
    return 0;
}
/* -----------------01.02.99 11:38-------------------
 *
 * --------------------------------------------------*/
SwXShape::SwXShape(uno::Reference< uno::XInterface > & xShape) :
    aPropSet(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_SHAPE)),
    _pMap(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_SHAPE)),
    pImpl(new SwShapeDescriptor_Impl),
    m_bDescriptor(sal_True)
{
    if(xShape.is())  // default Ctor
    {
        const uno::Type& rAggType = ::getCppuType((const uno::Reference< uno::XAggregation >*)0);
        //aAgg contains a reference of the SvxShape!
        {
            uno::Any aAgg = xShape->queryInterface(rAggType);
            if(aAgg.getValueType() == rAggType)
                xShapeAgg = *(uno::Reference< uno::XAggregation >*)aAgg.getValue();
        }
        xShape = 0;
        m_refCount++;
        if( xShapeAgg.is() )
            xShapeAgg->setDelegator( (cppu::OWeakObject*)this );
        m_refCount--;

        uno::Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pShape = 0;
        if(xShapeTunnel.is())
            pShape = (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId());

        SdrObject* pObj = pShape ? pShape->GetSdrObject() : 0;
        if(pObj)
        {
            SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
            if(pFmt)
                pFmt->Add(this);
        }
    }
}

/*-- 22.01.99 11:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXShape::~SwXShape()
{
    if (xShapeAgg.is())
    {
        uno::Reference< uno::XInterface >  xRef;
        xShapeAgg->setDelegator(xRef);
    }
    delete pImpl;
}
/* -----------------------------16.06.00 12:21--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXShape::queryInterface( const uno::Type& aType ) throw(RuntimeException)
{
    Any aRet = SwXShapeBaseClass::queryInterface(aType);
    if(!aRet.hasValue() && xShapeAgg.is())
        aRet = xShapeAgg->queryAggregation(aType);
    return aRet;
}
/* -----------------------------16.06.00 12:21--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Type > SAL_CALL SwXShape::getTypes(  ) throw(RuntimeException)
{
    Sequence< uno::Type > aRet = SwXShapeBaseClass::getTypes();
    if(xShapeAgg.is())
    {
        Any aProv = xShapeAgg->queryAggregation(::getCppuType((Reference< XTypeProvider >*)0));
        if(aProv.hasValue())
        {
            Reference< XTypeProvider > xAggProv;
            aProv >>= xAggProv;
            Sequence< uno::Type > aAggTypes = xAggProv->getTypes();
            const uno::Type* pAggTypes = aAggTypes.getConstArray();
            long nIndex = aRet.getLength();

            aRet.realloc(nIndex + aAggTypes.getLength());
            uno::Type* pBaseTypes = aRet.getArray();

            for(long i = 0; i < aAggTypes.getLength(); i++)
                pBaseTypes[nIndex++] = pAggTypes[i];
        }
    }
    return aRet;
}
/*-- 22.01.99 11:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XPropertySetInfo >  SwXShape::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Reference< XPropertySetInfo >  aRet;
    if(xShapeAgg.is())
    {
        const uno::Type& rPropSetType = ::getCppuType((const uno::Reference< XPropertySet >*)0);
        uno::Any aPSet = xShapeAgg->queryAggregation( rPropSetType );
        if(aPSet.getValueType() == rPropSetType && aPSet.getValue())
        {
            uno::Reference< XPropertySet >  xPrSet = *(uno::Reference< XPropertySet >*)aPSet.getValue();
            uno::Reference< XPropertySetInfo >  xInfo = xPrSet->getPropertySetInfo();
            // PropertySetInfo verlaengern!
            const uno::Sequence<Property> aPropSeq = xInfo->getProperties();
            aRet = new SfxExtItemPropertySetInfo( _pMap, aPropSeq );
        }
    }
    if(!aRet.is())
        aRet = new SfxItemPropertySetInfo( _pMap );
    return aRet;
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException,
        IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFrmFmt*   pFmt = GetFrmFmt();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    if(xShapeAgg.is())
    {
        if(pMap)
        {
            //mit Layout kann der Anker umgesetzt werden, ohne dass sich die Position aendert
            if(pFmt)
            {
                SwAttrSet aSet(pFmt->GetAttrSet());
                if(pFmt->GetDoc()->GetRootFrm() &&
                    COMPARE_EQUAL == rPropertyName.compareToAscii(UNO_NAME_ANCHOR_TYPE))
                {
                    UnoActionContext aCtx(pFmt->GetDoc());
                    SdrObject* pObj = pFmt->FindSdrObject();
                    SdrMarkList aList;
                    SdrMark aMark(pObj);
                    aList.InsertEntry(aMark);
                    sal_Int16 nAnchor;
                    aValue >>= nAnchor;
                    pFmt->GetDoc()->ChgAnchor( aList, nAnchor, sal_False, sal_True );
                }
                else
                {
                    aPropSet.setPropertyValue(rPropertyName, aValue, aSet);
                    pFmt->SetAttr(aSet);
                }
            }
            else
            {
                SfxPoolItem* pItem = 0;
                switch(pMap->nWID)
                {
                    case RES_ANCHOR:
                        pItem = pImpl->GetAnchor(sal_True);
                    break;
                    case RES_HORI_ORIENT:
                        pItem = pImpl->GetHOrient(sal_True);
                    break;
                    case RES_VERT_ORIENT:
                        pItem = pImpl->GetVOrient(sal_True);
                    break;
                    case  RES_LR_SPACE:
                        pItem = pImpl->GetLRSpace(sal_True);
                    break;
                    case  RES_UL_SPACE:
                        pItem = pImpl->GetULSpace(sal_True);
                    break;
                    case  RES_SURROUND:
                        pItem = pImpl->GetSurround(sal_True);
                    break;
                    case  FN_TEXT_RANGE:
                    {
                        const uno::Type rTextRangeType = ::getCppuType((uno::Reference< XTextRange>*)0);
                        if(aValue.getValueType() == rTextRangeType)
                        {
                            uno::Reference< XTextRange > & rRange = pImpl->GetTextRange();
                            rRange = *(uno::Reference< XTextRange > *)aValue.getValue();
                        }
                    }
                    break;
                }
                if(pItem)
                    ((SfxPoolItem*)pItem)->PutValue(aValue, pMap->nMemberId);
            }
        }
        else
        {
            uno::Reference< XPropertySet >  xPrSet;
            const uno::Type& rPSetType = ::getCppuType((const uno::Reference< XPropertySet >*)0);
            uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
            if(aPSet.getValueType() != rPSetType || !aPSet.getValue())
                throw uno::RuntimeException();
            xPrSet = *(uno::Reference< XPropertySet >*)aPSet.getValue();
            xPrSet->setPropertyValue(rPropertyName, aValue);
        }
    }
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXShape::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    //TODO: Descriptor interface
    if(xShapeAgg.is())
    {
        const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if(pMap)
        {
            if(pFmt)
            {
                const SwAttrSet& rSet = pFmt->GetAttrSet();
                aRet = aPropSet.getPropertyValue(rPropertyName, rSet);
            }
            else
            {
                SfxPoolItem* pItem = 0;
                switch(pMap->nWID)
                {
                    case RES_ANCHOR:
                        pItem = pImpl->GetAnchor();
                    break;
                    case RES_HORI_ORIENT:
                        pItem = pImpl->GetHOrient();
                    break;
                    case RES_VERT_ORIENT:
                        pItem = pImpl->GetVOrient();
                    break;
                    case  RES_LR_SPACE:
                        pItem = pImpl->GetLRSpace();
                    break;
                    case  RES_UL_SPACE:
                        pItem = pImpl->GetULSpace();
                    break;
                    case  RES_SURROUND:
                        pItem = pImpl->GetSurround();
                    break;
                    case FN_TEXT_RANGE :
                        aRet.setValue(&pImpl->GetTextRange(), ::getCppuType((Reference<XTextRange>*)0));
                    break;
                }
                if(pItem)
                    pItem->QueryValue(aRet, pMap->nMemberId);
            }
        }
        else
        {
            uno::Reference< XPropertySet >  xPrSet;
            const uno::Type& rPSetType = ::getCppuType((const uno::Reference< XPropertySet >*)0);
            uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
            if(aPSet.getValueType() != rPSetType || !aPSet.getValue())
                throw uno::RuntimeException();
            xPrSet = *(uno::Reference< XPropertySet >*)aPSet.getValue();
            aRet = xPrSet->getPropertyValue(rPropertyName);
        }
    }
    return aRet;
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::addPropertyChangeListener(const OUString& PropertyName, const uno::Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::removePropertyChangeListener(
    const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 22.01.99 11:42:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::addVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 22.01.99 11:42:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::removeVetoableChangeListener(const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 22.01.99 11:42:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, uno::RuntimeException )
{
    //hier passiert nichts
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
uno::Reference< XTextRange >  SwXShape::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    uno::Reference< XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if( pFmt->GetAnchor().GetAnchorId() != FLY_PAGE )
        {
            const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
            SwPaM aPam(rPos);
            DBG_ERROR("ParentText ?")
            uno::Reference< XText >  xText;
            aRef = new SwXTextRange(aPam, xText);
        }
    }
    else
        aRef = pImpl->GetTextRange();
    return aRef;
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::dispose(void) throw( uno::RuntimeException )
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
            pSvxShape->dispose();
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::addEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    uno::Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
         pSvxShape->addEventListener(aListener);
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( uno::RuntimeException )
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
        pSvxShape->removeEventListener(aListener);
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
OUString SwXShape::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXShape");
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXShape::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    sal_Bool bRet = sal_False;
    if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.drawing.Shape"))
        bRet = sal_True;
    else if(xShapeAgg.is())
    {
        uno::Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pSvxShape = GetSvxShape();
        bRet = pSvxShape->supportsService(rServiceName);
    }
    return bRet;
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXShape::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq;
    if(xShapeAgg.is())
    {
        uno::Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pSvxShape = GetSvxShape();
        if(pSvxShape)
            aSeq = pSvxShape->getSupportedServiceNames();
    }
    else
    {
        aSeq.realloc(1);
        aSeq.getArray()[0] = C2U("com.sun.star.drawing.Shape");
    }
    return aSeq;
}
/* -----------------------------15.03.00 14:54--------------------------------

 ---------------------------------------------------------------------------*/
SvxShape*   SwXShape::GetSvxShape()
{
    SvxShape* pSvxShape = 0;
    if(xShapeAgg.is())
    {
        uno::Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        if(xShapeTunnel.is())
            pSvxShape = (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId());
    }
    return pSvxShape;
}

