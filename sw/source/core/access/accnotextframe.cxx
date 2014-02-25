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
#include <hints.hxx>
#include "accnotextframe.hxx"
#include <fmturl.hxx>
#include <accnotexthyperlink.hxx>
#include <svtools/imap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <doc.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using utl::AccessibleRelationSetHelper;

const SwNoTxtNode *SwAccessibleNoTextFrame::GetNoTxtNode() const
{
    const SwNoTxtNode *pNd  = 0;
    const SwFlyFrm *pFlyFrm = static_cast< const SwFlyFrm *>( GetFrm() );
    if( pFlyFrm->Lower() && pFlyFrm->Lower()->IsNoTxtFrm() )
    {
        const SwCntntFrm *pCntFrm =
            static_cast<const SwCntntFrm *>( pFlyFrm->Lower() );
        const SwCntntNode* pSwCntntNode = pCntFrm->GetNode();
        if(pSwCntntNode != NULL)
        {
            pNd = pSwCntntNode->GetNoTxtNode();
        }
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
        if ( msDesc.isEmpty() &&
             msTitle != GetName() )
        {
            msDesc = msTitle;
        }
    }
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
         msTitle.isEmpty() )
    {
        SwAccessibleFrameBase::Modify( pOld, pNew );
        if (!GetRegisteredIn())
            return; // probably was deleted - avoid doing anything
    }

    const SwNoTxtNode *pNd = GetNoTxtNode();
    OSL_ENSURE( pNd == aDepend.GetRegisteredIn(), "invalid frame" );
    switch( nWhich )
    {
        // #i73249#
        case RES_TITLE_CHANGED:
        {
            const OUString& sOldTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pOld)->GetString() );
            const OUString& sNewTitle(
                        dynamic_cast<const SwStringMsgPoolItem*>(pNew)->GetString() );
            if ( sOldTitle == sNewTitle )
            {
                break;
            }
            msTitle = sNewTitle;
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::NAME_CHANGED;
            aEvent.OldValue <<= sOldTitle;
            aEvent.NewValue <<= msTitle;
            FireAccessibleEvent( aEvent );

            if ( !pNd->GetDescription().isEmpty() )
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

                const OUString& rDesc = pNd->GetDescription();
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

void SwAccessibleNoTextFrame::Dispose( sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    if( aDepend.GetRegisteredIn() )
        const_cast < SwModify *>( aDepend.GetRegisteredIn() )->Remove( &aDepend );

    SwAccessibleFrameBase::Dispose( bRecursive );
}

// #i73249#
OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleName (void)
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    if ( !msTitle.isEmpty() )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleDescription (void)
        throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return msDesc;
}

// XInterface

uno::Any SAL_CALL SwAccessibleNoTextFrame::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException, std::exception)
{
    if( aType ==
        ::getCppuType( static_cast<uno::Reference<XAccessibleImage>*>( NULL ) ) )
    {
        uno::Reference<XAccessibleImage> xImage = this;
        uno::Any aAny;
        aAny <<= xImage;
        return aAny;
    }
    else if ( aType == ::getCppuType((uno::Reference<XAccessibleHypertext> *)0) )
    {
        uno::Reference<XAccessibleHypertext> aAccHypertext = this;
        uno::Any aAny;
        aAny <<= aAccHypertext;
        return aAny;
    }
    else
        return SwAccessibleContext::queryInterface( aType );
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SwAccessibleNoTextFrame::getTypes() throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleFrameBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleImage > * >( 0 ) );

    return aTypes;
}

/// XAccessibleImage
/** implementation of the XAccessibleImage methods is a no-brainer, as
    all relevant information is already accessible through other
    methods. So we just delegate to those. */

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageDescription()
    throw ( uno::RuntimeException, std::exception )
{
    return getAccessibleDescription();
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageHeight(  )
    throw ( uno::RuntimeException, std::exception )
{
    return getSize().Height;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageWidth(  )
    throw ( uno::RuntimeException, std::exception )
{
    return getSize().Width;
}

//=====  XAccesibleText  ==================================================
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setCaretPosition( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception){return 0;}
sal_Unicode SAL_CALL SwAccessibleNoTextFrame::getCharacter( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception){return 0;}
::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SwAccessibleNoTextFrame::getCharacterAttributes( sal_Int32 , const ::com::sun::star::uno::Sequence< OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    uno::Sequence<beans::PropertyValue> aValues(0);
    return aValues;
}
::com::sun::star::awt::Rectangle SAL_CALL SwAccessibleNoTextFrame::getCharacterBounds( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    return com::sun::star::awt::Rectangle(0, 0, 0, 0 );
}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getIndexAtPoint( const ::com::sun::star::awt::Point& ) throw (::com::sun::star::uno::RuntimeException, std::exception){return 0;}
OUString SAL_CALL SwAccessibleNoTextFrame::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return OUString();}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setSelection( sal_Int32 , sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception){return sal_True;}
OUString SAL_CALL SwAccessibleNoTextFrame::getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception){return OUString();}
OUString SAL_CALL SwAccessibleNoTextFrame::getTextRange( sal_Int32 , sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception){return OUString();}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextAtIndex( sal_Int32 , sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBeforeIndex( sal_Int32, sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}
::com::sun::star::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBehindIndex( sal_Int32 , sal_Int16 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    ::com::sun::star::accessibility::TextSegment aResult;
    return aResult;
}

sal_Bool SAL_CALL SwAccessibleNoTextFrame::copyText( sal_Int32, sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception){return sal_True;}


//
//  XAccessibleHyperText
//
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkCount()
throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    sal_Int32 nCount = 0;
    SwFmtURL aURL( ((SwLayoutFrm*)GetFrm())->GetFmt()->GetURL() );

    if(aURL.GetMap() || !aURL.GetURL().isEmpty())
        nCount = 1;

    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleNoTextFrame::getHyperLink( sal_Int32 nLinkIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    CHECK_FOR_DEFUNC( XAccessibleHypertext );

    uno::Reference< XAccessibleHyperlink > xRet;

    SwFmtURL aURL( ((SwLayoutFrm*)GetFrm())->GetFmt()->GetURL() );

    if( nLinkIndex > 0 )
        throw lang::IndexOutOfBoundsException();

    if( aURL.GetMap() || !aURL.GetURL().isEmpty() )
    {
        if ( !alink.is() )
        {
            alink = new SwAccessibleNoTextHyperlink( this, GetFrm() );
        }

        return alink;
    }

    return NULL;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkIndex( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return 0;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleNoTextFrame::getAccessibleRelationSet( )
    throw ( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    return new AccessibleRelationSetHelper();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
