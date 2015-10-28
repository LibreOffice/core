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
#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_UNOLAYER_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_UNOLAYER_HXX

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include <com/sun/star/drawing/XLayerManager.hpp>

#include <cppuhelper/implbase.hxx>
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
class SdLayer : public ::cppu::WeakImplHelper< css::drawing::XLayer,
                                                css::lang::XServiceInfo,
                                                css::container::XChild,
                                                css::lang::XUnoTunnel,
                                                css::lang::XComponent >
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
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::container::XChild

    /** Returns the layer manager that manages this layer.
    */
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getParent(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    /** Not implemented.  Always throws an exception.
        @raises NoSupportException.
    */
    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& Parent ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;

private:
    SdLayerManager*     pLayerManager;
    css::uno::Reference< css::drawing::XLayerManager > mxLayerManager;

    SdrLayer*           pLayer;
    const SvxItemPropertySet*   pPropSet;

    bool get( LayerAttribute what ) throw();
    void set( LayerAttribute what, bool flag ) throw();

};

/***********************************************************************
*                                                                      *
***********************************************************************/

class SdLayerManager : public ::cppu::WeakImplHelper< css::drawing::XLayerManager,
                                                       css::container::XNameAccess,
                                                       css::lang::XServiceInfo,
                                                       css::lang::XUnoTunnel,
                                                       css::lang::XComponent >
{
    friend class SdLayer;

public:
    explicit SdLayerManager( SdXImpressDocument& rMyModel ) throw();
    virtual ~SdLayerManager() throw();

    // uno helper
    UNO3_GETIMPLEMENTATION_DECL( SdLayerManager )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XLayerManager
    virtual css::uno::Reference< css::drawing::XLayer > SAL_CALL insertNewByIndex( sal_Int32 nIndex ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XLayer >& xLayer ) throw(css::container::NoSuchElementException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL attachShapeToLayer( const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::drawing::XLayer >& xLayer ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::drawing::XLayer > SAL_CALL getLayerForShape( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

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
    css::uno::Reference< css::drawing::XLayer> GetLayer (SdrLayer* pLayer);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

private:
    SdXImpressDocument* mpModel;
    SvUnoWeakContainer* mpLayers;

    ::sd::View* GetView() const throw();
    ::sd::DrawDocShell* GetDocShell() const throw() { return mpModel->mpDocShell; }
    void UpdateLayerView( bool modify = true ) const throw();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
