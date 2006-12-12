/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplereferencecomponent.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:47:21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SALHELPER_SIMPLEREFERENCECOMPONENT_HXX_
#include "helper/simplereferencecomponent.hxx"
#endif

#include "com/sun/star/uno/RuntimeException.hpp"

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif

#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

using com::sun::star::uno::RuntimeException;
using sd::SimpleReferenceComponent;

SimpleReferenceComponent::SimpleReferenceComponent()
: m_nCount(0)
, mbDisposed(false)
{
}

SimpleReferenceComponent::~SimpleReferenceComponent()
{
    OSL_ASSERT(m_nCount == 0);
    OSL_ASSERT(mbDisposed);
}

void SimpleReferenceComponent::acquire()
{
    osl_incrementInterlockedCount(&m_nCount);
}

void SimpleReferenceComponent::release()
{
    if((1 == m_nCount) && !mbDisposed)
    {
        try
        {
            Dispose();
        }
        catch (RuntimeException &
#if OSL_DEBUG_LEVEL > 0
            exc
#endif
            ) // don't break throw ()
        {
#if OSL_DEBUG_LEVEL > 0
            rtl::OString msg( rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, msg.getStr() );
#endif
        }
    }

    if(osl_decrementInterlockedCount(&m_nCount) == 0) delete this;
}

void SimpleReferenceComponent::Dispose()
{
    if( !mbDisposed )
    {
        mbDisposed = true;
        disposing();
    }
}

void SimpleReferenceComponent::disposing()
{
}

void * SimpleReferenceComponent::operator new(std::size_t nSize)
    SAL_THROW((std::bad_alloc))
{
    return ::operator new(nSize);
}

void * SimpleReferenceComponent::operator new(std::size_t nSize,
                                           std::nothrow_t const &
#ifndef WNT
                                           rNothrow
#endif
                                           )
    SAL_THROW(())
{
#if defined WNT
    return ::operator new(nSize);
        // WNT lacks a global nothrow operator new...
#else // WNT
    return ::operator new(nSize, rNothrow);
#endif // WNT
}

void SimpleReferenceComponent::operator delete(void * pPtr) SAL_THROW(())
{
    ::operator delete(pPtr);
}

void SimpleReferenceComponent::operator delete(void * pPtr,
                                            std::nothrow_t const &
#ifndef WNT
                                            rNothrow
#endif
)
    SAL_THROW(())
{
#if defined WNT || (defined IRIX && !defined GCC)
    ::operator delete(pPtr); // WNT lacks a global nothrow operator delete...
#else // WNT
    ::operator delete(pPtr, rNothrow);
#endif // WNT
}
