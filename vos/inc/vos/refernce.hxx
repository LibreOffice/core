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

#ifndef _VOS_REFERNCE_HXX_
#define _VOS_REFERNCE_HXX_

#   include <vos/types.hxx>
#   include <osl/interlck.h>
#   include <vos/object.hxx>

namespace vos
{

/** Interface for refernce-counting
*/
class IReference
{
public:

    IReference() { }
    virtual ~IReference() { }

    typedef oslInterlockedCount RefCount;

    virtual RefCount SAL_CALL acquire()=0;
    virtual RefCount SAL_CALL release()=0;

    virtual RefCount SAL_CALL referenced() const=0;
};

class ORefCount
{
public:
    typedef IReference::RefCount RefCount;

    ORefCount() { m_RefCount = 0; }
    ORefCount(RefCount n) { m_RefCount = n; }
    virtual ~ORefCount();

    RefCount SAL_CALL acquire() { return (osl_incrementInterlockedCount(&m_RefCount)); };
    RefCount SAL_CALL release() { return (osl_decrementInterlockedCount(&m_RefCount)); };

    RefCount SAL_CALL operator++()   { return acquire(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL operator--()   { return release(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL referenced() const
        { return (m_RefCount); }

protected:
    RefCount m_RefCount;

private:
    // disable copy/assignment
    ORefCount(const ORefCount&);
    ORefCount& SAL_CALL operator= (const ORefCount&);
};

class OReference : public vos::IReference
{
public:
    OReference();
    virtual ~OReference();

    virtual RefCount SAL_CALL acquire();
    virtual RefCount SAL_CALL release();

    virtual RefCount SAL_CALL referenced() const
        { return (m_RefCount.referenced()); }

protected:
    ORefCount m_RefCount;

private:
    // disable copy/assignment
    OReference(const OReference&);
    OReference& SAL_CALL operator= (const OReference&);
};

}

#endif  // _VOS_REFERNCE_HXX_

