/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplereferencecomponent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-12-14 16:51:04 $
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

#ifndef _SD_SIMPLEREFERENCECOMPONENT_HXX_
#define _SD_SIMPLEREFERENCECOMPONENT_HXX_

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

namespace sd {

/** A simple base implementation for reference-counted components.
    acts like sal::SimpleReferenceObject but calls the virtual disposing()
    methods before the ref count switches from 1 to zero.
 */
class SimpleReferenceComponent
{
public:
    SimpleReferenceComponent();

    /** @ATTENTION
        The results are undefined if, for any individual instance of
        SimpleReferenceComponent, the total number of calls to acquire() exceeds
        the total number of calls to release() by a plattform dependent amount
        (which, hopefully, is quite large).
     */
    void acquire();
    void release();

    void Dispose();

    bool isDisposed() const { return mbDisposed; }

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize) SAL_THROW((std::bad_alloc));

    /** see general class documentation
     */
    static void * operator new(std::size_t nSize,
                               std::nothrow_t const & rNothrow)
       ;

    /** see general class documentation
     */
    static void operator delete(void * pPtr);

    /** see general class documentation
     */
    static void operator delete(void * pPtr, std::nothrow_t const & rNothrow)
       ;

protected:
    virtual void disposing();

    virtual ~SimpleReferenceComponent();

private:
    oslInterlockedCount m_nCount;

    /** not implemented
        @internal
     */
    SimpleReferenceComponent(SimpleReferenceComponent &);

    /** not implemented
        @internal
     */
    void operator =(SimpleReferenceComponent);

    /** not implemented (see general class documentation)
        @internal
     */
    static void * operator new[](std::size_t);

    /** not implemented (see general class documentation)
        @internal
     */
    static void operator delete[](void * pPtr);

    bool mbDisposed;
};

}

#endif // _SALHELPER_SimpleReferenceComponent_HXX_
