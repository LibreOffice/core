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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#include <vcl/status.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>

#include <svx/dialogs.hrc>
#include <svx/modctrl.hxx>
#include <svx/dialmgr.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::PropertyValue;
using ::rtl::OUString;

SFX_IMPL_STATUSBAR_CONTROL(SvxModifyControl, SfxBoolItem);


namespace
{
const unsigned _FEEDBACK_TIMEOUT = 3000;
}


// class SvxModifyControl ------------------------------------------------

struct SvxModifyControl::ImplData
{
    enum ModificationState
    {
        MODIFICATION_STATE_NO = 0,
        MODIFICATION_STATE_YES,
        MODIFICATION_STATE_FEEDBACK,
        MODIFICATION_STATE_SIZE
    };

    Timer maTimer;
    Image maImages[MODIFICATION_STATE_SIZE];

    ModificationState mnModState;

    ImplData():
        mnModState(MODIFICATION_STATE_NO)
    {
        maImages[MODIFICATION_STATE_NO]       = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_NO));
        maImages[MODIFICATION_STATE_YES]      = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_YES));
        maImages[MODIFICATION_STATE_FEEDBACK] = Image(SVX_RES(RID_SVXBMP_DOC_MODIFIED_FEEDBACK));
        maTimer.SetTimeout(_FEEDBACK_TIMEOUT);
    }
};

SvxModifyControl::SvxModifyControl( sal_uInt16 _nSlotId,
                                    sal_uInt16 _nId,
                                    StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mpImpl(new ImplData)
{
    mpImpl->maTimer.SetTimeoutHdl( LINK(this, SvxModifyControl, OnTimer) );
}

// -----------------------------------------------------------------------

void SvxModifyControl::StateChanged( sal_uInt16, SfxItemState eState,
                                     const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        return;

    DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
    SfxBoolItem* pItem = (SfxBoolItem*)pState;
    mpImpl->maTimer.Stop();

    bool modified = pItem->GetValue();
    bool start = ( !modified && mpImpl->mnModState == ImplData::MODIFICATION_STATE_YES);  // should timer be started and feedback image displayed ?

    mpImpl->mnModState = (start ? ImplData::MODIFICATION_STATE_FEEDBACK : (modified ? ImplData::MODIFICATION_STATE_YES : ImplData::MODIFICATION_STATE_NO));

    _repaint();

    int nResId = modified ? RID_SVXSTR_DOC_MODIFIED_YES : RID_SVXSTR_DOC_MODIFIED_NO;
    GetStatusBar().SetQuickHelpText(GetId(), SVX_RESSTR(nResId));

    if ( start )
        mpImpl->maTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxModifyControl, OnTimer, Timer *, pTimer )
{
    if (pTimer == 0)
        return 0;

    pTimer->Stop();
    mpImpl->mnModState = ImplData::MODIFICATION_STATE_NO;

    _repaint();

    return 0;
}

// -----------------------------------------------------------------------

void SvxModifyControl::_repaint()
{
    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint
}

// -----------------------------------------------------------------------

namespace {

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

}


void SvxModifyControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev =  rUsrEvt.GetDevice();
    Rectangle           aRect = rUsrEvt.GetRect();

    ImplData::ModificationState state = mpImpl->mnModState;
    Point aPt = centerImage(aRect, mpImpl->maImages[state]);
    pDev->DrawImage(aPt, mpImpl->maImages[state]);
}

void SvxModifyControl::DoubleClick()
{
    if (mpImpl->mnModState != ImplData::MODIFICATION_STATE_YES)
        // document not modified.  nothing to do here.
        return;

    Sequence<PropertyValue> aArgs;
    execute(OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Save")), aArgs);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
