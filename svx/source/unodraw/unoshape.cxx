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

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <vcl/fltcall.hxx>
#include <o3tl/any.hxx>
#include <osl/mutex.hxx>
#include <editeng/unotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/svdopage.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/svdogrp.hxx>
#include <svx/scene3d.hxx>
#include <svx/svdmodel.hxx>
#include <svx/globl3d.hxx>
#include <svx/fmglob.hxx>
#include <svx/unopage.hxx>
#include <svx/view3d.hxx>
#include <svx/unoshape.hxx>
#include <svx/svxids.hrc>
#include <svx/unoshtxt.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoshprp.hxx>
#include <svx/sxciaitm.hxx>
#include <svx/svdograf.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdomeas.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpool.hxx>
#include <tools/gen.hxx>
#include <svx/svdocapt.hxx>
#include <svx/obj3d.hxx>
#include <tools/diagnose_ex.h>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/unomaster.hxx>
#include <editeng/outlobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include "gluepts.hxx"
#include "shapeimpl.hxx"
#include <sal/log.hxx>

#include <svx/lathe3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

#include <vcl/wmf.hxx>

#include <memory>
#include <vector>
#include <iostream>

#include <bitmaps.hlst>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using svx::PropertyValueProvider;

class GDIMetaFile;

struct SvxShapeImpl
{
    SvxShape&       mrAntiImpl;
    std::unique_ptr<SfxItemSet> mpItemSet;
    sal_uInt32      mnObjId;
    SvxShapeMaster* mpMaster;
    bool            mbHasSdrObjectOwnership;
    bool            mbDisposing;

    /** CL, OD 2005-07-19 #i52126# - this is initially 0 and set when
     *  a SvxShape::Create() call is executed. It is then set to the created
     *  SdrObject so a multiple call to SvxShape::Create() with same SdrObject
     *  is prohibited.
     */
    ::tools::WeakReference< SdrObject > mpCreatedObj;

    // for xComponent
    ::comphelper::OInterfaceContainerHelper2   maDisposeListeners;
    svx::PropertyChangeNotifier       maPropertyNotifier;

    SvxShapeImpl( SvxShape& _rAntiImpl, ::osl::Mutex& _rMutex )
        :mrAntiImpl( _rAntiImpl )
        ,mnObjId( 0 )
        ,mpMaster( nullptr )
        ,mbHasSdrObjectOwnership( false )
        ,mbDisposing( false )
        ,mpCreatedObj()
        ,maDisposeListeners( _rMutex )
        ,maPropertyNotifier( _rAntiImpl, _rMutex )
    {
    }
};

class ShapePositionProvider : public PropertyValueProvider
{
public:
    explicit ShapePositionProvider( const SvxShapeImpl& _shapeImpl )
        :PropertyValueProvider( _shapeImpl.mrAntiImpl, "Position" )
    {
    }

protected:
    virtual void getCurrentValue( Any& _out_rCurrentValue ) const override
    {
        _out_rCurrentValue <<= static_cast< SvxShape& >( getContext() ).getPosition();
    }
};


class ShapeSizeProvider : public PropertyValueProvider
{
public:
    explicit ShapeSizeProvider( const SvxShapeImpl& _shapeImpl )
        :PropertyValueProvider( _shapeImpl.mrAntiImpl, "Size" )
    {
    }

protected:
    virtual void getCurrentValue( Any& _out_rCurrentValue ) const override
    {
        _out_rCurrentValue <<= static_cast< SvxShape& >( getContext() ).getSize();
    }
};

SvxShape::SvxShape( SdrObject* pObject )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
,   maPropMapEntries(getSvxMapProvider().GetMap(SVXMAP_SHAPE))
,   mpSdrObjectWeakReference(pObject)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::SvxShape( SdrObject* pObject, const SfxItemPropertyMapEntry* pEntries, const SvxItemPropertySet* pPropertySet )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(pPropertySet)
,   maPropMapEntries(pEntries)
,   mpSdrObjectWeakReference(pObject)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::SvxShape()
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
,   maPropMapEntries(getSvxMapProvider().GetMap(SVXMAP_SHAPE))
,   mpSdrObjectWeakReference(nullptr)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::~SvxShape() throw()
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount == 0, "Locked shape was disposed!" );

    if ( mpImpl->mpMaster )
        mpImpl->mpMaster->dispose();

    if ( HasSdrObject() )
    {
        EndListening(GetSdrObject()->getSdrModelFromSdrObject());
        GetSdrObject()->setUnoShape(nullptr);
    }

    if( HasSdrObjectOwnership() && HasSdrObject() )
    {
        mpImpl->mbHasSdrObjectOwnership = false;
        SdrObject* pObject = GetSdrObject();
        SdrObject::Free( pObject );
    }

    EndListeningAll(); // call explicitly within SolarMutexGuard
}


void SvxShape::TakeSdrObjectOwnership()
{
    mpImpl->mbHasSdrObjectOwnership = true;
}


void SvxShape::InvalidateSdrObject()
{
    if(HasSdrObject())
    {
        EndListening(GetSdrObject()->getSdrModelFromSdrObject());
    }

    if (HasSdrObjectOwnership())
        return;

    mpSdrObjectWeakReference.reset( nullptr );
};

bool SvxShape::HasSdrObjectOwnership() const
{
    if ( !mpImpl->mbHasSdrObjectOwnership )
        return false;

    OSL_ENSURE( HasSdrObject(), "SvxShape::HasSdrObjectOwnership: have the ownership of an object which I don't know!" );
    return HasSdrObject();
}


void SvxShape::setShapeKind( sal_uInt32 nKind )
{
    mpImpl->mnObjId = nKind;
}


sal_uInt32 SvxShape::getShapeKind() const
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

namespace
{
    class theSvxShapeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvxShapeUnoTunnelId > {};
}

const css::uno::Sequence< sal_Int8 > & SvxShape::getUnoTunnelId() throw()
{
    return theSvxShapeUnoTunnelId::get().getSeq();
}


SvxShape* SvxShape::getImplementation( const uno::Reference< uno::XInterface >& xInt )
{
    uno::Reference< lang::XUnoTunnel > xUT( xInt, css::uno::UNO_QUERY );
    if( xUT.is() )
        return reinterpret_cast<SvxShape*>(sal::static_int_cast<sal_uIntPtr>(xUT->getSomething( SvxShape::getUnoTunnelId())));
    else
        return nullptr;
}


sal_Int64 SAL_CALL SvxShape::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    else
    {
        return 0;
    }
}


svx::PropertyChangeNotifier& SvxShape::getShapePropertyChangeNotifier()
{
    return mpImpl->maPropertyNotifier;
}


void SvxShape::impl_construct()
{
    mpImpl->maPropertyNotifier.registerProvider( svx::ShapeProperty::Position,
        std::shared_ptr<svx::IPropertyValueProvider>( new ShapePositionProvider( *mpImpl ) ) );
    mpImpl->maPropertyNotifier.registerProvider( svx::ShapeProperty::Size,
        std::shared_ptr<svx::IPropertyValueProvider>( new ShapeSizeProvider( *mpImpl ) ) );

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
        GetSdrObject()->setUnoShape(*this);
    }
    osl_atomic_decrement( &m_refCount );

    // #i40944#
    // Do not simply return when no model but do the type corrections
    // following below.
    const SdrInventor nInventor = GetSdrObject()->GetObjInventor();

    // is it one of ours (svx) ?
    if( nInventor == SdrInventor::Default || nInventor == SdrInventor::E3d || nInventor == SdrInventor::FmForm )
    {
        if(nInventor == SdrInventor::FmForm)
        {
            mpImpl->mnObjId = OBJ_UNO;
        }
        else
        {
            mpImpl->mnObjId = GetSdrObject()->GetObjIdentifier();
            if( nInventor == SdrInventor::E3d )
                mpImpl->mnObjId |= E3D_INVENTOR_FLAG;
        }

        switch(mpImpl->mnObjId)
        {
        case OBJ_CCUT:          // segment of circle
        case OBJ_CARC:          // arc of circle
        case OBJ_SECT:          // sector
            mpImpl->mnObjId = OBJ_CIRC;
            break;
        }
    }
}


