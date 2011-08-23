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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <swtypes.hxx>
#include <cmdid.h>

#include <unomid.h>
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _UNOCOLL_HXX
#include <unocoll.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _SVX_UNOPRNMS_HXX
#include <bf_svx/unoprnms.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPORT_HXX
#include <unoport.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <bf_svx/svdview.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <bf_svx/unoshape.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <bf_svx/fmglob.hxx>
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
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _CRSTATE_HXX //autogen
#include <crstate.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _E3D_SCENE3D_HXX
#include <bf_svx/scene3d.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <bf_sfx2/sfxuno.hxx>
#endif
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::rtl;

DECLARE_STL_USTRINGACCESS_MAP( Sequence< sal_Int8 > *,  SwShapeImplementationIdMap );

static SwShapeImplementationIdMap aImplementationIdMap;

/* -----------------22.01.99 13:19-------------------
 *
 * --------------------------------------------------*/
class SwShapeDescriptor_Impl
{
    SwFmtHoriOrient* 	pHOrient;
    SwFmtVertOrient* 	pVOrient;
    SwFmtAnchor*		pAnchor;
    SwFmtSurround*		pSurround;
    SvxULSpaceItem*		pULSpace;
    SvxLRSpaceItem*		pLRSpace;
    sal_Bool            bOpaque;
    uno::Reference< XTextRange >        xTextRange;

public:
    SwShapeDescriptor_Impl() :
     pHOrient(0),
     pVOrient(0),
     pAnchor(0),
     pSurround(0),
     pULSpace(0),
     pLRSpace(0),
     bOpaque(sal_False)
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
    SwFmtAnchor* 	GetAnchor(sal_Bool bCreate = sal_False)
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

    SwFmtSurround*	GetSurround(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pSurround)
                pSurround = new SwFmtSurround();
            return pSurround;
        }
    SvxLRSpaceItem*	GetLRSpace(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pLRSpace)
                pLRSpace = new SvxLRSpaceItem();
            return pLRSpace;
        }
    SvxULSpaceItem*	GetULSpace(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pULSpace)
                pULSpace = new SvxULSpaceItem();
            return pULSpace;
        }
    uno::Reference< XTextRange > &	GetTextRange()
    {
        return xTextRange;
    }
    sal_Bool    IsOpaque()
        {
            return bOpaque;
        }
    const sal_Bool&    GetOpaque()
        {
            return bOpaque;
        }
    void RemoveHOrient(){DELETEZ(pHOrient);}
    void RemoveVOrient(){DELETEZ(pVOrient);}
    void RemoveAnchor(){DELETEZ(pAnchor);}
    void RemoveSurround(){DELETEZ(pSurround);}
    void RemoveULSpace(){DELETEZ(pULSpace);}
    void RemoveLRSpace(){DELETEZ(pLRSpace);}
    void SetOpaque(sal_Bool bSet){bOpaque = bSet;}
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
SwFmDrawPage::~SwFmDrawPage() throw ()
{
    RemovePageView();
}
/*-- 22.01.99 11:13:07---------------------------------------------------

  -----------------------------------------------------------------------*/
