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

#include <doc.hxx>
#include <frmtool.hxx>
#include <hints.hxx>
#include <fmtornt.hxx>
#include <rootfrm.hxx>
#include <flyfrms.hxx>
#include <dflyobj.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>

SwFlyInContentFrame::SwFlyInContentFrame( SwFlyFrameFormat *pFormat, SwFrame* pSib, SwFrame *pAnch ) :
    SwFlyFrame( pFormat, pSib, pAnch )
{
    m_bInCnt = true;
    SwTwips nRel = pFormat->GetVertOrient().GetPos();
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    Point aRelPos;
    if( pAnch && pAnch->IsVertical() )
        aRelPos.setX(-nRel);
    else
        aRelPos.setY(nRel);
    SetCurrRelPos( aRelPos );
}

void SwFlyInContentFrame::DestroyImpl()
{
    if ( !GetFormat()->GetDoc()->IsInDtor() && GetAnchorFrame() )
    {
        SwRect aTmp( GetObjRectWithSpaces() );
        SwFlyInContentFrame::NotifyBackground( FindPageFrame(), aTmp, PrepareHint::FlyFrameLeave );
    }

    SwFlyFrame::DestroyImpl();
}

SwFlyInContentFrame::~SwFlyInContentFrame()
{
}

// #i28701#

void SwFlyInContentFrame::SetRefPoint( const Point& rPoint,
                                 const Point& rRelAttr,
                                 const Point& rRelPos )
{
    // OD 2004-05-27 #i26791# - member <aRelPos> moved to <SwAnchoredObject>
    OSL_ENSURE( rPoint != m_aRef || rRelAttr != GetCurrRelPos(), "SetRefPoint: no change" );
    std::unique_ptr<SwFlyNotify, o3tl::default_delete<SwFlyNotify>> xNotify;
    // No notify at a locked fly frame, if a fly frame is locked, there's
    // already a SwFlyNotify object on the stack (MakeAll).
    if( !IsLocked() )
        xNotify.reset(new SwFlyNotify( this ));
    m_aRef = rPoint;
    SetCurrRelPos( rRelAttr );
    SwRectFnSet aRectFnSet(GetAnchorFrame());

    {
        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
        aRectFnSet.SetPos( aFrm, rPoint + rRelPos );
    }

    // #i68520#
    InvalidateObjRectWithSpaces();
    if (xNotify)
    {
        InvalidatePage();
        setFrameAreaPositionValid(false);
        m_bInvalid  = true;
        Calc(getRootFrame()->GetCurrShell()->GetOut());
        xNotify.reset();
    }
}

void SwFlyInContentFrame::SwClientNotify(const SwModify& rMod, const SfxHint& rHint)
{
    auto pLegacy = dynamic_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy)
        return;
    std::pair<std::unique_ptr<SwAttrSetChg>, std::unique_ptr<SwAttrSetChg>> aTweakedChgs;
    std::pair<const SfxPoolItem*, const SfxPoolItem*> aSuperArgs(nullptr, nullptr);
    switch(pLegacy->GetWhich())
    {
        case RES_ATTRSET_CHG:
        {
            auto pOldAttrSetChg = static_cast<const SwAttrSetChg*>(pLegacy->m_pOld);
            auto pNewAttrSetChg = static_cast<const SwAttrSetChg*>(pLegacy->m_pNew);
            if(pOldAttrSetChg
                    && pNewAttrSetChg
                    && ((SfxItemState::SET == pNewAttrSetChg->GetChgSet()->GetItemState(RES_SURROUND, false))
                    || (SfxItemState::SET == pNewAttrSetChg->GetChgSet()->GetItemState(RES_FRMMACRO, false))))
            {
                aTweakedChgs.second = std::make_unique<SwAttrSetChg>(*pOldAttrSetChg);
                aTweakedChgs.second->ClearItem(RES_SURROUND);
                aTweakedChgs.second->ClearItem(RES_FRMMACRO);
                if(aTweakedChgs.second->Count())
                {
                    aTweakedChgs.first = std::make_unique<SwAttrSetChg>(*pOldAttrSetChg);
                    aTweakedChgs.first->ClearItem(RES_SURROUND);
                    aTweakedChgs.first->ClearItem(RES_FRMMACRO);
                    aSuperArgs = std::pair<const SfxPoolItem*, const SfxPoolItem*>(aTweakedChgs.first.get(), aTweakedChgs.second.get());
                }
            } else if (pNewAttrSetChg && pNewAttrSetChg->GetChgSet()->Count())
                aSuperArgs = std::pair<const SfxPoolItem*, const SfxPoolItem*>(pLegacy->m_pOld, pLegacy->m_pNew);
            break;
        }
        case RES_SURROUND:
        case RES_FRMMACRO:
            break;
        default:
            aSuperArgs = std::pair<const SfxPoolItem*, const SfxPoolItem*>(pLegacy->m_pOld, pLegacy->m_pNew);
    }
    if(aSuperArgs.second)
    {
        SwFlyFrame::SwClientNotify(rMod, sw::LegacyModifyHint(aSuperArgs.first, aSuperArgs.second));
        if(GetAnchorFrame())
            AnchorFrame()->Prepare(PrepareHint::FlyFrameAttributesChanged, GetFormat());
    }
}

