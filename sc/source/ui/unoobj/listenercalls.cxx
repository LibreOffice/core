/*************************************************************************
 *
 *  $RCSfile: listenercalls.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:51:19 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
        std::list<ScUnoListenerEntry>::iterator aItr = aEntries.begin();
        while ( aItr != aEntries.end() )
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

