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

#include "eventexport.hxx"
#include "strings.hxx"
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    //= OEventDescriptorMapper
    OEventDescriptorMapper::OEventDescriptorMapper(const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        // translate the events
        OUString sLibrary, sLocalMacroName;
        for (const auto& rEvent : _rEvents)
        {
            // the name of the event is build from listener interface and listener method name
            OUString sName = rEvent.ListenerType
                + EVENT_NAME_SEPARATOR
                + rEvent.EventMethod;

            Sequence< PropertyValue >& rMappedEvent = m_aMappedEvents[sName];

            sLocalMacroName = rEvent.ScriptCode;
            sLibrary.clear();
            if (rEvent.ScriptType == EVENT_STARBASIC)
            {   // for StarBasic, the library name is part of the ScriptCode
                sal_Int32 nPrefixLen = sLocalMacroName.indexOf( ':' );
                SAL_WARN_IF( 0 > nPrefixLen, "xmloff", "OEventDescriptorMapper::OEventDescriptorMapper: invalid script code prefix!" );
                if ( 0 <= nPrefixLen )
                {
                    // the export handler for StarBasic expects "StarOffice", not "application" for application modules ...
                    sLibrary = sLocalMacroName.copy( 0, nPrefixLen );
                    if (sLibrary == EVENT_APPLICATION)
                        sLibrary = EVENT_STAROFFICE;

                    sLocalMacroName = sLocalMacroName.copy( nPrefixLen + 1 );
                }
                // tree property values to describe one event ...
                rMappedEvent.realloc( sLibrary.isEmpty() ? 2 : 3 );
                auto pMappedEvent = rMappedEvent.getArray();

                // ... the type
                pMappedEvent[0] = PropertyValue(EVENT_TYPE, -1, Any(rEvent.ScriptType), PropertyState_DIRECT_VALUE);

                // and the macro name
                pMappedEvent[1] = PropertyValue(EVENT_LOCALMACRONAME, -1, Any(sLocalMacroName), PropertyState_DIRECT_VALUE);

                // the library
                if ( !sLibrary.isEmpty() )
                    pMappedEvent[2] = PropertyValue(EVENT_LIBRARY, -1, Any(sLibrary), PropertyState_DIRECT_VALUE);
            }
            else
            {
                rMappedEvent = { PropertyValue(EVENT_TYPE, -1, Any(rEvent.ScriptType), PropertyState_DIRECT_VALUE),
                                 // and the macro name
                                 PropertyValue(EVENT_SCRIPTURL, -1, Any(rEvent.ScriptCode), PropertyState_DIRECT_VALUE) };
            }
        }
    }

    void SAL_CALL OEventDescriptorMapper::replaceByName( const OUString&, const Any& )
    {
        throw IllegalArgumentException(
            u"replacing is not implemented for this wrapper class."_ustr, getXWeak(), 1);
    }

    Any SAL_CALL OEventDescriptorMapper::getByName( const OUString& _rName )
    {
        MapString2PropertyValueSequence::const_iterator aPos = m_aMappedEvents.find(_rName);
        if (m_aMappedEvents.end() == aPos)
            throw NoSuchElementException(
                "There is no element named " + _rName,
                getXWeak());

        return Any(aPos->second);
    }

    Sequence< OUString > SAL_CALL OEventDescriptorMapper::getElementNames(  )
    {
        return comphelper::mapKeysToSequence(m_aMappedEvents);
    }

    sal_Bool SAL_CALL OEventDescriptorMapper::hasByName( const OUString& _rName )
    {
        MapString2PropertyValueSequence::const_iterator aPos = m_aMappedEvents.find(_rName);
        return m_aMappedEvents.end() != aPos;
    }

    Type SAL_CALL OEventDescriptorMapper::getElementType(  )
    {
        return ::cppu::UnoType<PropertyValue>::get();
    }

    sal_Bool SAL_CALL OEventDescriptorMapper::hasElements(  )
    {
        return !m_aMappedEvents.empty();
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
