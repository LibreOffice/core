/*************************************************************************
 *
 *  $RCSfile: textcontrolcombo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 13:57:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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

void TextControlCombo::Arrange( FixedText& _rFTcomplete, BOOL bShow )
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

void TextControlCombo::Show( BOOL _bVisible, USHORT _nFlags )
{
    mrCtrl.Show( _bVisible, _nFlags );
    mrFTbefore.Show( _bVisible, _nFlags );
    mrFTafter.Show( _bVisible, _nFlags );
}

void TextControlCombo::Enable( BOOL _bEnable, BOOL _bChild )
{
    mrCtrl.Enable( _bEnable, _bChild );
    mrFTbefore.Enable( _bEnable, _bChild );
    mrFTafter.Enable( _bEnable, _bChild );
}


