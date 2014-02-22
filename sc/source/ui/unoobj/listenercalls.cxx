/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
                
                
            }

            
            
            

            aItr = aEntries.erase(aItr);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
