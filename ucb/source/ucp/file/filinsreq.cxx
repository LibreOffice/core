 /*************************************************************************
 *
 *  $RCSfile: filinsreq.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 09:56:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "filinsreq.hxx"
#include "shell.hxx"
#include "filglob.hxx"
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif



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
    : m_aSeq( 2 ),
      p1( new XInteractionSupplyNameImpl ),
      p2( new XInteractionAbortImpl ),
      m_aClashingName(aClashingName),
      m_xOrigin(xOrigin),
      m_nErrorCode(0),
      m_nMinorError(0)
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
        prop.Name = rtl::OUString::createFromAscii("ResourceName");
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
