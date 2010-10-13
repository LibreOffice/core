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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include "ftpintreq.hxx"

using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::task;
using namespace ftp;


XInteractionApproveImpl::XInteractionApproveImpl()
    : m_bSelected(false)
{
}


void SAL_CALL
XInteractionApproveImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInteractionApproveImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionApproveImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionApprove*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionApproveImpl,
                      XTypeProvider,
                      XInteractionApprove )


void SAL_CALL XInteractionApproveImpl::select()
    throw (RuntimeException)
{
    m_bSelected = true;
}


bool XInteractionApproveImpl::isSelected() const
{
    return m_bSelected;
}


// XInteractionDisapproveImpl

XInteractionDisapproveImpl::XInteractionDisapproveImpl()
    : m_bSelected(false)
{
}


void SAL_CALL
XInteractionDisapproveImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInteractionDisapproveImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionDisapproveImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionDisapprove*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionDisapproveImpl,
                      XTypeProvider,
                      XInteractionDisapprove )


void SAL_CALL XInteractionDisapproveImpl::select()
    throw (RuntimeException)

{
    m_bSelected = true;
}


// XInteractionRequestImpl

XInteractionRequestImpl::XInteractionRequestImpl(const rtl::OUString& aName)
    : p1( new XInteractionApproveImpl ),
      p2( new XInteractionDisapproveImpl ),
      m_aName(aName),
      m_aSeq( 2 )
{
    m_aSeq[0] = Reference<XInteractionContinuation>(p1);
    m_aSeq[1] = Reference<XInteractionContinuation>(p2);
}


void SAL_CALL
XInteractionRequestImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionRequestImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionRequestImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface(
        rType,
        SAL_STATIC_CAST( lang::XTypeProvider*, this ),
        SAL_STATIC_CAST( XInteractionRequest*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
/////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionRequestImpl,
                      XTypeProvider,
                      XInteractionRequest )


Any SAL_CALL XInteractionRequestImpl::getRequest(  )
    throw (RuntimeException)
{
    Any aAny;
    UnsupportedNameClashException excep;
    excep.NameClash = NameClash::ERROR;
    aAny <<= excep;
    return aAny;
}


Sequence<Reference<XInteractionContinuation > > SAL_CALL
XInteractionRequestImpl::getContinuations(  )
    throw (RuntimeException)
{
    return m_aSeq;
}


bool XInteractionRequestImpl::approved() const
{
    return p1->isSelected();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
