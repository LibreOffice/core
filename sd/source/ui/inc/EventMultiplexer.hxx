/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventMultiplexer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:04:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_TOOLS_EVENT_MULTIPLEXER_HXX
#define SD_TOOLS_EVENT_MULTIPLEXER_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#include <set>
#include <memory>

class Link;

namespace sd {
class ViewShellBase;
}

namespace sd { namespace tools {

class EventMultiplexerEvent
{
public:
    enum EventId {
        EID_DISPOSING = 1,
        EID_EDIT_VIEW_SELECTION,
        EID_SLIDE_SORTER_SELECTION,
        EID_CURRENT_PAGE,
        EID_MAIN_VIEW_REMOVED,
        EID_MAIN_VIEW_ADDED,
        EID_VIEW_REMOVED,
        EID_VIEW_ADDED,
        EID_EDIT_MODE,
        EID_PAGE_ORDER,
        EID_BEGIN_TEXT_EDIT,
        EID_END_TEXT_EDIT
    };

    const ViewShellBase& mrBase;
    EventId meEventId;
    void* mpUserData;

    EventMultiplexerEvent (
        const ViewShellBase& rBase,
        EventId eEventId,
        void* pUserData);
};

/** This convenience class joins multiple event broadcaster and forwards
    events to listeners.  It thus takes the burden from the listeners to
    register at all these broadcasters with their different boradcasting
    techniques and event types.

    When a listener is registered it can specify the types of events it
    wants to be informed of.  This can be done with code like the following:

    mrViewShellBase.GetEventMultiplexer().AddEventListener (
        LINK(this,MasterPagesSelector,EventMultiplexerListener),
        tools::EventMultiplexer::ET_MAIN_VIEW
        | tools::EventMultiplexer::ET_EDIT_MODE);

    Here mrViewShellBase is an instance of the ViewShellBase class.  The
    listener will be called when the main view is switched or its edit mode
    changes.  The later is not possible for all types of ViewShell but then
    these events will not be sent.
*/
class EventMultiplexer
{
public:
    /** The event types are internally converted to bits in a sal_uInt32 so
        there may be not more than 32 of them.
    */
    typedef sal_uInt32 EventType;
    typedef sal_uInt32 EventTypeSet;
    static const EventTypeSet ET_DISPOSING;
    static const EventTypeSet ET_EDIT_VIEW_SELECTION;
    static const EventTypeSet ET_SLIDE_SORTER_SELECTION;
    static const EventTypeSet ET_CURRENT_PAGE;
    static const EventTypeSet ET_MAIN_VIEW;
    static const EventTypeSet ET_VIEW;
    static const EventTypeSet ET_EDIT_MODE;
    static const EventTypeSet ET_PAGE_ORDER;
    static const EventTypeSet ET_TEXT_EDIT;

    static const EventTypeSet ETS_EMPTY_SET;
    static const EventTypeSet ETS_FULL_SET;

    EventMultiplexer (ViewShellBase& rBase);
    ~EventMultiplexer (void);

    /** Add an event listener that will be informed about the event types
        specified in rEventTypes.
    */
    void AddEventListener (
        Link& rCallback,
        EventTypeSet aEventTypeSet = ETS_FULL_SET);

    void RemoveEventListener (
        Link& rCallback,
        EventTypeSet aEventTypeSet = ETS_FULL_SET);

    void MultiplexEvent(
        EventMultiplexerEvent::EventId eEventId,
        void* pUserData = 0
        );
private:
    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;
};

} } // end of namespace ::sd::tools

#endif
