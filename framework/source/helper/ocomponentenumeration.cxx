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

#include <helper/ocomponentenumeration.hxx>

#include <vcl/svapp.hxx>
#include <sal/log.hxx>

namespace framework{

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::cppu;

//  constructor

OComponentEnumeration::OComponentEnumeration( std::vector< css::uno::Reference< XComponent > >&& seqComponents )
        :   m_nPosition     ( 0                             )   // 0 is the first position for a valid list and the right value for an invalid list to!
        ,   m_seqComponents ( std::move(seqComponents)      )
{}

//  destructor

OComponentEnumeration::~OComponentEnumeration()
{
    // Reset instance, free memory...
    impl_resetObject();
}

//  XEventListener
void SAL_CALL OComponentEnumeration::disposing( const EventObject& aEvent )
{
    SolarMutexGuard g;

    // Safe impossible cases
    // This method is not specified for all incoming parameters.
    SAL_WARN_IF( !aEvent.Source.is(), "fwk", "OComponentEnumeration::disposing(): Invalid parameter detected!" );

    // Reset instance to defaults, release references and free memory.
    impl_resetObject();
}

//  XEnumeration
sal_Bool SAL_CALL OComponentEnumeration::hasMoreElements()
{
    SolarMutexGuard g;

    // First position in a valid list is 0.
    // => The last one is getLength() - 1!
    // m_nPosition's current value is the position for the next element, which will be return, if user call "nextElement()"
    // => We have more elements if current position less than the length of the list!
    return ( m_nPosition < static_cast<sal_uInt32>(m_seqComponents.size()) );
}

//  XEnumeration

Any SAL_CALL OComponentEnumeration::nextElement()
{
    SolarMutexGuard g;

    // If we have no elements or end of enumeration is arrived ...
    if ( !hasMoreElements() )
    {
        // .. throw an exception!
        throw NoSuchElementException();
    }

    // Else; Get next element from list ...
    Any aComponent;
    aComponent <<= m_seqComponents[m_nPosition];
    // ... and step to next element!
    ++m_nPosition;

    // Return listitem.
    return aComponent;
}

//  protected method

void OComponentEnumeration::impl_resetObject()
{
    // Attention:
    // Write this for multiple calls - NOT AT THE SAME TIME - but for more than one call again)!
    // It exist two ways to call this method. From destructor and from disposing().
    // I can't say, which one is the first. Normally the disposing-call - but other way...

    // Delete list of components.
    m_seqComponents.clear();
    // Reset position in list.
    // The list has no elements anymore. m_nPosition is normally the current position in list for nextElement!
    // But a position of 0 in a list of 0 items is an invalid state. This constellation can't work in future.
    // End of enumeration is arrived!
    // (see hasMoreElements() for more details...)
    m_nPosition = 0;
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
