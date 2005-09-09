/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:02:49 $
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


#ifndef _VOS_REFOBJ_HXX_
#define _VOS_REFOBJ_HXX_

/**
    ORefObj<T>

    template type to implement handle/body behaviour
    with reference-counting.
*/

#ifndef _VOS_REFERNCE_HXX_
#   include <vos/refernce.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#   include <vos/diagnose.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

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

#ifdef _USE_NAMESPACE
}
#endif


#endif // _VOS_REF_HXX_