void SvxShape::Create( SdrObject* pNewObj, SvxDrawPage* /*pNewPage*/ )
{
    DBG_TESTSOLARMUTEX();

    OSL_PRECOND( pNewObj, "SvxShape::Create: invalid new object!" );
    if ( !pNewObj )
        return;

    SdrObject* pCreatedObj = mpImpl->mpCreatedObj.get();
    OSL_ENSURE( ( pCreatedObj == nullptr ) || ( pCreatedObj == pNewObj ),
        "SvxShape::Create: the same shape used for two different objects?! Strange ..." );

    // Correct condition (#i52126#)
    if ( pCreatedObj == pNewObj )
        return;

    // Correct condition (#i52126#)
    mpImpl->mpCreatedObj = pNewObj;

    if( HasSdrObject() )
    {
        EndListening( GetSdrObject()->getSdrModelFromSdrObject() );
    }

    mpSdrObjectWeakReference.reset( pNewObj );

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

void SvxShape::ForceMetricToItemPoolMetric(Pair& rPoint) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(HasSdrObject())
    {
        MapUnit eMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    rPoint.A() = MM_TO_TWIPS(rPoint.A());
                    rPoint.B() = MM_TO_TWIPS(rPoint.B());
                    break;
                }
                default:
                {
                    OSL_FAIL("AW: Missing unit translation to PoolMetric!");
                }
            }
        }
    }
}

void SvxShape::ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(HasSdrObject())
    {
        MapUnit eMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    basegfx::B2DHomMatrix aTransform;
                    const double fMMToTWIPS(72.0 / 127.0);

                    aTransform.scale(fMMToTWIPS, fMMToTWIPS);
                    rPolyPolygon.transform(aTransform);
                    break;
                }
                default:
                {
                    OSL_FAIL("Missing unit translation to PoolMetric!");
                }
            }
        }
    }
}

void SvxShape::ForceMetricToItemPoolMetric(basegfx::B2DHomMatrix& rB2DHomMatrix) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(HasSdrObject())
    {
        MapUnit eMapUnit(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0));
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    const double fMMToTWIPS(72.0 / 127.0);
                    const basegfx::utils::B2DHomMatrixBufferedDecompose aDecomposedTransform(rB2DHomMatrix);
                    rB2DHomMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                        aDecomposedTransform.getScale() * fMMToTWIPS,
                        aDecomposedTransform.getShearX(),
                        aDecomposedTransform.getRotate(),
                        aDecomposedTransform.getTranslate() * fMMToTWIPS);
                    break;
                }
                default:
                {
                    OSL_FAIL("Missing unit translation to PoolMetric!");
                }
            }
        }
    }
}

void SvxShape::ForceMetricTo100th_mm(Pair& rPoint) const throw()
{
    DBG_TESTSOLARMUTEX();
    MapUnit eMapUnit = MapUnit::Map100thMM;
    if(HasSdrObject())
    {
        eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    rPoint.A() = TWIPS_TO_MM(rPoint.A());
                    rPoint.B() = TWIPS_TO_MM(rPoint.B());
                    break;
                }
                default:
                {
                    OSL_FAIL("AW: Missing unit translation to 100th mm!");
                }
            }
        }
    }
}

void SvxShape::ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const throw()
{
    DBG_TESTSOLARMUTEX();
    MapUnit eMapUnit = MapUnit::Map100thMM;
    if(HasSdrObject())
    {
        eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    basegfx::B2DHomMatrix aTransform;
                    const double fTWIPSToMM(127.0 / 72.0);
                    aTransform.scale(fTWIPSToMM, fTWIPSToMM);
                    rPolyPolygon.transform(aTransform);
                    break;
                }
                default:
                {
                    OSL_FAIL("Missing unit translation to 100th mm!");
                }
            }
        }
    }
}

void SvxShape::ForceMetricTo100th_mm(basegfx::B2DHomMatrix& rB2DHomMatrix) const throw()
{
    DBG_TESTSOLARMUTEX();
    MapUnit eMapUnit = MapUnit::Map100thMM;
    if(HasSdrObject())
    {
        eMapUnit = GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetMetric(0);
        if(eMapUnit != MapUnit::Map100thMM)
        {
            switch(eMapUnit)
            {
                case MapUnit::MapTwip :
                {
                    const double fTWIPSToMM(127.0 / 72.0);
                    const basegfx::utils::B2DHomMatrixBufferedDecompose aDecomposedTransform(rB2DHomMatrix);
                    rB2DHomMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                        aDecomposedTransform.getScale() * fTWIPSToMM,
                        aDecomposedTransform.getShearX(),
                        aDecomposedTransform.getRotate(),
                        aDecomposedTransform.getTranslate() * fTWIPSToMM);
                    break;
                }
                default:
                {
                    OSL_FAIL("Missing unit translation to 100th mm!");
                }
            }
        }
    }
}

void SvxItemPropertySet_ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet,
  SfxItemSet& rSet, const uno::Reference< beans::XPropertySet >& xSet, const SfxItemPropertyMap* pMap )
{
    if(rPropSet.AreThereOwnUsrAnys())
    {
        const SfxItemPropertyMap& rSrc = rPropSet.getPropertyMap();
        PropertyEntryVector_t aSrcPropVector = rSrc.getPropertyEntries();
        PropertyEntryVector_t::const_iterator aSrcIt = aSrcPropVector.begin();

        while(aSrcIt != aSrcPropVector.end())
        {
            const sal_uInt16 nWID = aSrcIt->nWID;
            if(SfxItemPool::IsWhich(nWID)
                    && (nWID < OWN_ATTR_VALUE_START || nWID > OWN_ATTR_VALUE_END)
                    && rPropSet.GetUsrAnyForID(nWID))
                rSet.Put(rSet.GetPool()->GetDefaultItem(nWID));
            ++aSrcIt;
        }

        aSrcIt = aSrcPropVector.begin();
        while(aSrcIt != aSrcPropVector.end())
        {
            if(aSrcIt->nWID)
            {
                uno::Any* pUsrAny = rPropSet.GetUsrAnyForID(aSrcIt->nWID);
                if(pUsrAny)
                {
                    // search for equivalent entry in pDst
                    const SfxItemPropertySimpleEntry* pEntry = pMap->getByName( aSrcIt->sName );
                    if(pEntry)
                    {
                        // entry found
                        if(pEntry->nWID >= OWN_ATTR_VALUE_START && pEntry->nWID <= OWN_ATTR_VALUE_END)
                        {
                            // special ID in PropertySet, can only be set
                            // directly at the object
                            xSet->setPropertyValue( aSrcIt->sName, *pUsrAny);
                        }
                        else
                        {
                            SvxItemPropertySet_setPropertyValue(pEntry, *pUsrAny, rSet);
                        }
                    }
                }
            }

            // next entry
            ++aSrcIt;
        }
        const_cast< SvxItemPropertySet& >(rPropSet).ClearAllUsrAny();
    }
}


