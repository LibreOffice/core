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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXREGION_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXREGION_HXX


#include <com/sun/star/awt/XRegion.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>

#include <vcl/region.hxx>


//  class VCLXRegion


class VCLXRegion :  public ::com::sun::star::awt::XRegion,
                    public ::com::sun::star::lang::XTypeProvider,
                    public ::com::sun::star::lang::XUnoTunnel,
                    public ::cppu::OWeakObject
{
private:
    ::osl::Mutex    maMutex;
    vcl::Region          maRegion;

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }

public:
                    VCLXRegion();
                    virtual ~VCLXRegion();

    const vcl::Region&   GetRegion() const                   { return maRegion; }

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   GetUnoTunnelId() throw();
    static VCLXRegion*                                          GetImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxIFace );
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XRegion
     ::com::sun::star::awt::Rectangle       SAL_CALL getBounds() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL clear() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL unionRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL intersectRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL excludeRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL xOrRectangle( const ::com::sun::star::awt::Rectangle& rRect ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL unionRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL intersectRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL excludeRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     void                                   SAL_CALL xOrRegion( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >& rxRegion ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
     ::com::sun::star::uno::Sequence< ::com::sun::star::awt::Rectangle > SAL_CALL getRectangles() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

};



#endif // INCLUDED_TOOLKIT_AWT_VCLXREGION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
