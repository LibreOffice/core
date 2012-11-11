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


#include <cmdid.h>

#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unoparagraph.hxx>
#include <unotextrange.hxx>
#include <svx/svditer.hxx>
#include <swunohelper.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <fmtcntnt.hxx>
#include <fmtflcnt.hxx>
#include <txtatr.hxx>
#include <docsh.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <TextCursorHelper.hxx>
#include <dflyobj.hxx>
#include <ndtxt.hxx>
#include <svx/svdview.hxx>
#include <svx/unoshape.hxx>
#include <dcontact.hxx>
#include <svx/fmglob.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfollowtextflow.hxx>
#include <rootfrm.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <crstate.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/makesequence.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <switerator.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;

DECLARE_STL_USTRINGACCESS_MAP( uno::Sequence< sal_Int8 > *,  SwShapeImplementationIdMap );

static SwShapeImplementationIdMap aImplementationIdMap;

class SwShapeDescriptor_Impl
{
    SwFmtHoriOrient*    pHOrient;
    SwFmtVertOrient*    pVOrient;
    SwFmtAnchor*        pAnchor;
    SwFmtSurround*      pSurround;
    SvxULSpaceItem*     pULSpace;
    SvxLRSpaceItem*     pLRSpace;
    sal_Bool            bOpaque;
    uno::Reference< text::XTextRange > xTextRange;
    // #i26791#
    SwFmtFollowTextFlow* mpFollowTextFlow;
    // #i28701#
    SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPos;
    // #i28749#
    sal_Int16 mnPositionLayoutDir;

public:
    bool    bInitializedPropertyNotifier;

public:
    SwShapeDescriptor_Impl() :
     // #i32349# - no defaults, in order to determine on
     // adding a shape, if positioning attributes are set or not.
     pHOrient( 0L ),
     pVOrient( 0L ),
     pAnchor(0),
     pSurround(0),
     pULSpace(0),
     pLRSpace(0),
     bOpaque(sal_False),
     // #i26791#
     mpFollowTextFlow( new SwFmtFollowTextFlow( sal_False ) ),
     // #i28701# #i35017#
     pWrapInfluenceOnObjPos( new SwFmtWrapInfluenceOnObjPos(
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT ) ),
     // #i28749#
     mnPositionLayoutDir( text::PositionLayoutDir::PositionInLayoutDirOfAnchor ),
     bInitializedPropertyNotifier(false)
     {}

    ~SwShapeDescriptor_Impl()
    {
        delete pHOrient;
        delete pVOrient;
        delete pAnchor;
        delete pSurround;
        delete pULSpace;
        delete pLRSpace;
        // #i26791#
        delete mpFollowTextFlow;
        // #i28701#
        delete pWrapInfluenceOnObjPos;
    }
    SwFmtAnchor*    GetAnchor(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pAnchor)
            {
                pAnchor = new SwFmtAnchor(FLY_AS_CHAR);
            }
            return pAnchor;
        }
    SwFmtHoriOrient* GetHOrient(sal_Bool bCreate = sal_False)
        {
            if (bCreate && !pHOrient)
            {
                // #i26791#
                pHOrient = new SwFmtHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
            }
            return pHOrient;
        }
    SwFmtVertOrient* GetVOrient(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pVOrient)
            {
                // #i26791#
                pVOrient = new SwFmtVertOrient( 0, text::VertOrientation::NONE, text::RelOrientation::FRAME );
            }
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
                pLRSpace = new SvxLRSpaceItem(RES_LR_SPACE);
            return pLRSpace;
        }
    SvxULSpaceItem* GetULSpace(sal_Bool bCreate = sal_False)
        {
            if(bCreate && !pULSpace)
                pULSpace = new SvxULSpaceItem(RES_UL_SPACE);
            return pULSpace;
        }
    uno::Reference< text::XTextRange > &    GetTextRange()
    {
        return xTextRange;
    }
    sal_Bool    IsOpaque() const
        {
            return bOpaque;
        }
    const sal_Bool&    GetOpaque() const
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

    // #i26791#
    SwFmtFollowTextFlow* GetFollowTextFlow( sal_Bool _bCreate = sal_False )
    {
        if ( _bCreate && !mpFollowTextFlow )
            mpFollowTextFlow = new SwFmtFollowTextFlow( sal_False );
        return mpFollowTextFlow;
    }
    void RemoveFollowTextFlow()
    {
        DELETEZ(mpFollowTextFlow);
    }

    // #i28749#
    sal_Int16 GetPositionLayoutDir() const
    {
        return mnPositionLayoutDir;
    }
    void SetPositionLayoutDir( sal_Int16 _nPositionLayoutDir )
    {
        switch ( _nPositionLayoutDir )
        {
            case text::PositionLayoutDir::PositionInHoriL2R:
            case text::PositionLayoutDir::PositionInLayoutDirOfAnchor:
            {
                mnPositionLayoutDir = _nPositionLayoutDir;
            }
            break;
            default:
            {
                OSL_FAIL( "<SwShapeDescriptor_Impl::SetPositionLayoutDir(..)> - invalid attribute value." );
            }
        }
    }
    void RemovePositionLayoutDir()
    {
        mnPositionLayoutDir = text::PositionLayoutDir::PositionInLayoutDirOfAnchor;
    }

    // #i28701#
    inline SwFmtWrapInfluenceOnObjPos* GetWrapInfluenceOnObjPos(
                                        const sal_Bool _bCreate = sal_False )
    {
        if ( _bCreate && !pWrapInfluenceOnObjPos )
        {
            pWrapInfluenceOnObjPos = new SwFmtWrapInfluenceOnObjPos(
                        // #i35017#
                        text::WrapInfluenceOnPosition::ONCE_CONCURRENT );
        }
        return pWrapInfluenceOnObjPos;
    }
    inline void RemoveWrapInfluenceOnObjPos()
    {
        DELETEZ(pWrapInfluenceOnObjPos);
    }
};

/****************************************************************************
    class SwFmDrawPage
****************************************************************************/
SwFmDrawPage::SwFmDrawPage( SdrPage* pPage ) :
    SvxFmDrawPage( pPage ), pPageView(0)
{
}

SwFmDrawPage::~SwFmDrawPage() throw ()
{
    RemovePageView();
}

const SdrMarkList&  SwFmDrawPage::PreGroup(const uno::Reference< drawing::XShapes > & xShapes)
{
    _SelectObjectsInView( xShapes, GetPageView() );
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    return rMarkList;
}

void SwFmDrawPage::PreUnGroup(const uno::Reference< drawing::XShapeGroup >  xShapeGroup)
{
    uno::Reference< drawing::XShape >  xShape( xShapeGroup, uno::UNO_QUERY);
    _SelectObjectInView( xShape, GetPageView() );
}

SdrPageView*    SwFmDrawPage::GetPageView()
{
    if(!pPageView)
        pPageView = mpView->ShowSdrPage( mpPage );
    return pPageView;
}

void    SwFmDrawPage::RemovePageView()
{
    if(pPageView && mpView)
        mpView->HideSdrPage();
    pPageView = 0;
}

uno::Reference< uno::XInterface >   SwFmDrawPage::GetInterface( SdrObject* pObj )
{
    uno::Reference< XInterface >  xShape;
    if( pObj )
    {
        SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
        SwXShape* pxShape = SwIterator<SwXShape,SwFmt>::FirstElement( *pFmt );
        if(pxShape)
        {
            xShape =  *(cppu::OWeakObject*)pxShape;
        }
        else
            xShape = pObj->getUnoShape();
    }
    return xShape;
}

SdrObject* SwFmDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape > & xShape ) throw ()
{
    //TODO: stimmt das so - kann die Methode weg?
    return SvxFmDrawPage::_CreateSdrObject( xShape );
}

uno::Reference< drawing::XShape >  SwFmDrawPage::_CreateShape( SdrObject *pObj ) const throw ()
{
    uno::Reference< drawing::XShape >  xRet;
    if(pObj->ISA(SwVirtFlyDrawObj) || pObj->GetObjInventor() == SWGInventor)
    {
        SwFlyDrawContact* pFlyContact = (SwFlyDrawContact*)pObj->GetUserCall();
        if(pFlyContact)
        {
            FlyCntType eType = FLYCNTTYPE_ALL;
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
            {
                OSL_FAIL( "<SwFmDrawPage::_CreateShape(..)> - could not retrieve type. Thus, no shape created." );
                return xRet;
            }
            OSL_ENSURE( eType != FLYCNTTYPE_ALL, "unexpected FlyCntType value for eType" );
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
            pShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SwXShape::getUnoTunnelId()) ));
        if(!pShape)
        {
            xShapeTunnel = 0;
            uno::Reference< uno::XInterface > xCreate(xRet, uno::UNO_QUERY);
            xRet = 0;
            uno::Reference< beans::XPropertySet >  xPrSet;
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
    class SwXShapesEnumeration
****************************************************************************/
namespace
{
    class SwXShapesEnumeration
        : public SwSimpleEnumeration_Base
    {
        private:
            typedef ::std::list< ::com::sun::star::uno::Any > shapescontainer_t;
            shapescontainer_t m_aShapes;
        protected:
            virtual ~SwXShapesEnumeration() {};
        public:
            SwXShapesEnumeration(SwXDrawPage* const pDrawPage);

            //XEnumeration
            virtual sal_Bool SAL_CALL hasMoreElements(void) throw(uno::RuntimeException);
            virtual uno::Any SAL_CALL nextElement(void) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName(void) throw(uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(uno::RuntimeException);
            virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames(void) throw(uno::RuntimeException);
    };
}

SwXShapesEnumeration::SwXShapesEnumeration(SwXDrawPage* const pDrawPage)
    : m_aShapes()
{
    SolarMutexGuard aGuard;
    ::std::insert_iterator<shapescontainer_t> pInserter = ::std::insert_iterator<shapescontainer_t>(m_aShapes, m_aShapes.begin());
    sal_Int32 nCount = pDrawPage->getCount();
    for(sal_Int32 nIdx = 0; nIdx < nCount; nIdx++)
    {
        uno::Reference<drawing::XShape> xShape = uno::Reference<drawing::XShape>(pDrawPage->getByIndex(nIdx), uno::UNO_QUERY);
        *pInserter++ = uno::makeAny(xShape);
    }
}

sal_Bool SwXShapesEnumeration::hasMoreElements(void) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return !m_aShapes.empty();
}

uno::Any SwXShapesEnumeration::nextElement(void) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if(m_aShapes.empty())
        throw container::NoSuchElementException();
    uno::Any aResult = *m_aShapes.begin();
    m_aShapes.pop_front();
    return aResult;
}

