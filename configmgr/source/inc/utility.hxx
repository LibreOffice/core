/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: utility.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:01:52 $
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

#ifndef CONFIGMGR_UTILITY_HXX_
#define CONFIGMGR_UTILITY_HXX_

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif

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

    struct Refcounted
    : virtual salhelper::SimpleReferenceObject
    {
    };

}

#endif // CONFIGMGR_UTILITY_HXX_


