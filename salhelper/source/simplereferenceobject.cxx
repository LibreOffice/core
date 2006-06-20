/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplereferenceobject.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:11:45 $
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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include "salhelper/simplereferenceobject.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

using salhelper::SimpleReferenceObject;

SimpleReferenceObject::~SimpleReferenceObject() SAL_THROW(())
{
    OSL_ASSERT(m_nCount == 0);
}

void * SimpleReferenceObject::operator new(std::size_t nSize)
    SAL_THROW((std::bad_alloc))
{
    return ::operator new(nSize);
}

void * SimpleReferenceObject::operator new(std::size_t nSize,
                                           std::nothrow_t const &)
    SAL_THROW(())
{
#if defined WNT
    return ::operator new(nSize);
        // WNT lacks a global nothrow operator new...
#else // WNT
    return ::operator new(nSize, std::nothrow);
#endif // WNT
}

void SimpleReferenceObject::operator delete(void * pPtr) SAL_THROW(())
{
    ::operator delete(pPtr);
}

void SimpleReferenceObject::operator delete(void * pPtr, std::nothrow_t const &)
    SAL_THROW(())
{
#if defined WNT || (defined IRIX && !defined GCC)
    ::operator delete(pPtr); // WNT lacks a global nothrow operator delete...
#else // WNT
    ::operator delete(pPtr, std::nothrow);
#endif // WNT
}