OUString SwXShapesEnumeration::getImplementationName(void) throw(uno::RuntimeException)
{
    return C2U("SwXShapeEnumeration");
}

sal_Bool SwXShapesEnumeration::supportsService(const OUString& ServiceName) throw(uno::RuntimeException)
{
    return C2U("com.sun.star.container.XEnumeration") == ServiceName;
}

uno::Sequence< OUString > SwXShapesEnumeration::getSupportedServiceNames(void) throw(uno::RuntimeException)
{
    return ::comphelper::makeSequence(C2U("com.sun.star.container.XEnumeration"));
}

/****************************************************************************
    class SwXDrawPage
****************************************************************************/
uno::Reference< container::XEnumeration > SwXDrawPage::createEnumeration(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return uno::Reference< container::XEnumeration >(
        new SwXShapesEnumeration(this));
}

rtl::OUString SwXDrawPage::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXDrawPage");
}

sal_Bool SwXDrawPage::supportsService(const rtl::OUString& rServiceName) throw( uno::RuntimeException )
{
    return C2U("com.sun.star.drawing.GenericDrawPage") == rServiceName;
}

uno::Sequence< rtl::OUString > SwXDrawPage::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.drawing.GenericDrawPage");
    return aRet;
}

SwXDrawPage::SwXDrawPage(SwDoc* pDc) :
    pDoc(pDc),
    pDrawPage(0)
{
}

SwXDrawPage::~SwXDrawPage()
{
    if(xPageAgg.is())
    {
        uno::Reference< uno::XInterface >  xInt;
        xPageAgg->setDelegator(xInt);
    }
}

uno::Any SwXDrawPage::queryInterface( const uno::Type& aType )
                                                throw( uno::RuntimeException )
{
    uno::Any aRet = SwXDrawPageBaseClass::queryInterface(aType);
    if(!aRet.hasValue())
    {
        // secure with checking if page exists. This may not be the case
        // either for new SW docs with no yet graphics usage or when
        // the doc is closed and someone else still holds a UNO reference
        // to the XDrawPage (in that case, pDoc is set to 0)
        SwFmDrawPage* pPage = GetSvxPage();

        if(pPage)
        {
            aRet = pPage->queryAggregation(aType);
        }
    }
    return aRet;
}

uno::Sequence< uno::Type > SwXDrawPage::getTypes() throw( uno::RuntimeException )
{
    uno::Sequence< uno::Type > aPageTypes = SwXDrawPageBaseClass::getTypes();
    uno::Sequence< uno::Type > aSvxTypes = GetSvxPage()->getTypes();

    long nIndex = aPageTypes.getLength();
    aPageTypes.realloc(aPageTypes.getLength() + aSvxTypes.getLength() + 1);

    uno::Type* pPageTypes = aPageTypes.getArray();
    const uno::Type* pSvxTypes = aSvxTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos < aSvxTypes.getLength(); nPos++)
    {
        pPageTypes[nIndex++] = pSvxTypes[nPos];
    }
    pPageTypes[nIndex] = ::getCppuType((uno::Reference<form::XFormsSupplier2>*)0);
    return aPageTypes;
}

sal_Int32 SwXDrawPage::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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

uno::Any SwXDrawPage::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
               uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->GetDrawModel())
        throw lang::IndexOutOfBoundsException();

    ((SwXDrawPage*)this)->GetSvxPage();
    return pDrawPage->getByIndex( nIndex );
}

uno::Type  SwXDrawPage::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<drawing::XShape>*)0);
}

sal_Bool SwXDrawPage::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->GetDrawModel())
        return sal_False;
    else
        return ((SwXDrawPage*)this)->GetSvxPage()->hasElements();
}

void SwXDrawPage::add(const uno::Reference< drawing::XShape > & xShape)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pShape = 0;
    SvxShape* pSvxShape = 0;
    if(xShapeTunnel.is())
    {
        pShape      = reinterpret_cast< SwXShape * >(
                sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SwXShape::getUnoTunnelId()) ));
        pSvxShape   = reinterpret_cast< SvxShape * >(
                sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));
    }

    if(!pShape || pShape->GetRegisteredIn() || !pShape->m_bDescriptor )
    {
        uno::RuntimeException aExcept;
        if(pShape)
            aExcept.Message = C2U("object already inserted");
        else
            aExcept.Message = C2U("illegal object");
        throw aExcept;
    }

    if ( pSvxShape->GetSdrObject() )
    {
        if ( pSvxShape->GetSdrObject()->IsInserted() )
        {
            return;
        }
    }
    GetSvxPage()->add(xShape);

    uno::Reference< uno::XAggregation >     xAgg = pShape->GetAggregationInterface();

    OSL_ENSURE(pSvxShape, "warum gibt es hier kein SvxShape?");
    //diese Position ist auf jeden Fall in 1/100 mm
    awt::Point aMM100Pos(pSvxShape->getPosition());

    //jetzt noch die Properties aus dem SwShapeDescriptor_Impl auswerten
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
    SwFmtAnchor aAnchor( FLY_AS_CHAR );
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

        // #i32349# - if no horizontal position exists, create one
        if ( !pDesc->GetHOrient() )
        {
            SwFmtHoriOrient* pHori = pDesc->GetHOrient( sal_True );
            SwTwips nHoriPos = MM100_TO_TWIP(aMM100Pos.X);
            pHori->SetPos( nHoriPos );
        }
        {
            if(pDesc->GetHOrient()->GetHoriOrient() == text::HoriOrientation::NONE)
                aMM100Pos.X = TWIP_TO_MM100(pDesc->GetHOrient()->GetPos());
            aSet.Put( *pDesc->GetHOrient() );
        }
        // #i32349# - if no vertical position exists, create one
        if ( !pDesc->GetVOrient() )
        {
            SwFmtVertOrient* pVert = pDesc->GetVOrient( sal_True );
            SwTwips nVertPos = MM100_TO_TWIP(aMM100Pos.Y);
            pVert->SetPos( nVertPos );
        }
        {
            if(pDesc->GetVOrient()->GetVertOrient() == text::VertOrientation::NONE)
                aMM100Pos.Y = TWIP_TO_MM100(pDesc->GetVOrient()->GetPos());
            aSet.Put( *pDesc->GetVOrient() );
        }

        if(pDesc->GetSurround())
            aSet.Put( *pDesc->GetSurround());
        bOpaque = pDesc->IsOpaque();

        // #i26791#
        if ( pDesc->GetFollowTextFlow() )
        {
            aSet.Put( *pDesc->GetFollowTextFlow() );
        }

        // #i28701#
        if ( pDesc->GetWrapInfluenceOnObjPos() )
        {
            aSet.Put( *pDesc->GetWrapInfluenceOnObjPos() );
        }
    }

    pSvxShape->setPosition(aMM100Pos);
    SdrObject* pObj = pSvxShape->GetSdrObject();
    // #108784# - set layer of new drawing object to corresponding
    // invisible layer.
    if(FmFormInventor != pObj->GetObjInventor())
        pObj->SetLayer( bOpaque ? pDoc->GetInvisibleHeavenId() : pDoc->GetInvisibleHellId() );
    else
        pObj->SetLayer(pDoc->GetInvisibleControlsId());

    SwPaM* pPam = new SwPaM(pDoc->GetNodes().GetEndOfContent());
    SwUnoInternalPaM* pInternalPam = 0;
    uno::Reference< text::XTextRange >  xRg;
    if( pDesc && (xRg = pDesc->GetTextRange()).is() )
    {
        pInternalPam = new SwUnoInternalPaM(*pDoc);
        if (::sw::XTextRangeToSwPaM(*pInternalPam, xRg))
        {
            if(FLY_AT_FLY == aAnchor.GetAnchorId() &&
                                !pInternalPam->GetNode()->FindFlyStartNode())
            {
                        aAnchor.SetType(FLY_AS_CHAR);
            }
            else if (FLY_AT_PAGE == aAnchor.GetAnchorId())
            {
                aAnchor.SetAnchor(pInternalPam->Start());
            }
        }
        else
            throw uno::RuntimeException();
    }
    else if ((aAnchor.GetAnchorId() != FLY_AT_PAGE) && pDoc->GetCurrentLayout())
    {
        SwCrsrMoveState aState( MV_SETONLYTEXT );
        Point aTmp(MM100_TO_TWIP(aMM100Pos.X), MM100_TO_TWIP(aMM100Pos.Y));
        pDoc->GetCurrentLayout()->GetCrsrOfst( pPam->GetPoint(), aTmp, &aState );   //swmod 080218
        aAnchor.SetAnchor( pPam->GetPoint() );

        // #i32349# - adjustment of vertical positioning
        // attributes no longer needed, because its already got a default.
    }
    else
    {
        aAnchor.SetType(FLY_AT_PAGE);

        // #i32349# - adjustment of vertical positioning
        // attributes no longer needed, because its already got a default.
    }
    aSet.Put(aAnchor);
    SwPaM* pTemp = pInternalPam;
    if ( !pTemp )
        pTemp = pPam;
    UnoActionContext aAction(pDoc);
    pDoc->Insert( *pTemp, *pObj, &aSet, NULL );
    SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
    if(pFmt)
        pFmt->Add(pShape);
    pShape->m_bDescriptor = sal_False;

    delete pPam;
    delete pInternalPam;
}

