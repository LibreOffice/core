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

#include "textcontrolcombo.hxx"


TextControlCombo::TextControlCombo( Window* _pParent, const ResId& _rResId,
    Control& _rCtrl, FixedText& _rFTbefore, FixedText& _rFTafter )
    :Window         ( _pParent, _rResId )
    ,mrCtrl         ( _rCtrl )
    ,mrFTbefore     ( _rFTbefore )
    ,mrFTafter      ( _rFTafter )
{
}

TextControlCombo::~TextControlCombo()
{
}

void TextControlCombo::Arrange( FixedText& _rFTcomplete, sal_Bool /*bShow*/ )
{
    Point           aBasePos( GetPosPixel() );
    Size            aMetricVals( GetSizePixel() );

    long            nTextHeight = _rFTcomplete.GetSizePixel().Height();
    long            nCtrlHeight = mrCtrl.GetSizePixel().Height();

    // calc y positions / center vertical
    long            nYFT = aBasePos.Y();
    long            nYCtrl = nYFT;
    if( nCtrlHeight > nTextHeight )
        nYFT += aMetricVals.Height();
    else
        nYCtrl += aMetricVals.Height();

    // separate text parts
    const String    aReplStr( RTL_CONSTASCII_USTRINGPARAM( "%POSITION_OF_CONTROL" ) );
    String          aTxtBefore( _rFTcomplete.GetText() );
    String          aTxtAfter;
    xub_StrLen      nReplPos = aTxtBefore.Search( aReplStr );
    if( nReplPos != STRING_NOTFOUND )
    {
        xub_StrLen  nStrStartAfter = nReplPos + aReplStr.Len();
        aTxtAfter = String( aTxtBefore, nStrStartAfter, aTxtBefore.Len() - nStrStartAfter );
        aTxtBefore.Erase( nReplPos );
    }

    // arrange and fill Fixed Texts
    long            nX = aBasePos.X();
    long            nWidth = GetTextWidth( aTxtBefore );

    mrFTbefore.SetText( aTxtBefore );
    mrFTbefore.setPosSizePixel( nX, nYFT, nWidth, nTextHeight );

    nX += nWidth;
    nX += aMetricVals.Width();
    mrCtrl.SetPosPixel( Point( nX, nYCtrl ) );

    nX += mrCtrl.GetSizePixel().Width();
    nX += aMetricVals.Width();
    mrFTafter.SetText( aTxtAfter );
    mrFTafter.setPosSizePixel( nX, nYFT, GetTextWidth( aTxtAfter ), nTextHeight );

    _rFTcomplete.Hide();

    Show();

    Window::Hide();
}

void TextControlCombo::Show( sal_Bool _bVisible, sal_uInt16 _nFlags )
{
    mrCtrl.Show( _bVisible, _nFlags );
    mrFTbefore.Show( _bVisible, _nFlags );
    mrFTafter.Show( _bVisible, _nFlags );
}

void TextControlCombo::Enable( sal_Bool _bEnable, sal_Bool _bChild )
{
    mrCtrl.Enable( _bEnable, _bChild );
    mrFTbefore.Enable( _bEnable, _bChild );
    mrFTafter.Enable( _bEnable, _bChild );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
