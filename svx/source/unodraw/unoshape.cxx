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

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <editeng/unotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/virdev.hxx>
#include <svx/sdangitm.hxx>
#include <svx/svdlayer.hxx>
#include <svx/svdopage.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobjkind.hxx>
#include <svx/unopage.hxx>
#include <svx/unoshape.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoshprp.hxx>
#include <svx/svdograf.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdpool.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>
#include <tools/gen.hxx>
#include <tools/UnitConversion.hxx>
#include <svx/svdoedge.hxx>
#include <svx/svdocapt.hxx>
#include <svx/obj3d.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/unomaster.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "gluepts.hxx"
#include "shapeimpl.hxx"
#include <sal/log.hxx>

#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/converters.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/wmf.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/svdopath.hxx>
#include <svx/SvxXTextColumns.hxx>
#include <svx/xflclit.hxx>
#include <editeng/frmdiritem.hxx>

#include <memory>
#include <optional>
#include <vector>
#include <iostream>

#include <bitmaps.hlst>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

class GDIMetaFile;

struct SvxShapeImpl
{
    std::optional<SfxItemSet> moItemSet;
    SdrObjKind      mnObjId;
    SvxShapeMaster* mpMaster;
    bool            mbDisposing;

    /** CL, OD 2005-07-19 #i52126# - this is initially 0 and set when
     *  a SvxShape::Create() call is executed. It is then set to the created
     *  SdrObject so a multiple call to SvxShape::Create() with same SdrObject
     *  is prohibited.
     */
    ::unotools::WeakReference< SdrObject > mxCreatedObj;

    // for xComponent
    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> maDisposeListeners;
    ::comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, css::beans::XPropertyChangeListener> maPropertyChangeListeners;

    SvxShapeImpl()
        :mnObjId( SdrObjKind::NONE )
        ,mpMaster( nullptr )
        ,mbDisposing( false )
    {
    }
};

namespace {


/// Calculates what scaling factor will be used for autofit text scaling of this shape.
SdrTextObj* getTextObjectWithFitToSize(SdrObject* pObject)
{
    SdrTextObj* pTextObj = DynCastSdrTextObj(pObject);
    if (!pTextObj)
    {
        return nullptr;
    }

    const SfxItemSet& rTextObjSet = pTextObj->GetMergedItemSet();
    if (rTextObjSet.GetItem<SdrTextFitToSizeTypeItem>(SDRATTR_TEXT_FITTOSIZE)->GetValue()
        != drawing::TextFitToSizeType_AUTOFIT)
    {
        return nullptr;
    }

    return pTextObj;
}
}

SvxShape::SvxShape( SdrObject* pObject )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
,   maPropMapEntries(getSvxMapProvider().GetMap(SVXMAP_SHAPE))
,   mxSdrObject(pObject)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::SvxShape( SdrObject* pObject, std::span<const SfxItemPropertyMapEntry> pEntries, const SvxItemPropertySet* pPropertySet )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(pPropertySet)
,   maPropMapEntries(pEntries)
,   mxSdrObject(pObject)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::~SvxShape() noexcept
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount == 0, "Locked shape was disposed!" );

    if ( mpImpl->mpMaster )
        mpImpl->mpMaster->dispose();

    if ( mxSdrObject )
    {
        EndListening(mxSdrObject->getSdrModelFromSdrObject());
        mxSdrObject->setUnoShape(nullptr);
        mxSdrObject.clear();
    }

    EndListeningAll(); // call explicitly within SolarMutexGuard
}


void SvxShape::InvalidateSdrObject()
{
    if(mxSdrObject)
    {
        EndListening(mxSdrObject->getSdrModelFromSdrObject());
        mxSdrObject.clear();
    }
};

void SvxShape::setShapeKind( SdrObjKind nKind )
{
    mpImpl->mnObjId = nKind;
}


SdrObjKind SvxShape::getShapeKind() const
{
    return mpImpl->mnObjId;
}


void SvxShape::setMaster( SvxShapeMaster* pMaster )
{
    mpImpl->mpMaster = pMaster;
}


uno::Any SAL_CALL SvxShape::queryAggregation( const uno::Type& rType )
{
    if( mpImpl->mpMaster )
    {
        uno::Any aAny;
        if( mpImpl->mpMaster->queryAggregation( rType, aAny ) )
            return aAny;
    }

    return SvxShape_UnoImplHelper::queryAggregation(rType);
}

const css::uno::Sequence< sal_Int8 > & SvxShape::getUnoTunnelId() noexcept
{
    static const comphelper::UnoIdInit theSvxShapeUnoTunnelId;
    return theSvxShapeUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SvxShape::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    return comphelper::getSomethingImpl(rId, this);
}


void SvxShape::notifyPropertyChange(const OUString& rPropName)
{
    std::unique_lock g(m_aMutex);
    comphelper::OInterfaceContainerHelper4<beans::XPropertyChangeListener>* pPropListeners =
            mpImpl->maPropertyChangeListeners.getContainer( g, rPropName );
    comphelper::OInterfaceContainerHelper4<beans::XPropertyChangeListener>* pAllListeners =
            mpImpl->maPropertyChangeListeners.getContainer( g, OUString() );
    if (pPropListeners || pAllListeners)
    {
        try
        {
            // Handle/OldValue not supported
            beans::PropertyChangeEvent aEvt;
            aEvt.Source = static_cast<cppu::OWeakObject*>(this);
            aEvt.PropertyName = rPropName;
            aEvt.NewValue = getPropertyValue(rPropName);
            if (pPropListeners)
                pPropListeners->notifyEach( g, &beans::XPropertyChangeListener::propertyChange, aEvt );
            if (pAllListeners)
                pAllListeners->notifyEach( g, &beans::XPropertyChangeListener::propertyChange, aEvt );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

    }
}

void SvxShape::impl_construct()
{
    if ( HasSdrObject() )
    {
        StartListening(GetSdrObject()->getSdrModelFromSdrObject());
        impl_initFromSdrObject();
    }
}


void SvxShape::impl_initFromSdrObject()
{
    DBG_TESTSOLARMUTEX();
    OSL_PRECOND( HasSdrObject(), "SvxShape::impl_initFromSdrObject: not to be called without SdrObject!" );
    if ( !HasSdrObject() )
        return;

    osl_atomic_increment( &m_refCount );
    {
        GetSdrObject()->setUnoShape(this);
    }
    osl_atomic_decrement( &m_refCount );

    // #i40944#
    // Do not simply return when no model but do the type corrections
    // following below.
    const SdrInventor nInventor = GetSdrObject()->GetObjInventor();

    // is it one of ours (svx) ?
    if( !(nInventor == SdrInventor::Default || nInventor == SdrInventor::E3d || nInventor == SdrInventor::FmForm) )
        return;

    if(nInventor == SdrInventor::FmForm)
    {
        mpImpl->mnObjId = SdrObjKind::UNO;
    }
    else
    {
        mpImpl->mnObjId = GetSdrObject()->GetObjIdentifier();
    }

    switch(mpImpl->mnObjId)
    {
    case SdrObjKind::CircleCut:          // segment of circle
    case SdrObjKind::CircleArc:          // arc of circle
    case SdrObjKind::CircleSection:          // sector
        mpImpl->mnObjId = SdrObjKind::CircleOrEllipse;
        break;
    default: ;
    }
}


void SvxShape::Create( SdrObject* pNewObj, SvxDrawPage* /*pNewPage*/ )
{
    DBG_TESTSOLARMUTEX();

    assert( pNewObj && "SvxShape::Create: invalid new object!" );
    if ( !pNewObj )
        return;

    rtl::Reference<SdrObject> pCreatedObj = mpImpl->mxCreatedObj.get();
    assert( ( !pCreatedObj || ( pCreatedObj == pNewObj ) ) &&
        "SvxShape::Create: the same shape used for two different objects?! Strange ..." );

    // Correct condition (#i52126#)
    if ( pCreatedObj == pNewObj )
        return;

    // Correct condition (#i52126#)
    mpImpl->mxCreatedObj = pNewObj;

    if( HasSdrObject() )
    {
        EndListening( GetSdrObject()->getSdrModelFromSdrObject() );
    }

    mxSdrObject = pNewObj;

    if( HasSdrObject() )
    {
        StartListening( GetSdrObject()->getSdrModelFromSdrObject() );
    }

    OSL_ENSURE( !mbIsMultiPropertyCall, "SvxShape::Create: hmm?" );
        // this was previously set in impl_initFromSdrObject, but I think it was superfluous
        // (it definitely was in the other context where it was called, but I strongly suppose
        // it was also superfluous when called from here)
    impl_initFromSdrObject();

    ObtainSettingsFromPropertySet( *mpPropSet );

    // save user call
    SdrObjUserCall* pUser = GetSdrObject()->GetUserCall();
    GetSdrObject()->SetUserCall(nullptr);

    setPosition( maPosition );
    setSize( maSize );

    // restore user call after we set the initial size
    GetSdrObject()->SetUserCall( pUser );

    // if this shape was already named, use this name
    if( !maShapeName.isEmpty() )
    {
        GetSdrObject()->SetName( maShapeName );
        maShapeName.clear();
    }
}

void SvxShape::ForceMetricToItemPoolMetric(Pair& rPoint) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    MapUnit eMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
    if(eMapUnit == MapUnit::Map100thMM)
        return;

    if (const auto eTo = MapToO3tlLength(eMapUnit); eTo != o3tl::Length::invalid)
    {
        rPoint.A() = o3tl::convert(rPoint.A(), o3tl::Length::mm100, eTo);
        rPoint.B() = o3tl::convert(rPoint.B(), o3tl::Length::mm100, eTo);
    }
    else
    {
        OSL_FAIL("AW: Missing unit translation to PoolMetric!");
    }
}

void SvxShape::ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    GetSdrObject()->ForceMetricToItemPoolMetric(rPolyPolygon);
}

void SvxShape::ForceMetricToItemPoolMetric(basegfx::B2DHomMatrix& rB2DHomMatrix) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    MapUnit eMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
    if(eMapUnit == MapUnit::Map100thMM)
        return;

    if (const auto eTo = MapToO3tlLength(eMapUnit); eTo != o3tl::Length::invalid)
    {
        const double fConvert(o3tl::convert(1.0, o3tl::Length::mm100, eTo));
        const basegfx::utils::B2DHomMatrixBufferedDecompose aDecomposedTransform(rB2DHomMatrix);
        rB2DHomMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
            aDecomposedTransform.getScale() * fConvert,
            aDecomposedTransform.getShearX(),
            aDecomposedTransform.getRotate(),
            aDecomposedTransform.getTranslate() * fConvert);
    }
    else
    {
        OSL_FAIL("Missing unit translation to PoolMetric!");
    }
}

void SvxShape::ForceMetricTo100th_mm(Pair& rPoint) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    MapUnit eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if(eMapUnit == MapUnit::Map100thMM)
        return;

    if (const auto eFrom = MapToO3tlLength(eMapUnit); eFrom != o3tl::Length::invalid)
    {
        rPoint.A() = o3tl::convert(rPoint.A(), eFrom, o3tl::Length::mm100);
        rPoint.B() = o3tl::convert(rPoint.B(), eFrom, o3tl::Length::mm100);
    }
    else
    {
        OSL_FAIL("AW: Missing unit translation to 100th mm!");
    }
}

void SvxShape::ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    MapUnit eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if(eMapUnit == MapUnit::Map100thMM)
        return;

    if (const auto eFrom = MapToO3tlLength(eMapUnit); eFrom != o3tl::Length::invalid)
    {
        const double fConvert(o3tl::convert(1.0, eFrom, o3tl::Length::mm100));
        rPolyPolygon.transform(basegfx::utils::createScaleB2DHomMatrix(fConvert, fConvert));
    }
    else
    {
        OSL_FAIL("Missing unit translation to 100th mm!");
    }
}

void SvxShape::ForceMetricTo100th_mm(basegfx::B2DHomMatrix& rB2DHomMatrix) const noexcept
{
    DBG_TESTSOLARMUTEX();
    if(!HasSdrObject())
        return;

    MapUnit eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
    if(eMapUnit == MapUnit::Map100thMM)
        return;

    if (const auto eFrom = MapToO3tlLength(eMapUnit); eFrom != o3tl::Length::invalid)
    {
        const double fConvert(o3tl::convert(1.0, eFrom, o3tl::Length::mm100));
        const basegfx::utils::B2DHomMatrixBufferedDecompose aDecomposedTransform(rB2DHomMatrix);
        rB2DHomMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
            aDecomposedTransform.getScale() * fConvert,
            aDecomposedTransform.getShearX(),
            aDecomposedTransform.getRotate(),
            aDecomposedTransform.getTranslate() * fConvert);
    }
    else
    {
        OSL_FAIL("Missing unit translation to 100th mm!");
    }
}

static void SvxItemPropertySet_ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet, SvxItemPropertySetUsrAnys& rAnys,
  SfxItemSet& rSet, const uno::Reference< beans::XPropertySet >& xSet, const SfxItemPropertyMap* pMap )
{
    if(!rAnys.AreThereOwnUsrAnys())
        return;

    const SfxItemPropertyMap& rSrc = rPropSet.getPropertyMap();

    for(const SfxItemPropertyMapEntry* pSrcProp : rSrc.getPropertyEntries())
    {
        const sal_uInt16 nWID = pSrcProp->nWID;
        if(SfxItemPool::IsWhich(nWID)
                && (nWID < OWN_ATTR_VALUE_START || nWID > OWN_ATTR_VALUE_END)
                && rAnys.GetUsrAnyForID(*pSrcProp))
            rSet.Put(rSet.GetPool()->GetUserOrPoolDefaultItem(nWID));
    }

    for(const SfxItemPropertyMapEntry* pSrcProp : rSrc.getPropertyEntries())
    {
        if(pSrcProp->nWID)
        {
            uno::Any* pUsrAny = rAnys.GetUsrAnyForID(*pSrcProp);
            if(pUsrAny)
            {
                // search for equivalent entry in pDst
                const SfxItemPropertyMapEntry* pEntry = pMap->getByName( pSrcProp->aName );
                if(pEntry)
                {
                    // entry found
                    if(pEntry->nWID >= OWN_ATTR_VALUE_START && pEntry->nWID <= OWN_ATTR_VALUE_END)
                    {
                        // special ID in PropertySet, can only be set
                        // directly at the object
                        xSet->setPropertyValue( pSrcProp->aName, *pUsrAny);
                    }
                    else
                    {
                        SvxItemPropertySet_setPropertyValue(pEntry, *pUsrAny, rSet);
                    }
                }
            }
        }
    }
    rAnys.ClearAllUsrAny();
}


