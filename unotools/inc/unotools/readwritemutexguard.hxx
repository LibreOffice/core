/*************************************************************************
 *
 *  $RCSfile: readwritemutexguard.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $ $
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

#ifndef INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX
#define INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

namespace utl {

class ReadWriteGuard;
class ReadWriteMutex
{
    friend class ReadWriteGuard;

            sal_uInt32          nReadCount;
            sal_uInt32          nBlockCriticalCount;
            ::osl::Mutex*       pMutex;
            ::osl::Mutex*       pWriteMutex;

public:
                                ReadWriteMutex()
                                    : nReadCount(0)
                                    , nBlockCriticalCount(0)
                                    , pMutex( new ::osl::Mutex )
                                    , pWriteMutex( new ::osl::Mutex )
                                    {}
                                ~ReadWriteMutex()
                                    {
                                        delete pMutex;
                                        delete pWriteMutex;
                                    }
};


namespace ReadWriteGuardMode {
const sal_Int32 nWrite          = 0x01;
const sal_Int32 nCriticalChange = 0x02 | nWrite;
const sal_Int32 nBlockCritical  = 0x04;     // only a block, not a read, exclusive flag!
}

/** Enable multiple threads to read simultaneously, but a write blocks all
    other reads and writes, and a read blocks any write.
    Used in I18N wrappers to be able to maintain a single instance of a wrapper
    for the standard Office locale.
    NEVER construct a writing guard if there is already a reading guard in the
    same context, the following will dead lock EVEN IN THE SAME THREAD!
    void foo()
    {
        ReadWriteGuard aGuard1( aMutex );
        bar();
    }
    void bar()
    {
        // waits forever for aGuard1
        ReadWriteGuard aGuard2( aMutex, ReadWriteGuardMode::nWrite );
    }
 */
class ReadWriteGuard
{
            ReadWriteMutex&     rMutex;
            sal_Int32           nMode;
public:
                                ReadWriteGuard(
                                    ReadWriteMutex& rMutex,
                                    sal_Int32 nRequestMode = 0  // read only
                                    );
                                ~ReadWriteGuard();

    /** Be careful with this, it does wait for ANY read to complete.
        The following will dead lock EVEN IN THE SAME THREAD!
        void foo()
        {
            ReadWriteGuard aGuard1( aMutex );
            bar();
        }
        void bar()
        {
            ReadWriteGuard aGuard2( aMutex );
            aGuard2.changeReadToWrite();    // waits forever for aGuard1
        }
     */
            void                changeReadToWrite();
};

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_READWRITEMUTEXGUARD_HXX

