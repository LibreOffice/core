/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>

#include <map>

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
#include <comphelper/makesequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <switerator.hxx>

using namespace ::com::sun::star;

typedef std::map<OUString, uno::Sequence< sal_Int8 > *> SwShapeImplementationIdMap;

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
    
    SwFmtFollowTextFlow* mpFollowTextFlow;
    
    SwFmtWrapInfluenceOnObjPos* pWrapInfluenceOnObjPos;
    
    sal_Int16 mnPositionLayoutDir;

public:
    bool    bInitializedPropertyNotifier;

public:
    SwShapeDescriptor_Impl() :
     
     
     pHOrient( 0L ),
     pVOrient( 0L ),
     pAnchor(0),
     pSurround(0),
     pULSpace(0),
     pLRSpace(0),
     bOpaque(sal_False),
     
     mpFollowTextFlow( new SwFmtFollowTextFlow( sal_False ) ),
     
     pWrapInfluenceOnObjPos( new SwFmtWrapInfluenceOnObjPos(
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT ) ),
     
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
        
        delete mpFollowTextFlow;
        
        delete pWrapInfluenceOnObjPos;
    }
    SwFmtAnchor*    GetAnchor(bool bCreate = false)
        {
            if(bCreate && !pAnchor)
            {
                pAnchor = new SwFmtAnchor(FLY_AS_CHAR);
            }
            return pAnchor;
        }
    SwFmtHoriOrient* GetHOrient(bool bCreate = false)
        {
            if (bCreate && !pHOrient)
            {
                
                pHOrient = new SwFmtHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
            }
            return pHOrient;
        }
    SwFmtVertOrient* GetVOrient(bool bCreate = false)
        {
            if(bCreate && !pVOrient)
            {
                
                pVOrient = new SwFmtVertOrient( 0, text::VertOrientation::NONE, text::RelOrientation::FRAME );
            }
            return pVOrient;
        }

    SwFmtSurround*  GetSurround(bool bCreate = false)
        {
            if(bCreate && !pSurround)
                pSurround = new SwFmtSurround();
            return pSurround;
        }
    SvxLRSpaceItem* GetLRSpace(bool bCreate = false)
        {
            if(bCreate && !pLRSpace)
                pLRSpace = new SvxLRSpaceItem(RES_LR_SPACE);
            return pLRSpace;
        }
    SvxULSpaceItem* GetULSpace(bool bCreate = false)
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

    
    SwFmtFollowTextFlow* GetFollowTextFlow( bool _bCreate = false )
    {
        if ( _bCreate && !mpFollowTextFlow )
            mpFollowTextFlow = new SwFmtFollowTextFlow( sal_False );
        return mpFollowTextFlow;
    }
    void RemoveFollowTextFlow()
    {
        DELETEZ(mpFollowTextFlow);
    }

    
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

    
    inline SwFmtWrapInfluenceOnObjPos* GetWrapInfluenceOnObjPos(
                                        const bool _bCreate = false )
    {
        if ( _bCreate && !pWrapInfluenceOnObjPos )
        {
            pWrapInfluenceOnObjPos = new SwFmtWrapInfluenceOnObjPos(
                        
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
        
        
        {
            xRet = SvxFmDrawPage::_CreateShape( pObj );
        }
        uno::Reference< XUnoTunnel > xShapeTunnel(xRet, uno::UNO_QUERY);
        
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

            
            virtual sal_Bool SAL_CALL hasMoreElements(void) throw(uno::RuntimeException);
            virtual uno::Any SAL_CALL nextElement(void) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

            
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
    return OUString("SwXShapeEnumeration");
}

sal_Bool SwXShapesEnumeration::supportsService(const OUString& ServiceName) throw(uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SwXShapesEnumeration::getSupportedServiceNames(void) throw(uno::RuntimeException)
{
    return ::comphelper::makeSequence(OUString("com.sun.star.container.XEnumeration"));
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

OUString SwXDrawPage::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXDrawPage");
}

sal_Bool SwXDrawPage::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXDrawPage::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.drawing.GenericDrawPage";
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
            aExcept.Message = "object already inserted";
        else
            aExcept.Message = "illegal object";
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
    
    awt::Point aMM100Pos(pSvxShape->getPosition());

    
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
    SwFmtAnchor aAnchor( FLY_AS_CHAR );
    sal_Bool bOpaque = sal_False;
    if( pDesc )
    {
        if(pDesc->GetSurround())
            aSet.Put( *pDesc->GetSurround());
        
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

        
        if ( !pDesc->GetHOrient() )
        {
            SwFmtHoriOrient* pHori = pDesc->GetHOrient( true );
            SwTwips nHoriPos = MM100_TO_TWIP(aMM100Pos.X);
            pHori->SetPos( nHoriPos );
        }
        {
            if(pDesc->GetHOrient()->GetHoriOrient() == text::HoriOrientation::NONE)
                aMM100Pos.X = TWIP_TO_MM100(pDesc->GetHOrient()->GetPos());
            aSet.Put( *pDesc->GetHOrient() );
        }
        
        if ( !pDesc->GetVOrient() )
        {
            SwFmtVertOrient* pVert = pDesc->GetVOrient( true );
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

        
        if ( pDesc->GetFollowTextFlow() )
        {
            aSet.Put( *pDesc->GetFollowTextFlow() );
        }

        
        if ( pDesc->GetWrapInfluenceOnObjPos() )
        {
            aSet.Put( *pDesc->GetWrapInfluenceOnObjPos() );
        }
    }

    pSvxShape->setPosition(aMM100Pos);
    SdrObject* pObj = pSvxShape->GetSdrObject();
    
    
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
        pDoc->GetCurrentLayout()->GetCrsrOfst( pPam->GetPoint(), aTmp, &aState );
        aAnchor.SetAnchor( pPam->GetPoint() );

        
        
    }
    else
    {
        aAnchor.SetType(FLY_AT_PAGE);

        
        
    }
    aSet.Put(aAnchor);
    SwPaM* pTemp = pInternalPam;
    if ( !pTemp )
        pTemp = pPam;
    UnoActionContext aAction(pDoc);
    pDoc->InsertDrawObj( *pTemp, *pObj, aSet );
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
        if(pPage)
        {
            
            const SdrMarkList& rMarkList = pPage->PreGroup(xShapes);
            if ( rMarkList.GetMarkCount() > 1 )
            {
                bool bFlyInCnt = false;
                for ( sal_uInt16 i = 0; !bFlyInCnt && i < rMarkList.GetMarkCount(); ++i )
                {
                    const SdrObject *pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if (FLY_AS_CHAR == ::FindFrmFmt(const_cast<SdrObject*>(
                                            pObj))->GetAnchor().GetAnchorId())
                    {
                        bFlyInCnt = true;
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
        if(pPage)
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
        
        SdrModel* pModel = pDoc->GetOrCreateDrawModel();
        SdrPage* pPage = pModel->GetPage( 0 );

        {
            
            
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
    if(xShape.is())  
    {
        const uno::Type& rAggType = ::getCppuType((const uno::Reference< uno::XAggregation >*)0);
        
        {
            uno::Any aAgg = xShape->queryInterface(rAggType);
            if(aAgg.getValueType() == rAggType)
                xShapeAgg = *(uno::Reference< uno::XAggregation >*)aAgg.getValue();
            
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
    
    if( !pImplementationId && xShapeAgg.is())
    {
        uno::Reference< XShape > xAggShape;
        xShapeAgg->queryAggregation( ::getCppuType((uno::Reference< XShape >*)0) ) >>= xAggShape;

        if( xAggShape.is() )
        {
            const OUString aShapeType( xAggShape->getShapeType() );
            
            SwShapeImplementationIdMap::iterator aIter( aImplementationIdMap.find(aShapeType ) );
            if( aIter == aImplementationIdMap.end() )
            {
                
                
                
                
                pImplementationId = new uno::Sequence< sal_Int8 >( 16 );
                rtl_createUuid( (sal_uInt8 *) pImplementationId->getArray(), 0, sal_True );
                aImplementationIdMap[ aShapeType ] = pImplementationId;
            }
            else
            {
                
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
            
            const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
            aRet = new SfxExtItemPropertySetInfo( m_pPropertyMapEntries, aPropSeq );
        }
    }
    if(!aRet.is())
        aRet = m_pPropSet->getPropertySetInfo();
    return aRet;
}

void SwXShape::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
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
                throw beans::PropertyVetoException ("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            
            if(pFmt)
            {
                SwAttrSet aSet(pFmt->GetAttrSet());
                SwDoc* pDoc = pFmt->GetDoc();
                if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORFRAME == pEntry->nMemberId)
                {
                    bool bDone = true;
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
                            bDone = true;
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
                
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    SwFmtAnchor aAnchor( static_cast<const SwFmtAnchor&>(aSet.Get( RES_ANCHOR )) );
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
                    {
                        
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
                                
                                
                                
                                
                                const SwPosition *pPos = aAnchor.GetCntntAnchor();
                                SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                                SAL_WARN_IF( !pTxtNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                                const sal_Int32 nIdx = pPos->nContent.GetIndex();
                                SwTxtAttr * const pHnt =
                                    pTxtNode->GetTxtAttrForCharAt(
                                        nIdx, RES_TXTATR_FLYCNT );
                                SAL_WARN_IF( !pHnt || pHnt->Which() != RES_TXTATR_FLYCNT,
                                            "sw.uno", "Missing FlyInCnt-Hint." );
                                SAL_WARN_IF( !pHnt || pHnt->GetFlyCnt().GetFrmFmt() != pFmt,
                                            "sw.uno", "Wrong TxtFlyCnt-Hint." );
                                const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt())
                                    .SetFlyFmt();

                                
                                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx );
                                
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
                
                else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
                {
                    sal_Int16 nPositionLayoutDir = 0;
                    aValue >>= nPositionLayoutDir;
                    pFmt->SetPositionLayoutDir( nPositionLayoutDir );
                }
                else if( pDoc->GetCurrentLayout())
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
                        ( pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT || pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH
                          || pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT_RELATION
                          || pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH_RELATION ) )
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    SAL_WARN_IF(!pSvxShape, "sw.uno", "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        sal_Int16 nPercent(100);
                        aValue >>= nPercent;
                        switch (pEntry->nMemberId)
                        {
                        case MID_FRMSIZE_REL_WIDTH:
                            pObj->SetRelativeWidth( nPercent / 100.0 );
                        break;
                        case MID_FRMSIZE_REL_HEIGHT:
                            pObj->SetRelativeHeight( nPercent / 100.0 );
                        break;
                        case MID_FRMSIZE_REL_WIDTH_RELATION:
                            pObj->SetRelativeWidthRelation(nPercent);
                        break;
                        case MID_FRMSIZE_REL_HEIGHT_RELATION:
                            pObj->SetRelativeHeightRelation(nPercent);
                        break;
                        }
                    }
                }
                else
                {
                    m_pPropSet->setPropertyValue( *pEntry, aValue, aSet );

                    if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                    {
                        bool bSetAttr = true;
                        sal_Int32 eNewAnchor = SWUnoHelper::GetEnumAsInt32( aValue );

                        
                        const SwFmtAnchor& rOldAnchor = pFmt->GetAnchor();
                        RndStdIds eOldAnchorId = rOldAnchor.GetAnchorId();
                        SdrObject* pObj = pFmt->FindSdrObject();
                        SwFrmFmt *pFlyFmt = FindFrmFmt( pObj );
                        pFlyFmt->DelFrms();
                        if( text::TextContentAnchorType_AS_CHARACTER != eNewAnchor &&
                            (FLY_AS_CHAR == eOldAnchorId))
                        {
                            
                            
                            
                            const SwPosition *pPos = rOldAnchor.GetCntntAnchor();
                            SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                            SAL_WARN_IF( !pTxtNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                            const sal_Int32 nIdx = pPos->nContent.GetIndex();
                            SwTxtAttr * const pHnt =
                                pTxtNode->GetTxtAttrForCharAt(
                                    nIdx, RES_TXTATR_FLYCNT );
                            SAL_WARN_IF( !pHnt || pHnt->Which() != RES_TXTATR_FLYCNT,
                                        "sw.uno", "Missing FlyInCnt-Hint." );
                            SAL_WARN_IF( !pHnt || pHnt->GetFlyCnt().GetFrmFmt() != pFlyFmt,
                                        "sw.uno", "Wrong TxtFlyCnt-Hint." );
                            const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt())
                                .SetFlyFmt();

                            
                            pTxtNode->DeleteAttributes(RES_TXTATR_FLYCNT, nIdx);
                        }
                        else if( text::TextContentAnchorType_AT_PAGE != eNewAnchor &&
                                (FLY_AT_PAGE == eOldAnchorId))
                        {
                            SwFmtAnchor aNewAnchor( dynamic_cast< const SwFmtAnchor& >( aSet.Get( RES_ANCHOR ) ) );
                            
                            
                            SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                            if( pDoc->GetCurrentLayout() )
                            {
                                SwCrsrMoveState aState( MV_SETONLYTEXT );
                                Point aTmp( pObj->GetSnapRect().TopLeft() );
                                pDoc->GetCurrentLayout()->GetCrsrOfst( aPam.GetPoint(), aTmp, &aState );
                            }
                            else
                            {
                                
                                aPam.Move( fnMoveBackward, fnGoDoc );
                            }
                            
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
                                
                                aPam.Move( fnMoveBackward, fnGoDoc );
                            }
                            
                            SwTxtNode *pNd = aPam.GetNode()->GetTxtNode();
                            SAL_WARN_IF( !pNd, "sw.uno", "Crsr is not in a TxtNode." );
                            SwFmtFlyCnt aFmt( pFlyFmt );
                            pNd->InsertItem(aFmt,
                                aPam.GetPoint()->nContent.GetIndex(), 0 );
                            aPam.GetPoint()->nContent--; 
                            SwFmtAnchor aNewAnchor(
                                dynamic_cast<const SwFmtAnchor&>(
                                    aSet.Get(RES_ANCHOR)));
                            aNewAnchor.SetAnchor( aPam.GetPoint() );
                            aSet.Put( aNewAnchor );
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
                        pItem = pImpl->GetAnchor(true);
                    break;
                    case RES_HORI_ORIENT:
                        pItem = pImpl->GetHOrient(true);
                    break;
                    case RES_VERT_ORIENT:
                        pItem = pImpl->GetVOrient(true);
                    break;
                    case  RES_LR_SPACE:
                        pItem = pImpl->GetLRSpace(true);
                    break;
                    case  RES_UL_SPACE:
                        pItem = pImpl->GetULSpace(true);
                    break;
                    case  RES_SURROUND:
                        pItem = pImpl->GetSurround(true);
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
                    
                    case RES_FOLLOW_TEXT_FLOW:
                    {
                        pItem = pImpl->GetFollowTextFlow( true );
                    }
                    break;
                    
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = pImpl->GetWrapInfluenceOnObjPos( true );
                    }
                    break;
                    
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
            
            
            
            
            awt::Point aKeepedPosition( 0, 0 );
            if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
            {
                    aKeepedPosition = getPosition();
            }
            if( pFmt && pFmt->GetDoc()->GetCurrentViewShell() )
            {
                UnoActionContext aCtx(pFmt->GetDoc());
                xPrSet->setPropertyValue(rPropertyName, aValue);
            }
            else
                xPrSet->setPropertyValue(rPropertyName, aValue);
            
            if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
            {
                setPosition( aKeepedPosition );
            }
        }
    }
}

uno::Any SwXShape::getPropertyValue(const OUString& rPropertyName)
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
                
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    const SwFmtAnchor aAnchor = pFmt->GetAnchor();
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
                    {
                        
                        
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
                            
                            uno::Any aAny;
                            aRet = aAny;
                        }
                    }
                }
                
                else if ( FN_SHAPE_TRANSFORMATION_IN_HORI_L2R == pEntry->nWID )
                {
                    
                    
                    aRet = _getPropAtAggrObj( OUString("Transformation") );
                }
                else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
                {
                    aRet <<= pFmt->GetPositionLayoutDir();
                }
                
                else if ( FN_SHAPE_STARTPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    
                    
                    aRet = _getPropAtAggrObj( OUString("StartPosition") );
                }
                else if ( FN_SHAPE_ENDPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    
                    
                    aRet = _getPropAtAggrObj( OUString("EndPosition") );
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
                    
                    case RES_FOLLOW_TEXT_FLOW :
                    {
                        pItem = pImpl->GetFollowTextFlow();
                    }
                    break;
                    
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = pImpl->GetWrapInfluenceOnObjPos();
                    }
                    break;
                    
                    case FN_SHAPE_TRANSFORMATION_IN_HORI_L2R:
                    {
                        
                        
                        aRet = _getPropAtAggrObj( OUString("Transformation") );
                    }
                    break;
                    case FN_SHAPE_POSITION_LAYOUT_DIR:
                    {
                        aRet <<= pImpl->GetPositionLayoutDir();
                    }
                    break;
                    
                    case FN_SHAPE_STARTPOSITION_IN_HORI_L2R:
                    {
                        
                        
                        aRet = _getPropAtAggrObj( OUString("StartPosition") );
                    }
                    break;
                    case FN_SHAPE_ENDPOSITION_IN_HORI_L2R:
                    {
                        
                        
                        aRet = _getPropAtAggrObj( OUString("EndPosition") );
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

            
            
            if ( rPropertyName == "Transformation" )
            {
                drawing::HomogenMatrix3 aMatrix;
                aRet >>= aMatrix;
                aRet <<= _ConvertTransformationToLayoutDir( aMatrix );
            }
            
            else if ( rPropertyName == "StartPosition" )
            {
                awt::Point aStartPos;
                aRet >>= aStartPos;
                
                aRet <<= _ConvertStartOrEndPosToLayoutDir( aStartPos );
            }
            else if ( rPropertyName == "EndPosition" )
            {
                awt::Point aEndPos;
                aRet >>= aEndPos;
                
                aRet <<= _ConvertStartOrEndPosToLayoutDir( aEndPos );
            }
            
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

uno::Any SwXShape::_getPropAtAggrObj( const OUString& _rPropertyName )
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

beans::PropertyState SwXShape::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aNames(1);
    OUString* pStrings = aNames.getArray();
    pStrings[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXShape::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt*   pFmt = GetFrmFmt();
    uno::Sequence< beans::PropertyState > aRet(aPropertyNames.getLength());
    if(xShapeAgg.is())
    {
        SvxShape* pSvxShape = GetSvxShape();
        bool bGroupMember = false;
        bool bFormControl = false;
        SdrObject* pObject = pSvxShape ? pSvxShape->GetSdrObject() : NULL;
        if(pObject)
        {
            bGroupMember = pObject->GetUpGroup() != 0;
            bFormControl = pObject->GetObjInventor() == FmFormInventor;
        }
        const OUString* pNames = aPropertyNames.getConstArray();
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
                    SfxItemState eItemState = rSet.GetItemState(pEntry->nWID, false);

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

void SwXShape::setPropertyToDefault( const OUString& rPropertyName )
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
                throw uno::RuntimeException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
                    
                    case RES_FOLLOW_TEXT_FLOW:
                    {
                        pImpl->RemoveFollowTextFlow();
                    }
                    break;
                    
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

uno::Any SwXShape::getPropertyDefault( const OUString& rPropertyName )
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
    const OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->addPropertyChangeListener( _propertyName, _listener );
}

void SwXShape::removePropertyChangeListener(
    const OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->removePropertyChangeListener( _propertyName, _listener );
}

void SwXShape::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/ )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXShape::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
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
                setPropertyValue("TextRange", aPos);
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
        
        SvxShape* pSvxShape = GetSvxShape();
        SdrObject* pObj = pSvxShape ? pSvxShape->GetSdrObject() : NULL;
        
        
        
        
        OSL_ENSURE( !pObj ||
                pObj->ISA(SwDrawVirtObj) ||
                pObj->GetUpGroup() ||
                pObj == pFmt->FindSdrObject(),
                "<SwXShape::dispose(..) - different 'master' drawing objects!!" );
        
        
        
        
        if ( pObj &&
             !pObj->ISA(SwDrawVirtObj) &&
             !pObj->GetUpGroup() &&
             pObj->IsInserted() )
        {
            if (pFmt->GetAnchor().GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                const sal_Int32 nIdx = rPos.nContent.GetIndex();
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

OUString SwXShape::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXShape");
}

sal_Bool SwXShape::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXShape::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq;
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
        aSeq.getArray()[0] = "com.sun.star.drawing.Shape";
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



awt::Point SAL_CALL SwXShape::getPosition() throw ( uno::RuntimeException )
{
    awt::Point aPos( _GetAttrPosition() );

    
    SvxShape* pSvxShape = GetSvxShape();
    if ( pSvxShape )
    {
        SdrObject* pTopGroupObj = _GetTopGroupObj( pSvxShape );
        if ( pTopGroupObj )
        {
            
            
            uno::Reference< drawing::XShape > xGroupShape =
                    uno::Reference< drawing::XShape >( pTopGroupObj->getUnoShape(),
                                                       uno::UNO_QUERY );
            aPos = xGroupShape->getPosition();
            
            
            
            
            const Rectangle aMemberObjRect = GetSvxShape()->GetSdrObject()->GetSnapRect();
            const Rectangle aGroupObjRect = pTopGroupObj->GetSnapRect();
            
            
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
        
        
        
        bool bApplyPosAtDrawObj(false);
        bool bNoAdjustOfPosProp(false);
        
        
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
        
        awt::Point aNewPos( aPosition );
        
        
        
        
        
        {
            aNewPos = _ConvertPositionToHoriL2R( aNewPos, getSize() );
        }
        
        
        uno::Reference< drawing::XShape > xGroupShape =
                uno::Reference< drawing::XShape >( pTopGroupObj->getUnoShape(),
                                                   uno::UNO_QUERY );
        {
            
            
            
            awt::Point aAttrPosInHoriL2R(
                    _ConvertPositionToHoriL2R( xGroupShape->getPosition(),
                                               xGroupShape->getSize() ) );
            aNewPos.X -= aAttrPosInHoriL2R.X;
            aNewPos.Y -= aAttrPosInHoriL2R.Y;
        }
        
        
        {
            
            
            
            uno::Reference< lang::XUnoTunnel > xGrpShapeTunnel(
                                                    pTopGroupObj->getUnoShape(),
                                                    uno::UNO_QUERY );
            SvxShape* pSvxGroupShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xGrpShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));
            const awt::Point aGroupPos = pSvxGroupShape->getPosition();
            aNewPos.X += aGroupPos.X;
            aNewPos.Y += aGroupPos.Y;
        }
        
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


OUString SAL_CALL SwXShape::getShapeType() throw ( uno::RuntimeException )
{
    OUString aType;
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

    uno::Any aHoriPos( getPropertyValue("HoriOrientPosition") );
    aHoriPos >>= aAttrPos.X;
    uno::Any aVertPos( getPropertyValue("VertOrientPosition") );
    aVertPos >>= aAttrPos.Y;
    
    
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
    
    
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        OUString sAnchorType( "AnchorType" );
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
            
            awt::Point aPos( getPosition() );
            
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            
            
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            
            if ( aTranslateDiff.X != 0 || aTranslateDiff.Y != 0 )
            {
                
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
    
    
    
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        OUString sAnchorType( "AnchorType" );
        uno::Any aAny = getPropertyValue( sAnchorType );
        aAny >>= eTextAnchorType;
    }
    if ( eTextAnchorType != text::TextContentAnchorType_AS_CHARACTER )
    {
        
        OUString aHoriPosPropStr("HoriOrientPosition");
        uno::Any aHoriPos( getPropertyValue( aHoriPosPropStr ) );
        sal_Int32 dCurrX = 0;
        aHoriPos >>= dCurrX;
        
        if ( dCurrX != _aPosition.X )
        {
            
            
            OUString aHoriOrientPropStr("HoriOrient");
            uno::Any aHoriOrient( getPropertyValue( aHoriOrientPropStr ) );
            sal_Int16 eHoriOrient;
            if (aHoriOrient >>= eHoriOrient) 
            {
                if ( eHoriOrient != text::HoriOrientation::NONE )
                {
                    eHoriOrient = text::HoriOrientation::NONE;
                    aHoriOrient <<= eHoriOrient;
                    setPropertyValue( aHoriOrientPropStr, aHoriOrient );
                }
            }
            
            aHoriPos <<= _aPosition.X;
            setPropertyValue( aHoriPosPropStr, aHoriPos );
        }
    }

    
    {
        
        OUString aVertPosPropStr("VertOrientPosition");
        uno::Any aVertPos( getPropertyValue( aVertPosPropStr ) );
        sal_Int32 dCurrY = 0;
        aVertPos >>= dCurrY;
        
        if ( dCurrY != _aPosition.Y )
        {
            
            
            OUString aVertOrientPropStr("VertOrient");
            uno::Any aVertOrient( getPropertyValue( aVertOrientPropStr ) );
            sal_Int16 eVertOrient;
            if (aVertOrient >>= eVertOrient) 
            {
                if ( eVertOrient != text::VertOrientation::NONE )
                {
                    eVertOrient = text::VertOrientation::NONE;
                    aVertOrient <<= eVertOrient;
                    setPropertyValue( aVertOrientPropStr, aVertOrient );
                }
            }
            
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
            
            awt::Point aPos( getPosition() );
            
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            
            
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            
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
            
            awt::Point aPos( getPosition() );
            
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    TWIP_TO_MM100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    TWIP_TO_MM100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            
            
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            
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
