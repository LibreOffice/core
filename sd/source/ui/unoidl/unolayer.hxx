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
#ifndef SD_UNOLAYER_HXX
#define SD_UNOLAYER_HXX

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>

#include <cppuhelper/implbase5.hxx>
#include <comphelper/servicehelper.hxx>

#include <unomodel.hxx>

class SdrLayer;
class SdLayerManager;
class SdXImpressDocument;
class SvUnoWeakContainer;

namespace sd {
class View;
}
enum LayerAttribute { VISIBLE, PRINTABLE, LOCKED };

/***********************************************************************
*                                                                      *
***********************************************************************/
class SdLayer : public ::cppu::WeakImplHelper5< ::com::sun::star::drawing::XLayer,
                                                ::com::sun::star::lang::XServiceInfo,
                                                ::com::sun::star::container::XChild,
                                                ::com::sun::star::lang::XUnoTunnel,
                                                ::com::sun::star::lang::XComponent >
{
public:
    SdLayer( SdLayerManager* pLayerManager_, SdrLayer* pSdrLayer_ ) throw();
    virtual ~SdLayer() throw();

    // intern
    SdrLayer* GetSdrLayer() const throw() { return pLayer; }

    static OUString convertToInternalName( const OUString& rName );
    static OUString convertToExternalName( const OUString& rName );

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayer )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XChild

    /** Returns the layer manager that manages this layer.
    */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    /** Not implemented.  Always throws an exception.
        @raises NoSupportException.
    */
    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

private:
    SdLayerManager*     pLayerManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayerManager > mxLayerManager;

    SdrLayer*           pLayer;
    const SvxItemPropertySet*   pPropSet;

    sal_Bool get( LayerAttribute what ) throw();
    void set( LayerAttribute what, sal_Bool flag ) throw();

};




/***********************************************************************
*                                                                      *
***********************************************************************/

class SdLayerManager : public ::cppu::WeakImplHelper5< ::com::sun::star::drawing::XLayerManager,
                                                       ::com::sun::star::container::XNameAccess,
                                                       ::com::sun::star::lang::XServiceInfo,
                                                       ::com::sun::star::lang::XUnoTunnel,
                                                       ::com::sun::star::lang::XComponent >
{
    friend class SdLayer;

public:
    SdLayerManager( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdLayerManager() throw();

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayerManager )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XLayerManager
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL remove( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL attachShapeToLayer( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer >& xLayer ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer > SAL_CALL getLayerForShape( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw(::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    /** Return the <type>XLayer</type> object that is associated with the
        given <type>SdrLayer</type> object.  If the requested object does
        not yet exist it is created.  All calls with the same argument
        return the same object.
        @param pLayer
            The <type>SdrLayer</type> object for which to return the
            associated <type>XLayer</type> object.
        @return
            The returned value is the unique <type>XLayer</type> object
            associated with the specified argument.  If no layer can be
            created for the argument than an empty reference is returned.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer> GetLayer (SdrLayer* pLayer);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

private:
    SdXImpressDocument* mpModel;
    SvUnoWeakContainer* mpLayers;

    ::sd::View* GetView() const throw();
    ::sd::DrawDocShell* GetDocShell() const throw() { return mpModel->mpDocShell; }
    void UpdateLayerView( sal_Bool modify = sal_True ) const throw();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