void SwXDrawPage::remove(const uno::Reference< drawing::XShape > & xShape) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(xShape, uno::UNO_QUERY);
    xComp->dispose();
}

uno::Reference< drawing::XShapeGroup >  SwXDrawPage::group(const uno::Reference< drawing::XShapes > & xShapes) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
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
                    const SdrObject *pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if (FLY_AS_CHAR == ::FindFrmFmt(const_cast<SdrObject*>(
                                            pObj))->GetAnchor().GetAnchorId())
                    {
                        bFlyInCnt = sal_True;
                    }
                }
                if( bFlyInCnt )
                    throw uno::RuntimeException();
                if( !bFlyInCnt )
                {
                    UnoActionContext aContext(pDoc);
                    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

                    SwDrawContact* pContact = pDoc->GroupSelection( *pPage->GetDrawView() );
                    pDoc->ChgAnchor(
                        pPage->GetDrawView()->GetMarkedObjectList(),
                        FLY_AT_PARA/*int eAnchorId*/,
                        true, false );

                    pPage->GetDrawView()->UnmarkAll();
                    if(pContact)
                    {
                        uno::Reference< uno::XInterface >  xInt = pPage->GetInterface( pContact->GetMaster() );
                        xRet = uno::Reference< drawing::XShapeGroup >(xInt, uno::UNO_QUERY);
                    }
                    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
                }
            }
            pPage->RemovePageView();
        }
    }
    return xRet;
}

void SwXDrawPage::ungroup(const uno::Reference< drawing::XShapeGroup > & xShapeGroup) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(xPageAgg.is())
    {
        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage)//kann das auch Null sein?
        {
            pPage->PreUnGroup(xShapeGroup);
            UnoActionContext aContext(pDoc);
            pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

            pDoc->UnGroupSelection( *pPage->GetDrawView() );
            pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkedObjectList(),
                        FLY_AT_PARA/*int eAnchorId*/,
                        true, false );
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
        }
        pPage->RemovePageView();
    }
}

SwFmDrawPage*   SwXDrawPage::GetSvxPage()
{
    if(!xPageAgg.is() && pDoc)
    {
        SolarMutexGuard aGuard;
        // #i52858#
        SdrModel* pModel = pDoc->GetOrCreateDrawModel();
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

// renamed and outlined to detect where it's called
void SwXDrawPage::InvalidateSwDoc()
{
    pDoc = 0;
}

TYPEINIT1(SwXShape, SwClient);

namespace
{
    class theSwXShapeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXShapeUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXShape::getUnoTunnelId()
{
    return theSwXShapeUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXShape::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }

    if( xShapeAgg.is() )
    {
        const uno::Type& rTunnelType = ::getCppuType((uno::Reference<lang::XUnoTunnel>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rTunnelType );
        if(aAgg.getValueType() == rTunnelType)
        {
            uno::Reference<lang::XUnoTunnel> xAggTunnel =
                    *(uno::Reference<lang::XUnoTunnel>*)aAgg.getValue();
            if(xAggTunnel.is())
                return xAggTunnel->getSomething(rId);
        }
    }
    return 0;
}
namespace
{
    static void lcl_addShapePropertyEventFactories( SdrObject& _rObj, SwXShape& _rShape )
    {
        ::svx::PPropertyValueProvider pProvider( new ::svx::PropertyValueProvider( _rShape, "AnchorType" ) );
        _rObj.getShapePropertyChangeNotifier().registerProvider( ::svx::eTextShapeAnchorType, pProvider );
    }
}


SwXShape::SwXShape(uno::Reference< uno::XInterface > & xShape) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_SHAPE)),
    m_pPropertyMapEntries(aSwMapProvider.GetPropertyMapEntries(PROPERTY_MAP_TEXT_SHAPE)),
    pImplementationId(0),
    pImpl(new SwShapeDescriptor_Impl()),
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
            // #i31698#
            if ( xShapeAgg.is() )
            {
                xShapeAgg->queryAggregation( ::getCppuType((uno::Reference< drawing::XShape >*)0) ) >>= mxShape;
                OSL_ENSURE( mxShape.is(),
                        "<SwXShape::SwXShape(..)> - no XShape found at <xShapeAgg>" );
            }
        }
        xShape = 0;
        m_refCount++;
        if( xShapeAgg.is() )
            xShapeAgg->setDelegator( (cppu::OWeakObject*)this );
        m_refCount--;

        uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pShape = 0;
        if(xShapeTunnel.is())
            pShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));

        SdrObject* pObj = pShape ? pShape->GetSdrObject() : 0;
        if(pObj)
        {
            SwFrmFmt* pFmt = ::FindFrmFmt( pObj );
            if(pFmt)
                pFmt->Add(this);

            lcl_addShapePropertyEventFactories( *pObj, *this );
            pImpl->bInitializedPropertyNotifier = true;
        }
    }
}

void SwXShape::AddExistingShapeToFmt( SdrObject& _rObj )
{
    SdrObjListIter aIter( _rObj, IM_DEEPNOGROUPS );
    while ( aIter.IsMore() )
    {
        SdrObject* pCurrent = aIter.Next();
        OSL_ENSURE( pCurrent, "SwXShape::AddExistingShapeToFmt: invalid object list element!" );
        if ( !pCurrent )
            continue;

        SwXShape* pSwShape = NULL;
        uno::Reference< lang::XUnoTunnel > xShapeTunnel( pCurrent->getWeakUnoShape(), uno::UNO_QUERY );
        if ( xShapeTunnel.is() )
            pSwShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething( SwXShape::getUnoTunnelId() ) ) );
        if ( pSwShape )
        {
            if ( pSwShape->m_bDescriptor )
            {
                SwFrmFmt* pFmt = ::FindFrmFmt( const_cast< SdrObject* >( pCurrent ) );
                if ( pFmt )
                    pFmt->Add( pSwShape );
                pSwShape->m_bDescriptor = sal_False;
            }

            if ( !pSwShape->pImpl->bInitializedPropertyNotifier )
            {
                lcl_addShapePropertyEventFactories( *pCurrent, *pSwShape );
                pSwShape->pImpl->bInitializedPropertyNotifier = true;
            }
        }
    }
}

SwXShape::~SwXShape()
{
    if (xShapeAgg.is())
    {
        uno::Reference< uno::XInterface >  xRef;
        xShapeAgg->setDelegator(xRef);
    }
    delete pImpl;
}

uno::Any SwXShape::queryInterface( const uno::Type& aType ) throw( uno::RuntimeException )
{
    uno::Any aRet = SwXShapeBaseClass::queryInterface(aType);
    // #i53320# - follow-up of #i31698#
    // interface drawing::XShape is overloaded. Thus, provide
    // correct object instance.
    if(!aRet.hasValue() && xShapeAgg.is())
    {
        if(aType == ::getCppuType((uno::Reference<XShape>*)0))
            aRet <<= uno::Reference<XShape>(this);
        else
            aRet = xShapeAgg->queryAggregation(aType);
    }
    return aRet;
}

