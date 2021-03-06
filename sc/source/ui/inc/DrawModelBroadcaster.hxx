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

#include <svl/lstner.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XShapeEventBroadcaster.hpp>
#include <unordered_map>

class SdrModel;

class ScDrawModelBroadcaster : public SfxListener,
    public ::cppu::WeakImplHelper< css::document::XShapeEventBroadcaster >
{
    mutable ::osl::Mutex maListenerMutex;
    ::comphelper::OInterfaceContainerHelper3<css::document::XEventListener> maEventListeners;
    std::unordered_map<css::uno::Reference< css::drawing::XShape >, css::uno::Reference< css::document::XShapeEventListener >> maShapeListeners;
    SdrModel *mpDrawModel;

public:

    ScDrawModelBroadcaster( SdrModel *pDrawModel );
    virtual ~ScDrawModelBroadcaster() override;

    // css::document::XEventBroadcaster
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener ) override;
    // css::document::XShapeEventBroadcaster
    virtual void SAL_CALL addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::document::XShapeEventListener >& xListener ) override;
    virtual void SAL_CALL removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape, const css::uno::Reference< css::document::XShapeEventListener >& xListener ) override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
