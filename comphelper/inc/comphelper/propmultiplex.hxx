/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propmultiplex.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#define _COMPHELPER_PROPERTY_MULTIPLEX_HXX_

#include <comphelper/propstate.hxx>
#include <cppuhelper/implbase1.hxx>
#include "comphelper/comphelperdllapi.h"

//=========================================================================
//= property helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

    class OPropertyChangeMultiplexer;

    //==================================================================
    //= OPropertyChangeListener
    //==================================================================
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

        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException) = 0;
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource) throw( ::com::sun::star::uno::RuntimeException);

    protected:
        /** If the derivee also owns the mutex which we know as reference, then call this within your
            derivee's dtor.
        */
        void    disposeAdapter();

        // pseudo-private. Making it private now could break compatibility
        void    setAdapter( OPropertyChangeMultiplexer* _pAdapter );
    };

    //==================================================================
    //= OPropertyChangeMultiplexer
    //==================================================================
    /// multiplexer for property changes
    class COMPHELPER_DLLPUBLIC OPropertyChangeMultiplexer   :public cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener>
    {
        friend class OPropertyChangeListener;
         ::com::sun::star::uno::Sequence< ::rtl::OUString >     m_aProperties;
         ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>   m_xSet;
        OPropertyChangeListener*                    m_pListener;
        sal_Int32                                   m_nLockCount;
        sal_Bool                                    m_bListening        : 1;
        sal_Bool                                    m_bAutoSetRelease   : 1;


        virtual ~OPropertyChangeMultiplexer();
    public:
        OPropertyChangeMultiplexer(OPropertyChangeListener* _pListener, const  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxSet, sal_Bool _bAutoReleaseSet = sal_True);

    // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw( ::com::sun::star::uno::RuntimeException);

        /// incremental lock
        void        lock();
        /// incremental unlock
        void        unlock();
        /// get the lock count
        sal_Int32   locked() const { return m_nLockCount; }

        void addProperty(const ::rtl::OUString& aPropertyName);
        void dispose();
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // _COMPHELPER_PROPERTY_MULTIPLEX_HXX_

