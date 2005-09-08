/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shareablemutex.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:19:11 $
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

#ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_
#define __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

namespace framework
{

class ShareableMutex
{
    public:
        ShareableMutex();
        ShareableMutex( const ShareableMutex& rShareableMutex );
        const ShareableMutex& operator=( const ShareableMutex& rShareableMutex );

        ~ShareableMutex();

        void acquire();
        void release();
        ::osl::Mutex& getShareableOslMutex();

    private:
        struct MutexRef
        {
            MutexRef() : m_refCount(0) {}
            void acquire()
            {
                osl_incrementInterlockedCount( &m_refCount );
            }

            void release()
            {
                if ( osl_decrementInterlockedCount( &m_refCount ) == 0 )
                    delete this;
            }

            oslInterlockedCount m_refCount;
            osl::Mutex          m_oslMutex;
        };

        MutexRef* pMutexRef;
};

class ShareGuard
{
    public:
        ShareGuard( ShareableMutex& rShareMutex ) :
            m_rShareMutex( rShareMutex )
        {
            m_rShareMutex.acquire();
        }

        ~ShareGuard()
        {
            m_rShareMutex.release();
        }

    private:
        ShareGuard();
        ShareGuard& operator=( const ShareGuard& );

        ShareableMutex& m_rShareMutex;
};

}

#endif // #ifndef __FRAMEWORK_HELPER_SHAREABLEMUTEX_HXX_
