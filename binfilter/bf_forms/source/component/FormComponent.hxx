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

#ifndef _FORMS_FORMCOMPONENT_HXX_
#define _FORMS_FORMCOMPONENT_HXX_

#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADLISTENER_HPP_
#include <com/sun/star/form/XLoadListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
#endif
#ifndef FORMS_COMPONENT_CLONEABLE_HXX
#include "cloneable.hxx"
#endif
namespace binfilter {

    using namespace ::comphelper;


//.........................................................................
namespace frm
{
//.........................................................................

// default tab index for components
const sal_Int16 FRM_DEFAULT_TABINDEX = 0;

// macro for quickly implementing XServiceInfo::getImplementationName
#define IMPLEMENTATION_NAME(ImplName)										\
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) \
        { return ::rtl::OUString::createFromAscii("com.sun.star.comp.forms.") + ::rtl::OUString::createFromAscii(#ImplName); }

// macro for overriding the getInfoServive method of OAggregationArrayUsageHelper
#define IMPLEMENT_INFO_SERVICE()	\
    virtual IPropertyInfoService* getInfoService() const { return &s_aPropInfos; }

//==================================================================
//= substitutions for the xxx_AGGREGATION_PROPERTY_HELPER-macros
//==================================================================
#define FRM_BEGIN_PROP_HELPER(count)	\
    BEGIN_AGGREGATION_PROPERTY_HELPER(count, m_xAggregateSet)	\

#define FRM_END_PROP_HELPER()	\
    END_AGGREGATION_PROPERTY_HELPER();	\

//=========================================================================
//= OControl
//= base class for form layer controls
//=========================================================================
typedef ::cppu::ImplHelper3	<	::com::sun::star::awt::XControl
                            ,	::com::sun::star::lang::XEventListener
                            ,	::com::sun::star::lang::XServiceInfo
                            > OControl_BASE;

class OControl	:public ::cppu::OComponentHelper
                ,public OControl_BASE
{
protected:
    osl::Mutex									m_aMutex;
    OImplementationIdsRef						m_aHoldIdHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
                                                m_xControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation>
                                                m_xAggregate;
    ::rtl::OUString								m_aService;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                                m_xServiceFactory;

public:
    OControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rFactory, const ::rtl::OUString& _sService);

    virtual ~OControl();

// UNO
    DECLARE_UNO3_AGG_DEFAULTS(OControl, OComponentHelper);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence<sal_Int8>			SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// XComponent (as base of XControl)
    virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException)
        { OComponentHelper::dispose(); }
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException)
        { OComponentHelper::addEventListener(_rxListener); }
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException)
        { OComponentHelper::removeEventListener(_rxListener); }

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual sal_Bool SAL_CALL			supportsService(const ::rtl::OUString& ServiceName) throw (::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL		getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL	getImplementationName() throw(::com::sun::star::uno::RuntimeException) = 0;

// XControl
    virtual void										SAL_CALL setContext(const InterfaceRef& Context) throw (::com::sun::star::uno::RuntimeException);
    virtual InterfaceRef								SAL_CALL getContext() throw (::com::sun::star::uno::RuntimeException);
    virtual void										SAL_CALL createPeer(const ::com::sun::star::uno::Reference<starawt::XToolkit>& Toolkit, const ::com::sun::star::uno::Reference<starawt::XWindowPeer>& Parent) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<starawt::XWindowPeer>	SAL_CALL getPeer() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool									SAL_CALL setModel(const ::com::sun::star::uno::Reference<starawt::XControlModel>& Model) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<starawt::XControlModel>	SAL_CALL getModel() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<starawt::XView>			SAL_CALL getView() throw (::com::sun::star::uno::RuntimeException);
    virtual void										SAL_CALL setDesignMode(sal_Bool bOn) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool									SAL_CALL isDesignMode() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool									SAL_CALL isTransparent() throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();
        // overwrite this and call the base class if you have additional types
};

//==================================================================
//= OBoundControl
//= a form control implementing the XBoundControl interface
//==================================================================
typedef ::cppu::ImplHelper1< ::com::sun::star::form::XBoundControl >  OBoundControl_BASE;
class OBoundControl	:public OControl
                    ,public OBoundControl_BASE
{
protected:
    sal_Bool	m_bLocked : 1;

public:
    OBoundControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sService);
    virtual ~OBoundControl();

    DECLARE_UNO3_AGG_DEFAULTS(OBoundControl, OControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

// XBoundControl
    virtual sal_Bool SAL_CALL	getLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL		setLock(sal_Bool _bLock) throw(::com::sun::star::uno::RuntimeException);
        // default implementation just disables the controls, overwrite _setLock to change this behaviour

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();
    // implement the lock setting
    virtual void		 _setLock(sal_Bool _bLock);
};