void SvxShape::ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet)
{
    DBG_TESTSOLARMUTEX();
    if(HasSdrObject() && rPropSet.AreThereOwnUsrAnys())
    {
        SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), svl::Items<SDRATTR_START, SDRATTR_END>{});
        Reference< beans::XPropertySet > xShape( static_cast<OWeakObject*>(this), UNO_QUERY );
        SvxItemPropertySet_ObtainSettingsFromPropertySet(rPropSet, aSet, xShape, &mpPropSet->getPropertyMap() );

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

    // tdf#118662 instead of creating an E3dView instance every time to paint
    // a single SdrObject, use the existing SdrObject::SingleObjectPainter to
    // use less resources and runtime
    ScopedVclPtrInstance< VirtualDevice > pVDev;
    const tools::Rectangle aBoundRect(GetSdrObject()->GetCurrentBoundRect());

    if(bMetaFile)
    {
        GDIMetaFile aMtf;

        pVDev->SetMapMode(MapMode(MapUnit::Map100thMM));
        pVDev->EnableOutput(false);
        aMtf.Record(pVDev);
        GetSdrObject()->SingleObjectPainter(*pVDev.get());
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
        const drawinglayer::primitive2d::Primitive2DContainer xPrimitives(
            GetSdrObject()->GetViewContact().getViewIndependentPrimitive2DContainer());

        if(!xPrimitives.empty())
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;
            const basegfx::B2DRange aRange(
                xPrimitives.getB2DRange(aViewInformation2D));

            if(!aRange.isEmpty())
            {
                const BitmapEx aBmp(
                    convertPrimitive2DSequenceToBitmapEx(
                        xPrimitives,
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


uno::Sequence< uno::Type > SvxShape::_getTypes()
{
    switch( mpImpl->mnObjId )
    {
    // shapes without text
    case OBJ_PAGE:
    case OBJ_FRAME:
    case OBJ_OLE2_PLUGIN:
    case OBJ_OLE2_APPLET:
    case E3D_CUBEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_SPHEREOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_LATHEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_EXTRUDEOBJ_ID|E3D_INVENTOR_FLAG:
    case E3D_POLYGONOBJ_ID|E3D_INVENTOR_FLAG:
    case OBJ_MEDIA:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 12 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                }
            }
            return aTypeSequence;
        }
    // group shape
    case OBJ_GRUP:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 14 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    *pTypes++ = cppu::UnoType<drawing::XShapes>::get();
                    *pTypes++ = cppu::UnoType<drawing::XShapeGroup>::get();
                }
            }
            return aTypeSequence;
        }
    // connector shape
    case OBJ_EDGE:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 17 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    *pTypes++ = cppu::UnoType<drawing::XConnectorShape>::get();
                    // from SvxUnoTextBase::getTypes()
                    *pTypes++ = cppu::UnoType<text::XTextAppend>::get();
                    *pTypes++ = cppu::UnoType<text::XTextCopy>::get();
                    *pTypes++ = cppu::UnoType<container::XEnumerationAccess>::get();
                    *pTypes++ = cppu::UnoType<text::XTextRangeMover>::get();
                }
            }
            return aTypeSequence;
        }
    // control shape
    case OBJ_UNO:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 13 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    *pTypes++ = cppu::UnoType<drawing::XControlShape>::get();
                }
            }
            return aTypeSequence;
        }
    // 3d scene shape
    case E3D_SCENE_ID|E3D_INVENTOR_FLAG:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 13 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    *pTypes++ = cppu::UnoType<drawing::XShapes>::get();
                }
            }
            return aTypeSequence;
        }
    case OBJ_CUSTOMSHAPE:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 16 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    // from SvxUnoTextBase::getTypes()
                    *pTypes++ = cppu::UnoType<text::XText>::get();
                    *pTypes++ = cppu::UnoType<container::XEnumerationAccess>::get();
                    *pTypes++ = cppu::UnoType<text::XTextRangeMover>::get();
                    *pTypes++ = cppu::UnoType<drawing::XEnhancedCustomShapeDefaulter>::get();
                }
            }
            return aTypeSequence;
        }
    // shapes with text
    case OBJ_RECT:
    case OBJ_CIRC:
    case OBJ_MEASURE:
    case OBJ_LINE:
    case OBJ_POLY:
    case OBJ_PLIN:
    case OBJ_PATHLINE:
    case OBJ_PATHFILL:
    case OBJ_FREELINE:
    case OBJ_FREEFILL:
    case OBJ_PATHPOLY:
    case OBJ_PATHPLIN:
    case OBJ_GRAF:
    case OBJ_TEXT:
    case OBJ_CAPTION:
    case OBJ_TABLE:
    case OBJ_OLE2: // #i118485# Moved to shapes with text
    default:
        {
            static css::uno::Sequence< css::uno::Type > aTypeSequence;

            if( aTypeSequence.getLength() == 0 )
            {
                // Ready for multithreading; get global mutex for first call of this method only! see before
                MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

                // Control these pointer again ... it can be, that another instance will be faster then these!
                if( aTypeSequence.getLength() == 0 )
                {
                    aTypeSequence.realloc( 16 );
                    uno::Type* pTypes = aTypeSequence.getArray();

                    *pTypes++ = cppu::UnoType<drawing::XShape>::get();
                    *pTypes++ = cppu::UnoType<lang::XComponent>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertySet>::get();
                    *pTypes++ = cppu::UnoType<beans::XPropertyState>::get();
                    *pTypes++ = cppu::UnoType<beans::XMultiPropertyStates>::get();
                    *pTypes++ = cppu::UnoType<drawing::XGluePointsSupplier>::get();
                    *pTypes++ = cppu::UnoType<container::XChild>::get();
                    *pTypes++ = cppu::UnoType<lang::XServiceInfo>::get();
                    *pTypes++ = cppu::UnoType<lang::XTypeProvider>::get();
                    *pTypes++ = cppu::UnoType<lang::XUnoTunnel>::get();
                    *pTypes++ = cppu::UnoType<container::XNamed>::get();
                    // from SvxUnoTextBase::getTypes()
                    *pTypes++ = cppu::UnoType<text::XTextAppend>::get();
                    *pTypes++ = cppu::UnoType<text::XTextCopy>::get();
                    *pTypes++ = cppu::UnoType<container::XEnumerationAccess>::get();
                    *pTypes++ = cppu::UnoType<text::XTextRangeMover>::get();
                }
            }
            return aTypeSequence;
        }
    }
}


uno::Sequence< sal_Int8 > SAL_CALL SvxShape::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

