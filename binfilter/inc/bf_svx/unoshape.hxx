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
#ifndef _SVX_UNOSHAPE_HXX
#define _SVX_UNOSHAPE_HXX

#include <com/sun/star/document/XActionLockable.hpp>
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
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

#include <tools/gen.hxx>

#include <bf_svtools/lstner.hxx>

#include <bf_svx/unoipset.hxx>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <osl/mutex.hxx>

#include <comphelper/servicehelper.hxx>

#include <cppuhelper/implbase11.hxx>

#include <bf_svx/unoprov.hxx>
class SvGlobalName;
namespace binfilter {
class SfxItemSet;

class SdrObject;
class SdrModel;
class SvxDrawPage;
class SvxShapeList;

class SvxShapeMutex
{
protected:
    ::osl::Mutex maMutex;
};

struct SvxShapeImpl;
class SvxShapeMaster;

// WARNING: if you update the supported interfaces, also update the
//			SvxShape::queryAggregation helper method
class SvxShape : public cppu::WeakAggImplHelper11<
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
                            ::com::sun::star::document::XActionLockable>,
                            public SfxListener,
                public SvxShapeMutex
{
 private:
    void		Init() throw();
    ::com::sun::star::awt::Size aSize;
    ::com::sun::star::awt::Point aPosition;
    ::rtl::OUString	aShapeType;
    ::rtl::OUString aShapeName;

    /** these members are used to optimize XMultiProperty calls */
    SvxShapeImpl* mpImpl;
    sal_Bool mbIsMultiPropertyCall;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XIndexContainer > mxGluePoints;

#ifndef SVX_LIGHT
    const SvGlobalName GetClassName_Impl(::rtl::OUString& rHexCLSID);
#endif
 protected:
    friend class SvxDrawPage;
    friend class SvxShapeConnector;
    friend class SdXShape;

    SvxItemPropertySet aPropSet;

    // for xComponent
    ::cppu::OInterfaceContainerHelper aDisposeListeners;
    BOOL bDisposing;

    SdrObject* pObj;
    SdrModel* pModel;

    // Umrechnungen fuer den Writer, der in TWIPS arbeitet
    void ForceMetricToItemPoolMetric(Pair& rPoint) const throw();
    void ForceMetricTo100th_mm(Pair& rPoint) const throw();

    ::com::sun::star::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertyMap* pMap ) const;

    sal_Bool queryAggregation( const ::com::sun::star::uno::Type & rType, ::com::sun::star::uno::Any& rAny );

    sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName );

    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

    /** used from the XActionLockable interface */
    sal_uInt16 mnLockCount;

    const SfxItemPropertyMap* getPropertyMap() const { return aPropSet.getPropertyMap(); }

    void updateShapeKind();

 public:
    SvxShape( SdrObject* pObj ) throw ();
    SvxShape( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ();
    virtual ~SvxShape() throw ();

    // Internals
    void ObtainSettingsFromPropertySet(SvxItemPropertySet& rPropSet) throw ();
    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

    void InvalidateSdrObject() { pObj = NULL; };
    SvxItemPropertySet& GetPropertySet() { return aPropSet; }
    SdrObject* GetSdrObject() const {return pObj;}
    void SetShapeType( const ::rtl::OUString& ShapeType ) { aShapeType = ShapeType; }
    ::com::sun::star::uno::Any GetBitmap( BOOL bMetaFile = FALSE ) const throw ();

    void setShapeKind( sal_uInt32 nKind );
    sal_uInt32 getShapeKind() const;

    // styles need this
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName, SfxItemSet& rSet, SdrModel* pModel );
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName, SfxItemSet& rSet );

    UNO3_GETIMPLEMENTATION_DECL( SvxShape )

    // access methods for master objects
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL _getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyDefault( const ::rtl::OUString& aPropertyName ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL _getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    void setMaster( SvxShapeMaster* pMaster );

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();

    // XNamed
    virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XShapeDescriptor
    virtual ::rtl::OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

    // XGluePointsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > SAL_CALL getGluePoints(  ) throw(::com::sun::star::uno::RuntimeException);

    // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addActionLock(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionLock(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionLocks( sal_Int16 nLock ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL resetActionLocks(  ) throw (::com::sun::star::uno::RuntimeException);
};
}//end of namespace binfilter
#include <bf_svx/unotext.hxx>
namespace binfilter {
class SvxShapeText : public SvxShape, public SvxUnoTextBase
{
protected:
/** called from the XActionLockable interface methods on initial locking */
virtual void lock();

/** called from the XActionLockable interface methods on final unlock */
virtual void unlock();

public:
SvxShapeText( SdrObject* pObj ) throw ();
SvxShapeText( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ();
virtual ~SvxShapeText() throw ();

virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

// XInterface
virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL acquire() throw();
virtual void SAL_CALL release() throw();

// XServiceInfo
virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);

// XUnoTunnel
virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException);

// XTypeProvider
virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

class SvxShapeRect : public SvxShapeText
{
public:
SvxShapeRect( SdrObject* pObj ) throw ();
virtual ~SvxShapeRect() throw ();

// XInterface
virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL acquire() throw();
virtual void SAL_CALL release() throw();

// XServiceInfo
virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};
}//end of namespace binfilter
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
namespace binfilter {
/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeGroup : public SvxShape,
public ::com::sun::star::drawing::XShapeGroup,
public ::com::sun::star::drawing::XShapes
{
private:
SvxDrawPage*	pPage;

public:
SvxShapeGroup( SdrObject* pObj,SvxDrawPage* pDrawPage ) throw ();
virtual ~SvxShapeGroup() throw ();

virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();

// XInterface
virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL acquire() throw();
virtual void SAL_CALL release() throw();

// XShapes
virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

// XElementAccess
virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

// XIndexAccess
virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

// XShapeDescriptor
virtual ::rtl::OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException);

// XShape
virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

// XShapeGroup
virtual void SAL_CALL enterGroup(  ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL leaveGroup(  ) throw(::com::sun::star::uno::RuntimeException);

// XServiceInfo
virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};
}//end of namespace binfilter