void SvxShape::ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet)
{
    DBG_TESTSOLARMUTEX();
    if(HasSdrObject() && maUrsAnys.AreThereOwnUsrAnys())
    {
        SfxItemSetFixed<SDRATTR_START, SDRATTR_END> aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool() );
        Reference< beans::XPropertySet > xShape(this);
        SvxItemPropertySet_ObtainSettingsFromPropertySet(rPropSet, maUrsAnys, aSet, xShape, &mpPropSet->getPropertyMap() );

        GetSdrObject()->SetMergedItemSetAndBroadcast(aSet);

        GetSdrObject()->ApplyNotPersistAttr( aSet );
    }
}

uno::Any SvxShape::GetBitmap( bool bMetaFile /* = false */ ) const
{
    DBG_TESTSOLARMUTEX();
    uno::Any aAny;

    if(!HasSdrObject() || nullptr == GetSdrObject()->getSdrPageFromSdrObject())
    {
        return aAny;
    }

    // tdf#118662 Emulate old behaviour of XclObjComment (see there)
    const SdrCaptionObj* pSdrCaptionObj(dynamic_cast<SdrCaptionObj*>(GetSdrObject()));
    if(nullptr != pSdrCaptionObj && pSdrCaptionObj->isSuppressGetBitmap())
    {
        return aAny;
    }

    // tdf#119180 If we do not ask for Metafile and we access a SdrGrafObj,
    // and content exists and is a Bitmap, take the shortcut.
    // Do *not* do this for Metafile - as can be seen, requested in that case
    // is a byte-sequence of a saved WMF format file (see below)
    if(!bMetaFile)
    {
        const SdrGrafObj* pSdrGrafObj(dynamic_cast<SdrGrafObj*>(GetSdrObject()));

        if(nullptr != pSdrGrafObj)
        {
            const Graphic& rGraphic(pSdrGrafObj->GetGraphic());

            if(GraphicType::Bitmap == rGraphic.GetType())
            {
                Reference< awt::XBitmap > xBmp( rGraphic.GetXGraphic(), UNO_QUERY );
                aAny <<= xBmp;

                return aAny;
            }
        }
    }

    // tdf#118662 instead of creating an E3dView instance every time to paint
    // a single SdrObject, use the existing SdrObject::SingleObjectPainter to
    // use less resources and runtime
    if(bMetaFile)
    {
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        const tools::Rectangle aBoundRect(GetSdrObject()->GetCurrentBoundRect());
        GDIMetaFile aMtf;

        pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
        pVDev->EnableOutput(false);
        aMtf.Record(pVDev);
        GetSdrObject()->SingleObjectPainter(*pVDev);
        aMtf.Stop();
        aMtf.WindStart();
        aMtf.Move(-aBoundRect.Left(), -aBoundRect.Top());
        aMtf.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        aMtf.SetPrefSize(aBoundRect.GetSize());

        SvMemoryStream aDestStrm(65535, 65535);

        ConvertGDIMetaFileToWMF(
            aMtf,
            aDestStrm,
            nullptr,
            false);

        const uno::Sequence<sal_Int8> aSeq(
            static_cast< const sal_Int8* >(aDestStrm.GetData()),
            aDestStrm.GetEndOfData());

        aAny <<= aSeq;
    }
    else
    {
        drawinglayer::primitive2d::Primitive2DContainer xPrimitives;
        GetSdrObject()->GetViewContact().getViewIndependentPrimitive2DContainer(xPrimitives);

        if(!xPrimitives.empty())
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            basegfx::B2DRange aRange(
                xPrimitives.getB2DRange(aViewInformation2D));

            if(!aRange.isEmpty())
            {
                const MapUnit aSourceMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetScaleUnit());

                if(MapUnit::Map100thMM != aSourceMapUnit)
                {
                    // tdf#119180 This is UNO API and thus works in 100th_mm,
                    // so if the MapMode from the used SdrModel is *not* equal
                    // to Map100thMM we need to embed the primitives to an adapting
                    // homogen transformation for correct values
                    const basegfx::B2DHomMatrix aMapTransform(
                        OutputDevice::LogicToLogic(
                            MapMode(aSourceMapUnit),
                            MapMode(MapUnit::Map100thMM)));

                    // Embed primitives to get them in 100th mm
                    xPrimitives = drawinglayer::primitive2d::Primitive2DContainer {
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                aMapTransform,
                                std::move(xPrimitives))
                    };

                    // Update basegfx::B2DRange aRange, too. Here we have the
                    // choice of transforming the existing value or get newly by
                    // again using 'xPrimitives.getB2DRange(aViewInformation2D)'
                    aRange.transform(aMapTransform);
                }

                const BitmapEx aBmp(
                    drawinglayer::convertPrimitive2DContainerToBitmapEx(
                        std::move(xPrimitives),
                        aRange));

                Graphic aGraph(aBmp);

                aGraph.SetPrefSize(aBmp.GetPrefSize());
                aGraph.SetPrefMapMode(aBmp.GetPrefMapMode());

                Reference< awt::XBitmap > xBmp( aGraph.GetXGraphic(), UNO_QUERY );
                aAny <<= xBmp;
            }
        }
    }

    return aAny;
}

uno::Sequence< uno::Type > SAL_CALL SvxShape::getTypes()
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getTypes();
    }
    else
    {
        return _getTypes();
    }
}


uno::Sequence< uno::Type > const & SvxShape::_getTypes()
{
    switch( mpImpl->mnObjId )
    {
    // shapes without text
    case SdrObjKind::Page:
    case SdrObjKind::OLEPluginFrame:
    case SdrObjKind::OLE2Plugin:
    case SdrObjKind::OLE2Applet:
    case SdrObjKind::E3D_Cube:
    case SdrObjKind::E3D_Sphere:
    case SdrObjKind::E3D_Lathe:
    case SdrObjKind::E3D_Extrusion:
    case SdrObjKind::E3D_Polygon:
    case SdrObjKind::Media:
    case SdrObjKind::Table:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
            };

            return aTypeSequence;
        }
    // group shape
    case SdrObjKind::Group:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                cppu::UnoType<drawing::XShapes>::get(),
                cppu::UnoType<drawing::XShapeGroup>::get(),
            };

            return aTypeSequence;
        }
    // connector shape
    case SdrObjKind::Edge:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                cppu::UnoType<drawing::XConnectorShape>::get(),
                // from SvxUnoTextBase::getTypes()
                cppu::UnoType<text::XTextAppend>::get(),
                cppu::UnoType<text::XTextCopy>::get(),
                cppu::UnoType<container::XEnumerationAccess>::get(),
                cppu::UnoType<text::XTextRangeMover>::get(),
            };

            return aTypeSequence;
        }
    // control shape
    case SdrObjKind::UNO:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                cppu::UnoType<drawing::XControlShape>::get(),
            };

            return aTypeSequence;
        }
    // 3d scene shape
    case SdrObjKind::E3D_Scene:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                cppu::UnoType<drawing::XShapes>::get(),
            };

            return aTypeSequence;
        }
    case SdrObjKind::CustomShape:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                // from SvxUnoTextBase::getTypes()
                cppu::UnoType<text::XText>::get(),
                cppu::UnoType<container::XEnumerationAccess>::get(),
                cppu::UnoType<text::XTextRangeMover>::get(),
                cppu::UnoType<drawing::XEnhancedCustomShapeDefaulter>::get(),
            };

            return aTypeSequence;
        }
    // shapes with text
    case SdrObjKind::Rectangle:
    case SdrObjKind::CircleOrEllipse:
    case SdrObjKind::Measure:
    case SdrObjKind::Line:
    case SdrObjKind::Polygon:
    case SdrObjKind::PolyLine:
    case SdrObjKind::PathLine:
    case SdrObjKind::PathFill:
    case SdrObjKind::FreehandLine:
    case SdrObjKind::FreehandFill:
    case SdrObjKind::PathPoly:
    case SdrObjKind::PathPolyLine:
    case SdrObjKind::Graphic:
    case SdrObjKind::Text:
    case SdrObjKind::Caption:
    case SdrObjKind::OLE2: // #i118485# Moved to shapes with text
    default:
        {
            static uno::Sequence<uno::Type> aTypeSequence{
                cppu::UnoType<drawing::XShape>::get(),
                cppu::UnoType<lang::XComponent>::get(),
                cppu::UnoType<beans::XPropertySet>::get(),
                cppu::UnoType<beans::XMultiPropertySet>::get(),
                cppu::UnoType<beans::XPropertyState>::get(),
                cppu::UnoType<beans::XMultiPropertyStates>::get(),
                cppu::UnoType<drawing::XGluePointsSupplier>::get(),
                cppu::UnoType<container::XChild>::get(),
                cppu::UnoType<lang::XServiceInfo>::get(),
                cppu::UnoType<lang::XTypeProvider>::get(),
                cppu::UnoType<lang::XUnoTunnel>::get(),
                cppu::UnoType<container::XNamed>::get(),
                // from SvxUnoTextBase::getTypes()
                cppu::UnoType<text::XTextAppend>::get(),
                cppu::UnoType<text::XTextCopy>::get(),
                cppu::UnoType<container::XEnumerationAccess>::get(),
                cppu::UnoType<text::XTextRangeMover>::get(),
            };

            return aTypeSequence;
        }
    }
}


uno::Sequence< sal_Int8 > SAL_CALL SvxShape::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SvxShape::Notify( SfxBroadcaster&, const SfxHint& rHint ) noexcept
{
    DBG_TESTSOLARMUTEX();

    // do cheap checks first, this method is hot
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    if( !mxSdrObject )
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
    // #i55919# SdrHintKind::ObjectChange is only interesting if it's for this object
    if ((pSdrHint->GetKind() != SdrHintKind::ModelCleared) &&
         (pSdrHint->GetKind() != SdrHintKind::ObjectChange || pSdrHint->GetObject() != mxSdrObject.get() ))
        return;

    // prevent object being deleted from under us
    rtl::Reference<SdrObject> xSdrSelf(mxSdrObject);
    uno::Reference< uno::XInterface > xSelf( mxSdrObject->getWeakUnoShape() );
    if( !xSelf.is() )
    {
        EndListening(mxSdrObject->getSdrModelFromSdrObject());
        mxSdrObject.clear();
        return;
    }

    if (pSdrHint->GetKind() == SdrHintKind::ObjectChange)
    {
        updateShapeKind();
    }
    else // (pSdrHint->GetKind() == SdrHintKind::ModelCleared)
    {
        EndListening(mxSdrObject->getSdrModelFromSdrObject());
        mxSdrObject->setUnoShape(nullptr);
        mxSdrObject.clear();

        if(!mpImpl->mbDisposing)
            dispose();
    }
}

// XShape


// The "*LogicRectHack" functions also existed in sch, and those
// duplicate symbols cause Bad Things To Happen (TM)  #i9462#.
// Prefixing with 'svx' and marking static to make sure name collisions
// do not occur.

static bool svx_needLogicRectHack( SdrObject const * pObj )
{
    if( pObj->GetObjInventor() == SdrInventor::Default)
    {
        switch(pObj->GetObjIdentifier())
        {
        case SdrObjKind::Group:
        case SdrObjKind::Line:
        case SdrObjKind::Polygon:
        case SdrObjKind::PolyLine:
        case SdrObjKind::PathLine:
        case SdrObjKind::PathFill:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::Edge:
        case SdrObjKind::PathPoly:
        case SdrObjKind::PathPolyLine:
        case SdrObjKind::Measure:
            return true;
        default:
            break;
        }
    }
    return false;
}


static tools::Rectangle svx_getLogicRectHack( SdrObject const * pObj )
{
    if(svx_needLogicRectHack(pObj))
    {
        return pObj->GetSnapRect();
    }
    else
    {
        return pObj->GetLogicRect();
    }
}


static void svx_setLogicRectHack( SdrObject* pObj, const tools::Rectangle& rRect )
{
    if(svx_needLogicRectHack(pObj))
    {
        pObj->SetSnapRect( rRect );
    }
    else
    {
        pObj->SetLogicRect( rRect );
    }
}


awt::Point SAL_CALL SvxShape::getPosition()
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
    {
        tools::Rectangle aRect( svx_getLogicRectHack(GetSdrObject()) );
        Point aPt( aRect.Left(), aRect.Top() );

        // Position is relative to anchor, so recalc to absolute position
        if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
            aPt -= GetSdrObject()->GetAnchorPos();

        ForceMetricTo100th_mm(aPt);
        return css::awt::Point( aPt.X(), aPt.Y() );
    }
    else
    {
        return maPosition;
    }
}


void SAL_CALL SvxShape::setPosition( const awt::Point& Position )
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
    {
        // do NOT move 3D objects, this would change the homogen
        // transformation matrix
        if(dynamic_cast<const E3dCompoundObject* >(GetSdrObject()) == nullptr)
        {
            tools::Rectangle aRect( svx_getLogicRectHack(GetSdrObject()) );
            Point aLocalPos( Position.X, Position.Y );
            ForceMetricToItemPoolMetric(aLocalPos);

            // Position is absolute, so recalc to position relative to anchor
            if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                aLocalPos += GetSdrObject()->GetAnchorPos();

            tools::Long nDX = aLocalPos.X() - aRect.Left();
            tools::Long nDY = aLocalPos.Y() - aRect.Top();

            GetSdrObject()->Move( Size( nDX, nDY ) );
            GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
        }
    }

    maPosition = Position;
}


awt::Size SAL_CALL SvxShape::getSize()
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
    {
        tools::Rectangle aRect( svx_getLogicRectHack(GetSdrObject()) );
        Size aObjSize( aRect.getOpenWidth(), aRect.getOpenHeight() );
        ForceMetricTo100th_mm(aObjSize);
        return css::awt::Size( aObjSize.getWidth(), aObjSize.getHeight() );
    }
    else
        return maSize;
}


