/*************************************************************************
 *
 *  $RCSfile: unoevent.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-14 16:07:12 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _SVTOOLS_UNOEVENT_HXX_
#include "unoevent.hxx"
#endif

#define ITEMID_MACRO    0

#ifndef _SFXMACITEM_HXX
#include "macitem.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::PropertyValue;
using ::cppu::WeakImplHelper2;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


const sal_Char sAPI_ServiceName[] = "com.sun.star.container.XNameReplace";
const sal_Char sAPI_SvDetachedEventDescriptor[] = "SvDetachedEventDescriptor";

//
// SvBaseEventDescriptor
//

SvBaseEventDescriptor::SvBaseEventDescriptor( const SvEventDescription* pSupportedMacroItems ) :
        sServiceName(RTL_CONSTASCII_USTRINGPARAM(sAPI_ServiceName)),
        mpSupportedMacroItems(pSupportedMacroItems),
        mnMacroItems(0),
        sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
        sMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName")),
        sLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
        sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic")),
        sJavaScript(RTL_CONSTASCII_USTRINGPARAM("JavaScript")),
        sScript(RTL_CONSTASCII_USTRINGPARAM("Script")),
        sNone(RTL_CONSTASCII_USTRINGPARAM("None")),
        sEmpty()
{
    DBG_ASSERT(pSupportedMacroItems != NULL, "Need a list of supported events!");

    for( ; mpSupportedMacroItems[mnMacroItems].mnEvent != 0; mnMacroItems++) ;
}


SvBaseEventDescriptor::~SvBaseEventDescriptor()
{
}

void SvBaseEventDescriptor::replaceByName(
    const OUString& rName,
    const Any& rElement )
    throw(
        IllegalArgumentException,
        NoSuchElementException,
        WrappedTargetException,
        RuntimeException)
{
    sal_uInt16 nMacroID = getMacroID(rName);

    // error checking
    if (0 == nMacroID)
        throw new NoSuchElementException();
    if (rElement.getValueType() != getElementType())
        throw new IllegalArgumentException();

    // get sequence
    Sequence<PropertyValue> aSequence;
    rElement >>= aSequence;

    // perform replace (in subclass)
    SvxMacro aMacro(sEmpty,sEmpty);
    getMacroFromAny(aMacro, rElement);
    replaceByName(nMacroID, aMacro);
}

Any SvBaseEventDescriptor::getByName(
    const OUString& rName )
    throw(
        NoSuchElementException,
        WrappedTargetException,
        RuntimeException)
{
    sal_uInt16 nMacroID = getMacroID(rName);

    // error checking
    if (0 == nMacroID)
        throw new NoSuchElementException();

    // perform get (in subclass)
    Any aAny;
    SvxMacro aMacro( sEmpty, sEmpty );
    getByName(aMacro, nMacroID);
    getAnyFromMacro(aAny, aMacro);
    return aAny;
}

Sequence<OUString> SvBaseEventDescriptor::getElementNames()
    throw(RuntimeException)
{
    // create and fill sequence
    Sequence<OUString> aSequence(mnMacroItems);
    for( sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        aSequence[i] = OUString::createFromAscii( mpSupportedMacroItems[i].mpEventName );
    }

    return aSequence;
}

sal_Bool SvBaseEventDescriptor::hasByName(
    const OUString& rName )
    throw(RuntimeException)
{
    sal_uInt16 nMacroID = getMacroID(rName);
    return (nMacroID != 0);
}

Type SvBaseEventDescriptor::getElementType()
    throw(RuntimeException)
{
    return ::getCppuType((Sequence<PropertyValue> *)0);
}

sal_Bool SvBaseEventDescriptor::hasElements()
    throw(RuntimeException)
{
    return mnMacroItems != 0;
}

sal_Bool SvBaseEventDescriptor::supportsService(const OUString& rServiceName)
    throw(RuntimeException)
{
    return sServiceName.equals(rServiceName);
}

Sequence<OUString> SvBaseEventDescriptor::getSupportedServiceNames(void)
    throw(RuntimeException)
{
    Sequence<OUString> aSequence(1);
    aSequence[0] = sServiceName;

    return aSequence;
}

sal_uInt16 SvBaseEventDescriptor::mapNameToEventID(const OUString& rName) const
{
    // iterate over known event names
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if (0 == rName.compareToAscii(mpSupportedMacroItems[i].mpEventName))
        {
            return mpSupportedMacroItems[i].mnEvent;
        }
    }

    // not found -> return zero
    return 0;
}

OUString SvBaseEventDescriptor::mapEventIDToName(sal_uInt16 nPoolID) const
{
    // iterate over known event IDs
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if (nPoolID == mpSupportedMacroItems[i].mnEvent)
        {
            return OUString::createFromAscii(mpSupportedMacroItems[i].mpEventName);
        }
    }

    // not found -> return empty string
    OUString sEmpty;
    return sEmpty;
}

sal_uInt16 SvBaseEventDescriptor::getMacroID(const OUString& rName) const
{
    return mapNameToEventID(rName) != 0;
}

void SvBaseEventDescriptor::getAnyFromMacro(Any& rAny,
                                       const SvxMacro& rMacro)
{
    sal_Bool bRetValueOK = sal_False;   // do we have a ret value?

    if (rMacro.HasMacro())
    {
        switch (rMacro.GetScriptType())
        {
            case STARBASIC:
            {
                // create sequence
                Sequence<PropertyValue> aSequence(3);
                Any aTmp;

                // create type
                PropertyValue aTypeValue;
                aTypeValue.Name = sEventType;
                aTmp <<= sStarBasic;
                aTypeValue.Value = aTmp;
                aSequence[0] = aTypeValue;

                // macro name
                PropertyValue aNameValue;
                aNameValue.Name = sMacroName;
                OUString sNameTmp(rMacro.GetMacName());
                aTmp <<= sNameTmp;
                aNameValue.Value = aTmp;
                aSequence[1] = aNameValue;

                // library name
                PropertyValue aLibValue;
                aLibValue.Name = sLibrary;
                OUString sLibTmp(rMacro.GetLibName());
                aTmp <<= sLibTmp;
                aLibValue.Value = aTmp;
                aSequence[2] = aLibValue;

                rAny <<= aSequence;
                bRetValueOK = sal_True;
                break;
            }

            case JAVASCRIPT:
            case EXTENDED_STYPE:
            default:
                DBG_ERROR("not implemented");
        }
    }
    // else: bRetValueOK not set

    // if we don't have a return value, make an empty one
    if (! bRetValueOK)
    {
        // create "None" macro
        Sequence<PropertyValue> aSequence(1);

        PropertyValue aKindValue;
        aKindValue.Name = sEventType;
        Any aTmp;
        aTmp <<= sNone;
        aKindValue.Value = aTmp;
        aSequence[0] = aKindValue;

        rAny <<= aSequence;
        bRetValueOK = sal_True;
    }
}


void SvBaseEventDescriptor::getMacroFromAny(
    SvxMacro& rMacro,
    const Any& rAny)
        throw ( IllegalArgumentException )
{
    // get sequence
    Sequence<PropertyValue> aSequence;
    rAny >>= aSequence;

    // process ...
    sal_Bool bTypeOK = sal_False;
    sal_Bool bNone = sal_False;     // true if EventType=="None"
    enum ScriptType eType;
    OUString sScriptVal;
    OUString sMacroVal;
    OUString sLibVal;
    sal_Int32 nCount = aSequence.getLength();
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        PropertyValue& aValue = aSequence[i];
        if (aValue.Name.equals(sEventType))
        {
            OUString sTmp;
            aValue.Value >>= sTmp;
            if (sTmp.equals(sStarBasic))
            {
                eType = STARBASIC;
                bTypeOK = sal_True;
            }
            else if (sTmp.equals(sJavaScript))
            {
                eType = JAVASCRIPT;
                bTypeOK = sal_True;
            }
            else if (sTmp.equals(sNone))
            {
                bNone = sal_True;
                bTypeOK = sal_True;
            }
            // else: unknown script type
        }
        else if (aValue.Name.equals(sMacroName))
        {
            aValue.Value >>= sMacroVal;
        }
        else if (aValue.Name.equals(sLibrary))
        {
            aValue.Value >>= sLibVal;
        }
        else if (aValue.Name.equals(sScript))
        {
            aValue.Value >>= sScriptVal;
        }
        // else: unknown PropertyValue -> ignore
    }

    if (bTypeOK)
    {
        if (bNone)
        {
            // return empty macro
            rMacro = SvxMacro( sEmpty, sEmpty );
        }
        else
        {
            if (eType == STARBASIC)
            {
                // create macro and return
                SvxMacro aMacro(sMacroVal, sLibVal, eType);
                rMacro = aMacro;
            }
            else
            {
                // we can't process type: abort
                // TODO: JavaScript macros
                throw new IllegalArgumentException();
            }
        }
    }
    else
    {
        // no valid type: abort
        throw new IllegalArgumentException();
    }
}




//
// SvEventDescriptor
//


SvEventDescriptor::SvEventDescriptor(
    XInterface& rParent,
    const SvEventDescription* pSupportedMacroItems) :
        SvBaseEventDescriptor(pSupportedMacroItems),
        xParentRef(&rParent)
{
}


SvEventDescriptor::~SvEventDescriptor()
{
    // automatically release xParentRef !
}

void SvEventDescriptor::replaceByName(
    const sal_uInt16 nEvent,
    const SvxMacro& rMacro)
        throw(
            IllegalArgumentException,
            NoSuchElementException,
            WrappedTargetException,
            RuntimeException)
{
    SvxMacroItem aItem(getMacroItemWhich());
    aItem.SetMacroTable(getMacroItem().GetMacroTable());
    aItem.SetMacro(nEvent, rMacro);
    setMacroItem(aItem);
}

void SvEventDescriptor::getByName(
    SvxMacro& rMacro,
    const sal_uInt16 nEvent )
        throw(
            NoSuchElementException,
            WrappedTargetException,
            RuntimeException)
{
    const SvxMacroItem& rItem = getMacroItem();
    if( !rItem.HasMacro( nEvent ) )
        throw NoSuchElementException();

    rMacro = rItem.GetMacro(nEvent);
}




//
// SvDetachedEventDescriptor
//

SvDetachedEventDescriptor::SvDetachedEventDescriptor(
    const SvEventDescription* pSupportedMacroItems) :
    SvBaseEventDescriptor(pSupportedMacroItems),
    sImplName(RTL_CONSTASCII_USTRINGPARAM(sAPI_SvDetachedEventDescriptor))
{
    // allocate aMacros
    aMacros = new SvxMacro*[mnMacroItems];

    // ... and initialize
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        aMacros[i] = NULL;
    }
}

SvDetachedEventDescriptor::~SvDetachedEventDescriptor()
{
    // delete contents of aMacros
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if (NULL != aMacros[i])
            delete aMacros[i];
    }

    delete aMacros;
}

sal_Int16 SvDetachedEventDescriptor::getIndex(const sal_uInt16 nID)
{
    // iterate over supported events
    sal_Int16 nIndex = 0;
    while ( (mpSupportedMacroItems[nIndex].mnEvent != nID) &&
            (mpSupportedMacroItems[nIndex].mnEvent != 0)      )
    {
        nIndex++;
    }
    return (mpSupportedMacroItems[nIndex].mnEvent == nID) ? nIndex : -1;
}

OUString SvDetachedEventDescriptor::getImplementationName()
    throw( ::com::sun::star::uno::RuntimeException )
{
    return sImplName;
}


void SvDetachedEventDescriptor::replaceByName(
    const sal_uInt16 nEvent,
    const SvxMacro& rMacro)
    throw(
        IllegalArgumentException,
        NoSuchElementException,
        WrappedTargetException,
        RuntimeException)
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex)
        throw new IllegalArgumentException();

    aMacros[nIndex] = new SvxMacro(rMacro.GetMacName(), rMacro.GetLibName(),
                                   rMacro.GetScriptType() );
}


void SvDetachedEventDescriptor::getByName(
    SvxMacro& rMacro,
    const sal_uInt16 nEvent )
    throw(
        NoSuchElementException,
        WrappedTargetException,
        RuntimeException)
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex )
        throw new NoSuchElementException();

    if( aMacros[nIndex] )
        rMacro = (*aMacros[nIndex]);
}

const sal_Bool SvDetachedEventDescriptor::hasByName(
    const sal_uInt16 nEvent )       /// item ID of event
        throw(IllegalArgumentException)
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex)
        throw new IllegalArgumentException();

    return (NULL == aMacros[nIndex]) ? sal_False : aMacros[nIndex]->HasMacro();
}


//
// SvMacroTableEventDescriptor
//

SvMacroTableEventDescriptor::SvMacroTableEventDescriptor(const SvEventDescription* pSupportedMacroItems) :
    SvDetachedEventDescriptor(pSupportedMacroItems)
{
}

SvMacroTableEventDescriptor::SvMacroTableEventDescriptor(
    const SvxMacroTableDtor& rMacroTable,
    const SvEventDescription* pSupportedMacroItems) :
        SvDetachedEventDescriptor(pSupportedMacroItems)
{
    copyMacrosFromTable(rMacroTable);
}

SvMacroTableEventDescriptor::~SvMacroTableEventDescriptor()
{
}

void SvMacroTableEventDescriptor::copyMacrosFromTable(
    const SvxMacroTableDtor& rMacroTable)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != NULL; i++)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* pMacro = rMacroTable.Get(nEvent);
        if (NULL != pMacro)
            replaceByName(nEvent, *pMacro);
    }

}

void SvMacroTableEventDescriptor::copyMacrosIntoTable(
    SvxMacroTableDtor& rMacroTable)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != NULL; i++)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasByName(nEvent))
        {
            SvxMacro* pMacro = new SvxMacro(sEmpty, sEmpty);
            getByName(*pMacro, nEvent);
            rMacroTable.Insert(nEvent, pMacro);
        }
    }
}



