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

#include <cmdid.h>

#include <drawdoc.hxx>
#include <unodraw.hxx>
#include <unocoll.hxx>
#include <unoframe.hxx>
#include <unoparagraph.hxx>
#include <unotextrange.hxx>
#include <svx/svditer.hxx>
#include <swunohelper.hxx>
#include <textboxhelper.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <cppuhelper/supportsservice.hxx>
#include <svx/scene3d.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <calbck.hxx>

using namespace ::com::sun::star;

class SwShapeDescriptor_Impl
{
    SwFormatHoriOrient*    pHOrient;
    SwFormatVertOrient*    pVOrient;
    SwFormatAnchor*        pAnchor;
    SwFormatSurround*      pSurround;
    SvxULSpaceItem*     pULSpace;
    SvxLRSpaceItem*     pLRSpace;
    bool            bOpaque;
    uno::Reference< text::XTextRange > xTextRange;
    // #i26791#
    SwFormatFollowTextFlow* mpFollowTextFlow;
    // #i28701#
    SwFormatWrapInfluenceOnObjPos* pWrapInfluenceOnObjPos;
    // #i28749#
    sal_Int16 mnPositionLayoutDir;

public:
    bool    bInitializedPropertyNotifier;

public:
    SwShapeDescriptor_Impl() :
     // #i32349# - no defaults, in order to determine on
     // adding a shape, if positioning attributes are set or not.
     pHOrient( nullptr ),
     pVOrient( nullptr ),
     pAnchor(nullptr),
     pSurround(nullptr),
     pULSpace(nullptr),
     pLRSpace(nullptr),
     bOpaque(false),
     // #i26791#
     mpFollowTextFlow( new SwFormatFollowTextFlow( false ) ),
     // #i28701# #i35017#
     pWrapInfluenceOnObjPos( new SwFormatWrapInfluenceOnObjPos(
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
    SwFormatAnchor*    GetAnchor(bool bCreate = false)
        {
            if(bCreate && !pAnchor)
            {
                pAnchor = new SwFormatAnchor(FLY_AS_CHAR);
            }
            return pAnchor;
        }
    SwFormatHoriOrient* GetHOrient(bool bCreate = false)
        {
            if (bCreate && !pHOrient)
            {
                // #i26791#
                pHOrient = new SwFormatHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME );
            }
            return pHOrient;
        }
    SwFormatVertOrient* GetVOrient(bool bCreate = false)
        {
            if(bCreate && !pVOrient)
            {
                // #i26791#
                pVOrient = new SwFormatVertOrient( 0, text::VertOrientation::NONE, text::RelOrientation::FRAME );
            }
            return pVOrient;
        }

    SwFormatSurround*  GetSurround(bool bCreate = false)
        {
            if(bCreate && !pSurround)
                pSurround = new SwFormatSurround();
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
    bool    IsOpaque() const
        {
            return bOpaque;
        }
    const bool&    GetOpaque() const
        {
            return bOpaque;
        }
    void RemoveHOrient(){DELETEZ(pHOrient);}
    void RemoveVOrient(){DELETEZ(pVOrient);}
    void RemoveAnchor(){DELETEZ(pAnchor);}
    void RemoveSurround(){DELETEZ(pSurround);}
    void RemoveULSpace(){DELETEZ(pULSpace);}
    void RemoveLRSpace(){DELETEZ(pLRSpace);}
    void SetOpaque(bool bSet){bOpaque = bSet;}

    // #i26791#
    SwFormatFollowTextFlow* GetFollowTextFlow( bool _bCreate = false )
    {
        if ( _bCreate && !mpFollowTextFlow )
            mpFollowTextFlow = new SwFormatFollowTextFlow( false );
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

    // #i28701#
    inline SwFormatWrapInfluenceOnObjPos* GetWrapInfluenceOnObjPos(
                                        const bool _bCreate = false )
    {
        if ( _bCreate && !pWrapInfluenceOnObjPos )
        {
            pWrapInfluenceOnObjPos = new SwFormatWrapInfluenceOnObjPos(
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

SwFmDrawPage::SwFmDrawPage( SdrPage* pPage ) :
    SvxFmDrawPage( pPage ), pPageView(nullptr)
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

void SwFmDrawPage::PreUnGroup(const uno::Reference< drawing::XShapeGroup >&  rShapeGroup)
{
    uno::Reference< drawing::XShape >  xShape( rShapeGroup, uno::UNO_QUERY);
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
    pPageView = nullptr;
}

uno::Reference< uno::XInterface >   SwFmDrawPage::GetInterface( SdrObject* pObj )
{
    uno::Reference< XInterface >  xShape;
    if( pObj )
    {
        SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
        SwXShape* pxShape = SwIterator<SwXShape,SwFormat>( *pFormat ).First();
        if(pxShape)
        {
            xShape =  *static_cast<cppu::OWeakObject*>(pxShape);
        }
        else
            xShape = pObj->getUnoShape();
    }
    return xShape;
}

SdrObject* SwFmDrawPage::_CreateSdrObject( const uno::Reference< drawing::XShape > & xShape )
    throw (uno::RuntimeException, std::exception)
{
    //FIXME: just a redirect call - can this method be deleted?
    return SvxFmDrawPage::_CreateSdrObject( xShape );
}

uno::Reference< drawing::XShape > SwFmDrawPage::_CreateShape( SdrObject *pObj ) const
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< drawing::XShape >  xRet;
    if(dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr || pObj->GetObjInventor() == SWGInventor)
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
                OSL_FAIL( "<SwFmDrawPage::_CreateShape(..)> - could not retrieve type. Thus, no shape created." );
                return xRet;
            }
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
        SwXShape* pShape = nullptr;
        if(xShapeTunnel.is())
            pShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SwXShape::getUnoTunnelId()) ));
        if(!pShape)
        {
            xShapeTunnel = nullptr;
            uno::Reference< uno::XInterface > xCreate(xRet, uno::UNO_QUERY);
            xRet = nullptr;
            uno::Reference< beans::XPropertySet >  xPrSet;
            if ( pObj->IsGroupObject() && (!pObj->Is3DObj() || (dynamic_cast<const E3dScene*>( pObj) !=  nullptr)) )
                xPrSet = new SwXGroupShape( xCreate );
            else
                xPrSet = new SwXShape( xCreate );
            xRet.set(xPrSet, uno::UNO_QUERY);
        }
    }
    return xRet;
}

namespace
{
    class SwXShapesEnumeration
        : public SwSimpleEnumeration_Base
    {
        private:
            typedef ::std::list< css::uno::Any > shapescontainer_t;
            shapescontainer_t m_aShapes;
        protected:
            virtual ~SwXShapesEnumeration() {};
        public:
            explicit SwXShapesEnumeration(SwXDrawPage* const pDrawPage);

            //XEnumeration
            virtual sal_Bool SAL_CALL hasMoreElements() throw(uno::RuntimeException, std::exception) override;
            virtual uno::Any SAL_CALL nextElement() throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;

            //XServiceInfo
            virtual OUString SAL_CALL getImplementationName() throw(uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(uno::RuntimeException, std::exception) override;
            virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(uno::RuntimeException, std::exception) override;
    };
}

SwXShapesEnumeration::SwXShapesEnumeration(SwXDrawPage* const pDrawPage)
    : m_aShapes()
{
    SolarMutexGuard aGuard;
    ::std::insert_iterator<shapescontainer_t> pInserter = ::std::insert_iterator<shapescontainer_t>(m_aShapes, m_aShapes.begin());
    sal_Int32 nCount = pDrawPage->getCount();
    std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDrawPage->GetDoc());
    for(sal_Int32 nIdx = 0; nIdx < nCount; nIdx++)
    {
        uno::Reference<drawing::XShape> xShape(pDrawPage->getByIndex(nIdx, &aTextBoxes), uno::UNO_QUERY);
        *pInserter++ = uno::makeAny(xShape);
    }
}

sal_Bool SwXShapesEnumeration::hasMoreElements() throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return !m_aShapes.empty();
}