void SAL_CALL SvxShape::setSize( const awt::Size& rSize )
{
    ::SolarMutexGuard aGuard;

    if(HasSdrObject())
    {
        tools::Rectangle aRect( svx_getLogicRectHack(GetSdrObject()) );
        Size aLocalSize( rSize.Width, rSize.Height );
        ForceMetricToItemPoolMetric(aLocalSize);

        if(GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == SdrObjKind::Measure )
        {
            Fraction aWdt(aLocalSize.Width(),aRect.Right()-aRect.Left());
            Fraction aHgt(aLocalSize.Height(),aRect.Bottom()-aRect.Top());
            Point aPt = GetSdrObject()->GetSnapRect().TopLeft();
            GetSdrObject()->Resize(aPt,aWdt,aHgt);
        }
        else
        {
            //aRect.SetSize(aLocalSize); // this call subtract 1 // https://bz.apache.org/ooo/show_bug.cgi?id=83193
            if ( !aLocalSize.Width() )
            {
                aRect.SetWidthEmpty();
            }
            else
                aRect.setWidth(aLocalSize.Width());
            if ( !aLocalSize.Height() )
            {
                aRect.SetHeightEmpty();
            }
            else
                aRect.setHeight(aLocalSize.Height());

            svx_setLogicRectHack( GetSdrObject(), aRect );
        }

        GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
    }
    maSize = rSize;
}


// XNamed
OUString SAL_CALL SvxShape::getName(  )
{
    ::SolarMutexGuard aGuard;
    if( HasSdrObject() )
    {
        return GetSdrObject()->GetName();
    }
    else
    {
        return maShapeName;
    }
}


void SAL_CALL SvxShape::setName( const OUString& aName )
{
    ::SolarMutexGuard aGuard;
    if( HasSdrObject() )
    {
        GetSdrObject()->SetName( aName );
    }
    else
    {
        maShapeName = aName;
    }
}

// XShapeDescriptor


OUString SAL_CALL SvxShape::getShapeType()
{
    if( !maShapeType.getLength() )
        return UHashMap::getNameFromId( mpImpl->mnObjId );
    else
        return maShapeType;
}

// XComponent


void SAL_CALL SvxShape::dispose()
{
    std::unique_lock g(m_aMutex);

    if( mpImpl->mbDisposing )
        return; // caught a recursion

    mpImpl->mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *static_cast<OWeakAggObject*>(this);
    mpImpl->maDisposeListeners.disposeAndClear(g, aEvt);
    mpImpl->maPropertyChangeListeners.disposeAndClear(g, aEvt);

    rtl::Reference<SdrObject> pObject = mxSdrObject;
    if (!pObject)
        return;

    EndListening( pObject->getSdrModelFromSdrObject() );

    if ( pObject->IsInserted() && pObject->getSdrPageFromSdrObject() )
    {
        SdrPage* pPage = pObject->getSdrPageFromSdrObject();
        // delete the SdrObject from the page
        const size_t nCount = pPage->GetObjCount();
        for ( size_t nNum = 0; nNum < nCount; ++nNum )
        {
            if ( pPage->GetObj( nNum ) == pObject.get() )
            {
                OSL_VERIFY( pPage->RemoveObject( nNum ) == pObject );
                break;
            }
        }
    }

    mxSdrObject.clear();
    pObject->setUnoShape(nullptr);
}


void SAL_CALL SvxShape::addEventListener( const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    mpImpl->maDisposeListeners.addInterface(g, xListener);
}


void SAL_CALL SvxShape::removeEventListener( const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    mpImpl->maDisposeListeners.removeInterface(g, aListener);
}

// XPropertySet


Reference< beans::XPropertySetInfo > SAL_CALL
    SvxShape::getPropertySetInfo()
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertySetInfo();
    }
    else
    {
        return _getPropertySetInfo();
    }
}

Reference< beans::XPropertySetInfo > const &
    SvxShape::_getPropertySetInfo()
{
    return mpPropSet->getPropertySetInfo();
}


void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& _propertyName, const Reference< beans::XPropertyChangeListener >& _listener  )
{
    std::unique_lock g(m_aMutex);
    mpImpl->maPropertyChangeListeners.addInterface( g, _propertyName, _listener );
}


void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& _propertyName, const Reference< beans::XPropertyChangeListener >& _listener  )
{
    std::unique_lock g(m_aMutex);
    mpImpl->maPropertyChangeListeners.removeInterface( g, _propertyName, _listener );
}


void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  )
{
    OSL_FAIL( "SvxShape::addVetoableChangeListener: don't have any vetoable properties, so why ...?" );
}


void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  )
{
    OSL_FAIL( "SvxShape::removeVetoableChangeListener: don't have any vetoable properties, so why ...?" );
}


bool SvxShape::SetFillAttribute( sal_uInt16 nWID, const OUString& rName )
{
    if(HasSdrObject())
    {
        SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), nWID, nWID );

        if( SetFillAttribute( nWID, rName, aSet, &GetSdrObject()->getSdrModelFromSdrObject() ) )
        {
            //GetSdrObject()->SetItemSetAndBroadcast(aSet);
            GetSdrObject()->SetMergedItemSetAndBroadcast(aSet);

            return true;
        }
    }

    return false;
}


bool SvxShape::SetFillAttribute( sal_uInt16 nWID, const OUString& rName, SfxItemSet& rSet, SdrModel const * pModel )
{
    // check if an item with the given name and which id is inside the models
    // pool or the stylesheet pool, if found it's put in the itemset
    if( !SetFillAttribute( nWID, rName, rSet ) )
    {
        // we did not find such item in one of the pools, so we check
        // the property lists that are loaded for the model for items
        // that support such.
        OUString aStrName = SvxUnogetInternalNameForItem(nWID, rName);

        switch( nWID )
        {
        case XATTR_FILLBITMAP:
        {
            XBitmapListRef pBitmapList = pModel->GetBitmapList();

            if( !pBitmapList.is() )
                return false;

            tools::Long nPos = pBitmapList->GetIndex(aStrName);
            if( nPos == -1 )
                return false;

            const XBitmapEntry* pEntry = pBitmapList->GetBitmap(nPos);
            XFillBitmapItem aBmpItem(rName, pEntry->GetGraphicObject());
            rSet.Put(aBmpItem);
            break;
        }
        case XATTR_FILLGRADIENT:
        {
            XGradientListRef pGradientList = pModel->GetGradientList();

            if( !pGradientList.is() )
                return false;

            tools::Long nPos = pGradientList->GetIndex(aStrName);
            if( nPos == -1 )
                return false;

            const XGradientEntry* pEntry = pGradientList->GetGradient(nPos);
            XFillGradientItem aGrdItem(rName, pEntry->GetGradient());
            rSet.Put( aGrdItem );
            break;
        }
        case XATTR_FILLHATCH:
        {
            XHatchListRef pHatchList = pModel->GetHatchList();

            if( !pHatchList.is() )
                return false;

            tools::Long nPos = pHatchList->GetIndex(aStrName);
            if( nPos == -1 )
                return false;

            const XHatchEntry* pEntry = pHatchList->GetHatch( nPos );
            XFillHatchItem aHatchItem(rName, pEntry->GetHatch());
            rSet.Put( aHatchItem );
            break;
        }
        case XATTR_LINEEND:
        case XATTR_LINESTART:
        {
            XLineEndListRef pLineEndList = pModel->GetLineEndList();

            if( !pLineEndList.is() )
                return false;

            tools::Long nPos = pLineEndList->GetIndex(aStrName);
            if( nPos == -1 )
                return false;

            const XLineEndEntry* pEntry = pLineEndList->GetLineEnd(nPos);
            if( sal_uInt16(XATTR_LINEEND) == nWID )
            {
                XLineEndItem aLEItem(rName, pEntry->GetLineEnd());
                rSet.Put( aLEItem );
            }
            else
            {
                XLineStartItem aLSItem(rName, pEntry->GetLineEnd());
                rSet.Put( aLSItem );
            }

            break;
        }
        case XATTR_LINEDASH:
        {
            XDashListRef pDashList = pModel->GetDashList();

            if( !pDashList.is() )
                return false;

            tools::Long nPos = pDashList->GetIndex(aStrName);
            if( nPos == -1 )
                return false;

            const XDashEntry* pEntry = pDashList->GetDash(nPos);
            XLineDashItem aDashItem(rName, pEntry->GetDash());
            rSet.Put( aDashItem );
            break;
        }
        default:
            return false;
        }
    }

    return true;
}


bool SvxShape::SetFillAttribute( sal_uInt16 nWID, const OUString& rName, SfxItemSet& rSet )
{
    OUString aName = SvxUnogetInternalNameForItem(nWID, rName);

    if (aName.isEmpty())
    {
        switch( nWID )
        {
        case XATTR_LINEEND:
        case XATTR_LINESTART:
            {
                const basegfx::B2DPolyPolygon aEmptyPoly;
                if( nWID == sal_uInt16(XATTR_LINEEND) )
                    rSet.Put( XLineEndItem( u""_ustr, aEmptyPoly ) );
                else
                    rSet.Put( XLineStartItem( u""_ustr, aEmptyPoly ) );

                return true;
            }
        case XATTR_FILLFLOATTRANSPARENCE:
            {
                // #85953# Set a disabled XFillFloatTransparenceItem
                rSet.Put(XFillFloatTransparenceItem());

                return true;
            }
        }

        return false;
    }

    ItemSurrogates aSurrogates;
    rSet.GetPool()->GetItemSurrogates(aSurrogates, nWID);
    for (const SfxPoolItem* p : aSurrogates)
    {
        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(p);
        if( pItem->GetName() == aName )
        {
            rSet.Put( *pItem );
            return true;
        }
    }

    return false;
}


void SAL_CALL SvxShape::setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
{
    if( mpImpl->mpMaster )
    {
        mpImpl->mpMaster->setPropertyValue( rPropertyName, rVal );
    }
    else
    {
        _setPropertyValue( rPropertyName, rVal );
    }
}

void SvxShape::_setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pMap = mpPropSet->getPropertyMapEntry(rPropertyName);

    if (!HasSdrObject())
    {
        // Since we have no actual sdr object right now, remember all
        // properties in a list. These properties will be set when the sdr
        // object is created.

        if (pMap && pMap->nWID)
        {
            // FIXME: We should throw a UnknownPropertyException here.
            //        But since this class is aggregated from classes that
            //        support additional properties that we don't know here we
            //        silently store *all* properties, even if they may be not
            //        supported after creation.
            SvxItemPropertySet::setPropertyValue( pMap, rVal, maUrsAnys );
        }

        return;
    }

    if (rPropertyName == "HandlePathObjScale")
    {
        auto pPathObj = dynamic_cast<SdrPathObj*>(GetSdrObject());
        if (pPathObj)
        {
            bool bHandleScale{};
            if (rVal >>= bHandleScale)
            {
                pPathObj->SetHandleScale(bHandleScale);
            }
        }
        return;
    }

    if (!pMap)
        throw beans::UnknownPropertyException( rPropertyName, getXWeak());

    if ((pMap->nFlags & beans::PropertyAttribute::READONLY) != 0)
        throw beans::PropertyVetoException(
            "Readonly property can't be set: " + rPropertyName,
            uno::Reference<drawing::XShape>(this));

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();

    if (setPropertyValueImpl(rPropertyName, pMap, rVal))
        return;

    DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST, "Not persist item not handled!" );
    DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

    bool bIsNotPersist = pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST && pMap->nWID != SDRATTR_TEXTDIRECTION;

    if( pMap->nWID == SDRATTR_CORNER_RADIUS )
    {
        sal_Int32 nCornerRadius = 0;
        if( !(rVal >>= nCornerRadius) || (nCornerRadius < 0) || (nCornerRadius > 5000000))
            throw IllegalArgumentException();
    }

    std::optional<SfxItemSet> xLocalSet;
    SfxItemSet* pSet;
    if( mbIsMultiPropertyCall && !bIsNotPersist )
    {
        if( !mpImpl->moItemSet )
        {
            mpImpl->moItemSet.emplace( GetSdrObject()->GetProperties().CreateObjectSpecificItemSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool() ) );
        }
        pSet = &*mpImpl->moItemSet;
    }
    else
    {
        xLocalSet.emplace( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), pMap->nWID, pMap->nWID);
        pSet = &*xLocalSet;
    }

    if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        pSet->Put(GetSdrObject()->GetMergedItem(pMap->nWID));

    if( !SvxUnoTextRangeBase::SetPropertyValueHelper( pMap, rVal, *pSet ))
    {
        if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        {
            if(bIsNotPersist)
            {
                // not-persistent attribute, get those extra
                GetSdrObject()->TakeNotPersistAttr(*pSet);
            }
        }

        if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        {
            // get default from ItemPool
            if(SfxItemPool::IsWhich(pMap->nWID))
                pSet->Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetUserOrPoolDefaultItem(pMap->nWID));
        }

        if( pSet->GetItemState( pMap->nWID ) == SfxItemState::SET )
        {
            SvxItemPropertySet_setPropertyValue( pMap, rVal, *pSet );
        }
    }

    if(bIsNotPersist)
    {
        // set not-persistent attribute extra
        GetSdrObject()->ApplyNotPersistAttr( *pSet );
    }
    else
    {
        // if we have a XMultiProperty call then the item set
        // will be set in setPropertyValues later
        if( !mbIsMultiPropertyCall )
            GetSdrObject()->SetMergedItemSetAndBroadcast( *pSet );
    }
}


uno::Any SAL_CALL SvxShape::getPropertyValue( const OUString& PropertyName )
{
    if ( mpImpl->mpMaster )
        return mpImpl->mpMaster->getPropertyValue( PropertyName );
    else
        return _getPropertyValue( PropertyName );
}


