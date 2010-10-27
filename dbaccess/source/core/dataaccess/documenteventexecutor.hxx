/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef DBACCESS_DOCUMENTEVENTEXECUTOR_HXX
#define DBACCESS_DOCUMENTEVENTEXECUTOR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

namespace comphelper
{
    class ComponentContext;
}

namespace dbaccess
{

    struct DocumentEventExecutor_Data;
    //====================================================================
    //= DocumentEventExecutor
    //====================================================================
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::document::XDocumentEventListener
                                    >   DocumentEventExecutor_Base;
    class DocumentEventExecutor : public DocumentEventExecutor_Base
    {
    public:
        DocumentEventExecutor(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& _rxDocument );

    protected:
        virtual ~DocumentEventExecutor();

        // css.document.XDocumentEventListener
        virtual void SAL_CALL documentEventOccured( const ::com::sun::star::document::DocumentEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
        // css.lang.XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ::std::auto_ptr< DocumentEventExecutor_Data >   m_pData;
    };

} // namespace dbaccess

#endif // DBACCESS_DOCUMENTEVENTEXECUTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
