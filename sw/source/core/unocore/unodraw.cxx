/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <initializer_list>
#include <memory>
#include <string_view>

#include <sal/log.hxx>

#include <cmdid.h>
#include <unomid.h>

#include <drawdoc.hxx>
#include <unodraw.hxx>
#include <unoframe.hxx>
#include <unoparagraph.hxx>
#include <unotextrange.hxx>
#include <svx/svditer.hxx>
#include <swunohelper.hxx>
#include <textboxhelper.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <fmtcntnt.hxx>
#include <fmtflcnt.hxx>
#include <txatbase.hxx>
#include <docsh.hxx>
#include <unomap.hxx>
#include <unoport.hxx>
#include <TextCursorHelper.hxx>
#include <dflyobj.hxx>
#include <ndtxt.hxx>
#include <svx/svdview.hxx>
#include <svx/unoshape.hxx>
#include <dcontact.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfollowtextflow.hxx>
#include <rootfrm.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <crstate.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;

class SwShapeDescriptor_Impl
{
    bool m_isInReading;
    std::unique_ptr<SwFormatHoriOrient> m_pHOrient;
    std::unique_ptr<SwFormatVertOrient> m_pVOrient;
    std::unique_ptr<SwFormatAnchor> m_pAnchor;
    std::unique_ptr<SwFormatSurround> m_pSurround;
    std::unique_ptr<SvxULSpaceItem> m_pULSpace;
    std::unique_ptr<SvxLRSpaceItem> m_pLRSpace;
    bool            m_bOpaque;
    uno::Reference< text::XTextRange > m_xTextRange;
    // #i26791#
    std::unique_ptr<SwFormatFollowTextFlow> m_pFollowTextFlow;
    // #i28701#
    std::unique_ptr<SwFormatWrapInfluenceOnObjPos> m_pWrapInfluenceOnObjPos;
    // #i28749#
    sal_Int16 mnPositionLayoutDir;

    SwShapeDescriptor_Impl(const SwShapeDescriptor_Impl&) = delete;
    SwShapeDescriptor_Impl& operator=(const SwShapeDescriptor_Impl&) = delete;

public:
    bool    m_bInitializedPropertyNotifier;

public:
    SwShapeDescriptor_Impl(SwDoc const*const pDoc)
        : m_isInReading(pDoc && pDoc->IsInReading())
     // #i32349# - no defaults, in order to determine on
     // adding a shape, if positioning attributes are set or not.
        , m_bOpaque(false)
     // #i26791#
        , m_pFollowTextFlow( new SwFormatFollowTextFlow(false) )
     // #i28701# #i35017#
        , m_pWrapInfluenceOnObjPos( new SwFormatWrapInfluenceOnObjPos(
                            text::WrapInfluenceOnPosition::ONCE_CONCURRENT) )
     // #i28749#
        , mnPositionLayoutDir(text::PositionLayoutDir::PositionInLayoutDirOfAnchor)
        , m_bInitializedPropertyNotifier(false)
     {}

    SwFormatAnchor*    GetAnchor(bool bCreate = false)
        {
            if (bCreate && !m_pAnchor)
            {
                m_pAnchor.reset(new SwFormatAnchor(RndStdIds::FLY_AS_CHAR));
            }
            return m_pAnchor.get();
        }
    SwFormatHoriOrient* GetHOrient(bool bCreate = false)
        {
            if (bCreate && !m_pHOrient)
            {
                // #i26791#
                m_pHOrient.reset(new SwFormatHoriOrient(0, text::HoriOrientation::NONE, text::RelOrientation::FRAME));
            }
            return m_pHOrient.get();
        }
    SwFormatVertOrient* GetVOrient(bool bCreate = false)
        {
            if (bCreate && !m_pVOrient)
            {
                if (m_isInReading && // tdf#113938 extensions might rely on old default
                    (!GetAnchor(true) || m_pAnchor->GetAnchorId() == RndStdIds::FLY_AS_CHAR))
                {   // for as-char, NONE ("from-top") is not a good default
                    m_pVOrient.reset(new SwFormatVertOrient(0, text::VertOrientation::TOP, text::RelOrientation::FRAME));
                }
                else
                {   // #i26791#
                    m_pVOrient.reset(new SwFormatVertOrient(0, text::VertOrientation::NONE, text::RelOrientation::FRAME));
                }
            }
            return m_pVOrient.get();
        }

    SwFormatSurround*  GetSurround(bool bCreate = false)
        {
            if (bCreate && !m_pSurround)
            {
                m_pSurround.reset(new SwFormatSurround());
            }
            return m_pSurround.get();
        }
    SvxLRSpaceItem* GetLRSpace(bool bCreate = false)
        {
            if (bCreate && !m_pLRSpace)
            {
                m_pLRSpace.reset(new SvxLRSpaceItem(RES_LR_SPACE));
            }
            return m_pLRSpace.get();
        }
    SvxULSpaceItem* GetULSpace(bool bCreate = false)
        {
            if (bCreate && !m_pULSpace)
            {
                m_pULSpace.reset(new SvxULSpaceItem(RES_UL_SPACE));
            }
            return m_pULSpace.get();
        }
    uno::Reference< text::XTextRange > &    GetTextRange()
    {
        return m_xTextRange;
    }
    bool    IsOpaque() const
        {
            return m_bOpaque;
        }
    const bool&    GetOpaque() const
        {
            return m_bOpaque;
        }
    void RemoveHOrient() { m_pHOrient.reset(); }
    void RemoveVOrient() { m_pVOrient.reset(); }
    void RemoveAnchor() { m_pAnchor.reset(); }
    void RemoveSurround() { m_pSurround.reset(); }
    void RemoveULSpace() { m_pULSpace.reset(); }
    void RemoveLRSpace() { m_pLRSpace.reset(); }
    void SetOpaque(bool bSet){m_bOpaque = bSet;}

    // #i26791#
    SwFormatFollowTextFlow* GetFollowTextFlow( bool _bCreate = false )
    {
        if (_bCreate && !m_pFollowTextFlow)
        {
            m_pFollowTextFlow.reset(new SwFormatFollowTextFlow(false));
        }
        return m_pFollowTextFlow.get();
    }
    void RemoveFollowTextFlow()
    {
        m_pFollowTextFlow.reset();
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

    // #i28701#
    SwFormatWrapInfluenceOnObjPos* GetWrapInfluenceOnObjPos(
                                        const bool _bCreate = false )
    {
        if (_bCreate && !m_pWrapInfluenceOnObjPos)
        {
            m_pWrapInfluenceOnObjPos.reset(new SwFormatWrapInfluenceOnObjPos(
                        // #i35017#
                        text::WrapInfluenceOnPosition::ONCE_CONCURRENT));
        }
        return m_pWrapInfluenceOnObjPos.get();
    }
    void RemoveWrapInfluenceOnObjPos()
    {
        m_pWrapInfluenceOnObjPos.reset();
    }
};

SwFmDrawPage::SwFmDrawPage( SdrPage* pPage ) :
    SvxFmDrawPage( pPage ), m_pPageView(nullptr)
{
}

SwFmDrawPage::~SwFmDrawPage() noexcept
{
    while (!m_vShapes.empty())
        m_vShapes.back()->dispose();
    RemovePageView();
}

const SdrMarkList&  SwFmDrawPage::PreGroup(const uno::Reference< drawing::XShapes > & xShapes)
{
    SelectObjectsInView( xShapes, GetPageView() );
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();
    return rMarkList;
}

void SwFmDrawPage::PreUnGroup(const uno::Reference< drawing::XShapeGroup >&  rShapeGroup)
{
    SelectObjectInView( rShapeGroup, GetPageView() );
}

SdrPageView*    SwFmDrawPage::GetPageView()
{
    if(!m_pPageView)
        m_pPageView = mpView->ShowSdrPage( mpPage );
    return m_pPageView;
}

void    SwFmDrawPage::RemovePageView()
{
    if(m_pPageView && mpView)
        mpView->HideSdrPage();
    m_pPageView = nullptr;
}

uno::Reference<drawing::XShape> SwFmDrawPage::GetShape(SdrObject* pObj)
{
    if(!pObj)
        return nullptr;
    SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
    SwFmDrawPage* pPage = dynamic_cast<SwFmDrawPage*>(pFormat);
    if(!pPage || pPage->m_vShapes.empty())
        return uno::Reference<drawing::XShape>(pObj->getUnoShape(), uno::UNO_QUERY);
    for(auto pShape : pPage->m_vShapes)
    {
        SvxShape* pSvxShape = pShape->GetSvxShape();
        if (pSvxShape && pSvxShape->GetSdrObject() == pObj)
            return uno::Reference<drawing::XShape>(static_cast<::cppu::OWeakObject*>(pShape), uno::UNO_QUERY);
    }
    return nullptr;
}

uno::Reference<drawing::XShapeGroup> SwFmDrawPage::GetShapeGroup(SdrObject* pObj)
{
    return uno::Reference<drawing::XShapeGroup>(GetShape(pObj), uno::UNO_QUERY);
}

uno::Reference< drawing::XShape > SwFmDrawPage::CreateShape( SdrObject *pObj ) const
{
    uno::Reference< drawing::XShape >  xRet;
    if(dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr || pObj->GetObjInventor() == SdrInventor::Swg)
    {
        SwFlyDrawContact* pFlyContact = static_cast<SwFlyDrawContact*>(pObj->GetUserCall());
        if(pFlyContact)
        {
            SwFrameFormat* pFlyFormat = pFlyContact->GetFormat();
            SwDoc* pDoc = pFlyFormat->GetDoc();
            const SwNodeIndex* pIdx;
            if( RES_FLYFRMFMT == pFlyFormat->Which()
                && nullptr != ( pIdx = pFlyFormat->GetContent().GetContentIdx() )
                && pIdx->GetNodes().IsDocNodes()
                )
            {
                const SwNode* pNd = pDoc->GetNodes()[ pIdx->GetIndex() + 1 ];
                if(!pNd->IsNoTextNode())
                {
                    xRet.set(SwXTextFrame::CreateXTextFrame(*pDoc, pFlyFormat),
                            uno::UNO_QUERY);
                }
                else if( pNd->IsGrfNode() )
                {
                    xRet.set(SwXTextGraphicObject::CreateXTextGraphicObject(
                                *pDoc, pFlyFormat), uno::UNO_QUERY);
                }
                else if( pNd->IsOLENode() )
                {
                    xRet.set(SwXTextEmbeddedObject::CreateXTextEmbeddedObject(
                                *pDoc, pFlyFormat), uno::UNO_QUERY);
                }
            }
            else
            {
                OSL_FAIL( "<SwFmDrawPage::CreateShape(..)> - could not retrieve type. Thus, no shape created." );
                return xRet;
            }
        }
    }
    else
    {
        // own block - temporary object has to be destroyed before
        // the delegator is set #81670#
        {
            xRet = SvxFmDrawPage::CreateShape( pObj );
        }
        uno::Reference< XUnoTunnel > xShapeTunnel(xRet, uno::UNO_QUERY);
        //don't create an SwXShape if it already exists
        rtl::Reference<SwXShape> pShape = comphelper::getFromUnoTunnel<SwXShape>(xShapeTunnel);
        if(!pShape)
        {
            xShapeTunnel = nullptr;
            uno::Reference< uno::XInterface > xCreate(xRet, uno::UNO_QUERY);
            xRet = nullptr;
            if ( pObj->IsGroupObject() && (!pObj->Is3DObj() || (dynamic_cast<const E3dScene*>( pObj) !=  nullptr)) )
                pShape = new SwXGroupShape(xCreate, nullptr);
            else
                pShape = new SwXShape(xCreate, nullptr);
            xRet = pShape;
        }
        const_cast<std::vector<SwXShape*>*>(&m_vShapes)->push_back(pShape.get());
        pShape->m_pPage = this;
    }
    return xRet;
}

namespace
{
    class SwXShapesEnumeration
        : public SwSimpleEnumeration_Base
    {
        private:
            std::vector< css::uno::Any > m_aShapes;
        protected:
            virtual ~SwXShapesEnumeration() override {};
        public:
            explicit SwXShapesEnumeration(SwXDrawPage* const pDrawPage);

            //XEnumeration
            virtual sal_Bool SAL_CALL hasMoreElements() override;
            virtual uno::Any SAL_CALL nextElement() override;

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName() override;
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
            virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    };
}

SwXShapesEnumeration::SwXShapesEnumeration(SwXDrawPage* const pDrawPage)
{
    SolarMutexGuard aGuard;
    sal_Int32 nCount = pDrawPage->getCount();
    m_aShapes.reserve(nCount);
    for(sal_Int32 nIdx = 0; nIdx < nCount; nIdx++)
    {
        uno::Reference<drawing::XShape> xShape(pDrawPage->getByIndex(nIdx), uno::UNO_QUERY);
        m_aShapes.push_back(uno::makeAny(xShape));
    }
}

sal_Bool SwXShapesEnumeration::hasMoreElements()
{
    SolarMutexGuard aGuard;
    return !m_aShapes.empty();
}

uno::Any SwXShapesEnumeration::nextElement()
{
    SolarMutexGuard aGuard;
    if(m_aShapes.empty())
        throw container::NoSuchElementException();
    uno::Any aResult = m_aShapes.back();
    m_aShapes.pop_back();
    return aResult;
}

OUString SwXShapesEnumeration::getImplementationName()
{
    return "SwXShapeEnumeration";
}

sal_Bool SwXShapesEnumeration::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SwXShapesEnumeration::getSupportedServiceNames()
{
    return { OUString("com.sun.star.container.XEnumeration") };
}

uno::Reference< container::XEnumeration > SwXDrawPage::createEnumeration()
{
    SolarMutexGuard aGuard;
    return uno::Reference< container::XEnumeration >(
        new SwXShapesEnumeration(this));
}

OUString SwXDrawPage::getImplementationName()
{
    return "SwXDrawPage";
}

sal_Bool SwXDrawPage::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXDrawPage::getSupportedServiceNames()
{
    return { "com.sun.star.drawing.GenericDrawPage" };
}

SwXDrawPage::SwXDrawPage(SwDoc* pDc) :
    m_pDoc(pDc)
{
}

SwXDrawPage::~SwXDrawPage()
{
    if(m_pDrawPage.is())
    {
        uno::Reference< uno::XInterface >  xInt;
        m_pDrawPage->setDelegator(xInt);
    }
}

uno::Any SwXDrawPage::queryInterface( const uno::Type& aType )
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

uno::Sequence< uno::Type > SwXDrawPage::getTypes()
{
    return comphelper::concatSequences(
                SwXDrawPageBaseClass::getTypes(),
                GetSvxPage()->getTypes(),
                uno::Sequence { cppu::UnoType<form::XFormsSupplier2>::get() });
}

sal_Int32 SwXDrawPage::getCount()
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        return 0;
    else
    {
        GetSvxPage();
        return SwTextBoxHelper::getCount(m_pDrawPage->GetSdrPage());
    }
}

