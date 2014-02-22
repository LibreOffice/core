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

#include <com/sun/star/util/XCloseable.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace utl
{
//......................................................................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::util::XCloseable;
    using ::com::sun::star::util::XCloseListener;
    using ::com::sun::star::util::CloseVetoException;
    using ::com::sun::star::lang::EventObject;

    //==================================================================================================================
    //= CloseListener_Impl
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1 <   XCloseListener
                                    >   CloseListener_Base;
    class CloseListener_Impl : public CloseListener_Base
    {
    public:
        CloseListener_Impl()
            :m_bHasOwnership( false )
        {
        }

        // XCloseListener
        virtual void SAL_CALL queryClosing( const EventObject& Source, ::sal_Bool GetsOwnership ) throw (CloseVetoException, RuntimeException);
        virtual void SAL_CALL notifyClosing( const EventObject& Source ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source) throw (RuntimeException);

        bool hasOwnership() const { return m_bHasOwnership; }

    protected:
        ~CloseListener_Impl()
        {
        }

    private:
        bool    m_bHasOwnership;
    };


    void SAL_CALL CloseListener_Impl::queryClosing( const EventObject& i_source, ::sal_Bool i_deliverOwnership ) throw (CloseVetoException, RuntimeException)
    {
        (void)i_source;

        if ( !m_bHasOwnership )
            m_bHasOwnership = i_deliverOwnership;

        throw CloseVetoException();
    }


    void SAL_CALL CloseListener_Impl::notifyClosing( const EventObject& i_source ) throw (RuntimeException)
    {
        (void)i_source;
    }


    void SAL_CALL CloseListener_Impl::disposing( const EventObject& i_source ) throw (RuntimeException)
    {
        (void)i_source;
    }

    //==================================================================================================================
    //= CloseVeto_Data
    //==================================================================================================================
    struct CloseVeto_Data
    {
        Reference< XCloseable >                 xCloseable;
        ::rtl::Reference< CloseListener_Impl >  pListener;
    };

    //==================================================================================================================
    //= operations
    //==================================================================================================================
    namespace
    {

        void lcl_init( CloseVeto_Data& i_data, const Reference< XInterface >& i_closeable )
        {
            i_data.xCloseable.set( i_closeable, UNO_QUERY );
            ENSURE_OR_RETURN_VOID( i_data.xCloseable.is(), "CloseVeto: the component is not closeable!" );

            i_data.pListener = new CloseListener_Impl;
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
                    i_data.xCloseable->close( sal_True );
                }
                catch( const CloseVetoException& ) { }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
        }
    }

    //==================================================================================================================
    //= CloseVeto
    //==================================================================================================================

    CloseVeto::CloseVeto( const Reference< XInterface >& i_closeable )
        :m_pData( new CloseVeto_Data )
    {
        lcl_init( *m_pData, i_closeable );
    }


    CloseVeto::~CloseVeto()
    {
        lcl_deinit( *m_pData );
    }

//......................................................................................................................
} // namespace utl
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
