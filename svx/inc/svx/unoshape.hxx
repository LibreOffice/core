/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoshape.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:33:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_UNOSHAPE_HXX
#define _SVX_UNOSHAPE_HXX

#ifndef _COM_SUN_STAR_DOCUMENT_XACTIONLOCKABLE_HPP_
#include <com/sun/star/document/XActionLockable.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XENHANCEDCUSTOMSHAPEDEFAULTER_HPP_
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XGLUEPOINTSSUPPLIER_HPP_
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONKIND_HPP_
#include <com/sun/star/drawing/PolygonKind.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSTATES_HPP_
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _TOOLS_WEAKBASE_HXX_
#include <tools/weakbase.hxx>
#endif

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

#ifndef _SVX_UNOIPSET_HXX_
#include <svx/unoipset.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <svx/svdouno.hxx>

#include <unotools/servicehelper.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase12.hxx>

#include <svx/unoprov.hxx>

class SfxItemSet;
class SdrModel;
class SvxDrawPage;
class SvGlobalName;

class SvxShapeMutex
{
protected:
    ::osl::Mutex maMutex;
};

struct SvxShapeImpl;
class SvxShapeMaster;

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
    SVX_DLLPRIVATE void     Init() throw();
    ::com::sun::star::awt::Size maSize;
    ::com::sun::star::awt::Point maPosition;
    ::rtl::OUString maShapeType;
    ::rtl::OUString maShapeName;

    /** these members are used to optimize XMultiProperty calls */
    SvxShapeImpl* mpImpl;
    bool mbIsMultiPropertyCall;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XIndexContainer > mxGluePoints;