uno::Any SwXShapesEnumeration::nextElement() throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(m_aShapes.empty())
        throw container::NoSuchElementException();
    uno::Any aResult = *m_aShapes.begin();
    m_aShapes.pop_front();
    return aResult;
}

OUString SwXShapesEnumeration::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("SwXShapeEnumeration");
}

sal_Bool SwXShapesEnumeration::supportsService(const OUString& ServiceName) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SwXShapesEnumeration::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return { OUString("com.sun.star.container.XEnumeration") };
}

uno::Reference< container::XEnumeration > SwXDrawPage::createEnumeration() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return uno::Reference< container::XEnumeration >(
        new SwXShapesEnumeration(this));
}

OUString SwXDrawPage::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXDrawPage");
}

sal_Bool SwXDrawPage::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXDrawPage::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence<OUString> aRet { "com.sun.star.drawing.GenericDrawPage" };
    return aRet;
}

SwXDrawPage::SwXDrawPage(SwDoc* pDc) :
    pDoc(pDc),
    pDrawPage(nullptr)
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
                                                throw( uno::RuntimeException, std::exception )
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

uno::Sequence< uno::Type > SwXDrawPage::getTypes() throw( uno::RuntimeException, std::exception )
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
    pPageTypes[nIndex] = cppu::UnoType<form::XFormsSupplier2>::get();
    return aPageTypes;
}

sal_Int32 SwXDrawPage::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        return 0;
    else
    {
        static_cast<SwXDrawPage*>(this)->GetSvxPage();

        std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);

        if (aTextBoxes.empty())
            return pDrawPage->getCount();
        else
            return SwTextBoxHelper::getCount(pDrawPage->GetSdrPage(), aTextBoxes);
    }
}

uno::Any SwXDrawPage::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
               uno::RuntimeException, std::exception )
{
    return getByIndex(nIndex, nullptr);
}

uno::Any SwXDrawPage::getByIndex(sal_Int32 nIndex, std::set<const SwFrameFormat*>* pTextBoxes)
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        throw lang::IndexOutOfBoundsException();

    static_cast<SwXDrawPage*>(this)->GetSvxPage();
    std::set<const SwFrameFormat*> aTextBoxes;
    if (!pTextBoxes)
    {
        // We got no set, so let's generate one.
        aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
        pTextBoxes = &aTextBoxes;
    }
    if (pTextBoxes->empty())
        return pDrawPage->getByIndex( nIndex );
    else
        return SwTextBoxHelper::getByIndex(pDrawPage->GetSdrPage(), nIndex, *pTextBoxes);
}

uno::Type  SwXDrawPage::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SwXDrawPage::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(!pDoc->getIDocumentDrawModelAccess().GetDrawModel())
        return sal_False;
    else
        return static_cast<SwXDrawPage*>(this)->GetSvxPage()->hasElements();
}