uno::Any SwXDrawPage::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        throw lang::IndexOutOfBoundsException();

    GetSvxPage();
    return SwTextBoxHelper::getByIndex(m_pDrawPage->GetSdrPage(), nIndex);
}

uno::Type  SwXDrawPage::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SwXDrawPage::hasElements()
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        return false;
    else
        return GetSvxPage()->hasElements();
}

void SwXDrawPage::add(const uno::Reference< drawing::XShape > & xShape)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pShape = comphelper::getFromUnoTunnel<SwXShape>(xShapeTunnel);
    SvxShape* pSvxShape = comphelper::getFromUnoTunnel<SvxShape>(xShapeTunnel);

    // this is not a writer shape
    if(!pShape)
        throw uno::RuntimeException("illegal object",
                                    static_cast< cppu::OWeakObject * > ( this ) );

    // we're already registered in the model / SwXDrawPage::add() already called
    if(pShape->m_pPage || pShape->m_pFormat || !pShape->m_bDescriptor )
        return;

    // we're inserted elsewhere already
    if ( pSvxShape->GetSdrObject() )
    {
        if ( pSvxShape->GetSdrObject()->IsInserted() )
        {
            return;
        }
    }
    GetSvxPage()->add(xShape);

    OSL_ENSURE(pSvxShape, "Why is here no SvxShape?");
    // this position is definitely in 1/100 mm
    awt::Point aMM100Pos(pSvxShape->getPosition());

    // now evaluate the properties of SwShapeDescriptor_Impl
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END-1> aSet( m_pDoc->GetAttrPool() );
    SwFormatAnchor aAnchor( RndStdIds::FLY_AS_CHAR );
    bool bOpaque = false;
    if( pDesc )
    {
        if(pDesc->GetSurround())
            aSet.Put( *pDesc->GetSurround());
        // all items are already in Twip
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
            SwFormatHoriOrient* pHori = pDesc->GetHOrient( true );
            SwTwips nHoriPos = o3tl::toTwips(aMM100Pos.X, o3tl::Length::mm100);
            pHori->SetPos( nHoriPos );
        }
        {
            if(pDesc->GetHOrient()->GetHoriOrient() == text::HoriOrientation::NONE)
                aMM100Pos.X = convertTwipToMm100(pDesc->GetHOrient()->GetPos());
            aSet.Put( *pDesc->GetHOrient() );
        }
        // #i32349# - if no vertical position exists, create one
        if ( !pDesc->GetVOrient() )
        {
            SwFormatVertOrient* pVert = pDesc->GetVOrient( true );
            SwTwips nVertPos = o3tl::toTwips(aMM100Pos.Y, o3tl::Length::mm100);
            pVert->SetPos( nVertPos );
        }
        {
            if(pDesc->GetVOrient()->GetVertOrient() == text::VertOrientation::NONE)
                aMM100Pos.Y = convertTwipToMm100(pDesc->GetVOrient()->GetPos());
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
    if(SdrInventor::FmForm != pObj->GetObjInventor())
        pObj->SetLayer( bOpaque ? m_pDoc->getIDocumentDrawModelAccess().GetInvisibleHeavenId() : m_pDoc->getIDocumentDrawModelAccess().GetInvisibleHellId() );
    else
        pObj->SetLayer(m_pDoc->getIDocumentDrawModelAccess().GetInvisibleControlsId());

    std::optional<SwPaM> pPam(m_pDoc->GetNodes().GetEndOfContent());
    std::unique_ptr<SwUnoInternalPaM> pInternalPam;
    uno::Reference< text::XTextRange >  xRg;
    if( pDesc && (xRg = pDesc->GetTextRange()).is() )
    {
        pInternalPam.reset(new SwUnoInternalPaM(*m_pDoc));
        if (!::sw::XTextRangeToSwPaM(*pInternalPam, xRg))
            throw uno::RuntimeException();

        if(RndStdIds::FLY_AT_FLY == aAnchor.GetAnchorId() &&
                            !pInternalPam->GetNode().FindFlyStartNode())
        {
                    aAnchor.SetType(RndStdIds::FLY_AS_CHAR);
        }
        else if (RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId())
        {
            aAnchor.SetAnchor(pInternalPam->Start());
        }

    }
    else if ((aAnchor.GetAnchorId() != RndStdIds::FLY_AT_PAGE) && m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout())
    {
        SwCursorMoveState aState( CursorMoveState::SetOnlyText );
        Point aTmp(o3tl::toTwips(aMM100Pos.X, o3tl::Length::mm100), o3tl::toTwips(aMM100Pos.Y, o3tl::Length::mm100));
        m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( pPam->GetPoint(), aTmp, &aState );
        aAnchor.SetAnchor( pPam->GetPoint() );

        // #i32349# - adjustment of vertical positioning
        // attributes no longer needed, because it's already got a default.
    }
    else
    {
        aAnchor.SetType(RndStdIds::FLY_AT_PAGE);

        // #i32349# - adjustment of vertical positioning
        // attributes no longer needed, because it's already got a default.
    }
    aSet.Put(aAnchor);
    SwPaM* pTemp = pInternalPam.get();
    if ( !pTemp )
        pTemp = &*pPam;
    UnoActionContext aAction(m_pDoc);
    m_pDoc->getIDocumentContentOperations().InsertDrawObj( *pTemp, *pObj, aSet );

    if (pSvxShape->GetSdrObject()->GetName().isEmpty())
    {
        pSvxShape->GetSdrObject()->SetName(m_pDoc->GetUniqueShapeName());
    }

    SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
    if (pFormat)
    {
        if (pFormat->GetName().isEmpty())
        {
            pFormat->SetName(pSvxShape->GetSdrObject()->GetName(), false);
        }
        pShape->SetFrameFormat(pFormat);
    }
    pShape->m_bDescriptor = false;

    pPam.reset();
    pInternalPam.reset();
}

void SwXDrawPage::remove(const uno::Reference< drawing::XShape > & xShape)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    // tdf#41466 remove TextFrame too which is belonged to the actual shape
    auto xTextFrame = SwTextBoxHelper::getUnoTextFrame(xShape);
    if (xTextFrame)
    {
        uno::Reference<lang::XComponent> xComp(xTextFrame, uno::UNO_QUERY);
        if (xComp)
            xComp->dispose();
    }
    // remove shape
    uno::Reference<lang::XComponent> xComp(xShape, uno::UNO_QUERY);
    xComp->dispose();
}

