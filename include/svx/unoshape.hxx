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

#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <sal/types.h>
#include <tools/weakbase.hxx>
#include <svl/lstner.hxx>
#include <editeng/unoipset.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <osl/mutex.hxx>
#include <svx/svxdllapi.h>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <svx/svdouno.hxx>

#include <comphelper/servicehelper.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase12.hxx>

#include <svx/unoprov.hxx>

class SfxItemSet;
class SdrModel;
class SvxDrawPage;
class SvGlobalName;
class Pair;
class IOpenGLRenderer;

// Dimension arrows change size/position on save/reload (#i59051#)
namespace basegfx
    {
        class B2DPolyPolygon;
    } // end of namespace basegfx

class SvxShapeMutex
{
protected:
    ::osl::Mutex maMutex;
};

struct SvxShapeImpl;
class SvxShapeMaster;
class SvxItemPropertySet;
class SfxItemSet;

void SVX_DLLPUBLIC SvxItemPropertySet_setPropertyValue( const SfxItemPropertySimpleEntry* pMap,
        const css::uno::Any& rVal, SfxItemSet& rSet );

css::uno::Any SVX_DLLPUBLIC SvxItemPropertySet_getPropertyValue( const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet );


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

class SVX_DLLPUBLIC SvxShape : public SvxShape_UnoImplHelper,
                 public SfxListener,
                 public SvxShapeMutex
{
private:
    css::awt::Size maSize;
    css::awt::Point maPosition;
    OUString maShapeType;
    OUString maShapeName;

    /** these members are used to optimize XMultiProperty calls */
    SvxShapeImpl* mpImpl;
    bool mbIsMultiPropertyCall;

    css::uno::WeakReference< css::container::XIndexContainer > mxGluePoints;

protected:
    friend class SvxDrawPage;
    friend class SvxShapeConnector;
    friend class SdXShape;

    const SvxItemPropertySet* mpPropSet;
    const SfxItemPropertyMapEntry* maPropMapEntries;

    ::tools::WeakReference< SdrObject > mpObj;
    SdrModel* mpModel;
    // translations for writer, which works in TWIPS
    void ForceMetricToItemPoolMetric(Pair& rPoint) const throw();
    void ForceMetricTo100th_mm(Pair& rPoint) const throw();
    // Dimension arrows change size/position on save/reload (#i59051#)
    void ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const throw();
    void ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const throw();

    css::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap ) const;

    bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName );

    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

    /** used from the XActionLockable interface */
    sal_uInt16 mnLockCount;

    const SfxItemPropertyMapEntry* getPropertyMapEntries() const { return maPropMapEntries; }

    void updateShapeKind();
    void endSetPropertyValues();

    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception);
    virtual bool getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);

public:
    SvxShape( SdrObject* pObj ) throw (css::uno::RuntimeException);
    SvxShape( SdrObject* pObject, const SfxItemPropertyMapEntry* pEntries, const SvxItemPropertySet* pPropertySet ) throw (css::uno::RuntimeException);
    SvxShape() throw (css::uno::RuntimeException);
    virtual ~SvxShape() throw ();

    // Internals
    void ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet);
    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = nullptr );
    /** takes the ownership of the SdrObject.

        When the shape is disposed, and it has the ownership of its associated SdrObject, then
        it will delete this object.
    */
    void TakeSdrObjectOwnership();
    bool HasSdrObjectOwnership() const;

    void ChangeModel( SdrModel* pNewModel );

    void InvalidateSdrObject() { mpObj.reset( nullptr ); };
    SdrObject* GetSdrObject() const {return mpObj.get();}
    void SetShapeType( const OUString& ShapeType ) { maShapeType = ShapeType; }
    css::uno::Any GetBitmap( bool bMetaFile = false ) const
        throw (css::uno::RuntimeException, std::exception);

    svx::PropertyChangeNotifier& getShapePropertyChangeNotifier();

    void setShapeKind( sal_uInt32 nKind );
    sal_uInt32 getShapeKind() const;

    // styles need this
    static bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet, SdrModel* pModel );
    static bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet );

    /** same as SetFillAttribute but for property names instead of which ids,
        and the property found is returned instead of set at the object
        directly.
     */

    UNO3_GETIMPLEMENTATION_DECL( SvxShape )

    // access methods for master objects
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL _getPropertySetInfo(  ) throw(css::uno::RuntimeException);
    void SAL_CALL _setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception);
    css::uno::Any SAL_CALL _getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    css::beans::PropertyState SAL_CALL _getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    void SAL_CALL _setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    css::uno::Any SAL_CALL _getPropertyDefault( const OUString& aPropertyName )     throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException);

    css::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames() throw(css::uno::RuntimeException);

    css::uno::Sequence< css::uno::Type > SAL_CALL _getTypes(  ) throw(css::uno::RuntimeException);

    void setMaster( SvxShapeMaster* pMaster );

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw () override;

    // XAggregation
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) throw (css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )  throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault(
        const css::uno::Sequence<
        OUString >& aPropertyNames )
        throw (css::beans::UnknownPropertyException,
               css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any >
    SAL_CALL getPropertyDefaults(
        const css::uno::Sequence<
        OUString >& aPropertyNames )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;

    // XGluePointsSupplier
    virtual css::uno::Reference< css::container::XIndexContainer > SAL_CALL getGluePoints(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addActionLock(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeActionLock(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL resetActionLocks(  ) throw (css::uno::RuntimeException, std::exception) override;

private:
    /** initializes SdrObj-dependent members. Only to be called when GetSdrObject() != NULL
    */
    SVX_DLLPRIVATE void impl_initFromSdrObject();
    /// CTOR-Impl
    SVX_DLLPRIVATE void impl_construct();
};

#include <editeng/unotext.hxx>

class SVX_DLLPUBLIC SvxShapeText : public SvxShape, public SvxUnoTextBase
{
protected:
    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock() override;

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock() override;

protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, css::beans::PropertyState& rState ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException) override;
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException) override;

public:
    SvxShapeText( SdrObject* pObj ) throw ();
    SvxShapeText( SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet ) throw ();
    virtual ~SvxShapeText() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = nullptr ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& aString ) throw(css::uno::RuntimeException, std::exception) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
};

