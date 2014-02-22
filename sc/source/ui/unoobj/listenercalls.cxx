/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/util/XModifyListener.hpp>

#include "listenercalls.hxx"

using namespace com::sun::star;



ScUnoListenerCalls::ScUnoListenerCalls()
{
}

ScUnoListenerCalls::~ScUnoListenerCalls()
{
    OSL_ENSURE( aEntries.empty(), "unhandled listener calls remaining" );
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
            catch ( const uno::RuntimeException& )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
