/*************************************************************************
 *
 *  $RCSfile: accessiblecontexthelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2002-04-26 05:51:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX
#define COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OAccessibleContextHelper
    //=====================================================================

    class OContextHelper_Impl;
    typedef ::cppu::WeakAggComponentImplHelper2 <   ::drafts::com::sun::star::accessibility::XAccessibleContext,
                                                    ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster
                                                >   OAccessibleContextHelper_Base;

    /** helper class for implementing an AccessibleContext
    */
    class OAccessibleContextHelper
                :public ::comphelper::OBaseMutex
                ,public OAccessibleContextHelper_Base
    {
    private:
        OContextHelper_Impl*    m_pImpl;

    protected:
        OAccessibleContextHelper( );
        ~OAccessibleContextHelper( );

        /** late construction
        @param _rxAccessible
            the Accessible object which created this context.
            <p>If your derived implementation implements the XAccessible (and does not follow the proposed
            separation of XAccessible from XAccessibleContext), you may pass <code>this</code> here.</p>

            <p>The object is hold weak, so it's life time is not affected.</p>

            <p>The object is needed for performance reasons: for <method>getAccessibleIndexInParent</method>,
            all children (which are XAccessible's theirself) of our parent have to be asked. If we know our
            XAccessible, we can compare it with all the children, instead of asking all children for their
            context and comparing this context with ourself.</p>
        */
        void lateInit( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible >& _rxAccessible );

    public:
        // XAccessibleEventBroadcaster
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleContext - still waiting to be overwritten
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException) = 0;

        // XAccessibleContext - default implementations
        /** default implementation for retrieving the index of this object within the parent
            <p>This basic implementation here returns the index <code>i</code> of the child for which
                <code>&lt;parent&gt;.getAccessibleChild( i )</code> equals our creator.</p>
        */
        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException);
        /** default implementation for retrieving the locale
            <p>This basic implementation returns the locale of the parent context,
            as retrieved via getAccessibleParent()->getAccessibleContext.</p>
        */
        virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException);

    public:
        // helper struct for granting selective access rights
        struct OAccessControl { friend class OContextEntryGuard; };

        // ensures that the object is alive
        inline  void            ensureAlive( const OAccessControl& _rAccessControl ) const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );
        inline  ::osl::Mutex&   GetMutex( const OAccessControl& _rAccessControl );

    protected:
        // OComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        // helper
        /** notifies all AccessibleEventListeners of a certain event

        @precond    not too be called with our mutex locked
        @param  _nEventId
            the id of the even. See AccessibleEventType
        @param  _rOldValue
            the old value to be notified
        @param  _rNewValue
            the new value to be notified
        */
        void SAL_CALL   NotifyAccessibleEvent(
                    const sal_Int16 _nEventId,
                    const ::com::sun::star::uno::Any& _rOldValue,
                    const ::com::sun::star::uno::Any& _rNewValue
                );

        // life time control
        /// checks whether the object is alive (returns <TRUE/> then) or disposed
        sal_Bool    isAlive() const;
        /// checks for beeing alive. If the object is already disposed (i.e. not alive), an exception is thrown.
        void        ensureAlive() const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) );

        /** ensures that the object is disposed.
        @precond
            to be called from within the destructor of your derived class only!
        */
        void        ensureDisposed( );

        /** shortcut for retrieving the context of the parent (returned by getAccessibleParent)
        */
        ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleContext >
                    implGetParentContext() SAL_THROW( ( ::com::sun::star::uno::RuntimeException ) );

        // access to the base class' broadcast helper/mutex
        ::cppu::OBroadcastHelper&       GetBroadcastHelper()        { return rBHelper; }
        const ::cppu::OBroadcastHelper& GetBroadcastHelper() const  { return rBHelper; }
        ::osl::Mutex&                   GetMutex()                  { return m_aMutex; }
    };

    //---------------------------------------------------------------------
    inline  void OAccessibleContextHelper::ensureAlive( const OAccessControl& _rAccessControl ) const SAL_THROW( ( ::com::sun::star::lang::DisposedException ) )
    {
        ensureAlive();
    }

    //---------------------------------------------------------------------
    inline  ::osl::Mutex& OAccessibleContextHelper::GetMutex( const OAccessControl& _rAccessControl )
    {
        return GetMutex();
    }

    //=====================================================================
    //= OContextEntryGuard
    //=====================================================================
    typedef ::osl::ClearableMutexGuard  OContextEntryGuard_Base;
    /** helper class for guarding the entry into OAccessibleContextHelper methods.

        <p>The class has two responsibilities:
        <ul><li>it locks the mutex of an OAccessibleContextHelper instance, as long as the guard lives</li>
            <li>it checks if an given OAccessibleContextHelper instance is alive, else an exception is thrown
                our of the constructor of the guard</li>
        </ul>
        <br/>
        This makes it your first choice (hopefully :) for guarding any interface method implementations of
        you derived class.
        </p>
    */
    class OContextEntryGuard : public OContextEntryGuard_Base
    {
    public:
        /** constructs the guard

            <p>The given context (it's mutex, respectively) is locked, and an exception is thrown if the context
            is not alive anymore. In the latter case, of course, the mutex is freed, again.</p>

        @param _pContext
            the context which shall be guarded
        @precond <arg>_pContext</arg> != NULL
        */
        inline OContextEntryGuard( OAccessibleContextHelper* _pContext );

        /** destructs the guard.
            <p>The context (it's mutex, respectively) is unlocked.</p>
        */
        inline ~OContextEntryGuard();
    };

    //.....................................................................
    inline OContextEntryGuard::OContextEntryGuard( OAccessibleContextHelper* _pContext  )
        :OContextEntryGuard_Base( _pContext->GetMutex( OAccessibleContextHelper::OAccessControl() ) )
    {
        _pContext->ensureAlive( OAccessibleContextHelper::OAccessControl() );
    }

    //.....................................................................
    inline OContextEntryGuard::~OContextEntryGuard()
    {
    }


//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_ACCESSIBLE_CONTEXT_HELPER_HXX

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2002/04/23 11:07:38  fs
 *  initial checkin - helper for implementing an XAccessibleContext
 *
 *
 *  Revision 1.0 17.04.2002 15:47:54  fs
 ************************************************************************/

