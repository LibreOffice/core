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

// class SvxModifyControl ------------------------------------------------

struct SvxModifyControl::ImplData
{
    Image maModifiedButton;
    Image maNonModifiedButton;

    bool mbModified;

    ImplData() :
        maModifiedButton( SVX_RES(RID_SVXBMP_DOC_MODIFIED_YES) ),
        maNonModifiedButton( SVX_RES(RID_SVXBMP_DOC_MODIFIED_NO) ),
        mbModified(false)
    {
    }
};

SvxModifyControl::SvxModifyControl( sal_uInt16 _nSlotId,
                                    sal_uInt16 _nId,
                                    StatusBar& rStb ) :

    SfxStatusBarControl( _nSlotId, _nId, rStb ),
    mpImpl(new ImplData)
{
}

// -----------------------------------------------------------------------

void SvxModifyControl::StateChanged( sal_uInt16, SfxItemState eState,
                                     const SfxPoolItem* pState )
{
    if ( SFX_ITEM_AVAILABLE != eState )
        return;

    DBG_ASSERT( pState->ISA( SfxBoolItem ), "invalid item type" );
    SfxBoolItem* pItem = (SfxBoolItem*)pState;
    mpImpl->mbModified = pItem->GetValue();

    if ( GetStatusBar().AreItemsVisible() )
        GetStatusBar().SetItemData( GetId(), 0 );    // force repaint

    int nResId = mpImpl->mbModified ? RID_SVXSTR_DOC_MODIFIED_YES : RID_SVXSTR_DOC_MODIFIED_NO;
    GetStatusBar().SetQuickHelpText(GetId(), SVX_RESSTR(nResId));
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

    if (mpImpl->mbModified)
    {
        Point aPt = centerImage(aRect, mpImpl->maModifiedButton);
        pDev->DrawImage(aPt, mpImpl->maModifiedButton);
    }
    else
    {
        Point aPt = centerImage(aRect, mpImpl->maNonModifiedButton);
        pDev->DrawImage(aPt, mpImpl->maNonModifiedButton);
    }
}

void SvxModifyControl::DoubleClick()
{
    if (!mpImpl->mbModified)
        // document not modified.  nothing to do here.
        return;

    Sequence<PropertyValue> aArgs;
    execute(OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Save")), aArgs);
}

sal_uIntPtr SvxModifyControl::GetDefItemWidth(const StatusBar& rStb)
{
    return rStb.GetTextWidth(String::CreateFromAscii("XX"));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
