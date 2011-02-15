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
#include "precompiled_svtools.hxx"

/* Test the SfxUShortRanges class */

#ifndef _SFXITEMS_HXX
#include <sfxitems.hxx>
#endif

sal_uInt16 nValues1[] = {340,472,482,4288,4374,4890,5026,5356,5724,6130,6186,6666,9222,9764,9908,
                        10110,11302,11748,11946,12298,12344,12642,12958,13076,13410,13496,14462,
                        14480,14592,16292,16788,17102,17792,17844,18182,18396,18522,18974,19292,
                        19630,20336,21808,21972,22382,23568,25156,26978,27158,27730,28014,28280,
                        28774,29258,30920,30938,31356,31582,32194,32734,32893,33757,33911,34013,
                        34565,35283,38895,40273,41281,42157,44429,44553,45825,46367,48163,48417,
                        49501,50499,52105,53035,53715,53899,54015,54131,54865,55155,55507,56325,
                        57045,59243,59833,59859,59951,60361,61387,62453,62703,63155,63517,63621,
                        63895,0};

sal_uInt16 nValues2[] = {2362,3100,3286,3548,4056,4224,5058,5198,5398,5990,6910,7034,7046,8574,8762,
                    9360,9494,9720,11432,12900,12946,13626,13844,14344,15028,16306,16494,16972,
                    17340,17504,17610,18270,18506,19832,19962,20356,20792,21146,21270,21722,
                    22486,22752,23554,24148,24210,24910,25006,25500,26464,26952,27216,28394,
                    28398,29602,31130,31642,31894,33343,34191,34593,34931,36221,36637,37287,
                    37765,39853,41231,41711,41799,44213,44863,46491,47459,48707,48911,49353,
                    49923,49997,50117,51357,51519,52043,52563,52727,53781,54065,54249,55333,
                    56549,57361,57777,58165,58507,59229,59719,60379,60845,62697,63123,65169,0};

#include <sv.hxx>

SfxUShortRanges aRanges1a( nValues1 );
SfxUShortRanges aRanges1b( nValues1 );
SfxUShortRanges aRanges1c( nValues1 );
SfxUShortRanges aRanges2( nValues2 );


String aStr1;
String aStr2;
String aStr3;


static String MakeString_Impl( SfxUShortRanges& aRanges )
{
    String aStr;

    sal_uInt16 nPos = 0;
    const sal_uInt16* pRanges = ( const sal_uInt16*)aRanges;

    while( pRanges[ nPos ] != 0 )
    {
        aStr += pRanges[ nPos ];
        aStr += " - ";
        aStr += pRanges[ nPos+1 ];
        aStr += ", ";
        nPos += 2;
    }


    return aStr;
}


// -------------------------------------------------------------------------

class MyApp : public Application
{
public:
  void Main( int, char* [] );
};

// -------------------------------------------------------------------------

class MyDialog : public ModalDialog
{
private:
    GroupBox aGroup1;
    MultiLineEdit aEdit1;
    GroupBox aGroup2;
    MultiLineEdit aEdit2;
    GroupBox aGroup3;
    MultiLineEdit aEdit3;
    OKButton      aBtn1;
    CancelButton  aBtn2;

public:
  MyDialog( Window* pParent );

  void MouseButtonDown( const MouseEvent& rMEvt );
};

// -------------------------------------------------------------------------

MyDialog::MyDialog( Window* pParent ) :
    ModalDialog( pParent, WB_STDMODAL | WB_3DLOOK ),
    aGroup1( this ),
    aEdit1( this, WB_BORDER | WB_READONLY ),
    aGroup2( this ),
    aEdit2( this, WB_BORDER | WB_READONLY ),
    aGroup3( this ),
    aEdit3( this, WB_BORDER | WB_READONLY ),
    aBtn1( this, WB_DEFBUTTON ),
    aBtn2( this )
{
    aGroup1.SetText( "Union" );
    aGroup1.SetPosSizePixel( Point( 5, 5 ), Size( 330, 80 ) );
    aGroup1.Show();

    aEdit1.SetPosSizePixel( Point ( 10, 20 ), Size( 320, 60 ) );
    aEdit1.SetText( aStr1 );
    aEdit1.Show();

    aGroup2.SetText( "Difference" );
    aGroup2.SetPosSizePixel( Point( 5, 90 ), Size( 330, 80 ) );
    aGroup2.Show();

    aEdit2.SetPosSizePixel( Point( 10, 105 ), Size( 320, 60 ) );
    aEdit2.SetText( aStr2 );
    aEdit2.Show();

    aGroup3.SetText( "Intersection" );
    aGroup3.SetPosSizePixel( Point( 5, 175 ), Size( 330, 80 ) );
    aGroup3.Show();

    aEdit3.SetPosSizePixel( Point( 10, 190 ), Size( 320, 60 ) );
    aEdit3.SetText( aStr3 );
    aEdit3.Show();

    aBtn1.SetPosSizePixel( Point( 10, 310 ), Size( 100, 25 ) );
    aBtn1.Show();

    aBtn2.SetPosSizePixel( Point( 120, 310 ), Size( 100, 25 ) );
    aBtn2.Show();

    SetOutputSizePixel( Size( 340, 345 ) );
    SetText( "SfxUShortRanges Test" );
}

// -------------------------------------------------------------------------

void MyDialog::MouseButtonDown( const MouseEvent& rMEvt )
{
}


// -------------------------------------------------------------------------

class MyWin : public WorkWindow
{
public:
  MyWin( Window* pParent, WinBits aWinStyle ) :
    WorkWindow( pParent, aWinStyle )
  {}

  void MouseButtonDown( const MouseEvent& );
};


// -------------------------------------------------------------------------

MyApp aMyApp;

void MyApp::Main( int, char* [] )
{
    EnableSVLook();
    SetAppFont( System::GetStandardFont( STDFONT_APP ) );

    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( "SfxUShortRanges - Test" );
    aMainWin.Show();

    aRanges1a += aRanges2;
    aRanges1b -= aRanges2;
    aRanges1c /= aRanges2;

    aStr1 = MakeString_Impl( aRanges1a );
    aStr2 = MakeString_Impl( aRanges1b );
    aStr3 = MakeString_Impl( aRanges1c );

    Execute();
}

// -------------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
  if( rMEvt.IsLeft() )
    {
      MyDialog* pDialog = new MyDialog( this );
      pDialog->Execute();
      delete pDialog;
    }
}