uno::Reference< drawing::XShapeGroup >  SwXDrawPage::group(const uno::Reference< drawing::XShapes > & xShapes)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc || !xShapes.is())
        throw uno::RuntimeException();
    uno::Reference< drawing::XShapeGroup >  xRet;
    if(m_pDrawPage.is())
    {

        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage) //TODO: can this be Null?
        {
            // mark and return MarkList
            const SdrMarkList& rMarkList = pPage->PreGroup(xShapes);
            if ( rMarkList.GetMarkCount() > 0 )
            {
                for (size_t i = 0; i < rMarkList.GetMarkCount(); ++i)
                {
                    const SdrObject *pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if (RndStdIds::FLY_AS_CHAR == ::FindFrameFormat(const_cast<SdrObject*>(
                                            pObj))->GetAnchor().GetAnchorId())
                    {
                        throw lang::IllegalArgumentException(
                            "Shape must not have 'as character' anchor!", nullptr, 0);
                    }
                }

                UnoActionContext aContext(m_pDoc);
                m_pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );

                SwDrawContact* pContact = m_pDoc->GroupSelection( *pPage->GetDrawView() );
                m_pDoc->ChgAnchor(
                    pPage->GetDrawView()->GetMarkedObjectList(),
                    RndStdIds::FLY_AT_PARA,
                    true, false );

                pPage->GetDrawView()->UnmarkAll();
                if(pContact)
                    xRet = SwFmDrawPage::GetShapeGroup( pContact->GetMaster() );
                m_pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
            }
            pPage->RemovePageView();
        }
    }
    return xRet;
}

void SwXDrawPage::ungroup(const uno::Reference< drawing::XShapeGroup > & rShapeGroup)
{
    SolarMutexGuard aGuard;
    if(!m_pDoc)
        throw uno::RuntimeException();
    if(!m_pDrawPage.is())
        return;

    SwFmDrawPage* pPage = GetSvxPage();
    if(!pPage) //TODO: can this be Null?
        return;

    pPage->PreUnGroup(rShapeGroup);
    UnoActionContext aContext(m_pDoc);
    m_pDoc->GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );

    m_pDoc->UnGroupSelection( *pPage->GetDrawView() );
    m_pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkedObjectList(),
                RndStdIds::FLY_AT_PARA,
                true, false );
    m_pDoc->GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    pPage->RemovePageView();
}

SwFmDrawPage*   SwXDrawPage::GetSvxPage()
{
    if(!m_pDrawPage.is() && m_pDoc)
    {
        SolarMutexGuard aGuard;
        // #i52858#
        SwDrawModel* pModel = m_pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        SdrPage* pPage = pModel->GetPage( 0 );

        {
            // We need a Ref to the object during queryInterface or else
            // it will be deleted
            m_pDrawPage = new SwFmDrawPage(pPage);
        }
        if( m_pDrawPage.is() )
            m_pDrawPage->setDelegator( static_cast<cppu::OWeakObject*>(this) );
    }
    return m_pDrawPage.get();
}

/**
 * Renamed and outlined to detect where it's called
 */
void SwXDrawPage::InvalidateSwDoc()
{
    m_pDoc = nullptr;
}

const uno::Sequence< sal_Int8 > & SwXShape::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSwXShapeUnoTunnelId;
    return theSwXShapeUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SwXShape::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    if( comphelper::isUnoTunnelId<SwXShape>(rId) )
    {
        return comphelper::getSomething_cast(this);
    }

    if( m_xShapeAgg.is() )
    {
        const uno::Type& rTunnelType = cppu::UnoType<lang::XUnoTunnel>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rTunnelType );
        if(auto xAggTunnel = o3tl::tryAccess<uno::Reference<lang::XUnoTunnel>>(
               aAgg))
        {
            if(xAggTunnel->is())
                return (*xAggTunnel)->getSomething(rId);
        }
    }
    return 0;
}
namespace
{
    void lcl_addShapePropertyEventFactories( SdrObject& _rObj, SwXShape& _rShape )
    {
        auto pProvider = std::make_shared<svx::PropertyValueProvider>( _rShape, "AnchorType" );
        _rObj.getShapePropertyChangeNotifier().registerProvider( svx::ShapeProperty::TextDocAnchor, pProvider );
    }
}

SwXShape::SwXShape(
        uno::Reference<uno::XInterface> & xShape,
        SwDoc const*const pDoc)
    : m_pPage(nullptr)
    , m_pFormat(nullptr)
    , m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_SHAPE))
    , m_pPropertyMapEntries(aSwMapProvider.GetPropertyMapEntries(PROPERTY_MAP_TEXT_SHAPE))
    , m_pImpl(new SwShapeDescriptor_Impl(pDoc))
    , m_bDescriptor(true)
{
    if(!xShape.is())  // default Ctor
        return;

    const uno::Type& rAggType = cppu::UnoType<uno::XAggregation>::get();
    //aAgg contains a reference of the SvxShape!
    {
        uno::Any aAgg = xShape->queryInterface(rAggType);
        aAgg >>= m_xShapeAgg;
        // #i31698#
        if ( m_xShapeAgg.is() )
        {
            m_xShapeAgg->queryAggregation( cppu::UnoType<drawing::XShape>::get()) >>= mxShape;
            OSL_ENSURE( mxShape.is(),
                    "<SwXShape::SwXShape(..)> - no XShape found at <xShapeAgg>" );
        }
    }
    xShape = nullptr;
    osl_atomic_increment(&m_refCount);
    if( m_xShapeAgg.is() )
        m_xShapeAgg->setDelegator( static_cast<cppu::OWeakObject*>(this) );
    osl_atomic_decrement(&m_refCount);

    SdrObject* pObj = SdrObject::getSdrObjectFromXShape(m_xShapeAgg);
    if(pObj)
    {
        auto pFormat = ::FindFrameFormat( pObj );
        if(pFormat)
            SetFrameFormat(pFormat);

        lcl_addShapePropertyEventFactories( *pObj, *this );
        m_pImpl->m_bInitializedPropertyNotifier = true;
    }

}

void SwXShape::AddExistingShapeToFormat( SdrObject const & _rObj )
{
    SdrObjListIter aIter( _rObj, SdrIterMode::DeepNoGroups );
    while ( aIter.IsMore() )
    {
        SdrObject* pCurrent = aIter.Next();
        OSL_ENSURE( pCurrent, "SwXShape::AddExistingShapeToFormat: invalid object list element!" );
        if ( !pCurrent )
            continue;

        auto pSwShape = comphelper::getFromUnoTunnel<SwXShape>(pCurrent->getWeakUnoShape());
        if ( pSwShape )
        {
            if ( pSwShape->m_bDescriptor )
            {
                auto pFormat = ::FindFrameFormat( pCurrent );
                if ( pFormat )
                    pSwShape->SetFrameFormat(pFormat);
                pSwShape->m_bDescriptor = false;
            }

            if ( !pSwShape->m_pImpl->m_bInitializedPropertyNotifier )
            {
                lcl_addShapePropertyEventFactories( *pCurrent, *pSwShape );
                pSwShape->m_pImpl->m_bInitializedPropertyNotifier = true;
            }
        }
    }
}

SwXShape::~SwXShape()
{
    SolarMutexGuard aGuard;

    if (m_xShapeAgg.is())
    {
        uno::Reference< uno::XInterface >  xRef;
        m_xShapeAgg->setDelegator(xRef);
    }
    m_pImpl.reset();
    EndListeningAll();
    if(m_pPage)
       const_cast<SwFmDrawPage*>(m_pPage)->RemoveShape(this);
    m_pPage = nullptr;
}

uno::Any SwXShape::queryInterface( const uno::Type& aType )
{
    uno::Any aRet;
    SdrObject* pObj = nullptr;

    if ((aType == cppu::UnoType<text::XText>::get())
        || (aType == cppu::UnoType<text::XTextRange>::get())
        || (aType == cppu::UnoType<text::XTextAppend>::get()))
    {
        pObj = SdrObject::getSdrObjectFromXShape(mxShape);

        aRet = SwTextBoxHelper::queryInterface(GetFrameFormat(), aType, pObj);
        if (aRet.hasValue())
            return aRet;
    }
    aRet = SwXShapeBaseClass::queryInterface(aType);
    // #i53320# - follow-up of #i31698#
    // interface drawing::XShape is overloaded. Thus, provide
    // correct object instance.
    if(!aRet.hasValue() && m_xShapeAgg.is())
    {
        if(aType == cppu::UnoType<XShape>::get())
            aRet <<= uno::Reference<XShape>(this);
        else
            aRet = m_xShapeAgg->queryAggregation(aType);
    }
    return aRet;
}

uno::Sequence< uno::Type > SwXShape::getTypes(  )
{
    uno::Sequence< uno::Type > aRet = SwXShapeBaseClass::getTypes();
    if(m_xShapeAgg.is())
    {
        uno::Any aProv = m_xShapeAgg->queryAggregation(cppu::UnoType<XTypeProvider>::get());
        if(aProv.hasValue())
        {
            uno::Reference< XTypeProvider > xAggProv;
            aProv >>= xAggProv;
            return comphelper::concatSequences(aRet, xAggProv->getTypes());
        }
    }
    return aRet;
}

uno::Sequence< sal_Int8 > SwXShape::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference< beans::XPropertySetInfo >  SwXShape::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    if (!mxPropertySetInfo)
    {
        uno::Reference< beans::XPropertySetInfo >  aRet;
        if(m_xShapeAgg.is())
        {
            const uno::Type& rPropSetType = cppu::UnoType<beans::XPropertySet>::get();
            uno::Any aPSet = m_xShapeAgg->queryAggregation( rPropSetType );
            if(auto xPrSet = o3tl::tryAccess<uno::Reference<beans::XPropertySet>>(
                   aPSet))
            {
                uno::Reference< beans::XPropertySetInfo >  xInfo = (*xPrSet)->getPropertySetInfo();
                // Expand PropertySetInfo!
                const uno::Sequence<beans::Property> aPropSeq = xInfo->getProperties();
                mxPropertySetInfo = new SfxExtItemPropertySetInfo( m_pPropertyMapEntries, aPropSeq );
            }
        }
        if(!mxPropertySetInfo)
            mxPropertySetInfo = m_pPropSet->getPropertySetInfo();
    }
    return mxPropertySetInfo;
}

