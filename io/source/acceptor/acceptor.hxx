/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceptor.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:28:11 $
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

#ifndef _OSL_PIPE_HXX_
#include <osl/pipe.hxx>
#endif

#ifndef _OSL_SOCKET_HXX_
#include <osl/socket.hxx>
#endif

#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif

#include <com/sun/star/connection/XConnection.hpp>

namespace io_acceptor {

    extern rtl_StandardModuleCount g_moduleCount;

    class PipeAcceptor
    {
    public:
        PipeAcceptor( const ::rtl::OUString &sPipeName , const ::rtl::OUString &sConnectionDescription );

        void init();
        ::com::sun::star::uno::Reference < ::com::sun::star::connection::XConnection >  accept(  );

        void stopAccepting();

        ::osl::Mutex m_mutex;
        ::osl::Pipe m_pipe;
        ::rtl::OUString m_sPipeName;
        ::rtl::OUString m_sConnectionDescription;
        sal_Bool m_bClosed;
    };

    class SocketAcceptor
    {
    public:
        SocketAcceptor( const ::rtl::OUString & sSocketName ,
                        sal_uInt16 nPort,
                        sal_Bool bTcpNoDelay,
                        const ::rtl::OUString &sConnectionDescription );

        void init();
        ::com::sun::star::uno::Reference < ::com::sun::star::connection::XConnection > accept();

        void stopAccepting();

        ::osl::SocketAddr m_addr;
        ::osl::AcceptorSocket m_socket;
        ::rtl::OUString m_sSocketName;
        ::rtl::OUString m_sConnectionDescription;
        sal_uInt16 m_nPort;
        sal_Bool m_bTcpNoDelay;
        sal_Bool m_bClosed;
    };

};