uno::Any SvxShape::_getPropertyValue( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    uno::Any aAny;
    if(HasSdrObject())
    {
        if(pMap == nullptr )
            throw beans::UnknownPropertyException( PropertyName, getXWeak());

        if( !getPropertyValueImpl( PropertyName, pMap, aAny ) )
        {
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || (pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST), "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), pMap->nWID, pMap->nWID );
            aSet.Put(GetSdrObject()->GetMergedItem(pMap->nWID));

            if(SvxUnoTextRangeBase::GetPropertyValueHelper(  aSet, pMap, aAny ))
                return aAny;

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // not-persistent attribute, get those extra
                    GetSdrObject()->TakeNotPersistAttr(aSet);
                }
            }

            if(!aSet.Count())
            {
                // get default from ItemPool
                if(SfxItemPool::IsWhich(pMap->nWID))
                    aSet.Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetUserOrPoolDefaultItem(pMap->nWID));
            }

            if(aSet.Count())
                aAny = GetAnyForItem( aSet, pMap );
        }
    }
    else
    {

// Fixme: we should return default values for OWN_ATTR !

        if(pMap && pMap->nWID)
//      FixMe: see setPropertyValue
            aAny = mpPropSet->getPropertyValue( pMap, maUrsAnys );

    }
    return aAny;
}


// XMultiPropertySet
void SAL_CALL SvxShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues )
{
    ::SolarMutexGuard aSolarGuard;

    const sal_Int32 nCount = aPropertyNames.getLength();
    if (nCount != aValues.getLength())
        throw css::lang::IllegalArgumentException(u"lengths do not match"_ustr,
                                                  getXWeak(), -1);

    const OUString* pNames = aPropertyNames.getConstArray();
    const uno::Any* pValues = aValues.getConstArray();

    // make sure mbIsMultiPropertyCall and mpImpl->mpItemSet are
    // reset even when an exception is thrown
    const ::comphelper::ScopeGuard aGuard( [this] () { return this->endSetPropertyValues(); } );

    mbIsMultiPropertyCall = true;

    if( mpImpl->mpMaster )
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pValues++ )
        {
            try
            {
                setPropertyValue( *pNames, *pValues );
            }
            catch (beans::UnknownPropertyException&)
            {
                // ignore, various code likes to opportunistically set properties on objects that don't support those properties
            }
            catch (uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xSet;
        queryInterface( cppu::UnoType<beans::XPropertySet>::get()) >>= xSet;

        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pNames++, pValues++ )
        {
            try
            {
                xSet->setPropertyValue( *pNames, *pValues );
            }
            catch (beans::UnknownPropertyException&)
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
            catch (uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
    }

    if( mpImpl->moItemSet && HasSdrObject() )
        GetSdrObject()->SetMergedItemSetAndBroadcast( *mpImpl->moItemSet );
}


void SvxShape::endSetPropertyValues()
{
    mbIsMultiPropertyCall = false;
    mpImpl->moItemSet.reset();
}


css::uno::Sequence< css::uno::Any > SAL_CALL SvxShape::getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames )
{
    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    uno::Sequence< uno::Any > aRet( nCount );
    uno::Any* pValue = aRet.getArray();

    if( mpImpl->mpMaster )
    {
        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++, pNames++ )
        {
            try
            {
                *pValue = getPropertyValue( *pNames );
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "SvxShape::getPropertyValues, unknown property asked" );
            }
        }
    }
    else
    {
        uno::Reference< beans::XPropertySet > xSet;
        queryInterface( cppu::UnoType<beans::XPropertySet>::get()) >>= xSet;

        for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++, pValue++, pNames++ )
        {
            try
            {
                *pValue = xSet->getPropertyValue( *pNames );
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "SvxShape::getPropertyValues, unknown property asked" );
            }
        }
    }

    return aRet;
}

void SAL_CALL SvxShape::addPropertiesChangeListener( const css::uno::Sequence< OUString >& , const css::uno::Reference< css::beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SvxShape::removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >&  )
{
}

void SAL_CALL SvxShape::firePropertiesChangeEvent( const css::uno::Sequence< OUString >& , const css::uno::Reference< css::beans::XPropertiesChangeListener >&  )
{
}


uno::Any SvxShape::GetAnyForItem( SfxItemSet const & aSet, const SfxItemPropertyMapEntry* pMap ) const
{
    DBG_TESTSOLARMUTEX();
    uno::Any aAny;

    switch(pMap->nWID)
    {
    case SDRATTR_CIRCSTARTANGLE:
    {
        if(const SdrAngleItem* pPoolItem = aSet.GetItemIfSet(SDRATTR_CIRCSTARTANGLE,false))
        {
            Degree100 nAngle = pPoolItem->GetValue();
            aAny <<= nAngle.get();
        }
        break;
    }

    case SDRATTR_CIRCENDANGLE:
    {
        if (const SdrAngleItem* pPoolItem = aSet.GetItemIfSet(SDRATTR_CIRCENDANGLE,false))
        {
            Degree100 nAngle = pPoolItem->GetValue();
            aAny <<= nAngle.get();
        }
        break;
    }

    case SDRATTR_CIRCKIND:
    {
        if( GetSdrObject()->GetObjInventor() == SdrInventor::Default)
        {
            drawing::CircleKind eKind;
            switch(GetSdrObject()->GetObjIdentifier())
            {
            case SdrObjKind::CircleOrEllipse:          // circle, ellipse
                eKind = drawing::CircleKind_FULL;
                break;
            case SdrObjKind::CircleCut:          // segment of circle
                eKind = drawing::CircleKind_CUT;
                break;
            case SdrObjKind::CircleArc:          // arc of circle
                eKind = drawing::CircleKind_ARC;
                break;
            case SdrObjKind::CircleSection:          // sector
                eKind = drawing::CircleKind_SECTION;
                break;
            default:
                break;
            }
            aAny <<= eKind;
        }
        break;
    }
    default:
    {
        // get value from ItemSet
        aAny = SvxItemPropertySet_getPropertyValue( pMap, aSet );

        if( pMap->aType != aAny.getValueType() )
        {
            // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
            if( ( pMap->aType == ::cppu::UnoType<sal_Int16>::get()) && aAny.getValueType() == ::cppu::UnoType<sal_Int32>::get() )
            {
                sal_Int32 nValue = 0;
                aAny >>= nValue;
                aAny <<= static_cast<sal_Int16>(nValue);
            }
            else
            {
                SAL_WARN("svx", "SvxShape::GetAnyForItem() Return value has wrong Type, " << pMap->aType << " != " << aAny.getValueType());
            }
        }

    }
    }

    return aAny;
}


// XPropertyState
beans::PropertyState SAL_CALL SvxShape::getPropertyState( const OUString& PropertyName )
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertyState( PropertyName );
    }
    else
    {
        return _getPropertyState( PropertyName );
    }
}

