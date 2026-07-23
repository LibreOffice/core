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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>
#include <mutex>
#include <utility>
#include <vector>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvasFactory.hpp>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/functional.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <comphelper/configuration.hxx>
#include <canvas.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;


namespace
{

class CanvasFactory
    : public ::cppu::WeakImplHelper< lang::XServiceInfo,
                                      rendering::XCanvasFactory >
{
    mutable std::mutex                m_mutex;
    Reference<XComponentContext>      m_xContext;

public:
    virtual ~CanvasFactory() override;
    explicit CanvasFactory( Reference<XComponentContext> const & xContext );

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( OUString const & serviceName ) override;
    virtual Sequence<OUString> getSupportedServiceNames() override;
    
    virtual Reference< rendering::XCanvas > create( sal_Int64 nOutDev ) override;
};

CanvasFactory::CanvasFactory( Reference<XComponentContext> const & xContext ) :
    m_xContext(xContext)
{
}

CanvasFactory::~CanvasFactory()
{
}


// XServiceInfo
OUString CanvasFactory::getImplementationName()
{
    return u"com.sun.star.comp.rendering.CanvasFactory"_ustr;
}

bool CanvasFactory::supportsService( OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

Sequence<OUString> CanvasFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.rendering.CanvasFactory"_ustr };
}

Reference< rendering::XCanvas > CanvasFactory::create( sal_Int64 nOutDev )
{
    return new vclcanvas::Canvas(nOutDev);
}

} // anon namespace


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_CanvasFactory_get_implementation(css::uno::XComponentContext* context,
                                                             cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new CanvasFactory(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