void SvxShape::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    DBG_TESTSOLARMUTEX();
    if( !HasSdrObject() )
        return;

    // #i55919# SdrHintKind::ObjectChange is only interesting if it's for this object

    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (!pSdrHint ||
        ((pSdrHint->GetKind() != SdrHintKind::ModelCleared) &&
         (pSdrHint->GetKind() != SdrHintKind::ObjectChange || pSdrHint->GetObject() != GetSdrObject() )))
        return;

    uno::Reference< uno::XInterface > xSelf( GetSdrObject()->getWeakUnoShape() );
    if( !xSelf.is() )
    {
        EndListening(GetSdrObject()->getSdrModelFromSdrObject());
        mpSdrObjectWeakReference.reset( nullptr );
        return;
    }

    bool bClearMe = false;

    switch( pSdrHint->GetKind() )
    {
        case SdrHintKind::ObjectChange:
        {
            updateShapeKind();
            break;
        }
        case SdrHintKind::ModelCleared:
        {
            bClearMe = true;
            break;
        }
        default:
            break;
    };

    if( bClearMe )
    {
        SdrObject* pSdrObject(GetSdrObject());

        if(!HasSdrObjectOwnership())
        {
            if(nullptr != pSdrObject)
            {
                EndListening(pSdrObject->getSdrModelFromSdrObject());
                pSdrObject->setUnoShape(nullptr);
            }

            mpSdrObjectWeakReference.reset(nullptr);

            // SdrModel *is* going down, try to Free SdrObject even
            // when !HasSdrObjectOwnership
            if(nullptr != pSdrObject && !pSdrObject->IsInserted())
            {
                SdrObject::Free(pSdrObject);
            }
        }

        if(!mpImpl->mbDisposing)
        {
            dispose();
        }
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
        case OBJ_GRUP:
        case OBJ_LINE:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_SPLNLINE:
        case OBJ_SPLNFILL:
        case OBJ_EDGE:
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
        case OBJ_MEASURE:
            return true;
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

            long nDX = aLocalPos.X() - aRect.Left();
            long nDY = aLocalPos.Y() - aRect.Top();

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
        Size aObjSize( aRect.getWidth(), aRect.getHeight() );
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
        // #i123539# optimization for 3D chart object generation: do not use UNO
        // API commands to get the range, this is too expensive since for 3D
        // scenes it may recalculate the whole scene since in AOO this depends
        // on the contained geometry (layouted to show all content)
        const bool b3DConstruction(dynamic_cast< E3dObject* >(GetSdrObject()) && GetSdrObject()->getSdrModelFromSdrObject().isLocked());
        tools::Rectangle aRect(
            b3DConstruction ?
                tools::Rectangle(maPosition.X, maPosition.Y, maSize.Width, maSize.Height) :
                svx_getLogicRectHack(GetSdrObject()) );
        Size aLocalSize( rSize.Width, rSize.Height );
        ForceMetricToItemPoolMetric(aLocalSize);

        if(GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == OBJ_MEASURE )
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
    ::SolarMutexGuard aGuard;

    if( mpImpl->mbDisposing )
        return; // caught a recursion

    mpImpl->mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *static_cast<OWeakAggObject*>(this);
    mpImpl->maDisposeListeners.disposeAndClear(aEvt);
    mpImpl->maPropertyNotifier.disposing();

    if ( HasSdrObject() )
    {
        EndListening( GetSdrObject()->getSdrModelFromSdrObject() );
        bool bFreeSdrObject = false;

        if ( GetSdrObject()->IsInserted() && GetSdrObject()->getSdrPageFromSdrObject() )
        {
            OSL_ENSURE( HasSdrObjectOwnership(), "SvxShape::dispose: is the below code correct?" );
                // normally, we are allowed to free the SdrObject only if we have its ownership.
                // Why isn't this checked here?

            SdrPage* pPage = GetSdrObject()->getSdrPageFromSdrObject();
            // delete the SdrObject from the page
            const size_t nCount = pPage->GetObjCount();
            for ( size_t nNum = 0; nNum < nCount; ++nNum )
            {
                if ( pPage->GetObj( nNum ) == GetSdrObject() )
                {
                    OSL_VERIFY( pPage->RemoveObject( nNum ) == GetSdrObject() );
                    bFreeSdrObject = true;
                    break;
                }
            }
        }

        GetSdrObject()->setUnoShape(nullptr);

        if ( bFreeSdrObject )
        {
            // in case we have the ownership of the SdrObject, a Free
            // would do nothing. So ensure the ownership is reset.
            mpImpl->mbHasSdrObjectOwnership = false;
            SdrObject* pObject = GetSdrObject();
            SdrObject::Free( pObject );
        }
    }
}


void SAL_CALL SvxShape::addEventListener( const Reference< lang::XEventListener >& xListener )
{
    mpImpl->maDisposeListeners.addInterface(xListener);
}


void SAL_CALL SvxShape::removeEventListener( const Reference< lang::XEventListener >& aListener )
{
   mpImpl->maDisposeListeners.removeInterface(aListener);
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
    ::osl::MutexGuard aGuard( maMutex );
    mpImpl->maPropertyNotifier.addPropertyChangeListener( _propertyName, _listener );
}


void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& _propertyName, const Reference< beans::XPropertyChangeListener >& _listener  )
{
    ::osl::MutexGuard aGuard( maMutex );
    mpImpl->maPropertyNotifier.removePropertyChangeListener( _propertyName, _listener );
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
        SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(), {{nWID, nWID}} );

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

            long nPos = pBitmapList->GetIndex(aStrName);
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

            long nPos = pGradientList->GetIndex(aStrName);
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

            long nPos = pHatchList->GetIndex(aStrName);
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

            long nPos = pLineEndList->GetIndex(aStrName);
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

            long nPos = pDashList->GetIndex(aStrName);
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
                    rSet.Put( XLineEndItem( "", aEmptyPoly ) );
                else
                    rSet.Put( XLineStartItem( "", aEmptyPoly ) );

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

    const SfxItemPool* pPool = rSet.GetPool();

    const sal_uInt32 nCount = pPool->GetItemCount2(nWID);

    for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(pPool->GetItem2(nWID, nSurrogate));
        if( pItem && ( pItem->GetName() == aName ) )
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

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(rPropertyName);

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
            mpPropSet->setPropertyValue( pMap, rVal );
        }

        return;
    }

    if (!pMap)
    {
        SAL_WARN("svx.uno", "Unknown Property: " << rPropertyName);
        throw beans::UnknownPropertyException( rPropertyName, static_cast<cppu::OWeakObject*>(this));
    }

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

    if( pMap->nWID == SDRATTR_ECKENRADIUS )
    {
        sal_Int32 nCornerRadius = 0;
        if( !(rVal >>= nCornerRadius) || (nCornerRadius < 0) || (nCornerRadius > 5000000))
            throw IllegalArgumentException();
    }

    SfxItemSet* pSet;
    if( mbIsMultiPropertyCall && !bIsNotPersist )
    {
        if( mpImpl->mpItemSet == nullptr )
        {
            mpImpl->mpItemSet = GetSdrObject()->GetMergedItemSet().Clone();
        }
        pSet = mpImpl->mpItemSet.get();
    }
    else
    {
        pSet = new SfxItemSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(),  {{pMap->nWID, pMap->nWID}});
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
                pSet->Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetDefaultItem(pMap->nWID));
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
        delete pSet;
    }
    else
    {
        // if we have a XMultiProperty call then the item set
        // will be set in setPropertyValues later
        if( !mbIsMultiPropertyCall )
        {
            GetSdrObject()->SetMergedItemSetAndBroadcast( *pSet );

            delete pSet;
        }
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

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    uno::Any aAny;
    if(HasSdrObject())
    {
        if(pMap == nullptr )
            throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

        if( !getPropertyValueImpl( PropertyName, pMap, aAny ) )
        {
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || (pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST), "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(),    {{pMap->nWID, pMap->nWID}});
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
                    aSet.Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetDefaultItem(pMap->nWID));
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
            aAny = mpPropSet->getPropertyValue( pMap );

    }
    return aAny;
}


// XMultiPropertySet
void SAL_CALL SvxShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues )
{
    ::SolarMutexGuard aSolarGuard;

    const sal_Int32 nCount = aPropertyNames.getLength();
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
            catch( beans::UnknownPropertyException& ) {}
            catch( uno::Exception& ) {}
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
            catch( beans::UnknownPropertyException& ) {}
            catch( uno::Exception& ) {}
        }
    }

    if( mpImpl->mpItemSet && HasSdrObject() )
        GetSdrObject()->SetMergedItemSetAndBroadcast( *mpImpl->mpItemSet );
}


