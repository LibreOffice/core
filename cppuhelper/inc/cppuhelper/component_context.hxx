/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: component_context.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#define _CPPUHELPER_COMPONENT_CONTEXT_HXX_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>


namespace cppu
{

/** Context entries init struct calling createComponentContext().
*/
struct ContextEntry_Init
{
    /** late init denotes a object that will be raised when first get() is calling for it

        The context implementation expects either a ::com::sun::star::lang::XSingleComponentFactory
        object as value (to instanciate the object) or a string as value for raising
        a service via the used service manager.
    */
    bool bLateInitService;
    /** name of context value
    */
    ::rtl::OUString name;
    /** context value
    */
    ::com::sun::star::uno::Any value;

    /** Default ctor.
    */
    inline ContextEntry_Init() SAL_THROW( () )
        : bLateInitService( false )
        {}
    /** Ctor.

        @param name_
               name of entry
        @param value_
               value of entry
        @param bLateInitService_
               whether this entry is a late-init named object entry
               (value is object factory or service string)
    */
    inline ContextEntry_Init(
        ::rtl::OUString const & name_,
        ::com::sun::star::uno::Any const & value_,
        bool bLateInitService_ = false ) SAL_THROW( () )
            : bLateInitService( bLateInitService_ ),
              name( name_ ),
              value( value_ )
        {}
};

/** Creates a component context with the given entries.

    @param pEntries array of entries
    @param nEntries number of entries
    @param xDelegate delegation to further context, if value was not found
    @return new context object
*/
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
SAL_CALL createComponentContext(
    ContextEntry_Init const * pEntries, sal_Int32 nEntries,
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xDelegate =
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
    SAL_THROW( () );

}

#endif
