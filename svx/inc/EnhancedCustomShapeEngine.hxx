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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/XCustomShapeHandle.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>

class SVXCORE_DLLPUBLIC EnhancedCustomShapeEngine
    : public cppu::WeakImplHelper<css::lang::XServiceInfo, css::drawing::XCustomShapeEngine>
{
    /// Cannot use a rtl::Reference here because EnhancedCustomShapeEngine is owned by
    /// SdrObjCustomShape, so this is basically a parent pointer.
    SdrObjCustomShape* mpCustomShape;
    bool mbForceGroupWithText;

    static rtl::Reference<SdrObject> ImplForceGroupWithText(SdrObjCustomShape& rSdrObjCustomShape,
                                                            SdrObject* pRenderedShape);

public:
    EnhancedCustomShapeEngine(const css::uno::Sequence<css::uno::Any>& aArguments);
    EnhancedCustomShapeEngine(SdrObjCustomShape& rShape);

    // XInterface
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XCustomShapeEngine
    virtual css::uno::Reference<css::drawing::XShape> SAL_CALL render() override;

    std::vector<css::uno::Reference<css::drawing::XCustomShapeHandle>> getInteraction();
    tools::Rectangle getTextBounds() const;
    basegfx::B2DPolyPolygon getB2DLineGeometry() const;
    rtl::Reference<SdrObject> render2() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
