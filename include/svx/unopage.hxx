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
#ifndef _SVX_UNOWPAGE_HXX
#define _SVX_UNOWPAGE_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XShapeCombiner.hpp>
#include <com/sun/star/drawing/XShapeBinder.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <svl/lstner.hxx>
#include <editeng/mutxhelp.hxx>
#include "svx/svxdllapi.h"

#include <cppuhelper/implbase5.hxx>
#include <comphelper/servicehelper.hxx>

#include <svx/unoprov.hxx>

class SdrPage;
class SdrModel;
class SdrView;
class SdrPageView;
class SdrObject;
class SvxShape;
class SvxShapeGroup;
class SvxShapeConnector;

/**
* Macros to convert Twips<->100tel mm
*/
#define TWIPS_TO_MM(val) ((val * 127 + 36) / 72)
#define MM_TO_TWIPS(val) ((val * 72 + 63) / 127)

class SVX_DLLPUBLIC SvxDrawPage : public ::cppu::WeakAggImplHelper5< ::com::sun::star::drawing::XDrawPage,
                                               ::com::sun::star::drawing::XShapeGrouper,
                                               ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::lang::XUnoTunnel,
                                               ::com::sun::star::lang::XComponent>,
                    public SfxListener,
                    protected SvxMutexHelper
{
 protected:
    cppu::OBroadcastHelper mrBHelper;

    SdrPage*        mpPage;
    SdrModel*       mpModel;
    SdrView*        mpView;

    void    _SelectObjectsInView( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& aShapes, SdrPageView*   pPageView ) throw ();
    void    _SelectObjectInView( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, SdrPageView*  pPageView ) throw();

    virtual void disposing() throw();

 public:
    SvxDrawPage( SdrPage* pPage ) throw();
    virtual ~SvxDrawPage() throw();

    // Internals
    SdrPage* GetSdrPage() const { return mpPage; }
    void ChangeModel( SdrModel* pNewModel );

    // Creation of a SdrObject and insertion into the SdrPage
    SdrObject *CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    // Determine Type and Inventor
    void GetTypeAndInventor( sal_uInt16& rType, sal_uInt32& rInventor, const OUString& aName ) const throw();

    // Creating a SdrObject using it's Description.
    // Can be used by derived classes to support their owen Shapes (e.g. Controls).
    virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape ) throw();

    static SvxShape* CreateShapeByTypeAndInventor( sal_uInt16 nType, sal_uInt32 nInventor, SdrObject *pObj = NULL, SvxDrawPage *pPage = NULL ) throw();

    // The following method is called if a SvxShape object is to be created.
    // Derived classes can create a derivation or an SvxShape aggregating object.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > _CreateShape( SdrObject *pObj ) const throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxDrawPage )

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // XInterface
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

    // XShapeGrouper
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup > SAL_CALL group( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ungroup( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapeGroup >& aGroup ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
