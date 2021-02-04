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
#ifndef INCLUDED_SVX_UNOPAGE_HXX
#define INCLUDED_SVX_UNOPAGE_HXX

#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/drawing/XShapes3.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/basemutex.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdobjkind.hxx>

#include <cppuhelper/implbase7.hxx>
#include <comphelper/servicehelper.hxx>

#include <memory>

class SdrPage;
class SdrModel;
class SdrView;
class SdrPageView;
class SdrObject;
class SvxShape;
class SvxShapeGroup;
class SvxShapeConnector;
enum class SdrInventor : sal_uInt32;

class SVXCORE_DLLPUBLIC SvxDrawPage : protected cppu::BaseMutex,
                                    public ::cppu::WeakAggImplHelper7< css::drawing::XDrawPage,
                                               css::drawing::XShapeGrouper,
                                               css::drawing::XShapes2,
                                               css::drawing::XShapes3,
                                               css::lang::XServiceInfo,
                                               css::lang::XUnoTunnel,
                                               css::lang::XComponent>

{
 protected:
    cppu::OBroadcastHelper mrBHelper;

    SdrPage*        mpPage;     // TTTT should be reference
    SdrModel*       mpModel;    // TTTT probably not needed -> use from SdrPage
    std::unique_ptr<SdrView> mpView;

    void    SelectObjectsInView( const css::uno::Reference< css::drawing::XShapes >& aShapes, SdrPageView*   pPageView ) throw ();
    void    SelectObjectInView( const css::uno::Reference< css::drawing::XShape >& xShape, SdrPageView*  pPageView ) throw();

    virtual void disposing() throw();

 public:
    SvxDrawPage(SdrPage* pPage);
    virtual ~SvxDrawPage() throw() override;

    // Internals
    SdrPage* GetSdrPage() const { return mpPage; }

    // Creation of a SdrObject and insertion into the SdrPage
    SdrObject *CreateSdrObject( const css::uno::Reference< css::drawing::XShape >& xShape, bool bBeginning = false ) throw();

    // Determine Type and Inventor
    static void GetTypeAndInventor( SdrObjKind& rType, SdrInventor& rInventor, const OUString& aName ) throw();

    // Creating a SdrObject using it's Description.
    // Can be used by derived classes to support their owen Shapes (e.g. Controls).
    /// @throws css::uno::RuntimeException
    virtual SdrObject *CreateSdrObject_( const css::uno::Reference< css::drawing::XShape >& xShape );

    /// @throws css::uno::RuntimeException
    static SvxShape* CreateShapeByTypeAndInventor( sal_uInt16 nType, SdrInventor nInventor, SdrObject *pObj, SvxDrawPage *pPage = nullptr, OUString const & referer = OUString() );

    // The following method is called if a SvxShape object is to be created.
    // Derived classes can create a derivation or an SvxShape aggregating object.
    /// @throws css::uno::RuntimeException
    virtual css::uno::Reference< css::drawing::XShape > CreateShape( SdrObject *pObj ) const;

    UNO3_GETIMPLEMENTATION_DECL( SvxDrawPage )

    // XInterface
    virtual void SAL_CALL release() throw() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XShapes2
    virtual void SAL_CALL addTop( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL addBottom( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XShapes3
    virtual void SAL_CALL sort( const css::uno::Sequence< sal_Int32 >& sortOrder ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XShapeGrouper
    virtual css::uno::Reference< css::drawing::XShapeGroup > SAL_CALL group( const css::uno::Reference< css::drawing::XShapes >& xShapes ) override;
    virtual void SAL_CALL ungroup( const css::uno::Reference< css::drawing::XShapeGroup >& aGroup ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
