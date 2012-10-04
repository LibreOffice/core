/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
        (static_cast< lang::XTypeProvider* >(this)),
        (static_cast< XInteractionApprove* >(this)) );
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
        (static_cast< lang::XTypeProvider* >(this)),
        (static_cast< XInteractionDisapprove* >(this)) );
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
        (static_cast< lang::XTypeProvider* >(this)),
        (static_cast< XInteractionRequest* >(this)) );
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
