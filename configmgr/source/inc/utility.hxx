/*************************************************************************
 *
 *  $RCSfile: utility.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:10 $
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


