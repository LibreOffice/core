/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SelectionMultiplex.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_COMPHELPER_SELECTION_MULTIPLEX_HXX
#define INCLUDED_COMPHELPER_SELECTION_MULTIPLEX_HXX

#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase1.hxx>
#include "comphelper/comphelperdllapi.h"

//=========================================================================
//= selection helper classes
//=========================================================================

//.........................................................................
namespace comphelper
{
//.........................................................................

    class OSelectionChangeMultiplexer;

    //==================================================================
    //= OSelectionChangeListener
    //==================================================================
    /// simple listener adapter for selections
    class COMPHELPER_DLLPUBLIC OSelectionChangeListener
    {
        friend class OSelectionChangeMultiplexer;

        OSelectionChangeMultiplexer*    m_pAdapter;
        ::osl::Mutex&                   m_rMutex;

    public:
        OSelectionChangeListener(::osl::Mutex& _rMutex)
            : m_pAdapter(NULL), m_rMutex(_rMutex) { }
        virtual ~OSelectionChangeListener();

        virtual void _selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource) throw( ::com::sun::star::uno::RuntimeException);

    protected:
        /** If the derivee also owns the mutex which we know as reference, then call this within your
            derivee's dtor.
        */
        void    disposeAdapter();

        // pseudo-private. Making it private now could break compatibility
        void    setAdapter( OSelectionChangeMultiplexer* _pAdapter );
    };

    //==================================================================
    //= OSelectionChangeMultiplexer
    //==================================================================
    /// multiplexer for selection changes
    class COMPHELPER_DLLPUBLIC OSelectionChangeMultiplexer  :public cppu::WeakImplHelper1< ::com::sun::star::view::XSelectionChangeListener>
    {
        friend class OSelectionChangeListener;
         ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier>  m_xSet;
        OSelectionChangeListener*                   m_pListener;
        sal_Int32                                   m_nLockCount;
        sal_Bool                                    m_bListening        : 1;
        sal_Bool                                    m_bAutoSetRelease   : 1;

        OSelectionChangeMultiplexer(const OSelectionChangeMultiplexer&);
        OSelectionChangeMultiplexer& operator=(const OSelectionChangeMultiplexer&);
    protected:
        virtual ~OSelectionChangeMultiplexer();
    public:
        OSelectionChangeMultiplexer(OSelectionChangeListener* _pListener, const  ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier>& _rxSet, sal_Bool _bAutoReleaseSet = sal_True);

    // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException);

    // XSelectionChangeListener
        virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        /// incremental lock
        void        lock();
        /// incremental unlock
        void        unlock();
        /// get the lock count
        sal_Int32   locked() const { return m_nLockCount; }

        void dispose();
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // INCLUDED_COMPHELPER_SELECTION_MULTIPLEX_HXX