beans::PropertyState SvxShape::_getPropertyState( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !HasSdrObject() || pMap == nullptr )
        throw beans::UnknownPropertyException( PropertyName, getXWeak());

    beans::PropertyState eState;
    if( !getPropertyStateImpl( pMap, eState ) )
    {
        const SfxItemSet& rSet = GetSdrObject()->GetMergedItemSet();

        switch( rSet.GetItemState( pMap->nWID, false ) )
        {
        case SfxItemState::SET:
            eState = beans::PropertyState_DIRECT_VALUE;
            break;
        case SfxItemState::DEFAULT:
            eState = beans::PropertyState_DEFAULT_VALUE;
            break;
        default:
            eState = beans::PropertyState_AMBIGUOUS_VALUE;
            break;
        }

        // if an item is set, this doesn't mean we want it :)
        if( beans::PropertyState_DIRECT_VALUE == eState )
        {
            switch( pMap->nWID )
            {
            // the following items are disabled by changing the
            // fill style or the line style. so there is no need
            // to export items without names which should be empty
            case XATTR_FILLBITMAP:
            case XATTR_FILLGRADIENT:
            case XATTR_FILLHATCH:
            case XATTR_LINEDASH:
                {
                    const NameOrIndex* pItem = rSet.GetItem<NameOrIndex>(pMap->nWID);
                    if( ( pItem == nullptr ) || pItem->GetName().isEmpty() )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
                break;

            // #i36115#
            // If e.g. the LineStart is on NONE and thus the string has length 0, it still
            // may be a hard attribute covering the set LineStart of the parent (Style).
            // #i37644#
            // same is for fill float transparency
            case XATTR_LINEEND:
            case XATTR_LINESTART:
            case XATTR_FILLFLOATTRANSPARENCE:
                {
                    const NameOrIndex* pItem = rSet.GetItem<NameOrIndex>(pMap->nWID);
                    if ( pItem == nullptr )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
                break;
            case XATTR_FILLCOLOR:
                if (pMap->nMemberId == MID_COLOR_THEME_INDEX)
                {
                    const XFillColorItem* pColor = rSet.GetItem<XFillColorItem>(pMap->nWID);
                    if (!pColor->getComplexColor().isValidThemeType())
                    {
                        eState = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pMap->nMemberId == MID_COLOR_LUM_MOD)
                {
                    const XFillColorItem* pColor = rSet.GetItem<XFillColorItem>(pMap->nWID);
                    sal_Int16 nLumMod = 10000;
                    for (auto const& rTransform : pColor->getComplexColor().getTransformations())
                    {
                        if (rTransform.meType == model::TransformationType::LumMod)
                            nLumMod = rTransform.mnValue;
                    }
                    if (nLumMod == 10000)
                    {
                        eState = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pMap->nMemberId == MID_COLOR_LUM_OFF)
                {
                    const XFillColorItem* pColor = rSet.GetItem<XFillColorItem>(pMap->nWID);
                    sal_Int16 nLumOff = 0;
                    for (auto const& rTransform : pColor->getComplexColor().getTransformations())
                    {
                        if (rTransform.meType == model::TransformationType::LumOff)
                            nLumOff = rTransform.mnValue;
                    }
                    if (nLumOff == 0)
                    {
                        eState = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
                else if (pMap->nMemberId == MID_COMPLEX_COLOR)
                {
                    auto const* pColor = rSet.GetItem<XFillColorItem>(pMap->nWID);
                    if (pColor->getComplexColor().getType() == model::ColorType::Unused)
                    {
                        eState = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
                break;
            case XATTR_LINECOLOR:
                if (pMap->nMemberId == MID_COMPLEX_COLOR)
                {
                    auto const* pColor = rSet.GetItem<XLineColorItem>(pMap->nWID);
                    if (pColor->getComplexColor().getType() == model::ColorType::Unused)
                    {
                        eState = beans::PropertyState_DEFAULT_VALUE;
                    }
                }
                break;
            }
        }
    }
    return eState;
}

bool SvxShape::setPropertyValueImpl( const OUString&, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue )
{
    rtl::Reference<SdrObject> pSdrObject = GetSdrObject();
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CAPTION_POINT:
    {
        awt::Point aPnt;
        if( rValue >>= aPnt )
        {
            Point aVclPoint( aPnt.X, aPnt.Y );

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            // Need to adapt aVclPoint from 100thmm to app-specific
            ForceMetricToItemPoolMetric(aVclPoint);

            // #90763# position is relative to top left, make it absolute
            basegfx::B2DPolyPolygon aNewPolyPolygon;
            basegfx::B2DHomMatrix aNewHomogenMatrix;
            pSdrObject->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aVclPoint.AdjustX(basegfx::fround<tools::Long>(aNewHomogenMatrix.get(0, 2)) );
            aVclPoint.AdjustY(basegfx::fround<tools::Long>(aNewHomogenMatrix.get(1, 2)) );

            // #88491# position relative to anchor
            if( pSdrObject->getSdrModelFromSdrObject().IsWriter() )
            {
                aVclPoint += pSdrObject->GetAnchorPos();
            }

            static_cast<SdrCaptionObj*>(pSdrObject.get())->SetTailPos(aVclPoint);

            return true;
        }
        break;
    }
    case OWN_ATTR_TRANSFORMATION:
    {
        drawing::HomogenMatrix3 aMatrix;
        if(rValue >>= aMatrix)
        {
            basegfx::B2DPolyPolygon aNewPolyPolygon;
            basegfx::B2DHomMatrix aNewHomogenMatrix;

            // tdf#117145 SdrModel data is app-specific
            pSdrObject->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aNewHomogenMatrix.set(0, 0, aMatrix.Line1.Column1);
            aNewHomogenMatrix.set(0, 1, aMatrix.Line1.Column2);
            aNewHomogenMatrix.set(0, 2, aMatrix.Line1.Column3);
            aNewHomogenMatrix.set(1, 0, aMatrix.Line2.Column1);
            aNewHomogenMatrix.set(1, 1, aMatrix.Line2.Column2);
            aNewHomogenMatrix.set(1, 2, aMatrix.Line2.Column3);
            // For this to be a valid 2D transform matrix, the last row must be [0,0,1]
            assert( aMatrix.Line3.Column1 == 0 );
            assert( aMatrix.Line3.Column2 == 0 );
            assert( aMatrix.Line3.Column3 == 1 );

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            // Need to adapt aNewHomogenMatrix from 100thmm to app-specific
            ForceMetricToItemPoolMetric(aNewHomogenMatrix);

            pSdrObject->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            return true;
        }
        break;
    }

    case OWN_ATTR_ZORDER:
    {
        sal_Int32 nNewOrdNum = 0;
        if(rValue >>= nNewOrdNum)
        {
            SdrObjList* pObjList = pSdrObject->getParentSdrObjListFromSdrObject();
            if( pObjList )
                pObjList->SetExistingObjectOrdNum( pSdrObject.get(), static_cast<size_t>(nNewOrdNum) );
            return true;
        }
        break;
    }
    case OWN_ATTR_FRAMERECT:
    {
        awt::Rectangle aUnoRect;
        if(rValue >>= aUnoRect)
        {
            Point aTopLeft( aUnoRect.X, aUnoRect.Y );
            Size aObjSize( aUnoRect.Width, aUnoRect.Height );
            ForceMetricToItemPoolMetric(aTopLeft);
            ForceMetricToItemPoolMetric(aObjSize);
            tools::Rectangle aRect(aTopLeft, aObjSize);
            pSdrObject->SetSnapRect(aRect);
            return true;
        }
        break;
    }
    case OWN_ATTR_MIRRORED:
    {
        bool bMirror;
        if(rValue >>= bMirror )
        {
            SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >( pSdrObject.get() );
            if( pObj )
                pObj->SetMirrored(bMirror);
            return true;
        }
        break;
    }
    case OWN_ATTR_EDGE_START_OBJ:
    case OWN_ATTR_EDGE_END_OBJ:
    case OWN_ATTR_GLUEID_HEAD:
    case OWN_ATTR_GLUEID_TAIL:
    case OWN_ATTR_EDGE_START_POS:
    case OWN_ATTR_EDGE_END_POS:
    case OWN_ATTR_EDGE_POLYPOLYGONBEZIER:
    {
        SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >(pSdrObject.get());
        if(pEdgeObj)
        {
            switch(pProperty->nWID)
            {
            case OWN_ATTR_EDGE_START_OBJ:
            case OWN_ATTR_EDGE_END_OBJ:
                {
                    Reference< drawing::XShape > xShape;
                    if( rValue >>= xShape )
                    {
                        SdrObject* pNode = SdrObject::getSdrObjectFromXShape(xShape);
                        if( pNode )
                        {
                            pEdgeObj->ConnectToNode( pProperty->nWID == OWN_ATTR_EDGE_START_OBJ, pNode );
                            pEdgeObj->setGluePointIndex( pProperty->nWID == OWN_ATTR_EDGE_START_OBJ );
                            return true;
                        }
                    }
                    break;
                }

            case OWN_ATTR_EDGE_START_POS:
            case OWN_ATTR_EDGE_END_POS:
                {
                    awt::Point aUnoPoint;
                    if( rValue >>= aUnoPoint )
                    {
                        Point aPoint( aUnoPoint.X, aUnoPoint.Y );

                        // Reintroduction of fix for issue i59051 (#i108851#)
                        // perform metric change before applying anchor position,
                        // because the anchor position is in pool metric.
                        ForceMetricToItemPoolMetric( aPoint );
                        if( pSdrObject->getSdrModelFromSdrObject().IsWriter() )
                            aPoint += pSdrObject->GetAnchorPos();

                        pEdgeObj->SetTailPoint( pProperty->nWID == OWN_ATTR_EDGE_START_POS, aPoint );
                        return true;
                    }
                    break;
                }

            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
                {
                    sal_Int32 nId = 0;
                    if( rValue >>= nId )
                    {
                        pEdgeObj->setGluePointIndex( pProperty->nWID == OWN_ATTR_GLUEID_HEAD, nId );
                        return true;
                    }
                    break;
                }
            case OWN_ATTR_EDGE_POLYPOLYGONBEZIER:
                {
                    basegfx::B2DPolyPolygon aNewPolyPolygon;

                    // #123616# be a little bit more flexible regarding the data type used
                    if( auto s = o3tl::tryAccess<drawing::PointSequenceSequence>(rValue) )
                    {
                        // get polygpon data from PointSequenceSequence
                        aNewPolyPolygon = basegfx::utils::UnoPointSequenceSequenceToB2DPolyPolygon(
                            *s);
                    }
                    else if( auto cs = o3tl::tryAccess<drawing::PolyPolygonBezierCoords>(rValue) )
                    {
                        // get polygpon data from PolyPolygonBezierCoords
                        aNewPolyPolygon = basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
                            *cs);
                    }

                    if(aNewPolyPolygon.count())
                    {
                        // Reintroduction of fix for issue i59051 (#i108851#)
                        ForceMetricToItemPoolMetric( aNewPolyPolygon );
                        if( pSdrObject->getSdrModelFromSdrObject().IsWriter() )
                        {
                            Point aPoint( pSdrObject->GetAnchorPos() );
                            aNewPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(aPoint.X(), aPoint.Y()));
                        }
                        pEdgeObj->SetEdgeTrackPath( aNewPolyPolygon );
                        return true;
                    }
                }
            }
        }
        break;
    }
    case OWN_ATTR_MEASURE_START_POS:
    case OWN_ATTR_MEASURE_END_POS:
    {
        SdrMeasureObj* pMeasureObj = dynamic_cast< SdrMeasureObj* >(pSdrObject.get());
        awt::Point aUnoPoint;
        if(pMeasureObj && ( rValue >>= aUnoPoint ) )
        {
            Point aPoint( aUnoPoint.X, aUnoPoint.Y );

            // Reintroduction of fix for issue #i59051# (#i108851#)
            ForceMetricToItemPoolMetric( aPoint );
            if( pSdrObject->getSdrModelFromSdrObject().IsWriter() )
                aPoint += pSdrObject->GetAnchorPos();

            pMeasureObj->NbcSetPoint( aPoint, pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 );
            pMeasureObj->SetChanged();
            pMeasureObj->BroadcastObjectChange();
            return true;
        }
        break;
    }
    case OWN_ATTR_FILLBMP_MODE:
        {
            drawing::BitmapMode eMode;
            if(!(rValue >>= eMode) )
            {
                sal_Int32 nMode = 0;
                if(!(rValue >>= nMode))
                    break;

                eMode = static_cast<drawing::BitmapMode>(nMode);
            }
            pSdrObject->SetMergedItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
            pSdrObject->SetMergedItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
            return true;
        }

    case SDRATTR_LAYERID:
    {
        sal_Int16 nLayerId = sal_Int16();
        if( rValue >>= nLayerId )
        {
            SdrLayer* pLayer = pSdrObject->getSdrModelFromSdrObject().GetLayerAdmin().GetLayerPerID(SdrLayerID(nLayerId));
            if( pLayer )
            {
                pSdrObject->SetLayer(SdrLayerID(nLayerId));
                return true;
            }
        }
        break;
    }

    case SDRATTR_LAYERNAME:
    {
        OUString aLayerName;
        if( rValue >>= aLayerName )
        {
            const SdrLayer* pLayer = pSdrObject->getSdrModelFromSdrObject().GetLayerAdmin().GetLayer(aLayerName);
            if( pLayer != nullptr )
            {
                pSdrObject->SetLayer( pLayer->GetID() );
                return true;
            }
        }
        break;
    }
    case SDRATTR_ROTATEANGLE:
    {
        sal_Int32 nTmp = 0;
        if( rValue >>= nTmp )
        {
            Degree100 nAngle(nTmp);
            Point aRef1(pSdrObject->GetSnapRect().Center());
            nAngle -= pSdrObject->GetRotateAngle();
            if (nAngle)
            {
                double nSin = sin(toRadians(nAngle));
                double nCos = cos(toRadians(nAngle));
                pSdrObject->Rotate(aRef1,nAngle,nSin,nCos);
            }
            return true;
        }

        break;
    }

    case SDRATTR_SHEARANGLE:
    {
        sal_Int32 nTmp = 0;
        if( rValue >>= nTmp )
        {
            Degree100 nShear(nTmp);
            nShear -= pSdrObject->GetShearAngle();
            if(nShear)
            {
                Point aRef1(pSdrObject->GetSnapRect().Center());
                double nTan = tan(toRadians(nShear));
                pSdrObject->Shear(aRef1,nShear,nTan,false);
                return true;
            }
        }

        break;
    }

    case OWN_ATTR_INTEROPGRABBAG:
    {
        pSdrObject->SetGrabBagItem(rValue);
        return true;
    }

    case SDRATTR_OBJMOVEPROTECT:
    {
        bool bMoveProtect;
        if( rValue >>= bMoveProtect )
        {
            pSdrObject->SetMoveProtect(bMoveProtect);
            return true;
        }
        break;
    }
    case SDRATTR_OBJECTNAME:
    {
        OUString aName;
        if( rValue >>= aName )
        {
            pSdrObject->SetName( aName );
            return true;
        }
        break;
    }

    case OWN_ATTR_TEXTFITTOSIZE_FONT_SCALE:
    {
        double fScale = 0.0;
        if (rValue >>= fScale)
        {
            SdrTextFitToSizeTypeItem aItem(pSdrObject->GetMergedItem(SDRATTR_TEXT_FITTOSIZE));
            aItem.setFontScale(fScale / 100.0);
            pSdrObject->SetMergedItem(aItem);
            return true;
        }
        break;
    }

    case OWN_ATTR_TEXTFITTOSIZE_SPACING_SCALE:
    {
        double fScale = 0.0;
        if (rValue >>= fScale)
        {
            SdrTextFitToSizeTypeItem aItem(pSdrObject->GetMergedItem(SDRATTR_TEXT_FITTOSIZE));
            aItem.setSpacingScale(fScale / 100.0);
            pSdrObject->SetMergedItem(aItem);
            return true;
        }
        break;
    }

    // #i68101#
    case OWN_ATTR_MISC_OBJ_TITLE:
    {
        OUString aTitle;
        if( rValue >>= aTitle )
        {
            pSdrObject->SetTitle( aTitle );
            return true;
        }
        break;
    }
    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription;
        if( rValue >>= aDescription )
        {
            pSdrObject->SetDescription( aDescription );
            return true;
        }
        break;
    }
    case OWN_ATTR_MISC_OBJ_DECORATIVE:
    {
        bool isDecorative;
        if (rValue >>= isDecorative)
        {
            pSdrObject->SetDecorative(isDecorative);
            return true;
        }
        break;
    }

    case SDRATTR_OBJPRINTABLE:
    {
        bool bPrintable;
        if( rValue >>= bPrintable )
        {
            pSdrObject->SetPrintable(bPrintable);
            return true;
        }
        break;
    }
    case SDRATTR_OBJVISIBLE:
    {
        bool bVisible;
        if( rValue >>= bVisible )
        {
            pSdrObject->SetVisible(bVisible);
            return true;
        }
        break;
    }
    case SDRATTR_OBJSIZEPROTECT:
    {
        bool bResizeProtect;
        if( rValue >>= bResizeProtect )
        {
            pSdrObject->SetResizeProtect(bResizeProtect);
            return true;
        }
        break;
    }
    case OWN_ATTR_PAGE_NUMBER:
    {
        sal_Int32 nPageNum = 0;
        if( (rValue >>= nPageNum) && ( nPageNum >= 0 ) && ( nPageNum <= 0xffff ) )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(pSdrObject.get());
            if( pPageObj )
            {
                SdrModel& rModel(pPageObj->getSdrModelFromSdrObject());
                SdrPage* pNewPage = nullptr;
                const sal_uInt16 nDestinationPageNum(static_cast<sal_uInt16>((nPageNum << 1) - 1));

                if(nDestinationPageNum < rModel.GetPageCount())
                {
                    pNewPage = rModel.GetPage(nDestinationPageNum);
                }

                pPageObj->SetReferencedPage(pNewPage);
            }

            return true;
        }
        break;
    }
    case XATTR_FILLBITMAP:
    case XATTR_FILLGRADIENT:
    case XATTR_FILLHATCH:
    case XATTR_FILLFLOATTRANSPARENCE:
    case XATTR_LINEEND:
    case XATTR_LINESTART:
    case XATTR_LINEDASH:
    {
        if( pProperty->nMemberId == MID_NAME )
        {
            OUString aApiName;
            if( rValue >>= aApiName )
            {
                if( SetFillAttribute( pProperty->nWID, aApiName ) )
                    return true;
            }
            break;
        }
        else
        {
            return false;
        }
    }

    case OWN_ATTR_TEXTCOLUMNS:
    {
        if (auto pTextObj = DynCastSdrTextObj(pSdrObject.get()))
        {
            css::uno::Reference<css::text::XTextColumns> xTextColumns;
            if (rValue >>= xTextColumns)
            {
                pTextObj->SetTextColumnsNumber(xTextColumns->getColumnCount());
                if (css::uno::Reference<css::beans::XPropertySet> xPropSet{ xTextColumns,
                                                                            css::uno::UNO_QUERY })
                {
                    auto aVal = xPropSet->getPropertyValue(u"AutomaticDistance"_ustr);
                    if (sal_Int32 nSpacing; aVal >>= nSpacing)
                        pTextObj->SetTextColumnsSpacing(nSpacing);
                }
            }
        }
        return true;
    }

    case OWN_ATTR_HYPERLINK:
    {
        OUString sHyperlink;
        if (rValue >>= sHyperlink)
        {
            pSdrObject->setHyperlink(sHyperlink);
            return true;
        }
        break;
    }

    case SDRATTR_WRITINGMODE2:
    {
        SvxFrameDirectionItem aItem(SvxFrameDirection::Environment, SDRATTR_WRITINGMODE2);
        aItem.PutValue(rValue, 0);
        GetSdrObject()->SetMergedItem(aItem);
        return true;
        break;
    }

    default:
    {
        return false;
    }
    }

    OUString sExceptionMessage(
        "IllegalArgumentException in SvxShape::setPropertyValueImpl."
        " Property Type: "
        + pProperty->aType.getTypeName() + " Property Name: " + pProperty->aName
        + " Property nWID: " + OUString::number(pProperty->nWID)
        + " Value Type: " + (rValue.hasValue() ? rValue.getValueTypeName() : u"void (no value)"_ustr));

    throw lang::IllegalArgumentException(sExceptionMessage, nullptr, 1);
}


bool SvxShape::getPropertyValueImpl( const OUString&, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue )
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CAPTION_POINT:
    {
        Point aVclPoint = static_cast<SdrCaptionObj*>(GetSdrObject())->GetTailPos();

        // #88491# make pos relative to anchor
        if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
        {
            aVclPoint -= GetSdrObject()->GetAnchorPos();
        }

        // #90763# pos is absolute, make it relative to top left
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

        aVclPoint.AdjustX( -(basegfx::fround<tools::Long>(aNewHomogenMatrix.get(0, 2))) );
        aVclPoint.AdjustY( -(basegfx::fround<tools::Long>(aNewHomogenMatrix.get(1, 2))) );

        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        // Need to adapt aVclPoint from app-specific to 100thmm
        ForceMetricTo100th_mm(aVclPoint);

        awt::Point aPnt( aVclPoint.X(), aVclPoint.Y() );
        rValue <<= aPnt;
        break;
    }

    case OWN_ATTR_TRANSFORMATION:
    {
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
        drawing::HomogenMatrix3 aMatrix;

        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        // Need to adapt aNewHomogenMatrix from app-specific to 100thmm
        ForceMetricTo100th_mm(aNewHomogenMatrix);

        aMatrix.Line1.Column1 = aNewHomogenMatrix.get(0, 0);
        aMatrix.Line1.Column2 = aNewHomogenMatrix.get(0, 1);
        aMatrix.Line1.Column3 = aNewHomogenMatrix.get(0, 2);
        aMatrix.Line2.Column1 = aNewHomogenMatrix.get(1, 0);
        aMatrix.Line2.Column2 = aNewHomogenMatrix.get(1, 1);
        aMatrix.Line2.Column3 = aNewHomogenMatrix.get(1, 2);
        aMatrix.Line3.Column1 = 0;
        aMatrix.Line3.Column2 = 0;
        aMatrix.Line3.Column3 = 1;

        rValue <<= aMatrix;

        break;
    }

    case OWN_ATTR_ZORDER:
    {
        rValue <<= static_cast<sal_Int32>(GetSdrObject()->GetOrdNum());
        break;
    }

    case OWN_ATTR_BITMAP:
    {
        rValue = GetBitmap();
        if(!rValue.hasValue())
            throw uno::RuntimeException();

        break;
    }

    case OWN_ATTR_ISFONTWORK:
    {
        bool bIsFontwork = false;
        if (const SdrTextObj* pTextObj = DynCastSdrTextObj(GetSdrObject()))
            bIsFontwork = pTextObj->IsFontwork();
        rValue <<= bIsFontwork;
        break;
    }

    case OWN_ATTR_FRAMERECT:
    {
        tools::Rectangle aRect( GetSdrObject()->GetSnapRect() );
        Point aTopLeft( aRect.TopLeft() );
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        ForceMetricTo100th_mm(aTopLeft);
        ForceMetricTo100th_mm(aObjSize);
        css::awt::Rectangle aUnoRect(
            aTopLeft.X(), aTopLeft.Y(),
            aObjSize.getWidth(), aObjSize.getHeight() );
        rValue <<= aUnoRect;
        break;
    }

    case OWN_ATTR_BOUNDRECT:
    {
        tools::Rectangle aRect( GetSdrObject()->GetCurrentBoundRect() );
        Point aTopLeft( aRect.TopLeft() );
        Size aObjSize( aRect.GetWidth(), aRect.GetHeight() );
        ForceMetricTo100th_mm(aTopLeft);
        ForceMetricTo100th_mm(aObjSize);
        css::awt::Rectangle aUnoRect(
            aTopLeft.X(), aTopLeft.Y(),
            aObjSize.getWidth(), aObjSize.getHeight() );
        rValue <<= aUnoRect;
        break;
    }

    case OWN_ATTR_LDNAME:
    {
        OUString aName( GetSdrObject()->GetName() );
        rValue <<= aName;
        break;
    }

    case OWN_ATTR_LDBITMAP:
    {
        OUString sId;
        if( GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == SdrObjKind::OLE2 )
        {
            sId = RID_UNODRAW_OLE2;
        }
        else if( GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == SdrObjKind::Graphic )
        {
            sId = RID_UNODRAW_GRAPHICS;
        }
        else
        {
            sId = RID_UNODRAW_OBJECTS;
        }

        BitmapEx aBmp(sId);
        Reference<awt::XBitmap> xBmp(VCLUnoHelper::CreateBitmap(aBmp));

        rValue <<= xBmp;
        break;
    }

    case OWN_ATTR_MIRRORED:
    {
        bool bMirror = false;
        if( HasSdrObject() )
            if (auto pGrafObj = dynamic_cast<SdrGrafObj*>(GetSdrObject()) )
                bMirror = pGrafObj->IsMirrored();

        rValue <<= bMirror;
        break;
    }

    case OWN_ATTR_EDGE_START_OBJ:
    case OWN_ATTR_EDGE_START_POS:
    case OWN_ATTR_EDGE_END_POS:
    case OWN_ATTR_EDGE_END_OBJ:
    case OWN_ATTR_GLUEID_HEAD:
    case OWN_ATTR_GLUEID_TAIL:
    case OWN_ATTR_EDGE_POLYPOLYGONBEZIER:
    {
        SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(GetSdrObject());
        if(pEdgeObj)
        {
            switch(pProperty->nWID)
            {
            case OWN_ATTR_EDGE_START_OBJ:
            case OWN_ATTR_EDGE_END_OBJ:
                {
                    SdrObject* pNode = pEdgeObj->GetConnectedNode(pProperty->nWID == OWN_ATTR_EDGE_START_OBJ);
                    if(pNode)
                    {
                        Reference< drawing::XShape > xShape( GetXShapeForSdrObject( pNode ) );
                        if(xShape.is())
                            rValue <<= xShape;

                    }
                    break;
                }

            case OWN_ATTR_EDGE_START_POS:
            case OWN_ATTR_EDGE_END_POS:
                {
                    Point aPoint( pEdgeObj->GetTailPoint( pProperty->nWID == OWN_ATTR_EDGE_START_POS ) );
                    if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                        aPoint -= GetSdrObject()->GetAnchorPos();

                    ForceMetricTo100th_mm( aPoint );
                    awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

                    rValue <<= aUnoPoint;
                    break;
                }
            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
                {
                    rValue <<= pEdgeObj->getGluePointIndex( pProperty->nWID == OWN_ATTR_GLUEID_HEAD );
                    break;
                }
            case OWN_ATTR_EDGE_POLYPOLYGONBEZIER:
                {
                    basegfx::B2DPolyPolygon aPolyPoly( pEdgeObj->GetEdgeTrackPath() );
                    if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                    {
                        Point aPoint( GetSdrObject()->GetAnchorPos() );
                        aPolyPoly.transform(basegfx::utils::createTranslateB2DHomMatrix(-aPoint.X(), -aPoint.Y()));
                    }
                    // Reintroduction of fix for issue #i59051# (#i108851#)
                    ForceMetricTo100th_mm( aPolyPoly );
                    drawing::PolyPolygonBezierCoords aRetval;
                    basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords( aPolyPoly, aRetval);
                    rValue <<= aRetval;
                    break;
                }
            }
        }
        break;
    }

    case OWN_ATTR_MEASURE_START_POS:
    case OWN_ATTR_MEASURE_END_POS:
    {
        SdrMeasureObj* pMeasureObj = dynamic_cast<SdrMeasureObj*>(GetSdrObject());
        if(pMeasureObj)
        {
            Point aPoint( pMeasureObj->GetPoint( pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 ) );
            if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                aPoint -= GetSdrObject()->GetAnchorPos();

            // Reintroduction of fix for issue #i59051# (#i108851#)
            ForceMetricTo100th_mm( aPoint );
            awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

            rValue <<= aUnoPoint;
            break;
        }
        break;
    }

    case OWN_ATTR_FILLBMP_MODE:
    {
        const SfxItemSet& rObjItemSet = GetSdrObject()->GetMergedItemSet();

        if (rObjItemSet.Get(XATTR_FILLBMP_TILE).GetValue())
        {
            rValue <<= drawing::BitmapMode_REPEAT;
        }
        else if (rObjItemSet.Get(XATTR_FILLBMP_STRETCH).GetValue())
        {
            rValue <<= drawing::BitmapMode_STRETCH;
        }
        else
        {
            rValue <<= drawing::BitmapMode_NO_REPEAT;
        }
        break;
    }
    case SDRATTR_LAYERID:
        rValue <<= GetSdrObject()->GetLayer().get();
        break;

    case SDRATTR_LAYERNAME:
    {
        SdrLayer* pLayer = GetSdrObject()->getSdrModelFromSdrObject().GetLayerAdmin().GetLayerPerID(GetSdrObject()->GetLayer());
        if( pLayer )
        {
            rValue <<= pLayer->GetName();
        }
        break;
    }

    case SDRATTR_ROTATEANGLE:
        rValue <<= static_cast<sal_Int32>(GetSdrObject()->GetRotateAngle());
        break;

    case SDRATTR_SHEARANGLE:
        rValue <<= static_cast<sal_Int32>(GetSdrObject()->GetShearAngle());
        break;

    case OWN_ATTR_INTEROPGRABBAG:
    {
        GetSdrObject()->GetGrabBagItem(rValue);
        break;
    }

    case SDRATTR_OBJMOVEPROTECT:
        rValue <<= GetSdrObject()->IsMoveProtect();
        break;

    case SDRATTR_OBJECTNAME:
    {
        OUString aName( GetSdrObject()->GetName() );
        rValue <<= aName;
        break;
    }

    // #i68101#
    case OWN_ATTR_MISC_OBJ_TITLE:
    {
        OUString aTitle( GetSdrObject()->GetTitle() );
        rValue <<= aTitle;
        break;
    }

    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription( GetSdrObject()->GetDescription() );
        rValue <<= aDescription;
        break;
    }

    case OWN_ATTR_MISC_OBJ_DECORATIVE:
    {
        bool const isDecorative(GetSdrObject()->IsDecorative());
        rValue <<= isDecorative;
        break;
    }

    case SDRATTR_OBJPRINTABLE:
        rValue <<= GetSdrObject()->IsPrintable();
        break;

    case SDRATTR_OBJVISIBLE:
        rValue <<= GetSdrObject()->IsVisible();
        break;

    case SDRATTR_OBJSIZEPROTECT:
        rValue <<= GetSdrObject()->IsResizeProtect();
        break;

    case OWN_ATTR_PAGE_NUMBER:
    {
        SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(GetSdrObject());
        if(pPageObj)
        {
            SdrPage* pPage = pPageObj->GetReferencedPage();
            sal_Int32 nPageNumber = pPage ? pPage->GetPageNum() : 0;
            nPageNumber++;
            nPageNumber >>= 1;
            rValue <<= nPageNumber;
        }
        break;
    }

    case OWN_ATTR_UINAME_SINGULAR:
    {
        rValue <<= GetSdrObject()->TakeObjNameSingul();
        break;
    }

    case OWN_ATTR_TEXTFITTOSIZE_FONT_SCALE:
    {
        auto* pTextObject = getTextObjectWithFitToSize(GetSdrObject());
        if (pTextObject)
        {
            rValue <<= pTextObject->GetFontScale() * 100.0;
        }
        break;
    }

    case OWN_ATTR_TEXTFITTOSIZE_SPACING_SCALE:
    {
        auto* pTextObject = getTextObjectWithFitToSize(GetSdrObject());
        if (pTextObject)
        {
            rValue <<= pTextObject->GetSpacingScale() * 100.0;
        }
        break;
    }

    case OWN_ATTR_UINAME_PLURAL:
    {
        rValue <<= GetSdrObject()->TakeObjNamePlural();
        break;
    }
    case OWN_ATTR_METAFILE:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>(GetSdrObject());
        if( pObj )
        {
            const Graphic* pGraphic = pObj->GetGraphic();
            if( pGraphic )
            {
                bool bIsWMF = false;
                if ( pGraphic->IsGfxLink() )
                {
                    GfxLink aLnk = pGraphic->GetGfxLink();
                    if ( aLnk.GetType() == GfxLinkType::NativeWmf )
                    {
                        bIsWMF = true;
                        uno::Sequence<sal_Int8> aSeq(reinterpret_cast<sal_Int8 const *>(aLnk.GetData()), static_cast<sal_Int32>(aLnk.GetDataSize()));
                        rValue <<= aSeq;
                    }
                }
                if ( !bIsWMF )
                {
                    // #119735# just use GetGDIMetaFile, it will create a buffered version of contained bitmap now automatically
                    GDIMetaFile aMtf(pObj->GetGraphic()->GetGDIMetaFile());
                    SvMemoryStream aDestStrm( 65535, 65535 );
                    ConvertGDIMetaFileToWMF( aMtf, aDestStrm, nullptr, false );
                    const uno::Sequence<sal_Int8> aSeq(
                        static_cast< const sal_Int8* >(aDestStrm.GetData()),
                        aDestStrm.GetEndOfData());
                    rValue <<= aSeq;
                }
            }
        }
        else
        {
            rValue = GetBitmap( true );
        }
        break;
    }

    case OWN_ATTR_TEXTCOLUMNS:
    {
        if (auto pTextObj = DynCastSdrTextObj(GetSdrObject()))
        {
            if (pTextObj->HasTextColumnsNumber() || pTextObj->HasTextColumnsSpacing())
            {
                auto xIf = SvxXTextColumns_createInstance();
                css::uno::Reference<css::text::XTextColumns> xCols(xIf, css::uno::UNO_QUERY_THROW);
                xCols->setColumnCount(pTextObj->GetTextColumnsNumber());
                css::uno::Reference<css::beans::XPropertySet> xProp(xIf, css::uno::UNO_QUERY_THROW);
                xProp->setPropertyValue(u"AutomaticDistance"_ustr,
                                        css::uno::Any(pTextObj->GetTextColumnsSpacing()));
                rValue <<= xIf;
            }
        }
        break;
    }

    case OWN_ATTR_HYPERLINK:
    {
        rValue <<= GetSdrObject()->getHyperlink();
        break;
    }

    default:
        return false;
    }
    return true;
}