const SdrMarkList& 	SwFmDrawPage::PreGroup(const uno::Reference< drawing::XShapes > & xShapes)
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
/*void 	SwFmDrawPage::PostGroup()
{
    pView->GroupMarked();
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
SdrPageView*	SwFmDrawPage::GetPageView()
{
    if(!pPageView)
        pPageView = pView->ShowPage( pPage, Point() );
    return pPageView;
}
/*-- 22.01.99 11:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void	SwFmDrawPage::RemovePageView()
{
    if(pPageView && pView)
        pView->HidePage( pPageView );
    pPageView = 0;
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< uno::XInterface >  	SwFmDrawPage::GetInterface( SdrObject* pObj )
{
    uno::Reference< XInterface >  xShape;
    if( pObj )
    {
        SwFrmFmt* pFmt = ::binfilter::FindFrmFmt( pObj );
        SwXShape* pxShape = (SwXShape*)SwClientIter( *pFmt ).
                                                First( TYPE( SwXShape ));
        if(pxShape)
        {
            xShape =  *(cppu::OWeakObject*)pxShape;
        }
        else
            xShape = pObj->getUnoShape();
    }
    return xShape;
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/
SdrObject* SwFmDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape > & xShape ) throw ()
{
    //TODO: stimmt das so - kann die Methode weg?
    return SvxFmDrawPage::_CreateSdrObject( xShape );
}
/*-- 22.01.99 11:13:09---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Reference< drawing::XShape >  SwFmDrawPage::_CreateShape( SdrObject *pObj ) const throw ()
{
    uno::Reference< drawing::XShape >  xRet;
    if(pObj->IsWriterFlyFrame() || pObj->GetObjInventor() == SWGInventor)
    {
        SwFlyDrawContact* pFlyContact = (SwFlyDrawContact*)pObj->GetUserCall();
        if(pFlyContact)
        {
            FlyCntType eType;
            SwFrmFmt* pFlyFmt = pFlyContact->GetFmt();
            SwDoc* pDoc = pFlyFmt->GetDoc();
            const SwNodeIndex* pIdx;
            if( RES_FLYFRMFMT == pFlyFmt->Which()
                && 0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() )
                && pIdx->GetNodes().IsDocNodes()
                )
            {
                const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
                if(!pNd->IsNoTxtNode())
                    eType = FLYCNTTYPE_FRM;
                else if( pNd->IsGrfNode() )
                    eType = FLYCNTTYPE_GRF;
                else if( pNd->IsOLENode() )
                    eType = FLYCNTTYPE_OLE;
            }
            else
                throw RuntimeException();
            xRet = SwXFrames::GetObject( *pFlyFmt, eType );
        }
     }
    else
    {
        // own block - temporary object has to be destroyed before
        // the delegator is set #81670#
        {
            xRet = SvxFmDrawPage::_CreateShape( pObj );
        }
        uno::Reference< XUnoTunnel > xShapeTunnel(xRet, uno::UNO_QUERY);
        //don't create an SwXShape if it already exists
        SwXShape* pShape = 0;
        if(xShapeTunnel.is())
            pShape = (SwXShape*)xShapeTunnel->getSomething(SwXShape::getUnoTunnelId());
        if(!pShape)
        {
            xShapeTunnel = 0;
            uno::Reference< uno::XInterface > xCreate(xRet, uno::UNO_QUERY);
            xRet = 0;
            Reference< XPropertySet >  xPrSet;
            if ( pObj->IsGroupObject() && (!pObj->Is3DObj() || ( PTR_CAST(E3dScene,pObj ) != NULL ) ) )
                xPrSet = new SwXGroupShape( xCreate );
            else
                xPrSet = new SwXShape( xCreate );
            xRet = uno::Reference< drawing::XShape >(xPrSet, uno::UNO_QUERY);
        }
    }
    return xRet;
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
    return C2U("com.sun.star.drawing.GenericDrawPage") == rServiceName;
}
/* -----------------------------06.04.00 13:14--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXDrawPage::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.drawing.GenericDrawPage");
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
    uno::Reference< XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pShape = 0;
    SvxShape* pSvxShape = 0;
    if(xShapeTunnel.is())
    {
        pShape = (SwXShape*)xShapeTunnel->getSomething(SwXShape::getUnoTunnelId());
        pSvxShape = (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId());
    }

    if(!pShape || pShape->GetRegisteredIn() || !pShape->m_bDescriptor )
    {
        RuntimeException aExcept;
        if(pShape)
            aExcept.Message = C2U("object already inserted");
        else
            aExcept.Message = C2U("illegal object");
        throw aExcept;
    }
    GetSvxPage()->add(xShape);

    uno::Reference< uno::XAggregation > 	xAgg = pShape->GetAggregationInterface();

    DBG_ASSERT(pSvxShape, "warum gibt es hier kein SvxShape?");
    //diese Position ist auf jeden Fall in 1/100 mm
    awt::Point aMM100Pos(pSvxShape->getPosition());

    //jetzt noch die Properties aus dem SwShapeDescriptor_Impl auswerten
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
    SwFmtAnchor aAnchor( FLY_IN_CNTNT );
    sal_Bool bOpaque = sal_False;
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
        bOpaque = pDesc->IsOpaque();
    }

    pSvxShape->setPosition(aMM100Pos);
    SdrObject* pObj = pSvxShape->GetSdrObject();
    // OD 25.06.2003 #108784# - set layer of new drawing object to corresponding
    // invisible layer.
    if(FmFormInventor != pObj->GetObjInventor())
        pObj->SetLayer( bOpaque ? pDoc->GetInvisibleHeavenId() : pDoc->GetInvisibleHellId() );
    else
        pObj->SetLayer(pDoc->GetInvisibleControlsId());

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
            else if(FLY_PAGE == aAnchor.GetAnchorId())
                aAnchor.SetAnchor(pInternalPam->Start());
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
    SwFrmFmt* pFmt = ::binfilter::FindFrmFmt( pObj );
    if(pFmt)
        pFmt->Add(pShape);
    pShape->m_bDescriptor = sal_False;

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
    Reference<lang::XComponent> xComp(xShape, UNO_QUERY);
    xComp->dispose();
}
/* -----------------17.02.99 10:38-------------------
 *
 * --------------------------------------------------*/