void SwXShape::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if(!m_xShapeAgg.is())
        return;

    if(pEntry)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException ("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        // with the layout it is possible to move the anchor without changing the position
        if(pFormat)
        {
            SwAttrSet aSet(pFormat->GetAttrSet());
            SwDoc* pDoc = pFormat->GetDoc();
            if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORFRAME == pEntry->nMemberId)
            {
                bool bDone = false;
                uno::Reference<text::XTextFrame> xFrame;
                if(aValue >>= xFrame)
                {
                    SwXFrame* pFrame = comphelper::getFromUnoTunnel<SwXFrame>(xFrame);
                    if(pFrame && pFrame->GetFrameFormat() &&
                        pFrame->GetFrameFormat()->GetDoc() == pDoc)
                    {
                        UnoActionContext aCtx(pDoc);
                        SfxItemSetFixed<RES_FRMATR_BEGIN, RES_FRMATR_END - 1> aItemSet( pDoc->GetAttrPool() );
                        aItemSet.SetParent(&pFormat->GetAttrSet());
                        SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aItemSet.Get(pEntry->nWID));
                        SwPosition aPos(*pFrame->GetFrameFormat()->GetContent().GetContentIdx());
                        aAnchor.SetAnchor(&aPos);
                        aAnchor.SetType(RndStdIds::FLY_AT_FLY);
                        aItemSet.Put(aAnchor);
                        pFormat->SetFormatAttr(aItemSet);
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
                    // set layer of new drawing
                    // object to corresponding invisible layer.
                    bool bIsVisible = pDoc->getIDocumentDrawModelAccess().IsVisibleLayerId( pObj->GetLayer() );
                    if(SdrInventor::FmForm != pObj->GetObjInventor())
                    {
                        pObj->SetLayer( *o3tl::doAccess<bool>(aValue)
                                        ? ( bIsVisible ? pDoc->getIDocumentDrawModelAccess().GetHeavenId() : pDoc->getIDocumentDrawModelAccess().GetInvisibleHeavenId() )
                                        : ( bIsVisible ? pDoc->getIDocumentDrawModelAccess().GetHellId() : pDoc->getIDocumentDrawModelAccess().GetInvisibleHellId() ));
                    }
                    else
                    {
                        pObj->SetLayer( bIsVisible ? pDoc->getIDocumentDrawModelAccess().GetControlsId() : pDoc->getIDocumentDrawModelAccess().GetInvisibleControlsId());
                    }

                }

            }
            // #i26791# - special handling for property FN_TEXT_RANGE
            else if ( FN_TEXT_RANGE == pEntry->nWID )
            {
                SwFormatAnchor aAnchor( aSet.Get( RES_ANCHOR ) );
                if (aAnchor.GetAnchorId() == RndStdIds::FLY_AT_PAGE)
                {
                    // set property <TextRange> not valid for to-page anchored shapes
                    throw lang::IllegalArgumentException();
                }

                std::unique_ptr<SwUnoInternalPaM> pInternalPam(
                                new SwUnoInternalPaM( *(pFormat->GetDoc()) ));
                uno::Reference< text::XTextRange > xRg;
                aValue >>= xRg;
                if (!::sw::XTextRangeToSwPaM(*pInternalPam, xRg) )
                {
                    throw uno::RuntimeException();
                }

                if (aAnchor.GetAnchorId() == RndStdIds::FLY_AS_CHAR)
                {
                    //delete old SwFormatFlyCnt
                    //With AnchorAsCharacter the current TextAttribute has to be deleted.
                    //Tbis removes the frame format too.
                    //To prevent this the connection between format and attribute has to be broken before.
                    const SwPosition *pPos = aAnchor.GetContentAnchor();
                    SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
                    SAL_WARN_IF( !pTextNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                    const sal_Int32 nIdx = pPos->nContent.GetIndex();
                    SwTextAttr * const pHint =
                            pTextNode->GetTextAttrForCharAt(
                            nIdx, RES_TXTATR_FLYCNT );
                    assert(pHint && "Missing Hint.");
                    SAL_WARN_IF( pHint->Which() != RES_TXTATR_FLYCNT,
                                "sw.uno", "Missing FlyInCnt-Hint." );
                    SAL_WARN_IF( pHint->GetFlyCnt().GetFrameFormat() != pFormat,
                                "sw.uno", "Wrong TextFlyCnt-Hint." );
                    const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt())
                        .SetFlyFormat();

                    //The connection is removed now the attribute can be deleted.
                    pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx );
                    //create a new one
                    SwTextNode *pNd = pInternalPam->GetNode().GetTextNode();
                    SAL_WARN_IF( !pNd, "sw.uno", "Cursor not at TextNode." );
                    SwFormatFlyCnt aFormat( pFormat );
                    pNd->InsertItem(aFormat, pInternalPam->GetPoint()
                            ->nContent.GetIndex(), 0 );
                }
                else
                {
                    aAnchor.SetAnchor( pInternalPam->GetPoint() );
                    aSet.Put(aAnchor);
                    pFormat->SetFormatAttr(aSet);
                }
            }
            else if (pEntry->nWID == FN_TEXT_BOX)
            {
                if (pEntry->nMemberId == MID_TEXT_BOX)
                {
                    bool bValue(false);
                    aValue >>= bValue;
                    if (bValue)
                        SwTextBoxHelper::create(pFormat, GetSvxShape()->GetSdrObject());
                    else
                        SwTextBoxHelper::destroy(pFormat, GetSvxShape()->GetSdrObject());
                }
                else if (pEntry->nMemberId == MID_TEXT_BOX_CONTENT)
                {
                    if (aValue.getValueType() == cppu::UnoType<uno::Reference<text::XTextFrame>>::get())
                        SwTextBoxHelper::set(pFormat, GetSvxShape()->GetSdrObject(),
                                             aValue.get<uno::Reference<text::XTextFrame>>());
                    else
                        SAL_WARN( "sw.uno", "This is not a TextFrame!" );
                }
            }
            else if (pEntry->nWID == RES_CHAIN)
            {
                if (pEntry->nMemberId == MID_CHAIN_NEXTNAME || pEntry->nMemberId == MID_CHAIN_PREVNAME)
                    SwTextBoxHelper::syncProperty(pFormat, pEntry->nWID, pEntry->nMemberId, aValue,
                                                  SdrObject::getSdrObjectFromXShape(mxShape));
            }
            // #i28749#
            else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
            {
                sal_Int16 nPositionLayoutDir = 0;
                aValue >>= nPositionLayoutDir;
                pFormat->SetPositionLayoutDir( nPositionLayoutDir );
            }
            else if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout())
            {
                UnoActionContext aCtx(pDoc);
                if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                {
                    SdrObject* pObj = pFormat->FindSdrObject();
                    SdrMarkList aList;
                    SdrMark aMark(pObj);
                    aList.InsertEntry(aMark);
                    sal_Int32 nAnchor = 0;
                    cppu::enum2int( nAnchor, aValue );
                    pDoc->ChgAnchor( aList, static_cast<RndStdIds>(nAnchor),
                                            false, true );
                }
                else
                {
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pFormat->SetFormatAttr(aSet);
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
            else if (pEntry->nWID == RES_HORI_ORIENT
                && pEntry->nMemberId == MID_HORIORIENT_RELATION
                && aSet.Get(RES_ANCHOR).GetAnchorId() == RndStdIds::FLY_AT_PAGE)
            {
                uno::Any value(aValue);
                sal_Int16 nRelOrient(text::RelOrientation::PAGE_FRAME);
                aValue >>= nRelOrient;
                if (sw::GetAtPageRelOrientation(nRelOrient, true))
                {
                    SAL_WARN("sw.core", "SwXShape: fixing invalid horizontal RelOrientation for at-page anchor");
                    value <<= nRelOrient;
                }
                m_pPropSet->setPropertyValue( *pEntry, value, aSet );
                pFormat->SetFormatAttr(aSet);
            }
            else
            {
                m_pPropSet->setPropertyValue( *pEntry, aValue, aSet );

                if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                {
                    bool bSetAttr = true;
                    text::TextContentAnchorType eNewAnchor = static_cast<text::TextContentAnchorType>(SWUnoHelper::GetEnumAsInt32( aValue ));

                    //if old anchor was in_cntnt the related text attribute has to be removed
                    const SwFormatAnchor& rOldAnchor = pFormat->GetAnchor();
                    RndStdIds eOldAnchorId = rOldAnchor.GetAnchorId();
                    SdrObject* pObj = pFormat->FindSdrObject();
                    SwFrameFormat *pFlyFormat = FindFrameFormat( pObj );
                    pFlyFormat->DelFrames();
                    if( text::TextContentAnchorType_AS_CHARACTER != eNewAnchor &&
                        (RndStdIds::FLY_AS_CHAR == eOldAnchorId))
                    {
                        //With AnchorAsCharacter the current TextAttribute has to be deleted.
                        //Tbis removes the frame format too.
                        //To prevent this the connection between format and attribute has to be broken before.
                        const SwPosition *pPos = rOldAnchor.GetContentAnchor();
                        SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
                        SAL_WARN_IF( !pTextNode->HasHints(), "sw.uno", "Missing FlyInCnt-Hint." );
                        const sal_Int32 nIdx = pPos->nContent.GetIndex();
                        SwTextAttr * const pHint =
                            pTextNode->GetTextAttrForCharAt(
                                nIdx, RES_TXTATR_FLYCNT );
                        assert(pHint && "Missing Hint.");
                        SAL_WARN_IF( pHint->Which() != RES_TXTATR_FLYCNT,
                                    "sw.uno", "Missing FlyInCnt-Hint." );
                        SAL_WARN_IF( pHint->GetFlyCnt().GetFrameFormat() != pFlyFormat,
                                    "sw.uno", "Wrong TextFlyCnt-Hint." );
                        const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt())
                            .SetFlyFormat();

                        //The connection is removed now the attribute can be deleted.
                        pTextNode->DeleteAttributes(RES_TXTATR_FLYCNT, nIdx);
                    }
                    else if( text::TextContentAnchorType_AT_PAGE != eNewAnchor &&
                            (RndStdIds::FLY_AT_PAGE == eOldAnchorId))
                    {
                        SwFormatAnchor aNewAnchor( dynamic_cast< const SwFormatAnchor& >( aSet.Get( RES_ANCHOR ) ) );
                        //if the fly has been anchored at page then it needs to be connected
                        //to the content position
                        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                        if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                        {
                            SwCursorMoveState aState( CursorMoveState::SetOnlyText );
                            Point aTmp( pObj->GetSnapRect().TopLeft() );
                            pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( aPam.GetPoint(), aTmp, &aState );
                        }
                        else
                        {
                            //without access to the layout the last node of the body will be used as anchor position
                            aPam.Move( fnMoveBackward, GoInDoc );
                        }
                        //anchor position has to be inserted after the text attribute has been inserted
                        aNewAnchor.SetAnchor( aPam.GetPoint() );
                        aSet.Put( aNewAnchor );
                        pFormat->SetFormatAttr(aSet);
                        bSetAttr = false;
                    }
                    if( text::TextContentAnchorType_AS_CHARACTER == eNewAnchor &&
                        (RndStdIds::FLY_AS_CHAR != eOldAnchorId))
                    {
                        SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                        if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                        {
                            SwCursorMoveState aState( CursorMoveState::SetOnlyText );
                            Point aTmp( pObj->GetSnapRect().TopLeft() );
                            pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetModelPositionForViewPoint( aPam.GetPoint(), aTmp, &aState );
                        }
                        else
                        {
                            //without access to the layout the last node of the body will be used as anchor position
                            aPam.Move( fnMoveBackward, GoInDoc );
                        }
                        //the RES_TXTATR_FLYCNT needs to be added now
                        SwTextNode *pNd = aPam.GetNode().GetTextNode();
                        SAL_WARN_IF( !pNd, "sw.uno", "Cursor is not in a TextNode." );
                        SwFormatFlyCnt aFormat( pFlyFormat );
                        pNd->InsertItem(aFormat,
                            aPam.GetPoint()->nContent.GetIndex(), 0 );
                        --aPam.GetPoint()->nContent; // InsertItem moved it
                        SwFormatAnchor aNewAnchor(
                            dynamic_cast<const SwFormatAnchor&>(
                                aSet.Get(RES_ANCHOR)));
                        aNewAnchor.SetAnchor( aPam.GetPoint() );
                        aSet.Put( aNewAnchor );
                    }
                    if( bSetAttr )
                        pFormat->SetFormatAttr(aSet);
                }
                else
                    pFormat->SetFormatAttr(aSet);
            }
            // We have a pFormat and a pEntry as well: try to sync TextBox property.
            SwTextBoxHelper::syncProperty(pFormat, pEntry->nWID, pEntry->nMemberId, aValue,
                                          SdrObject::getSdrObjectFromXShape(mxShape));
        }
        else
        {
            SfxPoolItem* pItem = nullptr;
            switch(pEntry->nWID)
            {
                case RES_ANCHOR:
                    pItem = m_pImpl->GetAnchor(true);
                break;
                case RES_HORI_ORIENT:
                    pItem = m_pImpl->GetHOrient(true);
                break;
                case RES_VERT_ORIENT:
                    pItem = m_pImpl->GetVOrient(true);
                break;
                case  RES_LR_SPACE:
                    pItem = m_pImpl->GetLRSpace(true);
                break;
                case  RES_UL_SPACE:
                    pItem = m_pImpl->GetULSpace(true);
                break;
                case  RES_SURROUND:
                    pItem = m_pImpl->GetSurround(true);
                break;
                case  FN_TEXT_RANGE:
                    if(auto tr = o3tl::tryAccess<
                           uno::Reference<text::XTextRange>>(aValue))
                    {
                        uno::Reference< text::XTextRange > & rRange = m_pImpl->GetTextRange();
                        rRange = *tr;
                    }
                break;
                case RES_OPAQUE :
                    m_pImpl->SetOpaque(*o3tl::doAccess<bool>(aValue));
                break;
                // #i26791#
                case RES_FOLLOW_TEXT_FLOW:
                {
                    pItem = m_pImpl->GetFollowTextFlow( true );
                }
                break;
                // #i28701#
                case RES_WRAP_INFLUENCE_ON_OBJPOS:
                {
                    pItem = m_pImpl->GetWrapInfluenceOnObjPos( true );
                }
                break;
                // #i28749#
                case FN_SHAPE_POSITION_LAYOUT_DIR :
                {
                    sal_Int16 nPositionLayoutDir = 0;
                    aValue >>= nPositionLayoutDir;
                    m_pImpl->SetPositionLayoutDir( nPositionLayoutDir );
                }
                break;
            }
            if(pItem)
                pItem->PutValue(aValue, pEntry->nMemberId);
        }
    }
    else
    {
        const uno::Type& rPSetType =
            cppu::UnoType<beans::XPropertySet>::get();
        uno::Any aPSet = m_xShapeAgg->queryAggregation(rPSetType);
        auto xPrSet = o3tl::tryAccess<uno::Reference<beans::XPropertySet>>(
            aPSet);
        if(!xPrSet)
            throw uno::RuntimeException();
        // #i31698# - setting the caption point of a
        // caption object doesn't have to change the object position.
        // Thus, keep the position, before the caption point is set and
        // restore it afterwards.
        awt::Point aKeepedPosition( 0, 0 );
        if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
        {
                aKeepedPosition = getPosition();
        }
        if( pFormat && pFormat->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
        {
            UnoActionContext aCtx(pFormat->GetDoc());
            (*xPrSet)->setPropertyValue(rPropertyName, aValue);
        }
        else
            (*xPrSet)->setPropertyValue(rPropertyName, aValue);

        if (pFormat)
        {
            // We have a pFormat (but no pEntry): try to sync TextBox property.
            SwTextBoxHelper::syncProperty(pFormat, rPropertyName, aValue,
                                          SdrObject::getSdrObjectFromXShape(mxShape));
        }

        // #i31698# - restore object position, if caption point is set.
        if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
        {
            setPosition( aKeepedPosition );
        }
    }
}