uno::Sequence< uno::Type > SwXShape::getTypes(  ) throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aRet = SwXShapeBaseClass::getTypes();
    if(xShapeAgg.is())
    {
        uno::Any aProv = xShapeAgg->queryAggregation(::getCppuType((uno::Reference< XTypeProvider >*)0));
        if(aProv.hasValue())
        {
            uno::Reference< XTypeProvider > xAggProv;
            aProv >>= xAggProv;
            uno::Sequence< uno::Type > aAggTypes = xAggProv->getTypes();
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

uno::Sequence< sal_Int8 > SwXShape::getImplementationId(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    // do we need to compute the implementation id for this instance?
    if( !pImplementationId && xShapeAgg.is())
    {
        uno::Reference< XShape > xAggShape;
        xShapeAgg->queryAggregation( ::getCppuType((uno::Reference< XShape >*)0) ) >>= xAggShape;

        if( xAggShape.is() )
        {
            const rtl::OUString aShapeType( xAggShape->getShapeType() );
            // did we already compute an implementation id for the agregated shape type?
            SwShapeImplementationIdMap::iterator aIter( aImplementationIdMap.find(aShapeType ) );
            if( aIter == aImplementationIdMap.end() )
            {
                // we need to create a new implementation id for this
                // note: this memory is not free'd until application exists
                //       but since we have a fixed set of shapetypes and the
                //       memory will be reused this is ok.
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
        OSL_FAIL( "Could not create an implementation id for a SwXShape!" );
        return uno::Sequence< sal_Int8 > ();
    }
    else
    {
        return *pImplementationId;
    }
}

uno::Reference< beans::XPropertySetInfo >  SwXShape::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< beans::XPropertySetInfo >  aRet;
    if(xShapeAgg.is())
    {
        const uno::Type& rPropSetType = ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
        uno::Any aPSet = xShapeAgg->queryAggregation( rPropSetType );
        if(aPSet.getValueType() == rPropSetType && aPSet.getValue())
        {
            uno::Reference< beans::XPropertySet >  xPrSet =
                    *(uno::Reference< beans::XPropertySet >*)aPSet.getValue();
            uno::Reference< beans::XPropertySetInfo >  xInfo = xPrSet->getPropertySetInfo();
            // PropertySetInfo verlaengern!
            const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
            aRet = new SfxExtItemPropertySetInfo( m_pPropertyMapEntries, aPropSeq );
        }
    }
    if(!aRet.is())
        aRet = m_pPropSet->getPropertySetInfo();
    return aRet;
}

void SwXShape::setPropertyValue(const rtl::OUString& rPropertyName, const uno::Any& aValue)
     throw( beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt*   pFmt = GetFrmFmt();
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if(xShapeAgg.is())
    {
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException ( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            //mit Layout kann der Anker umgesetzt werden, ohne dass sich die Position aendert
            if(pFmt)
            {
                SwAttrSet aSet(pFmt->GetAttrSet());
                SwDoc* pDoc = pFmt->GetDoc();
                if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORFRAME == pEntry->nMemberId)
                {
                    sal_Bool bDone = sal_True;
                    uno::Reference<text::XTextFrame> xFrame;
                    if(aValue >>= xFrame)
                    {
                        uno::Reference<lang::XUnoTunnel> xTunnel(xFrame, uno::UNO_QUERY);
                        SwXFrame* pFrame = xTunnel.is() ?
                                reinterpret_cast< SwXFrame * >(
                                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXFrame::getUnoTunnelId()) ))
                                : 0;
                        if(pFrame && pFrame->GetFrmFmt() &&
                            pFrame->GetFrmFmt()->GetDoc() == pDoc)
                        {
                            UnoActionContext aCtx(pDoc);
                            SfxItemSet aItemSet( pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                            aItemSet.SetParent(&pFmt->GetAttrSet());
                            SwFmtAnchor aAnchor = (const SwFmtAnchor&)aItemSet.Get(pEntry->nWID);
                            SwPosition aPos(*pFrame->GetFrmFmt()->GetCntnt().GetCntntIdx());
                            aAnchor.SetAnchor(&aPos);
                            aAnchor.SetType(FLY_AT_FLY);
                            aItemSet.Put(aAnchor);
                            pFmt->SetFmtAttr(aItemSet);
                            bDone = sal_True;
                        }
                    }
                    if(!bDone)
                        throw lang::IllegalArgumentException();
                }
                else if(RES_OPAQUE == pEntry->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    SAL_WARN_IF(!pSvxShape, "sw.uno", "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        // set layer of new drawing
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
                // #i26791# - special handling for property FN_TEXT_RANGE
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    SwFmtAnchor aAnchor( static_cast<const SwFmtAnchor&>(aSet.Get( RES_ANCHOR )) );
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
                    {
                        // set property <TextRange> not valid for to-page anchored shapes
                        throw lang::IllegalArgumentException();
                    }
                    else
                    {
                        SwUnoInternalPaM* pInternalPam =
                                        new SwUnoInternalPaM( *(pFmt->GetDoc()) );
                        uno::Reference< text::XTextRange > xRg;
                        aValue >>= xRg;
                        if (::sw::XTextRangeToSwPaM(*pInternalPam, xRg) )
                        {
                            if (aAnchor.GetAnchorId() == FLY_AS_CHAR)
                            {
                                //delete old SwFmtFlyCnt
                                //With AnchorAsCharacter the current TxtAttribute has to be deleted.
                                //Tbis removes the frame format too.
                                //To prevent this the connection between format and attribute has to be broken before.
                                const SwPosition *pPos = aAnchor.GetCntntAnchor();
                                SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                                SAL_WARN_IF( !pTxtNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                                const xub_StrLen nIdx = pPos->nContent.GetIndex();
                                SwTxtAttr * const pHnt =
                                    pTxtNode->GetTxtAttrForCharAt(
                                        nIdx, RES_TXTATR_FLYCNT );
                                SAL_WARN_IF( !pHnt || pHnt->Which() != RES_TXTATR_FLYCNT,
                                            "sw.uno", "Missing FlyInCnt-Hint." );
                                SAL_WARN_IF( !pHnt || pHnt->GetFlyCnt().GetFrmFmt() != pFmt,
                                            "sw.uno", "Wrong TxtFlyCnt-Hint." );
                                const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt())
                                    .SetFlyFmt();

                                //The connection is removed now the attribute can be deleted.
                                pTxtNode->DeleteAttributes(
                                    RES_TXTATR_FLYCNT, nIdx );
                                //create a new one
                                SwTxtNode *pNd = pInternalPam->GetNode()->GetTxtNode();
                                SAL_WARN_IF( !pNd, "sw.uno", "Cursor not at TxtNode." );
                                SwFmtFlyCnt aFmt( pFmt );
                                pNd->InsertItem(aFmt, pInternalPam->GetPoint()
                                        ->nContent.GetIndex(), 0 );
                            }
                            else
                            {
                                aAnchor.SetAnchor( pInternalPam->GetPoint() );
                                aSet.Put(aAnchor);
                                pFmt->SetFmtAttr(aSet);
                            }
                        }
                        else
                        {
                            throw uno::RuntimeException();
                        }
                        delete pInternalPam;
                    }
                }
                // #i28749#
                else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
                {
                    sal_Int16 nPositionLayoutDir = 0;
                    aValue >>= nPositionLayoutDir;
                    pFmt->SetPositionLayoutDir( nPositionLayoutDir );
                }
                else if( pDoc->GetCurrentLayout())  //swmod 080218
                {
                    UnoActionContext aCtx(pDoc);
                    if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                    {
                        SdrObject* pObj = pFmt->FindSdrObject();
                        SdrMarkList aList;
                        SdrMark aMark(pObj);
                        aList.InsertEntry(aMark);
                        sal_Int32 nAnchor = 0;
                        cppu::enum2int( nAnchor, aValue );
                        pDoc->ChgAnchor( aList, (RndStdIds)nAnchor,
                                                false, true );
                    }
                    else
                    {
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                        pFmt->SetFmtAttr(aSet);
                    }
                }
                else if( RES_FRM_SIZE == pEntry->nWID &&
                        ( pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT || pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH ) )
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    SAL_WARN_IF(!pSvxShape, "sw.uno", "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        sal_Int16 nPercent(100);
                        aValue >>= nPercent;
                        if ( pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH )
                            pObj->SetRelativeWidth( nPercent / 100.0 );
                        else
                            pObj->SetRelativeHeight( nPercent / 100.0 );
                    }
                }
                else
                {
                    m_pPropSet->setPropertyValue( *pEntry, aValue, aSet );

                    if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                    {
                        bool bSetAttr = true;
                        sal_Int32 eNewAnchor = SWUnoHelper::GetEnumAsInt32( aValue );

                        //if old anchor was in_cntnt the related text attribute has to be removed
                        const SwFmtAnchor& rOldAnchor = pFmt->GetAnchor();
                        RndStdIds eOldAnchorId = rOldAnchor.GetAnchorId();
                        SdrObject* pObj = pFmt->FindSdrObject();
                        SwFrmFmt *pFlyFmt = FindFrmFmt( pObj );
                        pFlyFmt->DelFrms();
                        if( text::TextContentAnchorType_AS_CHARACTER != eNewAnchor &&
                            (FLY_AS_CHAR == eOldAnchorId))
                        {
                            //With AnchorAsCharacter the current TxtAttribute has to be deleted.
                            //Tbis removes the frame format too.
                            //To prevent this the connection between format and attribute has to be broken before.
                            const SwPosition *pPos = rOldAnchor.GetCntntAnchor();
                            SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                            SAL_WARN_IF( !pTxtNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                            const xub_StrLen nIdx = pPos->nContent.GetIndex();
                            SwTxtAttr * const pHnt =
                                pTxtNode->GetTxtAttrForCharAt(
                                    nIdx, RES_TXTATR_FLYCNT );
                            SAL_WARN_IF( !pHnt || pHnt->Which() != RES_TXTATR_FLYCNT,
                                        "sw.uno", "Missing FlyInCnt-Hint." );
                            SAL_WARN_IF( !pHnt || pHnt->GetFlyCnt().GetFrmFmt() != pFlyFmt,
                                        "sw.uno", "Wrong TxtFlyCnt-Hint." );
                            const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt())
                                .SetFlyFmt();

                            //The connection is removed now the attribute can be deleted.
                            pTxtNode->DeleteAttributes(RES_TXTATR_FLYCNT, nIdx);
                        }
                        else if( text::TextContentAnchorType_AT_PAGE != eNewAnchor &&
                                (FLY_AT_PAGE == eOldAnchorId))
                        {
                            SwFmtAnchor aNewAnchor( dynamic_cast< const SwFmtAnchor& >( aSet.Get( RES_ANCHOR ) ) );
                            //if the fly has been anchored at page then it needs to be connected
                            //to the content position
                            SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                            if( pDoc->GetCurrentLayout() )
                            {
                                SwCrsrMoveState aState( MV_SETONLYTEXT );
                                Point aTmp( pObj->GetSnapRect().TopLeft() );
                                pDoc->GetCurrentLayout()->GetCrsrOfst( aPam.GetPoint(), aTmp, &aState );
                            }
                            else
                            {
                                //without access to the layout the last node of the body will be used as anchor position
                                aPam.Move( fnMoveBackward, fnGoDoc );
                            }
                            //anchor position has to be inserted after the text attribute has been inserted
                            aNewAnchor.SetAnchor( aPam.GetPoint() );
                            aSet.Put( aNewAnchor );
                            pFmt->SetFmtAttr(aSet);
                            bSetAttr = false;
                        }
                        if( text::TextContentAnchorType_AS_CHARACTER == eNewAnchor &&
                            (FLY_AS_CHAR != eOldAnchorId))
                        {
                            SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                            if( pDoc->GetCurrentLayout() )
                            {
                                SwCrsrMoveState aState( MV_SETONLYTEXT );
                                Point aTmp( pObj->GetSnapRect().TopLeft() );
                                pDoc->GetCurrentLayout()->GetCrsrOfst( aPam.GetPoint(), aTmp, &aState );
                            }
                            else
                            {
                                //without access to the layout the last node of the body will be used as anchor position
                                aPam.Move( fnMoveBackward, fnGoDoc );
                            }
                            //the RES_TXTATR_FLYCNT needs to be added now
                            SwTxtNode *pNd = aPam.GetNode()->GetTxtNode();
                            SAL_WARN_IF( !pNd, "sw.uno", "Crsr is not in a TxtNode." );
                            SwFmtFlyCnt aFmt( pFlyFmt );
                            pNd->InsertItem(aFmt,
                                aPam.GetPoint()->nContent.GetIndex(), 0 );
                            //aPam.GetPoint()->nContent--;
                        }
                        if( bSetAttr )
                            pFmt->SetFmtAttr(aSet);
                    }
                    else
                        pFmt->SetFmtAttr(aSet);
                }
            }
            else
            {
                SfxPoolItem* pItem = 0;
                switch(pEntry->nWID)
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
                        const uno::Type rTextRangeType =
                            ::getCppuType((uno::Reference< text::XTextRange>*)0);
                        if(aValue.getValueType() == rTextRangeType)
                        {
                            uno::Reference< text::XTextRange > & rRange = pImpl->GetTextRange();
                            rRange = *(uno::Reference< text::XTextRange > *)aValue.getValue();
                        }
                    }
                    break;
                    case RES_OPAQUE :
                        pImpl->SetOpaque(*(sal_Bool*)aValue.getValue());
                    break;
                    // #i26791#
                    case RES_FOLLOW_TEXT_FLOW:
                    {
                        pItem = pImpl->GetFollowTextFlow( sal_True );
                    }
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = pImpl->GetWrapInfluenceOnObjPos( sal_True );
                    }
                    break;
                    // #i28749#
                    case FN_SHAPE_POSITION_LAYOUT_DIR :
                    {
                        sal_Int16 nPositionLayoutDir = 0;
                        aValue >>= nPositionLayoutDir;
                        pImpl->SetPositionLayoutDir( nPositionLayoutDir );
                    }
                    break;
                }
                if(pItem)
                    ((SfxPoolItem*)pItem)->PutValue(aValue, pEntry->nMemberId);
            }
        }
        else
        {
            uno::Reference< beans::XPropertySet >  xPrSet;
            const uno::Type& rPSetType =
                ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
            uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
            if(aPSet.getValueType() != rPSetType || !aPSet.getValue())
                throw uno::RuntimeException();
            xPrSet = *(uno::Reference< beans::XPropertySet >*)aPSet.getValue();
            // #i31698# - setting the caption point of a
            // caption object doesn't have to change the object position.
            // Thus, keep the position, before the caption point is set and
            // restore it afterwards.
            awt::Point aKeepedPosition( 0, 0 );
            if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
            {
                    aKeepedPosition = getPosition();
            }
            if( pFmt && pFmt->GetDoc()->GetCurrentViewShell() ) //swmod 071108//swmod 071225
            {
                UnoActionContext aCtx(pFmt->GetDoc());
                xPrSet->setPropertyValue(rPropertyName, aValue);
            }
            else
                xPrSet->setPropertyValue(rPropertyName, aValue);
            // #i31698# - restore object position, if caption point is set.
            if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
            {
                setPosition( aKeepedPosition );
            }
        }
    }
}

