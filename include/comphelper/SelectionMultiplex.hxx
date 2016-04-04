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

#ifndef INCLUDED_COMPHELPER_SELECTIONMULTIPLEX_HXX
#define INCLUDED_COMPHELPER_SELECTIONMULTIPLEX_HXX

#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>


//= selection helper classes


namespace comphelper
{


    class OSelectionChangeMultiplexer;


    //= OSelectionChangeListener

    /// simple listener adapter for selections
    class COMPHELPER_DLLPUBLIC OSelectionChangeListener
    {
        friend class OSelectionChangeMultiplexer;

        OSelectionChangeMultiplexer*    m_pAdapter;
        ::osl::Mutex&                   m_rMutex;

    public:
        OSelectionChangeListener(::osl::Mutex& _rMutex)
            : m_pAdapter(nullptr), m_rMutex(_rMutex) { }
        virtual ~OSelectionChangeListener();

        virtual void _selectionChanged( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException) = 0;
        virtual void _disposing(const css::lang::EventObject& _rSource)
            throw (css::uno::RuntimeException, std::exception);

    protected:
        // pseudo-private. Making it private now could break compatibility
        void    setAdapter( OSelectionChangeMultiplexer* _pAdapter );
    };


    //= OSelectionChangeMultiplexer

    /// multiplexer for selection changes
    class COMPHELPER_DLLPUBLIC OSelectionChangeMultiplexer  :public cppu::WeakImplHelper< css::view::XSelectionChangeListener>
    {
        friend class OSelectionChangeListener;
        css::uno::Reference< css::view::XSelectionSupplier>  m_xSet;
        OSelectionChangeListener*                            m_pListener;
        sal_Int32                                            m_nLockCount;
        bool                                                 m_bListening        : 1;

        OSelectionChangeMultiplexer(const OSelectionChangeMultiplexer&) = delete;
        OSelectionChangeMultiplexer& operator=(const OSelectionChangeMultiplexer&) = delete;
    protected:
        virtual ~OSelectionChangeMultiplexer();
    public:
        OSelectionChangeMultiplexer(OSelectionChangeListener* _pListener, const  css::uno::Reference< css::view::XSelectionSupplier>& _rxSet);

    // XEventListener
        virtual void SAL_CALL disposing( const  css::lang::EventObject& Source ) throw( css::uno::RuntimeException, std::exception) override;

    // XSelectionChangeListener
        virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

        /// incremental lock
        void        lock();
        /// incremental unlock
        void        unlock();
        /// get the lock count
        sal_Int32   locked() const { return m_nLockCount; }

        void dispose();
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_SELECTIONMULTIPLEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
