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
#include <vcl/svapp.hxx>
#include <svl/itemprop.hxx>
#include <vcl/fltcall.hxx>
#include <osl/mutex.hxx>
#include <editeng/unotext.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include "svx/shapepropertynotifier.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/serviceinfohelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include "svx/svdopage.hxx"
#include "svx/xflbstit.hxx"
#include "svx/xflbmtit.hxx"
#include "svx/xlnstit.hxx"
#include "svx/xlnedit.hxx"
#include "svx/svdogrp.hxx"
#include "svx/scene3d.hxx"
#include "svx/svdmodel.hxx"
#include "svx/globl3d.hxx"
#include "svx/fmglob.hxx"
#include "svx/unopage.hxx"
#include "svx/view3d.hxx"
#include "svx/unoshape.hxx"
#include "svx/svxids.hrc"
#include "svx/unoshtxt.hxx"
#include "svx/svdpage.hxx"
#include "svx/unoshprp.hxx"
#include "svx/sxciaitm.hxx"
#include "svx/svdograf.hxx"
#include "svx/unoapi.hxx"
#include "svx/svdomeas.hxx"
#include "svx/svdpagv.hxx"
#include "svx/svdpool.hxx"
#include <tools/gen.hxx>
#include "svx/dialmgr.hxx"
#include "svx/dialogs.hrc"
#include "svx/svdocapt.hxx"
#include <svx/obj3d.hxx>
#include <tools/diagnose_ex.h>
#include "svx/xflftrit.hxx"
#include "svx/xtable.hxx"
#include "svx/xbtmpit.hxx"
#include "svx/xflgrit.hxx"
#include "svx/xflhtit.hxx"
#include "svx/xlndsit.hxx"
#include "svdglob.hxx"
#include "svx/svdstr.hrc"
#include "svx/unomaster.hxx"
#include <editeng/outlobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/tools/unotools.hxx>
#include "gluepts.hxx"
#include "shapeimpl.hxx"
#include <sal/log.hxx>

#include "svx/lathe3d.hxx"
#include "svx/extrud3d.hxx"

#include <vcl/wmf.hxx>

#include <memory>
#include <vector>

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
    SfxItemSet*     mpItemSet;
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
        ,mpItemSet( nullptr )
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

SvxShape::SvxShape( SdrObject* pObject ) throw( uno::RuntimeException )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
,   maPropMapEntries(getSvxMapProvider().GetMap(SVXMAP_SHAPE))
,   mpObj(pObject)
,   mpModel(nullptr)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::SvxShape( SdrObject* pObject, const SfxItemPropertyMapEntry* pEntries, const SvxItemPropertySet* pPropertySet ) throw( uno::RuntimeException )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(pPropertySet)
,   maPropMapEntries(pEntries)
,   mpObj(pObject)
,   mpModel(nullptr)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::SvxShape() throw( uno::RuntimeException )
:   maSize(100,100)
,   mpImpl( new SvxShapeImpl( *this, maMutex ) )
,   mbIsMultiPropertyCall(false)
,   mpPropSet(getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
,   maPropMapEntries(getSvxMapProvider().GetMap(SVXMAP_SHAPE))
,   mpObj(nullptr)
,   mpModel(nullptr)
,   mnLockCount(0)
{
    impl_construct();
}


SvxShape::~SvxShape() throw()
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount == 0, "Locked shape was disposed!" );

    if ( mpModel )
        EndListening( *mpModel );

    if ( mpImpl->mpMaster )
        mpImpl->mpMaster->dispose();

    if ( mpObj.is() )
        mpObj->setUnoShape(nullptr);

    if( HasSdrObjectOwnership() && mpObj.is() )
    {
        mpImpl->mbHasSdrObjectOwnership = false;
        SdrObject* pObject = mpObj.get();
        SdrObject::Free( pObject );
    }

    delete mpImpl;
    mpImpl = nullptr;

    EndListeningAll(); // call explicitly within SolarMutexGuard
}


void SvxShape::TakeSdrObjectOwnership()
{
    mpImpl->mbHasSdrObjectOwnership = true;
}


bool SvxShape::HasSdrObjectOwnership() const
{
    if ( !mpImpl->mbHasSdrObjectOwnership )
        return false;

    OSL_ENSURE( mpObj.is(), "SvxShape::HasSdrObjectOwnership: have the ownership of an object which I don't know!" );
    return mpObj.is();
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


uno::Any SAL_CALL SvxShape::queryAggregation( const uno::Type& rType ) throw (uno::RuntimeException, std::exception)
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


sal_Int64 SAL_CALL SvxShape::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) \
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
    mpImpl->maPropertyNotifier.registerProvider( svx::eShapePosition,
        svx::PPropertyValueProvider( new ShapePositionProvider( *mpImpl ) ) );
    mpImpl->maPropertyNotifier.registerProvider( svx::eShapeSize,
        svx::PPropertyValueProvider( new ShapeSizeProvider( *mpImpl ) ) );

    if ( mpObj.is() )
        impl_initFromSdrObject();
}


