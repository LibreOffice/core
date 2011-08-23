/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <map>
#include <vector>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/events/EventType.hpp>
#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include "event.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM { namespace events 
{

typedef std::vector< xmlNodePtr > NodeVector;
typedef std::multimap< xmlNodePtr, Reference< com::sun::star::xml::dom::events::XEventListener> > ListenerMap;
typedef std::map<OUString, ListenerMap*> TypeListenerMap;
typedef std::vector<ListenerMap::value_type> ListenerPairVector;

class CEventDispatcher
{
private:
    static TypeListenerMap captureListeners;
    static TypeListenerMap targetListeners;

public:
    static sal_Bool dispatchEvent(xmlNodePtr aNode, const Reference< XEvent >& aEvent);

    static void addListener(
        xmlNodePtr pNode, 
        OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    static void removeListener(
        xmlNodePtr pNode,
        OUString aType,
        const Reference<com::sun::star::xml::dom::events::XEventListener>& aListener,
        sal_Bool bCapture);

    static void callListeners(
        xmlNodePtr pNode, 
        OUString aType,
        const Reference< XEvent >& xEvent, 
        sal_Bool bCapture);
};
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
