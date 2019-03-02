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

#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <frmfmt.hxx>
#include <ndnotxt.hxx>
#include <flyfrm.hxx>
#include <cntfrm.hxx>
#include <notxtfrm.hxx>
#include <hints.hxx>
#include "accnotextframe.hxx"
#include <fmturl.hxx>
#include "accnotexthyperlink.hxx"
#include <vcl/imap.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <doc.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using utl::AccessibleRelationSetHelper;

const SwNoTextNode *SwAccessibleNoTextFrame::GetNoTextNode() const
{
    const SwNoTextNode *pNd  = nullptr;
    const SwFlyFrame *pFlyFrame = static_cast< const SwFlyFrame *>( GetFrame() );
    if( pFlyFrame->Lower() && pFlyFrame->Lower()->IsNoTextFrame() )
    {
        const SwNoTextFrame *pContentFrame =
            static_cast<const SwNoTextFrame *>(pFlyFrame->Lower());
        const SwContentNode* pSwContentNode = pContentFrame->GetNode();
        if(pSwContentNode != nullptr)
        {
            pNd = pSwContentNode->GetNoTextNode();
        }
    }

    return pNd;
}

SwAccessibleNoTextFrame::SwAccessibleNoTextFrame(
        std::shared_ptr<SwAccessibleMap> const& pInitMap,
        sal_Int16 nInitRole,
        const SwFlyFrame* pFlyFrame  ) :
    SwAccessibleFrameBase( pInitMap, nInitRole, pFlyFrame ),
    m_aListener(*this),
    msTitle(),
    msDesc()
{
    const SwNoTextNode* pNd = GetNoTextNode();
    m_aListener.StartListening(const_cast<SwNoTextNode*>(pNd));
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

    if (nWhich != RES_TITLE_CHANGED && nWhich != RES_DESCRIPTION_CHANGED)
        return;

    const SwNoTextNode *pNd = GetNoTextNode();
    assert( m_aListener.IsListeningTo(pNd) && "invalid frame" );
    switch( nWhich )
    {
        // #i73249#
        case RES_TITLE_CHANGED:
        {
            OUString sOldTitle, sNewTitle;
            const SwStringMsgPoolItem* pOldItem = dynamic_cast<const SwStringMsgPoolItem*>(pOld);
            if (pOldItem)
                sOldTitle = pOldItem->GetString();
            const SwStringMsgPoolItem* pNewItem = dynamic_cast<const SwStringMsgPoolItem*>(pNew);
            if (pNewItem)
                sNewTitle = pNewItem->GetString();
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
            [[fallthrough]];
        }
        case RES_DESCRIPTION_CHANGED:
        {
            if ( pNd && GetFrame() )
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

void SwAccessibleNoTextFrame::Dispose(bool bRecursive, bool bCanSkipInvisible)
{
    SolarMutexGuard aGuard;
    m_aListener.EndListeningAll();
    SwAccessibleFrameBase::Dispose(bRecursive, bCanSkipInvisible);
}

// #i73249#
OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleName()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    if ( !msTitle.isEmpty() )
    {
        return msTitle;
    }

    return SwAccessibleFrameBase::getAccessibleName();
}

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleDescription()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    return msDesc;
}

// XInterface

uno::Any SAL_CALL SwAccessibleNoTextFrame::queryInterface( const uno::Type& aType )
{
    if( aType ==
        ::cppu::UnoType<XAccessibleImage>::get() )
    {
        uno::Reference<XAccessibleImage> xImage = this;
        return uno::Any(xImage);
    }
    else if ( aType == cppu::UnoType<XAccessibleHypertext>::get())
    {
        uno::Reference<XAccessibleHypertext> aAccHypertext = this;
        return uno::Any( aAccHypertext );
    }
    else
        return SwAccessibleContext::queryInterface( aType );
}

// XTypeProvider

uno::Sequence< uno::Type > SAL_CALL SwAccessibleNoTextFrame::getTypes()
{
    return cppu::OTypeCollection(
        ::cppu::UnoType<XAccessibleImage>::get(),
        SwAccessibleFrameBase::getTypes() ).getTypes();
}

/// XAccessibleImage
/** implementation of the XAccessibleImage methods is a no-brainer, as
    all relevant information is already accessible through other
    methods. So we just delegate to those. */

OUString SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageDescription()
{
    return getAccessibleDescription();
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageHeight(  )
{
    return getSize().Height;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getAccessibleImageWidth(  )
{
    return getSize().Width;
}

// XAccessibleText
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCaretPosition(  ){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setCaretPosition( sal_Int32 ){return false;}
sal_Unicode SAL_CALL SwAccessibleNoTextFrame::getCharacter( sal_Int32 ){return 0;}
css::uno::Sequence< css::beans::PropertyValue > SAL_CALL SwAccessibleNoTextFrame::getCharacterAttributes( sal_Int32 , const css::uno::Sequence< OUString >& )
{
    return uno::Sequence<beans::PropertyValue>();
}
css::awt::Rectangle SAL_CALL SwAccessibleNoTextFrame::getCharacterBounds( sal_Int32 )
{
    return css::awt::Rectangle(0, 0, 0, 0 );
}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getCharacterCount(  ){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getIndexAtPoint( const css::awt::Point& ){return 0;}
OUString SAL_CALL SwAccessibleNoTextFrame::getSelectedText(  ){return OUString();}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionStart(  ){return 0;}
sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getSelectionEnd(  ){return 0;}
sal_Bool SAL_CALL SwAccessibleNoTextFrame::setSelection( sal_Int32 , sal_Int32 ){return true;}
OUString SAL_CALL SwAccessibleNoTextFrame::getText(  ){return OUString();}
OUString SAL_CALL SwAccessibleNoTextFrame::getTextRange( sal_Int32 , sal_Int32 ){return OUString();}
css::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextAtIndex( sal_Int32 , sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}
css::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBeforeIndex( sal_Int32, sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}
css::accessibility::TextSegment SAL_CALL SwAccessibleNoTextFrame::getTextBehindIndex( sal_Int32 , sal_Int16 )
{
    css::accessibility::TextSegment aResult;
    return aResult;
}

sal_Bool SAL_CALL SwAccessibleNoTextFrame::copyText( sal_Int32, sal_Int32 ){return true;}

//  XAccessibleHyperText

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkCount()
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    sal_Int32 nCount = 0;
    SwFormatURL aURL( static_cast<const SwLayoutFrame*>(GetFrame())->GetFormat()->GetURL() );

    if(aURL.GetMap() || !aURL.GetURL().isEmpty())
        nCount = 1;

    return nCount;
}

uno::Reference< XAccessibleHyperlink > SAL_CALL
    SwAccessibleNoTextFrame::getHyperLink( sal_Int32 nLinkIndex )
{
    SolarMutexGuard aGuard;

    ThrowIfDisposed();

    SwFormatURL aURL( static_cast<const SwLayoutFrame*>(GetFrame())->GetFormat()->GetURL() );

    if( nLinkIndex > 0 )
        throw lang::IndexOutOfBoundsException();

    if( aURL.GetMap() || !aURL.GetURL().isEmpty() )
    {
        if ( !m_xHyperlink.is() )
        {
            m_xHyperlink = new SwAccessibleNoTextHyperlink( this, GetFrame() );
        }

        return m_xHyperlink;
    }

    return nullptr;
}

sal_Int32 SAL_CALL SwAccessibleNoTextFrame::getHyperLinkIndex( sal_Int32 )
{
    return 0;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL SwAccessibleNoTextFrame::getAccessibleRelationSet( )
{
    return new AccessibleRelationSetHelper();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