uno::Reference< drawing::XShapeGroup >  SwXDrawPage::group(const uno::Reference< drawing::XShapes > & xShapes) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    if(!pDoc || !xShapes.is())
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
                    if ( FLY_IN_CNTNT == ::binfilter::FindFrmFmt( (SdrObject*)pObj )->GetAnchor().GetAnchorId() )
                        bFlyInCnt = sal_True;
                }
                if( bFlyInCnt )
                    throw uno::RuntimeException();
                if( !bFlyInCnt )
                {
                    UnoActionContext aContext(pDoc);

                    SwDrawContact* pContact = pDoc->GroupSelection( *pPage->GetDrawView() );
                    pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkList(), FLY_AT_CNTNT/*int eAnchorId*/,
                        sal_True, sal_False );

                    pPage->GetDrawView()->UnmarkAll();
                    if(pContact)
                    {
                        uno::Reference< uno::XInterface >  xInt = pPage->GetInterface( pContact->GetMaster() );
                        xRet = uno::Reference< drawing::XShapeGroup >(xInt, UNO_QUERY);
                    }
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

            pDoc->UnGroupSelection( *pPage->GetDrawView() );
            pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkList(), FLY_AT_CNTNT/*int eAnchorId*/,
                        sal_True, sal_False );
        }
        pPage->RemovePageView();
    }
}
/* -----------------05.05.98 17:05-------------------
 *
 * --------------------------------------------------*/
