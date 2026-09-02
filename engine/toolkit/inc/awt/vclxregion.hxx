/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <mutex>

#include <vcl/region.hxx>




class VCLXRegion final : public cppu::WeakImplHelper<
                            css::awt::XRegion>
{
    std::mutex    maMutex;
    vcl::Region          maRegion;

public:
                    VCLXRegion();
                    virtual ~VCLXRegion() override;

    const vcl::Region&   GetRegion() const                   { return maRegion; }

    // css::awt::XRegion
     css::awt::Rectangle       getBounds() override;
     void                                   clear() override;
     void                                   move( sal_Int32 nHorzMove, sal_Int32 nVertMove ) override;
     void                                   unionRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   intersectRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   excludeRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   xOrRectangle( const css::awt::Rectangle& rRect ) override;
     void                                   unionRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   intersectRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   excludeRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     void                                   xOrRegion( const css::uno::Reference< css::awt::XRegion >& rxRegion ) override;
     cpo::uno::Sequence< css::awt::Rectangle > getRectangles() override;

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
