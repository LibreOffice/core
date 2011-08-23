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

#ifndef _CHXCHART_OBJECT_HXX
#define _CHXCHART_OBJECT_HXX

#include <cppuhelper/implbase8.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/interfacecontainer.hxx>

// header for SvxServiceInfoHelper, SvxItemPropertySet
#include <bf_svx/unoprov.hxx>
namespace binfilter {

class ChartModel;
class SdrObject;

class ChXChartObject : public cppu::WeakImplHelper8<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::beans::XMultiPropertySet,
    ::com::sun::star::beans::XPropertyState,
    ::com::sun::star::beans::XMultiPropertyStates,
    ::com::sun::star::drawing::XShape,
    ::com::sun::star::lang::XComponent,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel >
{
protected:
    SvxItemPropertySet maPropSet;
    ChartModel* mpModel;
    long mnWhichId;
    long mnIndex;		// typically a data row index

public:
    ChXChartObject( long _MapId, ChartModel* _Model, long _WhichId, long _Index = -1 );
    virtual ~ChXChartObject();

    virtual long GetId() const;
    SdrObject* GetCurrentSdrObject() const;
     static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName,
                                            const ::com::sun::star::uno::Any& aValue )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );

    //	XMultiPropertySet
    virtual void SAL_CALL setPropertyValues	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) 
        throw (	::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) 
        throw (	::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) 
        throw (	::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener	(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) 
        throw (	::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
        throw (	::com::sun::star::uno::RuntimeException);


    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates(
        const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName )
        throw( ::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException );

    //	XMultiPropertyStates
/*    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) 
        throw (	::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
*/	virtual void SAL_CALL setAllPropertiesToDefault	(void)
        throw (	::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertiesToDefault	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
        throw (	::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults	(
                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) 
        throw (	::com::sun::star::beans::UnknownPropertyException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    // XShape
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize )
        throw( ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException ); 

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw( ::com::sun::star::uno::RuntimeException );

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier )
        throw( ::com::sun::star::uno::RuntimeException );
   

protected:
    /**	@descr	In an array of SfxItemPropertyMap entries advance from the position pointed to
            by pProperty to that property that has the name given by pPropertyName.  If the 
            property list does not contain such an entry an UnknownPropertyException is thrown.
        @param	pProperty Pointer into an array of properties.  It is modified such that after
            the methods returns it points to a property with the same name as pPropertyName.
        @param	pPropertyName	Name of the property that is searched for.
    */
    void AdvanceToName	(const SfxItemPropertyMap *& pProperty, 
                        const ::rtl::OUString * pPropertyName);
    
    /**	@descr	Creates an item set and fill it with the models default values.
            If derived classes need other than the default which ranges, the have to overload
            this method.
        @return	Returns a pointer to an item set.  The caller is responsible for deleting it.
    */				
    virtual	SfxItemSet * CreateItemSet	(void);
    
    /**	@descr	Retrieve the value of the property specified by rProperty from the item set given
            by rAttributes and put it into rValue.
        @param	rProperty	Specifies the property in question.
        @param	rValue	The property's value is stored into this argument.
        @param	rAttributes	The item set that contains all relevant items from which the property
            value is extracted.
    */
    virtual	void GetPropertyValue	(const SfxItemPropertyMap & rProperty,
                                    ::com::sun::star::uno::Any & rValue, 
                                    SfxItemSet & rAttributes);

private:
    ///	List of listeners for the XComponent interface.
    ::cppu::OInterfaceContainerHelper	maListenerList;
    
    ///	Mutex used by the interface container.
    ::osl::Mutex	maMutex;
};

} //namespace binfilter
#endif	// _CHXCHART_OBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
