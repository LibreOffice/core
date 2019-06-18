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

#ifndef INCLUDED_VBAHELPER_VBAWINDOWBASE_HXX
#define INCLUDED_VBAHELPER_VBAWINDOWBASE_HXX

#include <exception>

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <ooo/vba/XWindowBase.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vbahelper/vbadllapi.h>
#include <vbahelper/vbahelper.hxx>
#include <vbahelper/vbahelperinterface.hxx>

namespace com { namespace sun { namespace star {
    namespace awt { class XWindow2; }
    namespace awt { class XWindow; }
    namespace frame { class XController; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace ooo { namespace vba {
    class XHelperInterface;
} }

typedef InheritedHelperInterfaceWeakImpl< ov::XWindowBase > WindowBaseImpl_BASE;

class VBAHELPER_DLLPUBLIC VbaWindowBase : public WindowBaseImpl_BASE
{
public:
    /// @throws css::uno::RuntimeException
    VbaWindowBase(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XModel >& xModel,
        const css::uno::Reference< css::frame::XController >& xController );
    /// @throws css::uno::RuntimeException
    VbaWindowBase(
        css::uno::Sequence< css::uno::Any > const& aArgs,
        css::uno::Reference< css::uno::XComponentContext > const& xContext );

    // XWindowBase
    virtual sal_Int32 SAL_CALL getHeight() override ;
    virtual void SAL_CALL setHeight( sal_Int32 _height ) override ;
    virtual sal_Int32 SAL_CALL getLeft() override ;
    virtual void SAL_CALL setLeft( sal_Int32 _left ) override ;
    virtual sal_Int32 SAL_CALL getTop() override ;
    virtual void SAL_CALL setTop( sal_Int32 _top ) override ;
    virtual sal_Bool SAL_CALL getVisible() override;
    virtual void SAL_CALL setVisible( sal_Bool _visible ) override;
    virtual sal_Int32 SAL_CALL getWidth() override ;
    virtual void SAL_CALL setWidth( sal_Int32 _width ) override ;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

protected:
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::frame::XController > getController() const;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::awt::XWindow > getWindow() const;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::awt::XWindow2 > getWindow2() const;

    css::uno::Reference< css::frame::XModel > m_xModel;

private:
    /// @throws css::uno::RuntimeException
    void construct( const css::uno::Reference< css::frame::XController >& xController );

    css::uno::WeakReference< css::frame::XController > m_xController;
    css::uno::WeakReference< css::awt::XWindow > m_xWindow;
};

#endif // INCLUDED_VBAHELPER_VBAWINDOWBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