void SvxShape::endSetPropertyValues()
{
    mbIsMultiPropertyCall = false;
    mpImpl->mpItemSet.reset();
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


uno::Any SvxShape::GetAnyForItem( SfxItemSet const & aSet, const SfxItemPropertySimpleEntry* pMap ) const
{
    DBG_TESTSOLARMUTEX();
    uno::Any aAny;

    switch(pMap->nWID)
    {
    case SDRATTR_CIRCSTARTANGLE:
    {
        const SfxPoolItem* pPoolItem=nullptr;
        if(aSet.GetItemState(SDRATTR_CIRCSTARTANGLE,false,&pPoolItem)==SfxItemState::SET)
        {
            sal_Int32 nAngle = static_cast<const SdrAngleItem*>(pPoolItem)->GetValue();
            aAny <<= nAngle;
        }
        break;
    }

    case SDRATTR_CIRCENDANGLE:
    {
        const SfxPoolItem* pPoolItem=nullptr;
        if (aSet.GetItemState(SDRATTR_CIRCENDANGLE,false,&pPoolItem)==SfxItemState::SET)
        {
            sal_Int32 nAngle = static_cast<const SdrAngleItem*>(pPoolItem)->GetValue();
            aAny <<= nAngle;
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
            case OBJ_CIRC:          // circle, ellipse
                eKind = drawing::CircleKind_FULL;
                break;
            case OBJ_CCUT:          // segment of circle
                eKind = drawing::CircleKind_CUT;
                break;
            case OBJ_CARC:          // arc of circle
                eKind = drawing::CircleKind_ARC;
                break;
            case OBJ_SECT:          // sector
                eKind = drawing::CircleKind_SECTION;
                break;
            }
            aAny <<= eKind;
        }
        break;
    }
    default:
    {
        // get value form ItemSet
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
                OSL_FAIL("SvxShape::GetAnyForItem() Returnvalue has wrong Type!" );
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

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !HasSdrObject() || pMap == nullptr )
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

    beans::PropertyState eState;
    if( !getPropertyStateImpl( pMap, eState ) )
    {
        const SfxItemSet& rSet = GetSdrObject()->GetMergedItemSet();

        switch( rSet.GetItemState( pMap->nWID, false ) )
        {
        case SfxItemState::READONLY:
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

        // if a item is set, this doesn't mean we want it :)
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
            }
        }
    }
    return eState;
}