//==================================================================
//= OControlModel
//= model of a form layer control
//==================================================================
typedef ::cppu::ImplHelper5	<	::com::sun::star::form::XFormComponent
                            ,	::com::sun::star::io::XPersistObject
                            ,	::com::sun::star::container::XNamed
                            ,	::com::sun::star::lang::XServiceInfo
                            ,	::com::sun::star::util::XCloneable
                            >	OControlModel_BASE;


class OControlModel	:public ::cppu::OComponentHelper
                    ,public OPropertySetAggregationHelper
                    ,public OControlModel_BASE
                    ,public OCloneableAggregation
{

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>	m_xServiceFactory;

    ::osl::Mutex			m_aMutex;
        // we use this kind of mutex to check the correct using of MutexRelease
        // (in the product version OCountedMutex is defined as OMutex)

    InterfaceRef					m_xParent;					// ParentComponent
    OImplementationIdsRef			m_aHoldIdHelper;

    static ConcretInfoService s_aPropInfos;

    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
        getORB( ) const { return m_xServiceFactory; }

// <properties>
    ::rtl::OUString					m_aName;					// name of the control
    ::rtl::OUString					m_aTag;						// tag for additional data
    sal_Int16						m_nTabIndex;				// index within the taborder
    sal_Int16						m_nClassId;					// type of the control
// </properties>


protected:
    OControlModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rFactory,	// factory to create the aggregate with
        const ::rtl::OUString& _rUnoControlModelTypeName,						// service name of te model to aggregate
        const ::rtl::OUString& rDefault = ::rtl::OUString(),					// service name of the default control
        const sal_Bool _bSetDelegator = sal_True								// set to FALSE if you want to call setDelegator later (after returning from this ctor)
    );
    OControlModel(
        const OControlModel* _pOriginal,										// the original object to clone
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rFactory,	// factory to create the aggregate with
        const sal_Bool _bSetDelegator = sal_True								// set to FALSE if you want to call setDelegator later (after returning from this ctor)
    );
    virtual ~OControlModel();

    using OComponentHelper::rBHelper;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();

    void	readHelpTextCompatibly(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& _rxInStream);
    void	writeHelpTextCompatibly(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& _rxOutStream);

    void	doSetDelegator();
    void	doResetDelegator();

public:
    DECLARE_UNO3_AGG_DEFAULTS(OControl, OComponentHelper);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence<sal_Int8>			SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// XNamed
    virtual ::rtl::OUString SAL_CALL	getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL				setName(const ::rtl::OUString& aName) throw(::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual sal_Bool SAL_CALL			supportsService(const ::rtl::OUString& ServiceName) throw (::com::sun::star::uno::RuntimeException);
    virtual StringSequence SAL_CALL		getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL	getImplementationName() throw(::com::sun::star::uno::RuntimeException) = 0;

// XPersistObject
    virtual ::rtl::OUString SAL_CALL	getServiceName() throw(::com::sun::star::uno::RuntimeException) = 0;
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// XChild (base of XFormComponent)
    virtual InterfaceRef SAL_CALL	getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL			setParent(const InterfaceRef& Parent) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

// XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);

// ::com::sun::star::beans::XPropertyState
    virtual	::com::sun::star::beans::PropertyState getPropertyStateByHandle(sal_Int32 nHandle);
    virtual	void setPropertyToDefaultByHandle(sal_Int32 nHandle);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

// XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
};

//==================================================================
#define DECLARE_DEFAULT_XTOR( classname )	\
    classname( \
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, \
        const ::rtl::OUString& _rUnoControlModelTypeName, \
        const ::rtl::OUString& _rDefault, \
        const sal_Bool _bSetDelegator = sal_True \
    ); \
    classname( \
        const classname* _pOriginal, \
        const	::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, \
        const sal_Bool _bSetDelegator = sal_True \
    ); \
    ~classname() \

#define DECLARE_DEFAULT_LEAF_XTOR( classname )	\
    classname( \
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory \
    ); \
    classname( \
        const classname* _pOriginal, \
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory \
    ); \
    ~classname() \

//==================================================================
// XCloneable
#define DECLARE_XCLONEABLE( ) \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException)

#define IMPLEMENT_DEFAULT_CLONING( classname ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL classname::createClone( ) throw (::com::sun::star::uno::RuntimeException) \
    { \
        return new classname( this, getORB() ); \
    }

//==================================================================
//= OBoundControlModel
//= model of a form layer control which is bound to a data source field
//==================================================================
typedef ::cppu::ImplHelper3<	::com::sun::star::form::XLoadListener,
                                ::com::sun::star::form::XReset,
                                ::com::sun::star::beans::XPropertyChangeListener >	OBoundControlModel_BASE1;
