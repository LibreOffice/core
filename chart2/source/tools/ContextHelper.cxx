/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContextHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:27:39 $
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
#include "ContextHelper.hxx"
#include <cppuhelper/component_context.hxx>

#include <vector>

using namespace ::com::sun::star;

namespace chart
{

namespace ContextHelper
{

uno::Reference< uno::XComponentContext >
    createContext(
        const tContextEntryMapType & rMap,
        const uno::Reference< uno::XComponentContext > & rDelegateContext )
{
    ::std::vector< ::cppu::ContextEntry_Init > aVec( rMap.size());
    for( tContextEntryMapType::const_iterator aIt = rMap.begin();
         aIt != rMap.end();
         ++aIt )
    {
        aVec.push_back( ::cppu::ContextEntry_Init( (*aIt).first, (*aIt).second) );
    }

    return ::cppu::createComponentContext( & aVec[0], aVec.size(), rDelegateContext );
}

} //  namespace ContextHelper

} //  namespace chart
