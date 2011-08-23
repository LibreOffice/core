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
#ifndef _UNOLAYER_HXX
#define _UNOLAYER_HXX

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>

#include <cppuhelper/implbase4.hxx>
#include <comphelper/servicehelper.hxx>

#include <unomodel.hxx>
namespace binfilter {

class SdrLayer;
class SdLayerManager;
class SdXImpressDocument;
class SvUnoWeakContainer;

enum LayerAttribute { VISIBLE, PRINTABLE, LOCKED };

/***********************************************************************
*                                                                      *
***********************************************************************/
class SdLayer : public ::cppu::WeakImplHelper4< ::com::sun::star::drawing::XLayer,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::container::XChild,
                                                ::com::sun::star::lang::XUnoTunnel>
{
private:
    SdLayerManager*		pLayerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayerManager > mxLayerManager;

    SdrLayer*			pLayer;
    SvxItemPropertySet	aPropSet;

    sal_Bool get( LayerAttribute what ) throw();
    void set( LayerAttribute what, sal_Bool flag ) throw();

public:
    SdLayer( SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_ ) throw();
    virtual ~SdLayer() throw();

    // intern
    SdrLayer* GetSdrLayer() const throw() { return pLayer; }

    static String convertToInternalName( const ::rtl::OUString& rName );
    static ::rtl::OUString convertToExternalName( const String& rName );

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayer )

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException); 
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XChild
    
    /** Returns the layer manager that manages this layer.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);

    /** Not implemented.  Allways throws an exception.
        @raises NoSupportException.
    */
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

};




/***********************************************************************
*                                                                      *
***********************************************************************/

//#include <cppuhelper/implbase4.hxx>

class SdLayerManager : public ::cppu::WeakImplHelper4< ::com::sun::star::drawing::XLayerManager,
                                                       ::com::sun::star::container::XNameAccess,
                                                       ::com::sun::star::lang::XServiceInfo,
                                                       ::com::sun::star::lang::XUnoTunnel >
{
public:	
    SdLayerManager( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdLayerManager() throw();

    // uno helper
        static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XLayerManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attachShapeToLayer( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL getLayerForShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference<com::sun::star::drawing::XLayer> GetLayer (SdrLayer* pLayer);
    SdXImpressDocument& rModel;
    SvUnoWeakContainer*	mpLayers;
};

} //namespace binfilter
#endif


