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


#ifndef _VOS_EXECABL_HXX_
#define _VOS_EXECABL_HXX_

#   include <vos/types.hxx>
#   include <vos/refernce.hxx>

namespace vos
{


/** IExecutable

    The IExecutable-interface is to be understood as wrapper around
    your application-code, which allows it to be executed asynchronously.

    Wether you want real asynchronous behaviour or just pseudo-multitasking
    depends on which kind of execution-service you use to execute your executable.

    (E.g. Threadpool/Fiberpool)

    @author  Bernd Hofner
    @version 0.1
*/

class IExecutable : public NAMESPACE_VOS(IReference)
{
public:

    /** Overload to implement your functionality.
        Return True, if you want "execute()" to be called again.
    */
    virtual sal_Bool SAL_CALL execute()= 0;


    /** If you use blocking calls within your execute-function,
        you should provide here a means to unblock cleanly.
        @Return False if you are not able to unblock the
        thread.

    */
    virtual sal_Bool SAL_CALL unblock()= 0;

    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const= 0;
};


/** OExecutable
    added default impl. of IReferenceCounter
*/
class OExecutable : public NAMESPACE_VOS(IExecutable),
                    public NAMESPACE_VOS(OReference)

{
public:

    virtual ~OExecutable()
    {
    }

    /*
        Since the dominance of the OReferenceCounter impl.
        of the IReferenceCounter is not granted, delegate
        the methods to this branch of the diamond-shaped
        inheritance tree.
    */

    virtual RefCount SAL_CALL acquire()
    {
        return OReference::acquire();
    }
    virtual RefCount SAL_CALL release()
    {
        return OReference::release();
    }
    virtual RefCount SAL_CALL referenced() const
    {
        return OReference::referenced();
    }


    /** Default implementation of unblock does nothing.
    */
    virtual sal_Bool SAL_CALL unblock() { return sal_True; }


    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
        Use a default implementation of the comparison-operators
        here without a correct semantic.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const
    {
        return sal_True;
    }

    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const
    {
        return sal_False;
    }
};

}


#endif  // _VOS_EXECABL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