void SwXDrawPage::add(const uno::Reference< drawing::XShape > & xShape)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShape, uno::UNO_QUERY);
    SwXShape* pShape = nullptr;
    SvxShape* pSvxShape = nullptr;
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

    OSL_ENSURE(pSvxShape, "Why is here no SvxShape?");
    // this position is definitely in 1/100 mm
    awt::Point aMM100Pos(pSvxShape->getPosition());

    // now evaluate the properties of SwShapeDescriptor_Impl
    SwShapeDescriptor_Impl* pDesc = pShape->GetDescImpl();

    SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                        RES_FRMATR_END-1 );
    SwFormatAnchor aAnchor( FLY_AS_CHAR );
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
            SwTwips nHoriPos = convertMm100ToTwip(aMM100Pos.X);
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
            SwTwips nVertPos = convertMm100ToTwip(aMM100Pos.Y);
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
    if(FmFormInventor != pObj->GetObjInventor())
        pObj->SetLayer( bOpaque ? pDoc->getIDocumentDrawModelAccess().GetInvisibleHeavenId() : pDoc->getIDocumentDrawModelAccess().GetInvisibleHellId() );
    else
        pObj->SetLayer(pDoc->getIDocumentDrawModelAccess().GetInvisibleControlsId());

    SwPaM* pPam = new SwPaM(pDoc->GetNodes().GetEndOfContent());
    SwUnoInternalPaM* pInternalPam = nullptr;
    uno::Reference< text::XTextRange >  xRg;
    if( pDesc && (xRg = pDesc->GetTextRange()).is() )
    {
        pInternalPam = new SwUnoInternalPaM(*pDoc);
        if (::sw::XTextRangeToSwPaM(*pInternalPam, xRg))
        {
            if(FLY_AT_FLY == aAnchor.GetAnchorId() &&
                                !pInternalPam->GetNode().FindFlyStartNode())
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
    else if ((aAnchor.GetAnchorId() != FLY_AT_PAGE) && pDoc->getIDocumentLayoutAccess().GetCurrentLayout())
    {
        SwCursorMoveState aState( MV_SETONLYTEXT );
        Point aTmp(convertMm100ToTwip(aMM100Pos.X), convertMm100ToTwip(aMM100Pos.Y));
        pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetCursorOfst( pPam->GetPoint(), aTmp, &aState );
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
    pDoc->getIDocumentContentOperations().InsertDrawObj( *pTemp, *pObj, aSet );
    SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
    if(pFormat)
        pFormat->Add(pShape);
    pShape->m_bDescriptor = false;

    delete pPam;
    delete pInternalPam;
}

void SwXDrawPage::remove(const uno::Reference< drawing::XShape > & xShape) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(xShape, uno::UNO_QUERY);
    xComp->dispose();
}

uno::Reference< drawing::XShapeGroup >  SwXDrawPage::group(const uno::Reference< drawing::XShapes > & xShapes)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if(!pDoc || !xShapes.is())
        throw uno::RuntimeException();
    uno::Reference< drawing::XShapeGroup >  xRet;
    if(xPageAgg.is())
    {

        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage) //TODO: can this be Null?
        {
            // mark and return MarkList
            const SdrMarkList& rMarkList = pPage->PreGroup(xShapes);
            if ( rMarkList.GetMarkCount() > 1 )
            {
                bool bFlyInCnt = false;
                for ( size_t i = 0; !bFlyInCnt && i < rMarkList.GetMarkCount(); ++i )
                {
                    const SdrObject *pObj = rMarkList.GetMark( i )->GetMarkedSdrObj();
                    if (FLY_AS_CHAR == ::FindFrameFormat(const_cast<SdrObject*>(
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
                    pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );

                    SwDrawContact* pContact = pDoc->GroupSelection( *pPage->GetDrawView() );
                    pDoc->ChgAnchor(
                        pPage->GetDrawView()->GetMarkedObjectList(),
                        FLY_AT_PARA,
                        true, false );

                    pPage->GetDrawView()->UnmarkAll();
                    if(pContact)
                    {
                        uno::Reference< uno::XInterface >  xInt = SwFmDrawPage::GetInterface( pContact->GetMaster() );
                        xRet.set(xInt, uno::UNO_QUERY);
                    }
                    pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
                }
            }
            pPage->RemovePageView();
        }
    }
    return xRet;
}

void SwXDrawPage::ungroup(const uno::Reference< drawing::XShapeGroup > & rShapeGroup) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pDoc)
        throw uno::RuntimeException();
    if(xPageAgg.is())
    {
        SwFmDrawPage* pPage = GetSvxPage();
        if(pPage) //TODO: can this be Null?
        {
            pPage->PreUnGroup(rShapeGroup);
            UnoActionContext aContext(pDoc);
            pDoc->GetIDocumentUndoRedo().StartUndo( UNDO_START, nullptr );

            pDoc->UnGroupSelection( *pPage->GetDrawView() );
            pDoc->ChgAnchor( pPage->GetDrawView()->GetMarkedObjectList(),
                        FLY_AT_PARA,
                        true, false );
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, nullptr );
            pPage->RemovePageView();
        }
    }
}

SwFmDrawPage*   SwXDrawPage::GetSvxPage()
{
    if(!xPageAgg.is() && pDoc)
    {
        SolarMutexGuard aGuard;
        // #i52858#
        SwDrawModel* pModel = pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        SdrPage* pPage = pModel->GetPage( 0 );

        {
            // We need a Ref to the object during queryInterface or else
            // it will be deleted
            pDrawPage = new SwFmDrawPage(pPage);
            uno::Reference< drawing::XDrawPage >  xPage = pDrawPage;
            uno::Any aAgg = xPage->queryInterface(cppu::UnoType<uno::XAggregation>::get());
            if(aAgg.getValueType() == cppu::UnoType<uno::XAggregation>::get())
                xPageAgg = *static_cast<uno::Reference< uno::XAggregation > const *>(aAgg.getValue());
        }
        if( xPageAgg.is() )
            xPageAgg->setDelegator( static_cast<cppu::OWeakObject*>(this) );
    }
    return pDrawPage;
}

/**
 * Renamed and outlined to detect where it's called
 */
void SwXDrawPage::InvalidateSwDoc()
{
    pDoc = nullptr;
}

SwDoc* SwXDrawPage::GetDoc()
{
    return pDoc;
}


