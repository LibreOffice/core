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

#include <memory>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/GraphicObject.hxx>
#include <mutex>

using namespace css;

namespace {

typedef ::cppu::WeakImplHelper<graphic::XGraphicObject, css::lang::XServiceInfo> GraphicObject_BASE;

 // Simple uno wrapper around the GraphicObject class to allow basic
 // access. ( and solves a horrible cyclic link problem between
 // goodies/toolkit/extensions )
class GraphicObjectImpl : public GraphicObject_BASE
{
     std::mutex m_aMutex;
     std::optional<GraphicObject> mpGraphicObject;

public:
    /// @throws uno::RuntimeException
    explicit GraphicObjectImpl(uno::Sequence<uno::Any> const & rArgs);

     // XGraphicObject
    virtual uno::Reference<graphic::XGraphic> SAL_CALL getGraphic() override;
    virtual void SAL_CALL setGraphic(uno::Reference<graphic::XGraphic> const & rxGraphic) override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.graphic.GraphicObject"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.graphic.GraphicObject"_ustr };
    }
};

GraphicObjectImpl::GraphicObjectImpl(const uno::Sequence<uno::Any>& /*rArgs*/)
{
    mpGraphicObject.emplace();
}

uno::Reference<graphic::XGraphic> SAL_CALL GraphicObjectImpl::getGraphic()
{
    std::scoped_lock aGuard(m_aMutex);

    if (!mpGraphicObject)
        throw uno::RuntimeException();
    return mpGraphicObject->GetGraphic().GetXGraphic();
}

void SAL_CALL GraphicObjectImpl::setGraphic(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    std::scoped_lock aGuard(m_aMutex);

    if (!mpGraphicObject)
        throw uno::RuntimeException();
    Graphic aGraphic(rxGraphic);
    mpGraphicObject->SetGraphic(aGraphic);
}

} // end anonymous namespace

extern "C" SAL_DLLPUBLIC_EXPORT
css::uno::XInterface* com_sun_star_graphic_GraphicObject_get_implementation(
                            SAL_UNUSED_PARAMETER uno::XComponentContext*,
                            uno::Sequence<uno::Any> const & rArguments)
{
    return cppu::acquire(new GraphicObjectImpl(rArguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
