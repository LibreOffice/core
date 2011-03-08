/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_DOCUMENT_RENDERER_HXX
#define SD_DOCUMENT_RENDERER_HXX

#include "ViewShellBase.hxx"

#include <com/sun/star/view/XRenderable.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        ::com::sun::star::view::XRenderable
        > DocumentRendererInterfaceBase;
}


class DocumentRenderer
    : protected ::cppu::BaseMutex,
      public DocumentRendererInterfaceBase
{
public:
    DocumentRenderer (ViewShellBase& rBase);
    virtual ~DocumentRenderer (void);

    // XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount (
        const css::uno::Any& aSelection,
        const css::uno::Sequence<css::beans::PropertyValue >& xOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getRenderer (
        sal_Int32 nRenderer,
        const css::uno::Any& rSelection,
        const css::uno::Sequence<css::beans::PropertyValue>& rxOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

    virtual void SAL_CALL render (
        sal_Int32 nRenderer,
        const css::uno::Any& rSelection,
        const css::uno::Sequence<css::beans::PropertyValue>& rxOptions)
        throw (css::lang::IllegalArgumentException, css::uno::RuntimeException);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImpl;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
