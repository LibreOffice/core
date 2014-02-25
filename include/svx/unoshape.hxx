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

void SVX_DLLPUBLIC SvxItemPropertySet_setPropertyValue( const SvxItemPropertySet& rPropSet, const SfxItemPropertySimpleEntry* pMap,
        const com::sun::star::uno::Any& rVal, SfxItemSet& rSet );

com::sun::star::uno::Any SVX_DLLPUBLIC SvxItemPropertySet_getPropertyValue( const SvxItemPropertySet& rPropSet, const SfxItemPropertySimpleEntry* pMap, const SfxItemSet& rSet );


// WARNING: if you update the supported interfaces,
//          also update SvxShape::_getTypes()
typedef ::cppu::WeakAggImplHelper12<
    ::com::sun::star::drawing::XShape,
    ::com::sun::star::lang::XComponent,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XMultiPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::drawing::XGluePointsSupplier,
    ::com::sun::star::container::XChild,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::document::XActionLockable,
    ::com::sun::star::beans::XMultiPropertyStates> SvxShape_UnoImplHelper;

class SVX_DLLPUBLIC SvxShape : public SvxShape_UnoImplHelper,
                 public SfxListener,
                 public SvxShapeMutex
{
private:
    ::com::sun::star::awt::Size maSize;
    ::com::sun::star::awt::Point maPosition;
    OUString maShapeType;
    OUString maShapeName;

    /** these members are used to optimize XMultiProperty calls */
    SvxShapeImpl* mpImpl;
    bool mbIsMultiPropertyCall;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XIndexContainer > mxGluePoints;

protected:
    friend class SvxDrawPage;
    friend class SvxShapeConnector;
    friend class SdXShape;

    const SvxItemPropertySet* mpPropSet;
    const SfxItemPropertyMapEntry* maPropMapEntries;

    ::tools::WeakReference< SdrObject > mpObj;
    SdrModel* mpModel;
    // Umrechnungen fuer den Writer, der in TWIPS arbeitet
    void ForceMetricToItemPoolMetric(Pair& rPoint) const throw();
    void ForceMetricTo100th_mm(Pair& rPoint) const throw();
    // Dimension arrows change size/position on save/reload (#i59051#)
    void ForceMetricToItemPoolMetric(basegfx::B2DPolyPolygon& rPolyPolygon) const throw();
    void ForceMetricTo100th_mm(basegfx::B2DPolyPolygon& rPolyPolygon) const throw();

    ::com::sun::star::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertySimpleEntry* pMap ) const;

    sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName );

    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

    /** used from the XActionLockable interface */
    sal_uInt16 mnLockCount;

    const SfxItemPropertyMapEntry* getPropertyMapEntries() const { return maPropMapEntries; }

    void updateShapeKind();
    void endSetPropertyValues();

    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShape( SdrObject* pObj ) throw ();
    SvxShape( SdrObject* pObject, const SfxItemPropertyMapEntry* pEntries, const SvxItemPropertySet* pPropertySet ) throw ();
    SvxShape() throw ();
    virtual ~SvxShape() throw ();

    // Internals
    void ObtainSettingsFromPropertySet(const SvxItemPropertySet& rPropSet);
    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL );
    /** takes the ownership of the SdrObject.

        When the shape is disposed, and it has the ownership of its associated SdrObject, then
        it will delete this object.
    */
    void TakeSdrObjectOwnership();
    bool HasSdrObjectOwnership() const;

    void ChangeModel( SdrModel* pNewModel );

    void InvalidateSdrObject() { mpObj.reset( NULL ); };
    const SvxItemPropertySet& GetPropertySet() { return *mpPropSet; }
    SdrObject* GetSdrObject() const {return mpObj.get();}
    void SetShapeType( const OUString& ShapeType ) { maShapeType = ShapeType; }
    ::com::sun::star::uno::Any GetBitmap( sal_Bool bMetaFile = sal_False ) const throw ();

    ::svx::PropertyChangeNotifier& getShapePropertyChangeNotifier();

    void setShapeKind( sal_uInt32 nKind );
    sal_uInt32 getShapeKind() const;

    // styles need this
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet, SdrModel* pModel );
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const OUString& rName, SfxItemSet& rSet );

    /** same as SetFillAttribute but for property names instead of which ids,
        and the property found is returned instead of set at the object
        directly.
     */

    UNO3_GETIMPLEMENTATION_DECL( SvxShape )

    // access methods for master objects
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL _getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyDefault( const OUString& aPropertyName )     throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL _getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    void setMaster( SvxShapeMaster* pMaster );

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    // XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XNamed
    virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )  throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertiesToDefault(
        const ::com::sun::star::uno::Sequence<
        OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    SAL_CALL getPropertyDefaults(
        const ::com::sun::star::uno::Sequence<
        OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XGluePointsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > SAL_CALL getGluePoints(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addActionLock(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeActionLock(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int16 SAL_CALL resetActionLocks(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

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
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyStateImpl( const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertySimpleEntry* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShapeText( SdrObject* pObj ) throw ();
    SvxShapeText( SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet ) throw ();
    virtual ~SvxShapeText() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

class SvxShapeRect : public SvxShapeText
{
public:
    SvxShapeRect( SdrObject* pObj ) throw ();
    virtual ~SvxShapeRect() throw ();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeGroup : public SvxShape,
                      public ::com::sun::star::drawing::XShapeGroup,
                      public ::com::sun::star::drawing::XShapes2
{
private:
    rtl::Reference< SvxDrawPage> mxPage;

    void addUnoShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, sal_uIntPtr nPos );

public:
    SvxShapeGroup( SdrObject* pObj,SvxDrawPage* pDrawPage ) throw ();
    virtual ~SvxShapeGroup() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShapes2
    virtual void SAL_CALL addTop( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addBottom( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XShapeGroup
    virtual void SAL_CALL enterGroup(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL leaveGroup(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};
#include <com/sun/star/drawing/XConnectorShape.hpp>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeConnector : public ::com::sun::star::drawing::XConnectorShape,
                          public SvxShapeText
{
public:
    SvxShapeConnector( SdrObject* pObj ) throw();
    virtual ~SvxShapeConnector() throw();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XConnectorShape
    virtual void SAL_CALL connectStart( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape, ::com::sun::star::drawing::ConnectionType nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL connectEnd( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape, ::com::sun::star::drawing::ConnectionType nPos ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL disconnectBegin( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL disconnectEnd( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};
#include <com/sun/star/drawing/XControlShape.hpp>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SVX_DLLPUBLIC SvxShapeControl : public ::com::sun::star::drawing::XControlShape, public SvxShapeText
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxShapeControl( SdrObject* pObj ) throw();
    virtual ~SvxShapeControl() throw();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName )  throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XControlShape
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getControl() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xControl ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
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
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
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
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

// #i118485# changed parent to SvxShapeText to allow Text handling over UNO API
class SVX_DLLPUBLIC SvxOle2Shape : public SvxShapeText
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    void resetModifiedState();

    const SvGlobalName GetClassName_Impl(OUString& rHexCLSID);
public:
    SvxOle2Shape( SdrObject* pObj ) throw();
    SvxOle2Shape( SdrObject* pObject, const SfxItemPropertyMapEntry* pPropertyMap, const SvxItemPropertySet* pPropertySet ) throw ();
    virtual ~SvxOle2Shape() throw();

    sal_Bool createObject( const SvGlobalName &aClassName );

    sal_Bool createLink( const OUString& aLinkURL );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};


#include <basegfx/polygon/b2dpolypolygon.hxx>

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapePolyPolygon : public SvxShapeText
{
private:
    ::com::sun::star::drawing::PolygonKind mePolygonKind;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShapePolyPolygon( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew = com::sun::star::drawing::PolygonKind_LINE ) throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::beans::PropertyVetoException);
    virtual ~SvxShapePolyPolygon() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const basegfx::B2DPolyPolygon& rNew) throw();
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxShapePolyPolygonBezier : public SvxShapeText
{
private:
    ::com::sun::star::drawing::PolygonKind mePolygonKind;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    SvxShapePolyPolygonBezier( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew = com::sun::star::drawing::PolygonKind_PATHLINE) throw();
    virtual ~SvxShapePolyPolygonBezier() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const basegfx::B2DPolyPolygon & rNew) throw();
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxGraphicObject : public SvxShapeText
{
protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    SvxGraphicObject( SdrObject* pObj, OUString const & referer ) throw();
    virtual ~SvxGraphicObject() throw();

private:
    OUString referer_;
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSceneObject : public ::com::sun::star::drawing::XShapes, public SvxShape
{
private:
    rtl::Reference< SvxDrawPage > mxPage;

protected:
    using SvxShape::setPropertyValue;
    using SvxShape::getPropertyValue;

public:
    Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw();
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~Svx3DSceneObject() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DCubeObject : public SvxShape
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DCubeObject( SdrObject* pObj ) throw();
    virtual ~Svx3DCubeObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSphereObject : public SvxShape
{
public:
    Svx3DSphereObject( SdrObject* pObj ) throw();
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~Svx3DSphereObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DLatheObject : public SvxShape
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DLatheObject( SdrObject* pObj ) throw();
    virtual ~Svx3DLatheObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DExtrudeObject : public SvxShape
{
public:
    Svx3DExtrudeObject( SdrObject* pObj ) throw();
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~Svx3DExtrudeObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DPolygonObject : public SvxShape
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DPolygonObject( SdrObject* pObj ) throw();
    virtual ~Svx3DPolygonObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::drawing::XEnhancedCustomShapeDefaulter
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
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);


    virtual ~SvxCustomShape() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XShapeDescriptor
    virtual OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XPropertySet
    void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException, std::exception);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    //XEnhancedCustomShapeDefaulter
    virtual void SAL_CALL createCustomShapeDefaults( const OUString& rShapeType ) throw (::com::sun::star::uno::RuntimeException, std::exception);
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
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const OUString& rName, const SfxItemPropertySimpleEntry* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

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
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >
        m_xDummyObject;

public:
    SvxDummyShapeContainer( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xWrappedObject );
    virtual ~SvxDummyShapeContainer() throw();

    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > getWrappedShape()
            { return m_xDummyObject; }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
