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

#ifndef INCLUDED_SVX_UNOSHAPE_HXX
#define INCLUDED_SVX_UNOSHAPE_HXX

#include <memory>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/drawing/XConnectorShape.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <sal/types.h>
#include <svl/lstner.hxx>
#include <editeng/unoipset.hxx>
#include <svx/svxdllapi.h>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <editeng/unotext.hxx>

#include <svx/svdobj.hxx>
#include <unotools/weakref.hxx>

#include <comphelper/servicehelper.hxx>

#include <cppuhelper/implbase12.hxx>

class SfxItemSet;
class SdrModel;
class SvxDrawPage;
class SvGlobalName;
class Pair;

// Dimension arrows change size/position on save/reload (#i59051#)
namespace basegfx
    {
        class B2DPolyPolygon;
    } // end of namespace basegfx

struct SvxShapeImpl;
class SvxShapeMaster;
class SvxItemPropertySet;
class SfxItemSet;

void SVXCORE_DLLPUBLIC SvxItemPropertySet_setPropertyValue( const SfxItemPropertyMapEntry* pMap,
        const css::uno::Any& rVal, SfxItemSet& rSet );

css::uno::Any SVXCORE_DLLPUBLIC SvxItemPropertySet_getPropertyValue( const SfxItemPropertyMapEntry* pMap, const SfxItemSet& rSet );


// WARNING: if you update the supported interfaces,
//          also update SvxShape::_getTypes()
typedef ::cppu::WeakAggImplHelper12<
    css::drawing::XShape,
    css::lang::XComponent,
    css::beans::XPropertySet,
    css::beans::XMultiPropertySet,
    css::beans::XPropertyState,
    css::lang::XUnoTunnel,
    css::container::XNamed,
    css::drawing::XGluePointsSupplier,
    css::container::XChild,
    css::lang::XServiceInfo,
    css::document::XActionLockable,
    css::beans::XMultiPropertyStates> SvxShape_UnoImplHelper;

class SVXCORE_DLLPUBLIC SvxShape :
                 public SvxShape_UnoImplHelper,
                 public SfxListener
{
protected:
    friend class SvxDrawPage;
    friend class SvxShapeConnector;
    friend class SdXShape;

    // translations for writer, which works in TWIPS
    void ForceMetricToItemPoolMetric(Pair& rPoint) const noexcept;
    void ForceMetricToItemPoolMetric(Point& rPoint) const noexcept { ForceMetricToItemPoolMetric(rPoint.toPair()); }
    void ForceMetricToItemPoolMetric(Size& rPoint) const noexcept { ForceMetricToItemPoolMetric(rPoint.toPair()); }
    void ForceMetricTo100th_mm(Pair& rPoint) const noexcept;
    void ForceMetricTo100th_mm(Point& rPoint) const noexcept { ForceMetricTo100th_mm(rPoint.toPair()); }
    void ForceMetricTo100th_mm(Size& rPoint) const noexcept { ForceMetricTo100th_mm(rPoint.toPair()); }

    // version for basegfx::B2DPolyPolygon
    void ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const noexcept;
    void ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const noexcept;

    // tdf#117145 version for basegfx::B2DHomMatrix
    void ForceMetricToItemPoolMetric(basegfx::B2DHomMatrix& rB2DHomMatrix) const noexcept;
    void ForceMetricTo100th_mm(basegfx::B2DHomMatrix& rB2DHomMatrix) const noexcept;

    css::uno::Any GetAnyForItem( SfxItemSet const & aSet, const SfxItemPropertyMapEntry* pMap ) const;

    bool SetFillAttribute( sal_uInt16 nWID, const OUString& rName );

    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

    std::span<const SfxItemPropertyMapEntry> getPropertyMapEntries() const { return maPropMapEntries; }

    void updateShapeKind();
    void endSetPropertyValues();

    // override these for special property handling in subcasses. Return true if property is handled
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual bool getPropertyStateImpl( const SfxItemPropertyMapEntry* pProperty, css::beans::PropertyState& rState );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertyMapEntry* pProperty );

