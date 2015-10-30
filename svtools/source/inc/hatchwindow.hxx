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

#ifndef INCLUDED_SVTOOLS_SOURCE_INC_HATCHWINDOW_HXX
#define INCLUDED_SVTOOLS_SOURCE_INC_HATCHWINDOW_HXX

#include <com/sun/star/embed/XHatchWindow.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/typeprovider.hxx>

class SvResizeWindow;
class VCLXHatchWindow : public css::embed::XHatchWindow,
                        public VCLXWindow
{
    css::uno::Reference< css::embed::XHatchWindowController > m_xController;
    css::awt::Size aHatchBorderSize;
    VclPtr<SvResizeWindow> pHatchWindow;

public:
    VCLXHatchWindow();
    virtual ~VCLXHatchWindow();

    void initializeWindow( const css::uno::Reference< css::awt::XWindowPeer >& xParent,
                const css::awt::Rectangle& aBounds,
                const css::awt::Size& aSize );

    void QueryObjAreaPixel( Rectangle & );
    void RequestObjAreaPixel( const Rectangle & );
    void InplaceDeactivate();
    void Activated();
    void Deactivated();

    // XInterface
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL acquire() throw() override;
    void SAL_CALL release() throw() override;

    // XTypeProvider
    css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // XHatchWindow
    virtual void SAL_CALL setController( const css::uno::Reference< css::embed::XHatchWindowController >& xController ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getHatchBorderSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setHatchBorderSize( const css::awt::Size& _hatchbordersize ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SVTOOLS_SOURCE_INC_HATCHWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
