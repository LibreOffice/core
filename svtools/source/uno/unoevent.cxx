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
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <svtools/unoevent.hxx>
#include <svl/macitem.hxx>

using namespace ::com::sun::star;
using namespace css::uno;

using css::container::NoSuchElementException;
using css::container::XNameReplace;
using css::lang::IllegalArgumentException;
using css::lang::WrappedTargetException;
using css::beans::PropertyValue;


static const char sAPI_ServiceName[] = "com.sun.star.container.XNameReplace";
static const char sEventType[] = "EventType";
static const char sMacroName[] = "MacroName";
static const char sLibrary[] = "Library";
static const char sStarBasic[] = "StarBasic";
static const char sScript[] = "Script";
static const char sNone[] = "None";

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
                Sequence<PropertyValue> aSequence(3);
                Any aTmp;

                // create type
                PropertyValue aTypeValue;
                aTypeValue.Name = sEventType;
                aTmp <<= OUString(sStarBasic);
                aTypeValue.Value = aTmp;
                aSequence[0] = aTypeValue;

                // macro name
                PropertyValue aNameValue;
                aNameValue.Name = sMacroName;
                const OUString& sNameTmp(rMacro.GetMacName());
                aTmp <<= sNameTmp;
                aNameValue.Value = aTmp;
                aSequence[1] = aNameValue;

                // library name
                PropertyValue aLibValue;
                aLibValue.Name = sLibrary;
                const OUString& sLibTmp(rMacro.GetLibName());
                aTmp <<= sLibTmp;
                aLibValue.Value = aTmp;
                aSequence[2] = aLibValue;

                rAny <<= aSequence;
                bRetValueOK = true;
                break;
            }
            case EXTENDED_STYPE:
            {
                // create sequence
                Sequence<PropertyValue> aSequence(2);
                Any aTmp;

                // create type
                PropertyValue aTypeValue;
                aTypeValue.Name = sEventType;
                aTmp <<= OUString(sScript);
                aTypeValue.Value = aTmp;
                aSequence[0] = aTypeValue;

                // macro name
                PropertyValue aNameValue;
                aNameValue.Name = sScript;
                const OUString& sNameTmp(rMacro.GetMacName());
                aTmp <<= sNameTmp;
                aNameValue.Value = aTmp;
                aSequence[1] = aNameValue;

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
    if (! bRetValueOK)
    {
        // create "None" macro
        Sequence<PropertyValue> aSequence(1);

        PropertyValue aKindValue;
        aKindValue.Name = sEventType;
        Any aTmp;
        aTmp <<= OUString(sNone);
        aKindValue.Value = aTmp;
        aSequence[0] = aKindValue;

        rAny <<= aSequence;
    }
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
    sal_Int32 nCount = aSequence.getLength();
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        PropertyValue& aValue = aSequence[i];
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

    if (bTypeOK)
    {
        if (bNone)
        {
            // return empty macro
            rMacro = SvxMacro( "", "" );
        }
        else
        {
            if (eType == STARBASIC)
            {
                // create macro and return
                SvxMacro aMacro(sMacroVal, sLibVal, eType);
                rMacro = aMacro;
            }
            else if (eType == EXTENDED_STYPE)
            {
                SvxMacro aMacro(sScriptVal, sScript);
                rMacro = aMacro;
            }
            else
            {
                // we can't process type: abort
                // TODO: JavaScript macros
                throw IllegalArgumentException();
            }
        }
    }
    else
    {
        // no valid type: abort
        throw IllegalArgumentException();
    }
}

}

SvBaseEventDescriptor::SvBaseEventDescriptor( const SvEventDescription* pSupportedMacroItems ) :
        mpSupportedMacroItems(pSupportedMacroItems),
        mnMacroItems(0)
{
    assert(pSupportedMacroItems != nullptr && "Need a list of supported events!");

    for( ; mpSupportedMacroItems[mnMacroItems].mnEvent != 0; mnMacroItems++) ;
}


SvBaseEventDescriptor::~SvBaseEventDescriptor()
{
}

void SvBaseEventDescriptor::replaceByName(
    const OUString& rName,
    const Any& rElement )
{
    sal_uInt16 nMacroID = getMacroID(rName);

    // error checking
    if (0 == nMacroID)
        throw NoSuchElementException();
    if (rElement.getValueType() != getElementType())
        throw IllegalArgumentException();

    // get sequence
    Sequence<PropertyValue> aSequence;
    rElement >>= aSequence;

    // perform replace (in subclass)
    SvxMacro aMacro("","");
    getMacroFromAny(aMacro, rElement);
    replaceByName(nMacroID, aMacro);
}

Any SvBaseEventDescriptor::getByName(
    const OUString& rName )
{
    sal_uInt16 nMacroID = getMacroID(rName);

    // error checking
    if (0 == nMacroID)
        throw NoSuchElementException();

    // perform get (in subclass)
    Any aAny;
    SvxMacro aMacro( "", "" );
    getByName(aMacro, nMacroID);
    getAnyFromMacro(aAny, aMacro);
    return aAny;
}

Sequence<OUString> SvBaseEventDescriptor::getElementNames()
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
{
    sal_uInt16 nMacroID = getMacroID(rName);
    return (nMacroID != 0);
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
    Sequence<OUString> aSequence { sAPI_ServiceName };

    return aSequence;
}

sal_uInt16 SvBaseEventDescriptor::mapNameToEventID(const OUString& rName) const
{
    // iterate over known event names
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if( rName.equalsAscii(mpSupportedMacroItems[i].mpEventName))
        {
            return mpSupportedMacroItems[i].mnEvent;
        }
    }

    // not found -> return zero
    return 0;
}

sal_uInt16 SvBaseEventDescriptor::getMacroID(const OUString& rName) const
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
    const sal_uInt16 nEvent,
    const SvxMacro& rMacro)
{
    SvxMacroItem aItem(getMacroItemWhich());
    aItem.SetMacroTable(getMacroItem().GetMacroTable());
    aItem.SetMacro(nEvent, rMacro);
    setMacroItem(aItem);
}

void SvEventDescriptor::getByName(
    SvxMacro& rMacro,
    const sal_uInt16 nEvent )
{
    const SvxMacroItem& rItem = getMacroItem();
    if( rItem.HasMacro( nEvent ) )
        rMacro = rItem.GetMacro(nEvent);
    else
    {
        SvxMacro aEmptyMacro("", "");
        rMacro = aEmptyMacro;
    }
}


SvDetachedEventDescriptor::SvDetachedEventDescriptor(
    const SvEventDescription* pSupportedMacroItems) :
    SvBaseEventDescriptor(pSupportedMacroItems),
    sImplName("SvDetachedEventDescriptor")
{
    aMacros.resize(mnMacroItems);
}

SvDetachedEventDescriptor::~SvDetachedEventDescriptor()
{
}

sal_Int16 SvDetachedEventDescriptor::getIndex(const sal_uInt16 nID) const
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
{
    return sImplName;
}


void SvDetachedEventDescriptor::replaceByName(
    const sal_uInt16 nEvent,
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
    const sal_uInt16 nEvent )
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex )
        throw NoSuchElementException();

    if( aMacros[nIndex] )
        rMacro = (*aMacros[nIndex]);
}

bool SvDetachedEventDescriptor::hasById(
    const sal_uInt16 nEvent ) const     /// item ID of event
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
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
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
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasById(nEvent))
        {
            SvxMacro& rMacro = rMacroTable.Insert(nEvent, SvxMacro("", ""));
            getByName(rMacro, nEvent);
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