/// Here the content gets formatted initially.
void SwFlyInContentFrame::Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs )
{
    if ( !getFrameArea().Height() )
    {
        Lock(); //don't format the anchor on the crook.
        SwContentFrame *pContent = ContainsContent();
        while ( pContent )
        {   pContent->Calc(pRenderContext);
            pContent = pContent->GetNextContentFrame();
        }
        Unlock();
    }
    SwFlyFrame::Format( pRenderContext, pAttrs );
}

/** Calculate object position
 *
 * @note: In contrast to other Frames, we only calculate the relative position
 *        here. The absolute position is only calculated using SetAbsPos.
 **/
void SwFlyInContentFrame::MakeObjPos()
{
    if ( isFrameAreaPositionValid() )
        return;

    setFrameAreaPositionValid(true);
    SwFlyFrameFormat *pFormat = GetFormat();
    const SwFormatVertOrient &rVert = pFormat->GetVertOrient();
    //Update the current values in the format if needed, during this we of
    //course must not send any Modify.
    const bool bVert = GetAnchorFrame()->IsVertical();
    SwTwips nOld = rVert.GetPos();
    SwTwips nAct = bVert ? -GetCurrRelPos().X() : GetCurrRelPos().Y();
    if( nAct != nOld )
    {
        SwFormatVertOrient aVert( rVert );
        aVert.SetPos( nAct );
        pFormat->LockModify();
        pFormat->SetFormatAttr( aVert );
        pFormat->UnlockModify();
    }
}

void SwFlyInContentFrame::ActionOnInvalidation( const InvalidationType _nInvalid )
{
    if ( INVALID_POS == _nInvalid || INVALID_ALL == _nInvalid )
        AnchorFrame()->Prepare( PrepareHint::FlyFrameAttributesChanged, &GetFrameFormat() );
}

void SwFlyInContentFrame::NotifyBackground( SwPageFrame *, const SwRect& rRect,
                                       PrepareHint eHint)
{
    if ( eHint == PrepareHint::FlyFrameAttributesChanged )
        AnchorFrame()->Prepare( PrepareHint::FlyFrameAttributesChanged );
    else
        AnchorFrame()->Prepare( eHint, static_cast<void const *>(&rRect) );
}

Point const & SwFlyInContentFrame::GetRelPos() const
{
    Calc(getRootFrame()->GetCurrShell()->GetOut());
    return GetCurrRelPos();
}

/// @see SwRowFrame::RegistFlys()
void SwFlyInContentFrame::RegistFlys()
{
    SwPageFrame *pPage = FindPageFrame();
    OSL_ENSURE( pPage, "Register Flys without pages?" );
    ::RegistFlys( pPage, this );
}

void SwFlyInContentFrame::MakeAll(vcl::RenderContext* /*pRenderContext*/)
{
    // OD 2004-01-19 #110582#
    if ( !GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( GetVirtDrawObj()->GetLayer() ) )
    {
        return;
    }

    if ( !GetAnchorFrame() || IsLocked() || IsColLocked() || !FindPageFrame() )
        return;

    Lock(); // The curtain falls

        //does the notification in the DTor
    const SwFlyNotify aNotify( this );
    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    if ( IsClipped() )
    {
        setFrameAreaSizeValid(false);
        m_bHeightClipped = m_bWidthClipped = false;
    }

    while ( !isFrameAreaPositionValid() || !isFrameAreaSizeValid() || !isFramePrintAreaValid() || !m_bValidContentPos )
    {
        //Only stop, if the flag is set!!
        if ( !isFrameAreaSizeValid() )
        {
            setFramePrintAreaValid(false);
        }

        if ( !isFramePrintAreaValid() )
        {
            MakePrtArea( rAttrs );
            m_bValidContentPos = false;
        }

        if ( !isFrameAreaSizeValid() )
        {
            Format( getRootFrame()->GetCurrShell()->GetOut(), &rAttrs );
        }

        if ( !isFrameAreaPositionValid() )
        {
            MakeObjPos();
        }

        if ( !m_bValidContentPos )
            MakeContentPos( rAttrs );

        // re-activate clipping of as-character anchored Writer fly frames
        // depending on compatibility option <ClipAsCharacterAnchoredWriterFlyFrames>
        if ( isFrameAreaPositionValid() &&
            isFrameAreaSizeValid() &&
            GetFormat()->getIDocumentSettingAccess().get( DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME ) )
        {
            SwFrame* pFrame = AnchorFrame();
            if ( getFrameArea().Left() == (pFrame->getFrameArea().Left()+pFrame->getFramePrintArea().Left()) &&
                 getFrameArea().Width() > pFrame->getFramePrintArea().Width() )
            {
                SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                aFrm.Width( pFrame->getFramePrintArea().Width() );
                setFramePrintAreaValid(false);
                m_bWidthClipped = true;
            }
        }
    }
    Unlock();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
