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

#pragma once


#include <com/sun/star/awt/XRegion.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <mutex>

#include <vcl/region.hxx>




class VCLXRegion final : public cppu::WeakImplHelper<
                            css::awt::XRegion,
                            css::lang::XUnoTunnel>
{
    std::mutex    maMutex;
    vcl::Region          maRegion;

public:
                    VCLXRegion();
                    virtual ~VCLXRegion() override;

    const vcl::Region&   GetRegion() const                   { return maRegion; }

    // css::lang::XUnoTunnel
    UNO3_GETIMPLEMENTATION_DECL(VCLXRegion)

    // css::awt::XRegion
     css::awt::Rectangle       SAL_CALL getBounds() override;
     void                                   SAL_CALL clear() override;
     void                                   SAL_CALL move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) override;
     void                                   SAL_CALL unionRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   SAL_CALL intersectRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   SAL_CALL excludeRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   SAL_CALL xOrRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   SAL_CALL unionRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   SAL_CALL intersectRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   SAL_CALL excludeRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   SAL_CALL xOrRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     css::uno::Sequence< css::awt::Rectangle > SAL_CALL getRectangles() override;

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