class SvxShapeRect : public SvxShapeText
{
public:
    SvxShapeRect( SdrObject* pObj ) throw ();
    virtual ~SvxShapeRect() throw ();

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeGroup : public SvxShape,
                      public css::drawing::XShapeGroup,
                      public css::drawing::XShapes2,
                      public css::drawing::XShapes
{
private:
    rtl::Reference< SvxDrawPage> mxPage;

    void addUnoShape( const css::uno::Reference< css::drawing::XShape >& xShape, size_t nPos );

public:
    SvxShapeGroup( SdrObject* pObj,SvxDrawPage* pDrawPage ) throw ();
    virtual ~SvxShapeGroup() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = nullptr ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XShapes2
    virtual void SAL_CALL addTop( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addBottom( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XShapeGroup
    virtual void SAL_CALL enterGroup(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL leaveGroup(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
};
#include <com/sun/star/drawing/XConnectorShape.hpp>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeConnector : public css::drawing::XConnectorShape,
                          public SvxShapeText
{
public:
    SvxShapeConnector( SdrObject* pObj ) throw();
    virtual ~SvxShapeConnector() throw();

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XConnectorShape
    virtual void SAL_CALL connectStart( const css::uno::Reference< css::drawing::XConnectableShape >& xShape, css::drawing::ConnectionType nPos ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL connectEnd( const css::uno::Reference< css::drawing::XConnectableShape >& xShape, css::drawing::ConnectionType nPos ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disconnectBegin( const css::uno::Reference< css::drawing::XConnectableShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL disconnectEnd( const css::uno::Reference< css::drawing::XConnectableShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
};
#include <com/sun/star/drawing/XControlShape.hpp>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SVX_DLLPUBLIC SvxShapeControl : public css::drawing::XControlShape, public SvxShapeText
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxShapeControl( SdrObject* pObj ) throw();
    virtual ~SvxShapeControl() throw();

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )  throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XControlShape
    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getControl() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setControl( const css::uno::Reference< css::awt::XControlModel >& xControl ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeDimensioning : public SvxShapeText
{
public:
    SvxShapeDimensioning( SdrObject* pObj ) throw();
    virtual ~SvxShapeDimensioning() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeCircle : public SvxShapeText
{
public:
    SvxShapeCircle( SdrObject* pObj ) throw ();
    virtual ~SvxShapeCircle() throw ();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

// #i118485# changed parent to SvxShapeText to allow Text handling over UNO API
class SVX_DLLPUBLIC SvxOle2Shape : public SvxShapeText
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    void resetModifiedState();

    const SvGlobalName GetClassName_Impl(OUString& rHexCLSID);
public:
    SvxOle2Shape( SdrObject* pObj ) throw();
    SvxOle2Shape( SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet ) throw ();
    virtual ~SvxOle2Shape() throw();

    bool createObject( const SvGlobalName &aClassName );

    bool createLink( const OUString& aLinkURL );

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
};


#include <basegfx/polygon/b2dpolypolygon.hxx>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapePolyPolygon : public SvxShapeText
{
private:
    css::drawing::PolygonKind mePolygonKind;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;

public:
    SvxShapePolyPolygon( SdrObject* pObj , css::drawing::PolygonKind eNew = css::drawing::PolygonKind_LINE ) throw(css::lang::IllegalArgumentException, css::beans::PropertyVetoException);
    virtual ~SvxShapePolyPolygon() throw();

    // Local support functions
    css::drawing::PolygonKind GetPolygonKind() const throw() { return mePolygonKind;}
    void SetPolygon(const basegfx::B2DPolyPolygon& rNew) throw(css::uno::RuntimeException);
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxShapePolyPolygonBezier : public SvxShapeText
{
private:
    css::drawing::PolygonKind mePolygonKind;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;

    SvxShapePolyPolygonBezier( SdrObject* pObj , css::drawing::PolygonKind eNew = css::drawing::PolygonKind_PATHLINE) throw();
    virtual ~SvxShapePolyPolygonBezier() throw();

    // Local support functions
    css::drawing::PolygonKind GetPolygonKind() const throw() { return mePolygonKind;}
    void SetPolygon(const basegfx::B2DPolyPolygon & rNew) throw(css::uno::RuntimeException);
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxGraphicObject : public SvxShapeText
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

public:
    SvxGraphicObject( SdrObject* pObj, OUString const & referer ) throw();
    virtual ~SvxGraphicObject() throw();

private:
    OUString referer_;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSceneObject : public css::drawing::XShapes, public SvxShape
{
private:
    rtl::Reference< SvxDrawPage > mxPage;

protected:
    using SvxShape::setPropertyValue;
    using SvxShape::getPropertyValue;

public:
    Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw();
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyValueImpl(const OUString& rName, const SfxItemPropertySimpleEntry* pProperty,
        css::uno::Any& rValue )
            throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override;

    virtual ~Svx3DSceneObject() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = nullptr ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DCubeObject : public SvxShape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

public:
    Svx3DCubeObject( SdrObject* pObj ) throw();
    virtual ~Svx3DCubeObject() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSphereObject : public SvxShape
{
public:
    Svx3DSphereObject( SdrObject* pObj ) throw();
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual ~Svx3DSphereObject() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DLatheObject : public SvxShape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

public:
    Svx3DLatheObject( SdrObject* pObj ) throw();
    virtual ~Svx3DLatheObject() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DExtrudeObject : public SvxShape
{
public:
    Svx3DExtrudeObject( SdrObject* pObj ) throw();
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual ~Svx3DExtrudeObject() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DPolygonObject : public SvxShape
{
protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

public:
    Svx3DPolygonObject( SdrObject* pObj ) throw();
    virtual ~Svx3DPolygonObject() throw();

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
typedef ::cppu::WeakAggImplHelper1<
    css::drawing::XEnhancedCustomShapeDefaulter
    > SvxShape_UnoImplHelper1;

class SVX_DLLPUBLIC SvxCustomShape : public SvxShapeText, public SvxShape_UnoImplHelper1
{
private:
    rtl::Reference< SvxDrawPage > mxPage;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxCustomShape( SdrObject* pObj ) throw ();
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;


    virtual ~SvxCustomShape() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = nullptr ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(css::uno::RuntimeException, std::exception) override;

    // XShape
    virtual css::awt::Point SAL_CALL getPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPosition( const css::awt::Point& aPosition ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::awt::Size& aSize ) throw(css::beans::PropertyVetoException, css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw( css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    //XEnhancedCustomShapeDefaulter
    virtual void SAL_CALL createCustomShapeDefaults( const OUString& rShapeType ) throw (css::uno::RuntimeException, std::exception) override;
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxMediaShape : public SvxShape
{
public:
    SvxMediaShape( SdrObject* pObj, OUString const & referer ) throw();
    virtual     ~SvxMediaShape() throw();

protected:
    // override these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

private:
    OUString referer_;
};

/*
 * This is a really ugly hack for the chart2 OpenGL backend
 * SvxShapeGroup::add only accepts objects derived from SvxShape and silently drops
 * other objects. This fixes my life time problems but I will burn for it in hell.
 *
 * The object does nothing and should not be painted. It is just there to ensure that the
 * wrapped object is not deleted prematurely.
 */
class SVX_DLLPUBLIC SvxDummyShapeContainer : public SvxShape
{
private:
    css::uno::Reference< css::drawing::XShapes >
        m_xDummyObject;

public:
    SvxDummyShapeContainer( css::uno::Reference< css::drawing::XShapes > xWrappedObject );
    virtual ~SvxDummyShapeContainer() throw();

    css::uno::Reference< css::drawing::XShapes > getWrappedShape()
            { return m_xDummyObject; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
