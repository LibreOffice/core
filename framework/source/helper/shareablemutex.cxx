/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shareablemutex.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:26:31 $
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
#include <helper/shareablemutex.hxx>
#endif

namespace framework
{

ShareableMutex::ShareableMutex()
{
    pMutexRef = new MutexRef;
    pMutexRef->acquire();
}

ShareableMutex::ShareableMutex( const ShareableMutex& rShareableMutex )
{
    pMutexRef = rShareableMutex.pMutexRef;
    if ( pMutexRef )
        pMutexRef->acquire();
}

const ShareableMutex& ShareableMutex::operator=( const ShareableMutex& rShareableMutex )
{
    if ( rShareableMutex.pMutexRef )
        rShareableMutex.pMutexRef->acquire();
    if ( pMutexRef )
        pMutexRef->release();
    pMutexRef = rShareableMutex.pMutexRef;
    return *this;
}

ShareableMutex::~ShareableMutex()
{
    if ( pMutexRef )
        pMutexRef->release();
}

void ShareableMutex::acquire()
{
    if ( pMutexRef )
        pMutexRef->m_oslMutex.acquire();
}

void ShareableMutex::release()
{
    if ( pMutexRef )
        pMutexRef->m_oslMutex.release();
}

::osl::Mutex& ShareableMutex::getShareableOslMutex()
{
    return pMutexRef->m_oslMutex;
}

}
