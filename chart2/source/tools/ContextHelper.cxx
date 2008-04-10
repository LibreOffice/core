/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ContextHelper.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_chart2.hxx"
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
