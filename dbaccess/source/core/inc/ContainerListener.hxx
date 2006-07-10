/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContainerListener.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:11:47 $
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
#ifndef DBA_CONTAINERLISTENER_HXX
#define DBA_CONTAINERLISTENER_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERAPPROVELISTENER_HPP_
#include <com/sun/star/container/XContainerApproveListener.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

    //==========================================================================
    //= OContainerListener
    //==========================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::container::XContainerListener
                                    ,   ::com::sun::star::container::XContainerApproveListener
                                    >   OContainerListener_BASE;

    /** is helper class to avoid a cycle in refcount
    */
    class OContainerListener : public OContainerListener_BASE
    {
        ::osl::Mutex&       m_rMutex;
        OWeakObject&        m_rDestination;
        bool                m_bDisposed;

    public:
        OContainerListener( OWeakObject& _rDestination, ::osl::Mutex& _rMutex )
            :m_rMutex( _rMutex )
            ,m_rDestination( _rDestination )
            ,m_bDisposed( false )
        {
        }

        // XContainerApproveListener
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveInsertElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveReplaceElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveRemoveElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL dispose()
        {
            m_bDisposed = true;
        }

    protected:
        virtual ~OContainerListener();
    };
//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // DBA_CONTAINERLISTENER_HXX

