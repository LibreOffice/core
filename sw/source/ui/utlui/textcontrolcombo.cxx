/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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
    const String    aReplStr( RTL_CONSTASCII_STRINGPARAM( "%POSITION_OF_CONTROL" ) );
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
    mrFTbefore.SetPosSizePixel( nX, nYFT, nWidth, nTextHeight );

    nX += nWidth;
    nX += aMetricVals.Width();
    mrCtrl.SetPosPixel( Point( nX, nYCtrl ) );

    nX += mrCtrl.GetSizePixel().Width();
    nX += aMetricVals.Width();
    mrFTafter.SetText( aTxtAfter );
    mrFTafter.SetPosSizePixel( nX, nYFT, GetTextWidth( aTxtAfter ), nTextHeight );

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


