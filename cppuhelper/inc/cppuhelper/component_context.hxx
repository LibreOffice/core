/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: component_context.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:11:31 $
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