bool SvxShape::getPropertyStateImpl( const SfxItemPropertyMapEntry* pProperty, css::beans::PropertyState& rState )
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = GetSdrObject()->GetMergedItemSet();

        if( rSet.GetItemState( XATTR_FILLBMP_STRETCH, false ) == SfxItemState::SET ||
            rSet.GetItemState( XATTR_FILLBMP_TILE, false ) == SfxItemState::SET )
        {
            rState = beans::PropertyState_DIRECT_VALUE;
        }
        else
        {
            rState = beans::PropertyState_AMBIGUOUS_VALUE;
        }
    }
    else if((( pProperty->nWID >= OWN_ATTR_VALUE_START && pProperty->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pProperty->nWID >= SDRATTR_NOTPERSIST_FIRST && pProperty->nWID <= SDRATTR_NOTPERSIST_LAST )) && ( pProperty->nWID != SDRATTR_TEXTDIRECTION ) )
    {
        rState = beans::PropertyState_DIRECT_VALUE;
    }
    else
    {
        return false;
    }

    return true;
}


bool SvxShape::setPropertyToDefaultImpl( const SfxItemPropertyMapEntry* pProperty )
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        GetSdrObject()->ClearMergedItem( XATTR_FILLBMP_STRETCH );
        GetSdrObject()->ClearMergedItem( XATTR_FILLBMP_TILE );
        return true;
    }
    else if((pProperty->nWID >= OWN_ATTR_VALUE_START && pProperty->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pProperty->nWID >= SDRATTR_NOTPERSIST_FIRST && pProperty->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return true;
    }
    else
    {
        return false;
    }
}