protected:
    friend class SvxDrawPage;
    friend class SvxShapeConnector;
    friend class SdXShape;

    SvxItemPropertySet maPropSet;

    // for xComponent
    ::cppu::OInterfaceContainerHelper maDisposeListeners;
    bool mbDisposing;

    ::tools::WeakReference< SdrObject > mpObj;
    SdrModel* mpModel;
    // Umrechnungen fuer den Writer, der in TWIPS arbeitet
    void ForceMetricToItemPoolMetric(Pair& rPoint) const throw();
    void ForceMetricTo100th_mm(Pair& rPoint) const throw();

    ::com::sun::star::uno::Any GetAnyForItem( SfxItemSet& aSet, const SfxItemPropertyMap* pMap ) const;

    sal_Bool tryQueryAggregation( const com::sun::star::uno::Type & rType, com::sun::star::uno::Any& rAny );

    sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName );

    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

    /** used from the XActionLockable interface */
    sal_uInt16 mnLockCount;

    const SfxItemPropertyMap* getPropertyMap() const { return maPropSet.getPropertyMap(); }

    void updateShapeKind();
    void endSetPropertyValues();

    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyStateImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertyMap* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShape( SdrObject* pObj ) throw ();
    SvxShape( SdrObject* pObject, const SfxItemPropertyMap* pPropertySet ) throw ();
    SvxShape() throw ();
    virtual ~SvxShape() throw ();

    // Internals
    void ObtainSettingsFromPropertySet(SvxItemPropertySet& rPropSet) throw ();
    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw ();
    /** takes the ownership of the SdrObject.

        When the shape is disposed, and it has the ownership of its associated SdrObject, then
        it will delete this object.
    */
    void TakeSdrObjectOwnership();
    bool HasSdrObjectOwnership() const;

    void ChangeModel( SdrModel* pNewModel );

    void InvalidateSdrObject() { mpObj.reset( NULL ); };
    SvxItemPropertySet& GetPropertySet() { return maPropSet; }
    SdrObject* GetSdrObject() const {return mpObj.get();}
    void SetShapeType( const ::rtl::OUString& ShapeType ) { maShapeType = ShapeType; }
    ::com::sun::star::uno::Any GetBitmap( BOOL bMetaFile = FALSE ) const throw ();
    static SvxShape* GetShapeForSdrObj( SdrObject* pObj ) throw ();

    void setShapeKind( sal_uInt32 nKind );
    sal_uInt32 getShapeKind() const;

    // styles need this
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName, SfxItemSet& rSet, SdrModel* pModel );
    static sal_Bool SAL_CALL SetFillAttribute( sal_Int32 nWID, const ::rtl::OUString& rName, SfxItemSet& rSet );

    /** same as SetFillAttribute but for property names instead of which ids,
        and the property found is returned instead of set at the object
        directly.
     */
    static ::com::sun::star::uno::Any SAL_CALL GetFillAttributeByName(
        const ::rtl::OUString& rPropertyName, const ::rtl::OUString& rName, SdrModel* pModel );

    UNO3_GETIMPLEMENTATION_DECL( SvxShape )

    // access methods for master objects
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL _getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL _setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyDefault( const ::rtl::OUString& aPropertyName )     throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL _getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

    void setMaster( SvxShapeMaster* pMaster );
    const SvxShapeMaster* getMaster() const;
    SvxShapeMaster* getMaster();

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) throw ();


    /** called from SdrObject::SendUserCall
        Currently only called for SDRUSERCALL_CHILD_CHGATTR
    */
    virtual void onUserCall(SdrUserCallType eUserCall, const Rectangle& rBoundRect);
    // XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);

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
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName )  throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertiesToDefault(
        const ::com::sun::star::uno::Sequence<
        ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
    SAL_CALL getPropertyDefaults(
        const ::com::sun::star::uno::Sequence<
        ::rtl::OUString >& aPropertyNames )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

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

#include <svx/unotext.hxx>

class SVX_DLLPUBLIC SvxShapeText : public SvxShape, public SvxUnoTextBase
{
protected:
    /** called from the XActionLockable interface methods on initial locking */
    virtual void lock();

    /** called from the XActionLockable interface methods on final unlock */
    virtual void unlock();

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyStateImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::beans::PropertyState& rState ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual bool setPropertyToDefaultImpl( const SfxItemPropertyMap* pProperty ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShapeText() throw ();
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

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

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

#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEGROUP_HPP_
#include <com/sun/star/drawing/XShapeGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeGroup : public SvxShape,
                      public ::com::sun::star::drawing::XShapeGroup,
                      public ::com::sun::star::drawing::XShapes
{
private:
    rtl::Reference< SvxDrawPage> mxPage;

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

#ifndef _COM_SUN_STAR_DRAWING_XCONNECTORSHAPE_HPP_
#include <com/sun/star/drawing/XConnectorShape.hpp>
#endif

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

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

/***********************************************************************
*                                                                      *
***********************************************************************/
class SvxShapeControl : public ::com::sun::star::drawing::XControlShape, public SvxShapeText
{
private:
    void convertPropertyName( const rtl::OUString& rApiName, rtl::OUString& rInternalName );
    void valueAlignToParaAdjust(::com::sun::star::uno::Any& rValue);  //added by BerryJia for fixing Bug102407 2002-11-04
    void valueParaAdjustToAlign(::com::sun::star::uno::Any& rValue);  //added by BerryJia for fixing Bug102407 2002-11-04

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

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
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName )  throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

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
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    SvxShapePolyPolygon( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew = com::sun::star::drawing::PolygonKind_LINE ) throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::beans::PropertyVetoException);
    virtual ~SvxShapePolyPolygon() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const basegfx::B2DPolyPolygon& rNew) throw();
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
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
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    SvxShapePolyPolygonBezier( SdrObject* pObj , ::com::sun::star::drawing::PolygonKind eNew = com::sun::star::drawing::PolygonKind_PATHLINE) throw();
    virtual ~SvxShapePolyPolygonBezier() throw();

    // Local support functions
    ::com::sun::star::drawing::PolygonKind GetPolygonKind() const throw();
    void SetPolygon(const basegfx::B2DPolyPolygon & rNew) throw();
    basegfx::B2DPolyPolygon GetPolygon() const throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
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
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    SvxGraphicObject( SdrObject* pObj ) throw();
    virtual ~SvxGraphicObject() throw();
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
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

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
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DCubeObject( SdrObject* pObj ) throw();
    virtual ~Svx3DCubeObject() throw();

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
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~Svx3DSphereObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DLatheObject : public SvxShape
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DLatheObject( SdrObject* pObj ) throw();
    virtual ~Svx3DLatheObject() throw();

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
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~Svx3DExtrudeObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
class Svx3DPolygonObject : public SvxShape
{
protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

public:
    Svx3DPolygonObject( SdrObject* pObj ) throw();
    virtual ~Svx3DPolygonObject() throw();

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/
typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::drawing::XEnhancedCustomShapeDefaulter
    > SvxShape_UnoImplHelper1;

class SvxCustomShape : public SvxShapeText, public SvxShape_UnoImplHelper1
{
private:
    rtl::Reference< SvxDrawPage > mxPage;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxCustomShape( SdrObject* pObj ) throw ();
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual ~SvxCustomShape() throw ();

    virtual void Create( SdrObject* pNewOpj, SvxDrawPage* pNewPage = NULL ) throw();

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

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XEnhancedCustomShapeDefaulter
    virtual void SAL_CALL createCustomShapeDefaults( const rtl::OUString& rShapeType ) throw (::com::sun::star::uno::RuntimeException);
};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SvxMediaShape : public SvxShape
{
public:
    SvxMediaShape( SdrObject* pObj ) throw();
    virtual     ~SvxMediaShape() throw();

protected:
    // overide these for special property handling in subcasses. Return true if property is handled
    virtual bool setPropertyValueImpl( const SfxItemPropertyMap* pProperty, const ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual bool getPropertyValueImpl( const SfxItemPropertyMap* pProperty, ::com::sun::star::uno::Any& rValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

#endif
