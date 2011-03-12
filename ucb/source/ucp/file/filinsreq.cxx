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
#include "filinsreq.hxx"
#include "shell.hxx"
#include "filglob.hxx"
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>



using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace fileaccess;



void SAL_CALL
XInteractionSupplyNameImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionSupplyNameImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionSupplyNameImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionSupplyName*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionSupplyNameImpl,
                      XTypeProvider,
                      XInteractionSupplyName )



void SAL_CALL
XInteractionAbortImpl::acquire( void )
    throw()
{
    OWeakObject::acquire();
}



void SAL_CALL
XInteractionAbortImpl::release( void )
    throw()
{
    OWeakObject::release();
}



Any SAL_CALL
XInteractionAbortImpl::queryInterface( const Type& rType )
    throw( RuntimeException )
{
    Any aRet = cppu::queryInterface( rType,
                                     SAL_STATIC_CAST( lang::XTypeProvider*, this ),
                                     SAL_STATIC_CAST( XInteractionAbort*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionAbortImpl,
                      XTypeProvider,
                      XInteractionAbort )



XInteractionRequestImpl::XInteractionRequestImpl(
    const rtl::OUString& aClashingName,
    const Reference<XInterface>& xOrigin,
    shell *pShell,sal_Int32 CommandId)
    : p1( new XInteractionSupplyNameImpl ),
      p2( new XInteractionAbortImpl ),
      m_nErrorCode(0),
      m_nMinorError(0),
      m_aSeq( 2 ),
      m_aClashingName(aClashingName),
      m_xOrigin(xOrigin)
{
    if( pShell )
        pShell->retrieveError(CommandId,m_nErrorCode,m_nMinorError);
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
    Any aRet =
        cppu::queryInterface(
            rType,
            SAL_STATIC_CAST( lang::XTypeProvider*, this ),
            SAL_STATIC_CAST( XInteractionRequest*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////

XTYPEPROVIDER_IMPL_2( XInteractionRequestImpl,
                      XTypeProvider,
                      XInteractionRequest )


Any SAL_CALL
XInteractionRequestImpl::getRequest()
    throw(RuntimeException)
{
    Any aAny;
    if(m_nErrorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR)
    {
        NameClashException excep;
        excep.Name = m_aClashingName;
        excep.Classification = InteractionClassification_ERROR;
        excep.Context = m_xOrigin;
        excep.Message = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "folder exists and overwritte forbidden"));
        aAny <<= excep;
    }
    else if(m_nErrorCode == TASKHANDLING_INVALID_NAME_MKDIR)
    {
        InteractiveAugmentedIOException excep;
        excep.Code = IOErrorCode_INVALID_CHARACTER;
        PropertyValue prop;
        prop.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ResourceName"));
        prop.Handle = -1;
        prop.Value <<= m_aClashingName;
        Sequence<Any> seq(1);
        seq[0] <<= prop;
        excep.Arguments = seq;
        excep.Classification = InteractionClassification_ERROR;
        excep.Context = m_xOrigin;
        excep.Message = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "the name contained invalid characters"));
        aAny <<= excep;

    }
    return aAny;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