uno::Any SwXShape::getPropertyValue(const rtl::OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if(pFmt)
            {
                if(RES_OPAQUE == pEntry->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    OSL_ENSURE(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        // consider invisible layers
                        sal_Bool bOpaque =
                            ( pObj->GetLayer() != pFmt->GetDoc()->GetHellId() &&
                              pObj->GetLayer() != pFmt->GetDoc()->GetInvisibleHellId() );
                        aRet.setValue(&bOpaque, ::getBooleanCppuType());
                    }
                }
                else if(FN_ANCHOR_POSITION == pEntry->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    OSL_ENSURE(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        Point aPt = pObj->GetAnchorPos();
                        awt::Point aPoint( TWIP_TO_MM100( aPt.X() ),
                                           TWIP_TO_MM100( aPt.Y() ) );
                        aRet.setValue(&aPoint, ::getCppuType( (awt::Point*)0 ));
                    }
                }
                // #i26791# - special handling for FN_TEXT_RANGE
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    const SwFmtAnchor aAnchor = pFmt->GetAnchor();
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
                    {
                        // return nothing, because property <TextRange> isn't
                        // valid for to-page anchored shapes
                        uno::Any aAny;
                        aRet = aAny;
                    }
                    else
                    {
                        if ( aAnchor.GetCntntAnchor() )
                        {
                            const uno::Reference< text::XTextRange > xTextRange
                                = SwXTextRange::CreateXTextRange(
                                                    *pFmt->GetDoc(),
                                                    *aAnchor.GetCntntAnchor(),
                                                    0L );
                            aRet.setValue(&xTextRange, ::getCppuType((uno::Reference<text::XTextRange>*)0));
                        }
                        else
                        {
                            // return nothing
                            uno::Any aAny;
                            aRet = aAny;
                        }
                    }
                }
                // #i28749#
                else if ( FN_SHAPE_TRANSFORMATION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::Transformation>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Transformation")) );
                }
                else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
                {
                    aRet <<= pFmt->GetPositionLayoutDir();
                }
                // #i36248#
                else if ( FN_SHAPE_STARTPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::StartPosition>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")) );
                }
                else if ( FN_SHAPE_ENDPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::EndPosition>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")) );
                }
                else
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    m_pPropSet->getPropertyValue(*pEntry, rSet, aRet);
                }
            }
            else
            {
                SfxPoolItem* pItem = 0;
                switch(pEntry->nWID)
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
                        aRet.setValue(&pImpl->GetTextRange(), ::getCppuType((uno::Reference<text::XTextRange>*)0));
                    break;
                    case RES_OPAQUE :
                        aRet.setValue(&pImpl->GetOpaque(), ::getBooleanCppuType());
                    break;
                    case FN_ANCHOR_POSITION :
                    {
                        awt::Point aPoint;
                        aRet.setValue(&aPoint, ::getCppuType( (awt::Point*)0 ));
                    }
                    break;
                    // #i26791#
                    case RES_FOLLOW_TEXT_FLOW :
                    {
                        pItem = pImpl->GetFollowTextFlow();
                    }
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = pImpl->GetWrapInfluenceOnObjPos();
                    }
                    break;
                    // #i28749#
                    case FN_SHAPE_TRANSFORMATION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::Transformation>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Transformation")) );
                    }
                    break;
                    case FN_SHAPE_POSITION_LAYOUT_DIR:
                    {
                        aRet <<= pImpl->GetPositionLayoutDir();
                    }
                    break;
                    // #i36248#
                    case FN_SHAPE_STARTPOSITION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::StartPosition>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")) );
                    }
                    break;
                    case FN_SHAPE_ENDPOSITION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::StartPosition>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")) );
                    }
                    break;
                }
                if(pItem)
                    pItem->QueryValue(aRet, pEntry->nMemberId);
            }
        }
        else
        {
            aRet = _getPropAtAggrObj( rPropertyName );

            // #i31698# - convert the position (translation)
            // of the drawing object in the transformation
            if ( rPropertyName == "Transformation" )
            {
                drawing::HomogenMatrix3 aMatrix;
                aRet >>= aMatrix;
                aRet <<= _ConvertTransformationToLayoutDir( aMatrix );
            }
            // #i36248#
            else if ( rPropertyName == "StartPosition" )
            {
                awt::Point aStartPos;
                aRet >>= aStartPos;
                // #i59051#
                aRet <<= _ConvertStartOrEndPosToLayoutDir( aStartPos );
            }
            else if ( rPropertyName == "EndPosition" )
            {
                awt::Point aEndPos;
                aRet >>= aEndPos;
                // #i59051#
                aRet <<= _ConvertStartOrEndPosToLayoutDir( aEndPos );
            }
            // #i59051#
            else if ( rPropertyName == "PolyPolygonBezier" )
            {
                drawing::PolyPolygonBezierCoords aPath;
                aRet >>= aPath;
                aRet <<= _ConvertPolyPolygonBezierToLayoutDir( aPath );
            }
        }
    }
    return aRet;
}

uno::Any SwXShape::_getPropAtAggrObj( const ::rtl::OUString& _rPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    uno::Any aRet;

    uno::Reference< beans::XPropertySet >  xPrSet;
    const uno::Type& rPSetType =
                ::getCppuType((const uno::Reference< beans::XPropertySet >*)0);
    uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
    if ( aPSet.getValueType() != rPSetType || !aPSet.getValue() )
    {
        throw uno::RuntimeException();
    }
    xPrSet = *(uno::Reference< beans::XPropertySet >*)aPSet.getValue();
    aRet = xPrSet->getPropertyValue( _rPropertyName );

    return aRet;
}

