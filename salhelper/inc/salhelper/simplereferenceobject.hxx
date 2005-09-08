/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplereferenceobject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:59:43 $
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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#define _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_

#ifndef _OSL_INTERLCK_H_
#include "osl/interlck.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif
#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

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
 */
class SimpleReferenceObject
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
