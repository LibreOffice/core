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

#ifndef INCLUDED_COMPHELPER_PROPMULTIPLEX_HXX
#define INCLUDED_COMPHELPER_PROPMULTIPLEX_HXX

#include <comphelper/propstate.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>


//= property helper classes



namespace comphelper
{


    class OPropertyChangeMultiplexer;


    //= OPropertyChangeListener

    /// simple listener adapter for property sets
    class COMPHELPER_DLLPUBLIC OPropertyChangeListener
    {
        friend class OPropertyChangeMultiplexer;

        OPropertyChangeMultiplexer* m_pAdapter;
        ::osl::Mutex&               m_rMutex;

    public:
        OPropertyChangeListener(::osl::Mutex& _rMutex)
            : m_pAdapter(NULL), m_rMutex(_rMutex) { }
        virtual ~OPropertyChangeListener();

        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent)
            throw (css::uno::RuntimeException, std::exception) = 0;
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource)
            throw( ::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        /** If the derivee also owns the mutex which we know as reference, then call this within your
            derivee's dtor.
        */
        void    disposeAdapter();

        // pseudo-private. Making it private now could break compatibility
        void    setAdapter( OPropertyChangeMultiplexer* _pAdapter );
    };


    //= OPropertyChangeMultiplexer

    /// multiplexer for property changes
    class COMPHELPER_DLLPUBLIC OPropertyChangeMultiplexer   :public cppu::WeakImplHelper< ::com::sun::star::beans::XPropertyChangeListener>
    {
        friend class OPropertyChangeListener;
         ::com::sun::star::uno::Sequence< OUString >     m_aProperties;
         ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>   m_xSet;
        OPropertyChangeListener*                    m_pListener;
        sal_Int32                                   m_nLockCount;
        bool                                    m_bListening        : 1;
        bool                                    m_bAutoSetRelease   : 1;


        virtual ~OPropertyChangeMultiplexer();
    public:
        OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxSet, bool _bAutoReleaseSet = true);

    // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

        /// incremental lock
        void        lock();
        /// incremental unlock
        void        unlock();
        /// get the lock count
        sal_Int32   locked() const { return m_nLockCount; }

        void addProperty(const OUString& aPropertyName);
        void dispose();
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPMULTIPLEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
