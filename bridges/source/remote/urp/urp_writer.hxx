/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_writer.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:50:38 $
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
#include <list>

#include <osl/conditn.h>
#include <osl/mutex.hxx>

#include <rtl/ustring.hxx>

#include <osl/thread.hxx>

#include <com/sun/star/uno/Type.hxx>

struct remote_Connection;

namespace bridges_urp
{
    struct RemoteReleaseCall
    {
        ::rtl::OUString sOid;
        ::com::sun::star::uno::Type typeInterface;
    };

    struct urp_BridgeImpl;
    class OWriterThread :
        public ::osl::Thread
    {
    public:
        OWriterThread( remote_Connection *pConnection,
                       urp_BridgeImpl *m_pBridgeImpl,
                       uno_Environment *pEnvRemote);
        ~OWriterThread(  );

        virtual void SAL_CALL run();

        void touch( sal_Bool bImmediately );
        void sendEmptyMessage();

        void abort();

        void SAL_CALL insertReleaseRemoteCall (
            rtl_uString *pOid,typelib_TypeDescriptionReference *pTypeRef);
        void SAL_CALL executeReleaseRemoteCalls();

    private:
        void write();
        oslCondition m_oslCondition;
        sal_Bool m_bAbort;
        sal_Bool m_bInBlockingWait;
        sal_Bool m_bEnterBlockingWait;
        remote_Connection *m_pConnection;
        urp_BridgeImpl *m_pBridgeImpl;
        uno_Environment *m_pEnvRemote; // this is held weak only

        ::osl::Mutex m_releaseCallMutex;
        ::std::list< struct RemoteReleaseCall > m_lstReleaseCalls;
    };
}

