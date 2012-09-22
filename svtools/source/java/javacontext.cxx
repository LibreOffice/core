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

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Type.hxx"
#include <svtools/svtresid.hxx>
#include <svtools/javacontext.hxx>
#include <svtools/javainteractionhandler.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::task;
namespace svt
{

JavaContext::JavaContext( const Reference< XCurrentContext > & ctx,
                          bool bShowErrorsOnce)
    : m_aRefCount(0),
      m_xNextContext( ctx ),
      m_bShowErrorsOnce(bShowErrorsOnce)
{
}

JavaContext::~JavaContext()
{
}

Any SAL_CALL JavaContext::queryInterface(const Type& aType )
    throw (RuntimeException)
{
    if (aType == getCppuType(reinterpret_cast<Reference<XInterface>*>(0)))
        return Any(Reference<XInterface>(static_cast<XInterface*>(this)));
    else if (aType == getCppuType(reinterpret_cast<Reference<XCurrentContext>*>(0)))
        return Any(Reference<XCurrentContext>( static_cast<XCurrentContext*>(this)));
    return Any();
}

void SAL_CALL JavaContext::acquire(  ) throw ()
{
    osl_atomic_increment( &m_aRefCount );
}

void SAL_CALL JavaContext::release(  ) throw ()
{
    if (! osl_atomic_decrement( &m_aRefCount ))
        delete this;
}

Any SAL_CALL JavaContext::getValueByName( const ::rtl::OUString& Name) throw (RuntimeException)
{
    Any retVal;

    if ( 0 == Name.compareToAscii( JAVA_INTERACTION_HANDLER_NAME ))
    {
        {
            osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
            if (!m_xHandler.is())
                m_xHandler = Reference< XInteractionHandler >(
                    new JavaInteractionHandler(m_bShowErrorsOnce));
        }
        retVal = makeAny(m_xHandler);

    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
