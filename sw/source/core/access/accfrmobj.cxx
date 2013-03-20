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

#include <vcl/window.hxx>

namespace sw { namespace access {

SwAccessibleChild::SwAccessibleChild()
    : mpFrm( 0 )
    , mpDrawObj( 0 )
    , mpWindow( 0 )
{}

SwAccessibleChild::SwAccessibleChild( const SdrObject* pDrawObj )
    : mpFrm( 0 )
    , mpDrawObj( 0 )
    , mpWindow( 0 )
{
    Init( pDrawObj );
}

SwAccessibleChild::SwAccessibleChild( const SwFrm* pFrm )
    : mpFrm( 0 )
    , mpDrawObj( 0 )
    , mpWindow( 0 )
{
    Init( pFrm );
}

SwAccessibleChild::SwAccessibleChild( Window* pWindow )
    : mpFrm( 0 )
    , mpDrawObj( 0 )
    , mpWindow( 0 )
{
    Init( pWindow );
}

SwAccessibleChild::SwAccessibleChild( const SwFrm* pFrm,
                                      const SdrObject* pDrawObj,
                                      Window* pWindow )
{
    if ( pFrm )
    {
        Init( pFrm );
    }
    else if ( pDrawObj )
    {
        Init( pDrawObj );
    }
    else if ( pWindow )
    {
        Init( pWindow );
    }
    OSL_ENSURE( (!pFrm || pFrm == mpFrm) &&
            (!pDrawObj || pDrawObj == mpDrawObj) &&
            (!pWindow || pWindow == mpWindow),
            "invalid frame/object/window combination" );

}

void SwAccessibleChild::Init( const SdrObject* pDrawObj )
{
    mpDrawObj = pDrawObj;
    mpFrm = mpDrawObj && mpDrawObj->ISA(SwVirtFlyDrawObj)
            ? static_cast < const SwVirtFlyDrawObj * >( mpDrawObj )->GetFlyFrm()
            : 0;
    mpWindow = 0;
}

void SwAccessibleChild::Init( const SwFrm* pFrm )
{
    mpFrm = pFrm;
    mpDrawObj = mpFrm && mpFrm->IsFlyFrm()
                ? static_cast < const SwFlyFrm * >( mpFrm )->GetVirtDrawObj()
                : 0;
    mpWindow = 0;
}

void SwAccessibleChild::Init( Window* pWindow )
{
    mpWindow = pWindow;
    mpFrm = 0;
    mpDrawObj = 0;
}

bool SwAccessibleChild::IsAccessible( sal_Bool bPagePreview ) const
{
    bool bRet( false );

    if ( mpFrm )
    {
        bRet = mpFrm->IsAccessibleFrm() &&
               ( !mpFrm->IsCellFrm() ||
                 static_cast<const SwCellFrm *>( mpFrm )->GetTabBox()->GetSttNd() != 0 ) &&
               !mpFrm->IsInCoveredCell() &&
               ( bPagePreview ||
                 !mpFrm->IsPageFrm() );
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

    if ( mpFrm )
    {
        bRet = mpFrm->IsFlyFrm() &&
               static_cast< const SwFlyFrm *>(mpFrm)->IsFlyInCntFrm();
    }
    else if ( mpDrawObj )
    {
        const SwFrmFmt* mpFrmFmt = ::FindFrmFmt( mpDrawObj );
        bRet = mpFrmFmt
               ? (FLY_AS_CHAR == mpFrmFmt->GetAnchor().GetAnchorId())
               : false;
    }
    else if ( mpWindow )
    {
        bRet = false;
    }

    return bRet;
}

SwAccessibleChild::SwAccessibleChild( const SwAccessibleChild& r )
    : mpFrm( r.mpFrm )
    , mpDrawObj( r.mpDrawObj )
    , mpWindow( r.mpWindow )
{}

SwAccessibleChild& SwAccessibleChild::operator=( const SwAccessibleChild& r )
{
    mpDrawObj = r.mpDrawObj;
    mpFrm = r.mpFrm;
    mpWindow = r.mpWindow;

    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( const SdrObject* pDrawObj )
{
    Init( pDrawObj );
    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( const SwFrm* pFrm )
{
    Init( pFrm );
    return *this;
}

SwAccessibleChild& SwAccessibleChild::operator=( Window* pWindow )
{
    Init( pWindow );
    return *this;
}

bool SwAccessibleChild::operator==( const SwAccessibleChild& r ) const
{
    return mpFrm == r.mpFrm &&
           mpDrawObj == r.mpDrawObj &&
           mpWindow == r.mpWindow;
}

bool SwAccessibleChild::IsValid() const
{
    return mpFrm != 0 ||
           mpDrawObj != 0 ||
           mpWindow != 0;
}

const SdrObject* SwAccessibleChild::GetDrawObject() const
{
    return mpDrawObj;
}

const SwFrm *SwAccessibleChild::GetSwFrm() const
{
    return mpFrm;
}

Window* SwAccessibleChild::GetWindow() const
{
    return mpWindow;
}

bool SwAccessibleChild::IsVisibleChildrenOnly() const
{
    bool bRet( false );

    if ( !mpFrm )
    {
        bRet = true;
    }
    else
    {
        bRet = mpFrm->IsRootFrm() ||
               !( mpFrm->IsTabFrm() ||
                  mpFrm->IsInTab() ||
                  ( IsBoundAsChar() &&
                    static_cast<const SwFlyFrm*>(mpFrm)->GetAnchorFrm()->IsInTab() ) );
    }

    return bRet;
}

SwRect SwAccessibleChild::GetBox( const SwAccessibleMap& rAccMap ) const
{
    SwRect aBox;

    if ( mpFrm )
    {
        if ( mpFrm->IsPageFrm() &&
             static_cast< const SwPageFrm * >( mpFrm )->IsEmptyPage() )
        {
            aBox = SwRect( mpFrm->Frm().Left(), mpFrm->Frm().Top()-1, 1, 1 );
        }
        else if ( mpFrm->IsTabFrm() )
        {
            aBox = SwRect( mpFrm->Frm() );
            aBox.Intersection( mpFrm->GetUpper()->Frm() );
        }
        else
        {
            aBox = mpFrm->Frm();
        }
    }
    else if( mpDrawObj )
    {
        aBox = SwRect( mpDrawObj->GetCurrentBoundRect() );
    }
    else if ( mpWindow )
    {
        aBox = SwRect( rAccMap.GetShell()->GetWin()->PixelToLogic(
                                        Rectangle( mpWindow->GetPosPixel(),
                                                   mpWindow->GetSizePixel() ) ) );
}

    return aBox;
}

SwRect SwAccessibleChild::GetBounds( const SwAccessibleMap& rAccMap ) const
{
    SwRect aBound;

    if( mpFrm )
    {
        if( mpFrm->IsPageFrm() &&
            static_cast< const SwPageFrm * >( mpFrm )->IsEmptyPage() )
        {
            aBound = SwRect( mpFrm->Frm().Left(), mpFrm->Frm().Top()-1, 0, 0 );
        }
        else
            aBound = mpFrm->PaintArea();
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

const SwFrm* SwAccessibleChild::GetParent( const sal_Bool bInPagePreview ) const
{
    const SwFrm* pParent( 0 );

    if ( mpFrm )
    {
        if( mpFrm->IsFlyFrm() )
        {
            const SwFlyFrm* pFly = static_cast< const SwFlyFrm *>( mpFrm );
            if( pFly->IsFlyInCntFrm() )
            {
                // For FLY_AS_CHAR the parent is the anchor
                pParent = pFly->GetAnchorFrm();
                OSL_ENSURE( SwAccessibleChild( pParent ).IsAccessible( bInPagePreview ),
                        "parent is not accessible" );
            }
            else
            {
                // In any other case the parent is the root frm
                // (in page preview, the page frame)
                if( bInPagePreview )
                    pParent = pFly->FindPageFrm();
                else
                    pParent = pFly->getRootFrm();
            }
        }
        else
        {
            SwAccessibleChild aUpper( mpFrm->GetUpper() );
            while( aUpper.GetSwFrm() && !aUpper.IsAccessible(bInPagePreview) )
            {
                aUpper = aUpper.GetSwFrm()->GetUpper();
            }
            pParent = aUpper.GetSwFrm();
        }
    }
    else if( mpDrawObj )
    {
        const SwDrawContact *pContact =
            static_cast< const SwDrawContact* >( GetUserCall( mpDrawObj ) );
        OSL_ENSURE( pContact, "sdr contact is missing" );
        if( pContact )
        {
            const SwFrmFmt *pFrmFmt = pContact->GetFmt();
            OSL_ENSURE( pFrmFmt, "frame format is missing" );
            if( pFrmFmt && FLY_AS_CHAR == pFrmFmt->GetAnchor().GetAnchorId() )
            {
                // For FLY_AS_CHAR the parent is the anchor
                pParent = pContact->GetAnchorFrm();
                OSL_ENSURE( SwAccessibleChild( pParent ).IsAccessible( bInPagePreview ),
                        "parent is not accessible" );

            }
            else
            {
                // In any other case the parent is the root frm
                if( bInPagePreview )
                    pParent = pContact->GetAnchorFrm()->FindPageFrm();
                else
                    pParent = pContact->GetAnchorFrm()->getRootFrm();
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
                        pParent = pAccParentImpl->GetFrm();
                    }
                }
            }
        }
    }

    return pParent;
}

} } // eof of namespace sw::access

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
