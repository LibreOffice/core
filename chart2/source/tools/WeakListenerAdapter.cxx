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

#include "WeakListenerAdapter.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

WeakModifyListenerAdapter::WeakModifyListenerAdapter(
    const uno::WeakReference< util::XModifyListener > & xListener ) :
        WeakListenerAdapter< ::com::sun::star::util::XModifyListener >( xListener )
{}

WeakModifyListenerAdapter::~WeakModifyListenerAdapter()
{}

void SAL_CALL WeakModifyListenerAdapter::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    Reference< util::XModifyListener > xModListener( getListener() );
    if( xModListener.is())
        xModListener->modified( aEvent );
}

WeakSelectionChangeListenerAdapter::WeakSelectionChangeListenerAdapter(
    const Reference< view::XSelectionChangeListener > & xListener ) :
        WeakListenerAdapter< ::com::sun::star::view::XSelectionChangeListener >( xListener )
{}

WeakSelectionChangeListenerAdapter::~WeakSelectionChangeListenerAdapter()
{}

void SAL_CALL WeakSelectionChangeListenerAdapter::selectionChanged( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    Reference< view::XSelectionChangeListener > xSelChgListener( getListener() );
    if( xSelChgListener.is())
        xSelChgListener->selectionChanged( aEvent );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