void SvxShape::impl_initFromSdrObject()
{
    DBG_TESTSOLARMUTEX();
    OSL_PRECOND( mpObj.is(), "SvxShape::impl_initFromSdrObject: not to be called without SdrObject!" );
    if ( !mpObj.is() )
        return;

    osl_atomic_increment( &m_refCount );
    {
        mpObj->setUnoShape(*this);
    }
    osl_atomic_decrement( &m_refCount );

    mpModel = mpObj->GetModel();

    // #i40944#
    // Do not simply return when no model but do the type corrections
    // following below.
    if(mpModel)
    {
        StartListening( *mpModel );
    }

    const sal_uInt32 nInventor = mpObj->GetObjInventor();

    // is it one of ours (svx) ?
    if( nInventor == SdrInventor || nInventor == E3dInventor || nInventor == FmFormInventor )
    {
        if(nInventor == FmFormInventor)
        {
            mpImpl->mnObjId = OBJ_UNO;
        }
        else
        {
            mpImpl->mnObjId = mpObj->GetObjIdentifier();
            if( nInventor == E3dInventor )
                mpImpl->mnObjId |= E3D_INVENTOR_FLAG;
        }

        switch(mpImpl->mnObjId)
        {
        case OBJ_CCUT:          // segment of circle
        case OBJ_CARC:          // arc of circle
        case OBJ_SECT:          // sector
            mpImpl->mnObjId = OBJ_CIRC;
            break;

        case E3D_SCENE_ID | E3D_INVENTOR_FLAG:
            mpImpl->mnObjId = E3D_POLYSCENE_ID | E3D_INVENTOR_FLAG;
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
    if ( pCreatedObj != pNewObj )
    {
        DBG_ASSERT( pNewObj->GetModel(), "no model for SdrObject?" );
        // Correct condition (#i52126#)
        mpImpl->mpCreatedObj = pNewObj;

        if( mpObj.is() && mpObj->GetModel() )
        {
            EndListening( *mpObj->GetModel() );
        }

        mpObj.reset( pNewObj );

        OSL_ENSURE( !mbIsMultiPropertyCall, "SvxShape::Create: hmm?" );
            // this was previously set in impl_initFromSdrObject, but I think it was superfluous
            // (it definitely was in the other context where it was called, but I strongly suppose
            // it was also superfluous when called from here)
        impl_initFromSdrObject();

        ObtainSettingsFromPropertySet( *mpPropSet );

        // save user call
        SdrObjUserCall* pUser = mpObj->GetUserCall();
        mpObj->SetUserCall(nullptr);

        setPosition( maPosition );
        setSize( maSize );

        // restore user call after we set the initial size
        mpObj->SetUserCall( pUser );

        // if this shape was already named, use this name
        if( !maShapeName.isEmpty() )
        {
            mpObj->SetName( maShapeName );
            maShapeName.clear();
        }
    }
}


void SvxShape::ChangeModel( SdrModel* pNewModel )
{
    DBG_TESTSOLARMUTEX();
    if( mpObj.is() && mpObj->GetModel() )
    {
        if( mpObj->GetModel() != pNewModel )
        {
            EndListening( *mpObj->GetModel() );
        }
    }

    // Always listen to new model (#i52126#)
    if( pNewModel )
    {
        StartListening( *pNewModel );
    }

    // HACK #i53696# ChangeModel should be virtual, but it isn't. can't change that for 2.0.1
    SvxShapeText* pShapeText = dynamic_cast< SvxShapeText* >( this );
    if( pShapeText )
    {
        SvxTextEditSource* pTextEditSource = dynamic_cast< SvxTextEditSource* >( pShapeText->GetEditSource() );
        if( pTextEditSource )
            pTextEditSource->ChangeModel( pNewModel );
    }

    mpModel = pNewModel;

    if( mpImpl->mpMaster )
        mpImpl->mpMaster->modelChanged( pNewModel );
}


void SvxShape::ForceMetricToItemPoolMetric(Pair& rPoint) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(mpModel)
    {
        SfxMapUnit eMapUnit = mpModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
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

// Reintroduction of fix for issue i59051 (#i108851#)
void SvxShape::ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const throw()
{
    DBG_TESTSOLARMUTEX();
    if(mpModel)
    {
        SfxMapUnit eMapUnit = mpModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
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


void SvxShape::ForceMetricTo100th_mm(Pair& rPoint) const throw()
{
    DBG_TESTSOLARMUTEX();
    SfxMapUnit eMapUnit = SFX_MAPUNIT_100TH_MM;
    if(mpModel)
    {
        eMapUnit = mpModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
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

// Reintroduction of fix for issue i59051 (#i108851#)
void SvxShape::ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const throw()
{
    DBG_TESTSOLARMUTEX();
    SfxMapUnit eMapUnit = SFX_MAPUNIT_100TH_MM;
    if(mpModel)
    {
        eMapUnit = mpModel->GetItemPool().GetMetric(0);
        if(eMapUnit != SFX_MAPUNIT_100TH_MM)
        {
            switch(eMapUnit)
            {
                case SFX_MAPUNIT_TWIP :
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
                            if(SfxItemPool::IsWhich(pEntry->nWID))
                                rSet.Put(rSet.GetPool()->GetDefaultItem(pEntry->nWID));
                            // set
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
    if(mpObj.is() && rPropSet.AreThereOwnUsrAnys() && mpModel)
    {
        SfxItemSet aSet( mpModel->GetItemPool(), SDRATTR_START, SDRATTR_END);
        Reference< beans::XPropertySet > xShape( static_cast<OWeakObject*>(this), UNO_QUERY );
        SvxItemPropertySet_ObtainSettingsFromPropertySet(rPropSet, aSet, xShape, &mpPropSet->getPropertyMap() );

        mpObj->SetMergedItemSetAndBroadcast(aSet);

        mpObj->ApplyNotPersistAttr( aSet );
    }
}

uno::Any SvxShape::GetBitmap( bool bMetaFile /* = false */ ) const
    throw (uno::RuntimeException, std::exception)
{
    DBG_TESTSOLARMUTEX();
    uno::Any aAny;

    if( !mpObj.is() || mpModel == nullptr || !mpObj->IsInserted() || nullptr == mpObj->GetPage() )
        return aAny;

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    pVDev->SetMapMode(MapMode(MAP_100TH_MM));

    SdrModel* pModel = mpObj->GetModel();
    SdrPage* pPage = mpObj->GetPage();

    std::unique_ptr<E3dView> pView(new E3dView( pModel, pVDev.get() ));
    pView->hideMarkHandles();
    SdrPageView* pPageView = pView->ShowSdrPage(pPage);

    SdrObject *pTempObj = mpObj.get();
    pView->MarkObj(pTempObj,pPageView);

    Rectangle aRect(pTempObj->GetCurrentBoundRect());
    aRect.Justify();
    Size aSize(aRect.GetSize());

    GDIMetaFile aMtf( pView->GetMarkedObjMetaFile() );
    if( bMetaFile )
    {
        SvMemoryStream aDestStrm( 65535, 65535 );
        ConvertGDIMetaFileToWMF( aMtf, aDestStrm, nullptr, false );
        const uno::Sequence<sal_Int8> aSeq(
            static_cast< const sal_Int8* >(aDestStrm.GetData()),
            aDestStrm.GetEndOfData());
        aAny.setValue( &aSeq, cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );
    }
    else
    {
        Graphic aGraph(aMtf);
        aGraph.SetPrefSize(aSize);
        aGraph.SetPrefMapMode(MAP_100TH_MM);

        Reference< awt::XBitmap > xBmp( aGraph.GetXGraphic(), UNO_QUERY );
        aAny <<= xBmp;
    }

    pView->UnmarkAll();

    return aAny;
}

uno::Sequence< uno::Type > SAL_CALL SvxShape::getTypes()
    throw (uno::RuntimeException, std::exception)
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


uno::Sequence< uno::Type > SAL_CALL SvxShape::_getTypes()
    throw(uno::RuntimeException)
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
    case E3D_POLYSCENE_ID|E3D_INVENTOR_FLAG:
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
    throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SvxShape::Notify( SfxBroadcaster&, const SfxHint& rHint ) throw()
{
    DBG_TESTSOLARMUTEX();
    if( !mpObj.is() )
        return;

    // #i55919# HINT_OBJCHG is only interesting if it's for this object

    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (!pSdrHint || ( /* (pSdrHint->GetKind() != HINT_OBJREMOVED)  && */
        (pSdrHint->GetKind() != HINT_MODELCLEARED) &&
        // #110094#-9 (pSdrHint->GetKind() != HINT_OBJLISTCLEAR) &&
        ((pSdrHint->GetKind() != HINT_OBJCHG || pSdrHint->GetObject() != mpObj.get() ))))
        return;

    uno::Reference< uno::XInterface > xSelf( mpObj->getWeakUnoShape() );
    if( !xSelf.is() )
    {
        mpObj.reset( nullptr );
        return;
    }

    bool bClearMe = false;

    switch( pSdrHint->GetKind() )
    {
        case HINT_OBJCHG:
        {
            updateShapeKind();
            break;
        }
        case HINT_MODELCLEARED:
        {
            bClearMe = true;
            mpModel = nullptr;
            break;
        }
        default:
            break;
    };

    if( bClearMe )
    {
        if( !HasSdrObjectOwnership() ) {
            if( mpObj.is() )
                mpObj->setUnoShape( nullptr );
            mpObj.reset( nullptr );
        }
        if ( !mpImpl->mbDisposing )
            dispose();
    }
}

// XShape


// The "*LogicRectHack" functions also existed in sch, and those
// duplicate symbols cause Bad Things To Happen (TM)  #i9462#.
// Prefixing with 'svx' and marking static to make sure name collisions
// do not occur.

static bool svx_needLogicRectHack( SdrObject* pObj )
{
    if( pObj->GetObjInventor() == SdrInventor)
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


static Rectangle svx_getLogicRectHack( SdrObject* pObj )
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


static void svx_setLogicRectHack( SdrObject* pObj, const Rectangle& rRect )
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


awt::Point SAL_CALL SvxShape::getPosition() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpModel)
    {
        Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
        Point aPt( aRect.Left(), aRect.Top() );

        // Position is relativ to anchor, so recalc to absolut position
        if( mpModel->IsWriter() )
            aPt -= mpObj->GetAnchorPos();

        ForceMetricTo100th_mm(aPt);
        return css::awt::Point( aPt.X(), aPt.Y() );
    }
    else
    {
        return maPosition;
    }
}


void SAL_CALL SvxShape::setPosition( const awt::Point& Position ) throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpModel )
    {
        // do NOT move 3D objects, this would change the homogen
        // transformation matrix
        if(dynamic_cast<const E3dCompoundObject* >(mpObj.get()) == nullptr)
        {
            Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
            Point aLocalPos( Position.X, Position.Y );
            ForceMetricToItemPoolMetric(aLocalPos);

            // Position is absolut, so recalc to position relativ to anchor
            if( mpModel->IsWriter() )
                aLocalPos += mpObj->GetAnchorPos();

            long nDX = aLocalPos.X() - aRect.Left();
            long nDY = aLocalPos.Y() - aRect.Top();

            mpObj->Move( Size( nDX, nDY ) );
            mpModel->SetChanged();
        }
    }

    maPosition = Position;
}


awt::Size SAL_CALL SvxShape::getSize() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpModel)
    {
        Rectangle aRect( svx_getLogicRectHack(mpObj.get()) );
        Size aObjSize( aRect.getWidth(), aRect.getHeight() );
        ForceMetricTo100th_mm(aObjSize);
        return css::awt::Size( aObjSize.getWidth(), aObjSize.getHeight() );
    }
    else
        return maSize;
}


void SAL_CALL SvxShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpModel)
    {
        // #i123539# optimization for 3D chart object generation: do not use UNO
        // API commands to get the range, this is too expensive since for 3D
        // scenes it may recalculate the whole scene since in AOO this depends
        // on the contained geometry (layouted to show all content)
        const bool b3DConstruction(dynamic_cast< E3dObject* >(mpObj.get()) && mpModel->isLocked());
        Rectangle aRect(
            b3DConstruction ?
                Rectangle(maPosition.X, maPosition.Y, maSize.Width, maSize.Height) :
                svx_getLogicRectHack(mpObj.get()) );
        Size aLocalSize( rSize.Width, rSize.Height );
        ForceMetricToItemPoolMetric(aLocalSize);

        if(mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_MEASURE )
        {
            Fraction aWdt(aLocalSize.Width(),aRect.Right()-aRect.Left());
            Fraction aHgt(aLocalSize.Height(),aRect.Bottom()-aRect.Top());
            Point aPt = mpObj->GetSnapRect().TopLeft();
            mpObj->Resize(aPt,aWdt,aHgt);
        }
        else
        {
            //aRect.SetSize(aLocalSize); // this call subtract 1 // http://www.openoffice.org/issues/show_bug.cgi?id=83193
            if ( !aLocalSize.Width() )
            {
                aRect.Right() = RECT_EMPTY;
            }
            else
                aRect.setWidth(aLocalSize.Width());
            if ( !aLocalSize.Height() )
            {
                aRect.Bottom() = RECT_EMPTY;
            }
            else
                aRect.setHeight(aLocalSize.Height());

            svx_setLogicRectHack( mpObj.get(), aRect );
        }

        mpModel->SetChanged();
    }
    maSize = rSize;
}


// XNamed
OUString SAL_CALL SvxShape::getName(  ) throw(css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    if( mpObj.is() )
    {
        return mpObj->GetName();
    }
    else
    {
        return maShapeName;
    }
}


void SAL_CALL SvxShape::setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    if( mpObj.is() )
    {
        mpObj->SetName( aName );
    }
    else
    {
        maShapeName = aName;
    }
}

// XShapeDescriptor


OUString SAL_CALL SvxShape::getShapeType() throw(uno::RuntimeException, std::exception)
{
    if( !maShapeType.getLength() )
        return UHashMap::getNameFromId( mpImpl->mnObjId );
    else
        return maShapeType;
}

// XComponent


void SAL_CALL SvxShape::dispose() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpImpl->mbDisposing )
        return; // caught a recursion

    mpImpl->mbDisposing = true;

    lang::EventObject aEvt;
    aEvt.Source = *static_cast<OWeakAggObject*>(this);
    mpImpl->maDisposeListeners.disposeAndClear(aEvt);
    mpImpl->maPropertyNotifier.disposing();

    if ( mpObj.is() )
    {
        bool bFreeSdrObject = false;

        if ( mpObj->IsInserted() && mpObj->GetPage() )
        {
            OSL_ENSURE( HasSdrObjectOwnership(), "SvxShape::dispose: is the below code correct?" );
                // normally, we are allowed to free the SdrObject only if we have its ownership.
                // Why isn't this checked here?

            SdrPage* pPage = mpObj->GetPage();
            // SdrObject aus der Page loeschen
            const size_t nCount = pPage->GetObjCount();
            for ( size_t nNum = 0; nNum < nCount; ++nNum )
            {
                if ( pPage->GetObj( nNum ) == mpObj.get() )
                {
                    OSL_VERIFY( pPage->RemoveObject( nNum ) == mpObj.get() );
                    bFreeSdrObject = true;
                    break;
                }
            }
        }

        mpObj->setUnoShape(nullptr);

        if ( bFreeSdrObject )
        {
            // in case we have the ownership of the SdrObject, a Free
            // would do nothing. So ensure the ownership is reset.
            mpImpl->mbHasSdrObjectOwnership = false;
            SdrObject* pObject = mpObj.get();
            SdrObject::Free( pObject );
        }
    }

    if( mpModel )
    {
        EndListening( *mpModel );
        mpModel = nullptr;
    }
}