bool SvxShape::setPropertyValueImpl( const OUString&, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
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
            GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aVclPoint.AdjustX(basegfx::fround(aNewHomogenMatrix.get(0, 2)) );
            aVclPoint.AdjustY(basegfx::fround(aNewHomogenMatrix.get(1, 2)) );

            // #88491# position relative to anchor
            if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
            {
                aVclPoint += GetSdrObject()->GetAnchorPos();
            }

            static_cast<SdrCaptionObj*>(GetSdrObject())->SetTailPos(aVclPoint);

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
            GetSdrObject()->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aNewHomogenMatrix.set(0, 0, aMatrix.Line1.Column1);
            aNewHomogenMatrix.set(0, 1, aMatrix.Line1.Column2);
            aNewHomogenMatrix.set(0, 2, aMatrix.Line1.Column3);
            aNewHomogenMatrix.set(1, 0, aMatrix.Line2.Column1);
            aNewHomogenMatrix.set(1, 1, aMatrix.Line2.Column2);
            aNewHomogenMatrix.set(1, 2, aMatrix.Line2.Column3);
            aNewHomogenMatrix.set(2, 0, aMatrix.Line3.Column1);
            aNewHomogenMatrix.set(2, 1, aMatrix.Line3.Column2);
            aNewHomogenMatrix.set(2, 2, aMatrix.Line3.Column3);

            // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
            // Need to adapt aNewHomogenMatrix from 100thmm to app-specific
            ForceMetricToItemPoolMetric(aNewHomogenMatrix);

            GetSdrObject()->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            return true;
        }
        break;
    }

    case OWN_ATTR_ZORDER:
    {
        sal_Int32 nNewOrdNum = 0;
        if(rValue >>= nNewOrdNum)
        {
            SdrObjList* pObjList = GetSdrObject()->getParentSdrObjListFromSdrObject();
            if( pObjList )
            {
                SdrObject* pCheck =
                            pObjList->SetObjectOrdNum( GetSdrObject()->GetOrdNum(), static_cast<size_t>(nNewOrdNum) );
                DBG_ASSERT( pCheck == GetSdrObject(), "GetOrdNum() failed!" );
            }
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
            tools::Rectangle aRect;
            aRect.SetPos(aTopLeft);
            aRect.SetSize(aObjSize);
            GetSdrObject()->SetSnapRect(aRect);
            return true;
        }
        break;
    }
    case OWN_ATTR_MIRRORED:
    {
        bool bMirror;
        if(rValue >>= bMirror )
        {
            SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >( GetSdrObject() );
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
        SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >(GetSdrObject());
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
                        SdrObject* pNode = GetSdrObjectFromXShape( xShape );
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
                        if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                            aPoint += GetSdrObject()->GetAnchorPos();

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
                        if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                        {
                            Point aPoint( GetSdrObject()->GetAnchorPos() );
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
        SdrMeasureObj* pMeasureObj = dynamic_cast< SdrMeasureObj* >(GetSdrObject());
        awt::Point aUnoPoint;
        if(pMeasureObj && ( rValue >>= aUnoPoint ) )
        {
            Point aPoint( aUnoPoint.X, aUnoPoint.Y );

            // Reintroduction of fix for issue #i59051# (#i108851#)
            ForceMetricToItemPoolMetric( aPoint );
            if( GetSdrObject()->getSdrModelFromSdrObject().IsWriter() )
                aPoint += GetSdrObject()->GetAnchorPos();

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
            GetSdrObject()->SetMergedItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
            GetSdrObject()->SetMergedItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
            return true;
        }

    case SDRATTR_LAYERID:
    {
        sal_Int16 nLayerId = sal_Int16();
        if( rValue >>= nLayerId )
        {
            SdrLayer* pLayer = GetSdrObject()->getSdrModelFromSdrObject().GetLayerAdmin().GetLayerPerID(SdrLayerID(nLayerId));
            if( pLayer )
            {
                GetSdrObject()->SetLayer(SdrLayerID(nLayerId));
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
            const SdrLayer* pLayer = GetSdrObject()->getSdrModelFromSdrObject().GetLayerAdmin().GetLayer(aLayerName);
            if( pLayer != nullptr )
            {
                GetSdrObject()->SetLayer( pLayer->GetID() );
                return true;
            }
        }
        break;
    }
    case SDRATTR_ROTATEANGLE:
    {
        sal_Int32 nAngle = 0;
        if( rValue >>= nAngle )
        {
            Point aRef1(GetSdrObject()->GetSnapRect().Center());
            nAngle -= GetSdrObject()->GetRotateAngle();
            if (nAngle!=0)
            {
                double nSin=sin(nAngle*nPi180);
                double nCos=cos(nAngle*nPi180);
                GetSdrObject()->Rotate(aRef1,nAngle,nSin,nCos);
            }
            return true;
        }

        break;
    }

    case SDRATTR_SHEARANGLE:
    {
        sal_Int32 nShear = 0;
        if( rValue >>= nShear )
        {
            nShear -= GetSdrObject()->GetShearAngle();
            if(nShear != 0 )
            {
                Point aRef1(GetSdrObject()->GetSnapRect().Center());
                double nTan=tan(nShear*nPi180);
                GetSdrObject()->Shear(aRef1,nShear,nTan,false);
                return true;
            }
        }

        break;
    }

    case OWN_ATTR_INTEROPGRABBAG:
    {
        GetSdrObject()->SetGrabBagItem(rValue);
        return true;
    }

    case SDRATTR_OBJMOVEPROTECT:
    {
        bool bMoveProtect;
        if( rValue >>= bMoveProtect )
        {
            GetSdrObject()->SetMoveProtect(bMoveProtect);
            return true;
        }
        break;
    }
    case SDRATTR_OBJECTNAME:
    {
        OUString aName;
        if( rValue >>= aName )
        {
            GetSdrObject()->SetName( aName );
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
            GetSdrObject()->SetTitle( aTitle );
            return true;
        }
        break;
    }
    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription;
        if( rValue >>= aDescription )
        {
            GetSdrObject()->SetDescription( aDescription );
            return true;
        }
        break;
    }

    case SDRATTR_OBJPRINTABLE:
    {
        bool bPrintable;
        if( rValue >>= bPrintable )
        {
            GetSdrObject()->SetPrintable(bPrintable);
            return true;
        }
        break;
    }
    case SDRATTR_OBJVISIBLE:
    {
        bool bVisible;
        if( rValue >>= bVisible )
        {
            GetSdrObject()->SetVisible(bVisible);
            return true;
        }
        break;
    }
    case SDRATTR_OBJSIZEPROTECT:
    {
        bool bResizeProtect;
        if( rValue >>= bResizeProtect )
        {
            GetSdrObject()->SetResizeProtect(bResizeProtect);
            return true;
        }
        break;
    }
    case OWN_ATTR_PAGE_NUMBER:
    {
        sal_Int32 nPageNum = 0;
        if( (rValue >>= nPageNum) && ( nPageNum >= 0 ) && ( nPageNum <= 0xffff ) )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(GetSdrObject());
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
    default:
    {
        return false;
    }
    }
    throw lang::IllegalArgumentException();
}


bool SvxShape::getPropertyValueImpl( const OUString&, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
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

        aVclPoint.AdjustX( -(basegfx::fround(aNewHomogenMatrix.get(0, 2))) );
        aVclPoint.AdjustY( -(basegfx::fround(aNewHomogenMatrix.get(1, 2))) );

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
        aMatrix.Line3.Column1 = aNewHomogenMatrix.get(2, 0);
        aMatrix.Line3.Column2 = aNewHomogenMatrix.get(2, 1);
        aMatrix.Line3.Column3 = aNewHomogenMatrix.get(2, 2);

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
        rValue <<= dynamic_cast<const SdrTextObj*>(GetSdrObject()) != nullptr && static_cast<SdrTextObj*>(GetSdrObject())->IsFontwork();
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
        if( GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == OBJ_OLE2 )
        {
            sId = RID_UNODRAW_OLE2;
        }
        else if( GetSdrObject()->GetObjInventor() == SdrInventor::Default && GetSdrObject()->GetObjIdentifier() == OBJ_GRAF )
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
        if( HasSdrObject() && dynamic_cast<const SdrGrafObj*>(GetSdrObject()) != nullptr )
            bMirror = static_cast<SdrGrafObj*>(GetSdrObject())->IsMirrored();

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

        const XFillBmpStretchItem* pStretchItem = &rObjItemSet.Get(XATTR_FILLBMP_STRETCH);
        const XFillBmpTileItem* pTileItem = &rObjItemSet.Get(XATTR_FILLBMP_TILE);

        if( pTileItem && pTileItem->GetValue() )
        {
            rValue <<= drawing::BitmapMode_REPEAT;
        }
        else if( pStretchItem && pStretchItem->GetValue() )
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
        rValue <<= sal_uInt16(sal_uInt8(GetSdrObject()->GetLayer()));
        break;

    case SDRATTR_LAYERNAME:
    {
        SdrLayer* pLayer = GetSdrObject()->getSdrModelFromSdrObject().GetLayerAdmin().GetLayerPerID(GetSdrObject()->GetLayer());
        if( pLayer )
        {
            OUString aName( pLayer->GetName() );
            rValue <<= aName;
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
            sal_Int32 nPageNumber = pPage ? pPage->GetPageNum() : 0L;
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
                    // #119735# just use GetGDIMetaFile, it will create a bufferd version of contained bitmap now automatically
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


    default:
        return false;
    }
    return true;
}


bool SvxShape::getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState )
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


bool SvxShape::setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty )
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
    const OUString* pNames = aPropertyName.getConstArray();

    uno::Sequence< beans::PropertyState > aRet( nCount );
    beans::PropertyState* pState = aRet.getArray();

    for( sal_Int32 nIdx = 0; nIdx < nCount; nIdx++ )
        pState[nIdx] = getPropertyState( pNames[nIdx] );

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

    const SfxItemPropertySimpleEntry* pProperty = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !HasSdrObject() || pProperty == nullptr )
        throw beans::UnknownPropertyException( PropertyName, static_cast<cppu::OWeakObject*>(this));

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

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(aPropertyName);

    if( !HasSdrObject() || pMap == nullptr )
        throw beans::UnknownPropertyException( aPropertyName, static_cast<cppu::OWeakObject*>(this));

    if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return getPropertyValue( aPropertyName );
    }

    // get default from ItemPool
    if(!SfxItemPool::IsWhich(pMap->nWID))
        throw beans::UnknownPropertyException( "No WhichID " + OUString::number(pMap->nWID) + " for " + aPropertyName, static_cast<cppu::OWeakObject*>(this));

    SfxItemSet aSet( GetSdrObject()->getSdrModelFromSdrObject().GetItemPool(),    {{pMap->nWID, pMap->nWID}});
    aSet.Put(GetSdrObject()->getSdrModelFromSdrObject().GetItemPool().GetDefaultItem(pMap->nWID));

    return GetAnyForItem( aSet, pMap );
}

// XMultiPropertyStates
void SvxShape::setAllPropertiesToDefault()
{
    ::SolarMutexGuard aGuard;

    if( !HasSdrObject() )
        throw lang::DisposedException();
    GetSdrObject()->ClearMergedItem(); // nWhich == 0 => all

    if(dynamic_cast<const SdrGrafObj*>(GetSdrObject()) != nullptr)
    {
        // defaults for graphic objects have changed:
        GetSdrObject()->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );
        GetSdrObject()->SetMergedItem( XLineStyleItem( drawing::LineStyle_NONE ) );
    }

    // #i68523# special handling for Svx3DCharacterModeItem, this is not saved
    // but needs to be sal_True in svx, pool default (false) in sch. Since sch
    // does not load lathe or extrude objects, it is possible to set the items
    // here.
    // For other solution possibilities, see task description.
    if( dynamic_cast<const E3dLatheObj* >(GetSdrObject())  != nullptr|| dynamic_cast<const E3dExtrudeObj* >(GetSdrObject()) != nullptr)
    {
        GetSdrObject()->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    GetSdrObject()->getSdrModelFromSdrObject().SetChanged();
}

void SvxShape::setPropertiesToDefault(
    const uno::Sequence<OUString>& aPropertyNames )
{
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        setPropertyToDefault( aPropertyNames[pos] );
}

uno::Sequence<uno::Any> SvxShape::getPropertyDefaults(
    const uno::Sequence<OUString>& aPropertyNames )
{
    ::std::vector<uno::Any> ret;
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        ret.push_back( getPropertyDefault( aPropertyNames[pos] ) );
    return uno::Sequence<uno::Any>( &ret[0], ret.size() );
}


// XServiceInfo

OUString SAL_CALL SvxShape::getImplementationName()
{
    return OUString("SvxShape");
}

