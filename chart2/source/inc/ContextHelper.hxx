/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:14:35 $
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
#ifndef CHART_CONTEXTHELPER_HXX
#define CHART_CONTEXTHELPER_HXX

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>

#include <map>

namespace chart
{

namespace ContextHelper
{

typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any >
    tContextEntryMapType;
typedef ::comphelper::MakeMap< ::rtl::OUString, ::com::sun::star::uno::Any >
    MakeContextEntryMap;

/** creates a component context containing the properties given in the map.

    @param rDelegateContext
        delegation to further context
 */
::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
    createContext( const tContextEntryMapType & rMap,
                   const ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext > & rDelegateContext =
                   ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() );

} //  namespace ContextHelper

} //  namespace chart

// CHART_CONTEXTHELPER_HXX
#endif