uno::Sequence< beans::PropertyState > SAL_CALL SvxShape::getPropertyStates( const uno::Sequence< OUString >& aPropertyName )
{
    const sal_Int32 nCount = aPropertyName.getLength();
    uno::Sequence< beans::PropertyState > aRet( nCount );

    std::transform(aPropertyName.begin(), aPropertyName.end(), aRet.getArray(),
        [this](const OUString& rName) -> beans::PropertyState { return getPropertyState(rName); });

    return aRet;
}


void SAL_CALL SvxShape::setPropertyToDefault( const OUString& PropertyName )
{
    if( mpImpl->mpMaster )
    {
        mpImpl->mpMaster->setPropertyToDefault( PropertyName );
    }
    else
    {
        _setPropertyToDefault( PropertyName );
    }
}

void SvxShape::_setPropertyToDefault( const OUString& PropertyName )
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pProperty = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !HasSdrObject() || pProperty == nullptr )
        throw beans::UnknownPropertyException( PropertyName, getXWeak());

    if( !setPropertyToDefaultImpl( pProperty ) )
    {
        GetSdrObject()->ClearMergedItem( pProperty->nWID );
    }

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}


uno::Any SAL_CALL SvxShape::getPropertyDefault( const OUString& aPropertyName )
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getPropertyDefault( aPropertyName );
    }
    else
    {
        return _getPropertyDefault( aPropertyName );
    }
}

uno::Any SvxShape::_getPropertyDefault( const OUString& aPropertyName )
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertyMapEntry* pMap = mpPropSet->getPropertyMapEntry(aPropertyName);

    if( !HasSdrObject() || pMap == nullptr )
        throw beans::UnknownPropertyException( aPropertyName, getXWeak());

    if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return getPropertyValue( aPropertyName );
    }

    // get default from ItemPool
    if(!SfxItemPool::IsWhich(pMap->nWID))
        throw beans::UnknownPropertyException( "No WhichID " + OUString::number(pMap->nWID) + " for " + aPropertyName, getXWeak());

    SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), pMap->nWID, pMap->nWID );
    aSet.Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetUserOrPoolDefaultItem(pMap->nWID));

    return GetAnyForItem( aSet, pMap );
}

// XMultiPropertyStates
void SvxShape::setAllPropertiesToDefault()
{
    ::SolarMutexGuard aGuard;

    SdrObject* pSdrObj = GetSdrObject();
    if( !pSdrObj )
        throw lang::DisposedException();
    pSdrObj->ClearMergedItem(); // nWhich == 0 => all

    const SdrObjKind nObjId = pSdrObj->GetObjIdentifier();
    if(nObjId == SdrObjKind::Graphic) // SdrGrafObj
    {
        // defaults for graphic objects have changed:
        pSdrObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );
        pSdrObj->SetMergedItem( XLineStyleItem( drawing::LineStyle_NONE ) );
    }

    // #i68523# special handling for Svx3DCharacterModeItem, this is not saved
    // but needs to be sal_True in svx, pool default (false) in sch. Since sch
    // does not load lathe or extrude objects, it is possible to set the items
    // here.
    // For other solution possibilities, see task description.
    if( nObjId == SdrObjKind::E3D_Lathe /*E3dLatheObj*/ || nObjId == SdrObjKind::E3D_Extrusion /*E3dExtrudeObj*/ )
    {
        pSdrObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    pSdrObj->getSdrModelFromSdrObject().SetChanged();
}

void SvxShape::setPropertiesToDefault(
    const uno::Sequence<OUString>& aPropertyNames )
{
    for ( const auto& rPropertyName : aPropertyNames )
        setPropertyToDefault( rPropertyName );
}

uno::Sequence<uno::Any> SvxShape::getPropertyDefaults(
    const uno::Sequence<OUString>& aPropertyNames )
{
    ::std::vector<uno::Any> ret;
    ret.reserve(aPropertyNames.getLength());
    std::transform(aPropertyNames.begin(), aPropertyNames.end(), std::back_inserter(ret),
        [this](const OUString& rName) -> uno::Any { return getPropertyDefault(rName); });
    return uno::Sequence<uno::Any>( ret.data(), ret.size() );
}


// XServiceInfo

OUString SAL_CALL SvxShape::getImplementationName()
{
    return u"SvxShape"_ustr;
}

constexpr OUString sUNO_service_style_ParagraphProperties = u"com.sun.star.style.ParagraphProperties"_ustr;
constexpr OUString sUNO_service_style_ParagraphPropertiesComplex = u"com.sun.star.style.ParagraphPropertiesComplex"_ustr;
constexpr OUString sUNO_service_style_ParagraphPropertiesAsian = u"com.sun.star.style.ParagraphPropertiesAsian"_ustr;
constexpr OUString sUNO_service_style_CharacterProperties = u"com.sun.star.style.CharacterProperties"_ustr;
constexpr OUString sUNO_service_style_CharacterPropertiesComplex = u"com.sun.star.style.CharacterPropertiesComplex"_ustr;
constexpr OUString sUNO_service_style_CharacterPropertiesAsian = u"com.sun.star.style.CharacterPropertiesAsian"_ustr;

constexpr OUString sUNO_service_drawing_FillProperties    = u"com.sun.star.drawing.FillProperties"_ustr;
constexpr OUString sUNO_service_drawing_TextProperties    = u"com.sun.star.drawing.TextProperties"_ustr;
constexpr OUString sUNO_service_drawing_LineProperties    = u"com.sun.star.drawing.LineProperties"_ustr;
constexpr OUString sUNO_service_drawing_ConnectorProperties = u"com.sun.star.drawing.ConnectorProperties"_ustr;
constexpr OUString sUNO_service_drawing_MeasureProperties = u"com.sun.star.drawing.MeasureProperties"_ustr;
constexpr OUString sUNO_service_drawing_ShadowProperties  = u"com.sun.star.drawing.ShadowProperties"_ustr;

constexpr OUString sUNO_service_drawing_RotationDescriptor = u"com.sun.star.drawing.RotationDescriptor"_ustr;

constexpr OUString sUNO_service_drawing_Text              = u"com.sun.star.drawing.Text"_ustr;
constexpr OUString sUNO_service_drawing_GroupShape        = u"com.sun.star.drawing.GroupShape"_ustr;

constexpr OUString sUNO_service_drawing_CustomShapeProperties = u"com.sun.star.drawing.CustomShapeProperties"_ustr;
constexpr OUString sUNO_service_drawing_CustomShape       = u"com.sun.star.drawing.CustomShape"_ustr;

constexpr OUString sUNO_service_drawing_PolyPolygonDescriptor = u"com.sun.star.drawing.PolyPolygonDescriptor"_ustr;
constexpr OUString sUNO_service_drawing_PolyPolygonBezierDescriptor= u"com.sun.star.drawing.PolyPolygonBezierDescriptor"_ustr;

constexpr OUString sUNO_service_drawing_LineShape         = u"com.sun.star.drawing.LineShape"_ustr;
constexpr OUString sUNO_service_drawing_Shape             = u"com.sun.star.drawing.Shape"_ustr;
constexpr OUString sUNO_service_drawing_RectangleShape    = u"com.sun.star.drawing.RectangleShape"_ustr;
constexpr OUString sUNO_service_drawing_EllipseShape      = u"com.sun.star.drawing.EllipseShape"_ustr;
constexpr OUString sUNO_service_drawing_PolyPolygonShape  = u"com.sun.star.drawing.PolyPolygonShape"_ustr;
constexpr OUString sUNO_service_drawing_PolyLineShape     = u"com.sun.star.drawing.PolyLineShape"_ustr;
constexpr OUString sUNO_service_drawing_OpenBezierShape   = u"com.sun.star.drawing.OpenBezierShape"_ustr;
constexpr OUString sUNO_service_drawing_ClosedBezierShape = u"com.sun.star.drawing.ClosedBezierShape"_ustr;
constexpr OUString sUNO_service_drawing_TextShape         = u"com.sun.star.drawing.TextShape"_ustr;
constexpr OUString sUNO_service_drawing_GraphicObjectShape = u"com.sun.star.drawing.GraphicObjectShape"_ustr;
constexpr OUString sUNO_service_drawing_OLE2Shape         = u"com.sun.star.drawing.OLE2Shape"_ustr;
constexpr OUString sUNO_service_drawing_PageShape         = u"com.sun.star.drawing.PageShape"_ustr;
constexpr OUString sUNO_service_drawing_CaptionShape      = u"com.sun.star.drawing.CaptionShape"_ustr;
constexpr OUString sUNO_service_drawing_MeasureShape      = u"com.sun.star.drawing.MeasureShape"_ustr;
constexpr OUString sUNO_service_drawing_FrameShape        = u"com.sun.star.drawing.FrameShape"_ustr;
constexpr OUString sUNO_service_drawing_ControlShape      = u"com.sun.star.drawing.ControlShape"_ustr;
constexpr OUString sUNO_service_drawing_ConnectorShape    = u"com.sun.star.drawing.ConnectorShape"_ustr;
constexpr OUString sUNO_service_drawing_MediaShape        = u"com.sun.star.drawing.MediaShape"_ustr;


uno::Sequence< OUString > SAL_CALL SvxShape::getSupportedServiceNames()
{
    if( mpImpl->mpMaster )
    {
        return mpImpl->mpMaster->getSupportedServiceNames();
    }
    else
    {
        return _getSupportedServiceNames();
    }
}

