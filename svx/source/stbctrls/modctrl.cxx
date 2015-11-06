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

#include <vcl/status.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>

#include <svx/dialogs.hrc>
#include <svx/modctrl.hxx>
#include <svx/dialmgr.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include "modctrl_internal.hxx"

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;

SFX_IMPL_STATUSBAR_CONTROL(SvxModifyControl, SfxBoolItem);

struct SvxModifyControl::ImplData
{
    enum ModificationState
    {
        MODIFICATION_STATE_NO = 0,
        MODIFICATION_STATE_YES,
        MODIFICATION_STATE_FEEDBACK,
        MODIFICATION_STATE_SIZE
    };

    Idle  maIdle;
    Image maImages[MODIFICATION_STATE_SIZE];

    ModificationState mnModState;

    ImplData():
        mnModState(MODIFICATION_STATE_NO)
    {
        maImages[MODIFICATION_STATE_NO]       = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_NO));
        maImages[MODIFICATION_STATE_YES]      = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_YES));
        maImages[MODIFICATION_STATE_FEEDBACK] = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_FEEDBACK));

        maIdle.SetPriority(SchedulerPriority::LOWEST);
    }
};

SvxModifyControl::SvxModifyControl( sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mxImpl(new ImplData)
{
//#ifndef MACOSX
    if ( rStb.GetDPIScaleFactor() > 1 )
    {
        for (int i = 0; i < mxImpl->MODIFICATION_STATE_SIZE; i++)
        {
            BitmapEx b = mxImpl->maImages[i].GetBitmapEx();
            b.Scale(rStb.GetDPIScaleFactor(), rStb.GetDPIScaleFactor(), BmpScaleFlag::Fast);
            mxImpl->maImages[i] = Image(b);
        }
    }
//#endif
    mxImpl->maIdle.SetIdleHdl( LINK(this, SvxModifyControl, OnTimer) );
}



void SvxModifyControl::StateChanged( sal_uInt16, SfxItemState eState,
                                     const SfxPoolItem* pState )
{
    if ( SfxItemState::DEFAULT != eState )
        return;

    DBG_ASSERT( dynamic_cast<const SfxBoolItem*>( pState) !=  nullptr, "invalid item type" );
    const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pState);
    mxImpl->maIdle.Stop();

    bool modified = pItem->GetValue();
    bool start = ( !modified && mxImpl->mnModState == ImplData::MODIFICATION_STATE_YES);  // should timer be started and feedback image displayed ?

    mxImpl->mnModState = (start ? ImplData::MODIFICATION_STATE_FEEDBACK : (modified ? ImplData::MODIFICATION_STATE_YES : ImplData::MODIFICATION_STATE_NO));

    _repaint();

    int nResId = modified ? RID_SVXSTR_DOC_MODIFIED_YES : RID_SVXSTR_DOC_MODIFIED_NO;
    GetStatusBar().SetQuickHelpText(GetId(), SVX_RESSTR(nResId));

    if ( start )
        mxImpl->maIdle.Start();
}



IMPL_LINK_TYPED( SvxModifyControl, OnTimer, Idle *, pTimer, void )
{
    if (pTimer == 0)
        return;

    pTimer->Stop();
    mxImpl->mnModState = ImplData::MODIFICATION_STATE_NO;

    _repaint();
}



void SvxModifyControl::_repaint()
{
    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint
}

/**
 * Given a bounding rectangle and an image, determine the top-left position
 * of the image so that the image would look centered both horizontally and
 * vertically.
 *
 * @param rBoundingRect bounding rectangle
 * @param rImg image
 *
 * @return Point top-left corner of the centered image position
 */
Point centerImage(const Rectangle& rBoundingRect, const Image& rImg)
{
    Size aImgSize = rImg.GetSizePixel();
    Size aRectSize = rBoundingRect.GetSize();
    long nXOffset = (aRectSize.getWidth() - aImgSize.getWidth())/2;
    long nYOffset = (aRectSize.getHeight() - aImgSize.getHeight())/2;
    Point aPt = rBoundingRect.TopLeft();
    aPt += Point(nXOffset, nYOffset);
    return aPt;
}

void SvxModifyControl::Paint( const UserDrawEvent& rUsrEvt )
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    Rectangle aRect(rUsrEvt.GetRect());

    ImplData::ModificationState state = mxImpl->mnModState;
    Point aPt = centerImage(aRect, mxImpl->maImages[state]);
    pDev->DrawImage(aPt, mxImpl->maImages[state]);
}

void SvxModifyControl::Click()
{
    if (mxImpl->mnModState != ImplData::MODIFICATION_STATE_YES)
        // document not modified.  nothing to do here.
        return;

    Sequence<PropertyValue> aArgs;
    execute(".uno:Save", aArgs);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
