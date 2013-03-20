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

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <comphelper/servicehelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelation.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <accmap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <hints.hxx> // #i73249#
#include "acctextframe.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using utl::AccessibleRelationSetHelper;
using ::com::sun::star::accessibility::XAccessibleContext;

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextFrameView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleTextFrameView";

SwAccessibleTextFrame::SwAccessibleTextFrame(
        SwAccessibleMap* pInitMap,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleFrameBase( pInitMap, AccessibleRole::TEXT_FRAME, pFlyFrm ),
    msTitle(),
    msDesc()
{
    if ( pFlyFrm )
    {
        const SwFlyFrmFmt* pFlyFrmFmt =
                        dynamic_cast<const SwFlyFrmFmt*>( pFlyFrm->GetFmt() );
        msTitle = pFlyFrmFmt->GetObjTitle();

        msDesc = pFlyFrmFmt->GetObjDescription();
        if ( msDesc.isEmpty() &&
             msTitle != GetName() )
        {
            msDesc = msTitle;
        }
    }
}

SwAccessibleTextFrame::~SwAccessibleTextFrame()
{
}

void SwAccessibleTextFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    // #i73249# - suppress handling of RES_NAME_CHANGED
    // in case that attribute Title is used as the accessible name.
    if ( nWhich != RES_NAME_CHANGED ||
         msTitle.isEmpty() )
    {
        SwAccessibleFrameBase::Modify( pOld, pNew );
    }

    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm * >( GetFrm() );
    switch( nWhich )
    {
        // #i73249#
        case RES_TITLE_CHANGED:
        {
            const String& sOldTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pOld)->GetString() );
            const String& sNewTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pNew)->GetString() );
            if ( sOldTitle == sNewTitle )
            {
                break;
            }
            msTitle = sNewTitle;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.OldValue <<= OUString( sOldTitle );
            aEvent.NewValue <<= msTitle;
            FireAccessibleEvent( aEvent );

            const SwFlyFrmFmt* pFlyFrmFmt =
                            dynamic_cast<const SwFlyFrmFmt*>( pFlyFrm->GetFmt() );
            if ( !pFlyFrmFmt->GetObjDescription().isEmpty() )
            {
                break;
            }
        }
        // intentionally no break here
        case RES_DESCRIPTION_CHANGED:
        {
            if ( pFlyFrm )
            {
                const OUString sOldDesc( msDesc );

                const SwFlyFrmFmt* pFlyFrmFmt =
                                dynamic_cast<const SwFlyFrmFmt*>( pFlyFrm->GetFmt() );
                const String& rDesc = pFlyFrmFmt->GetObjDescription();
                msDesc = rDesc;
                if ( msDesc.isEmpty() &&
                     msTitle != GetName() )
                {
                    msDesc = msTitle;
                }

                if ( msDesc != sOldDesc )
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::DESCRIPTION_CHANGED;
                    aEvent.OldValue <<= sOldDesc;
                    aEvent.NewValue <<= msDesc;
                    FireAccessibleEvent( aEvent );
                }
            }
        }
        break;
    }
}

// #i73249#
OUString SAL_CALL SwAccessibleTextFrame::getAccessibleName (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    if ( !msTitle.isEmpty() )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}

OUString SAL_CALL SwAccessibleTextFrame::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return msDesc;

}

OUString SAL_CALL SwAccessibleTextFrame::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(sImplementationName);
}

sal_Bool SAL_CALL SwAccessibleTextFrame::supportsService(
        const OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleTextFrame::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( sServiceName );
    pArray[1] = OUString( sAccessibleServiceName );
    return aRet;
}

namespace
{
    class theSwAccessibleTextFrameImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessibleTextFrameImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleTextFrame::getImplementationId()
        throw(uno::RuntimeException)
{
    return theSwAccessibleTextFrameImplementationId::get().getSeq();
}

// XAccessibleRelationSet

SwFlyFrm* SwAccessibleTextFrame::getFlyFrm() const
{
    SwFlyFrm* pFlyFrm = NULL;

    const SwFrm* pFrm = GetFrm();
    OSL_ENSURE( pFrm != NULL, "frame expected" );
    if( pFrm->IsFlyFrm() )
    {
        pFlyFrm = static_cast<SwFlyFrm*>( const_cast<SwFrm*>( pFrm ) );
    }

    return pFlyFrm;
}

AccessibleRelation SwAccessibleTextFrame::makeRelation( sal_Int16 nType, const SwFlyFrm* pFrm )
{
    uno::Sequence<uno::Reference<XInterface> > aSequence(1);
    aSequence[0] = GetMap()->GetContext( pFrm );
    return AccessibleRelation( nType, aSequence );
}

uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleTextFrame::getAccessibleRelationSet( )
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleContext );

    // get the frame, and insert prev/next relations into helper

    AccessibleRelationSetHelper* pHelper = new AccessibleRelationSetHelper();

    SwFlyFrm* pFlyFrm = getFlyFrm();
    OSL_ENSURE( pFlyFrm != NULL, "fly frame expected" );

    const SwFlyFrm* pPrevFrm = pFlyFrm->GetPrevLink();
    if( pPrevFrm != NULL )
        pHelper->AddRelation( makeRelation(
            AccessibleRelationType::CONTENT_FLOWS_FROM, pPrevFrm ) );

    const SwFlyFrm* pNextFrm = pFlyFrm->GetNextLink();
    if( pNextFrm != NULL )
        pHelper->AddRelation( makeRelation(
            AccessibleRelationType::CONTENT_FLOWS_TO, pNextFrm ) );

    return pHelper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
