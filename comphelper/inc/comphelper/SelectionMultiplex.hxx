/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SelectionMultiplex.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 10:17:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_COMPHELPER_SELECTION_MULTIPLEX_HXX
#define INCLUDED_COMPHELPER_SELECTION_MULTIPLEX_HXX

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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

