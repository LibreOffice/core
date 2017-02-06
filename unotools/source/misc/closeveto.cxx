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

#include <unotools/closeveto.hxx>

#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

namespace utl
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::XCloseListener;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::lang::EventObject;

    //= CloseListener_Impl

    typedef ::cppu::WeakImplHelper <   XCloseListener
                                    >   CloseListener_Base;
    class CloseListener_Impl : public CloseListener_Base
    {
    public:
        explicit CloseListener_Impl(bool const bHasOwnership)
            : m_bHasOwnership(bHasOwnership)
        {
        }

        // XCloseListener
        virtual void SAL_CALL queryClosing( const EventObject& Source, sal_Bool GetsOwnership ) override;
        virtual void SAL_CALL notifyClosing( const EventObject& Source ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source) override;

        bool hasOwnership() const { return m_bHasOwnership; }

    protected:
        virtual ~CloseListener_Impl() override
        {
        }

    private:
        bool    m_bHasOwnership;
    };

    void SAL_CALL CloseListener_Impl::queryClosing( const EventObject& i_source, sal_Bool i_deliverOwnership )
    {
        (void)i_source;

        if ( !m_bHasOwnership )
            m_bHasOwnership = i_deliverOwnership;

        throw CloseVetoException();
    }

    void SAL_CALL CloseListener_Impl::notifyClosing( const EventObject& i_source )
    {
        (void)i_source;
    }

    void SAL_CALL CloseListener_Impl::disposing( const EventObject& i_source )
    {
        (void)i_source;
    }

    //= CloseVeto_Data

    struct CloseVeto_Data
    {
        Reference< XCloseable >                 xCloseable;
        ::rtl::Reference< CloseListener_Impl >  pListener;
    };

    //= operations

    namespace
    {

        void lcl_init( CloseVeto_Data& i_data, const Reference< XInterface >& i_closeable,
                bool const hasOwnership)
        {
            i_data.xCloseable.set( i_closeable, UNO_QUERY );
            ENSURE_OR_RETURN_VOID( i_data.xCloseable.is(), "CloseVeto: the component is not closeable!" );

            i_data.pListener = new CloseListener_Impl(hasOwnership);
            i_data.xCloseable->addCloseListener( i_data.pListener.get() );
        }

        void lcl_deinit( CloseVeto_Data& i_data )
        {
            if ( !i_data.xCloseable.is() )
                return;

            i_data.xCloseable->removeCloseListener( i_data.pListener.get() );
            if ( i_data.pListener->hasOwnership() )
            {
                try
                {
                    i_data.xCloseable->close( true );
                }
                catch( const CloseVetoException& ) { }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    //= CloseVeto
    CloseVeto::CloseVeto(const Reference< XInterface >& i_closeable,
            bool const hasOwnership)
        : m_xData(new CloseVeto_Data)
    {
        lcl_init(*m_xData, i_closeable, hasOwnership);
    }

    CloseVeto::~CloseVeto()
    {
        lcl_deinit(*m_xData);
    }

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
