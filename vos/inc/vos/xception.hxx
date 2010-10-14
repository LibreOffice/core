/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _XCEPTION_HXX_
#define _XCEPTION_HXX_

#include <vos/object.hxx>

#ifdef _MSC_VER
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