namespace
{
    class theSwXShapeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXShapeUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXShape::getUnoTunnelId()
{
    return theSwXShapeUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXShape::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }

    if( xShapeAgg.is() )
    {
        const uno::Type& rTunnelType = cppu::UnoType<lang::XUnoTunnel>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rTunnelType );
        if(aAgg.getValueType() == rTunnelType)
        {
            uno::Reference<lang::XUnoTunnel> xAggTunnel =
                    *static_cast<uno::Reference<lang::XUnoTunnel> const *>(aAgg.getValue());
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
        svx::PPropertyValueProvider pProvider( new svx::PropertyValueProvider( _rShape, "AnchorType" ) );
        _rObj.getShapePropertyChangeNotifier().registerProvider( svx::eTextShapeAnchorType, pProvider );
    }
}

SwXShape::SwXShape(uno::Reference< uno::XInterface > & xShape) :
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_SHAPE)),
    m_pPropertyMapEntries(aSwMapProvider.GetPropertyMapEntries(PROPERTY_MAP_TEXT_SHAPE)),
    pImpl(new SwShapeDescriptor_Impl()),
    m_bDescriptor(true)
{
    if(xShape.is())  // default Ctor
    {
        const uno::Type& rAggType = cppu::UnoType<uno::XAggregation>::get();
        //aAgg contains a reference of the SvxShape!
        {
            uno::Any aAgg = xShape->queryInterface(rAggType);
            if(aAgg.getValueType() == rAggType)
                xShapeAgg = *static_cast<uno::Reference< uno::XAggregation > const *>(aAgg.getValue());
            // #i31698#
            if ( xShapeAgg.is() )
            {
                xShapeAgg->queryAggregation( cppu::UnoType<drawing::XShape>::get()) >>= mxShape;
                OSL_ENSURE( mxShape.is(),
                        "<SwXShape::SwXShape(..)> - no XShape found at <xShapeAgg>" );
            }
        }
        xShape = nullptr;
        m_refCount++;
        if( xShapeAgg.is() )
            xShapeAgg->setDelegator( static_cast<cppu::OWeakObject*>(this) );
        m_refCount--;

        uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
        SvxShape* pShape = nullptr;
        if(xShapeTunnel.is())
            pShape = reinterpret_cast< SvxShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething(SvxShape::getUnoTunnelId()) ));

        SdrObject* pObj = pShape ? pShape->GetSdrObject() : nullptr;
        if(pObj)
        {
            SwFrameFormat* pFormat = ::FindFrameFormat( pObj );
            if(pFormat)
                pFormat->Add(this);

            lcl_addShapePropertyEventFactories( *pObj, *this );
            pImpl->bInitializedPropertyNotifier = true;
        }
    }
}

