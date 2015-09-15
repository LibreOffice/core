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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTEXECUTOR_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTEXECUTOR_HXX

#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace dbaccess
{

    struct DocumentEventExecutor_Data;
    // DocumentEventExecutor
    typedef ::cppu::WeakImplHelper <   css::document::XDocumentEventListener
                                    >   DocumentEventExecutor_Base;
    class DocumentEventExecutor : public DocumentEventExecutor_Base
    {
    public:
        DocumentEventExecutor(
            const css::uno::Reference< css::uno::XComponentContext >& _rContext,
            const css::uno::Reference< css::document::XEventsSupplier >& _rxDocument );

    protected:
        virtual ~DocumentEventExecutor();

        // css.document.XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // css.lang.XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    private:
        ::std::unique_ptr< DocumentEventExecutor_Data >   m_pData;
    };

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTEXECUTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
