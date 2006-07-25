/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventexport.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-25 09:23:10 $
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
            {   // for StarBasic, the library name is part of the ScriptCode
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
            else
            {
                rMappedEvent.realloc( 2 );
                rMappedEvent[0] = PropertyValue(EVENT_TYPE, -1, makeAny(pEvents->ScriptType), PropertyState_DIRECT_VALUE);
                // and the macro name
                rMappedEvent[1] = PropertyValue(EVENT_SCRIPTURL, -1, makeAny(pEvents->ScriptCode), PropertyState_DIRECT_VALUE);
            }
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OEventDescriptorMapper::replaceByName( const ::rtl::OUString&, const Any& ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
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
        for (   ConstMapString2PropertyValueSequenceIterator aCollect = m_aMappedEvents.begin();
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
}   // namespace xmloff
//.........................................................................