void SAL_CALL SvxShape::addEventListener( const Reference< lang::XEventListener >& xListener )
    throw(uno::RuntimeException, std::exception)
{
    mpImpl->maDisposeListeners.addInterface(xListener);
}


void SAL_CALL SvxShape::removeEventListener( const Reference< lang::XEventListener >& aListener ) throw(uno::RuntimeException, std::exception)
{
   mpImpl->maDisposeListeners.removeInterface(aListener);
}

// XPropertySet


Reference< beans::XPropertySetInfo > SAL_CALL
    SvxShape::getPropertySetInfo() throw(uno::RuntimeException, std::exception)
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

Reference< beans::XPropertySetInfo > SAL_CALL
    SvxShape::_getPropertySetInfo() throw(uno::RuntimeException)
{
    return mpPropSet->getPropertySetInfo();
}


void SAL_CALL SvxShape::addPropertyChangeListener( const OUString& _propertyName, const Reference< beans::XPropertyChangeListener >& _listener  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( maMutex );
    mpImpl->maPropertyNotifier.addPropertyChangeListener( _propertyName, _listener );
}


void SAL_CALL SvxShape::removePropertyChangeListener( const OUString& _propertyName, const Reference< beans::XPropertyChangeListener >& _listener  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( maMutex );
    mpImpl->maPropertyNotifier.removePropertyChangeListener( _propertyName, _listener );
}


void SAL_CALL SvxShape::addVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "SvxShape::addVetoableChangeListener: don't have any vetoable properties, so why ...?" );
}


void SAL_CALL SvxShape::removeVetoableChangeListener( const OUString& , const Reference< beans::XVetoableChangeListener >&  ) throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "SvxShape::removeVetoableChangeListener: don't have any vetoable properties, so why ...?" );
}


bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName )
{
    SfxItemSet aSet( mpModel->GetItemPool(),    (sal_uInt16)nWID, (sal_uInt16)nWID );

    if( SetFillAttribute( nWID, rName, aSet, mpModel ) )
    {
        //mpObj->SetItemSetAndBroadcast(aSet);
        mpObj->SetMergedItemSetAndBroadcast(aSet);

        return true;
    }
    else
    {
        return false;
    }
}


bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet, SdrModel* pModel )
{
    // check if an item with the given name and which id is inside the models
    // pool or the stylesheet pool, if found it's put in the itemset
    if( !SetFillAttribute( nWID, rName, rSet ) )
    {
        // we did not find such item in one of the pools, so we check
        // the property lists that are loaded for the model for items
        // that support such.
        OUString aStrName = SvxUnogetInternalNameForItem((sal_Int16)nWID, rName);

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

            XBitmapEntry* pEntry = pBitmapList->GetBitmap( nPos );
            XFillBitmapItem aBmpItem;
            aBmpItem.SetWhich( XATTR_FILLBITMAP );
            aBmpItem.SetName( rName );
            aBmpItem.SetGraphicObject(pEntry->GetGraphicObject());
            rSet.Put( aBmpItem );
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

            XGradientEntry* pEntry = pGradientList->GetGradient( nPos );
            XFillGradientItem aGrdItem;
            aGrdItem.SetWhich( XATTR_FILLGRADIENT );
            aGrdItem.SetName( rName );
            aGrdItem.SetGradientValue( pEntry->GetGradient() );
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

            XHatchEntry* pEntry = pHatchList->GetHatch( nPos );
            XFillHatchItem aHatchItem;
            aHatchItem.SetWhich( XATTR_FILLHATCH );
            aHatchItem.SetName( rName );
            aHatchItem.SetHatchValue( pEntry->GetHatch() );
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

            XLineEndEntry* pEntry = pLineEndList->GetLineEnd( nPos );
            if( XATTR_LINEEND == nWID )
            {
                XLineEndItem aLEItem;
                aLEItem.SetWhich( XATTR_LINEEND );
                aLEItem.SetName( rName );
                aLEItem.SetLineEndValue( pEntry->GetLineEnd() );
                rSet.Put( aLEItem );
            }
            else
            {
                XLineStartItem aLSItem;
                aLSItem.SetWhich( XATTR_LINESTART );
                aLSItem.SetName( rName );
                aLSItem.SetLineStartValue( pEntry->GetLineEnd() );
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

            XDashEntry* pEntry = pDashList->GetDash( nPos );
            XLineDashItem aDashItem;
            aDashItem.SetWhich( XATTR_LINEDASH );
            aDashItem.SetName( rName );
            aDashItem.SetDashValue( pEntry->GetDash() );
            rSet.Put( aDashItem );
            break;
        }
        default:
            return false;
        }
    }

    return true;
}


