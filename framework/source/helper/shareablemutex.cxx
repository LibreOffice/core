/*************************************************************************
 *
 *  $RCSfile: shareablemutex.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-02-25 17:47:29 $
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
