/*************************************************************************
 *
 *  $RCSfile: connector.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: svesik $ $Date: 2000-11-23 14:41:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/connection/XConnectionBroadcaster.hpp>

#include <hash_set>

#ifndef _VOS_SOCKET_HXX_
#       include <vos/socket.hxx>
#endif
#ifndef _VOS_PIPE_HXX_
#       include <vos/pipe.hxx>
#endif

namespace stoc_connector
{
    template<class T>
    struct ReferenceHash
    {
        size_t operator () (const ::com::sun::star::uno::Reference<T> & ref) const
        {
            return (size_t)ref.get();
        }
    };

    template<class T>
    struct ReferenceEqual
    {
        sal_Bool operator () (const ::com::sun::star::uno::Reference<T> & op1,
                              const ::com::sun::star::uno::Reference<T> & op2) const
        {
            return op1.get() == op2.get();
        }
    };

    typedef ::std::hash_set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>,
                             ReferenceHash< ::com::sun::star::io::XStreamListener>,
                             ReferenceEqual< ::com::sun::star::io::XStreamListener> >
            XStreamListener_hash_set;

    class PipeConnection :
        public ::cppu::WeakImplHelper1< ::com::sun::star::connection::XConnection >

    {
    public:
        PipeConnection( const ::rtl::OUString & s , sal_Bool bIgnoreClose );

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);
    public:
        ::vos::OStreamPipe m_pipe;
        oslInterlockedCount m_nStatus;
        ::rtl::OUString m_sDescription;
        sal_Bool m_bIgnoreClose;
    };

    class SocketConnection :
        public ::cppu::WeakImplHelper2< ::com::sun::star::connection::XConnection, ::com::sun::star::connection::XConnectionBroadcaster >

    {
    public:
        SocketConnection( const ::rtl::OUString & s , sal_uInt16 nPort  , sal_Bool bIgnoreClose );

        virtual sal_Int32 SAL_CALL read( ::com::sun::star::uno::Sequence< sal_Int8 >& aReadBytes,
                                         sal_Int32 nBytesToRead )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL write( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL flush(  ) throw(
            ::com::sun::star::io::IOException,
            ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL close(  )
            throw(::com::sun::star::io::IOException,
                  ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);


        // XConnectionBroadcaster
        virtual void SAL_CALL addStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStreamListener(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener>& aListener)
            throw(::com::sun::star::uno::RuntimeException);

    public:
        ::vos::OConnectorSocket m_socket;
        ::vos::OInetSocketAddr m_addr;
        oslInterlockedCount m_nStatus;
        ::rtl::OUString m_sDescription;
        sal_Bool m_bIgnoreClose;

        ::osl::Mutex _mutex;
        sal_Bool     _started;
        sal_Bool     _closed;
        sal_Bool     _error;

          XStreamListener_hash_set _listeners;
    };
};


