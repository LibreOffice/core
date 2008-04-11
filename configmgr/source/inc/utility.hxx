/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: utility.hxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_UTILITY_HXX_
#define CONFIGMGR_UTILITY_HXX_
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable : 4350) // behavior change: 'member1' called instead of 'member2'
#endif

#include <com/sun/star/uno/RuntimeException.hpp>
#include <salhelper/simplereferenceobject.hxx>
#include "datalock.hxx"

#include "osl/diagnose.h"

#define CFG_NOTHROW() SAL_THROW( () )

#define CFG_THROW1( Ex1 )                       SAL_THROW( (Ex1) )
#define CFG_THROW2( Ex1,Ex2 )                   SAL_THROW( (Ex1,Ex2) )
#define CFG_THROW3( Ex1,Ex2,Ex3 )               SAL_THROW( (Ex1,Ex2,Ex3) )
#define CFG_THROW4( Ex1,Ex2,Ex3,Ex4 )           SAL_THROW( (Ex1,Ex2,Ex3,Ex4) )
#define CFG_THROW5( Ex1,Ex2,Ex3,Ex4,Ex5 )       SAL_THROW( (Ex1,Ex2,Ex3,Ex4,Ex5) )
#define CFG_THROW6( Ex1,Ex2,Ex3,Ex4,Ex5,Ex6 )   SAL_THROW( (Ex1,Ex2,Ex3,Ex4,Ex5,Ex6) )

#define CFG_UNO_THROW1( Ex1 )           \
    SAL_THROW( (::com::sun::star::Ex1,  \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW2( Ex1,Ex2 )       \
    SAL_THROW( (::com::sun::star::Ex1, ::com::sun::star::Ex2,   \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW3( Ex1,Ex2,Ex3 )   \
    SAL_THROW( (::com::sun::star::Ex1, ::com::sun::star::Ex2, ::com::sun::star::Ex3,    \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW4( Ex1,Ex2,Ex3,Ex4 )   \
    SAL_THROW( (::com::sun::star::Ex1, ::com::sun::star::Ex2, ::com::sun::star::Ex3,    \
                ::com::sun::star::Ex4,  \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW5( Ex1,Ex2,Ex3,Ex4,Ex5 )   \
    SAL_THROW( (::com::sun::star::Ex1, ::com::sun::star::Ex2, ::com::sun::star::Ex3,    \
                ::com::sun::star::Ex4, ::com::sun::star::Ex5,   \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW6( Ex1,Ex2,Ex3,Ex4,Ex5,Ex6 )   \
    SAL_THROW( (::com::sun::star::Ex1, ::com::sun::star::Ex2, ::com::sun::star::Ex3,    \
                ::com::sun::star::Ex4, ::com::sun::star::Ex5, ::com::sun::star::Ex6,    \
                ::com::sun::star::uno::RuntimeException) )

#define CFG_UNO_THROW_ALL(  ) CFG_UNO_THROW1(uno::Exception)
#define CFG_UNO_THROW_RTE(  ) CFG_UNO_THROW1(uno::RuntimeException)

namespace configmgr
{
    class Noncopyable
    {
    protected:
        Noncopyable() {}
        ~Noncopyable() {}
    private:
        Noncopyable     (Noncopyable& notImplemented);
        void operator=  (Noncopyable& notImplemented);
    };

    // Used for internal, non-UNO objects
    class SimpleReferenceObject
    {
    public:
        inline SimpleReferenceObject() SAL_THROW(()): m_nCount(0) {}
#define SIMPLE_REFERENCE_FAST
#ifdef SIMPLE_REFERENCE_FAST
        inline void acquire() SAL_THROW(())
        {
//            OSL_ASSERT(UnoApiLock::isHeld());
            m_nCount++;
        }
        inline void release() SAL_THROW(())
        {
//            OSL_ASSERT(UnoApiLock::isHeld());
            if (--m_nCount == 0)
                delete this;
        }
#else
        void acquire() SAL_THROW(());
        void release() SAL_THROW(());
#endif

    protected:
        virtual ~SimpleReferenceObject() SAL_THROW(());
    private:
        sal_uInt32 m_nCount;
        // not implemented:
        SimpleReferenceObject(SimpleReferenceObject &);
        void operator =(SimpleReferenceObject);
    };

    struct Refcounted : virtual configmgr::SimpleReferenceObject
    {
    };
}

#endif // CONFIGMGR_UTILITY_HXX_