SwFmDrawPage* 	SwXDrawPage::GetSvxPage()
{
    if(!xPageAgg.is() && pDoc)
    {
        vos::OGuard  aGuard(Application::GetSolarMutex());
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
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
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
    pImplementationId(0),
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
            SwFrmFmt* pFmt = ::binfilter::FindFrmFmt( pObj );
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
Sequence< Type > SwXShape::getTypes(  ) throw(RuntimeException)
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
/* -----------------------------04.04.01 07:37--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< sal_Int8 > SwXShape::getImplementationId(  ) throw(RuntimeException)
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    // do we need to compute the implementation id for this instance?
    if( !pImplementationId && xShapeAgg.is())
    {
        Reference< XShape > xAggShape;
        xShapeAgg->queryAggregation( ::getCppuType((Reference< XShape >*)0) ) >>= xAggShape;

        if( xAggShape.is() )
        {
            const OUString aShapeType( xAggShape->getShapeType() );
            // did we already compute an implementation id for the agregated shape type?
            SwShapeImplementationIdMap::iterator aIter( aImplementationIdMap.find(aShapeType ) );
            if( aIter == aImplementationIdMap.end() )
            {
                // we need to create a new implementation id for this
                // note: this memory is not free'd until application exists
                //		 but since we have a fixed set of shapetypes and the
                //		 memory will be reused this is ok.
                pImplementationId = new uno::Sequence< sal_Int8 >( 16 );
                rtl_createUuid( (sal_uInt8 *) pImplementationId->getArray(), 0, sal_True );
                aImplementationIdMap[ aShapeType ] = pImplementationId;
            }
            else
            {
                // use the already computed implementation id
                pImplementationId = (*aIter).second;
            }
        }
    }
    if( NULL == pImplementationId )
    {
        DBG_ERROR( "Could not create an implementation id for a SwXShape!" );
        return Sequence< sal_Int8 > ();
    }
    else
    {
        return *pImplementationId;
    }
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
    SwFrmFmt* 	pFmt = GetFrmFmt();
    const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                                _pMap, rPropertyName);
    if(xShapeAgg.is())
    {
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            //mit Layout kann der Anker umgesetzt werden, ohne dass sich die Position aendert
            if(pFmt)
            {
                SwAttrSet aSet(pFmt->GetAttrSet());
                SwDoc* pDoc = pFmt->GetDoc();
                if(RES_ANCHOR == pMap->nWID && MID_ANCHOR_ANCHORFRAME == pMap->nMemberId)
                {
                    sal_Bool bDone = sal_True;
                    Reference<XTextFrame> xFrame;
                    if(aValue >>= xFrame)
                    {
                        Reference<XUnoTunnel> xTunnel(xFrame, UNO_QUERY);
                        SwXFrame* pFrame = xTunnel.is() ?
                                (SwXFrame*)xTunnel->getSomething(SwXFrame::getUnoTunnelId()) : 0;
                        if(pFrame && pFrame->GetFrmFmt() &&
                            pFrame->GetFrmFmt()->GetDoc() == pDoc)
                        {
                            UnoActionContext aCtx(pDoc);
                            SfxItemSet aSet( pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                            aSet.SetParent(&pFmt->GetAttrSet());
                            SwFmtAnchor aAnchor = (const SwFmtAnchor&)aSet.Get(pMap->nWID);
                            SwPosition aPos(*pFrame->GetFrmFmt()->GetCntnt().GetCntntIdx());
                            aAnchor.SetAnchor(&aPos);
                            aAnchor.SetType(FLY_AT_FLY);
                            aSet.Put(aAnchor);
                            pFmt->SetAttr(aSet);
                            bDone = sal_True;
                        }
                    }
                    if(!bDone)
                        throw IllegalArgumentException();
                }
                 else if(RES_OPAQUE == pMap->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    DBG_ASSERT(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        // OD 25.06.2003 #108784# - set layer of new drawing
                        // object to corresponding invisible layer.
                        bool bIsVisible = pDoc->IsVisibleLayerId( pObj->GetLayer() );
                        if(FmFormInventor != pObj->GetObjInventor())
                        {
                            pObj->SetLayer( *(sal_Bool*)aValue.getValue()
                                            ? ( bIsVisible ? pDoc->GetHeavenId() : pDoc->GetInvisibleHeavenId() )
                                            : ( bIsVisible ? pDoc->GetHellId() : pDoc->GetInvisibleHellId() ));
                        }
                        else
                        {
                            pObj->SetLayer( bIsVisible ? pDoc->GetControlsId() : pDoc->GetInvisibleControlsId());
                        }

                    }

                }
                else if( pDoc->GetRootFrm() )
                {
                    UnoActionContext aCtx(pDoc);
                    if(RES_ANCHOR == pMap->nWID && MID_ANCHOR_ANCHORTYPE == pMap->nMemberId)
                    {
                        SdrObject* pObj = pFmt->FindSdrObject();
                        SdrMarkList aList;
                        SdrMark aMark(pObj);
                        aList.InsertEntry(aMark);
                        sal_Int32 nAnchor;
                        cppu::enum2int( nAnchor, aValue );
                        pDoc->ChgAnchor( aList, nAnchor,
                                                sal_False, sal_True );
                    }
                    else
                    {
                        aPropSet.setPropertyValue(*pMap, aValue, aSet);
                        pFmt->SetAttr(aSet);
                    }
                }
                else
                {
                    aPropSet.setPropertyValue(*pMap, aValue, aSet);
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
                    case RES_OPAQUE :
                        pImpl->SetOpaque(*(sal_Bool*)aValue.getValue());
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
            if( pFmt && pFmt->GetDoc()->GetRootFrm() )
            {
                UnoActionContext aCtx(pFmt->GetDoc());
                xPrSet->setPropertyValue(rPropertyName, aValue);
            }
            else
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
    SwFrmFmt* 	pFmt = GetFrmFmt();
    if(xShapeAgg.is())
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                    _pMap, rPropertyName);
        if(pMap)
        {
            if(pFmt)
            {
                if(RES_OPAQUE == pMap->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    DBG_ASSERT(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        // OD 02.07.2003 #108784# - consider invisible layers
                        sal_Bool bOpaque =
                            ( pObj->GetLayer() != pFmt->GetDoc()->GetHellId() &&
                              pObj->GetLayer() != pFmt->GetDoc()->GetInvisibleHellId() );
                        aRet.setValue(&bOpaque, ::getBooleanCppuType());
                    }
                }
                else if(FN_ANCHOR_POSITION == pMap->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    DBG_ASSERT(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        Point aPt = pObj->GetAnchorPos();
                        awt::Point aPoint( TWIP_TO_MM100( aPt.X() ),
                                           TWIP_TO_MM100( aPt.Y() ) );
                        aRet.setValue(&aPoint, ::getCppuType( (::com::sun::star::awt::Point*)0 ));
                    }
                }
                else
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    aRet = aPropSet.getPropertyValue(*pMap, rSet);
                }
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
                    case RES_OPAQUE :
                        aRet.setValue(&pImpl->GetOpaque(), ::getBooleanCppuType());
                    break;
                    case FN_ANCHOR_POSITION :
                        awt::Point aPoint;
                        aRet.setValue(&aPoint, ::getCppuType( (::com::sun::star::awt::Point*)0 ));
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
/* -----------------------------02.11.00 09:41--------------------------------

 ---------------------------------------------------------------------------*/
PropertyState SwXShape::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    Sequence< OUString > aNames(1);
    OUString* pStrings = aNames.getArray();
    pStrings[0] = rPropertyName;
    Sequence< PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}
/* -----------------------------02.11.00 09:41--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< PropertyState > SwXShape::getPropertyStates(
    const Sequence< OUString >& aPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFrmFmt* 	pFmt = GetFrmFmt();
    Sequence< PropertyState > aRet(aPropertyNames.getLength());
    if(xShapeAgg.is())
    {
        SvxShape* pSvxShape = GetSvxShape();
        sal_Bool bGroupMember = sal_False;
        sal_Bool bFormControl = sal_False;
        SdrObject* pObject = pSvxShape->GetSdrObject();
        if(pObject)
        {
            bGroupMember = pObject->GetUpGroup() != 0;
            bFormControl = pObject->GetObjInventor() == FmFormInventor;
        }
        const OUString* pNames = aPropertyNames.getConstArray();
        PropertyState* pRet = aRet.getArray();
        Reference< XPropertyState >  xShapePrState;
        for(sal_Int32 nProperty = 0; nProperty < aPropertyNames.getLength(); nProperty++)
        {
            const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                        _pMap, pNames[nProperty]);
            if(pMap)
            {
                if(RES_OPAQUE == pMap->nWID)
                    pRet[nProperty] = bFormControl ?
                        PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
                else if(FN_ANCHOR_POSITION == pMap->nWID)
                    pRet[nProperty] = PropertyState_DIRECT_VALUE;
                else if(FN_TEXT_RANGE == pMap->nWID)
                    pRet[nProperty] = PropertyState_DIRECT_VALUE;
                else if(bGroupMember)
                    pRet[nProperty] = PropertyState_DEFAULT_VALUE;
                else if(pFmt)
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    SfxItemState eItemState = rSet.GetItemState(pMap->nWID, FALSE);

                    //special handling for #88835#: vertical orientation should only be
                    //exported if the anchor type is FLY_AUTO_CNTNT
                    if(RES_VERT_ORIENT == pMap->nWID && SFX_ITEM_SET == eItemState)
                    {
                        const SwFmtAnchor& rAnchor = rSet.GetAnchor();
                        if(rAnchor.GetAnchorId() != FLY_IN_CNTNT)
                        {
                            eItemState = SFX_ITEM_DEFAULT;
                        }
                    }
                    if(SFX_ITEM_SET == eItemState)
                        pRet[nProperty] = PropertyState_DIRECT_VALUE;
                    else if(SFX_ITEM_DEFAULT == eItemState)
                        pRet[nProperty] = PropertyState_DEFAULT_VALUE;
                    else
                        pRet[nProperty] = PropertyState_AMBIGUOUS_VALUE;
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
                    }
                    if(pItem)
                        pRet[nProperty] = PropertyState_DIRECT_VALUE;
                    else
                        pRet[nProperty] = PropertyState_DEFAULT_VALUE;
                }
            }
            else
            {
                if(!xShapePrState.is())
                {
                    const uno::Type& rPStateType = ::getCppuType((Reference< XPropertyState >*)0);
                    uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
                    if(aPState.getValueType() != rPStateType || !aPState.getValue())
                        throw uno::RuntimeException();
                    xShapePrState = *(Reference< XPropertyState >*)aPState.getValue();
                }
                pRet[nProperty] = xShapePrState->getPropertyState(pNames[nProperty]);
            }
        }
    }
    else
        throw RuntimeException();
    return aRet;
}
/* -----------------------------02.11.00 09:41--------------------------------

 ---------------------------------------------------------------------------*/
void SwXShape::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFrmFmt* 	pFmt = GetFrmFmt();
    if(xShapeAgg.is())
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                        _pMap, rPropertyName);
        if(pMap)
        {
            if ( pMap->nFlags & PropertyAttribute::READONLY)
                throw PropertyVetoException ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            if(pFmt)
            {
                const SfxItemSet& rSet = pFmt->GetAttrSet();
                SfxItemSet aSet(pFmt->GetDoc()->GetAttrPool(), pMap->nWID, pMap->nWID);
                aSet.SetParent(&rSet);
                aSet.ClearItem(pMap->nWID);
                pFmt->GetDoc()->SetAttr(aSet, *pFmt);
            }
            else
            {
                switch(pMap->nWID)
                {
                    case RES_ANCHOR:		pImpl->RemoveAnchor(); 	break;
                    case RES_HORI_ORIENT: 	pImpl->RemoveHOrient(); break;
                    case RES_VERT_ORIENT:	pImpl->RemoveVOrient();	break;
                    case  RES_LR_SPACE:		pImpl->RemoveLRSpace();	break;
                    case  RES_UL_SPACE:		pImpl->RemoveULSpace();	break;
                    case  RES_SURROUND:		pImpl->RemoveSurround();break;
                    case RES_OPAQUE :       pImpl->SetOpaque(sal_False);  break;
                    case FN_TEXT_RANGE :
                    break;
                }
            }
        }
        else
        {
            const uno::Type& rPStateType = ::getCppuType((Reference< XPropertyState >*)0);
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            Reference< XPropertyState > xShapePrState = *(Reference< XPropertyState >*)aPState.getValue();
            xShapePrState->setPropertyToDefault( rPropertyName );
        }
    }
    else
        throw RuntimeException();
}
/* -----------------------------02.11.00 09:41--------------------------------

 ---------------------------------------------------------------------------*/
