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

#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>

#include <cppuhelper/implbase.hxx>
#include <unotools/weakref.hxx>

namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XURLTransformer; }

namespace dbaccess
{
    class ODatabaseDocument;

    // DocumentEventExecutor
    typedef ::cppu::WeakImplHelper <   css::document::XDocumentEventListener
                                    >   DocumentEventExecutor_Base;
    class DocumentEventExecutor : public DocumentEventExecutor_Base
    {
    public:
        DocumentEventExecutor(
            const css::uno::Reference< css::uno::XComponentContext >& _rContext,
            const rtl::Reference< ODatabaseDocument >& _rxDocument );

    protected:
        virtual ~DocumentEventExecutor() override;

        // css.document.XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) override;
        // css.lang.XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    private:
        unotools::WeakReference< ODatabaseDocument > mxDocument;
        css::uno::Reference< css::util::XURLTransformer > mxURLTransformer;
    };

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
