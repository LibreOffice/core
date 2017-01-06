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

#include <accfrmobj.hxx>

#include <accmap.hxx>
#include <acccontext.hxx>

#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <flyfrm.hxx>
#include <pagefrm.hxx>
#include <cellfrm.hxx>
#include <swtable.hxx>
#include <dflyobj.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <dcontact.hxx>

#include <pam.hxx>

#include <vcl/window.hxx>

namespace sw { namespace access {

SwAccessibleChild::SwAccessibleChild()
    : mpFrame( nullptr )
    , mpDrawObj( nullptr )
    , mpWindow( nullptr )
{}

SwAccessibleChild::SwAccessibleChild( const SdrObject* pDrawObj )
    : mpFrame( nullptr )
    , mpDrawObj( nullptr )
    , mpWindow( nullptr )
{
    Init( pDrawObj );
}

SwAccessibleChild::SwAccessibleChild( const SwFrame* pFrame )
    : mpFrame( nullptr )
    , mpDrawObj( nullptr )
    , mpWindow( nullptr )
{
    Init( pFrame );
}

SwAccessibleChild::SwAccessibleChild( vcl::Window* pWindow )
    : mpFrame( nullptr )
    , mpDrawObj( nullptr )
    , mpWindow( nullptr )
{
    Init( pWindow );
}

SwAccessibleChild::SwAccessibleChild( const SwFrame* pFrame,
                                      const SdrObject* pDrawObj,
                                      vcl::Window* pWindow )
    : mpFrame( nullptr )
    , mpDrawObj( nullptr )
    , mpWindow( nullptr )
{
    if ( pFrame )
    {
        Init( pFrame );
    }
    else if ( pDrawObj )
    {
        Init( pDrawObj );
    }
    else if ( pWindow )
    {
        Init( pWindow );
    }
    OSL_ENSURE( (!pFrame || pFrame == mpFrame) &&
            (!pDrawObj || pDrawObj == mpDrawObj) &&
            (!pWindow || pWindow == mpWindow),
            "invalid frame/object/window combination" );

}

void SwAccessibleChild::Init( const SdrObject* pDrawObj )
{
    mpDrawObj = pDrawObj;
    mpFrame = mpDrawObj && dynamic_cast<const SwVirtFlyDrawObj*>( mpDrawObj) !=  nullptr
            ? static_cast < const SwVirtFlyDrawObj * >( mpDrawObj )->GetFlyFrame()
            : nullptr;
    mpWindow = nullptr;
}

void SwAccessibleChild::Init( const SwFrame* pFrame )
{
    mpFrame = pFrame;
    mpDrawObj = mpFrame && mpFrame->IsFlyFrame()
                ? static_cast < const SwFlyFrame * >( mpFrame )->GetVirtDrawObj()
                : nullptr;
    mpWindow = nullptr;
}

void SwAccessibleChild::Init( vcl::Window* pWindow )
{
    mpWindow = pWindow;
    mpFrame = nullptr;
    mpDrawObj = nullptr;
}

bool SwAccessibleChild::IsAccessible( bool bPagePreview ) const
{
    bool bRet( false );

    if ( mpFrame )
    {
        bRet = mpFrame->IsAccessibleFrame() &&
               ( !mpFrame->IsCellFrame() ||
                 static_cast<const SwCellFrame *>( mpFrame )->GetTabBox()->GetSttNd() != nullptr ) &&
               !mpFrame->IsInCoveredCell() &&
               ( bPagePreview ||
                 !mpFrame->IsPageFrame() );
    }
    else if ( mpDrawObj )
    {
        bRet = true;
    }
    else if ( mpWindow )
    {
        bRet = true;
    }

    return bRet;
}

bool SwAccessibleChild::IsBoundAsChar() const
{
    bool bRet( false );

    if ( mpFrame )
    {
        bRet = mpFrame->IsFlyFrame() &&
               static_cast< const SwFlyFrame *>(mpFrame)->IsFlyInContentFrame();
    }
    else if ( mpDrawObj )
    {
        const SwFrameFormat* mpFrameFormat = ::FindFrameFormat( mpDrawObj );
        bRet = mpFrameFormat
               && (FLY_AS_CHAR == mpFrameFormat->GetAnchor().GetAnchorId());
    }
    else if ( mpWindow )
    {
        bRet = false;
    }

    return bRet;
}

SwAccessibleChild::SwAccessibleChild( const SwAccessibleChild& r )
    : mpFrame( r.mpFrame )
    , mpDrawObj( r.mpDrawObj )
    , mpWindow( r.mpWindow )
{}

SwAccessibleChild& SwAccessibleChild::operator=( const SwAccessibleChild& r )
{
    mpDrawObj = r.mpDrawObj;
    mpFrame = r.mpFrame;
    mpWindow = r.mpWindow;

    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( const SdrObject* pDrawObj )
{
    Init( pDrawObj );
    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( const SwFrame* pFrame )
{
    Init( pFrame );
    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( vcl::Window* pWindow )
{
    Init( pWindow );
    return *this;
}

bool SwAccessibleChild::operator==( const SwAccessibleChild& r ) const
{
    return mpFrame == r.mpFrame &&
           mpDrawObj == r.mpDrawObj &&
           mpWindow == r.mpWindow;
}

bool SwAccessibleChild::IsValid() const
{
    return mpFrame != nullptr ||
           mpDrawObj != nullptr ||
           mpWindow != nullptr;
}

bool SwAccessibleChild::IsVisibleChildrenOnly() const
{
    bool bRet( false );

    if ( !mpFrame )
    {
        bRet = true;
    }
    else
    {
        bRet = mpFrame->IsRootFrame() ||
               !( mpFrame->IsTabFrame() ||
                  mpFrame->IsInTab() ||
                  ( IsBoundAsChar() &&
                    static_cast<const SwFlyFrame*>(mpFrame)->GetAnchorFrame()->IsInTab() ) );
    }

    return bRet;
}

SwRect SwAccessibleChild::GetBox( const SwAccessibleMap& rAccMap ) const
{
    SwRect aBox;

    if ( mpFrame )
    {
        if ( mpFrame->IsPageFrame() &&
             static_cast< const SwPageFrame * >( mpFrame )->IsEmptyPage() )
        {
            aBox = SwRect( mpFrame->Frame().Left(), mpFrame->Frame().Top()-1, 1, 1 );
        }
        else if ( mpFrame->IsTabFrame() )
        {
            aBox = SwRect( mpFrame->Frame() );
            aBox.Intersection( mpFrame->GetUpper()->Frame() );
        }
        else
        {
            aBox = mpFrame->Frame();
        }
    }
    else if( mpDrawObj )
    {
        aBox = SwRect( mpDrawObj->GetCurrentBoundRect() );
    }
    else if ( mpWindow )
    {
        vcl::Window *pWin = rAccMap.GetShell()->GetWin();
        if (pWin)
        {
            aBox = SwRect( pWin->PixelToLogic(
                                            Rectangle( mpWindow->GetPosPixel(),
                                                       mpWindow->GetSizePixel() ) ) );
        }
    }

    return aBox;
}

SwRect SwAccessibleChild::GetBounds( const SwAccessibleMap& rAccMap ) const
{
    SwRect aBound;

    if( mpFrame )
    {
        if( mpFrame->IsPageFrame() &&
            static_cast< const SwPageFrame * >( mpFrame )->IsEmptyPage() )
        {
            aBound = SwRect( mpFrame->Frame().Left(), mpFrame->Frame().Top()-1, 0, 0 );
        }
        else
            aBound = mpFrame->PaintArea();
    }
    else if( mpDrawObj )
    {
        aBound = GetBox( rAccMap );
    }
    else if ( mpWindow )
    {
        aBound = GetBox( rAccMap );
    }

    return aBound;
}

bool SwAccessibleChild::AlwaysIncludeAsChild() const
{
    bool bAlwaysIncludedAsChild( false );

    if ( mpWindow )
    {
        bAlwaysIncludedAsChild = true;
    }

    return bAlwaysIncludedAsChild;
}

const SwFrame* SwAccessibleChild::GetParent( const bool bInPagePreview ) const
{
    const SwFrame* pParent( nullptr );

    if ( mpFrame )
    {
        if( mpFrame->IsFlyFrame() )
        {
            const SwFlyFrame* pFly = static_cast< const SwFlyFrame *>( mpFrame );
            if( pFly->IsFlyInContentFrame() )
            {
                // For FLY_AS_CHAR the parent is the anchor
                pParent = pFly->GetAnchorFrame();
                OSL_ENSURE( SwAccessibleChild( pParent ).IsAccessible( bInPagePreview ),
                        "parent is not accessible" );
            }
            else
            {
                // In any other case the parent is the root frm
                // (in page preview, the page frame)
                if( bInPagePreview )
                    pParent = pFly->FindPageFrame();
                else
                    pParent = pFly->getRootFrame();
            }
        }
        else
        {
            SwAccessibleChild aUpper( mpFrame->GetUpper() );
            while( aUpper.GetSwFrame() && !aUpper.IsAccessible(bInPagePreview) )
            {
                aUpper = aUpper.GetSwFrame()->GetUpper();
            }
            pParent = aUpper.GetSwFrame();
        }
    }
    else if( mpDrawObj )
    {
        const SwDrawContact *pContact =
            static_cast< const SwDrawContact* >( GetUserCall( mpDrawObj ) );
        OSL_ENSURE( pContact, "sdr contact is missing" );
        if( pContact )
        {
            const SwFrameFormat *pFrameFormat = pContact->GetFormat();
            OSL_ENSURE( pFrameFormat, "frame format is missing" );
            if( pFrameFormat && FLY_AS_CHAR == pFrameFormat->GetAnchor().GetAnchorId() )
            {
                // For FLY_AS_CHAR the parent is the anchor
                pParent = pContact->GetAnchorFrame();
                OSL_ENSURE( SwAccessibleChild( pParent ).IsAccessible( bInPagePreview ),
                        "parent is not accessible" );

            }
            else
            {
                // In any other case the parent is the root frm
                SwFrame const*const pAnchor(pContact->GetAnchorFrame());
                if (pAnchor) // null if object removed from layout
                {
                    if (bInPagePreview)
                        pParent = pAnchor->FindPageFrame();
                    else
                        pParent = pAnchor->getRootFrame();
                }
            }
        }
    }
    else if ( mpWindow )
    {
        css::uno::Reference < css::accessibility::XAccessible > xAcc =
                                                    mpWindow->GetAccessible();
        if ( xAcc.is() )
        {
            css::uno::Reference < css::accessibility::XAccessibleContext > xAccContext =
                                                xAcc->getAccessibleContext();
            if ( xAccContext.is() )
            {
                css::uno::Reference < css::accessibility::XAccessible > xAccParent =
                                                xAccContext->getAccessibleParent();
                if ( xAccParent.is() )
                {
                    SwAccessibleContext* pAccParentImpl =
                                dynamic_cast< SwAccessibleContext *>( xAccParent.get() );
                    if ( pAccParentImpl )
                    {
                        pParent = pAccParentImpl->GetFrame();
                    }
                }
            }
        }
    }

    return pParent;
}

} } // eof of namespace sw::access

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
