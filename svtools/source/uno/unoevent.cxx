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

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <sfx2/event.hxx>
#include <svtools/unoevent.hxx>
#include <svl/macitem.hxx>

using namespace ::com::sun::star;
using namespace css::uno;

using css::container::NoSuchElementException;
using css::container::XNameReplace;
using css::lang::IllegalArgumentException;
using css::beans::PropertyValue;


constexpr OUString sAPI_ServiceName = u"com.sun.star.container.XNameReplace"_ustr;
constexpr OUString sEventType = u"EventType"_ustr;
constexpr OUString sMacroName = u"MacroName"_ustr;
constexpr OUString sLibrary = u"Library"_ustr;
constexpr OUString sStarBasic = u"StarBasic"_ustr;
constexpr OUString sScript = u"Script"_ustr;
constexpr OUString sNone = u"None"_ustr;

namespace {

void getAnyFromMacro(Any& rAny, const SvxMacro& rMacro)
{
    bool bRetValueOK = false;   // do we have a ret value?

    if (rMacro.HasMacro())
    {
        switch (rMacro.GetScriptType())
        {
            case STARBASIC:
            {
                // create sequence
                Sequence<PropertyValue> aSequence(
                      // create type
                    { comphelper::makePropertyValue(sEventType, sStarBasic),
                      // macro name
                      comphelper::makePropertyValue(sMacroName, rMacro.GetMacName()),
                      // library name
                      comphelper::makePropertyValue(sLibrary, rMacro.GetLibName()) });

                rAny <<= aSequence;
                bRetValueOK = true;
                break;
            }
            case EXTENDED_STYPE:
            {
                // create sequence
                Sequence<PropertyValue> aSequence(
                      // create type
                    { comphelper::makePropertyValue(sEventType, sScript),
                      // macro name
                      comphelper::makePropertyValue(sScript, rMacro.GetMacName()) });

                rAny <<= aSequence;
                bRetValueOK = true;
                break;
            }
            case JAVASCRIPT:
            default:
                OSL_FAIL("not implemented");
        }
    }
    // else: bRetValueOK not set

    // if we don't have a return value, make an empty one
    if ( bRetValueOK)
        return;

    // create "None" macro
    Sequence<PropertyValue> aSequence{ comphelper::makePropertyValue(sEventType, sNone) };
    rAny <<= aSequence;
}

/// @throws IllegalArgumentException
void getMacroFromAny(
    SvxMacro& rMacro,
    const Any& rAny)
{
    // get sequence
    Sequence<PropertyValue> aSequence;
    rAny >>= aSequence;

    // process ...
    bool bTypeOK = false;
    bool bNone = false;     // true if EventType=="None"
    enum ScriptType eType = EXTENDED_STYPE;
    OUString sScriptVal;
    OUString sMacroVal;
    OUString sLibVal;
    for (const PropertyValue& aValue : aSequence)
    {
        if (aValue.Name == sEventType)
        {
            OUString sTmp;
            aValue.Value >>= sTmp;
            if (sTmp == sStarBasic)
            {
                eType = STARBASIC;
                bTypeOK = true;
            }
            else if (sTmp == "JavaScript")
            {
                eType = JAVASCRIPT;
                bTypeOK = true;
            }
            else if (sTmp == sScript)
            {
                eType = EXTENDED_STYPE;
                bTypeOK = true;
            }
            else if (sTmp == sNone)
            {
                bNone = true;
                bTypeOK = true;
            }
            // else: unknown script type
        }
        else if (aValue.Name == sMacroName)
        {
            aValue.Value >>= sMacroVal;
        }
        else if (aValue.Name == sLibrary)
        {
            aValue.Value >>= sLibVal;
        }
        else if (aValue.Name == sScript)
        {
            aValue.Value >>= sScriptVal;
        }
        // else: unknown PropertyValue -> ignore
    }

    if (!bTypeOK)
    {
        // no valid type: abort
        throw IllegalArgumentException();
    }

    if (bNone)
    {
        // return empty macro
        rMacro = SvxMacro( u""_ustr, u""_ustr );
    }
    else
    {
        if (eType == STARBASIC)
        {
            // create macro and return
            rMacro = SvxMacro(sMacroVal, sLibVal, eType);
        }
        else if (eType == EXTENDED_STYPE)
        {
            rMacro = SvxMacro(sScriptVal, sScript);
        }
        else
        {
            // we can't process type: abort
            // TODO: JavaScript macros
            throw IllegalArgumentException();
        }
    }
}

}

SvBaseEventDescriptor::SvBaseEventDescriptor( const SvEventDescription* pSupportedMacroItems ) :
        mpSupportedMacroItems(pSupportedMacroItems),
        mnMacroItems(0)
{
    assert(pSupportedMacroItems != nullptr && "Need a list of supported events!");

    for( ; mpSupportedMacroItems[mnMacroItems].mnEvent != SvMacroItemId::NONE; mnMacroItems++) ;
}


SvBaseEventDescriptor::~SvBaseEventDescriptor()
{
}

void SvBaseEventDescriptor::replaceByName(
    const OUString& rName,
    const Any& rElement )
{
    SvMacroItemId nMacroID = getMacroID(rName);

    // error checking
    if (SvMacroItemId::NONE == nMacroID)
        throw NoSuchElementException();
    if (rElement.getValueType() != getElementType())
        throw IllegalArgumentException();

    // get sequence
    Sequence<PropertyValue> aSequence;
    rElement >>= aSequence;

    // perform replace (in subclass)
    SvxMacro aMacro(u""_ustr,u""_ustr);
    getMacroFromAny(aMacro, rElement);
    replaceByName(nMacroID, aMacro);
}