public:
    /// @throws css::uno::RuntimeException
    SvxShape( SdrObject* pObj );
    /// @throws css::uno::RuntimeException
    SvxShape( SdrObject* pObject, std::span<const SfxItemPropertyMapEntry> pEntries, const SvxItemPropertySet* pPropertySet );
    virtual ~SvxShape() noexcept override;

    // Internals
    void ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet);
    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage );

    // used exclusively by SdrObject
    void InvalidateSdrObject();

    // Encapsulated access to SdrObject
    SdrObject* GetSdrObject() const { return mxSdrObject.get(); }
    bool HasSdrObject() const { return mxSdrObject.is(); }

    void SetShapeType( const OUString& ShapeType ) { maShapeType = ShapeType; }
    /// @throws css::uno::RuntimeException
    css::uno::Any GetBitmap( bool bMetaFile = false ) const;

    void notifyPropertyChange(const OUString& rPropName);

    void setShapeKind( SdrObjKind nKind );
    SdrObjKind getShapeKind() const;

    // styles need this
    static bool SetFillAttribute( sal_uInt16 nWID, const OUString& rName, SfxItemSet& rSet, SdrModel const * pModel );
    static bool SetFillAttribute( sal_uInt16 nWID, const OUString& rName, SfxItemSet& rSet );

    /** same as SetFillAttribute but for property names instead of which ids,
        and the property found is returned instead of set at the object
        directly.
     */

    UNO3_GETIMPLEMENTATION_DECL( SvxShape )

    // access methods for master objects
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::beans::XPropertySetInfo > const & _getPropertySetInfo(  );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void _setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Any _getPropertyValue( const OUString& PropertyName );

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    css::beans::PropertyState _getPropertyState( const OUString& PropertyName );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    void _setPropertyToDefault( const OUString& PropertyName );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Any _getPropertyDefault( const OUString& aPropertyName );

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< OUString > _getSupportedServiceNames();

    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Type > const & _getTypes(  );

    void setMaster( SvxShapeMaster* pMaster );

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) noexcept override;

    // XAggregation
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType ) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) override;
    virtual void SAL_CALL setName( const OUString& aName ) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault() override;
    virtual void SAL_CALL setPropertiesToDefault(
        const css::uno::Sequence<
        OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any >
    SAL_CALL getPropertyDefaults(
        const css::uno::Sequence<
        OUString >& aPropertyNames ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    // XGluePointsSupplier
    virtual css::uno::Reference< css::container::XIndexContainer > SAL_CALL getGluePoints(  ) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked(  ) override;
    virtual void SAL_CALL addActionLock(  ) override;
    virtual void SAL_CALL removeActionLock(  ) override;
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 SAL_CALL resetActionLocks(  ) override;

private:
    /** initializes SdrObj-dependent members. Only to be called when GetSdrObject() != NULL
    */
    SVX_DLLPRIVATE void impl_initFromSdrObject();
    /// CTOR-Impl
    SVX_DLLPRIVATE void impl_construct();

    std::mutex m_aMutex;
    css::awt::Size maSize;
    css::awt::Point maPosition;
    OUString maShapeType;
    OUString maShapeName;
    /** these members are used to optimize XMultiProperty calls */
    std::unique_ptr<SvxShapeImpl> mpImpl;
    bool mbIsMultiPropertyCall;

    css::uno::WeakReference< css::container::XIndexContainer > mxGluePoints;
    const SvxItemPropertySet* mpPropSet;
    SvxItemPropertySetUsrAnys maUrsAnys;
    std::span<const SfxItemPropertyMapEntry> maPropMapEntries;
    rtl::Reference< SdrObject > mxSdrObject;
    /** used from the XActionLockable interface */
    sal_uInt16 mnLockCount;
};

class SVXCORE_DLLPUBLIC SvxShapeText : public SvxShape, public SvxUnoTextBase
{
protected:
    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock() override;

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock() override;

protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;
    virtual bool getPropertyStateImpl( const SfxItemPropertyMapEntry* pProperty, css::beans::PropertyState& rState ) override;
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertyMapEntry* pProperty ) override;

public:
    SvxShapeText(SdrObject* pObj);
    SvxShapeText(SdrObject* pObject, std::span<const SfxItemPropertyMapEntry> pPropertyMap, const SvxItemPropertySet* pPropertySet);
    virtual ~SvxShapeText() noexcept override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShape::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShape::release(); }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString( const OUString& aString ) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};