const char sUNO_service_style_ParagraphProperties[]          = "com.sun.star.style.ParagraphProperties";
const char sUNO_service_style_ParagraphPropertiesComplex[]   = "com.sun.star.style.ParagraphPropertiesComplex";
const char sUNO_service_style_ParagraphPropertiesAsian[]     = "com.sun.star.style.ParagraphPropertiesAsian";
const char sUNO_service_style_CharacterProperties[]          = "com.sun.star.style.CharacterProperties";
const char sUNO_service_style_CharacterPropertiesComplex[]   = "com.sun.star.style.CharacterPropertiesComplex";
const char sUNO_service_style_CharacterPropertiesAsian[]     = "com.sun.star.style.CharacterPropertiesAsian";

const char sUNO_service_drawing_FillProperties[]             = "com.sun.star.drawing.FillProperties";
const char sUNO_service_drawing_TextProperties[]             = "com.sun.star.drawing.TextProperties";
const char sUNO_service_drawing_LineProperties[]             = "com.sun.star.drawing.LineProperties";
const char sUNO_service_drawing_ConnectorProperties[]        = "com.sun.star.drawing.ConnectorProperties";
const char sUNO_service_drawing_MeasureProperties[]          = "com.sun.star.drawing.MeasureProperties";
const char sUNO_service_drawing_ShadowProperties[]           = "com.sun.star.drawing.ShadowProperties";

const char sUNO_service_drawing_RotationDescriptor[]         = "com.sun.star.drawing.RotationDescriptor";

const char sUNO_service_drawing_Text[]                       = "com.sun.star.drawing.Text";
const char sUNO_service_drawing_GroupShape[]                 = "com.sun.star.drawing.GroupShape";

const char sUNO_service_drawing_CustomShapeProperties[]      = "com.sun.star.drawing.CustomShapeProperties";
const char sUNO_service_drawing_CustomShape[]                = "com.sun.star.drawing.CustomShape";

const char sUNO_service_drawing_PolyPolygonDescriptor[]      = "com.sun.star.drawing.PolyPolygonDescriptor";
const char sUNO_service_drawing_PolyPolygonBezierDescriptor[]= "com.sun.star.drawing.PolyPolygonBezierDescriptor";

const char sUNO_service_drawing_LineShape[]                  = "com.sun.star.drawing.LineShape";
const char sUNO_service_drawing_Shape[]                      = "com.sun.star.drawing.Shape";
const char sUNO_service_drawing_RectangleShape[]             = "com.sun.star.drawing.RectangleShape";
const char sUNO_service_drawing_EllipseShape[]               = "com.sun.star.drawing.EllipseShape";
const char sUNO_service_drawing_PolyPolygonShape[]           = "com.sun.star.drawing.PolyPolygonShape";
const char sUNO_service_drawing_PolyLineShape[]              = "com.sun.star.drawing.PolyLineShape";
const char sUNO_service_drawing_OpenBezierShape[]            = "com.sun.star.drawing.OpenBezierShape";
const char sUNO_service_drawing_ClosedBezierShape[]          = "com.sun.star.drawing.ClosedBezierShape";
const char sUNO_service_drawing_TextShape[]                  = "com.sun.star.drawing.TextShape";
const char sUNO_service_drawing_GraphicObjectShape[]         = "com.sun.star.drawing.GraphicObjectShape";
const char sUNO_service_drawing_OLE2Shape[]                  = "com.sun.star.drawing.OLE2Shape";
const char sUNO_service_drawing_PageShape[]                  = "com.sun.star.drawing.PageShape";
const char sUNO_service_drawing_CaptionShape[]               = "com.sun.star.drawing.CaptionShape";
const char sUNO_service_drawing_MeasureShape[]               = "com.sun.star.drawing.MeasureShape";
const char sUNO_service_drawing_FrameShape[]                 = "com.sun.star.drawing.FrameShape";
const char sUNO_service_drawing_ControlShape[]               = "com.sun.star.drawing.ControlShape";
const char sUNO_service_drawing_ConnectorShape[]             = "com.sun.star.drawing.ConnectorShape";
const char sUNO_service_drawing_MediaShape[]                 = "com.sun.star.drawing.MediaShape";


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
        const sal_uInt16 nIdent = GetSdrObject()->GetObjIdentifier();

        switch(nIdent)
        {
        case OBJ_GRUP:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_GroupServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_GroupServices,
                            {sUNO_service_drawing_GroupShape,
                              sUNO_service_drawing_Shape} );

                        pSeq = &SvxShape_GroupServices;
                }

                return *pSeq;
            }
        case OBJ_CUSTOMSHAPE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_CustomShapeServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_CustomShapeServices,
                            {sUNO_service_drawing_CustomShape,
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
                            sUNO_service_drawing_RotationDescriptor});
                        pSeq = &SvxShape_CustomShapeServices;
                }
                return *pSeq;
            }
        case OBJ_LINE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_LineServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_LineServices,
                            {sUNO_service_drawing_LineShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_LineServices;
                }
                return *pSeq;
            }

        case OBJ_RECT:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_RectServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_RectServices,
                            {sUNO_service_drawing_RectangleShape,

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
                            sUNO_service_drawing_RotationDescriptor});
                        pSeq = &SvxShape_RectServices;
                }
                return *pSeq;
            }

        case OBJ_CIRC:
        case OBJ_SECT:
        case OBJ_CARC:
        case OBJ_CCUT:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_CircServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_CircServices,
                            {sUNO_service_drawing_EllipseShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_CircServices;
                }

                return *pSeq;
            }

        case OBJ_PATHPLIN:
        case OBJ_PLIN:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_PathServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_PathServices,
                            {sUNO_service_drawing_PolyLineShape,

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
                            sUNO_service_drawing_RotationDescriptor});
                        pSeq = &SvxShape_PathServices;
                }
                return *pSeq;
            }

        case OBJ_PATHPOLY:
        case OBJ_POLY:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_PolyServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_PolyServices,
                            {sUNO_service_drawing_PolyPolygonShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_PolyServices;
                }
                return *pSeq;
            }

        case OBJ_FREELINE:
        case OBJ_PATHLINE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_FreeLineServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_FreeLineServices,
                            {sUNO_service_drawing_OpenBezierShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_FreeLineServices;
                }

                return *pSeq;
            }

        case OBJ_FREEFILL:
        case OBJ_PATHFILL:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_FreeFillServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_FreeFillServices,
                            {sUNO_service_drawing_ClosedBezierShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_FreeFillServices;
                }
                return *pSeq;
            }

        case OBJ_OUTLINETEXT:
        case OBJ_TITLETEXT:
        case OBJ_TEXT:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_TextServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_TextServices,
                            {sUNO_service_drawing_TextShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_TextServices;
                }
                return *pSeq;
            }

        case OBJ_GRAF:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_GrafServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_GrafServices,
                            {sUNO_service_drawing_GraphicObjectShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_GrafServices;
                }
                return *pSeq;
            }

        case OBJ_OLE2:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_Ole2Services;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_Ole2Services,
                            {sUNO_service_drawing_OLE2Shape,
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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_Ole2Services;
                }
                return *pSeq;
            }

        case OBJ_CAPTION:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_CaptionServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_CaptionServices,
                            {sUNO_service_drawing_CaptionShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_CaptionServices;
                }

                return *pSeq;
            }

        case OBJ_PAGE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_PageServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_PageServices,
                            {sUNO_service_drawing_PageShape,
                            sUNO_service_drawing_Shape} );

                        pSeq = &SvxShape_PageServices;
                }

                return *pSeq;
            }

        case OBJ_MEASURE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_MeasureServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_MeasureServices,
                            {sUNO_service_drawing_MeasureShape,

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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_MeasureServices;
                }

                return *pSeq;
            }

        case OBJ_FRAME:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_FrameServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_FrameServices,
                            {sUNO_service_drawing_FrameShape,
                            sUNO_service_drawing_Shape} );

                        pSeq = &SvxShape_FrameServices;
                }

                return *pSeq;
            }

        case OBJ_UNO:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_UnoServices;
                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_UnoServices,
                            {sUNO_service_drawing_ControlShape,
                            sUNO_service_drawing_Shape} );

                        pSeq = &SvxShape_UnoServices;
                }
                return *pSeq;
            }

        case OBJ_EDGE:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_EdgeServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_EdgeServices,
                            {sUNO_service_drawing_ConnectorShape,
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
                            sUNO_service_drawing_RotationDescriptor});

                        pSeq = &SvxShape_EdgeServices;
                }
                return *pSeq;
            }
        case OBJ_MEDIA:
            {
                static uno::Sequence< OUString > *pSeq = nullptr;
                if( nullptr == pSeq )
                {
                        static uno::Sequence< OUString > SvxShape_MediaServices;

                        comphelper::ServiceInfoHelper::addToSequence( SvxShape_MediaServices,
                            {sUNO_service_drawing_MediaShape,
                            sUNO_service_drawing_Shape});

                        pSeq = &SvxShape_MediaServices;
                }
                return *pSeq;
            }
        }
    }
    else if( HasSdrObject() && GetSdrObject()->GetObjInventor() == SdrInventor::FmForm)
    {
#if OSL_DEBUG_LEVEL > 0
        const sal_uInt16 nIdent = GetSdrObject()->GetObjIdentifier();
        OSL_ENSURE( nIdent == OBJ_UNO, "SvxShape::_getSupportedServiceNames: SdrInventor::FmForm, but no UNO object?" );
#endif
        static uno::Sequence< OUString > *pSeq = nullptr;
        if( nullptr == pSeq )
        {
                static uno::Sequence< OUString > SvxShape_UnoServices;
                comphelper::ServiceInfoHelper::addToSequence( SvxShape_UnoServices,
                    {sUNO_service_drawing_ControlShape,
                    sUNO_service_drawing_Shape} );

                pSeq = &SvxShape_UnoServices;
        }
        return *pSeq;
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
        uno::Reference< container::XIndexContainer > xNew( SvxUnoGluePointAccess_createInstance( GetSdrObject() ), uno::UNO_QUERY );
        mxGluePoints = xGluePoints = xNew;
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


/** since polygon shapes can change theire kind during editing, we have
    to recheck it here.
    Circle shapes also change theire kind, but theire all treated equal
    so no update is necessary.
*/
void SvxShape::updateShapeKind()
{
    switch( mpImpl->mnObjId )
    {
        case OBJ_LINE:
        case OBJ_POLY:
        case OBJ_PLIN:
        case OBJ_PATHLINE:
        case OBJ_PATHFILL:
        case OBJ_FREELINE:
        case OBJ_FREEFILL:
        case OBJ_PATHPOLY:
        case OBJ_PATHPLIN:
        {
            const sal_uInt32 nId = GetSdrObject()->GetObjIdentifier();

            if( nId != mpImpl->mnObjId )
            {
                mpImpl->mnObjId = nId;

            }
            break;
        }
    }
}

SvxShapeText::SvxShapeText(SdrObject* pObject)
: SvxShape( pObject, getSvxMapProvider().GetMap(SVXMAP_TEXT), getSvxMapProvider().GetPropertySet(SVXMAP_TEXT, SdrObject::GetGlobalDrawObjectItemPool()) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject )
        SetEditSource( new SvxTextEditSource( pObject, nullptr ) );
}