Any SvBaseEventDescriptor::getByName(
    const OUString& rName )
{
    SvMacroItemId nMacroID = getMacroID(rName);

    // error checking
    if (SvMacroItemId::NONE == nMacroID)
        throw NoSuchElementException();

    // perform get (in subclass)
    Any aAny;
    SvxMacro aMacro( u""_ustr, u""_ustr );
    getByName(aMacro, nMacroID);
    getAnyFromMacro(aAny, aMacro);
    return aAny;
}

Sequence<OUString> SvBaseEventDescriptor::getElementNames()
{
    // create and fill sequence
    Sequence<OUString> aSequence(mnMacroItems);
    auto aSequenceRange = asNonConstRange(aSequence);
    for( sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        aSequenceRange[i] = OUString::createFromAscii( mpSupportedMacroItems[i].mpEventName );
    }

    return aSequence;
}

sal_Bool SvBaseEventDescriptor::hasByName(
    const OUString& rName )
{
    SvMacroItemId nMacroID = getMacroID(rName);
    return (nMacroID != SvMacroItemId::NONE);
}

Type SvBaseEventDescriptor::getElementType()
{
    return cppu::UnoType<Sequence<PropertyValue>>::get();
}

sal_Bool SvBaseEventDescriptor::hasElements()
{
    return mnMacroItems != 0;
}

sal_Bool SvBaseEventDescriptor::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SvBaseEventDescriptor::getSupportedServiceNames()
{
    return { sAPI_ServiceName };
}

SvMacroItemId SvBaseEventDescriptor::mapNameToEventID(std::u16string_view rName) const
{
    // iterate over known event names
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if( o3tl::equalsAscii(rName, mpSupportedMacroItems[i].mpEventName))
        {
            return mpSupportedMacroItems[i].mnEvent;
        }
    }

    // not found -> return zero
    return SvMacroItemId::NONE;
}

SvMacroItemId SvBaseEventDescriptor::getMacroID(std::u16string_view rName) const
{
    return mapNameToEventID(rName);
}

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
    const SvMacroItemId nEvent,
    const SvxMacro& rMacro)
{
    SvxMacroItem aItem(getMacroItemWhich());
    aItem.SetMacroTable(getMacroItem().GetMacroTable());
    aItem.SetMacro(nEvent, rMacro);
    setMacroItem(aItem);
}

void SvEventDescriptor::getByName(
    SvxMacro& rMacro,
    const SvMacroItemId nEvent )
{
    const SvxMacroItem& rItem = getMacroItem();
    if (rItem.HasMacro(nEvent))
        rMacro = rItem.GetMacro(nEvent);
    else
        rMacro = SvxMacro(u""_ustr, u""_ustr);
}

SvDetachedEventDescriptor::SvDetachedEventDescriptor(
    const SvEventDescription* pSupportedMacroItems) :
    SvBaseEventDescriptor(pSupportedMacroItems)
{
    aMacros.resize(mnMacroItems);
}

SvDetachedEventDescriptor::~SvDetachedEventDescriptor()
{
}

sal_Int16 SvDetachedEventDescriptor::getIndex(const SvMacroItemId nID) const
{
    // iterate over supported events
    sal_Int16 nIndex = 0;
    while ( (mpSupportedMacroItems[nIndex].mnEvent != nID) &&
            (mpSupportedMacroItems[nIndex].mnEvent != SvMacroItemId::NONE)      )
    {
        nIndex++;
    }
    return (mpSupportedMacroItems[nIndex].mnEvent == nID) ? nIndex : -1;
}

OUString SvDetachedEventDescriptor::getImplementationName()
{
    return u"SvDetachedEventDescriptor"_ustr;
}


void SvDetachedEventDescriptor::replaceByName(
    const SvMacroItemId nEvent,
    const SvxMacro& rMacro)
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex)
        throw IllegalArgumentException();

    aMacros[nIndex].reset( new SvxMacro(rMacro.GetMacName(), rMacro.GetLibName(),
                                   rMacro.GetScriptType() ) );
}


void SvDetachedEventDescriptor::getByName(
    SvxMacro& rMacro,
    const SvMacroItemId nEvent )
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex )
        throw NoSuchElementException();

    if( aMacros[nIndex] )
        rMacro = *aMacros[nIndex];
}

bool SvDetachedEventDescriptor::hasById(
    const SvMacroItemId nEvent ) const     /// item ID of event
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex)
        throw IllegalArgumentException();

    return (nullptr != aMacros[nIndex]) && aMacros[nIndex]->HasMacro();
}


SvMacroTableEventDescriptor::SvMacroTableEventDescriptor(const SvEventDescription* pSupportedMacroItems) :
    SvDetachedEventDescriptor(pSupportedMacroItems)
{
}

SvMacroTableEventDescriptor::SvMacroTableEventDescriptor(
    const SvxMacroTableDtor& rMacroTable,
    const SvEventDescription* pSupportedMacroItems) :
        SvDetachedEventDescriptor(pSupportedMacroItems)
{
    assert(mpSupportedMacroItems);
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != SvMacroItemId::NONE; i++)
    {
        const SvMacroItemId nEvent = mpSupportedMacroItems[i].mnEvent;
        const SvxMacro* pMacro = rMacroTable.Get(nEvent);
        if (nullptr != pMacro)
            replaceByName(nEvent, *pMacro);
    }
}

SvMacroTableEventDescriptor::~SvMacroTableEventDescriptor()
{
}

void SvMacroTableEventDescriptor::copyMacrosIntoTable(
    SvxMacroTableDtor& rMacroTable)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != SvMacroItemId::NONE; i++)
    {
        const SvMacroItemId nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasById(nEvent))
        {
            SvxMacro& rMacro = rMacroTable.Insert(nEvent, SvxMacro(u""_ustr, u""_ustr));
            getByName(rMacro, nEvent);
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