beans::PropertyState SwXShape::getPropertyState( const rtl::OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< rtl::OUString > aNames(1);
    rtl::OUString* pStrings = aNames.getArray();
    pStrings[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXShape::getPropertyStates(
    const uno::Sequence< rtl::OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt*   pFmt = GetFrmFmt();
    uno::Sequence< beans::PropertyState > aRet(aPropertyNames.getLength());
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
        const rtl::OUString* pNames = aPropertyNames.getConstArray();
        beans::PropertyState* pRet = aRet.getArray();
        uno::Reference< XPropertyState >  xShapePrState;
        for(sal_Int32 nProperty = 0; nProperty < aPropertyNames.getLength(); nProperty++)
        {
            const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( pNames[nProperty] );
            if(pEntry)
            {
                if(RES_OPAQUE == pEntry->nWID)
                    pRet[nProperty] = bFormControl ?
                        beans::PropertyState_DEFAULT_VALUE : beans::PropertyState_DIRECT_VALUE;
                else if(FN_ANCHOR_POSITION == pEntry->nWID)
                    pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                else if(FN_TEXT_RANGE == pEntry->nWID)
                    pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                else if(bGroupMember)
                    pRet[nProperty] = beans::PropertyState_DEFAULT_VALUE;
                else if(pFmt)
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    SfxItemState eItemState = rSet.GetItemState(pEntry->nWID, sal_False);

                    if(SFX_ITEM_SET == eItemState)
                        pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                    else if(SFX_ITEM_DEFAULT == eItemState)
                        pRet[nProperty] = beans::PropertyState_DEFAULT_VALUE;
                    else
                        pRet[nProperty] = beans::PropertyState_AMBIGUOUS_VALUE;
                }
                else
                {
                    SfxPoolItem* pItem = 0;
                    switch(pEntry->nWID)
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
                        // #i28701#
                        case RES_WRAP_INFLUENCE_ON_OBJPOS:
                        {
                            pItem = pImpl->GetWrapInfluenceOnObjPos();
                        }
                        break;
                    }
                    if(pItem)
                        pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                    else
                        pRet[nProperty] = beans::PropertyState_DEFAULT_VALUE;
                }
            }
            else
            {
                if(!xShapePrState.is())
                {
                    const uno::Type& rPStateType = ::getCppuType((uno::Reference< XPropertyState >*)0);
                    uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
                    if(aPState.getValueType() != rPStateType || !aPState.getValue())
                        throw uno::RuntimeException();
                    xShapePrState = *(uno::Reference< XPropertyState >*)aPState.getValue();
                }
                pRet[nProperty] = xShapePrState->getPropertyState(pNames[nProperty]);
            }
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXShape::setPropertyToDefault( const rtl::OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw uno::RuntimeException( rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            if(pFmt)
            {
                const SfxItemSet& rSet = pFmt->GetAttrSet();
                SfxItemSet aSet(pFmt->GetDoc()->GetAttrPool(), pEntry->nWID, pEntry->nWID);
                aSet.SetParent(&rSet);
                aSet.ClearItem(pEntry->nWID);
                pFmt->GetDoc()->SetAttr(aSet, *pFmt);
            }
            else
            {
                switch(pEntry->nWID)
                {
                    case RES_ANCHOR:        pImpl->RemoveAnchor();  break;
                    case RES_HORI_ORIENT:   pImpl->RemoveHOrient(); break;
                    case RES_VERT_ORIENT:   pImpl->RemoveVOrient(); break;
                    case  RES_LR_SPACE:     pImpl->RemoveLRSpace(); break;
                    case  RES_UL_SPACE:     pImpl->RemoveULSpace(); break;
                    case  RES_SURROUND:     pImpl->RemoveSurround();break;
                    case RES_OPAQUE :       pImpl->SetOpaque(sal_False);  break;
                    case FN_TEXT_RANGE :
                    break;
                    // #i26791#
                    case RES_FOLLOW_TEXT_FLOW:
                    {
                        pImpl->RemoveFollowTextFlow();
                    }
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pImpl->RemoveWrapInfluenceOnObjPos();
                    }
                    break;
                }
            }
        }
        else
        {
            const uno::Type& rPStateType = ::getCppuType((uno::Reference< XPropertyState >*)0);
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            uno::Reference< XPropertyState > xShapePrState = *(uno::Reference< XPropertyState >*)aPState.getValue();
            xShapePrState->setPropertyToDefault( rPropertyName );
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXShape::getPropertyDefault( const rtl::OUString& rPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt*   pFmt = GetFrmFmt();
    uno::Any aRet;
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if(pEntry->nWID < RES_FRMATR_END && pFmt)
            {
                const SfxPoolItem& rDefItem =
                    pFmt->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
                rDefItem.QueryValue(aRet, pEntry->nMemberId);
            }
            else
                throw uno::RuntimeException();
        }
        else
        {
            const uno::Type& rPStateType = ::getCppuType((uno::Reference< XPropertyState >*)0);
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            uno::Reference< XPropertyState > xShapePrState = *(uno::Reference< XPropertyState >*)aPState.getValue();
            xShapePrState->getPropertyDefault( rPropertyName );
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXShape::addPropertyChangeListener(
    const rtl::OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no shape aggregate" ) ), *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( beans::XPropertySet::static_type() ) >>= xShapeProps )
        xShapeProps->addPropertyChangeListener( _propertyName, _listener );
}

void SwXShape::removePropertyChangeListener(
    const rtl::OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no shape aggregate" ) ), *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( beans::XPropertySet::static_type() ) >>= xShapeProps )
        xShapeProps->removePropertyChangeListener( _propertyName, _listener );
}

void SwXShape::addVetoableChangeListener(
    const rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/ )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXShape::removeVetoableChangeListener(
    const rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXShape::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

void SwXShape::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;

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
        SwXParagraph* pParagraph = 0;

        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pText   = reinterpret_cast< SwXText * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXText::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
        pPortion = reinterpret_cast< SwXTextPortion * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextPortion::getUnoTunnelId()) ));
        pParagraph = reinterpret_cast< SwXParagraph * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXParagraph::getUnoTunnelId( ) ) ) );

        if (pRange)
            pDoc = pRange->GetDoc();
        else if (!pDoc && pText)
            pDoc = pText->GetDoc();
        else if (!pDoc && pCursor)
            pDoc = pCursor->GetDoc();
        else if ( !pDoc && pPortion && pPortion->GetCursor() )
        {
            pDoc = pPortion->GetCursor()->GetDoc();
        }
        else if ( !pDoc && pParagraph && pParagraph->GetTxtNode( ) )
        {
            pDoc = const_cast<SwDoc*>(pParagraph->GetTxtNode()->GetDoc());
        }

    }

    if(!pDoc)
        throw uno::RuntimeException();
    SwDocShell *pDocSh = pDoc->GetDocShell();
    if (pDocSh)
    {
        uno::Reference< frame::XModel > xModel;
        xModel = pDocSh->GetModel();
        uno::Reference< drawing::XDrawPageSupplier > xDPS(xModel, uno::UNO_QUERY);
        if (xDPS.is())
        {
            uno::Reference< drawing::XDrawPage > xDP( xDPS->getDrawPage() );
            if (xDP.is())
            {
                uno::Any aPos;
                aPos <<= xTextRange;
                setPropertyValue(S2U("TextRange"), aPos);
                uno::Reference< drawing::XShape > xTemp( (cppu::OWeakObject*) this, uno::UNO_QUERY );
                xDP->add( xTemp );
            }
        }
    }
}

uno::Reference< text::XTextRange >  SwXShape::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if ((rAnchor.GetAnchorId() != FLY_AT_PAGE) ||
            (rAnchor.GetCntntAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
            aRef = SwXTextRange::CreateXTextRange(*pFmt->GetDoc(), rPos, 0);
        }
    }
    else
        aRef = pImpl->GetTextRange();
    return aRef;
}

void SwXShape::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        // determine correct <SdrObject>
        SdrObject* pObj = GetSvxShape()->GetSdrObject();
        // safety assertion:
        // <pObj> must be the same as <pFmt->FindSdrObject()>, if <pObj> isn't
        // a 'virtual' drawing object.
        // correct assertion and refine it for safety reason.
        OSL_ENSURE( !pObj ||
                pObj->ISA(SwDrawVirtObj) ||
                pObj->GetUpGroup() ||
                pObj == pFmt->FindSdrObject(),
                "<SwXShape::dispose(..) - different 'master' drawing objects!!" );
        // perform delete of draw frame format *not*
        // for 'virtual' drawing objects.
        // no delete of draw format for members
        // of a group
        if ( pObj &&
             !pObj->ISA(SwDrawVirtObj) &&
             !pObj->GetUpGroup() &&
             pObj->IsInserted() )
        {
            if (pFmt->GetAnchor().GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                const xub_StrLen nIdx = rPos.nContent.GetIndex();
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx );
            }
            else
                pFmt->GetDoc()->DelLayoutFmt( pFmt );
        }
    }
    if(xShapeAgg.is())
    {
        uno::Any aAgg(xShapeAgg->queryAggregation( ::getCppuType((uno::Reference<XComponent>*)0)));
        uno::Reference<XComponent> xComp;
        aAgg >>= xComp;
        if(xComp.is())
            xComp->dispose();
    }
}

void SwXShape::addEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
                    throw( uno::RuntimeException )
{
    uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
         pSvxShape->addEventListener(aListener);
}

void SwXShape::removeEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
                    throw( uno::RuntimeException )
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
        pSvxShape->removeEventListener(aListener);
}

rtl::OUString SwXShape::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXShape");
}

sal_Bool SwXShape::supportsService(const rtl::OUString& rServiceName) throw( uno::RuntimeException )
{
    sal_Bool bRet = sal_False;
    if(COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.drawing.Shape"))
        bRet = sal_True;
    else if(xShapeAgg.is())
    {
        uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pSvxShape = GetSvxShape();
        bRet = pSvxShape->supportsService(rServiceName);
    }
    return bRet;
}

uno::Sequence< rtl::OUString > SwXShape::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > aSeq;
    if(xShapeAgg.is())
    {
        uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
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

SvxShape*   SwXShape::GetSvxShape()
{
    SvxShape* pSvxShape = 0;
    if(xShapeAgg.is())
    {
        uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        if(xShapeTunnel.is())
            pSvxShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));
    }
    return pSvxShape;
}

