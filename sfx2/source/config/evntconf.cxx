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


#include <boost/scoped_ptr.hpp>

#include <vcl/msgbox.hxx>
#include <svl/lstner.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbx.hxx>
#include <sot/storage.hxx>
#include <unotools/securityoptions.hxx>

#include <rtl/ustring.h>
#include <com/sun/star/uno/Any.hxx>
#include <framework/eventsconfiguration.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/evntconf.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>
#include "sfx2/sfxresid.hxx"
#include "eventsupplier.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/GlobalEventBroadcaster.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

// -----------------------------------------------------------------------
TYPEINIT1(SfxEventHint, SfxHint);
TYPEINIT1(SfxEventNamesItem, SfxPoolItem);
TYPEINIT1(SfxViewEventHint, SfxEventHint);

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
    for ( size_t i = 0, n = aEventNamesList.size(); i < n; ++i )
        delete aEventNamesList[ i ];
    aEventNamesList.clear();
}

int SfxEventNamesItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SfxEventNamesList& rOwn = aEventsList;
    const SfxEventNamesList& rOther = ( (SfxEventNamesItem&) rAttr ).aEventsList;

    if ( rOwn.size() != rOther.size() )
        return sal_False;

    for ( size_t nNo = 0, nCnt = rOwn.size(); nNo < nCnt; ++nNo )
    {
        const SfxEventName *pOwn = rOwn.at( nNo );
        const SfxEventName *pOther = rOther.at( nNo );
        if (    pOwn->mnId != pOther->mnId ||
                pOwn->maEventName != pOther->maEventName ||
                pOwn->maUIName != pOther->maUIName )
            return sal_False;
    }

    return sal_True;

}

SfxItemPresentation SfxEventNamesItem::GetPresentation( SfxItemPresentation,
                                    SfxMapUnit,
                                    SfxMapUnit,
                                    XubString &rText,
                                    const IntlWrapper* ) const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
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

void SfxEventNamesItem::AddEvent( const String& rName, const String& rUIName, sal_uInt16 nID )
{
    aEventsList.push_back( new SfxEventName( nID, rName, rUIName.Len() ? rUIName : rName ) );
}


//==========================================================================

//--------------------------------------------------------------------------
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

            ::rtl::OUString aType(STAR_BASIC );
            ::rtl::OUString aLib  = pMacro->GetLibName();
            ::rtl::OUString aMacro = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= aType;

            pValues[ 1 ].Name = ::rtl::OUString(PROP_LIBRARY );
            pValues[ 1 ].Value <<= aLib;

            pValues[ 2 ].Name = ::rtl::OUString(PROP_MACRO_NAME );
            pValues[ 2 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == EXTENDED_STYPE )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            ::rtl::OUString aLib   = pMacro->GetLibName();
            ::rtl::OUString aMacro = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= aLib;

            pValues[ 1 ].Name = ::rtl::OUString(PROP_SCRIPT );
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == JAVASCRIPT )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            ::rtl::OUString aMacro  = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(PROP_EVENT_TYPE );
            pValues[ 0 ].Value <<= ::rtl::OUString(SVX_MACRO_LANGUAGE_JAVASCRIPT);

            pValues[ 1 ].Name = ::rtl::OUString(PROP_MACRO_NAME );
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else
        {
            SAL_WARN( "sfx2.config", "CreateEventData_Impl(): ScriptType not supported!");
        }
    }
    else
    {
        uno::Sequence < beans::PropertyValue > aProperties;
        aEventData <<= aProperties;
    }

    return aEventData;
}

//--------------------------------------------------------------------------
void PropagateEvent_Impl( SfxObjectShell *pDoc, rtl::OUString aEventName, const SvxMacro* pMacro )
{
    uno::Reference < document::XEventsSupplier > xSupplier;
    if ( pDoc )
    {
        xSupplier = uno::Reference < document::XEventsSupplier >( pDoc->GetModel(), uno::UNO_QUERY );
    }
    else
    {
        xSupplier = uno::Reference < document::XEventsSupplier >
                ( frame::GlobalEventBroadcaster::create(::comphelper::getProcessComponentContext()),
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
            catch( const ::com::sun::star::lang::IllegalArgumentException& )
            {
                SAL_WARN( "sfx2.config", "PropagateEvents_Impl: caught IllegalArgumentException" );
            }
            catch( const ::com::sun::star::container::NoSuchElementException& )
            {
                SAL_WARN( "sfx2.config", "PropagateEvents_Impl: caught NoSuchElementException" );
            }
        }
        else {
            DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );
        }
    }
}

//--------------------------------------------------------------------------------------------------------
void SfxEventConfiguration::ConfigureEvent( rtl::OUString aName, const SvxMacro& rMacro, SfxObjectShell *pDoc )
{
    boost::scoped_ptr<SvxMacro> pMacro;
    if ( rMacro.HasMacro() )
        pMacro.reset( new SvxMacro( rMacro.GetMacName(), rMacro.GetLibName(), rMacro.GetScriptType() ) );
    PropagateEvent_Impl( pDoc ? pDoc : 0, aName, pMacro.get() );
}

// -------------------------------------------------------------------------------------------------------
SvxMacro* SfxEventConfiguration::ConvertToMacro( const com::sun::star::uno::Any& rElement, SfxObjectShell* pDoc, sal_Bool bBlowUp )
{
    return SfxEvents_Impl::ConvertToMacro( rElement, pDoc, bBlowUp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