#include <com/sun/star/drawing/XConnectorShape.hpp>
namespace binfilter {
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
virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL acquire() throw();
virtual void SAL_CALL release() throw();

// XShapeDescriptor
virtual ::rtl::OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException);

// XShape
virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

// XConnectorShape
virtual void SAL_CALL connectStart( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape, ::com::sun::star::drawing::ConnectionType nPos ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL connectEnd( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape, ::com::sun::star::drawing::ConnectionType nPos ) throw(::com::sun::star::uno::RuntimeException);
virtual void SAL_CALL disconnectBegin( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disconnectEnd( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XConnectableShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};
}//end of namespace binfilter
#include <com/sun/star/drawing/XControlShape.hpp>
namespace binfilter {
/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeControl : public ::com::sun::star::drawing::XControlShape, public SvxShapeText
{
private:

    void convertPropertyName( const ::rtl::OUString& rApiName, ::rtl::OUString& rInternalName, sal_Bool& rNeedsConversion );
    void valueAlignToParaAdjust(::com::sun::star::uno::Any& rValue);  //added by BerryJia for fixing Bug102407 2002-11-04
    void valueParaAdjustToAlign(::com::sun::star::uno::Any& rValue);  //added by BerryJia for fixing Bug102407 2002-11-04

public:
    SvxShapeControl( SdrObject* pObj ) throw();
    virtual ~SvxShapeControl() throw();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XShapeDescriptor
    virtual ::rtl::OUString SAL_CALL getShapeType() throw(::com::sun::star::uno::RuntimeException);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    // XControlShape
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getControl() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xControl ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
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
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
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
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};
}//end of namespace binfilter
#include <bf_svx/xpoly.hxx>

namespace binfilter {
/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapePolyPolygon : public SvxShapeText
{
private:
    ::com::sun::star::drawing::PolygonKind ePolygonKind;
    XPolyPolygon aEmptyPoly;

public:
    SvxShapePolyPolygon( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::PropertyVetoException);
    virtual ~SvxShapePolyPolygon() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const XPolyPolygon& rNew) throw();
    const XPolyPolygon& GetPolygon() const throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxShapePolyPolygonBezier : public SvxShapeText
{
private:
    ::com::sun::star::drawing::PolygonKind ePolygonKind;
    XPolyPolygon aEmptyPoly;

public:
    SvxShapePolyPolygonBezier( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew ) throw();
    virtual ~SvxShapePolyPolygonBezier() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const XPolyPolygon& rNew) throw();
    const XPolyPolygon& GetPolygon() const throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxGraphicObject : public SvxShapeText
{
public:
    SvxGraphicObject( SdrObject* pObj ) throw();
    virtual ~SvxGraphicObject() throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,  ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSceneObject : public ::com::sun::star::drawing::XShapes, public SvxShape
{
private:
    SvxDrawPage*	pPage;

public:
    Svx3DSceneObject( SdrObject* pObj, SvxDrawPage* pDrawPage ) throw();
    virtual ~Svx3DSceneObject() throw();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XShapes
    virtual void SAL_CALL add( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DCubeObject : public SvxShape
{
public:
    Svx3DCubeObject( SdrObject* pObj ) throw();
    virtual ~Svx3DCubeObject() throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DSphereObject : public SvxShape
{
public:
    Svx3DSphereObject( SdrObject* pObj ) throw();
    virtual ~Svx3DSphereObject() throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DLatheObject : public SvxShape
{
public:
    Svx3DLatheObject( SdrObject* pObj ) throw();
    virtual ~Svx3DLatheObject() throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DExtrudeObject : public SvxShape
{
public:
    Svx3DExtrudeObject( SdrObject* pObj ) throw();
    virtual ~Svx3DExtrudeObject() throw();

    //XPropertySet
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DPolygonObject : public SvxShape
{
public:
    Svx3DPolygonObject( SdrObject* pObj ) throw();
    virtual ~Svx3DPolygonObject() throw();

    //XPropertySet
    virtual void SAL_CALL 	setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) 	throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
