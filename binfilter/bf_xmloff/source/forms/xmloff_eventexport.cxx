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

#ifndef _XMLOFF_FORMS_EVENTEXPORT_HXX_
#include "eventexport.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
namespace binfilter {

//.........................................................................
namespace xmloff
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OEventDescriptorMapper
    //=====================================================================
    //---------------------------------------------------------------------
    OEventDescriptorMapper::OEventDescriptorMapper(const Sequence< ScriptEventDescriptor >& _rEvents)
    {
        sal_Int32 nEvents = _rEvents.getLength();

        // translate the events
        const ScriptEventDescriptor* pEvents = _rEvents.getConstArray();
        ::rtl::OUString sName;
        ::rtl::OUString sLibrary, sLocalMacroName;
        sal_Int32 nNameSeparatorPos(-1);
        for (sal_Int32 i=0; i<nEvents; ++i, ++pEvents)
        {
            // the name of the event is build from listener interface and listener method name
            sName = pEvents->ListenerType;
            sName += EVENT_NAME_SEPARATOR;
            sName += pEvents->EventMethod;

            Sequence< PropertyValue >& rMappedEvent = m_aMappedEvents[sName];

            sLocalMacroName = pEvents->ScriptCode;
            sLibrary = ::rtl::OUString();
            if ( 0 == pEvents->ScriptType.compareToAscii( EVENT_STARBASIC ) )
            {	// for StarBasic, the library name is part of the ScriptCode
                sal_Int32 nPrefixLen = sLocalMacroName.indexOf( ':' );
                DBG_ASSERT( 0 <= nPrefixLen, "OEventDescriptorMapper::OEventDescriptorMapper: invalid script code prefix!" );
                if ( 0 <= nPrefixLen )
                {
                    // the export handler for StarBasic expects "StarOffice", not "application" for application modules ...
                    sLibrary = sLocalMacroName.copy( 0, nPrefixLen );
                    if ( sLibrary.equalsAscii( EVENT_APPLICATION ) )
                        sLibrary = EVENT_STAROFFICE;

                    sLocalMacroName = sLocalMacroName.copy( nPrefixLen + 1 );
                }
            }

            // tree property values to describe one event ...
            rMappedEvent.realloc( sLibrary.getLength() ? 3 : 2 );

            // ... the type
            rMappedEvent[0] = PropertyValue(EVENT_TYPE, -1, makeAny(pEvents->ScriptType), PropertyState_DIRECT_VALUE);

            // and the macro name
            rMappedEvent[1] = PropertyValue(EVENT_LOCALMACRONAME, -1, makeAny(sLocalMacroName), PropertyState_DIRECT_VALUE);

            // the library
            if ( sLibrary.getLength() )
                rMappedEvent[2] = PropertyValue(EVENT_LIBRARY, -1, makeAny(sLibrary), PropertyState_DIRECT_VALUE);
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEventDescriptorMapper::replaceByName( const ::rtl::OUString& _rNamee, const Any& _rElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        throw IllegalArgumentException(
            ::rtl::OUString::createFromAscii("replacing is not implemented for this wrapper class."), static_cast< ::cppu::OWeakObject* >(this), 1);
    }

    //---------------------------------------------------------------------
    Any SAL_CALL OEventDescriptorMapper::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ConstMapString2PropertyValueSequenceIterator aPos = m_aMappedEvents.find(_rName);
        if (m_aMappedEvents.end() == aPos)
            throw NoSuchElementException(
                ::rtl::OUString::createFromAscii("There is no element named ") += _rName,
                static_cast< ::cppu::OWeakObject* >(this));

        return makeAny(aPos->second);
    }

    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OEventDescriptorMapper::getElementNames(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aReturn(m_aMappedEvents.size());
        ::rtl::OUString* pReturn = aReturn.getArray();
        for	(	ConstMapString2PropertyValueSequenceIterator aCollect = m_aMappedEvents.begin();
                aCollect != m_aMappedEvents.end();
                ++aCollect, ++pReturn
            )
            *pReturn = aCollect->first;

        return aReturn;
    }

    //---------------------------------------------------------------------
    sal_Bool SAL_CALL OEventDescriptorMapper::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
    {
        ConstMapString2PropertyValueSequenceIterator aPos = m_aMappedEvents.find(_rName);
        return m_aMappedEvents.end() != aPos;
    }

    //---------------------------------------------------------------------
    Type SAL_CALL OEventDescriptorMapper::getElementType(  ) throw(RuntimeException)
    {
        return ::getCppuType(static_cast< PropertyValue* >(NULL));
    }

    //---------------------------------------------------------------------
    sal_Bool SAL_CALL OEventDescriptorMapper::hasElements(  ) throw(RuntimeException)
    {
        return 0 != m_aMappedEvents.size();
    }

//.........................................................................
}	// namespace xmloff
//.........................................................................


}//end of namespace binfilter