bool SAL_CALL SvxShape::SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet )
{
    OUString aName = SvxUnogetInternalNameForItem((sal_Int16)nWID, rName);

    if (aName.isEmpty())
    {
        switch( nWID )
        {
        case XATTR_LINEEND:
        case XATTR_LINESTART:
            {
                const basegfx::B2DPolyPolygon aEmptyPoly;
                if( nWID == XATTR_LINEEND )
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

    const sal_uInt32 nCount = pPool->GetItemCount2((sal_uInt16)nWID);

    for( sal_uInt32 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
    {
        const NameOrIndex* pItem = static_cast<const NameOrIndex*>(pPool->GetItem2((sal_uInt16)nWID, nSurrogate));
        if( pItem && ( pItem->GetName() == aName ) )
        {
            rSet.Put( *pItem );
            return true;
        }
    }

    return false;
}


void SAL_CALL SvxShape::setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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

void SAL_CALL SvxShape::_setPropertyValue( const OUString& rPropertyName, const uno::Any& rVal )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(rPropertyName);

    if (!mpObj.is() || !mpModel)
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
        throw beans::UnknownPropertyException();

    if ((pMap->nFlags & beans::PropertyAttribute::READONLY) != 0)
        throw beans::PropertyVetoException(
            "Readonly property can't be set: " + rPropertyName,
            uno::Reference<drawing::XShape>(this));

    mpModel->SetChanged();

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
            pSet = mpImpl->mpItemSet = mpObj->GetMergedItemSet().Clone();
        }
        else
        {
            pSet = mpImpl->mpItemSet;
        }
    }
    else
    {
        pSet = new SfxItemSet( mpModel->GetItemPool(),  pMap->nWID, pMap->nWID);
    }

    if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        pSet->Put(mpObj->GetMergedItem(pMap->nWID));

    if( !SvxUnoTextRangeBase::SetPropertyValueHelper( *pSet, pMap, rVal, *pSet ))
    {
        if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        {
            if(bIsNotPersist)
            {
                // not-persistent attribute, get those extra
                mpObj->TakeNotPersistAttr(*pSet);
            }
        }

        if( pSet->GetItemState( pMap->nWID ) != SfxItemState::SET )
        {
            // get default from ItemPool
            if(SfxItemPool::IsWhich(pMap->nWID))
                pSet->Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));
        }

        if( pSet->GetItemState( pMap->nWID ) == SfxItemState::SET )
        {
            SvxItemPropertySet_setPropertyValue( pMap, rVal, *pSet );
        }
    }

    if(bIsNotPersist)
    {
        // set not-persistent attribute extra
        mpObj->ApplyNotPersistAttr( *pSet );
        delete pSet;
    }
    else
    {
        // if we have a XMultiProperty call then the item set
        // will be set in setPropertyValues later
        if( !mbIsMultiPropertyCall )
        {
            mpObj->SetMergedItemSetAndBroadcast( *pSet );

            delete pSet;
        }
    }
}


uno::Any SAL_CALL SvxShape::getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if ( mpImpl->mpMaster )
        return mpImpl->mpMaster->getPropertyValue( PropertyName );
    else
        return _getPropertyValue( PropertyName );
}


uno::Any SvxShape::_getPropertyValue( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    uno::Any aAny;
    if( mpObj.is() && mpModel )
    {
        if(pMap == nullptr )
            throw beans::UnknownPropertyException();

        if( !getPropertyValueImpl( PropertyName, pMap, aAny ) )
        {
            DBG_ASSERT( pMap->nWID == SDRATTR_TEXTDIRECTION || (pMap->nWID < SDRATTR_NOTPERSIST_FIRST || pMap->nWID > SDRATTR_NOTPERSIST_LAST), "Not persist item not handled!" );
            DBG_ASSERT( pMap->nWID < OWN_ATTR_VALUE_START || pMap->nWID > OWN_ATTR_VALUE_END, "Not item property not handled!" );

            SfxItemSet aSet( mpModel->GetItemPool(),    pMap->nWID, pMap->nWID);
            aSet.Put(mpObj->GetMergedItem(pMap->nWID));

            if(SvxUnoTextRangeBase::GetPropertyValueHelper(  aSet, pMap, aAny ))
                return aAny;

            if(!aSet.Count())
            {
                if(pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST)
                {
                    // not-persistent attribute, get those extra
                    mpObj->TakeNotPersistAttr(aSet);
                }
            }

            if(!aSet.Count())
            {
                // get default from ItemPool
                if(SfxItemPool::IsWhich(pMap->nWID))
                    aSet.Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));
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
void SAL_CALL SvxShape::setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aSolarGuard;

    const sal_Int32 nCount = aPropertyNames.getLength();
    const OUString* pNames = aPropertyNames.getConstArray();

    const uno::Any* pValues = aValues.getConstArray();

    // make sure mbIsMultiPropertyCall and mpImpl->mpItemSet are
    // reseted even when an exception is thrown
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
            catch( beans::UnknownPropertyException& e )
            {
                (void)e;
            }
            catch( uno::Exception& ex )
            {
                (void)ex;
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
            catch( beans::UnknownPropertyException& e )
            {
                (void)e;
            }
            catch( uno::Exception& ex )
            {
                (void)ex;
            }
        }
    }

    if( mpImpl->mpItemSet && mpObj.is() )
        mpObj->SetMergedItemSetAndBroadcast( *mpImpl->mpItemSet );
}


void SvxShape::endSetPropertyValues()
{
    mbIsMultiPropertyCall = false;
    if( mpImpl->mpItemSet )
    {
        delete mpImpl->mpItemSet;
        mpImpl->mpItemSet = nullptr;
    }
}


css::uno::Sequence< css::uno::Any > SAL_CALL SvxShape::getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::uno::RuntimeException, std::exception)
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

void SAL_CALL SvxShape::addPropertiesChangeListener( const css::uno::Sequence< OUString >& , const css::uno::Reference< css::beans::XPropertiesChangeListener >&  ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxShape::removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >&  ) throw (css::uno::RuntimeException, std::exception)
{
}

void SAL_CALL SvxShape::firePropertiesChangeEvent( const css::uno::Sequence< OUString >& , const css::uno::Reference< css::beans::XPropertiesChangeListener >&  ) throw (css::uno::RuntimeException, std::exception)
{
}


uno::Any SvxShape::GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap ) const
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
        if( mpObj->GetObjInventor() == SdrInventor)
        {
            drawing::CircleKind eKind;
            switch(mpObj->GetObjIdentifier())
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
                aAny <<= (sal_Int16)nValue;
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
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
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

beans::PropertyState SAL_CALL SvxShape::_getPropertyState( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !mpObj.is() || pMap == nullptr )
        throw beans::UnknownPropertyException();

    beans::PropertyState eState;
    if( !getPropertyStateImpl( pMap, eState ) )
    {
        const SfxItemSet& rSet = mpObj->GetMergedItemSet();

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
        if( ( beans::PropertyState_DIRECT_VALUE == eState ) )
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
                    const NameOrIndex* pItem = static_cast<const NameOrIndex*>(rSet.GetItem((sal_uInt16)pMap->nWID));
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
                    const NameOrIndex* pItem = static_cast<const NameOrIndex*>(rSet.GetItem((sal_uInt16)pMap->nWID));
                    if ( pItem == nullptr )
                        eState = beans::PropertyState_DEFAULT_VALUE;
                }
                break;
            }
        }
    }
    return eState;
}

