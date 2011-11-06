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



#include <osl/pipe.hxx>
#include <osl/socket.hxx>
#include <rtl/unload.h>

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

}

