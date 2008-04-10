/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listenercalls.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <com/sun/star/util/XModifyListener.hpp>
#include <tools/debug.hxx>

#include "listenercalls.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScUnoListenerCalls::ScUnoListenerCalls()
{
}

ScUnoListenerCalls::~ScUnoListenerCalls()
{
    DBG_ASSERT( aEntries.empty(), "unhandled listener calls remaining" );
}

void ScUnoListenerCalls::Add( const uno::Reference<util::XModifyListener>& rListener,
                                const lang::EventObject& rEvent )
{
    if ( rListener.is() )
        aEntries.push_back( ScUnoListenerEntry( rListener, rEvent ) );
}

void ScUnoListenerCalls::ExecuteAndClear()
{
    //  Execute all stored calls and remove them from the list.
    //  During each modified() call, Add may be called again.
    //  These new calls are executed here, too.

    if (!aEntries.empty())
    {
        std::list<ScUnoListenerEntry>::iterator aItr(aEntries.begin());
        std::list<ScUnoListenerEntry>::iterator aEndItr(aEntries.end());
        while ( aItr != aEndItr )
        {
            ScUnoListenerEntry aEntry = *aItr;
            try
            {
                aEntry.xListener->modified( aEntry.aEvent );
            }
            catch ( uno::RuntimeException )
            {
                // the listener is an external object and may throw a RuntimeException
                // for reasons we don't know
            }

            //  New calls that are added during the modified() call are appended to the end
            //  of aEntries, so the loop will catch them, too (as long as erase happens
            //  after modified).

            aItr = aEntries.erase(aItr);
        }
    }
}