uno::Any SwXShape::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(m_xShapeAgg.is())
    {
        const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if(pFormat)
            {
                if(RES_OPAQUE == pEntry->nWID)
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    OSL_ENSURE(pSvxShape, "No SvxShape found!");
                    if(pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        // consider invisible layers
                        aRet <<=
                            ( pObj->GetLayer() != pFormat->GetDoc()->getIDocumentDrawModelAccess().GetHellId() &&
                              pObj->GetLayer() != pFormat->GetDoc()->getIDocumentDrawModelAccess().GetInvisibleHellId() );
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
                        awt::Point aPoint( convertTwipToMm100( aPt.X() ),
                                           convertTwipToMm100( aPt.Y() ) );
                        aRet <<= aPoint;
                    }
                }
                // #i26791# - special handling for FN_TEXT_RANGE
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    const SwFormatAnchor aAnchor = pFormat->GetAnchor();
                    if (aAnchor.GetAnchorId() == RndStdIds::FLY_AT_PAGE)
                    {
                        // return nothing, because property <TextRange> isn't
                        // valid for to-page anchored shapes
                        uno::Any aAny;
                        aRet = aAny;
                    }
                    else
                    {
                        if ( aAnchor.GetContentAnchor() )
                        {
                            const uno::Reference< text::XTextRange > xTextRange
                                = SwXTextRange::CreateXTextRange(
                                                    *pFormat->GetDoc(),
                                                    *aAnchor.GetContentAnchor(),
                                                    nullptr );
                            aRet <<= xTextRange;
                        }
                        else
                        {
                            // return nothing
                            uno::Any aAny;
                            aRet = aAny;
                        }
                    }
                }
                else if (pEntry->nWID == FN_TEXT_BOX)
                {
                    if (pEntry->nMemberId == MID_TEXT_BOX)
                    {
                        auto pSvxShape = GetSvxShape();
                        bool bValue = SwTextBoxHelper::isTextBox(
                            pFormat, RES_DRAWFRMFMT,
                            ((pSvxShape && pSvxShape->GetSdrObject()) ? pSvxShape->GetSdrObject()
                                : pFormat->FindRealSdrObject()));
                        aRet <<= bValue;
                    }
                    else if (pEntry->nMemberId == MID_TEXT_BOX_CONTENT)
                    {
                        auto pObj = SdrObject::getSdrObjectFromXShape(mxShape);
                        auto xRange = SwTextBoxHelper::queryInterface(
                            pFormat, cppu::UnoType<text::XText>::get(),
                            pObj ? pObj : pFormat->FindRealSdrObject());
                        uno::Reference<text::XTextFrame> xFrame(xRange, uno::UNO_QUERY);
                        if (xFrame.is())
                            aRet <<= xFrame;
                    }
                }
                else if (pEntry->nWID == RES_CHAIN)
                {
                    switch (pEntry->nMemberId)
                    {
                    case MID_CHAIN_PREVNAME:
                    case MID_CHAIN_NEXTNAME:
                    case MID_CHAIN_NAME:
                        SwTextBoxHelper::getProperty(pFormat, pEntry->nWID, pEntry->nMemberId, aRet);
                    break;
                    }
                }
                // #i28749#
                else if ( FN_SHAPE_TRANSFORMATION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::Transformation>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( "Transformation" );
                }
                else if ( FN_SHAPE_POSITION_LAYOUT_DIR == pEntry->nWID )
                {
                    aRet <<= pFormat->GetPositionLayoutDir();
                }
                // #i36248#
                else if ( FN_SHAPE_STARTPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::StartPosition>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( "StartPosition" );
                }
                else if ( FN_SHAPE_ENDPOSITION_IN_HORI_L2R == pEntry->nWID )
                {
                    // get property <::drawing::Shape::EndPosition>
                    // without conversion to layout direction as below
                    aRet = _getPropAtAggrObj( "EndPosition" );
                }
                else if (pEntry->nWID == RES_FRM_SIZE &&
                         (pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT ||
                          pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH ||
                          pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT_RELATION ||
                          pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH_RELATION))
                {
                    SvxShape* pSvxShape = GetSvxShape();
                    SAL_WARN_IF(!pSvxShape, "sw.uno", "No SvxShape found!");
                    sal_Int16 nRet = 0;
                    if (pSvxShape)
                    {
                        SdrObject* pObj = pSvxShape->GetSdrObject();
                        switch (pEntry->nMemberId)
                        {
                        case MID_FRMSIZE_REL_WIDTH:
                            if (pObj->GetRelativeWidth())
                                nRet = *pObj->GetRelativeWidth() * 100;
                            break;
                        case MID_FRMSIZE_REL_HEIGHT:
                            if (pObj->GetRelativeHeight())
                                nRet = *pObj->GetRelativeHeight() * 100;
                            break;
                        case MID_FRMSIZE_REL_WIDTH_RELATION:
                            nRet = pObj->GetRelativeWidthRelation();
                            break;
                        case MID_FRMSIZE_REL_HEIGHT_RELATION:
                            nRet = pObj->GetRelativeHeightRelation();
                            break;
                        }
                    }
                    aRet <<= nRet;
                }
                else
                {
                    const SwAttrSet& rSet = pFormat->GetAttrSet();
                    m_pPropSet->getPropertyValue(*pEntry, rSet, aRet);
                }
            }
            else
            {
                SfxPoolItem* pItem = nullptr;
                switch(pEntry->nWID)
                {
                    case RES_ANCHOR:
                        pItem = m_pImpl->GetAnchor();
                    break;
                    case RES_HORI_ORIENT:
                        pItem = m_pImpl->GetHOrient();
                    break;
                    case RES_VERT_ORIENT:
                        pItem = m_pImpl->GetVOrient();
                    break;
                    case  RES_LR_SPACE:
                        pItem = m_pImpl->GetLRSpace();
                    break;
                    case  RES_UL_SPACE:
                        pItem = m_pImpl->GetULSpace();
                    break;
                    case  RES_SURROUND:
                        pItem = m_pImpl->GetSurround();
                    break;
                    case FN_TEXT_RANGE :
                        aRet <<= m_pImpl->GetTextRange();
                    break;
                    case RES_OPAQUE :
                        aRet <<= m_pImpl->GetOpaque();
                    break;
                    case FN_ANCHOR_POSITION :
                    {
                        aRet <<= awt::Point();
                    }
                    break;
                    // #i26791#
                    case RES_FOLLOW_TEXT_FLOW :
                    {
                        pItem = m_pImpl->GetFollowTextFlow();
                    }
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = m_pImpl->GetWrapInfluenceOnObjPos();
                    }
                    break;
                    // #i28749#
                    case FN_SHAPE_TRANSFORMATION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::Transformation>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( "Transformation" );
                    }
                    break;
                    case FN_SHAPE_POSITION_LAYOUT_DIR:
                    {
                        aRet <<= m_pImpl->GetPositionLayoutDir();
                    }
                    break;
                    // #i36248#
                    case FN_SHAPE_STARTPOSITION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::StartPosition>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( "StartPosition" );
                    }
                    break;
                    case FN_SHAPE_ENDPOSITION_IN_HORI_L2R:
                    {
                        // get property <::drawing::Shape::StartPosition>
                        // without conversion to layout direction as below
                        aRet = _getPropAtAggrObj( "EndPosition" );
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
                aRet <<= ConvertTransformationToLayoutDir( aMatrix );
            }
            // #i36248#
            else if ( rPropertyName == "StartPosition" )
            {
                awt::Point aStartPos;
                aRet >>= aStartPos;
                // #i59051#
                aRet <<= ConvertStartOrEndPosToLayoutDir( aStartPos );
            }
            else if ( rPropertyName == "EndPosition" )
            {
                awt::Point aEndPos;
                aRet >>= aEndPos;
                // #i59051#
                aRet <<= ConvertStartOrEndPosToLayoutDir( aEndPos );
            }
            // #i59051#
            else if ( rPropertyName == "PolyPolygonBezier" )
            {
                drawing::PolyPolygonBezierCoords aPath;
                aRet >>= aPath;
                aRet <<= ConvertPolyPolygonBezierToLayoutDir( aPath );
            }
            else if (rPropertyName == "ZOrder")
            {
                // Convert the real draw page position to the logical one that ignores textboxes.
                if (pFormat)
                {
                    const SdrObject* pObj = pFormat->FindRealSdrObject();
                    if (pObj)
                    {
                        bool bConvert = true;
                        if (SvxShape* pSvxShape = GetSvxShape())
                            // In case of group shapes, pSvxShape points to the child shape, while pObj points to the outermost group shape.
                            if (pSvxShape->GetSdrObject() != pObj)
                                // Textboxes are not expected inside group shapes, so no conversion is necessary there.
                                bConvert = false;
                        if (bConvert)
                        {
                            aRet <<= SwTextBoxHelper::getOrdNum(pObj);
                        }
                    }
                }
            }
        }
    }
    return aRet;
}