void SwXShape::AddExistingShapeToFormat( SdrObject& _rObj )
{
    SdrObjListIter aIter( _rObj, IM_DEEPNOGROUPS );
    while ( aIter.IsMore() )
    {
        SdrObject* pCurrent = aIter.Next();
        OSL_ENSURE( pCurrent, "SwXShape::AddExistingShapeToFormat: invalid object list element!" );
        if ( !pCurrent )
            continue;

        SwXShape* pSwShape = nullptr;
        uno::Reference< lang::XUnoTunnel > xShapeTunnel( pCurrent->getWeakUnoShape(), uno::UNO_QUERY );
        if ( xShapeTunnel.is() )
            pSwShape = reinterpret_cast< SwXShape * >(
                    sal::static_int_cast< sal_IntPtr >( xShapeTunnel->getSomething( SwXShape::getUnoTunnelId() ) ) );
        if ( pSwShape )
        {
            if ( pSwShape->m_bDescriptor )
            {
                SwFrameFormat* pFormat = ::FindFrameFormat( pCurrent );
                if ( pFormat )
                    pFormat->Add( pSwShape );
                pSwShape->m_bDescriptor = false;
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
    SolarMutexGuard aGuard;
    if (xShapeAgg.is())
    {
        uno::Reference< uno::XInterface >  xRef;
        xShapeAgg->setDelegator(xRef);
    }
    delete pImpl;
    if(GetRegisteredIn())
        GetRegisteredIn()->Remove(this);
}

uno::Any SwXShape::queryInterface( const uno::Type& aType ) throw( uno::RuntimeException, std::exception )
{
    uno::Any aRet = SwTextBoxHelper::queryInterface(GetFrameFormat(), aType);
    if (aRet.hasValue())
        return aRet;

    aRet = SwXShapeBaseClass::queryInterface(aType);
    // #i53320# - follow-up of #i31698#
    // interface drawing::XShape is overloaded. Thus, provide
    // correct object instance.
    if(!aRet.hasValue() && xShapeAgg.is())
    {
        if(aType == cppu::UnoType<XShape>::get())
            aRet <<= uno::Reference<XShape>(this);
        else
            aRet = xShapeAgg->queryAggregation(aType);
    }
    return aRet;
}

uno::Sequence< uno::Type > SwXShape::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aRet = SwXShapeBaseClass::getTypes();
    if(xShapeAgg.is())
    {
        uno::Any aProv = xShapeAgg->queryAggregation(cppu::UnoType<XTypeProvider>::get());
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

uno::Sequence< sal_Int8 > SwXShape::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference< beans::XPropertySetInfo >  SwXShape::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< beans::XPropertySetInfo >  aRet;
    if(xShapeAgg.is())
    {
        const uno::Type& rPropSetType = cppu::UnoType<beans::XPropertySet>::get();
        uno::Any aPSet = xShapeAgg->queryAggregation( rPropSetType );
        if(aPSet.getValueType() == rPropSetType && aPSet.getValue())
        {
            uno::Reference< beans::XPropertySet >  xPrSet =
                    *static_cast<uno::Reference< beans::XPropertySet > const *>(aPSet.getValue());
            uno::Reference< beans::XPropertySetInfo >  xInfo = xPrSet->getPropertySetInfo();
            // Expand PropertySetInfo!
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
            uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
    if(xShapeAgg.is())
    {
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
                        uno::Reference<lang::XUnoTunnel> xTunnel(xFrame, uno::UNO_QUERY);
                        SwXFrame* pFrame = xTunnel.is() ?
                                reinterpret_cast< SwXFrame * >(
                                    sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXFrame::getUnoTunnelId()) ))
                                : nullptr;
                        if(pFrame && pFrame->GetFrameFormat() &&
                            pFrame->GetFrameFormat()->GetDoc() == pDoc)
                        {
                            UnoActionContext aCtx(pDoc);
                            SfxItemSet aItemSet( pDoc->GetAttrPool(),
                                        RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                            aItemSet.SetParent(&pFormat->GetAttrSet());
                            SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aItemSet.Get(pEntry->nWID));
                            SwPosition aPos(*pFrame->GetFrameFormat()->GetContent().GetContentIdx());
                            aAnchor.SetAnchor(&aPos);
                            aAnchor.SetType(FLY_AT_FLY);
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
                        if(FmFormInventor != pObj->GetObjInventor())
                        {
                            pObj->SetLayer( *static_cast<sal_Bool const *>(aValue.getValue())
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
                    SwFormatAnchor aAnchor( static_cast<const SwFormatAnchor&>(aSet.Get( RES_ANCHOR )) );
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
                    {
                        // set property <TextRange> not valid for to-page anchored shapes
                        throw lang::IllegalArgumentException();
                    }
                    else
                    {
                        SwUnoInternalPaM* pInternalPam =
                                        new SwUnoInternalPaM( *(pFormat->GetDoc()) );
                        uno::Reference< text::XTextRange > xRg;
                        aValue >>= xRg;
                        if (::sw::XTextRangeToSwPaM(*pInternalPam, xRg) )
                        {
                            if (aAnchor.GetAnchorId() == FLY_AS_CHAR)
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
                                SAL_WARN_IF( !pHint || pHint->Which() != RES_TXTATR_FLYCNT,
                                            "sw.uno", "Missing FlyInCnt-Hint." );
                                SAL_WARN_IF( !pHint || pHint->GetFlyCnt().GetFrameFormat() != pFormat,
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
                        else
                        {
                            throw uno::RuntimeException();
                        }
                        delete pInternalPam;
                    }
                }
                else if (pEntry->nWID == FN_TEXT_BOX)
                {
                    bool bValue(false);
                    aValue >>= bValue;
                    if (bValue)
                        SwTextBoxHelper::create(pFormat);
                    else
                        SwTextBoxHelper::destroy(pFormat);

                }
                else if (pEntry->nWID == RES_CHAIN)
                {
                    if (pEntry->nMemberId == MID_CHAIN_NEXTNAME || pEntry->nMemberId == MID_CHAIN_PREVNAME)
                        SwTextBoxHelper::syncProperty(pFormat, pEntry->nWID, pEntry->nMemberId, aValue);
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
                        pDoc->ChgAnchor( aList, (RndStdIds)nAnchor,
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
                else
                {
                    m_pPropSet->setPropertyValue( *pEntry, aValue, aSet );

                    if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
                    {
                        bool bSetAttr = true;
                        sal_Int32 eNewAnchor = SWUnoHelper::GetEnumAsInt32( aValue );

                        //if old anchor was in_cntnt the related text attribute has to be removed
                        const SwFormatAnchor& rOldAnchor = pFormat->GetAnchor();
                        RndStdIds eOldAnchorId = rOldAnchor.GetAnchorId();
                        SdrObject* pObj = pFormat->FindSdrObject();
                        SwFrameFormat *pFlyFormat = FindFrameFormat( pObj );
                        pFlyFormat->DelFrames();
                        if( text::TextContentAnchorType_AS_CHARACTER != eNewAnchor &&
                            (FLY_AS_CHAR == eOldAnchorId))
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
                            SAL_WARN_IF( !pHint || pHint->Which() != RES_TXTATR_FLYCNT,
                                        "sw.uno", "Missing FlyInCnt-Hint." );
                            SAL_WARN_IF( !pHint || pHint->GetFlyCnt().GetFrameFormat() != pFlyFormat,
                                        "sw.uno", "Wrong TextFlyCnt-Hint." );
                            const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt())
                                .SetFlyFormat();

                            //The connection is removed now the attribute can be deleted.
                            pTextNode->DeleteAttributes(RES_TXTATR_FLYCNT, nIdx);
                        }
                        else if( text::TextContentAnchorType_AT_PAGE != eNewAnchor &&
                                (FLY_AT_PAGE == eOldAnchorId))
                        {
                            SwFormatAnchor aNewAnchor( dynamic_cast< const SwFormatAnchor& >( aSet.Get( RES_ANCHOR ) ) );
                            //if the fly has been anchored at page then it needs to be connected
                            //to the content position
                            SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                            if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                            {
                                SwCursorMoveState aState( MV_SETONLYTEXT );
                                Point aTmp( pObj->GetSnapRect().TopLeft() );
                                pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetCursorOfst( aPam.GetPoint(), aTmp, &aState );
                            }
                            else
                            {
                                //without access to the layout the last node of the body will be used as anchor position
                                aPam.Move( fnMoveBackward, fnGoDoc );
                            }
                            //anchor position has to be inserted after the text attribute has been inserted
                            aNewAnchor.SetAnchor( aPam.GetPoint() );
                            aSet.Put( aNewAnchor );
                            pFormat->SetFormatAttr(aSet);
                            bSetAttr = false;
                        }
                        if( text::TextContentAnchorType_AS_CHARACTER == eNewAnchor &&
                            (FLY_AS_CHAR != eOldAnchorId))
                        {
                            SwPaM aPam(pDoc->GetNodes().GetEndOfContent());
                            if( pDoc->getIDocumentLayoutAccess().GetCurrentLayout() )
                            {
                                SwCursorMoveState aState( MV_SETONLYTEXT );
                                Point aTmp( pObj->GetSnapRect().TopLeft() );
                                pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetCursorOfst( aPam.GetPoint(), aTmp, &aState );
                            }
                            else
                            {
                                //without access to the layout the last node of the body will be used as anchor position
                                aPam.Move( fnMoveBackward, fnGoDoc );
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
                SwTextBoxHelper::syncProperty(pFormat, pEntry->nWID, pEntry->nMemberId, aValue);
            }
            else
            {
                SfxPoolItem* pItem = nullptr;
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
                            cppu::UnoType<text::XTextRange>::get();
                        if(aValue.getValueType() == rTextRangeType)
                        {
                            uno::Reference< text::XTextRange > & rRange = pImpl->GetTextRange();
                            rRange = *static_cast<uno::Reference< text::XTextRange > const *>(aValue.getValue());
                        }
                    }
                    break;
                    case RES_OPAQUE :
                        pImpl->SetOpaque(*static_cast<sal_Bool const *>(aValue.getValue()));
                    break;
                    // #i26791#
                    case RES_FOLLOW_TEXT_FLOW:
                    {
                        pItem = pImpl->GetFollowTextFlow( true );
                    }
                    break;
                    // #i28701#
                    case RES_WRAP_INFLUENCE_ON_OBJPOS:
                    {
                        pItem = pImpl->GetWrapInfluenceOnObjPos( true );
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
                    static_cast<SfxPoolItem*>(pItem)->PutValue(aValue, pEntry->nMemberId);
            }
        }
        else
        {
            uno::Reference< beans::XPropertySet >  xPrSet;
            const uno::Type& rPSetType =
                cppu::UnoType<beans::XPropertySet>::get();
            uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
            if(aPSet.getValueType() != rPSetType || !aPSet.getValue())
                throw uno::RuntimeException();
            xPrSet = *static_cast<uno::Reference< beans::XPropertySet > const *>(aPSet.getValue());
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
                xPrSet->setPropertyValue(rPropertyName, aValue);
            }
            else
                xPrSet->setPropertyValue(rPropertyName, aValue);

            if (pFormat)
            {
                // We have a pFormat (but no pEntry): try to sync TextBox property.
                SwTextBoxHelper::syncProperty(pFormat, rPropertyName, aValue);
            }

            // #i31698# - restore object position, if caption point is set.
            if ( rPropertyName == "CaptionPoint" && getShapeType() == "com.sun.star.drawing.CaptionShape" )
            {
                setPosition( aKeepedPosition );
            }
        }
    }
}

uno::Any SwXShape::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
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
                        aRet.setValue(&aPoint, cppu::UnoType<awt::Point>::get());
                    }
                }
                // #i26791# - special handling for FN_TEXT_RANGE
                else if ( FN_TEXT_RANGE == pEntry->nWID )
                {
                    const SwFormatAnchor aAnchor = pFormat->GetAnchor();
                    if (aAnchor.GetAnchorId() == FLY_AT_PAGE)
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
                            aRet.setValue(&xTextRange, cppu::UnoType<text::XTextRange>::get());
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
                    bool bValue = SwTextBoxHelper::findTextBox(pFormat);
                    aRet <<= bValue;
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
                    aRet = uno::makeAny(nRet);
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
                        aRet.setValue(&pImpl->GetTextRange(), cppu::UnoType<text::XTextRange>::get());
                    break;
                    case RES_OPAQUE :
                        aRet <<= pImpl->GetOpaque();
                    break;
                    case FN_ANCHOR_POSITION :
                    {
                        awt::Point aPoint;
                        aRet.setValue(&aPoint, cppu::UnoType<awt::Point>::get());
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
                        aRet = _getPropAtAggrObj( "Transformation" );
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
                            std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pFormat->GetDoc());
                            aRet <<= SwTextBoxHelper::getOrdNum(pObj, aTextBoxes);
                        }
                    }
                }
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
                cppu::UnoType<beans::XPropertySet>::get();
    uno::Any aPSet = xShapeAgg->queryAggregation(rPSetType);
    if ( aPSet.getValueType() != rPSetType || !aPSet.getValue() )
    {
        throw uno::RuntimeException();
    }
    xPrSet = *static_cast<uno::Reference< beans::XPropertySet > const *>(aPSet.getValue());
    aRet = xPrSet->getPropertyValue( _rPropertyName );

    return aRet;
}

beans::PropertyState SwXShape::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXShape::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    uno::Sequence< beans::PropertyState > aRet(aPropertyNames.getLength());
    if(xShapeAgg.is())
    {
        SvxShape* pSvxShape = GetSvxShape();
        bool bGroupMember = false;
        bool bFormControl = false;
        SdrObject* pObject = pSvxShape ? pSvxShape->GetSdrObject() : nullptr;
        if(pObject)
        {
            bGroupMember = pObject->GetUpGroup() != nullptr;
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
                else if (pEntry->nWID == RES_FRM_SIZE &&
                         (pEntry->nMemberId == MID_FRMSIZE_REL_HEIGHT_RELATION ||
                          pEntry->nMemberId == MID_FRMSIZE_REL_WIDTH_RELATION))
                    pRet[nProperty] = beans::PropertyState_DIRECT_VALUE;
                else if (pEntry->nWID == FN_TEXT_BOX)
                {
                    // The TextBox property is set, if we can find a textbox for this shape.
                    if (pFormat && SwTextBoxHelper::findTextBox(pFormat))
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
                    const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
                    uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
                    if(aPState.getValueType() != rPStateType || !aPState.getValue())
                        throw uno::RuntimeException();
                    xShapePrState = *static_cast<uno::Reference< XPropertyState > const *>(aPState.getValue());
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
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
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
                    case RES_ANCHOR:        pImpl->RemoveAnchor();  break;
                    case RES_HORI_ORIENT:   pImpl->RemoveHOrient(); break;
                    case RES_VERT_ORIENT:   pImpl->RemoveVOrient(); break;
                    case  RES_LR_SPACE:     pImpl->RemoveLRSpace(); break;
                    case  RES_UL_SPACE:     pImpl->RemoveULSpace(); break;
                    case  RES_SURROUND:     pImpl->RemoveSurround();break;
                    case RES_OPAQUE :       pImpl->SetOpaque(false);  break;
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
            const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            uno::Reference< XPropertyState > xShapePrState = *static_cast<uno::Reference< XPropertyState > const *>(aPState.getValue());
            xShapePrState->setPropertyToDefault( rPropertyName );
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXShape::getPropertyDefault( const OUString& rPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat*   pFormat = GetFrameFormat();
    uno::Any aRet;
    if(xShapeAgg.is())
    {
        const SfxItemPropertySimpleEntry*  pEntry = m_pPropSet->getPropertyMap().getByName( rPropertyName );
        if(pEntry)
        {
            if(pEntry->nWID < RES_FRMATR_END && pFormat)
            {
                const SfxPoolItem& rDefItem =
                    pFormat->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
                rDefItem.QueryValue(aRet, pEntry->nMemberId);
            }
            else
                throw uno::RuntimeException();
        }
        else
        {
            const uno::Type& rPStateType = cppu::UnoType<XPropertyState>::get();
            uno::Any aPState = xShapeAgg->queryAggregation(rPStateType);
            if(aPState.getValueType() != rPStateType || !aPState.getValue())
                throw uno::RuntimeException();
            uno::Reference< XPropertyState > xShapePrState = *static_cast<uno::Reference< XPropertyState > const *>(aPState.getValue());
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
           uno::RuntimeException, std::exception )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->addPropertyChangeListener( _propertyName, _listener );
}

void SwXShape::removePropertyChangeListener(
    const OUString& _propertyName,
    const uno::Reference< beans::XPropertyChangeListener > & _listener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    if ( !xShapeAgg.is() )
        throw uno::RuntimeException("no shape aggregate", *this );

    // must be handled by the aggregate
    uno::Reference< beans::XPropertySet > xShapeProps;
    if ( xShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get() ) >>= xShapeProps )
        xShapeProps->removePropertyChangeListener( _propertyName, _listener );
}

void SwXShape::addVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/ )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXShape::removeVetoableChangeListener(
    const OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXShape::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

void SwXShape::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    // get access to SwDoc
    // (see also SwXTextRange::XTextRangeToSwPaM)
    SwDoc*      pDoc = nullptr;
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    if(xRangeTunnel.is())
    {
        SwXTextRange* pRange = nullptr;
        OTextCursorHelper* pCursor = nullptr;
        SwXTextPortion* pPortion = nullptr;
        SwXText* pText = nullptr;
        SwXParagraph* pParagraph = nullptr;

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
            pDoc = &pRange->GetDoc();
        else if (!pDoc && pText)
            pDoc = pText->GetDoc();
        else if (!pDoc && pCursor)
            pDoc = pCursor->GetDoc();
        else if ( !pDoc && pPortion )
        {
            pDoc = pPortion->GetCursor().GetDoc();
        }
        else if ( !pDoc && pParagraph && pParagraph->GetTextNode( ) )
        {
            pDoc = const_cast<SwDoc*>(pParagraph->GetTextNode()->GetDoc());
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
                uno::Reference< drawing::XShape > xTemp( static_cast<cppu::OWeakObject*>(this), uno::UNO_QUERY );
                xDP->add( xTemp );
            }
        }
    }
}

uno::Reference< text::XTextRange >  SwXShape::getAnchor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if ((rAnchor.GetAnchorId() != FLY_AT_PAGE) ||
            (rAnchor.GetContentAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(pFormat->GetAnchor().GetContentAnchor());
            aRef = SwXTextRange::CreateXTextRange(*pFormat->GetDoc(), rPos, nullptr);
        }
    }
    else
        aRef = pImpl->GetTextRange();
    return aRef;
}

void SwXShape::dispose() throw( uno::RuntimeException, std::exception )
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
                pObj->GetUpGroup() ||
                pObj == pFormat->FindSdrObject(),
                "<SwXShape::dispose(..) - different 'master' drawing objects!!" );
        // perform delete of draw frame format *not*
        // for 'virtual' drawing objects.
        // no delete of draw format for members
        // of a group
        if ( pObj &&
             dynamic_cast<const SwDrawVirtObj*>( pObj) ==  nullptr &&
             !pObj->GetUpGroup() &&
             pObj->IsInserted() )
        {
            if (pFormat->GetAnchor().GetAnchorId() == FLY_AS_CHAR)
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
    if(xShapeAgg.is())
    {
        uno::Any aAgg(xShapeAgg->queryAggregation( cppu::UnoType<XComponent>::get()));
        uno::Reference<XComponent> xComp;
        aAgg >>= xComp;
        if(xComp.is())
            xComp->dispose();
    }
}

void SwXShape::addEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
                    throw( uno::RuntimeException, std::exception )
{
    uno::Reference< lang::XUnoTunnel > xShapeTunnel(xShapeAgg, uno::UNO_QUERY);
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
         pSvxShape->addEventListener(aListener);
}

void SwXShape::removeEventListener(
                    const uno::Reference< lang::XEventListener > & aListener)
                    throw( uno::RuntimeException, std::exception )
{
    SvxShape* pSvxShape = GetSvxShape();
    if(pSvxShape)
        pSvxShape->removeEventListener(aListener);
}

OUString SwXShape::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXShape");
}

sal_Bool SwXShape::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXShape::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
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
    SvxShape* pSvxShape = nullptr;
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
awt::Point SAL_CALL SwXShape::getPosition() throw ( uno::RuntimeException, std::exception )
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
            aOffset.X = convertTwipToMm100(aOffset.X);
            aOffset.Y = convertTwipToMm100(aOffset.Y);
            aPos.X += aOffset.X;
            aPos.Y += aOffset.Y;
        }
    }

    return aPos;
}

