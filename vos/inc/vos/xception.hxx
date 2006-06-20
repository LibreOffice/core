/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xception.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:06:05 $
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

#ifndef _XCEPTION_HXX_
#define _XCEPTION_HXX_

#ifndef _VOS_OBJECT_HXX_
#include <vos/object.hxx>
#endif

#ifdef SAL_W32
#pragma warning( disable : 4290 )
#endif

namespace vos
{

/* avoid clashes with <vos/exception.hxx> */
#define OException OBaseException

/*
 * Macros for true try/catch based Exception Handling (public)
 * based on true rtti type checking
 */

#define THROW_AGAIN                     throw;
#define THROW( Constructor )            throw Constructor;

/*
 * declaration of the exceptions that may be thrown by a function
 * (e.g.) void myfunction(sal_Int32 a) throw ( std::bad_alloc );
 * is not fully supported by all compilers
 */

#define THROWS( ARG ) throw ARG

/*
 * just a base class for further exceptions
 */

class OException : public OObject
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OException, vos));

public:
    virtual ~OException();
    OException() {}
    OException( const OException & ) : OObject() {}

    OException& SAL_CALL operator=(const OException&)
        { return *this; }
};

}

#endif /* _XCEPTION_HXX_ */