uno::Any SwXShape::_getPropAtAggrObj( const OUString& _rPropertyName )
{
    uno::Any aRet;

    const uno::Type& rPSetType =
                cppu::UnoType<beans::XPropertySet>::get();
    uno::Any aPSet = m_xShapeAgg->queryAggregation(rPSetType);
    auto xPrSet = o3tl::tryAccess<uno::Reference<beans::XPropertySet>>(aPSet);
    if ( !xPrSet )
    {
        throw uno::RuntimeException();
    }
    aRet = (*xPrSet)->getPropertyValue( _rPropertyName );

    return aRet;
}

beans::PropertyState SwXShape::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXShape::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    uno::Sequence< beans::PropertyState > aRet(aPropertyNames.getLength());
    if(!m_xShapeAgg.is())
        throw uno::RuntimeException();

    SvxShape* pSvxShape = GetSvxShape();
    bool bGroupMember = false;
    bool bFormControl = false;
    SdrObject* pObject = pSvxShape ? pSvxShape->GetSdrObject() : nullptr;
    if(pObject)
    {
        bGroupMember = pObject->getParentSdrObjectFromSdrObject() != nullptr;
        bFormControl = pObject->GetObjInventor() == SdrInventor::FmForm;
    }
    const OUString* pNames = aPropertyNames.getConstArray();
    beans::PropertyState* pRet = aRet.getArray();
    uno::Reference< XPropertyState >  xShapePrState;
    for(sal_Int32 nProperty = 0; nProperty < aPropertyNames.getLength(); nProperty++)
    {
        const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( pNames[nProperty] );
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
            else if (pEntry->nWID == RES_FRM_SIZE &&
                     (pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT_RELATION ||
                      pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH_RELATION))
                pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
            else if (pEntry->nWID == FN_TEXT_BOX)
            {
                // The TextBox property is set, if we can find a textbox for this shape.
                if (pFormat
                    && SwTextBoxHelper::isTextBox(pFormat, RES_DRAWFRMFMT,
                                                  SdrObject::getSdrObjectFromXShape(mxShape)))
                    pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                else
                    pRet[nProperty] = beans::PropertyState_DEFAULT_VALUE;
            }
            else if(pFormat)
            {
                const SwAttrSet& rSet = pFormat->GetAttrSet();
                SfxItemState eItemState = rSet.GetItemState(pEntry->nWID, false);

                if(SfxItemState::SET == eItemState)
                    pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                else if(SfxItemState::DEFAULT == eItemState)
                    pRet[nProperty] = beans::PropertyState_DEFAULT_VALUE;
                else
                    pRet[nProperty] = beans::PropertyState_AMBIGUOUS_VALUE;
            }
            else
            {
                SfxPoolItem* pItem = nullptr;
                switch(pEntry->nWID)
                {
                    case RES_ANCHOR:
                        pItem = m_pImpl->GetAnchor();
                    break;
                    case RES_HORI_ORIENT:
                        pItem = m_pImpl->GetHOrient();
                    break;
                    case RES_VERT_ORIENT:
                        pItem = m_pImpl->GetVOrient();
                    break;
                    case  RES_LR_SPACE:
                        pItem = m_pImpl->GetLRSpace();
                    break;
                    case  RES_UL_SPACE:
                        pItem = m_pImpl->GetULSpace();
                    break;
                    case  RES_SURROUND:
                        pItem = m_pImpl->GetSurround();
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = m_pImpl->GetWrapInfluenceOnObjPos();
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
                const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
                uno::Any aPState = m_xShapeAgg->queryAggregation(rPStateType);
                auto ps = o3tl::tryAccess<uno::Reference<XPropertyState>>(
                    aPState);
                if(!ps)
                    throw uno::RuntimeException();
                xShapePrState = *ps;
            }
            pRet[nProperty] = xShapePrState->getPropertyState(pNames[nProperty]);
        }
    }

    return aRet;
}

void SwXShape::setPropertyToDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(!m_xShapeAgg.is())
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if(pEntry)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if(pFormat)
        {
            const SfxItemSet& rSet = pFormat->GetAttrSet();
            SfxItemSet aSet(pFormat->GetDoc()->GetAttrPool(), pEntry->nWID, pEntry->nWID);
            aSet.SetParent(&rSet);
            aSet.ClearItem(pEntry->nWID);
            pFormat->GetDoc()->SetAttr(aSet, *pFormat);
        }
        else
        {
            switch(pEntry->nWID)
            {
                case RES_ANCHOR:        m_pImpl->RemoveAnchor();  break;
                case RES_HORI_ORIENT:   m_pImpl->RemoveHOrient(); break;
                case RES_VERT_ORIENT:   m_pImpl->RemoveVOrient(); break;
                case  RES_LR_SPACE:     m_pImpl->RemoveLRSpace(); break;
                case  RES_UL_SPACE:     m_pImpl->RemoveULSpace(); break;
                case  RES_SURROUND:     m_pImpl->RemoveSurround();break;
                case RES_OPAQUE :       m_pImpl->SetOpaque(false);  break;
                case FN_TEXT_RANGE :
                break;
                // #i26791#
                case RES_FOLLOW_TEXT_FLOW:
                {
                    m_pImpl->RemoveFollowTextFlow();
                }
                break;
                // #i28701#
                case RES_WRAP_INFLUENCE_ON_OBJPOS:
                {
                    m_pImpl->RemoveWrapInfluenceOnObjPos();
                }
                break;
            }
        }
    }
    else
    {
        const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
        uno::Any aPState = m_xShapeAgg->queryAggregation(rPStateType);
        auto xShapePrState = o3tl::tryAccess<uno::Reference<XPropertyState>>(
            aPState);
        if(!xShapePrState)
            throw uno::RuntimeException();
        (*xShapePrState)->setPropertyToDefault( rPropertyName );
    }

}

uno::Any SwXShape::getPropertyDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    uno::Any aRet;
    if(!m_xShapeAgg.is())
        throw uno::RuntimeException();

    const SfxItemPropertyMapEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if(pEntry)
    {
        if(!(pEntry->nWID < RES_FRMATR_END && pFormat))
            throw uno::RuntimeException();

        const SfxPoolItem& rDefItem =
            pFormat->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
        rDefItem.QueryValue(aRet, pEntry->nMemberId);

    }
    else
    {
        const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
        uno::Any aPState = m_xShapeAgg->queryAggregation(rPStateType);
        auto xShapePrState = o3tl::tryAccess<uno::Reference<XPropertyState>>(
            aPState);
        if(!xShapePrState)
            throw uno::RuntimeException();
        (*xShapePrState)->getPropertyDefault( rPropertyName );
    }

    return aRet;
}

