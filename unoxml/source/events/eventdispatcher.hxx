
//#include <multimap>
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
typedef std::multimap< xmlNodePtr, Reference< XEventListener> > ListenerMap;
typedef std::map<EventType, ListenerMap*> TypeListenerMap;typedef std::vector<ListenerMap::value_type> ListenerPairVector;

class CEventDispatcher
{
private:
    static TypeListenerMap captureListeners;
    static TypeListenerMap targetListeners;

public:
    static sal_Bool dispatchEvent(xmlNodePtr aNode, const Reference< XEvent >& aEvent);

    static void addListener(
        xmlNodePtr pNode, 
        EventType aType,
        const Reference<XEventListener>& aListener,
        sal_Bool bCapture);

    static void removeListener(
        xmlNodePtr pNode,
        EventType aType,
        const Reference<XEventListener>& aListener,
        sal_Bool bCapture);

    static void callListeners(
        xmlNodePtr pNode, 
        EventType aType,
        const Reference< XEvent >& xEvent, 
        sal_Bool bCapture);
};
}}