class UNLESS_MERGELIBS(SAL_DLLPUBLIC_RTTI) SvxShapeRect final : public SvxShapeText
{
public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxShapeRect(SdrObject* pObj);
    virtual ~SvxShapeRect() noexcept override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { OWeakAggObject::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { OWeakAggObject::release(); }

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/// This only exists so I have a common base class for SvxShapeGroup and Svx3DSceneObject
class SVXCORE_DLLPUBLIC SvxShapeGroupAnyD : public SvxShape,
                      public css::drawing::XShapes
{
public:
    SvxShapeGroupAnyD( SdrObject* pObject, std::span<const SfxItemPropertyMapEntry> pEntries, const SvxItemPropertySet* pPropertySet );
    virtual ~SvxShapeGroupAnyD() noexcept override;

    virtual void SAL_CALL acquire() noexcept override
    { SvxShape::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShape::release(); }

    virtual void addShape(SvxShape& rShape) = 0;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxShapeGroup final : public SvxShapeGroupAnyD,
                      public css::drawing::XShapeGroup,
                      public css::drawing::XShapes2
{
private:
    /// using a weak reference to prevent leaks via ref-counting cycles
    unotools::WeakReference< SvxDrawPage> mxWeakPage;

    void addUnoShape( const css::uno::Reference< css::drawing::XShape >& xShape, size_t nPos );
    void addShape(SvxShape& rShape, size_t nPos);

public:
    SvxShapeGroup(SdrObject* pObj,SvxDrawPage* pDrawPage);
    virtual ~SvxShapeGroup() noexcept override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShape::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShape::release(); }

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XShapes2
    virtual void SAL_CALL addTop( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL addBottom( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    // XShapeGroup
    virtual void SAL_CALL enterGroup(  ) override;
    virtual void SAL_CALL leaveGroup(  ) override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    virtual void addShape(SvxShape& rShape) final override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeConnector final : public css::drawing::XConnectorShape,
                          public SvxShapeText
{
public:
    SvxShapeConnector(SdrObject* pObj);
    virtual ~SvxShapeConnector() noexcept override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShapeText::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShapeText::release(); }

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    // XConnectorShape
    virtual void SAL_CALL connectStart( const css::uno::Reference< css::drawing::XConnectableShape >& xShape, css::drawing::ConnectionType nPos ) override;
    virtual void SAL_CALL connectEnd( const css::uno::Reference< css::drawing::XConnectableShape >& xShape, css::drawing::ConnectionType nPos ) override;
    virtual void SAL_CALL disconnectBegin( const css::uno::Reference< css::drawing::XConnectableShape >& xShape ) override;
    virtual void SAL_CALL disconnectEnd( const css::uno::Reference< css::drawing::XConnectableShape >& xShape ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) SvxShapeControl final : public css::drawing::XControlShape, public SvxShapeText
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxShapeControl(SdrObject* pObj);
    virtual ~SvxShapeControl() noexcept override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShapeText::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShapeText::release(); }

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) override;

    // XControlShape
    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getControl() override;
    virtual void SAL_CALL setControl( const css::uno::Reference< css::awt::XControlModel >& xControl ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeDimensioning final : public SvxShapeText
{
public:
    SvxShapeDimensioning(SdrObject* pObj);
    virtual ~SvxShapeDimensioning() noexcept override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeCircle final : public SvxShapeText
{
public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxShapeCircle(SdrObject* pObj);
    virtual ~SvxShapeCircle() noexcept override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

// #i118485# changed parent to SvxShapeText to allow Text handling over UNO API
class UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) SvxOle2Shape : public SvxShapeText
{
private:
    OUString referer_;
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

    void resetModifiedState();

    SvGlobalName GetClassName_Impl(OUString& rHexCLSID);
public:
    SvxOle2Shape(SdrObject* pObj, OUString referer);
    SvxOle2Shape(SdrObject* pObject, OUString referer, std::span<const SfxItemPropertyMapEntry> pPropertyMap, const SvxItemPropertySet* pPropertySet);
    virtual ~SvxOle2Shape() noexcept override;

    bool createObject( const SvGlobalName &aClassName );

    void createLink( const OUString& aLinkURL );

    virtual OUString GetAndClearInitialFrameURL();
};


/***********************************************************************
*                                                                      *
***********************************************************************/
class UNLESS_MERGELIBS(SAL_DLLPUBLIC_RTTI) SvxShapePolyPolygon final : public SvxShapeText
{
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

    // local helper to detect PolygonKind from SdrObject::GetObjIdentifier()
    css::drawing::PolygonKind GetPolygonKind() const;

public:
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::beans::PropertyVetoException
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxShapePolyPolygon( SdrObject* pObj );
    virtual ~SvxShapePolyPolygon() noexcept override;

    // Local support functions
    /// @throws css::uno::RuntimeException
    void SetPolygon(const basegfx::B2DPolyPolygon& rNew);
    basegfx::B2DPolyPolygon GetPolygon() const noexcept;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxGraphicObject final : public SvxShapeText
{
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) SvxGraphicObject(SdrObject* pObj);
    virtual ~SvxGraphicObject() noexcept override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class UNLESS_MERGELIBS(SAL_DLLPUBLIC_RTTI) Svx3DSceneObject final : public SvxShapeGroupAnyD
{
private:
    rtl::Reference< SvxDrawPage > mxPage;

protected:
    using SvxShape::setPropertyValue;
    using SvxShape::getPropertyValue;

public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) Svx3DSceneObject(SdrObject* pObj, SvxDrawPage* pDrawPage);
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl(const OUString& rName, const SfxItemPropertyMapEntry* pProperty,
        css::uno::Any& rValue ) override;

    virtual ~Svx3DSceneObject() noexcept override;

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShape::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShape::release(); }

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    virtual void addShape(SvxShape& rShape) override final;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DCubeObject final : public SvxShape
{
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

public:
    Svx3DCubeObject(SdrObject* pObj);
    virtual ~Svx3DCubeObject() noexcept override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSphereObject final : public SvxShape
{
public:
    Svx3DSphereObject(SdrObject* pObj);
private:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

    virtual ~Svx3DSphereObject() noexcept override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DLatheObject final : public SvxShape
{
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) Svx3DLatheObject(SdrObject* pObj);
    virtual ~Svx3DLatheObject() noexcept override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DExtrudeObject final : public SvxShape
{
public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) Svx3DExtrudeObject(SdrObject* pObj);
private:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

    virtual ~Svx3DExtrudeObject() noexcept override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DPolygonObject final : public SvxShape
{
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;

public:
    UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) Svx3DPolygonObject(SdrObject* pObj);
    virtual ~Svx3DPolygonObject() noexcept override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SVXCORE_DLLPUBLIC SvxCustomShape final : public SvxShapeText, public css::drawing::XEnhancedCustomShapeDefaulter
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxCustomShape(SdrObject* pObj);
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;


    virtual ~SvxCustomShape() noexcept override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override
    { SvxShapeText::acquire(); }
    virtual void SAL_CALL release() noexcept override
    { SvxShapeText::release(); }

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() override;

    // XPropertySet
    void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    //XEnhancedCustomShapeDefaulter
    virtual void SAL_CALL createCustomShapeDefaults( const OUString& rShapeType ) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxMediaShape final : public SvxShape
{
public:
    SvxMediaShape(SdrObject* pObj, OUString referer);
    virtual     ~SvxMediaShape() noexcept override;

private:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, const css::uno::Any& rValue ) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertyMapEntry* pProperty, css::uno::Any& rValue ) override;
    bool getPropertyStateImpl(const SfxItemPropertyMapEntry* pProperty,
                              css::beans::PropertyState& rState) override;

    OUString referer_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