typedef ::cppu::ImplHelper1<	::com::sun::star::form::XBoundComponent>			OBoundControlModel_BASE2;

class OBoundControlModel	:public OControlModel
                            ,public OBoundControlModel_BASE1
                            ,public OBoundControlModel_BASE2
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>		m_xField;
protected:
    cppu::OInterfaceContainerHelper	m_aUpdateListeners;
    cppu::OInterfaceContainerHelper	m_aResetListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>				m_xCursor;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumnUpdate>			m_xColumnUpdate;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>				m_xColumn;
    

    ::rtl::OUString					m_aLabelServiceName;
        // when setting the label for our control (property FM_PROP_CONTROLLABEL, member m_xLabelControl),
        // we accept only objects supporting an XControlModel interface, an XServiceInfo interface and
        // support for a service (XServiceInfo::supportsService) determined by this string.
        // Any other arguments will throw an IllegalArgumentException.
        // The default value is FM_COMPONENT_FIXEDTEXT.
    ::rtl::OUString					m_sDataFieldConnectivityProperty;

// <properties>
    ::rtl::OUString								m_aControlSource;			// Datenquelle, Name des Feldes
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>	m_xLabelControl;			// reference to a sibling control (model) which is our label
// </properties>

    sal_Bool					m_bLoaded		: 1;
    sal_Bool					m_bRequired		: 1;
    sal_Bool					m_bCommitable	: 1;
    sal_Bool					m_bForwardValueChanges : 1;

    sal_Bool					m_bResetting	: 1;	// sal_True while in ::reset

protected:
    OBoundControlModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rFactory,	// factory to create the aggregate with
        const ::rtl::OUString& _rUnoControlModelTypeName,						// service name of te model to aggregate
        const ::rtl::OUString& _rDefault = ::rtl::OUString(),					// service name of the default control
        const sal_Bool _bCommitable = sal_True,									// is the control (model) commitable ?
        const sal_Bool _bSetDelegator = sal_True								// set to FALSE if you want to call setDelegator later (after returning from this ctor)
    );
    OBoundControlModel(
        const OBoundControlModel* _pOriginal,									// the original object to clone
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rFactory,	// factory to create the aggregate with
        const sal_Bool _bCommitable = sal_True,									// is the control (model) commitable ?
        const sal_Bool _bSetDelegator = sal_True								// set to FALSE if you want to call setDelegator later (after returning from this ctor)
    );
    virtual ~OBoundControlModel();

    // UNO
    virtual sal_Bool		 _commit() { return sal_True; }

    // Laden der Werte aus dem Datenbankfeld
    // wird bei Reset und Load aufgerufen, der Wert kann als Parameter uebergeben werden
    virtual void		 _onValueChanged() = 0;
    virtual void		 _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void		 _unloaded();
    virtual void		 _reset();

    /**	nFieldType ist der Typ des Feldes, an das das Model gebunden werden soll.
        Das Binden erfolgt genau dann, wenn Rueckgabewert sal_True.
        Die Standard-Implementation erlaubt alles ausser den drei binary-Typen und
        FieldType_OTHER.
    */
    virtual sal_Bool		_approve(sal_Int32 _nColumnType);

    // We can't write (new) common properties in this base class, as the file format doesn't allow this
    // (unfortunally). So derived classes may use the following to methods. They secured the written
    // data with marks, so any new common properties in newer versions will be skipped by older ones.
    void	writeCommonProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream);
    void	readCommonProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream);
    // the next method may be used in derived classes's read when an unknown version is encountered
    void	defaultCommonProperties();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();

    void setField( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxField,sal_Bool _bFire=sal_True);
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& getField() const
    {
        return m_xField;
    }

public:
    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OBoundControlModel, OControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

// OComponentHelper
    virtual void SAL_CALL disposing();

// XReset
    virtual void SAL_CALL reset(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

// XServiceInfo
    virtual StringSequence SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// XChild
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

// XPersistObject
    virtual void SAL_CALL write( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& OutStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& InStream ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// XBoundComponent
    virtual sal_Bool SAL_CALL commit() throw(::com::sun::star::uno::RuntimeException);
// XUpdateBroadcaster (base of XBoundComponent)
    virtual void SAL_CALL addUpdateListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeUpdateListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

// XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);

// XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw (::com::sun::star::uno::RuntimeException);

// XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

// XLoadListener
    virtual void SAL_CALL loaded( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloading( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloading( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL reloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

private:
    sal_Bool connectToField(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _rxForm);
    void resetField() { m_xColumnUpdate = NULL; m_xColumn = NULL; m_xField = NULL; }
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_FORMCOMPONENT_HXX_

