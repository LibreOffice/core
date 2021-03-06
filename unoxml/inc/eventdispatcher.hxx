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

#pragma once

#include <map>

#include <libxml/tree.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>

namespace osl { class Mutex; }

namespace DOM {

class CDocument;

namespace events {

typedef std::multimap< xmlNodePtr, css::uno::Reference< css::xml::dom::events::XEventListener> > ListenerMap;
typedef std::map<OUString, ListenerMap> TypeListenerMap;

class CEventDispatcher
{
private:
    TypeListenerMap m_CaptureListeners;
    TypeListenerMap m_TargetListeners;

public:
    void addListener(
        xmlNodePtr pNode,
        const OUString& aType,
        const css::uno::Reference<css::xml::dom::events::XEventListener>& aListener,
        bool bCapture);

    void removeListener(
        xmlNodePtr pNode,
        const OUString& aType,
        const css::uno::Reference<css::xml::dom::events::XEventListener>& aListener,
        bool bCapture);

    static void callListeners(
        TypeListenerMap const& rTMap,
        xmlNodePtr const pNode,
        const OUString& aType,
        const css::uno::Reference< css::xml::dom::events::XEvent >& xEvent);

    void dispatchEvent(
        DOM::CDocument & rDocument,
        ::osl::Mutex & rMutex,
        xmlNodePtr const pNode,
        css::uno::Reference<css::xml::dom::XNode> const& xNode,
        css::uno::Reference< css::xml::dom::events::XEvent > const& xEvent) const;

    ~CEventDispatcher();
};

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