// #i31698#
// implementation of virtual methods from drawing::XShape
awt::Point SAL_CALL SwXShape::getPosition() throw ( uno::RuntimeException )
{
    awt::Point aPos( _GetAttrPosition() );

    // handle group members
    SvxShape* pSvxShape = GetSvxShape();
    if ( pSvxShape )
    {
        SdrObject* pTopGroupObj = _GetTopGroupObj( pSvxShape );
        if ( pTopGroupObj )
        {
            // #i34750# - get attribute position of top group
            // shape and add offset between top group object and group member
            uno::Reference< drawing::XShape > xGroupShape =
                    uno::Reference< drawing::XShape >( pTopGroupObj->getUnoShape(),
                                                       uno::UNO_QUERY );
            aPos = xGroupShape->getPosition();
            // add offset between top group object and group member
            // to the determined attribute position
            // #i34750#:
            // consider the layout direction
            const Rectangle aMemberObjRect = GetSvxShape()->GetSdrObject()->GetSnapRect();
            const Rectangle aGroupObjRect = pTopGroupObj->GetSnapRect();
            // #i53320# - relative position of group member and
            // top group object is always given in horizontal left-to-right layout.
            awt::Point aOffset( 0, 0 );
                {
                    aOffset.X = ( aMemberObjRect.Left() - aGroupObjRect.Left() );
                    aOffset.Y = ( aMemberObjRect.Top() - aGroupObjRect.Top() );
                }
            aOffset.X = TWIP_TO_MM100(aOffset.X);
            aOffset.Y = TWIP_TO_MM100(aOffset.Y);
            aPos.X += aOffset.X;
            aPos.Y += aOffset.Y;
        }
    }

    return aPos;
}

void SAL_CALL SwXShape::setPosition( const awt::Point& aPosition )
                                                throw ( uno::RuntimeException )
{
    SdrObject* pTopGroupObj = _GetTopGroupObj();
    if ( !pTopGroupObj )
    {
        // #i37877# - no adjustment of position attributes,
        // if the position also has to be applied at the drawing object and
        // a contact object is already registered at the drawing object.
        bool bApplyPosAtDrawObj(false);
        bool bNoAdjustOfPosProp(false);
        // #i35798# - apply position also to drawing object,
        // if drawing object has no anchor position set.
        if ( mxShape.is() )
        {
            SvxShape* pSvxShape = GetSvxShape();
            if ( pSvxShape )
            {
                const SdrObject* pObj = pSvxShape->GetSdrObject();
                if ( pObj &&
                     pObj->GetAnchorPos().X() == 0 &&
                     pObj->GetAnchorPos().Y() == 0 )
                {
                    bApplyPosAtDrawObj = true;
                    if ( pObj->GetUserCall() &&
                         pObj->GetUserCall()->ISA(SwDrawContact) )
                    {
                        bNoAdjustOfPosProp = true;
                    }
                }
            }
        }
        // shape isn't a group member. Thus, set positioning attributes
        if ( !bNoAdjustOfPosProp )
        {
            _AdjustPositionProperties( aPosition );
        }
        if ( bApplyPosAtDrawObj )
        {
            mxShape->setPosition( aPosition );
        }
    }
    else if ( mxShape.is() )
    {
        // shape is a member of a group. Thus, set its position.
        awt::Point aNewPos( aPosition );
        // The given position is given in the according layout direction. Thus,
        // it has to be converted to a position in horizontal left-to-right
        // layout.
        // convert given absolute attribute position in layout direction into
        // position in horizontal left-to-right layout.
        {
            aNewPos = _ConvertPositionToHoriL2R( aNewPos, getSize() );
        }
        // Convert given absolute position in horizontal left-to-right
        // layout into relative position in horizontal left-to-right layout.
        uno::Reference< drawing::XShape > xGroupShape =
                uno::Reference< drawing::XShape >( pTopGroupObj->getUnoShape(),
                                                   uno::UNO_QUERY );
        {
            // #i34750#
            // use method <xGroupShape->getPosition()> to get the correct
            // position of the top group object.
            awt::Point aAttrPosInHoriL2R(
                    _ConvertPositionToHoriL2R( xGroupShape->getPosition(),
                                               xGroupShape->getSize() ) );
            aNewPos.X -= aAttrPosInHoriL2R.X;
            aNewPos.Y -= aAttrPosInHoriL2R.Y;
        }
        // convert relative position in horizontal left-to-right layout into
        // absolute position in horizontal left-to-right layout
        {
            // #i34750#
            // use method <SvxShape->getPosition()> to get the correct
            // 'Drawing layer' position of the top group shape.
            uno::Reference< lang::XUnoTunnel > xGrpShapeTunnel(
                                                    pTopGroupObj->getUnoShape(),
                                                    uno::UNO_QUERY );
            SvxShape* pSvxGroupShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xGrpShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));
            const awt::Point aGroupPos = pSvxGroupShape->getPosition();
            aNewPos.X += aGroupPos.X;
            aNewPos.Y += aGroupPos.Y;
        }
        // set position
        mxShape->setPosition( aNewPos );
    }
}

awt::Size SAL_CALL SwXShape::getSize() throw ( uno::RuntimeException )
{
    awt::Size aSize;
    if ( mxShape.is() )
    {
        aSize = mxShape->getSize();
    }
    return aSize;
}

void SAL_CALL SwXShape::setSize( const awt::Size& aSize )
                                        throw ( beans::PropertyVetoException,
                                                uno::RuntimeException )
{
    if ( mxShape.is() )
    {
        mxShape->setSize( aSize );
    }
}
// #i31698#
// implementation of virtual methods from drawing::XShapeDescriptor
::rtl::OUString SAL_CALL SwXShape::getShapeType() throw ( uno::RuntimeException )
{
    ::rtl::OUString aType;
    if ( mxShape.is() )
    {
        aType = mxShape->getShapeType();
    }
    return aType;
}
/** method to determine top group object
    #i31698#
*/
SdrObject* SwXShape::_GetTopGroupObj( SvxShape* _pSvxShape )
{
    SdrObject* pTopGroupObj( 0L );

    SvxShape* pSvxShape = _pSvxShape ? _pSvxShape : GetSvxShape();
    if ( pSvxShape )
    {
        SdrObject* pSdrObj = pSvxShape->GetSdrObject();
        if ( pSdrObj && pSdrObj->GetUpGroup() )
        {
            pTopGroupObj = pSdrObj->GetUpGroup();
            while ( pTopGroupObj->GetUpGroup() )
            {
                pTopGroupObj = pTopGroupObj->GetUpGroup();
            }
        }
    }

    return pTopGroupObj;
}

/** method to determine position according to the positioning attributes
    #i31698#
*/
awt::Point SwXShape::_GetAttrPosition()
{
    awt::Point aAttrPos;

    uno::Any aHoriPos( getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")) ) );
    aHoriPos >>= aAttrPos.X;
    uno::Any aVertPos( getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")) ) );
    aVertPos >>= aAttrPos.Y;
    // #i35798# - fallback, if attribute position is (0,0)
    // and no anchor position is applied to the drawing object
    SvxShape* pSvxShape = GetSvxShape();
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        if ( pObj &&
             pObj->GetAnchorPos().X() == 0 &&
             pObj->GetAnchorPos().Y() == 0 &&
             aAttrPos.X == 0 && aAttrPos.Y == 0 )
        {
            const Rectangle aObjRect = pObj->GetSnapRect();
            aAttrPos.X = TWIP_TO_MM100(aObjRect.Left());
            aAttrPos.Y = TWIP_TO_MM100(aObjRect.Top());
        }
    }
    // #i35007# - If drawing object is anchored as-character,
    // it's x-position isn't sensible. Thus, return the x-position as zero in this case.
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        rtl::OUString sAnchorType( RTL_CONSTASCII_USTRINGPARAM( "AnchorType" ) );
        uno::Any aAny = getPropertyValue( sAnchorType );
        aAny >>= eTextAnchorType;
    }
    if ( eTextAnchorType == text::TextContentAnchorType_AS_CHARACTER )
    {
        aAttrPos.X = 0;
    }

    return aAttrPos;
}

/** method to convert the position (translation) of the drawing object to
    the layout direction horizontal left-to-right.
    #i31698#
*/
awt::Point SwXShape::_ConvertPositionToHoriL2R( const awt::Point _aObjPos,
                                                const awt::Size _aObjSize )
{
    awt::Point aObjPosInHoriL2R( _aObjPos );

    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if ( pFrmFmt )
    {
        SwFrmFmt::tLayoutDir eLayoutDir = pFrmFmt->GetLayoutDir();
        switch ( eLayoutDir )
        {
            case SwFrmFmt::HORI_L2R:
            {
                // nothing to do
            }
            break;
            case SwFrmFmt::HORI_R2L:
            {
                aObjPosInHoriL2R.X = -_aObjPos.X - _aObjSize.Width;
            }
            break;
            case SwFrmFmt::VERT_R2L:
            {
                aObjPosInHoriL2R.X = -_aObjPos.Y - _aObjSize.Width;
                aObjPosInHoriL2R.Y = _aObjPos.X;
            }
            break;
            default:
            {
                OSL_FAIL( "<SwXShape::_ConvertPositionToHoriL2R(..)> - unsupported layout direction" );
            }
        }
    }

    return aObjPosInHoriL2R;
}

/** method to convert the transformation of the drawing object to the layout
    direction, the drawing object is in
    #i31698#
*/
drawing::HomogenMatrix3 SwXShape::_ConvertTransformationToLayoutDir(
                                    drawing::HomogenMatrix3 _aMatrixInHoriL2R )
{
    drawing::HomogenMatrix3 aMatrix( _aMatrixInHoriL2R );

    // #i44334#, #i44681# - direct manipulation of the
    // tranformation structure isn't valid, if it contains rotation.
    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::_ConvertTransformationToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::_ConvertTransformationToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            // determine difference between these positions according to the
            // Writer coordinate system
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            // apply translation difference to transformation matrix.
            if ( aTranslateDiff.X != 0 || aTranslateDiff.Y != 0 )
            {
                // #i73079# - use correct matrix type
                ::basegfx::B2DHomMatrix aTempMatrix;

                aTempMatrix.set(0, 0, aMatrix.Line1.Column1 );
                aTempMatrix.set(0, 1, aMatrix.Line1.Column2 );
                aTempMatrix.set(0, 2, aMatrix.Line1.Column3 );
                aTempMatrix.set(1, 0, aMatrix.Line2.Column1 );
                aTempMatrix.set(1, 1, aMatrix.Line2.Column2 );
                aTempMatrix.set(1, 2, aMatrix.Line2.Column3 );
                aTempMatrix.set(2, 0, aMatrix.Line3.Column1 );
                aTempMatrix.set(2, 1, aMatrix.Line3.Column2 );
                aTempMatrix.set(2, 2, aMatrix.Line3.Column3 );
                // #i73079#
                aTempMatrix.translate( aTranslateDiff.X, aTranslateDiff.Y );
                aMatrix.Line1.Column1 = aTempMatrix.get(0, 0);
                aMatrix.Line1.Column2 = aTempMatrix.get(0, 1);
                aMatrix.Line1.Column3 = aTempMatrix.get(0, 2);
                aMatrix.Line2.Column1 = aTempMatrix.get(1, 0);
                aMatrix.Line2.Column2 = aTempMatrix.get(1, 1);
                aMatrix.Line2.Column3 = aTempMatrix.get(1, 2);
                aMatrix.Line3.Column1 = aTempMatrix.get(2, 0);
                aMatrix.Line3.Column2 = aTempMatrix.get(2, 1);
                aMatrix.Line3.Column3 = aTempMatrix.get(2, 2);
            }
        }
    }

    return aMatrix;
}