bool SvxShape::setPropertyValueImpl( const OUString&, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CAPTION_POINT:
    {
        awt::Point aPnt;
        if( rValue >>= aPnt )
        {
            Point aVclPoint( aPnt.X, aPnt.Y );

            // #90763# position is relative to top left, make it absolute
            basegfx::B2DPolyPolygon aNewPolyPolygon;
            basegfx::B2DHomMatrix aNewHomogenMatrix;
            mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aVclPoint.X() += basegfx::fround(aNewHomogenMatrix.get(0, 2));
            aVclPoint.Y() += basegfx::fround(aNewHomogenMatrix.get(1, 2));

            // #88657# metric of pool maybe twips (writer)
            ForceMetricToItemPoolMetric(aVclPoint);

            // #88491# position relative to anchor
            if( mpModel->IsWriter() )
            {
                aVclPoint += mpObj->GetAnchorPos();
            }

            static_cast<SdrCaptionObj*>(mpObj.get())->SetTailPos(aVclPoint);

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

            mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

            aNewHomogenMatrix.set(0, 0, aMatrix.Line1.Column1);
            aNewHomogenMatrix.set(0, 1, aMatrix.Line1.Column2);
            aNewHomogenMatrix.set(0, 2, aMatrix.Line1.Column3);
            aNewHomogenMatrix.set(1, 0, aMatrix.Line2.Column1);
            aNewHomogenMatrix.set(1, 1, aMatrix.Line2.Column2);
            aNewHomogenMatrix.set(1, 2, aMatrix.Line2.Column3);
            aNewHomogenMatrix.set(2, 0, aMatrix.Line3.Column1);
            aNewHomogenMatrix.set(2, 1, aMatrix.Line3.Column2);
            aNewHomogenMatrix.set(2, 2, aMatrix.Line3.Column3);

            mpObj->TRSetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
            return true;
        }
        break;
    }

    case OWN_ATTR_ZORDER:
    {
        sal_Int32 nNewOrdNum = 0;
        if(rValue >>= nNewOrdNum)
        {
            SdrObjList* pObjList = mpObj->GetObjList();
            if( pObjList )
            {
                SdrObject* pCheck =
                            pObjList->SetObjectOrdNum( mpObj->GetOrdNum(), static_cast<size_t>(nNewOrdNum) );
                DBG_ASSERT( pCheck == mpObj.get(), "GetOrdNum() failed!" );
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
            Rectangle aRect;
            aRect.SetPos(aTopLeft);
            aRect.SetSize(aObjSize);
            mpObj->SetSnapRect(aRect);
            return true;
        }
        break;
    }
    case OWN_ATTR_MIRRORED:
    {
        bool bMirror;
        if(rValue >>= bMirror )
        {
            SdrGrafObj* pObj = dynamic_cast< SdrGrafObj* >( mpObj.get() );
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
        SdrEdgeObj* pEdgeObj = dynamic_cast< SdrEdgeObj* >(mpObj.get());
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
                        if( mpModel->IsWriter() )
                            aPoint += mpObj->GetAnchorPos();

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
                    if( rValue.getValueType() == cppu::UnoType<drawing::PointSequenceSequence>::get())
                    {
                        // get polygpon data from PointSequenceSequence
                        aNewPolyPolygon = basegfx::tools::UnoPointSequenceSequenceToB2DPolyPolygon(
                            *static_cast<const drawing::PointSequenceSequence*>(rValue.getValue()));
                    }
                    else if( rValue.getValueType() == cppu::UnoType<drawing::PolyPolygonBezierCoords>::get())
                    {
                        // get polygpon data from PolyPolygonBezierCoords
                        aNewPolyPolygon = basegfx::tools::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
                            *static_cast<const drawing::PolyPolygonBezierCoords*>(rValue.getValue()));
                    }

                    if(aNewPolyPolygon.count())
                    {
                        // Reintroduction of fix for issue i59051 (#i108851#)
                        ForceMetricToItemPoolMetric( aNewPolyPolygon );
                        if( mpModel->IsWriter() )
                        {
                            Point aPoint( mpObj->GetAnchorPos() );
                            aNewPolyPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(aPoint.X(), aPoint.Y()));
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
        SdrMeasureObj* pMeasureObj = dynamic_cast< SdrMeasureObj* >(mpObj.get());
        awt::Point aUnoPoint;
        if(pMeasureObj && ( rValue >>= aUnoPoint ) )
        {
            Point aPoint( aUnoPoint.X, aUnoPoint.Y );

            // Reintroduction of fix for issue #i59051# (#i108851#)
            ForceMetricToItemPoolMetric( aPoint );
            if( mpModel->IsWriter() )
                aPoint += mpObj->GetAnchorPos();

            pMeasureObj->NbcSetPoint( aPoint, pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0L : 1L );
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

                eMode = (drawing::BitmapMode)nMode;
            }
            mpObj->SetMergedItem( XFillBmpStretchItem( eMode == drawing::BitmapMode_STRETCH ) );
            mpObj->SetMergedItem( XFillBmpTileItem( eMode == drawing::BitmapMode_REPEAT ) );
            return true;
        }

    case SDRATTR_LAYERID:
    {
        sal_Int16 nLayerId = sal_Int16();
        if( rValue >>= nLayerId )
        {
            SdrLayer* pLayer = mpModel->GetLayerAdmin().GetLayerPerID((unsigned char)nLayerId);
            if( pLayer )
            {
                mpObj->SetLayer((unsigned char)nLayerId);
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
            const SdrLayer* pLayer=mpModel->GetLayerAdmin().GetLayer(aLayerName, true);
            if( pLayer != nullptr )
            {
                mpObj->SetLayer( pLayer->GetID() );
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
            Point aRef1(mpObj->GetSnapRect().Center());
            nAngle -= mpObj->GetRotateAngle();
            if (nAngle!=0)
            {
                double nSin=sin(nAngle*nPi180);
                double nCos=cos(nAngle*nPi180);
                mpObj->Rotate(aRef1,nAngle,nSin,nCos);
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
            nShear -= mpObj->GetShearAngle();
            if(nShear != 0 )
            {
                Point aRef1(mpObj->GetSnapRect().Center());
                double nTan=tan(nShear*nPi180);
                mpObj->Shear(aRef1,nShear,nTan,false);
                return true;
            }
        }

        break;
    }

    case OWN_ATTR_INTEROPGRABBAG:
    {
        mpObj->SetGrabBagItem(rValue);
        return true;
    }

    case SDRATTR_OBJMOVEPROTECT:
    {
        bool bMoveProtect;
        if( rValue >>= bMoveProtect )
        {
            mpObj->SetMoveProtect(bMoveProtect);
            return true;
        }
        break;
    }
    case SDRATTR_OBJECTNAME:
    {
        OUString aName;
        if( rValue >>= aName )
        {
            mpObj->SetName( aName );
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
            mpObj->SetTitle( aTitle );
            return true;
        }
        break;
    }
    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription;
        if( rValue >>= aDescription )
        {
            mpObj->SetDescription( aDescription );
            return true;
        }
        break;
    }

    case SDRATTR_OBJPRINTABLE:
    {
        bool bPrintable;
        if( rValue >>= bPrintable )
        {
            mpObj->SetPrintable(bPrintable);
            return true;
        }
        break;
    }
    case SDRATTR_OBJVISIBLE:
    {
        bool bVisible;
        if( rValue >>= bVisible )
        {
            mpObj->SetVisible(bVisible);
            return true;
        }
        break;
    }
    case SDRATTR_OBJSIZEPROTECT:
    {
        bool bResizeProtect;
        if( rValue >>= bResizeProtect )
        {
            mpObj->SetResizeProtect(bResizeProtect);
            return true;
        }
        break;
    }
    case OWN_ATTR_PAGE_NUMBER:
    {
        sal_Int32 nPageNum = 0;
        if( (rValue >>= nPageNum) && ( nPageNum >= 0 ) && ( nPageNum <= 0xffff ) )
        {
            SdrPageObj* pPageObj = dynamic_cast< SdrPageObj* >(mpObj.get());
            if( pPageObj )
            {
                SdrModel* pModel = pPageObj->GetModel();
                SdrPage* pNewPage = nullptr;
                const sal_uInt16 nDestinationPageNum((sal_uInt16)((nPageNum << 1L) - 1L));

                if(pModel)
                {
                    if(nDestinationPageNum < pModel->GetPageCount())
                    {
                        pNewPage = pModel->GetPage(nDestinationPageNum);
                    }
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


bool SvxShape::getPropertyValueImpl( const OUString&, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    switch( pProperty->nWID )
    {
    case OWN_ATTR_CAPTION_POINT:
    {
        Point aVclPoint = static_cast<SdrCaptionObj*>(mpObj.get())->GetTailPos();

        // #88491# make pos relative to anchor
        if( mpModel->IsWriter() )
        {
            aVclPoint -= mpObj->GetAnchorPos();
        }

        // #88657# metric of pool maybe twips (writer)
        ForceMetricTo100th_mm(aVclPoint);

        // #90763# pos is absolute, make it relative to top left
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);

        aVclPoint.X() -= basegfx::fround(aNewHomogenMatrix.get(0, 2));
        aVclPoint.Y() -= basegfx::fround(aNewHomogenMatrix.get(1, 2));

        awt::Point aPnt( aVclPoint.X(), aVclPoint.Y() );
        rValue <<= aPnt;
        break;
    }

    case OWN_ATTR_TRANSFORMATION:
    {
        basegfx::B2DPolyPolygon aNewPolyPolygon;
        basegfx::B2DHomMatrix aNewHomogenMatrix;
        mpObj->TRGetBaseGeometry(aNewHomogenMatrix, aNewPolyPolygon);
        drawing::HomogenMatrix3 aMatrix;

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
        rValue <<= (sal_Int32)mpObj->GetOrdNum();
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
        rValue <<= dynamic_cast<const SdrTextObj*>(mpObj.get()) != nullptr && static_cast<SdrTextObj*>(mpObj.get())->IsFontwork();
        break;
    }

    case OWN_ATTR_FRAMERECT:
    {
        Rectangle aRect( mpObj->GetSnapRect() );
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
        Rectangle aRect( mpObj->GetCurrentBoundRect() );
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
        OUString aName( mpObj->GetName() );
        rValue <<= aName;
        break;
    }

    case OWN_ATTR_LDBITMAP:
    {
        sal_uInt16 nId;
        if( mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_OLE2 )
        {
            nId = RID_UNODRAW_OLE2;
        }
        else if( mpObj->GetObjInventor() == SdrInventor && mpObj->GetObjIdentifier() == OBJ_GRAF )
        {
            nId = RID_UNODRAW_GRAPHICS;
        }
        else
        {
            nId = RID_UNODRAW_OBJECTS;
        }

        BitmapEx aBmp( SVX_RES(nId) );
        Reference< awt::XBitmap > xBmp( VCLUnoHelper::CreateBitmap( aBmp ) );

        rValue <<= xBmp;
        break;
    }

    case OWN_ATTR_MIRRORED:
    {
        bool bMirror = false;
        if( mpObj.is() && dynamic_cast<const SdrGrafObj*>(mpObj.get()) != nullptr )
            bMirror = static_cast<SdrGrafObj*>(mpObj.get())->IsMirrored();

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
        SdrEdgeObj* pEdgeObj = dynamic_cast<SdrEdgeObj*>(mpObj.get());
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
                    if( mpModel->IsWriter() )
                        aPoint -= mpObj->GetAnchorPos();

                    ForceMetricTo100th_mm( aPoint );
                    awt::Point aUnoPoint( aPoint.X(), aPoint.Y() );

                    rValue <<= aUnoPoint;
                    break;
                }
            case OWN_ATTR_GLUEID_HEAD:
            case OWN_ATTR_GLUEID_TAIL:
                {
                    rValue <<= (sal_Int32)pEdgeObj->getGluePointIndex( pProperty->nWID == OWN_ATTR_GLUEID_HEAD );
                    break;
                }
            case OWN_ATTR_EDGE_POLYPOLYGONBEZIER:
                {
                    basegfx::B2DPolyPolygon aPolyPoly( pEdgeObj->GetEdgeTrackPath() );
                    if( mpModel->IsWriter() )
                    {
                        Point aPoint( mpObj->GetAnchorPos() );
                        aPolyPoly.transform(basegfx::tools::createTranslateB2DHomMatrix(-aPoint.X(), -aPoint.Y()));
                    }
                    // Reintroduction of fix for issue #i59051# (#i108851#)
                    ForceMetricTo100th_mm( aPolyPoly );
                    drawing::PolyPolygonBezierCoords aRetval;
                    basegfx::unotools::b2DPolyPolygonToPolyPolygonBezier( aPolyPoly, aRetval);
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
        SdrMeasureObj* pMeasureObj = dynamic_cast<SdrMeasureObj*>(mpObj.get());
        if(pMeasureObj)
        {
            Point aPoint( pMeasureObj->GetPoint( pProperty->nWID == OWN_ATTR_MEASURE_START_POS ? 0 : 1 ) );
            if( mpModel->IsWriter() )
                aPoint -= mpObj->GetAnchorPos();

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
        const SfxItemSet& rObjItemSet = mpObj->GetMergedItemSet();

        const XFillBmpStretchItem* pStretchItem = static_cast<const XFillBmpStretchItem*>(&rObjItemSet.Get(XATTR_FILLBMP_STRETCH));
        const XFillBmpTileItem* pTileItem = static_cast<const XFillBmpTileItem*>(&rObjItemSet.Get(XATTR_FILLBMP_TILE));

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
        rValue <<= (sal_Int16)mpObj->GetLayer();
        break;

    case SDRATTR_LAYERNAME:
    {
        SdrLayer* pLayer = mpModel->GetLayerAdmin().GetLayerPerID(mpObj->GetLayer());
        if( pLayer )
        {
            OUString aName( pLayer->GetName() );
            rValue <<= aName;
        }
        break;
    }

    case SDRATTR_ROTATEANGLE:
        rValue <<= (sal_Int32)mpObj->GetRotateAngle();
        break;

    case SDRATTR_SHEARANGLE:
        rValue <<= (sal_Int32)mpObj->GetShearAngle();
        break;

    case OWN_ATTR_INTEROPGRABBAG:
    {
        mpObj->GetGrabBagItem(rValue);
        break;
    }

    case SDRATTR_OBJMOVEPROTECT:
        rValue = uno::makeAny( mpObj->IsMoveProtect() );
        break;

    case SDRATTR_OBJECTNAME:
    {
        OUString aName( mpObj->GetName() );
        rValue <<= aName;
        break;
    }

    // #i68101#
    case OWN_ATTR_MISC_OBJ_TITLE:
    {
        OUString aTitle( mpObj->GetTitle() );
        rValue <<= aTitle;
        break;
    }

    case OWN_ATTR_MISC_OBJ_DESCRIPTION:
    {
        OUString aDescription( mpObj->GetDescription() );
        rValue <<= aDescription;
        break;
    }

    case SDRATTR_OBJPRINTABLE:
        rValue <<= mpObj->IsPrintable();
        break;

    case SDRATTR_OBJVISIBLE:
        rValue <<= mpObj->IsVisible();
        break;

    case SDRATTR_OBJSIZEPROTECT:
        rValue <<= mpObj->IsResizeProtect();
        break;

    case OWN_ATTR_PAGE_NUMBER:
    {
        SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(mpObj.get());
        if(pPageObj)
        {
            SdrPage* pPage = pPageObj->GetReferencedPage();
            sal_Int32 nPageNumber = (pPage) ? pPage->GetPageNum() : 0L;
            nPageNumber++;
            nPageNumber >>= 1;
            rValue <<= nPageNumber;
        }
        break;
    }

    case OWN_ATTR_UINAME_SINGULAR:
    {
        rValue <<= mpObj->TakeObjNameSingul();
        break;
    }

    case OWN_ATTR_UINAME_PLURAL:
    {
        rValue <<= mpObj->TakeObjNamePlural();
        break;
    }
    case OWN_ATTR_METAFILE:
    {
        SdrOle2Obj* pObj = dynamic_cast<SdrOle2Obj*>(mpObj.get());
        if( pObj )
        {
            const Graphic* pGraphic = pObj->GetGraphic();
            if( pGraphic )
            {
                bool bIsWMF = false;
                if ( pGraphic->IsLink() )
                {
                    GfxLink aLnk = pGraphic->GetLink();
                    if ( aLnk.GetType() == GFX_LINK_TYPE_NATIVE_WMF )
                    {
                        bIsWMF = true;
                        uno::Sequence<sal_Int8> aSeq(reinterpret_cast<sal_Int8 const *>(aLnk.GetData()), (sal_Int32) aLnk.GetDataSize());
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


bool SvxShape::getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        const SfxItemSet& rSet = mpObj->GetMergedItemSet();

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


bool SvxShape::setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    if( pProperty->nWID == OWN_ATTR_FILLBMP_MODE )
    {
        mpObj->ClearMergedItem( XATTR_FILLBMP_STRETCH );
        mpObj->ClearMergedItem( XATTR_FILLBMP_TILE );
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
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
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
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
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

void SAL_CALL SvxShape::_setPropertyToDefault( const OUString& PropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pProperty = mpPropSet->getPropertyMapEntry(PropertyName);

    if( !mpObj.is() || mpModel == nullptr || pProperty == nullptr )
        throw beans::UnknownPropertyException();

    if( !setPropertyToDefaultImpl( pProperty ) )
    {
        mpObj->ClearMergedItem( pProperty->nWID );
    }

    mpModel->SetChanged();
}


uno::Any SAL_CALL SvxShape::getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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

uno::Any SAL_CALL SvxShape::_getPropertyDefault( const OUString& aPropertyName )
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    const SfxItemPropertySimpleEntry* pMap = mpPropSet->getPropertyMapEntry(aPropertyName);

    if( !mpObj.is() || pMap == nullptr || mpModel == nullptr )
        throw beans::UnknownPropertyException();

    if(( pMap->nWID >= OWN_ATTR_VALUE_START && pMap->nWID <= OWN_ATTR_VALUE_END ) ||
       ( pMap->nWID >= SDRATTR_NOTPERSIST_FIRST && pMap->nWID <= SDRATTR_NOTPERSIST_LAST ))
    {
        return getPropertyValue( aPropertyName );
    }

    // get default from ItemPool
    if(!SfxItemPool::IsWhich(pMap->nWID))
        throw beans::UnknownPropertyException();

    SfxItemSet aSet( mpModel->GetItemPool(),    pMap->nWID, pMap->nWID);
    aSet.Put(mpModel->GetItemPool().GetDefaultItem(pMap->nWID));

    return GetAnyForItem( aSet, pMap );
}

// XMultiPropertyStates
void SvxShape::setAllPropertiesToDefault() throw (uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( !mpObj.is() )
        throw lang::DisposedException();
    mpObj->ClearMergedItem(); // nWhich == 0 => all

    if(dynamic_cast<const SdrGrafObj*>(mpObj.get()) != nullptr)
    {
        // defaults for graphic objects have changed:
        mpObj->SetMergedItem( XFillStyleItem( drawing::FillStyle_NONE ) );
        mpObj->SetMergedItem( XLineStyleItem( drawing::LineStyle_NONE ) );
    }

    // #i68523# special handling for Svx3DCharacterModeItem, this is not saved
    // but needs to be sal_True in svx, pool default (false) in sch. Since sch
    // does not load lathe or extrude objects, it is possible to set the items
    // here.
    // For other solution possibilities, see task description.
    if( dynamic_cast<const E3dLatheObj* >(mpObj.get())  != nullptr|| dynamic_cast<const E3dExtrudeObj* >(mpObj.get()) != nullptr)
    {
        mpObj->SetMergedItem(Svx3DCharacterModeItem(true));
    }

    mpModel->SetChanged();
}

void SvxShape::setPropertiesToDefault(
    const uno::Sequence<OUString>& aPropertyNames )
    throw (beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        setPropertyToDefault( aPropertyNames[pos] );
}

uno::Sequence<uno::Any> SvxShape::getPropertyDefaults(
    const uno::Sequence<OUString>& aPropertyNames )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException,
           uno::RuntimeException, std::exception)
{
    ::std::vector<uno::Any> ret;
    for ( sal_Int32 pos = 0; pos < aPropertyNames.getLength(); ++pos )
        ret.push_back( getPropertyDefault( aPropertyNames[pos] ) );
    return uno::Sequence<uno::Any>( &ret[0], ret.size() );
}


// XServiceInfo

OUString SAL_CALL SvxShape::getImplementationName()
    throw(uno::RuntimeException, std::exception)
{
    return OUString("SvxShape");
}

#define STAR_NAMESPACE "com.sun.star."

const char sUNO_service_style_ParagraphProperties[]          = STAR_NAMESPACE "style.ParagraphProperties";
const char sUNO_service_style_ParagraphPropertiesComplex[]   = STAR_NAMESPACE "style.ParagraphPropertiesComplex";
const char sUNO_service_style_ParagraphPropertiesAsian[]     = STAR_NAMESPACE "style.ParagraphPropertiesAsian";
const char sUNO_service_style_CharacterProperties[]          = STAR_NAMESPACE "style.CharacterProperties";
const char sUNO_service_style_CharacterPropertiesComplex[]   = STAR_NAMESPACE "style.CharacterPropertiesComplex";
const char sUNO_service_style_CharacterPropertiesAsian[]     = STAR_NAMESPACE "style.CharacterPropertiesAsian";

const char sUNO_service_drawing_FillProperties[]             = STAR_NAMESPACE "drawing.FillProperties";
const char sUNO_service_drawing_TextProperties[]             = STAR_NAMESPACE "drawing.TextProperties";
const char sUNO_service_drawing_LineProperties[]             = STAR_NAMESPACE "drawing.LineProperties";
const char sUNO_service_drawing_ConnectorProperties[]        = STAR_NAMESPACE "drawing.ConnectorProperties";
const char sUNO_service_drawing_MeasureProperties[]          = STAR_NAMESPACE "drawing.MeasureProperties";
const char sUNO_service_drawing_ShadowProperties[]           = STAR_NAMESPACE "drawing.ShadowProperties";

const char sUNO_service_drawing_RotationDescriptor[]         = STAR_NAMESPACE "drawing.RotationDescriptor";

const char sUNO_service_drawing_Text[]                       = STAR_NAMESPACE "drawing.Text";
const char sUNO_service_drawing_GroupShape[]                 = STAR_NAMESPACE "drawing.GroupShape";

const char sUNO_service_drawing_CustomShapeProperties[]      = STAR_NAMESPACE "drawing.CustomShapeProperties";
const char sUNO_service_drawing_CustomShape[]                    = STAR_NAMESPACE "drawing.CustomShape";

const char sUNO_service_drawing_PolyPolygonDescriptor[]      = STAR_NAMESPACE "drawing.PolyPolygonDescriptor";
const char sUNO_service_drawing_PolyPolygonBezierDescriptor[]= STAR_NAMESPACE "drawing.PolyPolygonBezierDescriptor";

const char sUNO_service_drawing_LineShape[]                  = STAR_NAMESPACE "drawing.LineShape";
const char sUNO_service_drawing_Shape[]                      = STAR_NAMESPACE "drawing.Shape";
const char sUNO_service_drawing_RectangleShape[]             = STAR_NAMESPACE "drawing.RectangleShape";
const char sUNO_service_drawing_EllipseShape[]               = STAR_NAMESPACE "drawing.EllipseShape";
const char sUNO_service_drawing_PolyPolygonShape[]           = STAR_NAMESPACE "drawing.PolyPolygonShape";
const char sUNO_service_drawing_PolyLineShape[]              = STAR_NAMESPACE "drawing.PolyLineShape";
const char sUNO_service_drawing_OpenBezierShape[]            = STAR_NAMESPACE "drawing.OpenBezierShape";
const char sUNO_service_drawing_ClosedBezierShape[]          = STAR_NAMESPACE "drawing.ClosedBezierShape";
const char sUNO_service_drawing_TextShape[]                  = STAR_NAMESPACE "drawing.TextShape";
const char sUNO_service_drawing_GraphicObjectShape[]         = STAR_NAMESPACE "drawing.GraphicObjectShape";
const char sUNO_service_drawing_OLE2Shape[]                  = STAR_NAMESPACE "drawing.OLE2Shape";
const char sUNO_service_drawing_PageShape[]                  = STAR_NAMESPACE "drawing.PageShape";
const char sUNO_service_drawing_CaptionShape[]               = STAR_NAMESPACE "drawing.CaptionShape";
const char sUNO_service_drawing_MeasureShape[]               = STAR_NAMESPACE "drawing.MeasureShape";
const char sUNO_service_drawing_FrameShape[]                 = STAR_NAMESPACE "drawing.FrameShape";
const char sUNO_service_drawing_ControlShape[]               = STAR_NAMESPACE "drawing.ControlShape";
const char sUNO_service_drawing_ConnectorShape[]             = STAR_NAMESPACE "drawing.ConnectorShape";
const char sUNO_service_drawing_MediaShape[]                 = STAR_NAMESPACE "drawing.MediaShape";


uno::Sequence< OUString > SAL_CALL SvxShape::getSupportedServiceNames()
    throw(uno::RuntimeException, std::exception)
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

uno::Sequence< OUString > SAL_CALL SvxShape::_getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpObj->GetObjInventor() == SdrInventor)
    {
        const sal_uInt16 nIdent = mpObj->GetObjIdentifier();

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
    else if( mpObj.is() && mpObj->GetObjInventor() == FmFormInventor)
    {
#if OSL_DEBUG_LEVEL > 0
        const sal_uInt16 nIdent = mpObj->GetObjIdentifier();
        OSL_ENSURE( nIdent == OBJ_UNO, "SvxShape::_getSupportedServiceNames: FmFormInventor, but no UNO object?" );
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

sal_Bool SAL_CALL SvxShape::supportsService( const OUString& ServiceName ) throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

// XGluePointsSupplier
uno::Reference< container::XIndexContainer > SAL_CALL SvxShape::getGluePoints()
    throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    uno::Reference< container::XIndexContainer > xGluePoints( mxGluePoints );

    if( mpObj.is() && !xGluePoints.is() )
    {
        uno::Reference< container::XIndexContainer > xNew( SvxUnoGluePointAccess_createInstance( mpObj.get() ), uno::UNO_QUERY );
        mxGluePoints = xGluePoints = xNew;
    }

    return xGluePoints;
}

// XChild
uno::Reference<uno::XInterface> SAL_CALL SvxShape::getParent()
    throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mpObj.is() && mpObj->GetObjList() )
    {
        SdrObjList* pObjList = mpObj->GetObjList();

        switch (pObjList->GetListKind())
        {
            case SDROBJLIST_GROUPOBJ:
                if (SdrObjGroup *pGroup = dynamic_cast<SdrObjGroup*>(pObjList->GetOwnerObj()))
                    return pGroup->getUnoShape();
                else if (E3dScene *pScene = dynamic_cast<E3dScene*>(pObjList->GetOwnerObj()))
                    return pScene->getUnoShape();
                break;
            case SDROBJLIST_DRAWPAGE:
            case SDROBJLIST_MASTERPAGE:
                return dynamic_cast<SdrPage&>(*pObjList).getUnoPage();
            default:
                OSL_FAIL( "SvxShape::getParent(  ): unexpected SdrObjListKind" );
                break;
        }
    }

    return uno::Reference<uno::XInterface>();
}

void SAL_CALL SvxShape::setParent( const css::uno::Reference< css::uno::XInterface >& )
    throw(lang::NoSupportException, uno::RuntimeException, std::exception)
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
sal_Bool SAL_CALL SvxShape::isActionLocked(  ) throw (css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    return mnLockCount != 0;
}


void SAL_CALL SvxShape::addActionLock(  ) throw (css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount < 0xffff, "lock overflow in SvxShape!" );
    mnLockCount++;

    if( mnLockCount == 1 )
        lock();
}


void SAL_CALL SvxShape::removeActionLock(  ) throw (css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    DBG_ASSERT( mnLockCount > 0, "lock underflow in SvxShape!" );
    mnLockCount--;

    if( mnLockCount == 0 )
        unlock();
}


void SAL_CALL SvxShape::setActionLocks( sal_Int16 nLock ) throw (css::uno::RuntimeException, std::exception )
{
    ::SolarMutexGuard aGuard;

    if( (mnLockCount == 0) && (nLock != 0) )
        unlock();

    if( (mnLockCount != 0) && (nLock == 0) )
        lock();

    mnLockCount = (sal_uInt16)nLock;
}


sal_Int16 SAL_CALL SvxShape::resetActionLocks(  ) throw (css::uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;

    if( mnLockCount != 0 )
        unlock();

    sal_Int16 nOldLocks = (sal_Int16)mnLockCount;
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
            const sal_uInt32 nId = mpObj->GetObjIdentifier();

            if( nId != mpImpl->mnObjId )
            {
                mpImpl->mnObjId = nId;

            }
            break;
        }
    }
}

SvxShapeText::SvxShapeText( SdrObject* pObject ) throw ()
: SvxShape( pObject, getSvxMapProvider().GetMap(SVXMAP_TEXT), getSvxMapProvider().GetPropertySet(SVXMAP_TEXT, SdrObject::GetGlobalDrawObjectItemPool()) ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject && pObject->GetModel() )
        SetEditSource( new SvxTextEditSource( pObject, nullptr ) );
}


SvxShapeText::SvxShapeText( SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet ) throw ()
: SvxShape( pObject, pPropertyMap, pPropertySet ), SvxUnoTextBase( ImplGetSvxUnoOutlinerTextCursorSvxPropertySet() )
{
    if( pObject && pObject->GetModel() )
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
    throw( uno::RuntimeException, std::exception )
{
    return SvxShape::queryInterface( rType );
}


uno::Any SAL_CALL SvxShapeText::queryAggregation( const uno::Type & rType )
    throw( uno::RuntimeException, std::exception )
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

OUString SAL_CALL SvxShapeText::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SvxShapeText");
}


uno::Sequence< OUString > SAL_CALL SvxShapeText::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    return SvxShape::getSupportedServiceNames();
}


sal_Bool SAL_CALL SvxShapeText::supportsService( const OUString& ServiceName ) throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(static_cast<SvxShape*>(this), ServiceName);
}

    // XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SvxShapeText::getTypes()
    throw( uno::RuntimeException, std::exception )
{
    return SvxShape::getTypes();
}

sal_Int64 SAL_CALL SvxShapeText::getSomething( const css::uno::Sequence< sal_Int8 >& rId ) throw(css::uno::RuntimeException, std::exception) \
{
    const sal_Int64 nReturn = SvxShape::getSomething( rId );
    if( nReturn )
        return nReturn;

    return SvxUnoTextBase::getSomething( rId );
}


uno::Sequence< sal_Int8 > SAL_CALL SvxShapeText::getImplementationId()
    throw( uno::RuntimeException, std::exception )
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
uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getStart() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getStart();

}

uno::Reference< text::XTextRange > SAL_CALL SvxShapeText::getEnd() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getEnd();
}

OUString SAL_CALL SvxShapeText::getString() throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    return SvxUnoTextBase::getString();
}


void SAL_CALL SvxShapeText::setString( const OUString& aString ) throw(uno::RuntimeException, std::exception)
{
    ::SolarMutexGuard aGuard;
    SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
    if( pForwarder )
        ::GetSelection( maSelection, pForwarder );
    SvxUnoTextBase::setString( aString );
}

// override these for special property handling in subcasses. Return true if property is handled
bool SvxShapeText::setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    // HACK-fix #99090#
    // since SdrTextObj::SetVerticalWriting exchanges
    // SDRATTR_TEXT_AUTOGROWWIDTH and SDRATTR_TEXT_AUTOGROWHEIGHT,
    // we have to set the textdirection here

    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObj.get() );
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

bool SvxShapeText::getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    if( pProperty->nWID == SDRATTR_TEXTDIRECTION )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mpObj.get() );
        if( pTextObj && pTextObj->IsVerticalWriting() )
            rValue <<= css::text::WritingMode_TB_RL;
        else
            rValue <<= css::text::WritingMode_LR_TB;
        return true;
    }

    return SvxShape::getPropertyValueImpl( rName, pProperty, rValue );
}

bool SvxShapeText::getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    return SvxShape::getPropertyStateImpl( pProperty, rState );
}

bool SvxShapeText::setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    return SvxShape::setPropertyToDefaultImpl( pProperty );
}

SvxShapeRect::SvxShapeRect( SdrObject* pObj ) throw()
: SvxShapeText( pObj, getSvxMapProvider().GetMap(SVXMAP_SHAPE), getSvxMapProvider().GetPropertySet(SVXMAP_SHAPE, SdrObject::GetGlobalDrawObjectItemPool()))
{
}

SvxShapeRect::~SvxShapeRect() throw()
{
}

uno::Any SAL_CALL SvxShapeRect::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException, std::exception)
{
    return SvxShapeText::queryInterface( rType );
}

uno::Any SAL_CALL SvxShapeRect::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException, std::exception)
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

uno::Sequence< OUString > SvxShapeRect::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
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
