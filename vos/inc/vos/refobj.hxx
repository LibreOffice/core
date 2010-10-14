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


#ifndef _VOS_REFOBJ_HXX_
#define _VOS_REFOBJ_HXX_

/**
    ORefObj<T>

    template type to implement handle/body behaviour
    with reference-counting.
*/

#include <vos/refernce.hxx>
#include <osl/diagnose.h>

namespace vos
{

template <class T>
class ORefObj : public IReference
{
public:
    ORefObj(const T& Obj);

    inline ~ORefObj();

    virtual RefCount SAL_CALL acquire()
        { return (m_RefCount.acquire()); }
    virtual RefCount SAL_CALL release()
        { return (m_RefCount.release()); }
    virtual RefCount SAL_CALL referenced() const
        { return (m_RefCount.referenced()); }

    T& SAL_CALL operator=(const T& Obj);

    SAL_CALL operator T&();
    SAL_CALL operator const T&() const;

    T& SAL_CALL operator() ();
    const T& SAL_CALL operator() () const;

    const   T& SAL_CALL getObj() const;
    T& SAL_CALL getObj();

protected:
    T         m_Obj;
    ORefCount m_RefCount;

private:
    ORefObj(const ORefObj<T>& handle);
    ORefObj<T>& SAL_CALL operator= (const ORefObj<T>& handle);
};

// include template implementation
#include <vos/refobj.inl>

}


#endif // _VOS_REF_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
