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

#include "dlgedlist.hxx"
#include "dlgedobj.hxx"

namespace basctl
{

// DlgEdPropListenerImpl
DlgEdPropListenerImpl::DlgEdPropListenerImpl (DlgEdObj& rObj) :
    rDlgEdObj(rObj)
{
}

DlgEdPropListenerImpl::~DlgEdPropListenerImpl()
{
}

// XEventListener
void SAL_CALL DlgEdPropListenerImpl::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException, std::exception)
{
}

// XPropertyChangeListener
void SAL_CALL DlgEdPropListenerImpl::propertyChange( const  css::beans::PropertyChangeEvent& evt ) throw( css::uno::RuntimeException, std::exception)
{
    rDlgEdObj._propertyChange( evt );
}

// DlgEdEvtContListenerImpl
DlgEdEvtContListenerImpl::DlgEdEvtContListenerImpl (DlgEdObj& rObj) :
    rDlgEdObj(rObj)
{
}

DlgEdEvtContListenerImpl::~DlgEdEvtContListenerImpl()
{
}

// XEventListener
void SAL_CALL DlgEdEvtContListenerImpl::disposing( const  css::lang::EventObject& ) throw( css::uno::RuntimeException, std::exception)
{
}

// XContainerListener
void SAL_CALL DlgEdEvtContListenerImpl::elementInserted(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception)
{
    rDlgEdObj._elementInserted( Event );
}

void SAL_CALL DlgEdEvtContListenerImpl::elementReplaced(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception)
{
    rDlgEdObj._elementReplaced( Event );
}

void SAL_CALL DlgEdEvtContListenerImpl::elementRemoved(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception)
{
    rDlgEdObj._elementRemoved( Event );
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