/** method to adjust the positioning properties
    #i31698#
*/
void SwXShape::_AdjustPositionProperties( const awt::Point _aPosition )
{
    // handle x-position
    // #i35007# - no handling of x-position, if drawing
    // object is anchored as-character, because it doesn't make sense.
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        rtl::OUString sAnchorType( RTL_CONSTASCII_USTRINGPARAM( "AnchorType" ) );
        uno::Any aAny = getPropertyValue( sAnchorType );
        aAny >>= eTextAnchorType;
    }
    if ( eTextAnchorType != text::TextContentAnchorType_AS_CHARACTER )
    {
        // determine current x-postion
        rtl::OUString aHoriPosPropStr( RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition") );
        uno::Any aHoriPos( getPropertyValue( aHoriPosPropStr ) );
        sal_Int32 dCurrX = 0;
        aHoriPos >>= dCurrX;
        // change x-position attribute, if needed
        if ( dCurrX != _aPosition.X )
        {
            // adjust x-position orientation to text::HoriOrientation::NONE, if needed
            // Note: has to be done before setting x-position attribute
            rtl::OUString aHoriOrientPropStr( RTL_CONSTASCII_USTRINGPARAM("HoriOrient") );
            uno::Any aHoriOrient( getPropertyValue( aHoriOrientPropStr ) );
            sal_Int16 eHoriOrient;
            if (aHoriOrient >>= eHoriOrient) // may be void
            {
                if ( eHoriOrient != text::HoriOrientation::NONE )
                {
                    eHoriOrient = text::HoriOrientation::NONE;
                    aHoriOrient <<= eHoriOrient;
                    setPropertyValue( aHoriOrientPropStr, aHoriOrient );
                }
            }
            // set x-position attribute
            aHoriPos <<= _aPosition.X;
            setPropertyValue( aHoriPosPropStr, aHoriPos );
        }
    }

    // handle y-position
    {
        // determine current y-postion
        rtl::OUString aVertPosPropStr( RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition") );
        uno::Any aVertPos( getPropertyValue( aVertPosPropStr ) );
        sal_Int32 dCurrY = 0;
        aVertPos >>= dCurrY;
        // change y-position attribute, if needed
        if ( dCurrY != _aPosition.Y )
        {
            // adjust y-position orientation to text::VertOrientation::NONE, if needed
            // Note: has to be done before setting y-position attribute
            rtl::OUString aVertOrientPropStr( RTL_CONSTASCII_USTRINGPARAM("VertOrient") );
            uno::Any aVertOrient( getPropertyValue( aVertOrientPropStr ) );
            sal_Int16 eVertOrient;
            if (aVertOrient >>= eVertOrient) // may be void
            {
                if ( eVertOrient != text::VertOrientation::NONE )
                {
                    eVertOrient = text::VertOrientation::NONE;
                    aVertOrient <<= eVertOrient;
                    setPropertyValue( aVertOrientPropStr, aVertOrient );
                }
            }
            // set y-position attribute
            aVertPos <<= _aPosition.Y;
            setPropertyValue( aVertPosPropStr, aVertPos );
        }
    }
}

/** method to convert start or end position of the drawing object to the
    Writer specific position, which is the attribute position in layout direction
    #i59051#
*/
::com::sun::star::awt::Point SwXShape::_ConvertStartOrEndPosToLayoutDir(
                            const ::com::sun::star::awt::Point& aStartOrEndPos )
{
    awt::Point aConvertedPos( aStartOrEndPos );

    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::_ConvertStartOrEndPosToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::_ConvertStartOrEndPosToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            // determine difference between these positions according to the
            // Writer coordinate system
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            // apply translation difference to transformation matrix.
            if ( aTranslateDiff.X != 0 || aTranslateDiff.Y != 0 )
            {
                aConvertedPos.X = aConvertedPos.X + aTranslateDiff.X;
                aConvertedPos.Y = aConvertedPos.Y + aTranslateDiff.Y;
            }
        }
    }

    return aConvertedPos;
}

::com::sun::star::drawing::PolyPolygonBezierCoords SwXShape::_ConvertPolyPolygonBezierToLayoutDir(
                    const ::com::sun::star::drawing::PolyPolygonBezierCoords& aPath )
{
    drawing::PolyPolygonBezierCoords aConvertedPath( aPath );

    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::_ConvertStartOrEndPosToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::_ConvertStartOrEndPosToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            // determine difference between these positions according to the
            // Writer coordinate system
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            // apply translation difference to PolyPolygonBezier.
            if ( aTranslateDiff.X != 0 || aTranslateDiff.Y != 0 )
            {
                const basegfx::B2DHomMatrix aMatrix(basegfx::tools::createTranslateB2DHomMatrix(
                    aTranslateDiff.X, aTranslateDiff.Y));

                const sal_Int32 nOuterSequenceCount(aConvertedPath.Coordinates.getLength());
                drawing::PointSequence* pInnerSequence = aConvertedPath.Coordinates.getArray();
                for(sal_Int32 a(0); a < nOuterSequenceCount; a++)
                {
                    const sal_Int32 nInnerSequenceCount(pInnerSequence->getLength());
                    awt::Point* pArray = pInnerSequence->getArray();

                    for(sal_Int32 b(0); b < nInnerSequenceCount; b++)
                    {
                        basegfx::B2DPoint aNewCoordinatePair(pArray->X, pArray->Y);
                        aNewCoordinatePair *= aMatrix;
                        pArray->X = basegfx::fround(aNewCoordinatePair.getX());
                        pArray->Y = basegfx::fround(aNewCoordinatePair.getY());
                        pArray++;
                    }
                }
            }
        }
    }

    return aConvertedPath;
}

SwXGroupShape::SwXGroupShape(uno::Reference< XInterface > & xShape) :
        SwXShape(xShape)
{
#if OSL_DEBUG_LEVEL > 0
    uno::Reference<XShapes> xShapes(xShapeAgg, uno::UNO_QUERY);
    OSL_ENSURE(xShapes.is(), "no SvxShape found or shape is not a group shape");
#endif
}

SwXGroupShape::~SwXGroupShape()
{
}

uno::Any SwXGroupShape::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet;
    if(rType == ::getCppuType((uno::Reference<XShapes>*)0))
        aRet <<= uno::Reference<XShapes>(this);
    else
        aRet = SwXShape::queryInterface(rType);
    return aRet;
}

void SwXGroupShape::acquire(  ) throw()
{
    SwXShape::acquire();
}

void SwXGroupShape::release(  ) throw()
{
    SwXShape::release();
}

void SwXGroupShape::add( const uno::Reference< XShape >& xShape ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SvxShape* pSvxShape = GetSvxShape();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pSvxShape && pFmt)
    {
        uno::Reference<XShapes> xShapes;
        if( xShapeAgg.is() )
        {
            const uno::Type& rType = ::getCppuType((uno::Reference<XShapes>*)0 );
            uno::Any aAgg = xShapeAgg->queryAggregation( rType );
            aAgg >>= xShapes;
        }
        if(xShapes.is())
            xShapes->add(xShape);
        else
            throw uno::RuntimeException();

        uno::Reference<lang::XUnoTunnel> xTunnel(xShape, uno::UNO_QUERY);
        SwXShape* pSwShape = 0;
        if(xShape.is())
            pSwShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXShape::getUnoTunnelId()) ));
        if(pSwShape && pSwShape->m_bDescriptor)
        {
            SvxShape* pAddShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SvxShape::getUnoTunnelId()) ));
            if(pAddShape)
            {
                SdrObject* pObj = pAddShape->GetSdrObject();
                if(pObj)
                {
                    SwDoc* pDoc = pFmt->GetDoc();
                    // set layer of new drawing
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
            SwFrmFmt* pShapeFmt = ::FindFrmFmt( pSvxShape->GetSdrObject() );
            if(pShapeFmt)
                pFmt->Add(pSwShape);
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXGroupShape::remove( const uno::Reference< XShape >& xShape ) throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<XShapes> xShapes;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((uno::Reference<XShapes>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xShapes;
    }
    if(!xShapes.is())
        throw uno::RuntimeException();
    xShapes->remove(xShape);
}

sal_Int32 SwXGroupShape::getCount(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((uno::Reference<XIndexAccess>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getCount();
}

uno::Any SwXGroupShape::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
               uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((uno::Reference<XIndexAccess>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getByIndex(nIndex);
}

uno::Type SwXGroupShape::getElementType(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((uno::Reference<XIndexAccess>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getElementType();
}

sal_Bool SwXGroupShape::hasElements(  ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = ::getCppuType((uno::Reference<XIndexAccess>*)0 );
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
