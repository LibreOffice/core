/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <vcl/msgbox.hxx>
#include <tools/resary.hxx>
#include <svl/lstner.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmod.hxx>
#include <tools/urlobj.hxx>
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
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

// -----------------------------------------------------------------------
TYPEINIT1(SfxEventHint, SfxHint);
TYPEINIT1(SfxEventNamesItem, SfxPoolItem);
TYPEINIT1(SfxViewEventHint, SfxHint);

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

            ::rtl::OUString aType(RTL_CONSTASCII_USTRINGPARAM( STAR_BASIC ));
            ::rtl::OUString aLib  = pMacro->GetLibName();
            ::rtl::OUString aMacro = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
            pValues[ 0 ].Value <<= aType;

            pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_LIBRARY ));
            pValues[ 1 ].Value <<= aLib;

            pValues[ 2 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_MACRO_NAME ));
            pValues[ 2 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == EXTENDED_STYPE )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            ::rtl::OUString aLib   = pMacro->GetLibName();
            ::rtl::OUString aMacro = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
            pValues[ 0 ].Value <<= aLib;

            pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_SCRIPT ));
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else if ( pMacro->GetScriptType() == JAVASCRIPT )
        {
            uno::Sequence < beans::PropertyValue > aProperties(2);
            beans::PropertyValue *pValues = aProperties.getArray();

            ::rtl::OUString aMacro  = pMacro->GetMacName();

            pValues[ 0 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_EVENT_TYPE ));
            pValues[ 0 ].Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVX_MACRO_LANGUAGE_JAVASCRIPT));

            pValues[ 1 ].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( PROP_MACRO_NAME ));
            pValues[ 1 ].Value <<= aMacro;

            aEventData <<= aProperties;
        }
        else
        {
            DBG_ERRORFILE( "CreateEventData_Impl(): ScriptType not supported!");
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
                ( ::comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.GlobalEventBroadcaster"))), uno::UNO_QUERY );
    }

    if ( xSupplier.is() )
    {
        uno::Reference < container::XNameReplace > xEvents = xSupplier->getEvents();
        if ( aEventName.getLength() )
        {
            uno::Any aEventData = CreateEventData_Impl( pMacro );

            try
            {
                xEvents->replaceByName( aEventName, aEventData );
            }
            catch( ::com::sun::star::lang::IllegalArgumentException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught IllegalArgumentException" ); }
            catch( ::com::sun::star::container::NoSuchElementException )
            { DBG_ERRORFILE( "PropagateEvents_Impl: caught NoSuchElementException" ); }
        }
        else {
            DBG_WARNING( "PropagateEvents_Impl: Got unkown event" );
        }
    }
}

//--------------------------------------------------------------------------------------------------------
void SfxEventConfiguration::ConfigureEvent( rtl::OUString aName, const SvxMacro& rMacro, SfxObjectShell *pDoc )
{
    SvxMacro *pMacro = NULL;
    if ( rMacro.GetMacName().Len() )
        pMacro = new SvxMacro( rMacro.GetMacName(), rMacro.GetLibName(), rMacro.GetScriptType() );
    if ( pDoc )
    {
        PropagateEvent_Impl( pDoc, aName, pMacro );
    }
    else
    {
        PropagateEvent_Impl( NULL, aName, pMacro );
    }
}

// -------------------------------------------------------------------------------------------------------
SvxMacro* SfxEventConfiguration::ConvertToMacro( const com::sun::star::uno::Any& rElement, SfxObjectShell* pDoc, sal_Bool bBlowUp )
{
    return SfxEvents_Impl::ConvertToMacro( rElement, pDoc, bBlowUp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
