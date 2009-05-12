/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: current_context.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#define _UNO_CURRENT_CONTEXT_HXX_

#include <uno/current_context.h>

#include <com/sun/star/uno/XCurrentContext.hpp>


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Getting the current context.
    @attention
    Don't spread the returned interface around to other threads.  Every thread has its own
    current context.

    @return current context or null ref, if none is set
*/
inline Reference< XCurrentContext > SAL_CALL getCurrentContext()
    SAL_THROW( () )
{
    Reference< XCurrentContext > xRet;
    ::rtl::OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    ::uno_getCurrentContext( (void **)&xRet, aEnvTypeName.pData, 0 );
    return xRet;
}
/** Setting the current context.

    @param xContext current context to be set
    @return true, if context has been successfully set
*/
inline bool SAL_CALL setCurrentContext(
    Reference< XCurrentContext > const & xContext )
    SAL_THROW( () )
{
    ::rtl::OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    return (::uno_setCurrentContext( xContext.get(), aEnvTypeName.pData, 0 ) != sal_False);
}

/** Objects of this class are used for applying a current context until they are destructed, i.e.
    the ctor of this class saves the previous and sets the given context while the dtor restores
    the previous one upon destruction.
*/
class ContextLayer
{
    /** this C++ environment type name.
        @internal
    */
    ::rtl::OUString m_aEnvTypeName;
    /** previous context
        @internal
    */
    Reference< XCurrentContext > m_xPreviousContext;

public:
    /** Constructor: Saves the previous context and sets the new (given) one.

        @param xNewContext new context to be set
    */
    inline ContextLayer(
        Reference< XCurrentContext > const & xNewContext = Reference< XCurrentContext >() )
        SAL_THROW( () );
    /** Destructor: restores the previous context.
    */
    inline ~ContextLayer() SAL_THROW( () );

    /** Gets the previously set context.

        @return the previously set context
    */
    inline Reference< XCurrentContext > SAL_CALL getPreviousContext() const
        SAL_THROW( () )
        { return m_xPreviousContext; }
};
//__________________________________________________________________________________________________
inline ContextLayer::ContextLayer( Reference< XCurrentContext > const & xNewContext )
    SAL_THROW( () )
    : m_aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) )
{
    ::uno_getCurrentContext( (void **)&m_xPreviousContext, m_aEnvTypeName.pData, 0 );
    ::uno_setCurrentContext( xNewContext.get(), m_aEnvTypeName.pData, 0 );
}
//__________________________________________________________________________________________________
inline ContextLayer::~ContextLayer()
    SAL_THROW( () )
{
    ::uno_setCurrentContext( m_xPreviousContext.get(), m_aEnvTypeName.pData, 0 );
}

}
}
}
}

#endif