SvxShapeText::SvxShapeText(SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet)
: SvxShape( pObject, pPropertyMap, pPropertySet ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject )
        SetEditSource( new SvxTextEditSource( pObject, nullptr ) );
}


SvxShapeText::~SvxShapeText() throw ()
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


void SAL_CALL SvxShapeText::acquire() throw()
{
    SvxShape::acquire();
}


void SAL_CALL SvxShapeText::release() throw()
{
    SvxShape::release();
}

// XServiceInfo

OUString SAL_CALL SvxShapeText::getImplementationName()
{
    return OUString("SvxShapeText");
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
bool SvxShapeText::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue )
{
    // HACK-fix #99090#
    // since SdrTextObj::SetVerticalWriting exchanges
    // SDRATTR_TEXT_AUTOGROWWIDTH and SDRATTR_TEXT_AUTOGROWHEIGHT,
    // we have to set the textdirection here

    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( GetSdrObject() );
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

bool SvxShapeText::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue )
{
    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( GetSdrObject() );
        if( pTextObj && pTextObj->IsVerticalWriting() )
            rValue <<= css::text::WritingMode_TB_RL;
        else
            rValue <<= css::text::WritingMode_LR_TB;
        return true;
    }

    return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
}

bool SvxShapeText::getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState )
{
    return SvxShape::getPropertyStateImpl( pProperty, rState );
}

bool SvxShapeText::setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty )
{
    return SvxShape::setPropertyToDefaultImpl( pProperty );
}

SvxShapeRect::SvxShapeRect(SdrObject* pObj)
: SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_SHAPE), getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
{
}

SvxShapeRect::~SvxShapeRect() throw()
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

void SAL_CALL SvxShapeRect::acquire() throw()
{
    OWeakAggObject::acquire();
}

void SAL_CALL SvxShapeRect::release() throw()
{
    OWeakAggObject::release();
}

// XServiceInfo

uno::Sequence< OUString > SvxShapeRect::getSupportedServiceNames()
{
    return SvxShape::getSupportedServiceNames();
}

/** returns a StarOffice API wrapper for the given SdrObject */
uno::Reference< drawing::XShape > GetXShapeForSdrObject( SdrObject* pObj ) throw ()
{
    uno::Reference< drawing::XShape > xShape( pObj->getUnoShape(), uno::UNO_QUERY );
    return xShape;
}

/** returns the SdrObject from the given StarOffice API wrapper */
SdrObject* GetSdrObjectFromXShape( const uno::Reference< drawing::XShape >& xShape ) throw()
{
    SvxShape* pShape = SvxShape::getImplementation( xShape );
    return pShape ? pShape->GetSdrObject() : nullptr;
}


SdrObject* SdrObject::getSdrObjectFromXShape( const css::uno::Reference< css::uno::XInterface >& xInt )
{
    SvxShape* pSvxShape = SvxShape::getImplementation( xInt );
    return pSvxShape ? pSvxShape->GetSdrObject() : nullptr;
}

uno::Any SvxItemPropertySet_getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet )
{
    if(!pMap || !pMap->nWID)
        return uno::Any();

    // Check is for items that store either metric values if they are positive or percentage if they are negative.
    bool bDontConvertNegativeValues = ( pMap->nWID == XATTR_FILLBMP_SIZEX || pMap->nWID == XATTR_FILLBMP_SIZEY );
    return SvxItemPropertySet::getPropertyValue( pMap, rSet, (pMap->nWID != SDRATTR_XMLATTRIBUTES), bDontConvertNegativeValues );
}

void SvxItemPropertySet_setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const uno::Any& rVal, SfxItemSet& rSet )
{
    if(!pMap || !pMap->nWID)
        return;

    bool bDontConvertNegativeValues = ( pMap->nWID == XATTR_FILLBMP_SIZEX || pMap->nWID == XATTR_FILLBMP_SIZEY );
    SvxItemPropertySet::setPropertyValue( pMap, rVal, rSet, bDontConvertNegativeValues );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
