/*************************************************************************
 *
 *  $RCSfile: eventimport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:36:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_FORMS_EVENTIMPORT_HXX_
#include "eventimport.hxx"
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::container;

    //=====================================================================
    //= OFormEventsImportContext
    //=====================================================================
    //---------------------------------------------------------------------
    OFormEventsImportContext::OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName, IEventAttacher& _rEventAttacher)
        :XMLEventsImportContext(_rImport, _nPrefix, _rLocalName)
        ,m_rEventAttacher(_rEventAttacher)
    {
    }

    //---------------------------------------------------------------------
    void OFormEventsImportContext::EndElement()
    {
        Sequence< ScriptEventDescriptor > aTranslated(aCollectEvents.size());
        ScriptEventDescriptor* pTranslated = aTranslated.getArray();

        // loop through the collected events and translate them
        const PropertyValue* pEventDescription;
        const PropertyValue* pEventDescriptionEnd;
        sal_Int32 nSeparatorPos = -1;
        for (   EventsVector::const_iterator aEvent = aCollectEvents.begin();
                aEvent != aCollectEvents.end();
                ++aEvent, ++pTranslated
            )
        {
            // the name of the event is built from ListenerType::EventMethod
            nSeparatorPos = aEvent->first.indexOf(EVENT_NAME_SEPARATOR);
            OSL_ENSURE(-1 != nSeparatorPos, "OFormEventsImportContext::EndElement: invalid (unrecodnized) event name!");
            pTranslated->ListenerType = aEvent->first.copy(0, nSeparatorPos);
            pTranslated->EventMethod = aEvent->first.copy(nSeparatorPos + 2);

            // the local macro name and the event type are specified as properties
            pEventDescription = aEvent->second.getConstArray();
            pEventDescriptionEnd = pEventDescription + aEvent->second.getLength();
            for (;pEventDescription != pEventDescriptionEnd; ++pEventDescription)
            {
                if (0 == pEventDescription->Name.compareToAscii(EVENT_LOCALMACRONAME))
                    pEventDescription->Value >>= pTranslated->ScriptCode;
                if (0 == pEventDescription->Name.compareToAscii(EVENT_TYPE))
                    pEventDescription->Value >>= pTranslated->ScriptType;
            }
        }

        // register the events
        m_rEventAttacher.registerEvents(aTranslated);

        XMLEventsImportContext::EndElement();
    }

    //=====================================================================
    //= ODefaultEventAttacherManager
    //=====================================================================
    //-------------------------------------------------------------------------
    void ODefaultEventAttacherManager::registerEvents(const Reference< XPropertySet >& _rxElement,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(m_aEvents.end() == m_aEvents.find(_rxElement),
            "ODefaultEventAttacherManager::registerEvents: already have events for this object!");
        // for the moment, only remember the script events
        m_aEvents[_rxElement] = _rEvents;
    }

    //-------------------------------------------------------------------------
    void ODefaultEventAttacherManager::setEvents(const Reference< XIndexAccess >& _rxContainer)
    {
        Reference< XEventAttacherManager > xEventManager(_rxContainer, UNO_QUERY);
        if (!xEventManager.is())
        {
            OSL_ENSURE(sal_False, "ODefaultEventAttacherManager::setEvents: invalid argument!");
            return;
        }

        // loop through all elements
        sal_Int32 nCount = _rxContainer->getCount();
        Reference< XPropertySet > xCurrent;
        ConstMapPropertySet2ScriptSequenceIterator aRegisteredEventsPos;
        for (sal_Int32 i=0; i<nCount; ++i)
        {
            ::cppu::extractInterface(xCurrent, _rxContainer->getByIndex(i));
            if (xCurrent.is())
            {
                aRegisteredEventsPos = m_aEvents.find(xCurrent);
                if (m_aEvents.end() != aRegisteredEventsPos)
                    xEventManager->registerScriptEvents(i, aRegisteredEventsPos->second);
            }
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/01/02 15:55:58  fs
 *  initial checkin - helper for importing script events
 *
 *
 *  Revision 1.0 21.12.00 15:21:15  fs
 ************************************************************************/

