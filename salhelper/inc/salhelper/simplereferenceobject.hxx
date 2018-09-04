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



#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#define _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

#include "osl/interlck.h"
#include "sal/types.h"

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

#include <salhelper/salhelperdllapi.h>

namespace salhelper {

/** A simple base implementation for reference-counted objects.

    Classes that want to implement a reference-counting mechanism based on the
    acquire()/release() interface should derive from this class.

    The reason to have class local operators new and delete here is technical.
    Imagine a class D derived from SimpleReferenceObject, but implemented in
    another shared library that happens to use different global operators new
    and delete from those used in this shared library (which, sadly, seems to
    be possible with shared libraries).  Now, without the class local
    operators new and delete here, a code sequence like "new D" would use the
    global operator new as found in the other shared library, while the code
    sequence "delete this" in release() would use the global operator delete
    as found in this shared library---and these two operators would not be
    guaranteed to match.

    There are no overloaded operators new and delete for placement new here,
    because it is felt that the concept of placement new does not work well
    with the concept of reference-counted objects; so it seems best to simply
    leave those operators out.

    The same problem as with operators new and delete would also be there with
    operators new[] and delete[].  But since arrays of reference-counted
    objects are of no use, anyway, it seems best to simply declare and not
    define (private) operators new[] and delete[].

    Note how during the move to gbuild, the delete[] had to be implemented,
    as missing vector delete errors stopped linking. The small consolation is
    that is a private method, so it may as well not exist. Right?
 */
class SALHELPER_DLLPUBLIC SimpleReferenceObject
{
public:
    inline SimpleReferenceObject() SAL_THROW(()): m_nCount(0) {}

    /** @ATTENTION
        The results are undefined if, for any individual instance of
        SimpleReferenceObject, the total number of calls to acquire() exceeds
        the total number of calls to release() by a plattform dependent amount
        (which, hopefully, is quite large).
     */
    inline void acquire() SAL_THROW(())
    { osl_incrementInterlockedCount(&m_nCount); }

    inline void release() SAL_THROW(())
    { if (osl_decrementInterlockedCount(&m_nCount) == 0) delete this; }

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize) SAL_THROW((std::bad_alloc));

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize,
                               std::nothrow_t const & rNothrow)
        SAL_THROW(());

    /** see general class documentation
     */
    static void operator delete(void * pPtr) SAL_THROW(());

    /** see general class documentation
     */
    static void operator delete(void * pPtr, std::nothrow_t const & rNothrow)
        SAL_THROW(());

protected:
    virtual ~SimpleReferenceObject() SAL_THROW(());

private:
    oslInterlockedCount m_nCount;

    /** not implemented
        @internal
     */
    SimpleReferenceObject(SimpleReferenceObject &);

    /** not implemented
        @internal
     */
    void operator =(SimpleReferenceObject);

    /** not implemented (see general class documentation)
        @internal
     */
    static void * operator new[](std::size_t);

    /** not implemented (see general class documentation)
        @internal
     */
    static void operator delete[](void * pPtr);
};

}

#endif // _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
