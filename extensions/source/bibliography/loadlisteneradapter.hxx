/*************************************************************************
 *
 *  $RCSfile: loadlisteneradapter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:02 $
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

#ifndef EXTENSIONS_BIB_LOADLISTENERADAPTER_HXX
#define EXTENSIONS_BIB_LOADLISTENERADAPTER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

//.........................................................................
namespace bib
{
//.........................................................................

    //=====================================================================
    //= OComponentListener
    //=====================================================================
    class OComponentListener
    {
        friend class OComponentAdapterBase;

    private:
        OComponentAdapterBase*  m_pAdapter;
        ::osl::Mutex&           m_rMutex;

    protected:
        OComponentListener( ::osl::Mutex& _rMutex )
            :m_rMutex( _rMutex )
            ,m_pAdapter( NULL )
        {
        }

        virtual ~OComponentListener();

        // XEventListener equivalents
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw( ::com::sun::star::uno::RuntimeException );

    protected:
        void setAdapter( OComponentAdapterBase* _pAdapter );
    };

    //=====================================================================
    //= OComponentAdapterBase
    //=====================================================================
    class OComponentAdapterBase
    {
        friend class OComponentListener;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                                            m_xComponent;
        OComponentListener*                 m_pListener;
        sal_Int32                           m_nLockCount;
        sal_Bool                            m_bListening    : 1;
        sal_Bool                            m_bAutoRelease  : 1;

        // impl method for dispose - virtual, 'cause you at least need to remove the listener from the broadcaster
        virtual void disposing() = 0;

    protected:
        // attribute access for derivees
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >&
                                getComponent() const    { return m_xComponent; }
        OComponentListener*     getListener()           { return m_pListener; }

        // to be called by derivees which started listening at the component
        virtual void    startComponentListening() = 0;

        virtual ~OComponentAdapterBase();

    public:
        OComponentAdapterBase(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& _rxComp,
            sal_Bool _bAutoRelease = sal_True
        );

        // late construction
        // can be called from within you ctor, to have you're object fully initialized at the moment of
        // the call (which would not be the case when calling this ctor)
        void Init( OComponentListener* _pListener );

    // base for ref-counting, implemented by OComponentAdapter
        virtual void SAL_CALL acquire(  ) throw () = 0;
        virtual void SAL_CALL release(  ) throw () = 0;

    // helper
        /// incremental lock
        void        lock();
        /// incremental unlock
        void        unlock();
        /// get the lock count
        sal_Int32   locked() const { return m_nLockCount; }

        /// dispose the object - stop listening and such
        void dispose();

    protected:
    // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException);
    };

    //=====================================================================
    //= OLoadListener
    //=====================================================================
    class OLoadListener : public OComponentListener
    {
        friend class OLoadListenerAdapter;

    protected:
        OLoadListener( ::osl::Mutex& _rMutex ) : OComponentListener( _rMutex ) { }

    // XLoadListener equivalents
        virtual void _loaded( const ::com::sun::star::lang::EventObject& aEvent ) = 0;
        virtual void _unloading( const ::com::sun::star::lang::EventObject& aEvent ) = 0;
        virtual void _unloaded( const ::com::sun::star::lang::EventObject& aEvent ) = 0;
        virtual void _reloading( const ::com::sun::star::lang::EventObject& aEvent ) = 0;
        virtual void _reloaded( const ::com::sun::star::lang::EventObject& aEvent ) = 0;
    };

    //=====================================================================
    //= OLoadListenerAdapter
    //=====================================================================
    typedef ::cppu::WeakImplHelper1< ::com::sun::star::form::XLoadListener >    OLoadListenerAdapter_Base;
    class OLoadListenerAdapter
        :public OLoadListenerAdapter_Base
        ,public OComponentAdapterBase
    {
    protected:
        OLoadListener*      getLoadListener( )  { return static_cast< OLoadListener* >( getListener() ); }

    protected:
        virtual void    disposing();
        virtual void    startComponentListening();

    public:
        OLoadListenerAdapter(
            const ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >& _rxLoadable,
            sal_Bool _bAutoRelease = sal_True
        );


        virtual void SAL_CALL acquire(  ) throw ();
        virtual void SAL_CALL release(  ) throw ();

    protected:
    // XEventListener
        virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& _rSource ) throw( ::com::sun::star::uno::RuntimeException);

    // XLoadListener
        virtual void SAL_CALL loaded( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unloading( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL unloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reloading( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL reloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace bib
//.........................................................................

#endif // EXTENSIONS_BIB_LOADLISTENERADAPTER_HXX

