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
#include "precompiled_sw.hxx"


#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <frmfmt.hxx>
#include <ndnotxt.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
#include <hints.hxx> //#i73249#
#include "accnotextframe.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

const SwNoTxtNode *SwAccessibleNoTextFrame::GetNoTxtNode() const
{
    const SwNoTxtNode *pNd  = 0;
    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm *>( GetFrm() );
    if( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
    {
        const SwCntntFrm *pCntFrm =
            static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
        pNd = pCntFrm->GetNode()->GetNoTxtNode();
    }

    return pNd;
}

SwAccessibleNoTextFrame::SwAccessibleNoTextFrame(
        SwAccessibleMap* pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrm* pFlyFrm  ) :
    SwAccessibleFrameBase( pInitMap, nInitRole, pFlyFrm ),
    aDepend( this, const_cast < SwNoTxtNode * >( GetNoTxtNode() ) ),
    msTitle(),
    msDesc()
{
    const SwNoTxtNode* pNd = GetNoTxtNode();
    // #i73249#
    // consider new attributes Title and Description
    if( pNd )
    {
        msTitle = pNd->GetTitle();

        msDesc = pNd->GetDescription();
        if ( msDesc.getLength() == 0 &&
             msTitle != GetName() )
        {
            msDesc = msTitle;
        }
    }
    // <--
}

SwAccessibleNoTextFrame::~SwAccessibleNoTextFrame()
{
}

void SwAccessibleNoTextFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
    // #i73249#
    // suppress handling of RES_NAME_CHANGED in case that attribute Title is
    // used as the accessible name.
    if ( nWhich != RES_NAME_CHANGED ||
         msTitle.getLength() == 0 )
    {
        SwAccessibleFrameBase::Modify( pOld, pNew );
    }

    const SwNoTxtNode *pNd = GetNoTxtNode();
    OSL_ENSURE( pNd == aDepend.GetRegisteredIn(), "invalid frame" );
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

            if ( pNd->GetDescription().Len() != 0 )
            {
                break;
            }
        }
        // intentional no break here
        case RES_DESCRIPTION_CHANGED:
        {
            if ( pNd && GetFrm() )
            {
                const OUString sOldDesc( msDesc );

                const String& rDesc = pNd->GetDescription();
                msDesc = rDesc;
                if ( msDesc.getLength() == 0 &&
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

void SwAccessibleNoTextFrame::Dispose( sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    if( aDepend.GetRegisteredIn() )
        const_cast < SwModify *>( aDepend.GetRegisteredIn() )->Remove( &aDepend );

    SwAccessibleFrameBase::Dispose( bRecursive );
}

// #i73249#
OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleName (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    if ( msTitle.getLength() != 0 )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}
// <--

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return msDesc;
}



//
// XInterface
//

uno::Any SAL_CALL SwAccessibleNoTextFrame::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    if( aType ==
        ::getCppuType( static_cast<uno::Reference<XAccessibleImage>*>( NULL ) ) )
    {
        uno::Reference<XAccessibleImage> xImage = this;
        uno::Any aAny;
        aAny <<= xImage;
        return aAny;
    }
    else
        return SwAccessibleContext::queryInterface( aType );
}


//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleNoTextFrame::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleFrameBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleImage > * >( 0 ) );

    return aTypes;
}


//
// XAccessibleImage
//

// implementation of the XAccessibleImage methods is a no-brainer, as
// all releveant information is already accessible through other
// methods. So we just delegate to those.

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageDescription()
    throw ( uno::RuntimeException )
{
    return getAccessibleDescription();
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageHeight(  )
    throw ( uno::RuntimeException )
{
    return getSize().Height;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageWidth(  )
    throw ( uno::RuntimeException )
{
    return getSize().Width;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