void SAL_CALL SwXShape::setPosition( const awt::Point& aPosition )
                                                throw ( uno::RuntimeException, std::exception )
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

awt::Size SAL_CALL SwXShape::getSize() throw ( uno::RuntimeException, std::exception )
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
                                                uno::RuntimeException, std::exception )
{
    if ( mxShape.is() )
    {
        mxShape->setSize( aSize );
    }
    SwTextBoxHelper::syncProperty(GetFrameFormat(), RES_FRM_SIZE, MID_FRMSIZE_SIZE, uno::makeAny(aSize));
}
// #i31698#
// implementation of virtual methods from drawing::XShapeDescriptor
OUString SAL_CALL SwXShape::getShapeType() throw ( uno::RuntimeException, std::exception )
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
SdrObject* SwXShape::_GetTopGroupObj( SvxShape* _pSvxShape )
{
    SdrObject* pTopGroupObj( nullptr );

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
awt::Point SwXShape::_ConvertPositionToHoriL2R( const awt::Point& rObjPos,
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
                                    const drawing::HomogenMatrix3& rMatrixInHoriL2R )
{
    drawing::HomogenMatrix3 aMatrix(rMatrixInHoriL2R);

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
void SwXShape::_AdjustPositionProperties( const awt::Point& rPosition )
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
        const OUString aHoriPosPropStr("HoriOrientPosition");
        uno::Any aHoriPos( getPropertyValue( aHoriPosPropStr ) );
        sal_Int32 dCurrX = 0;
        aHoriPos >>= dCurrX;
        // change x-position attribute, if needed
        if ( dCurrX != rPosition.X )
        {
            // adjust x-position orientation to text::HoriOrientation::NONE, if needed
            // Note: has to be done before setting x-position attribute
            const OUString aHoriOrientPropStr("HoriOrient");
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
        const OUString aVertPosPropStr("VertOrientPosition");
        uno::Any aVertPos( getPropertyValue( aVertPosPropStr ) );
        sal_Int32 dCurrY = 0;
        aVertPos >>= dCurrY;
        // change y-position attribute, if needed
        if ( dCurrY != rPosition.Y )
        {
            // adjust y-position orientation to text::VertOrientation::NONE, if needed
            // Note: has to be done before setting y-position attribute
            const OUString aVertOrientPropStr("VertOrient");
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
css::awt::Point SwXShape::_ConvertStartOrEndPosToLayoutDir(
                            const css::awt::Point& aStartOrEndPos )
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

css::drawing::PolyPolygonBezierCoords SwXShape::_ConvertPolyPolygonBezierToLayoutDir(
                    const css::drawing::PolyPolygonBezierCoords& aPath )
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
                    convertTwipToMm100( aTmpObjPos.X() - pObj->GetAnchorPos().X() ),
                    convertTwipToMm100( aTmpObjPos.Y() - pObj->GetAnchorPos().Y() ) );
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

uno::Any SwXGroupShape::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException, std::exception)
{
    uno::Any aRet;
    if(rType == cppu::UnoType<XShapes>::get())
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

void SwXGroupShape::add( const uno::Reference< XShape >& xShape ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SvxShape* pSvxShape = GetSvxShape();
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pSvxShape && pFormat)
    {
        uno::Reference<XShapes> xShapes;
        if( xShapeAgg.is() )
        {
            const uno::Type& rType = cppu::UnoType<XShapes>::get();
            uno::Any aAgg = xShapeAgg->queryAggregation( rType );
            aAgg >>= xShapes;
        }
        if(xShapes.is())
            xShapes->add(xShape);
        else
            throw uno::RuntimeException();

        uno::Reference<lang::XUnoTunnel> xTunnel(xShape, uno::UNO_QUERY);
        SwXShape* pSwShape = nullptr;
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
                    SwDoc* pDoc = pFormat->GetDoc();
                    // set layer of new drawing
                    // object to corresponding invisible layer.
                    if( FmFormInventor != pObj->GetObjInventor())
                    {
                        pObj->SetLayer( pSwShape->pImpl->GetOpaque()
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
                pFormat->Add(pSwShape);
        }
    }
    else
        throw uno::RuntimeException();
}

void SwXGroupShape::remove( const uno::Reference< XShape >& xShape ) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<XShapes> xShapes;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XShapes>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xShapes;
    }
    if(!xShapes.is())
        throw uno::RuntimeException();
    xShapes->remove(xShape);
}

sal_Int32 SwXGroupShape::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getCount();
}

uno::Any SwXGroupShape::getByIndex(sal_Int32 nIndex)
        throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException,
               uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getByIndex(nIndex);
}

uno::Type SwXGroupShape::getElementType(  ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->getElementType();
}

sal_Bool SwXGroupShape::hasElements(  ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference<XIndexAccess> xAcc;
    if( xShapeAgg.is() )
    {
        const uno::Type& rType = cppu::UnoType<XIndexAccess>::get();
        uno::Any aAgg = xShapeAgg->queryAggregation( rType );
        aAgg >>= xAcc;
    }
    if(!xAcc.is())
        throw uno::RuntimeException();
    return xAcc->hasElements();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