void SwXShape::addPropertyChangeListener(
    const OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener )
{
    if ( !m_xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( m_xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->addPropertyChangeListener( _propertyName, _listener );
}

void SwXShape::removePropertyChangeListener(
    const OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener)
{
    if ( !m_xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( m_xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->removePropertyChangeListener( _propertyName, _listener );
}

void SwXShape::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/ )
{
    OSL_FAIL("not implemented");
}

void SwXShape::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXShape::Notify(const SfxHint& rHint)
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_pFormat = nullptr;
        EndListeningAll();
    }
}

void SwXShape::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;

    // get access to SwDoc
    // (see also SwXTextRange::XTextRangeToSwPaM)
    const SwDoc* pDoc = nullptr;
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    if(xRangeTunnel.is())
    {
        if (auto pRange = comphelper::getFromUnoTunnel<SwXTextRange>(xRangeTunnel))
            pDoc = &pRange->GetDoc();
        else if (auto pText = comphelper::getFromUnoTunnel<SwXText>(xRangeTunnel))
            pDoc = pText->GetDoc();
        else if (auto pCursor = comphelper::getFromUnoTunnel<OTextCursorHelper>(xRangeTunnel))
            pDoc = pCursor->GetDoc();
        else if (auto pPortion = comphelper::getFromUnoTunnel<SwXTextPortion>(xRangeTunnel))
            pDoc = &pPortion->GetCursor().GetDoc();
        else if (auto pParagraph = comphelper::getFromUnoTunnel<SwXParagraph>(xRangeTunnel);
                 pParagraph && pParagraph->GetTextNode())
            pDoc = &pParagraph->GetTextNode()->GetDoc();

    }

    if(!pDoc)
        throw uno::RuntimeException();
    const SwDocShell* pDocSh = pDoc->GetDocShell();
    if (!pDocSh)
        return;

    uno::Reference<frame::XModel> xModel = pDocSh->GetModel();
    uno::Reference< drawing::XDrawPageSupplier > xDPS(xModel, uno::UNO_QUERY);
    if (xDPS.is())
    {
        uno::Reference< drawing::XDrawPage > xDP( xDPS->getDrawPage() );
        if (xDP.is())
        {
            uno::Any aPos;
            aPos <<= xTextRange;
            setPropertyValue("TextRange", aPos);
            uno::Reference< drawing::XShape > xTemp( static_cast<cppu::OWeakObject*>(this), uno::UNO_QUERY );
            xDP->add( xTemp );
        }
    }
}

uno::Reference< text::XTextRange >  SwXShape::getAnchor()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if ((rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PAGE) ||
            (rAnchor.GetContentAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(pFormat->GetAnchor().GetContentAnchor());
            if (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_PARA)
            {   // ensure that SwXTextRange has SwIndex
                aRef = SwXTextRange::CreateXTextRange(*pFormat->GetDoc(), SwPosition(rPos.nNode), nullptr);
            }
            else
            {
                aRef = SwXTextRange::CreateXTextRange(*pFormat->GetDoc(), rPos, nullptr);
            }
        }
    }
    else
        aRef = m_pImpl->GetTextRange();
    return aRef;
}

void SwXShape::dispose()
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        // determine correct <SdrObject>
        SvxShape* pSvxShape = GetSvxShape();
        SdrObject* pObj = pSvxShape ? pSvxShape->GetSdrObject() : nullptr;
        // safety assertion:
        // <pObj> must be the same as <pFormat->FindSdrObject()>, if <pObj> isn't
        // a 'virtual' drawing object.
        // correct assertion and refine it for safety reason.
        OSL_ENSURE( !pObj ||
                dynamic_cast<const SwDrawVirtObj*>( pObj) !=  nullptr ||
                pObj->getParentSdrObjectFromSdrObject() ||
                pObj == pFormat->FindSdrObject(),
                "<SwXShape::dispose(..) - different 'master' drawing objects!!" );
        // perform delete of draw frame format *not*
        // for 'virtual' drawing objects.
        // no delete of draw format for members
        // of a group
        if ( pObj &&
             dynamic_cast<const SwDrawVirtObj*>( pObj) ==  nullptr &&
             !pObj->getParentSdrObjectFromSdrObject() &&
             pObj->IsInserted() )
        {
            if (pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
            {
                const SwPosition &rPos = *(pFormat->GetAnchor().GetContentAnchor());
                SwTextNode *pTextNode = rPos.nNode.GetNode().GetTextNode();
                const sal_Int32 nIdx = rPos.nContent.GetIndex();
                pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx );
            }
            else
                pFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat( pFormat );
        }
    }
    if(m_xShapeAgg.is())
    {
        uno::Any aAgg(m_xShapeAgg->queryAggregation( cppu::UnoType<XComponent>::get()));
        uno::Reference<XComponent> xComp;
        aAgg >>= xComp;
        if(xComp.is())
            xComp->dispose();
    }
    if(m_pPage)
        const_cast<SwFmDrawPage*>(m_pPage)->RemoveShape(this);
    m_pPage = nullptr;
}

void SwXShape::addEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
         pSvxShape->addEventListener(aListener);
}

void SwXShape::removeEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
        pSvxShape->removeEventListener(aListener);
}

OUString SwXShape::getImplementationName()
{
    return "SwXShape";
}

sal_Bool SwXShape::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXShape::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSeq;
    if (SvxShape* pSvxShape = GetSvxShape())
        aSeq = pSvxShape->getSupportedServiceNames();
    return comphelper::concatSequences(
        aSeq, std::initializer_list<std::u16string_view>{ u"com.sun.star.drawing.Shape" });
}

SvxShape*   SwXShape::GetSvxShape()
{
    if(m_xShapeAgg.is())
        return comphelper::getFromUnoTunnel<SvxShape>(m_xShapeAgg);
    return nullptr;
}

// #i31698#
// implementation of virtual methods from drawing::XShape
awt::Point SAL_CALL SwXShape::getPosition()
{
    awt::Point aPos( GetAttrPosition() );

    // handle group members
    SvxShape* pSvxShape = GetSvxShape();
    if ( pSvxShape )
    {
        SdrObject* pTopGroupObj = GetTopGroupObj( pSvxShape );
        if ( pTopGroupObj )
        {
            // #i34750# - get attribute position of top group
            // shape and add offset between top group object and group member
            uno::Reference< drawing::XShape > xGroupShape( pTopGroupObj->getUnoShape(), uno::UNO_QUERY );
            aPos = xGroupShape->getPosition();
            // add offset between top group object and group member
            // to the determined attribute position
            // #i34750#:
            // consider the layout direction
            const tools::Rectangle aMemberObjRect = GetSvxShape()->GetSdrObject()->GetSnapRect();
            const tools::Rectangle aGroupObjRect = pTopGroupObj->GetSnapRect();
            // #i53320# - relative position of group member and
            // top group object is always given in horizontal left-to-right layout.
            awt::Point aOffset( 0, 0 );
            {
                aOffset.X = ( aMemberObjRect.Left() - aGroupObjRect.Left() );
                aOffset.Y = ( aMemberObjRect.Top() - aGroupObjRect.Top() );
            }
            aOffset.X = convertTwipToMm100(aOffset.X);
            aOffset.Y = convertTwipToMm100(aOffset.Y);
            aPos.X += aOffset.X;
            aPos.Y += aOffset.Y;
        }
    }

    return aPos;
}

void SAL_CALL SwXShape::setPosition( const awt::Point& aPosition )
{
    SdrObject* pTopGroupObj = GetTopGroupObj();
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
                         dynamic_cast<const SwDrawContact*>( pObj->GetUserCall()) !=  nullptr )
                    {
                        bNoAdjustOfPosProp = true;
                    }
                }
            }
        }
        // shape isn't a group member. Thus, set positioning attributes
        if ( !bNoAdjustOfPosProp )
        {
            AdjustPositionProperties( aPosition );
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
            aNewPos = ConvertPositionToHoriL2R( aNewPos, getSize() );
        }
        // Convert given absolute position in horizontal left-to-right
        // layout into relative position in horizontal left-to-right layout.
        uno::Reference< drawing::XShape > xGroupShape( pTopGroupObj->getUnoShape(), uno::UNO_QUERY );
        {
            // #i34750#
            // use method <xGroupShape->getPosition()> to get the correct
            // position of the top group object.
            awt::Point aAttrPosInHoriL2R(
                    ConvertPositionToHoriL2R( xGroupShape->getPosition(),
                                               xGroupShape->getSize() ) );
            aNewPos.X = o3tl::saturating_sub(aNewPos.X, aAttrPosInHoriL2R.X);
            aNewPos.Y = o3tl::saturating_sub(aNewPos.Y, aAttrPosInHoriL2R.Y);
        }
        // convert relative position in horizontal left-to-right layout into
        // absolute position in horizontal left-to-right layout
        {
            // #i34750#
            // use method <SvxShape->getPosition()> to get the correct
            // 'Drawing layer' position of the top group shape.
            auto pSvxGroupShape = comphelper::getFromUnoTunnel<SvxShape>(pTopGroupObj->getUnoShape());
            const awt::Point aGroupPos = pSvxGroupShape->getPosition();
            aNewPos.X = o3tl::saturating_add(aNewPos.X, aGroupPos.X);
            aNewPos.Y = o3tl::saturating_add(aNewPos.Y, aGroupPos.Y);
        }
        // set position
        mxShape->setPosition( aNewPos );
    }
}

awt::Size SAL_CALL SwXShape::getSize()
{
    awt::Size aSize;
    if ( mxShape.is() )
    {
        aSize = mxShape->getSize();
    }
    return aSize;
}

void SAL_CALL SwXShape::setSize( const awt::Size& aSize )
{
    comphelper::ProfileZone aZone("SwXShape::setSize");

    if ( mxShape.is() )
    {
        mxShape->setSize( aSize );
    }
    SwTextBoxHelper::syncProperty(GetFrameFormat(), RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(aSize));
}
// #i31698#
// implementation of virtual methods from drawing::XShapeDescriptor
OUString SAL_CALL SwXShape::getShapeType()
{
    if ( mxShape.is() )
    {
        return mxShape->getShapeType();
    }
    return OUString();
}
/** method to determine top group object
    #i31698#
*/
SdrObject* SwXShape::GetTopGroupObj( SvxShape* _pSvxShape )
{
    SdrObject* pTopGroupObj( nullptr );

    SvxShape* pSvxShape = _pSvxShape ? _pSvxShape : GetSvxShape();
    if ( pSvxShape )
    {
        SdrObject* pSdrObj = pSvxShape->GetSdrObject();
        if ( pSdrObj && pSdrObj->getParentSdrObjectFromSdrObject() )
        {
            pTopGroupObj = pSdrObj->getParentSdrObjectFromSdrObject();
            while ( pTopGroupObj->getParentSdrObjectFromSdrObject() )
            {
                pTopGroupObj = pTopGroupObj->getParentSdrObjectFromSdrObject();
            }
        }
    }

    return pTopGroupObj;
}

