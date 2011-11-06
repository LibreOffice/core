/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

