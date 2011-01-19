/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef EVENT_EVENT_DISPATCHER_HXX
#define EVENT_EVENT_DISPATCHER_HXX

#include <map>
#include <vector>

#include <libxml/tree.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/events/EventType.hpp>
#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM {

class CDocument;

namespace events {

typedef std::vector< xmlNodePtr > NodeVector;
typedef std::multimap< xmlNodePtr, Reference< com::sun::star::xml::dom::events::XEventListener> > ListenerMap;
typedef std::map< ::rtl::OUString, ListenerMap*> TypeListenerMap;
typedef std::vector<ListenerMap::value_type> ListenerPairVector;

class CEventDispatcher
{
private:
    TypeListenerMap m_CaptureListeners;
    TypeListenerMap m_TargetListeners;

public:
    void addListener(
        xmlNodePtr pNode,
        ::rtl::OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    void removeListener(
        xmlNodePtr pNode,
        ::rtl::OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    void callListeners(
        xmlNodePtr const pNode,
        ::rtl::OUString aType,
        const Reference< XEvent >& xEvent,
        sal_Bool const bCapture) const;

    bool dispatchEvent(
        DOM::CDocument & rDocument,
        xmlNodePtr const pNode,
        Reference<XNode> const& xNode,
        Reference< XEvent > const& xEvent) const;
};

}}

#endif