/** method to determine position according to the positioning attributes
    #i31698#
*/
awt::Point SwXShape::GetAttrPosition()
{
    awt::Point aAttrPos;

    uno::Any aHoriPos( getPropertyValue("HoriOrientPosition") );
    aHoriPos >>= aAttrPos.X;
    uno::Any aVertPos( getPropertyValue("VertOrientPosition") );
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
            const tools::Rectangle aObjRect = pObj->GetSnapRect();
            aAttrPos.X = convertTwipToMm100(aObjRect.Left());
            aAttrPos.Y = convertTwipToMm100(aObjRect.Top());
        }
    }
    // #i35007# - If drawing object is anchored as-character,
    // it's x-position isn't sensible. Thus, return the x-position as zero in this case.
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        uno::Any aAny = getPropertyValue( "AnchorType" );
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
awt::Point SwXShape::ConvertPositionToHoriL2R( const awt::Point& rObjPos,
                                                const awt::Size& rObjSize )
{
    awt::Point aObjPosInHoriL2R( rObjPos );

    SwFrameFormat* pFrameFormat = GetFrameFormat();
    if ( pFrameFormat )
    {
        SwFrameFormat::tLayoutDir eLayoutDir = pFrameFormat->GetLayoutDir();
        switch ( eLayoutDir )
        {
            case SwFrameFormat::HORI_L2R:
            {
                // nothing to do
            }
            break;
            case SwFrameFormat::HORI_R2L:
            {
                aObjPosInHoriL2R.X = -rObjPos.X - rObjSize.Width;
            }
            break;
            case SwFrameFormat::VERT_R2L:
            {
                aObjPosInHoriL2R.X = -rObjPos.Y - rObjSize.Width;
                aObjPosInHoriL2R.Y = rObjPos.X;
            }
            break;
            default:
            {
                OSL_FAIL( "<SwXShape::ConvertPositionToHoriL2R(..)> - unsupported layout direction" );
            }
        }
    }

    return aObjPosInHoriL2R;
}

/** method to convert the transformation of the drawing object to the layout
    direction, the drawing object is in
    #i31698#
*/
drawing::HomogenMatrix3 SwXShape::ConvertTransformationToLayoutDir(
                                    const drawing::HomogenMatrix3& rMatrixInHoriL2R )
{
    drawing::HomogenMatrix3 aMatrix(rMatrixInHoriL2R);

    // #i44334#, #i44681# - direct manipulation of the
    // transformation structure isn't valid, if it contains rotation.
    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::ConvertTransformationToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::ConvertTransformationToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    convertTwipToMm100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    convertTwipToMm100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
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
void SwXShape::AdjustPositionProperties( const awt::Point& rPosition )
{
    // handle x-position
    // #i35007# - no handling of x-position, if drawing
    // object is anchored as-character, because it doesn't make sense.
    text::TextContentAnchorType eTextAnchorType =
                            text::TextContentAnchorType_AT_PARAGRAPH;
    {
        uno::Any aAny = getPropertyValue( "AnchorType" );
        aAny >>= eTextAnchorType;
    }
    if ( eTextAnchorType != text::TextContentAnchorType_AS_CHARACTER )
    {
        // determine current x-position
        static const OUStringLiteral aHoriPosPropStr(u"HoriOrientPosition");
        uno::Any aHoriPos( getPropertyValue( aHoriPosPropStr ) );
        sal_Int32 dCurrX = 0;
        aHoriPos >>= dCurrX;
        // change x-position attribute, if needed
        if ( dCurrX != rPosition.X )
        {
            // adjust x-position orientation to text::HoriOrientation::NONE, if needed
            // Note: has to be done before setting x-position attribute
            static const OUStringLiteral aHoriOrientPropStr(u"HoriOrient");
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
            aHoriPos <<= rPosition.X;
            setPropertyValue( aHoriPosPropStr, aHoriPos );
        }
    }

    // handle y-position
    {
        // determine current y-position
        static const OUStringLiteral aVertPosPropStr(u"VertOrientPosition");
        uno::Any aVertPos( getPropertyValue( aVertPosPropStr ) );
        sal_Int32 dCurrY = 0;
        aVertPos >>= dCurrY;
        // change y-position attribute, if needed
        if ( dCurrY != rPosition.Y )
        {
            // adjust y-position orientation to text::VertOrientation::NONE, if needed
            // Note: has to be done before setting y-position attribute
            static const OUStringLiteral aVertOrientPropStr(u"VertOrient");
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
            aVertPos <<= rPosition.Y;
            setPropertyValue( aVertPosPropStr, aVertPos );
        }
    }
}

/** method to convert start or end position of the drawing object to the
    Writer specific position, which is the attribute position in layout direction
    #i59051#
*/
css::awt::Point SwXShape::ConvertStartOrEndPosToLayoutDir(
                            const css::awt::Point& aStartOrEndPos )
{
    awt::Point aConvertedPos( aStartOrEndPos );

    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::ConvertStartOrEndPosToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::ConvertStartOrEndPosToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    convertTwipToMm100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    convertTwipToMm100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
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

css::drawing::PolyPolygonBezierCoords SwXShape::ConvertPolyPolygonBezierToLayoutDir(
                    const css::drawing::PolyPolygonBezierCoords& aPath )
{
    drawing::PolyPolygonBezierCoords aConvertedPath( aPath );

    SvxShape* pSvxShape = GetSvxShape();
    OSL_ENSURE( pSvxShape,
            "<SwXShape::ConvertStartOrEndPosToLayoutDir(..)> - no SvxShape found!");
    if ( pSvxShape )
    {
        const SdrObject* pObj = pSvxShape->GetSdrObject();
        OSL_ENSURE( pObj,
                "<SwXShape::ConvertStartOrEndPosToLayoutDir(..)> - no SdrObject found!");
        if ( pObj )
        {
            // get position of object in Writer coordinate system.
            awt::Point aPos( getPosition() );
            // get position of object in Drawing layer coordinate system
            const Point aTmpObjPos( pObj->GetSnapRect().TopLeft() );
            const awt::Point aObjPos(
                    convertTwipToMm100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    convertTwipToMm100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
            // determine difference between these positions according to the
            // Writer coordinate system
            const awt::Point aTranslateDiff( aPos.X - aObjPos.X,
                                             aPos.Y - aObjPos.Y );
            // apply translation difference to PolyPolygonBezier.
            if ( aTranslateDiff.X != 0 || aTranslateDiff.Y != 0 )
            {
                const basegfx::B2DHomMatrix aMatrix(basegfx::utils::createTranslateB2DHomMatrix(
                    aTranslateDiff.X, aTranslateDiff.Y));

                for(drawing::PointSequence& rInnerSequence : asNonConstRange(aConvertedPath.Coordinates))
                {
                    for(awt::Point& rPoint : asNonConstRange(rInnerSequence))
                    {
                        basegfx::B2DPoint aNewCoordinatePair(rPoint.X, rPoint.Y);
                        aNewCoordinatePair *= aMatrix;
                        rPoint.X = basegfx::fround(aNewCoordinatePair.getX());
                        rPoint.Y = basegfx::fround(aNewCoordinatePair.getY());
                    }
                }
            }
        }
    }

    return aConvertedPath;
}

SwXGroupShape::SwXGroupShape(uno::Reference<XInterface> & xShape,
                             SwDoc const*const pDoc)
    : SwXShape(xShape, pDoc)
{
#if OSL_DEBUG_LEVEL > 0
    uno::Reference<XShapes> xShapes(m_xShapeAgg, uno::UNO_QUERY);
    OSL_ENSURE(xShapes.is(), "no SvxShape found or shape is not a group shape");
#endif
}

SwXGroupShape::~SwXGroupShape()
{
}

uno::Any SwXGroupShape::queryInterface( const uno::Type& rType )
{
    uno::Any aRet;
    if(rType == cppu::UnoType<XShapes>::get())
        aRet <<= uno::Reference<XShapes>(this);
    else
        aRet = SwXShape::queryInterface(rType);
    return aRet;
}

void SwXGroupShape::acquire(  ) noexcept
{
    SwXShape::acquire();
}

void SwXGroupShape::release(  ) noexcept
{
    SwXShape::release();
}

void SwXGroupShape::add( const uno::Reference< XShape >& xShape )
{
    SolarMutexGuard aGuard;
    SvxShape* pSvxShape = GetSvxShape();
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!(pSvxShape && pFormat))
        throw uno::RuntimeException();

    uno::Reference<XShapes> xShapes;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XShapes>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xShapes;
    }
    if(!xShapes.is())
        throw uno::RuntimeException();

    xShapes->add(xShape);


    uno::Reference<lang::XUnoTunnel> xTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pSwShape = comphelper::getFromUnoTunnel<SwXShape>(xTunnel);
    if(!(pSwShape && pSwShape->m_bDescriptor))
        return;

    SvxShape* pAddShape = comphelper::getFromUnoTunnel<SvxShape>(xTunnel);
    if(pAddShape)
    {
        SdrObject* pObj = pAddShape->GetSdrObject();
        if(pObj)
        {
            SwDoc* pDoc = pFormat->GetDoc();
            // set layer of new drawing
            // object to corresponding invisible layer.
            if( SdrInventor::FmForm != pObj->GetObjInventor())
            {
                pObj->SetLayer( pSwShape->m_pImpl->GetOpaque()
                                ? pDoc->getIDocumentDrawModelAccess().GetInvisibleHeavenId()
                                : pDoc->getIDocumentDrawModelAccess().GetInvisibleHellId() );
            }
            else
            {
                pObj->SetLayer(pDoc->getIDocumentDrawModelAccess().GetInvisibleControlsId());
            }
        }
    }
    pSwShape->m_bDescriptor = false;
    //add the group member to the format of the group
    SwFrameFormat* pShapeFormat = ::FindFrameFormat( pSvxShape->GetSdrObject() );

    if(pShapeFormat)
        pSwShape->SetFrameFormat(pShapeFormat);

}

void SwXGroupShape::remove( const uno::Reference< XShape >& xShape )
{
    SolarMutexGuard aGuard;
    uno::Reference<XShapes> xShapes;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XShapes>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xShapes;
    }
    if(!xShapes.is())
        throw uno::RuntimeException();
    xShapes->remove(xShape);
}

sal_Int32 SwXGroupShape::getCount()
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getCount();
}

uno::Any SwXGroupShape::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getByIndex(nIndex);
}

uno::Type SwXGroupShape::getElementType(  )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getElementType();
}

sal_Bool SwXGroupShape::hasElements(  )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( m_xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = m_xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