Any SwXShape::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFrmFmt* 	pFmt = GetFrmFmt();
    Any aRet;
    if(xShapeAgg.is())
    {
        const SfxItemPropertyMap*	pMap = SfxItemPropertyMap::GetByName(
                                        _pMap, rPropertyName);
        if(pMap)
        {
            if(pMap->nWID < RES_FRMATR_END && pFmt)
            {
                const SfxPoolItem& rDefItem =
                    pFmt->GetDoc()->GetAttrPool().GetDefaultItem(pMap->nWID);
                rDefItem.QueryValue(aRet, pMap->nMemberId);
            }
            else
                throw RuntimeException();
        }
        else
        {
            const uno::Type& rPStateType = ::getCppuType((Reference< XPropertyState >*)0);
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            Reference< XPropertyState > xShapePrState = *(Reference< XPropertyState >*)aPState.getValue();
            xShapePrState->getPropertyDefault( rPropertyName );
        }
    }
    else
        throw RuntimeException();
    return aRet;
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::addPropertyChangeListener(const OUString& PropertyName, const Reference< XPropertyChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 22.01.99 11:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::removePropertyChangeListener(
    const OUString& PropertyName,
    const Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 22.01.99 11:42:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::addVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 22.01.99 11:42:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXShape::removeVetoableChangeListener(const OUString& PropertyName, const Reference< XVetoableChangeListener > & aListener) throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
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
void SwXShape::attach(const Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());

    // get access to SwDoc
    // (see also SwXTextRange::XTextRangeToSwPaM)
    SwDoc*      pDoc = 0;
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    if(xRangeTunnel.is())
    {
        SwXTextRange* pRange = 0;
        OTextCursorHelper* pCursor = 0;
        SwXTextPortion* pPortion = 0;
        SwXText* pText = 0;

        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pText = (SwXText*)xRangeTunnel->getSomething(
                                SwXText::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
        pPortion = (SwXTextPortion*)xRangeTunnel->getSomething(
                                SwXTextPortion::getUnoTunnelId());

        if (pRange)
            pDoc = pRange->GetDoc();
        else if (!pDoc && pText)
            pDoc = pText->GetDoc();
        else if (!pDoc && pCursor)
            pDoc = pCursor->GetDoc();
        else if ( !pDoc && pPortion && pPortion->GetCrsr() )
            pDoc = pPortion->GetCrsr()->GetDoc();

    }

    if(!pDoc)
        throw uno::RuntimeException();
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if (pDocSh)
    {
        uno::Reference< frame::XModel > xModel;
        xModel = pDocSh->GetModel();
        uno::Reference< drawing::XDrawPageSupplier > xDPS(xModel, UNO_QUERY);
        if (xDPS.is())
        {
            uno::Reference< drawing::XDrawPage > xDP( xDPS->getDrawPage() );
            if (xDP.is())
            {
                Any aPos;  aPos <<= xTextRange;
                setPropertyValue(S2U("TextRange"), aPos);
                Reference< drawing::XShape > xTemp( (cppu::OWeakObject*) this, UNO_QUERY );
                xDP->add( xTemp );
            }
        }
    }
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
Reference< XTextRange >  SwXShape::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    Reference< XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if( rAnchor.GetAnchorId() != FLY_PAGE ||
            (rAnchor.GetCntntAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
            aRef = SwXTextRange::CreateTextRangeFromPosition(pFmt->GetDoc(), rPos, 0);
        }
    }
    else
        aRef = pImpl->GetTextRange();
    return aRef;
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::dispose(void) throw( RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        // OD 10.07.2003 #110742# - determine correct <SdrObject>
        SdrObject* pObj = pFmt->FindSdrObject();
        // OD 10.07.2003 #110742# - safety assertion:
        // <pObj> must be the same as <pFmt->FindSdrObject()>, if <pObj> isn't
        // a 'virtual' drawing object.
        ASSERT( !pObj->ISA(SwDrawVirtObj) || pObj == pFmt->FindSdrObject(),
                "<SwXShape::dispose(..) - different 'master' drawing objects!!" );
        // OD 10.07.2003 #110742# - perform delete of draw frame format *not*
        // for 'virtual' drawing objects.
        if(pObj && pObj->IsInserted())
        {
            if( pFmt->GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
                {
                    const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                    SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                    const xub_StrLen nIdx = rPos.nContent.GetIndex();
                    pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
                }
            else
                pFmt->GetDoc()->DelLayoutFmt( pFmt );
        }
    }
    if(xShapeAgg.is())
    {
        uno::Any aAgg(xShapeAgg->queryAggregation( ::getCppuType((Reference<XComponent>*)0)));
        Reference<XComponent> xComp;
        aAgg >>= xComp;
        if(xComp.is())
            xComp->dispose();
    }
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::addEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, UNO_QUERY);
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
         pSvxShape->addEventListener(aListener);
}
/* -----------------14.04.99 13:02-------------------
 *
 * --------------------------------------------------*/
void SwXShape::removeEventListener(const Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
        pSvxShape->removeEventListener(aListener);
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
OUString SwXShape::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXShape");
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXShape::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    sal_Bool bRet = sal_False;
    if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.drawing.Shape"))
        bRet = sal_True;
    else if(xShapeAgg.is())
    {
        Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, UNO_QUERY);
        SvxShape* pSvxShape = GetSvxShape();
        bRet = pSvxShape->supportsService(rServiceName);
    }
    return bRet;
}
/* -----------------03.06.99 08:53-------------------
 *
 * --------------------------------------------------*/
Sequence< OUString > SwXShape::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aSeq;
    if(xShapeAgg.is())
    {
        Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, UNO_QUERY);
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
SvxShape*	SwXShape::GetSvxShape()
{
    SvxShape* pSvxShape = 0;
    if(xShapeAgg.is())
    {
        Reference< XUnoTunnel > xShapeTunnel(xShapeAgg, UNO_QUERY);
        if(xShapeTunnel.is())
            pSvxShape = (SvxShape*)xShapeTunnel->getSomething(SvxShape::getUnoTunnelId());
    }
    return pSvxShape;
}
/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXGroupShape::SwXGroupShape(Reference< XInterface > & xShape) :
        SwXShape(xShape)
{
#ifdef DBG_UTIL
    Reference<XShapes> xShapes(xShapeAgg, UNO_QUERY);
    DBG_ASSERT(xShapes.is(), "no SvxShape found or shape is not a group shape");
#endif
}

/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXGroupShape::~SwXGroupShape()
{
}
/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXGroupShape::queryInterface( const uno::Type& rType ) throw(RuntimeException)
{
    Any aRet;
    if(rType == ::getCppuType((Reference<XShapes>*)0))
        aRet <<= Reference<XShapes>(this);
    else
        aRet = SwXShape::queryInterface(rType);
    return aRet;
}
/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXGroupShape::acquire(  ) throw()
{
    SwXShape::acquire();
}
/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXGroupShape::release(  ) throw()
{
    SwXShape::release();
}
/*-- 31.05.01 09:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXGroupShape::add( const Reference< XShape >& xShape ) throw (RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    SvxShape* pSvxShape = GetSvxShape();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pSvxShape && pFmt)
    {
        sal_Bool bOk = FALSE;
        Reference<XShapes> xShapes;
        if( xShapeAgg.is() )
        {
            const uno::Type& rType = ::getCppuType((Reference<XShapes>*)0 );
            Any aAgg = xShapeAgg->queryAggregation( rType );
            aAgg >>= xShapes;
        }
        if(xShapes.is())
            xShapes->add(xShape);
        else
            throw RuntimeException();

        Reference<XUnoTunnel> xTunnel(xShape, UNO_QUERY);
        SwXShape* pSwShape = 0;
        if(xShape.is())
            pSwShape = (SwXShape*)xTunnel->getSomething(SwXShape::getUnoTunnelId());
        if(pSwShape && pSwShape->m_bDescriptor)
        {
            SvxShape* pAddShape = (SvxShape*)xTunnel->getSomething(SvxShape::getUnoTunnelId());
            if(pAddShape)
            {
                SdrObject* pObj = pAddShape->GetSdrObject();
                if(pObj)
                {
                    SwDoc* pDoc = pFmt->GetDoc();
                    // OD 25.06.2003 #108784# - set layer of new drawing
                    // object to corresponding invisible layer.
                    if( FmFormInventor != pObj->GetObjInventor())
                    {
                        pObj->SetLayer( pSwShape->pImpl->GetOpaque()
                                        ? pDoc->GetInvisibleHeavenId()
                                        : pDoc->GetInvisibleHellId() );
                    }
                    else
                    {
                        pObj->SetLayer(pDoc->GetInvisibleControlsId());
                    }
                }
            }
            pSwShape->m_bDescriptor = sal_False;
            //add the group member to the format of the group
            SwFrmFmt* pFmt = ::binfilter::FindFrmFmt( pSvxShape->GetSdrObject() );
            if(pFmt)
                pFmt->Add(pSwShape);
        }
    }
    else
        throw RuntimeException();
}
/*-- 31.05.01 09:59:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXGroupShape::remove( const Reference< XShape >& xShape ) throw (RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    sal_Bool bOk = FALSE;
    Reference<XShapes> xShapes;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((Reference<XShapes>*)0 );
        Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xShapes;
    }
    if(!xShapes.is())
        throw RuntimeException();
    xShapes->remove(xShape);
}
/*-- 31.05.01 09:59:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXGroupShape::getCount(void) throw( uno::RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    sal_Int32 nRet = 0;
    Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((Reference<XIndexAccess>*)0 );
        Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw RuntimeException();
    return xAcc->getCount();
}
/*-- 31.05.01 09:59:20---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXGroupShape::getByIndex(sal_Int32 nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((Reference<XIndexAccess>*)0 );
        Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw RuntimeException();
    return xAcc->getByIndex(nIndex);
}
/*-- 31.05.01 09:59:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXGroupShape::getElementType(  ) throw(RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((Reference<XIndexAccess>*)0 );
        Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw RuntimeException();
    return xAcc->getElementType();
}
/*-- 31.05.01 09:59:22---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXGroupShape::hasElements(  ) throw(RuntimeException)
{
    vos::OGuard  aGuard(Application::GetSolarMutex());
    Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((Reference<XIndexAccess>*)0 );
        Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw RuntimeException();
    return xAcc->hasElements();
}

}
