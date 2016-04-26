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


#include <memory>

#include <vcl/msgbox.hxx>
#include <svl/lstner.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbx.hxx>
#include <sot/storage.hxx>
#include <unotools/securityoptions.hxx>

#include <rtl/ustring.h>
#include <com/sun/star/uno/Any.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/evntconf.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxresid.hxx>
#include "eventsupplier.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>


using namespace com::sun::star;

SfxEventNamesList& SfxEventNamesList::operator=( const SfxEventNamesList& rTbl )
{
    DelDtor();
    for ( size_t i = 0, n = rTbl.size(); i < n; ++i )
    {
        SfxEventName* pTmp = rTbl.at( i );
        SfxEventName* pNew = new SfxEventName( *pTmp );
        aEventNamesList.push_back( pNew );
    }
    return *this;
}

void SfxEventNamesList::DelDtor()
{
    for (SfxEventName* i : aEventNamesList)
        delete i;
    aEventNamesList.clear();
}

bool SfxEventNamesItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SfxEventNamesList& rOwn = aEventsList;
    const SfxEventNamesList& rOther = static_cast<const SfxEventNamesItem&>( rAttr ).aEventsList;

    if ( rOwn.size() != rOther.size() )
        return false;

    for ( size_t nNo = 0, nCnt = rOwn.size(); nNo < nCnt; ++nNo )
    {
        const SfxEventName *pOwn = rOwn.at( nNo );
        const SfxEventName *pOther = rOther.at( nNo );
        if (    pOwn->mnId != pOther->mnId ||
                pOwn->maEventName != pOther->maEventName ||
                pOwn->maUIName != pOther->maUIName )
            return false;
    }

    return true;

}

bool SfxEventNamesItem::GetPresentation( SfxItemPresentation,
                                    SfxMapUnit,
                                    SfxMapUnit,
                                    OUString &rText,
                                    const IntlWrapper* ) const
{
    rText.clear();
    return false;
}

SfxPoolItem* SfxEventNamesItem::Clone( SfxItemPool *) const
{
    return new SfxEventNamesItem(*this);
}

SfxPoolItem* SfxEventNamesItem::Create(SvStream &, sal_uInt16) const
{
    OSL_FAIL("not streamable!");
    return new SfxEventNamesItem(Which());
}

SvStream& SfxEventNamesItem::Store(SvStream &rStream, sal_uInt16 ) const
{
    OSL_FAIL("not streamable!");
    return rStream;
}

sal_uInt16 SfxEventNamesItem::GetVersion( sal_uInt16 ) const
{
    OSL_FAIL("not streamable!");
    return 0;
}

void SfxEventNamesItem::AddEvent( const OUString& rName, const OUString& rUIName, sal_uInt16 nID )
{
    aEventsList.push_back( new SfxEventName( nID, rName, !rUIName.isEmpty() ? rUIName : rName ) );
}


uno::Any CreateEventData_Impl( const SvxMacro *pMacro )
{
/*
    This function converts a SvxMacro into an Any containing three
    properties. These properties are EventType and Script. Possible
    values for EventType ar StarBasic, JavaScript, ...
    The Script property should contain the URL to the macro and looks
    like "macro://./standard.module1.main()"

    If pMacro is NULL, we return an empty property sequence, so PropagateEvent_Impl
    can delete an event binding.
*/
    uno::Any aEventData;

    if ( pMacro )
    {
        if ( pMacro->GetScriptType() == STARBASIC )
        {
            uno::Sequence < beans::PropertyValue > aProperties(3);
            beans::PropertyValue *pValues = aProperties.getArray();

            pValues[ 0 ].Name = PROP_EVENT_TYPE;
            pValues[ 0 ].Value <<= OUString("STAR_BASIC");

            pValues[ 1 ].Name = PROP_LIBRARY;
            pValues[ 1 ].Value <<= pMacro->GetLibName();

            pValues[ 2 ].Name = PROP_MACRO_NAME;
            pValues[ 2 ].Value <<= pMacro->GetMacName();

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == EXTENDED_STYPE )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            pValues[ 0 ].Name = PROP_EVENT_TYPE;
            pValues[ 0 ].Value <<= pMacro->GetLibName();

            pValues[ 1 ].Name = PROP_SCRIPT;
            pValues[ 1 ].Value <<= pMacro->GetMacName();

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == JAVASCRIPT )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            pValues[ 0 ].Name = PROP_EVENT_TYPE;
            pValues[ 0 ].Value <<= OUString(SVX_MACRO_LANGUAGE_JAVASCRIPT);

            pValues[ 1 ].Name = PROP_MACRO_NAME;
            pValues[ 1 ].Value <<= pMacro->GetMacName();

            aEventData <<= aProperties;
        }
        else
        {
            SAL_WARN( "sfx.config", "CreateEventData_Impl(): ScriptType not supported!");
        }
    }
    else
    {
        uno::Sequence < beans::PropertyValue > aProperties;
        aEventData <<= aProperties;
    }

    return aEventData;
}


void PropagateEvent_Impl( SfxObjectShell *pDoc, const OUString& aEventName, const SvxMacro* pMacro )
{
    uno::Reference < document::XEventsSupplier > xSupplier;
    if ( pDoc )
    {
        xSupplier.set( pDoc->GetModel(), uno::UNO_QUERY );
    }
    else
    {
        xSupplier.set( frame::theGlobalEventBroadcaster::get(::comphelper::getProcessComponentContext()),
                       uno::UNO_QUERY );
    }

    if ( xSupplier.is() )
    {
        uno::Reference < container::XNameReplace > xEvents = xSupplier->getEvents();
        if ( !aEventName.isEmpty() )
        {
            uno::Any aEventData = CreateEventData_Impl( pMacro );

            try
            {
                xEvents->replaceByName( aEventName, aEventData );
            }
            catch( const css::lang::IllegalArgumentException& )
            {
                SAL_WARN( "sfx.config", "PropagateEvents_Impl: caught IllegalArgumentException" );
            }
            catch( const css::container::NoSuchElementException& )
            {
                SAL_WARN( "sfx.config", "PropagateEvents_Impl: caught NoSuchElementException" );
            }
        }
        else {
            SAL_INFO( "sfx.config", "PropagateEvents_Impl: Got unknown event" );
        }
    }
}


void SfxEventConfiguration::ConfigureEvent( const OUString& aName, const SvxMacro& rMacro, SfxObjectShell *pDoc )
{
    std::unique_ptr<SvxMacro> pMacro;
    if ( rMacro.HasMacro() )
        pMacro.reset( new SvxMacro( rMacro.GetMacName(), rMacro.GetLibName(), rMacro.GetScriptType() ) );
    PropagateEvent_Impl( pDoc ? pDoc : nullptr, aName, pMacro.get() );
}


SvxMacro* SfxEventConfiguration::ConvertToMacro( const css::uno::Any& rElement, SfxObjectShell* pDoc, bool bBlowUp )
{
    return SfxEvents_Impl::ConvertToMacro( rElement, pDoc, bBlowUp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
