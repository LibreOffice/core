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

#include "eventimport.hxx"
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <osl/diagnose.h>
#include "strings.hxx"

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::container;

    //= OFormEventsImportContext
    OFormEventsImportContext::OFormEventsImportContext(SvXMLImport& _rImport, sal_uInt16 _nPrefix, const OUString& _rLocalName, IEventAttacher& _rEventAttacher)
        :XMLEventsImportContext(_rImport, _nPrefix, _rLocalName)
        ,m_rEventAttacher(_rEventAttacher)
    {
    }

    void OFormEventsImportContext::EndElement()
    {
        Sequence< ScriptEventDescriptor > aTranslated(aCollectEvents.size());
        ScriptEventDescriptor* pTranslated = aTranslated.getArray();

        // loop through the collected events and translate them
        sal_Int32 nSeparatorPos = -1;
        for (   EventsVector::const_iterator aEvent = aCollectEvents.begin();
                aEvent != aCollectEvents.end();
                ++aEvent, ++pTranslated
            )
        {
            // the name of the event is built from ListenerType::EventMethod
            nSeparatorPos = aEvent->first.indexOf(EVENT_NAME_SEPARATOR);
            OSL_ENSURE(-1 != nSeparatorPos, "OFormEventsImportContext::EndElement: invalid (unrecognized) event name!");
            pTranslated->ListenerType = aEvent->first.copy(0, nSeparatorPos);
            pTranslated->EventMethod = aEvent->first.copy(nSeparatorPos + sizeof(EVENT_NAME_SEPARATOR) - 1);

            OUString sLibrary;

            // the local macro name and the event type are specified as properties
            const PropertyValue* pEventDescription = aEvent->second.getConstArray();
            const PropertyValue* pEventDescriptionEnd = pEventDescription + aEvent->second.getLength();
            for (;pEventDescription != pEventDescriptionEnd; ++pEventDescription)
            {
                if (pEventDescription->Name == EVENT_LOCALMACRONAME ||
                    pEventDescription->Name == EVENT_SCRIPTURL)
                    pEventDescription->Value >>= pTranslated->ScriptCode;
                else if (pEventDescription->Name == EVENT_TYPE)
                    pEventDescription->Value >>= pTranslated->ScriptType;
                else if (pEventDescription->Name == EVENT_LIBRARY)
                    pEventDescription->Value >>= sLibrary;
            }

            if (pTranslated->ScriptType == EVENT_STARBASIC)
            {
                if (sLibrary == EVENT_STAROFFICE)
                    sLibrary = EVENT_APPLICATION;

                if ( !sLibrary.isEmpty() )
                {
                    // for StarBasic, the library is prepended
                    sal_Unicode cLibSeparator = ':';
                    sLibrary += OUString( &cLibSeparator, 1 );
                }
                sLibrary += pTranslated->ScriptCode;
                pTranslated->ScriptCode = sLibrary;
            }
        }

        // register the events
        m_rEventAttacher.registerEvents(aTranslated);

        XMLEventsImportContext::EndElement();
    }

    //= ODefaultEventAttacherManager

    ODefaultEventAttacherManager::~ODefaultEventAttacherManager()
    {
    }

    void ODefaultEventAttacherManager::registerEvents(const Reference< XPropertySet >& _rxElement,
        const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        OSL_ENSURE(m_aEvents.end() == m_aEvents.find(_rxElement),
            "ODefaultEventAttacherManager::registerEvents: already have events for this object!");
        // for the moment, only remember the script events
        m_aEvents[_rxElement] = _rEvents;
    }

    void ODefaultEventAttacherManager::setEvents(const Reference< XIndexAccess >& _rxContainer)
    {
        Reference< XEventAttacherManager > xEventManager(_rxContainer, UNO_QUERY);
        if (!xEventManager.is())
        {
            OSL_FAIL("ODefaultEventAttacherManager::setEvents: invalid argument!");
            return;
        }

        // loop through all elements
        sal_Int32 nCount = _rxContainer->getCount();
        Reference< XPropertySet > xCurrent;
        MapPropertySet2ScriptSequence::const_iterator aRegisteredEventsPos;
        for (sal_Int32 i=0; i<nCount; ++i)
        {
            xCurrent.set(_rxContainer->getByIndex(i), css::uno::UNO_QUERY);
            if (xCurrent.is())
            {
                aRegisteredEventsPos = m_aEvents.find(xCurrent);
                if (m_aEvents.end() != aRegisteredEventsPos)
                    xEventManager->registerScriptEvents(i, aRegisteredEventsPos->second);
            }
        }
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
