/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/rtti.hxx>
#include <svtools/unoevent.hxx>
#include <svl/macitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using ::com::sun::star::container::NoSuchElementException;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::beans::PropertyValue;
using ::cppu::WeakImplHelper2;


const sal_Char sAPI_ServiceName[] = "com.sun.star.container.XNameReplace";
const sal_Char sAPI_SvDetachedEventDescriptor[] = "SvDetachedEventDescriptor";


SvBaseEventDescriptor::SvBaseEventDescriptor( const SvEventDescription* pSupportedMacroItems ) :
        sEventType("EventType"),
        sMacroName("MacroName"),
        sLibrary("Library"),
        sStarBasic("StarBasic"),
        sJavaScript("JavaScript"),
        sScript("Script"),
        sNone("None"),
        sServiceName(sAPI_ServiceName),
        sEmpty(),
        mpSupportedMacroItems(pSupportedMacroItems),
        mnMacroItems(0)
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

    
    if (0 == nMacroID)
        throw NoSuchElementException();
    if (rElement.getValueType() != getElementType())
        throw IllegalArgumentException();

    
    Sequence<PropertyValue> aSequence;
    rElement >>= aSequence;

    
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

    
    if (0 == nMacroID)
        throw NoSuchElementException();

    
    Any aAny;
    SvxMacro aMacro( sEmpty, sEmpty );
    getByName(aMacro, nMacroID);
    getAnyFromMacro(aAny, aMacro);
    return aAny;
}

Sequence<OUString> SvBaseEventDescriptor::getElementNames()
    throw(RuntimeException)
{
    
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
    return cppu::supportsService(this, rServiceName);
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
    
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if( rName.equalsAscii(mpSupportedMacroItems[i].mpEventName))
        {
            return mpSupportedMacroItems[i].mnEvent;
        }
    }

    
    return 0;
}

sal_uInt16 SvBaseEventDescriptor::getMacroID(const OUString& rName) const
{
    return mapNameToEventID(rName);
}

void SvBaseEventDescriptor::getAnyFromMacro(Any& rAny,
                                       const SvxMacro& rMacro)
{
    sal_Bool bRetValueOK = sal_False;   

    if (rMacro.HasMacro())
    {
        switch (rMacro.GetScriptType())
        {
            case STARBASIC:
            {
                
                Sequence<PropertyValue> aSequence(3);
                Any aTmp;

                
                PropertyValue aTypeValue;
                aTypeValue.Name = sEventType;
                aTmp <<= sStarBasic;
                aTypeValue.Value = aTmp;
                aSequence[0] = aTypeValue;

                
                PropertyValue aNameValue;
                aNameValue.Name = sMacroName;
                OUString sNameTmp(rMacro.GetMacName());
                aTmp <<= sNameTmp;
                aNameValue.Value = aTmp;
                aSequence[1] = aNameValue;

                
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
            case EXTENDED_STYPE:
            {
                
                Sequence<PropertyValue> aSequence(2);
                Any aTmp;

                
                PropertyValue aTypeValue;
                aTypeValue.Name = sEventType;
                aTmp <<= sScript;
                aTypeValue.Value = aTmp;
                aSequence[0] = aTypeValue;

                
                PropertyValue aNameValue;
                aNameValue.Name = sScript;
                OUString sNameTmp(rMacro.GetMacName());
                aTmp <<= sNameTmp;
                aNameValue.Value = aTmp;
                aSequence[1] = aNameValue;

                rAny <<= aSequence;
                bRetValueOK = sal_True;
                break;
                        }
            case JAVASCRIPT:
            default:
                OSL_FAIL("not implemented");
        }
    }
    

    
    if (! bRetValueOK)
    {
        
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
    
    Sequence<PropertyValue> aSequence;
    rAny >>= aSequence;

    
    sal_Bool bTypeOK = sal_False;
    sal_Bool bNone = sal_False;     
    enum ScriptType eType = EXTENDED_STYPE;
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
            else if (sTmp.equals(sScript))
            {
                eType = EXTENDED_STYPE;
                bTypeOK = sal_True;
            }
            else if (sTmp.equals(sNone))
            {
                bNone = sal_True;
                bTypeOK = sal_True;
            }
            
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
        
    }

    if (bTypeOK)
    {
        if (bNone)
        {
            
            rMacro = SvxMacro( sEmpty, sEmpty );
        }
        else
        {
            if (eType == STARBASIC)
            {
                
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
                
                
                throw IllegalArgumentException();
            }
        }
    }
    else
    {
        
        throw IllegalArgumentException();
    }
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
    if( rItem.HasMacro( nEvent ) )
        rMacro = rItem.GetMacro(nEvent);
    else
    {
        SvxMacro aEmptyMacro(sEmpty, sEmpty);
        rMacro = aEmptyMacro;
    }
}


SvDetachedEventDescriptor::SvDetachedEventDescriptor(
    const SvEventDescription* pSupportedMacroItems) :
    SvBaseEventDescriptor(pSupportedMacroItems),
    sImplName(sAPI_SvDetachedEventDescriptor)
{
    
    aMacros = new SvxMacro*[mnMacroItems];

    
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        aMacros[i] = NULL;
    }
}

SvDetachedEventDescriptor::~SvDetachedEventDescriptor()
{
    
    for(sal_Int16 i = 0; i < mnMacroItems; i++)
    {
        if (NULL != aMacros[i])
            delete aMacros[i];
    }

    delete [] aMacros;
}

sal_Int16 SvDetachedEventDescriptor::getIndex(const sal_uInt16 nID) const
{
    
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
        throw IllegalArgumentException();

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
        throw NoSuchElementException();

    if( aMacros[nIndex] )
        rMacro = (*aMacros[nIndex]);
}

sal_Bool SvDetachedEventDescriptor::hasByName(
    const sal_uInt16 nEvent ) const     
        throw(IllegalArgumentException)
{
    sal_Int16 nIndex = getIndex(nEvent);
    if (-1 == nIndex)
        throw IllegalArgumentException();

    return (NULL == aMacros[nIndex]) ? sal_False : aMacros[nIndex]->HasMacro();
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
    copyMacrosFromTable(rMacroTable);
}

SvMacroTableEventDescriptor::~SvMacroTableEventDescriptor()
{
}

void SvMacroTableEventDescriptor::copyMacrosFromTable(
    const SvxMacroTableDtor& rMacroTable)
{
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
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
    for(sal_Int16 i = 0; mpSupportedMacroItems[i].mnEvent != 0; i++)
    {
        const sal_uInt16 nEvent = mpSupportedMacroItems[i].mnEvent;
        if (hasByName(nEvent))
        {
            SvxMacro& rMacro = rMacroTable.Insert(nEvent, SvxMacro(sEmpty, sEmpty));
            getByName(rMacro, nEvent);
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