uno::Sequence< OUString > SvxShape::_getSupportedServiceNames()
{
    ::SolarMutexGuard aGuard;

    if( HasSdrObject() && GetSdrObject()->GetObjInventor() == SdrInventor::Default)
    {
        const SdrObjKind nIdent = GetSdrObject()->GetObjIdentifier();

        switch(nIdent)
        {
        case SdrObjKind::Group:
            {
                static const uno::Sequence<OUString> aSvxShape_GroupServices
                        = { sUNO_service_drawing_GroupShape,
                            sUNO_service_drawing_Shape };
                return aSvxShape_GroupServices;
            }
        case SdrObjKind::CustomShape:
            {
                static const uno::Sequence<OUString> aSvxShape_CustomShapeServices
                        = { sUNO_service_drawing_CustomShape,
                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_CustomShapeProperties,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_CustomShapeServices;
            }
        case SdrObjKind::Line:
            {
                static const uno::Sequence<OUString> aSvxShape_LineServices
                        = { sUNO_service_drawing_LineShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_LineServices;
            }

        case SdrObjKind::Rectangle:
            {
                static const uno::Sequence<OUString> aSvxShape_RectServices
                        = { sUNO_service_drawing_RectangleShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_RectServices;
            }

        case SdrObjKind::CircleOrEllipse:
        case SdrObjKind::CircleSection:
        case SdrObjKind::CircleArc:
        case SdrObjKind::CircleCut:
            {
                static const uno::Sequence<OUString> aSvxShape_CircServices
                        = { sUNO_service_drawing_EllipseShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_CircServices;
            }

        case SdrObjKind::PathPolyLine:
        case SdrObjKind::PolyLine:
            {
                static const uno::Sequence<OUString> aSvxShape_PathServices
                        = { sUNO_service_drawing_PolyLineShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_PolyPolygonDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_PathServices;
            }

        case SdrObjKind::PathPoly:
        case SdrObjKind::Polygon:
            {
                static const uno::Sequence<OUString> aSvxShape_PolyServices
                        = { sUNO_service_drawing_PolyPolygonShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_PolyServices;
            }

        case SdrObjKind::FreehandLine:
        case SdrObjKind::PathLine:
            {
                static const uno::Sequence<OUString> aSvxShape_FreeLineServices
                        = { sUNO_service_drawing_OpenBezierShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonBezierDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_FreeLineServices;
            }

        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathFill:
            {
                static const uno::Sequence<OUString> aSvxShape_FreeFillServices
                        = { sUNO_service_drawing_ClosedBezierShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,
                            sUNO_service_drawing_FillProperties,

                            sUNO_service_drawing_PolyPolygonBezierDescriptor,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_FreeFillServices;
            }

        case SdrObjKind::OutlineText:
        case SdrObjKind::TitleText:
        case SdrObjKind::Text:
            {
                static const uno::Sequence<OUString> aSvxShape_TextServices
                        = { sUNO_service_drawing_TextShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_TextServices;
            }

        case SdrObjKind::Graphic:
            {
                static const uno::Sequence<OUString> aSvxShape_GrafServices
                        = { sUNO_service_drawing_GraphicObjectShape,

                            sUNO_service_drawing_Shape,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor};
                return aSvxShape_GrafServices;
            }

        case SdrObjKind::OLE2:
            {
                static const uno::Sequence<OUString> aSvxShape_Ole2Services
                        = { sUNO_service_drawing_OLE2Shape,
                            sUNO_service_drawing_Shape,

                            // #i118485# Added Text, Shadow and Rotation
                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_Ole2Services;
            }

        case SdrObjKind::Caption:
            {
                static const uno::Sequence<OUString> aSvxShape_CaptionServices
                        = { sUNO_service_drawing_CaptionShape,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_FillProperties,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_CaptionServices;
            }

        case SdrObjKind::Page:
            {
                static const uno::Sequence<OUString> aSvxShape_PageServices
                        = { sUNO_service_drawing_PageShape,
                            sUNO_service_drawing_Shape };
                return aSvxShape_PageServices;
            }

        case SdrObjKind::Measure:
            {
                static const uno::Sequence<OUString> aSvxShape_MeasureServices
                        = { sUNO_service_drawing_MeasureShape,

                            sUNO_service_drawing_MeasureProperties,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_MeasureServices;
            }

        case SdrObjKind::OLEPluginFrame:
            {
                static const uno::Sequence<OUString> aSvxShape_FrameServices
                        = { sUNO_service_drawing_FrameShape,
                            sUNO_service_drawing_Shape };
                return aSvxShape_FrameServices;
            }

        case SdrObjKind::UNO:
            {
                static const uno::Sequence<OUString> aSvxShape_UnoServices
                        = { sUNO_service_drawing_ControlShape,
                            sUNO_service_drawing_Shape };
                return aSvxShape_UnoServices;
            }

        case SdrObjKind::Edge:
            {
                static const uno::Sequence<OUString> aSvxShape_EdgeServices
                        = { sUNO_service_drawing_ConnectorShape,
                            sUNO_service_drawing_ConnectorProperties,

                            sUNO_service_drawing_Shape,
                            sUNO_service_drawing_LineProperties,

                            sUNO_service_drawing_Text,
                            sUNO_service_drawing_TextProperties,
                            sUNO_service_style_ParagraphProperties,
                            sUNO_service_style_ParagraphPropertiesComplex,
                            sUNO_service_style_ParagraphPropertiesAsian,
                            sUNO_service_style_CharacterProperties,
                            sUNO_service_style_CharacterPropertiesComplex,
                            sUNO_service_style_CharacterPropertiesAsian,

                            sUNO_service_drawing_PolyPolygonDescriptor,
                            sUNO_service_drawing_ShadowProperties,
                            sUNO_service_drawing_RotationDescriptor };
                return aSvxShape_EdgeServices;
            }
        case SdrObjKind::Media:
            {
                static const uno::Sequence<OUString> aSvxShape_MediaServices
                        = { sUNO_service_drawing_MediaShape,
                            sUNO_service_drawing_Shape };
                return aSvxShape_MediaServices;
            }
        default: ;
        }
    }
    else if( HasSdrObject() && GetSdrObject()->GetObjInventor() == SdrInventor::FmForm)
    {
#if OSL_DEBUG_LEVEL > 0
        const SdrObjKind nIdent = GetSdrObject()->GetObjIdentifier();
        OSL_ENSURE( nIdent == SdrObjKind::UNO, "SvxShape::_getSupportedServiceNames: SdrInventor::FmForm, but no UNO object?" );
#endif
        static const uno::Sequence<OUString> aSvxShape_UnoServices
                = { sUNO_service_drawing_ControlShape,
                    sUNO_service_drawing_Shape };
        return aSvxShape_UnoServices;
    }
    OSL_FAIL( "SvxShape::_getSupportedServiceNames: could not determine object type!" );
    uno::Sequence< OUString > aSeq;
    return aSeq;
}

sal_Bool SAL_CALL SvxShape::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// XGluePointsSupplier
uno::Reference< container::XIndexContainer > SAL_CALL SvxShape::getGluePoints()
{
    ::SolarMutexGuard aGuard;
    uno::Reference< container::XIndexContainer > xGluePoints( mxGluePoints );

    if( HasSdrObject() && !xGluePoints.is() )
    {
        xGluePoints.set(SvxUnoGluePointAccess_createInstance(GetSdrObject()), uno::UNO_QUERY);
        mxGluePoints = xGluePoints;
    }

    return xGluePoints;
}

// XChild
uno::Reference<uno::XInterface> SAL_CALL SvxShape::getParent()
{
    ::SolarMutexGuard aGuard;
    const SdrObject* pSdrObject(GetSdrObject());

    if(nullptr != pSdrObject)
    {
        const SdrObjList* pParentSdrObjList(GetSdrObject()->getParentSdrObjListFromSdrObject());

        if(nullptr != pParentSdrObjList)
        {
            // SdrObject is member of a SdrObjList. That may be a SdrObject
            // (SdrObjGroup or E3dScene) or a SdrPage.
            // Check for SdrObject first - using getSdrPageFromSdrObjList
            // *will* get the SdrPage even when the SdrObject is deep buried
            // in a construct of SdrObjGroup.
            // We want to ask for the direct parent here...
            SdrObject* pParentSdrObject(pParentSdrObjList->getSdrObjectFromSdrObjList());

            if(nullptr != pParentSdrObject)
            {
                // SdrObject is member of a SdrObject-based Group (SdrObjGroup or E3dScene).
                return pParentSdrObject->getUnoShape();
            }
            else
            {
                SdrPage* pParentSdrPage(pParentSdrObjList->getSdrPageFromSdrObjList());

                if(nullptr != pParentSdrPage)
                {
                    // SdrObject is inserted to a SdrPage. Since
                    // we checked for getSdrObjectFromSdrObjList first,
                    // we can even say that it is directly member of that
                    // SdrPage.
                    return pParentSdrPage->getUnoPage();
                }
            }

            // not member of any SdrObjList, no parent
            OSL_FAIL( "SvxShape::getParent(  ): unexpected Parent SdrObjList" );
        }
    }

    // no SdrObject, no parent
    return uno::Reference<uno::XInterface>();
}

void SAL_CALL SvxShape::setParent( const css::uno::Reference< css::uno::XInterface >& )
{
    throw lang::NoSupportException();
}


/** called from the XActionLockable interface methods on initial locking */
void SvxShape::lock()
{
}


/** called from the XActionLockable interface methods on final unlock */
void SvxShape::unlock()
{
}


// XActionLockable
sal_Bool SAL_CALL SvxShape::isActionLocked(  )
{
    ::SolarMutexGuard aGuard;

    return mnLockCount != 0;
}


void SAL_CALL SvxShape::addActionLock(  )
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount < 0xffff, "lock overflow in SvxShape!" );
    mnLockCount++;

    if( mnLockCount == 1 )
        lock();
}


void SAL_CALL SvxShape::removeActionLock(  )
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount > 0, "lock underflow in SvxShape!" );
    mnLockCount--;

    if( mnLockCount == 0 )
        unlock();
}


void SAL_CALL SvxShape::setActionLocks( sal_Int16 nLock )
{
    ::SolarMutexGuard aGuard;

    if( (mnLockCount == 0) && (nLock != 0) )
        unlock();

    if( (mnLockCount != 0) && (nLock == 0) )
        lock();

    mnLockCount = static_cast<sal_uInt16>(nLock);
}


sal_Int16 SAL_CALL SvxShape::resetActionLocks(  )
{
    ::SolarMutexGuard aGuard;

    if( mnLockCount != 0 )
        unlock();

    sal_Int16 nOldLocks = static_cast<sal_Int16>(mnLockCount);
    mnLockCount = 0;

    return nOldLocks;
}


/** since polygon shapes can change their kind during editing, we have
    to recheck it here.
    Circle shapes also change their kind, but they are all treated equal
    so no update is necessary.
*/
void SvxShape::updateShapeKind()
{
    switch( mpImpl->mnObjId )
    {
        case SdrObjKind::Line:
        case SdrObjKind::Polygon:
        case SdrObjKind::PolyLine:
        case SdrObjKind::PathLine:
        case SdrObjKind::PathFill:
        case SdrObjKind::FreehandLine:
        case SdrObjKind::FreehandFill:
        case SdrObjKind::PathPoly:
        case SdrObjKind::PathPolyLine:
        {
            const SdrObjKind nId = GetSdrObject()->GetObjIdentifier();

            if( nId != mpImpl->mnObjId )
            {
                mpImpl->mnObjId = nId;

            }
            break;
        }
        default: ;
    }
}

SvxShapeText::SvxShapeText(SdrObject* pObject)
: SvxShape( pObject, getSvxMapProvider().GetMap(SVXMAP_TEXT), getSvxMapProvider().GetPropertySet(SVXMAP_TEXT, SdrObject::GetGlobalDrawObjectItemPool()) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject )
        SetEditSource( new SvxTextEditSource( pObject, nullptr ) );
}


SvxShapeText::SvxShapeText(SdrObject* pObject, std::span<const SfxItemPropertyMapEntry> pPropertyMap, const SvxItemPropertySet* pPropertySet)
: SvxShape( pObject, pPropertyMap, pPropertySet ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject )
        SetEditSource( new SvxTextEditSource( pObject, nullptr ) );
}


SvxShapeText::~SvxShapeText() noexcept
{
    // check if only this instance is registered at the ranges
    DBG_ASSERT( (nullptr == GetEditSource()) || (GetEditSource()->getRanges().size()==1),
        "svx::SvxShapeText::~SvxShapeText(), text shape with living text ranges destroyed!");
}

void SvxShapeText::Create( SdrObject* pNewObj, SvxDrawPage* pNewPage )
{
    if( pNewObj && (nullptr == GetEditSource()))
        SetEditSource( new SvxTextEditSource( pNewObj, nullptr ) );
    SvxShape::Create( pNewObj, pNewPage );
}

// XInterface

uno::Any SAL_CALL SvxShapeText::queryInterface( const uno::Type & rType )
{
    return SvxShape::queryInterface( rType );
}


uno::Any SAL_CALL SvxShapeText::queryAggregation( const uno::Type & rType )
{
    uno::Any aAny( SvxShape::queryAggregation( rType ) );
    if( aAny.hasValue() )
        return aAny;

    return SvxUnoTextBase::queryAggregation( rType );
}


// XServiceInfo

OUString SAL_CALL SvxShapeText::getImplementationName()
{
    return u"SvxShapeText"_ustr;
}


uno::Sequence< OUString > SAL_CALL SvxShapeText::getSupportedServiceNames()
{
    return SvxShape::getSupportedServiceNames();
}


sal_Bool SAL_CALL SvxShapeText::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(static_cast<SvxShape*>(this), ServiceName);
}

    // XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeText::getTypes()
{
    return SvxShape::getTypes();
}

sal_Int64 SAL_CALL SvxShapeText::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    const sal_Int64 nReturn = SvxShape::getSomething( rId );
    if( nReturn )
        return nReturn;

    return SvxUnoTextBase::getSomething( rId );
}


uno::Sequence< sal_Int8 > SAL_CALL SvxShapeText::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


/** called from the XActionLockable interface methods on initial locking */
void SvxShapeText::lock()
{
    SvxTextEditSource* pEditSource = static_cast<SvxTextEditSource*>(GetEditSource());
    if( pEditSource )
        pEditSource->lock();
}


/** called from the XActionLockable interface methods on final unlock */
void SvxShapeText::unlock()
{
    SvxTextEditSource* pEditSource = static_cast<SvxTextEditSource*>(GetEditSource());
    if( pEditSource )
        pEditSource->unlock();
}

// css::text::XTextRange
uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getStart()
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getStart();

}

uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getEnd()
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getEnd();
}

OUString SAL_CALL SvxShapeText::getString()
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getString();
}


void SAL_CALL SvxShapeText::setString( const OUString& aString )
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    SvxUnoTextBase::setString( aString );
}

// override these for special property handling in subcasses. Return true if property is handled
bool SvxShapeText::setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue )
{
    // HACK-fix #99090#
    // since SdrTextObj::SetVerticalWriting exchanges
    // SDRATTR_TEXT_AUTOGROWWIDTH and SDRATTR_TEXT_AUTOGROWHEIGHT,
    // we have to set the textdirection here

    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = DynCastSdrTextObj( GetSdrObject() );
        if( pTextObj )
        {
            css::text::WritingMode eMode;
            if( rValue >>= eMode )
            {
                pTextObj->SetVerticalWriting( eMode == css::text::WritingMode_TB_RL );
            }
        }
        return true;
    }
    return SvxShape::setPropertyValueImpl( rName, pProperty, rValue );
}

bool SvxShapeText::getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue )
{
    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = DynCastSdrTextObj( GetSdrObject() );
        if( pTextObj && pTextObj->IsVerticalWriting() )
            rValue <<= css::text::WritingMode_TB_RL;
        else
            rValue <<= css::text::WritingMode_LR_TB;
        return true;
    }

    return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
}

bool SvxShapeText::getPropertyStateImpl( const SfxItemPropertyMapEntry* pProperty, css::beans::PropertyState& rState )
{
    return SvxShape::getPropertyStateImpl( pProperty, rState );
}

bool SvxShapeText::setPropertyToDefaultImpl( const SfxItemPropertyMapEntry* pProperty )
{
    return SvxShape::setPropertyToDefaultImpl( pProperty );
}

SvxShapeRect::SvxShapeRect(SdrObject* pObj)
: SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_SHAPE), getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
{
}

SvxShapeRect::~SvxShapeRect() noexcept
{
}

uno::Any SAL_CALL SvxShapeRect::queryInterface( const uno::Type & rType )
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeRect::queryAggregation( const uno::Type & rType )
{
    return SvxShapeText::queryAggregation( rType );
}

// XServiceInfo

uno::Sequence< OUString > SvxShapeRect::getSupportedServiceNames()
{
    return SvxShape::getSupportedServiceNames();
}

/** returns a StarOffice API wrapper for the given SdrObject */
uno::Reference< drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) noexcept
{
    uno::Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}


SdrObject* SdrObject::getSdrObjectFromXShape( const css::uno::Reference< css::uno::XInterface >& xInt )
{
    SvxShape* pSvxShape = comphelper::getFromUnoTunnel<SvxShape>( xInt );
    return pSvxShape ? pSvxShape->GetSdrObject() : nullptr;
}

uno::Any SvxItemPropertySet_getPropertyValue( const SfxItemPropertyMapEntry* pMap, const SfxItemSet& rSet )
{
    if(!pMap || !pMap->nWID)
        return uno::Any();

    // Check is for items that store either metric values if they are positive or percentage if they are negative.
    bool bDontConvertNegativeValues = ( pMap->nWID == XATTR_FILLBMP_SIZEX || pMap->nWID == XATTR_FILLBMP_SIZEY );
    return SvxItemPropertySet::getPropertyValue( pMap, rSet, (pMap->nWID != SDRATTR_XMLATTRIBUTES), bDontConvertNegativeValues );
}

void SvxItemPropertySet_setPropertyValue( const SfxItemPropertyMapEntry* pMap, const uno::Any& rVal, SfxItemSet& rSet )
{
    if(!pMap || !pMap->nWID)
        return;

    bool bDontConvertNegativeValues = ( pMap->nWID == XATTR_FILLBMP_SIZEX || pMap->nWID == XATTR_FILLBMP_SIZEY );
    SvxItemPropertySet::setPropertyValue( pMap, rVal, rSet, bDontConvertNegativeValues );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
