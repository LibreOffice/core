/*************************************************************************
 *
 *  $RCSfile: sanedlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:52 $
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

#include <stdio.h>
#include <stdlib.h>

#include <vcl/config.hxx>
#include <vcl/msgbox.hxx>
#include <sanedlg.hxx>
#include <sanedlg.hrc>
#include <grid.hxx>

#define USE_SAVE_STATE
#undef  SAVE_ALL_STATES

ResId SaneResId( ULONG nID )
{
    static ResMgr* pResMgr = ResMgr::CreateResMgr( "san" MAKE_NUMSTR(SUPD) );
    return ResId( nID, pResMgr );
}

SaneDlg::SaneDlg( Window* pParent, Sane& rSane ) :
        ModalDialog( pParent, SaneResId( RID_SANE_DIALOG ) ),
        mrSane( rSane ),
        mpRange( 0 ),
        maMapMode( MAP_APPFONT ),
        mbIsDragging( FALSE ),
        mbDragDrawn( FALSE ),
        maOKButton( this, SaneResId( RID_SCAN_OK ) ),
        maCancelButton( this, SaneResId( RID_SCAN_CANCEL ) ),
        maPreviewButton( this, SaneResId( RID_PREVIEW_BTN ) ),
        maDeviceInfoButton( this, SaneResId( RID_DEVICEINFO_BTN ) ),
        maPreviewBox( this, SaneResId( RID_PREVIEW_BOX ) ),
        maAreaBox( this, SaneResId( RID_SCANAREA_BOX ) ),
        maDeviceBoxTxt( this, SaneResId( RID_DEVICE_BOX_TXT ) ),
        maScanLeftTxt( this, SaneResId( RID_SCAN_LEFT_TXT ) ),
        maScanTopTxt( this, SaneResId( RID_SCAN_TOP_TXT ) ),
        maRightTxt( this, SaneResId( RID_SCAN_RIGHT_TXT ) ),
        maBottomTxt( this, SaneResId( RID_SCAN_BOTTOM_TXT ) ),
        maReslTxt( this, SaneResId( RID_SCAN_RESOLUTION_TXT ) ),
        maOptionTitle( this, SaneResId( RID_SCAN_OPTIONTITLE_TXT ) ),
        maOptionsTxt( this, SaneResId( RID_SCAN_OPTION_TXT ) ),
        maOptionDescTxt( this, SaneResId( RID_SCAN_OPTION_DESC_TXT ) ),
        maVectorTxt( this, SaneResId( RID_SCAN_NUMERIC_VECTOR_TXT ) ),
        maLeftField( this, SaneResId( RID_SCAN_LEFT_BOX ) ),
        maTopField( this, SaneResId( RID_SCAN_TOP_BOX ) ),
        maRightField( this, SaneResId( RID_SCAN_RIGHT_BOX ) ),
        maBottomField( this, SaneResId( RID_SCAN_BOTTOM_BOX ) ),
        maDeviceBox( this, SaneResId( RID_DEVICE_BOX ) ),
        maOptionBox( this, SaneResId( RID_SCAN_OPTION_BOX ) ),
        maReslBox( this, SaneResId( RID_SCAN_RESOLUTION_BOX ) ),
        maBoolCheckBox( this, SaneResId( RID_SCAN_BOOL_OPTION_BOX ) ),
        maStringEdit( this, SaneResId( RID_SCAN_STRING_OPTION_EDT ) ),
        maQuantumRangeBox( this, SaneResId( RID_SCAN_QUANTUM_RANGE_BOX ) ),
        maStringRangeBox( this, SaneResId( RID_SCAN_STRING_RANGE_BOX ) ),
        maNumericEdit( this, SaneResId( RID_SCAN_NUMERIC_OPTION_EDT ) ),
        maButtonOption( this, SaneResId( RID_SCAN_BUTTON_OPTION_BTN ) ),
        maVectorBox( this, SaneResId( RID_SCAN_NUMERIC_VECTOR_BOX ) ),
        maAdvancedBox( this, SaneResId( RID_SCAN_ADVANCED_BOX ) ),
        maAdvancedTxt( this, SaneResId( RID_SCAN_ADVANCED_TXT ) )
{
    if( Sane::IsSane() )
    {
        InitDevices(); // opens first sane device
        DisableOption();
        InitFields();
    }

    maDeviceInfoButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maPreviewButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maButtonOption.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maDeviceBox.SetSelectHdl( LINK( this, SaneDlg, SelectHdl ) );
    maOptionBox.SetSelectHdl( LINK( this, SaneDlg, OptionsBoxSelectHdl ) );
    maOKButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maCancelButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maBoolCheckBox.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maStringEdit.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maNumericEdit.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maVectorBox.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maReslBox.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maStringRangeBox.SetSelectHdl( LINK( this, SaneDlg, SelectHdl ) );
    maQuantumRangeBox.SetSelectHdl( LINK( this, SaneDlg, SelectHdl ) );
    maLeftField.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maRightField.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maTopField.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maBottomField.SetModifyHdl( LINK( this, SaneDlg, ModifyHdl ) );
    maAdvancedBox.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );

    maOldLink = mrSane.SetReloadOptionsHdl( LINK( this, SaneDlg, ReloadSaneOptionsHdl ) );

    maOptionBox.SetNodeBitmaps(
        Bitmap( SaneResId( RID_SCAN_BITMAP_PLUS ) ),
        Bitmap( SaneResId( RID_SCAN_BITMAP_MINUS ) )
        );
    maOptionBox.SetWindowBits( WB_HASLINES              |
                               WB_HASBUTTONS            |
                               WB_NOINITIALSELECTION    |
                               WB_HASBUTTONSATROOT      |
                               WB_HASLINESATROOT
                               );
    FreeResource();
}

SaneDlg::~SaneDlg()
{
}

short SaneDlg::Execute()
{
    if( ! Sane::IsSane() )
    {
        ErrorBox aErrorBox( NULL, WB_OK | WB_DEF_OK,
                            String( SaneResId( RID_SANE_NOSANELIB_TXT ) ) );
        aErrorBox.Execute();
        return FALSE;
    }
    LoadState();
    return ModalDialog::Execute();
}

void SaneDlg::InitDevices()
{
    if( ! Sane::IsSane() )
        return;

    if( mrSane.IsOpen() )
        mrSane.Close();
    mrSane.ReloadDevices();
    maDeviceBox.Clear();
    for( int i = 0; i < Sane::CountDevices(); i++ )
        maDeviceBox.InsertEntry( Sane::GetName( i ) );
    if( Sane::CountDevices() )
    {
        mrSane.Open( 0 );
        maDeviceBox.SelectEntry( Sane::GetName( 0 ) );

    }
}

void SaneDlg::InitFields()
{
    if( ! Sane::IsSane() )
        return;

    int nOption, i, n, nValue;
    double fValue;
    BOOL bSuccess = FALSE;
    char *ppSpecialOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y",
        "preview"
    };

    maReslBox.Clear();
    maMinTopLeft = Point( 0, 0 );
    maMaxBottomRight = Point( PREVIEW_WIDTH,  PREVIEW_HEIGHT );

    if( ! mrSane.IsOpen() )
        return;

    // set Resolution
    nOption = mrSane.GetOptionByName( "resolution" );
    if( nOption != -1 )
    {
        double fRes;

        bSuccess = mrSane.GetOptionValue( nOption, fRes );
        if( bSuccess )
        {
            maReslBox.Enable( TRUE );

            maReslBox.SetValue( (long)fRes );
            double *pDouble = NULL;
            nValue = mrSane.GetRange( nOption, pDouble );
            if( nValue > -1 )
            {
                if( nValue )
                {
                    maReslBox.SetMin( (long)pDouble[0] );
                    maReslBox.SetMax( (long)pDouble[ nValue-1 ] );
                    for( i=0; i<nValue; i++ )
                    {
                        if( i == 0 || i == nValue-1 || ! ( ((int)pDouble[i]) % 20) )
                            maReslBox.InsertValue( (long)pDouble[i] );
                    }
                }
                else
                {
                    maReslBox.SetMin( (long)pDouble[0] );
                    maReslBox.SetMax( (long)pDouble[1] );
                    maReslBox.InsertValue( (long)pDouble[0] );
                    maReslBox.InsertValue( (long)pDouble[1] );
                }
                if( pDouble )
                    delete pDouble;
            }
            else
                maReslBox.Enable( FALSE );
        }
    }
    else
        maReslBox.Enable( FALSE );

    // set scan area
    for( i = 0; i < 4; i++ )
    {
        char *pOptionName;
        MetricField* pField;
        switch( i )
        {
            case 0:
                pOptionName = "tl-x";
                pField = &maLeftField;
                break;
            case 1:
                pOptionName = "tl-y";
                pField = &maTopField;
                break;
            case 2:
                pOptionName = "br-x";
                pField = &maRightField;
                break;
            case 3:
                pOptionName = "br-y";
                pField = &maBottomField;
        }
        nOption = mrSane.GetOptionByName( pOptionName );
        bSuccess = FALSE;
        if( nOption != -1 )
        {
            bSuccess = mrSane.GetOptionValue( nOption, fValue, 0 );
            if( bSuccess )
            {
                if( mrSane.GetOptionUnit( nOption ) == SANE_UNIT_MM )
                {
                    pField->SetUnit( FUNIT_MM );
                    pField->SetValue( (int)fValue, FUNIT_MM );
                }
                else // SANE_UNIT_PIXEL
                {
                    pField->SetValue( (int)fValue, FUNIT_CUSTOM );
                    pField->SetCustomUnitText( String::CreateFromAscii( "Pixel" ) );
                }
                switch( i ) {
                    case 0: maTopLeft.X() = (int)fValue;break;
                    case 1: maTopLeft.Y() = (int)fValue;break;
                    case 2: maBottomRight.X() = (int)fValue;break;
                    case 3: maBottomRight.Y() = (int)fValue;break;
                }
            }
            double *pDouble = NULL;
            nValue = mrSane.GetRange( nOption, pDouble );
            if( nValue > -1 )
            {
                if( pDouble )
                {
                    pField->SetMin( (long)pDouble[0] );
                    if( nValue )
                        pField->SetMax( (long)pDouble[ nValue-1 ] );
                    else
                        pField->SetMax( (long)pDouble[ 1 ] );
                    delete pDouble;
                }
                switch( i ) {
                    case 0: maMinTopLeft.X() = pField->GetMin();break;
                    case 1: maMinTopLeft.Y() = pField->GetMin();break;
                    case 2: maMaxBottomRight.X() = pField->GetMax();break;
                    case 3: maMaxBottomRight.Y() = pField->GetMax();break;
                }
            }
            else
            {
                switch( i ) {
                    case 0: maMinTopLeft.X() = (int)fValue;break;
                    case 1: maMinTopLeft.Y() = (int)fValue;break;
                    case 2: maMaxBottomRight.X() = (int)fValue;break;
                    case 3: maMaxBottomRight.Y() = (int)fValue;break;
                }
            }
            pField->Enable( TRUE );
        }
        else
            pField->Enable( FALSE );
    }
    maTopLeft = GetPixelPos( maTopLeft );
    maBottomRight = GetPixelPos( maBottomRight );
    maPreviewRect = Rectangle( maTopLeft,
                               Size( maBottomRight.X() - maTopLeft.X(),
                                     maBottomRight.Y() - maTopLeft.Y() )
                               );
    // fill OptionBox
    maOptionBox.Clear();
    SvLBoxEntry* pParentEntry = 0;
    BOOL bGroupRejected = FALSE;
    for( i = 1; i < mrSane.CountOptions(); i++ )
    {
        String aOption=mrSane.GetOptionName( i );
        BOOL bInsertAdvanced =
            mrSane.GetOptionCap( i ) & SANE_CAP_ADVANCED &&
            ! maAdvancedBox.IsChecked() ? FALSE : TRUE;
        if( mrSane.GetOptionType( i ) == SANE_TYPE_GROUP )
        {
            if( bInsertAdvanced )
            {
                aOption = mrSane.GetOptionTitle( i );
                pParentEntry = maOptionBox.InsertEntry( aOption );
                bGroupRejected = FALSE;
            }
            else
                bGroupRejected = TRUE;
        }
        else if( aOption.Len() &&
                 ! ( mrSane.GetOptionCap( i ) &
                     (
                         SANE_CAP_HARD_SELECT |
                         SANE_CAP_INACTIVE
                         ) ) &&
                 bInsertAdvanced && ! bGroupRejected )
        {
            BOOL bIsSpecial = FALSE;
            for( n = 0; !bIsSpecial &&
                     n < sizeof(ppSpecialOptions)/sizeof(ppSpecialOptions[0]); n++ )
            {
                if( aOption.EqualsAscii( ppSpecialOptions[n] ) )
                    bIsSpecial=TRUE;
            }
            if( ! bIsSpecial )
            {
                if( pParentEntry )
                    maOptionBox.InsertEntry( aOption, pParentEntry );
                else
                    maOptionBox.InsertEntry( aOption );
            }
        }
    }
}

IMPL_LINK( SaneDlg, ClickBtnHdl, Button*, pButton )
{
    if( mrSane.IsOpen() )
    {
        if( pButton == &maDeviceInfoButton )
        {
            String aString( SaneResId( RID_SANE_DEVICEINFO_TXT ) );
            String aSR( RTL_CONSTASCII_USTRINGPARAM( "%s" ) );
            aString.SearchAndReplace( aSR, Sane::GetName( mrSane.GetDeviceNumber() ) );
            aString.SearchAndReplace( aSR, Sane::GetVendor( mrSane.GetDeviceNumber() ) );
            aString.SearchAndReplace( aSR, Sane::GetModel( mrSane.GetDeviceNumber() ) );
            aString.SearchAndReplace( aSR, Sane::GetType( mrSane.GetDeviceNumber() ) );
            InfoBox aInfoBox( this, aString );
            aInfoBox.Execute();
        }
        else if( pButton == &maPreviewButton )
            AcquirePreview();
        else if( pButton == &maBoolCheckBox )
        {
            mrSane.SetOptionValue( mnCurrentOption,
                                   maBoolCheckBox.IsChecked() ?
                                   (BOOL)TRUE : (BOOL)FALSE );
        }
        else if( pButton == &maButtonOption )
        {
            mrSane.ActivateButtonOption( mnCurrentOption );
        }
        else if( pButton == &maAdvancedBox )
        {
            ReloadSaneOptionsHdl( NULL );
        }
    }
    if( pButton == &maOKButton )
    {
        double fRes = (double)maReslBox.GetValue();
        SetAdjustedNumericalValue( "resolution", fRes );
        mrSane.SetReloadOptionsHdl( maOldLink );
        UpdateScanArea( TRUE );
        SaveState();
        EndDialog( mrSane.IsOpen() ? 1 : 0 );
    }
    else if( pButton == &maCancelButton )
    {
        mrSane.SetReloadOptionsHdl( maOldLink );
        mrSane.Close();
        EndDialog( 0 );
    }
    return 0;
}

IMPL_LINK( SaneDlg, SelectHdl, ListBox*, pListBox )
{
    if( pListBox == &maDeviceBox && Sane::IsSane() && Sane::CountDevices() )
    {
        String aNewDevice = maDeviceBox.GetSelectEntry();
        int nNumber;
        if( aNewDevice.Equals( Sane::GetName( nNumber = mrSane.GetDeviceNumber() ) ) )
        {
            mrSane.Close();
            mrSane.Open( nNumber );
            InitFields();
        }
    }
    if( mrSane.IsOpen() )
    {
        if( pListBox == &maQuantumRangeBox )
        {
            ByteString aValue( maQuantumRangeBox.GetSelectEntry(), gsl_getSystemTextEncoding() );
            double fValue = atof( aValue.GetBuffer() );
            mrSane.SetOptionValue( mnCurrentOption, fValue, mnCurrentElement );
        }
        else if( pListBox == &maStringRangeBox )
        {
            mrSane.SetOptionValue( mnCurrentOption, maStringRangeBox.GetSelectEntry() );
        }
    }
    return 0;
}

IMPL_LINK( SaneDlg, OptionsBoxSelectHdl, SvTreeListBox*, pBox )
{
    if( pBox == &maOptionBox && Sane::IsSane() )
    {
        String aOption =
            maOptionBox.GetEntryText( maOptionBox.FirstSelected() );
        int nOption = mrSane.GetOptionByName( ByteString( aOption, gsl_getSystemTextEncoding() ).GetBuffer() );
        if( nOption != -1 && nOption != mnCurrentOption )
        {
            DisableOption();
            mnCurrentOption = nOption;
            maOptionTitle.SetText( mrSane.GetOptionTitle( mnCurrentOption ) );
            SANE_Value_Type nType = mrSane.GetOptionType( mnCurrentOption );
            SANE_Constraint_Type nConstraint;
            switch( nType )
            {
                case SANE_TYPE_BOOL:    EstablishBoolOption();break;
                case SANE_TYPE_STRING:
                    nConstraint = mrSane.GetOptionConstraintType( mnCurrentOption );
                    if( nConstraint == SANE_CONSTRAINT_STRING_LIST )
                        EstablishStringRange();
                    else
                        EstablishStringOption();
                    break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    nConstraint = mrSane.GetOptionConstraintType( mnCurrentOption );
                    mnCurrentElement = 0;
                    if( nConstraint == SANE_CONSTRAINT_RANGE ||
                        nConstraint == SANE_CONSTRAINT_WORD_LIST )
                        EstablishQuantumRange();
                    else
                    {
                        mfMin = mfMax = 0.0;
                        EstablishNumericOption();
                    }
                    int nElements = mrSane.GetOptionElements( mnCurrentOption );
                    if( nElements > 1 )
                    {
                        if( nElements <= 10 )
                        {
                            maVectorBox.SetValue( 1 );
                            maVectorBox.SetMin( 1 );
                            maVectorBox.SetMax(
                                mrSane.GetOptionElements( mnCurrentOption ) );
                            maVectorBox.Show( TRUE );
                            maVectorTxt.Show( TRUE );
                        }
                        else
                        {
                            double* x = new double[ nElements ];
                            double* y = new double[ nElements ];
                            for( int i = 0; i < nElements; i++ )
                            {
                                x[ i ] = (double)i;
                                mrSane.GetOptionValue( mnCurrentOption, x[i], i );
                            }
                            GridWindow aGrid( x, y, nElements, this );
                            aGrid.SetText( mrSane.GetOptionName( mnCurrentOption ) );
                            aGrid.setBoundings( 0, mfMin, nElements, mfMax );
                            aGrid.Execute();

                            delete x;
                            delete y;
                        }
                    }
                }
                break;
                case SANE_TYPE_BUTTON:
                    EstablishButtonOption();
                    break;
                default: break;
            }
        }
    }
    return 0;
}

IMPL_LINK( SaneDlg, ModifyHdl, Edit*, pEdit )
{
    if( mrSane.IsOpen() )
    {
        if( pEdit == &maStringEdit )
        {
            mrSane.SetOptionValue( mnCurrentOption, maStringEdit.GetText() );
        }
        else if( pEdit == &maReslBox )
        {
            double fRes = (double)maReslBox.GetValue();
            int nOption = mrSane.GetOptionByName( "resolution" );
            if( nOption != -1 )
            {
                double* pDouble = NULL;
                int nValues = mrSane.GetRange( nOption, pDouble );
                if( nValues > 0 )
                {
                    int i;
                    for( i = 0; i < nValues; i++ )
                    {
                        if( fRes == pDouble[i] )
                            break;
                    }
                    if( i >= nValues )
                        fRes = pDouble[0];
                }
                else if( nValues == 0 )
                {
                    if( fRes < pDouble[ 0 ] )
                        fRes = pDouble[ 0 ];
                    if( fRes > pDouble[ 1 ] )
                        fRes = pDouble[ 1 ];
                }
                maReslBox.SetValue( (ULONG)fRes );
            }
        }
        else if( pEdit == &maNumericEdit )
        {
            double fValue;
            char pBuf[256];
            ByteString aContents( maNumericEdit.GetText(), gsl_getSystemTextEncoding() );
            fValue = atof( aContents.GetBuffer() );
            if( mfMin != mfMax && ( fValue < mfMin || fValue > mfMax ) )
            {
                if( fValue < mfMin )
                    fValue = mfMin;
                else if( fValue > mfMax )
                fValue = mfMax;
                sprintf( pBuf, "%g", fValue );
                maNumericEdit.SetText( String( pBuf, gsl_getSystemTextEncoding() ) );
            }
            mrSane.SetOptionValue( mnCurrentOption, fValue, mnCurrentElement );
        }
        else if( pEdit == &maVectorBox )
        {
            char pBuf[256];
            mnCurrentElement = maVectorBox.GetValue()-1;
            double fValue;
            mrSane.GetOptionValue( mnCurrentOption, fValue, mnCurrentElement );
            sprintf( pBuf, "%g", fValue );
            String aValue( pBuf, gsl_getSystemTextEncoding() );
            maNumericEdit.SetText( aValue );
            maQuantumRangeBox.SelectEntry( aValue );
        }
        else if( pEdit == &maTopField )
        {
            Point aPoint( 0, maTopField.GetValue() );
            aPoint = GetPixelPos( aPoint );
            maTopLeft.Y() = aPoint.Y();
            DrawDrag();
        }
        else if( pEdit == &maLeftField )
        {
            Point aPoint( maLeftField.GetValue(), 0 );
            aPoint = GetPixelPos( aPoint );
            maTopLeft.X() = aPoint.X();
            DrawDrag();
        }
        else if( pEdit == &maBottomField )
        {
            Point aPoint( 0, maBottomField.GetValue() );
            aPoint = GetPixelPos( aPoint );
            maBottomRight.Y() = aPoint.Y();
            DrawDrag();
        }
        else if( pEdit == &maRightField )
        {
            Point aPoint( maRightField.GetValue(), 0 );
            aPoint = GetPixelPos( aPoint );
            maBottomRight.X() = aPoint.X();
            DrawDrag();
        }
    }
    return 0;
}

IMPL_LINK( SaneDlg, ReloadSaneOptionsHdl, Sane*, pSane )
{
     mnCurrentOption = -1;
     mnCurrentElement = 0;
     DisableOption();
    InitFields();
    Rectangle aDummyRect( Point( 0, 0 ), GetSizePixel() );
    Paint( aDummyRect );
    return 0;
}

void SaneDlg::AcquirePreview()
{
    if( ! mrSane.IsOpen() )
        return;

    UpdateScanArea( TRUE );
    // set small resolution for preview
    double fResl = (double)maReslBox.GetValue();
    SetAdjustedNumericalValue( "resolution", 30.0 );

    int nOption = mrSane.GetOptionByName( "preview" );
    if( nOption == -1 )
    {
        String aString( SaneResId( RID_SANE_NORESOLUTIONOPTION_TXT ) );
        WarningBox aBox( this, WB_OK_CANCEL | WB_DEF_OK, aString );
        if( aBox.Execute() == RET_CANCEL )
            return;
    }
    else
        mrSane.SetOptionValue( nOption, (BOOL)TRUE );

    BitmapTransporter aTransporter;
    if( ! mrSane.Start( aTransporter ) )
    {
        ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK,
                            String( SaneResId( RID_SANE_SCANERROR_TXT ) ) );
        aErrorBox.Execute();
    }
    else
    {
#ifdef DEBUG
        aTransporter.getStream().Seek( STREAM_SEEK_TO_END );
        fprintf( stderr, "Previewbitmapstream contains %d bytes\n", aTransporter.getStream().Tell() );
#endif
        aTransporter.getStream().Seek( STREAM_SEEK_TO_BEGIN );
        maPreviewBitmap.Read( aTransporter.getStream(), TRUE );
    }

    SetAdjustedNumericalValue( "resolution", fResl );
    maReslBox.SetValue( (ULONG)fResl );

    maPreviewRect = Rectangle( maTopLeft,
                               Size( maBottomRight.X() - maTopLeft.X(),
                                     maBottomRight.Y() - maTopLeft.Y() )
                               );
    Paint( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
}

void SaneDlg::Paint( const Rectangle& rRect )
{
    SetMapMode( maMapMode );
    SetFillColor( Color( COL_WHITE ) );
    SetLineColor( Color( COL_WHITE ) );
    DrawRect( Rectangle( Point( PREVIEW_UPPER_LEFT, PREVIEW_UPPER_TOP ),
                         Size( PREVIEW_WIDTH, PREVIEW_HEIGHT ) ) );
    SetMapMode( MapMode( MAP_PIXEL ) );
    // check for sane values
    DrawBitmap( maPreviewRect.TopLeft(), maPreviewRect.GetSize(),
                maPreviewBitmap );

    mbDragDrawn = FALSE;
    DrawDrag();

    ModalDialog::Paint( rRect );
}

void SaneDlg::DisableOption()
{
    maBoolCheckBox.Show( FALSE );
    maStringEdit.Show( FALSE );
    maNumericEdit.Show( FALSE );
    maQuantumRangeBox.Show( FALSE );
    maStringRangeBox.Show( FALSE );
    maButtonOption.Show( FALSE );
    maVectorBox.Show( FALSE );
    maVectorTxt.Show( FALSE );
    maOptionDescTxt.Show( FALSE );
}

void SaneDlg::EstablishBoolOption()
{
    BOOL bSuccess, bValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, bValue );
    if( bSuccess )
    {
        maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
        maOptionDescTxt.Show( TRUE );
        maBoolCheckBox.Check( bValue );
        maBoolCheckBox.Show( TRUE );
    }
}

void SaneDlg::EstablishStringOption()
{
    BOOL bSuccess;
    ByteString aValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, aValue );
    if( bSuccess )
    {
        maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
        maOptionDescTxt.Show( TRUE );
        maStringEdit.SetText( String( aValue, gsl_getSystemTextEncoding() ) );
        maStringEdit.Show( TRUE );
    }
}

void SaneDlg::EstablishStringRange()
{
    const char** ppStrings = mrSane.GetStringConstraint( mnCurrentOption );
    maStringRangeBox.Clear();
    for( int i = 0; ppStrings[i] != 0; i++ )
        maStringRangeBox.InsertEntry( String( ppStrings[i], gsl_getSystemTextEncoding() ) );
    ByteString aValue;
    mrSane.GetOptionValue( mnCurrentOption, aValue );
    maStringRangeBox.SelectEntry( String( aValue, gsl_getSystemTextEncoding() ) );
    maStringRangeBox.Show( TRUE );
    maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
    maOptionDescTxt.Show( TRUE );
}

void SaneDlg::EstablishQuantumRange()
{
    if( mpRange )
    {
        delete mpRange;
        mpRange = 0;
    }
    int nValues = mrSane.GetRange( mnCurrentOption, mpRange );
    if( nValues == 0 )
    {
        mfMin = mpRange[ 0 ];
        mfMax = mpRange[ 1 ];
        delete mpRange;
        mpRange = 0;
        EstablishNumericOption();
    }
    else if( nValues > 0 )
    {
        char pBuf[ 256 ];
        maQuantumRangeBox.Clear();
        mfMin = mpRange[ 0 ];
        mfMax = mpRange[ nValues-1 ];
        for( int i = 0; i < nValues; i++ )
        {
            sprintf( pBuf, "%g", mpRange[ i ] );
            maQuantumRangeBox.InsertEntry( String( pBuf, gsl_getSystemTextEncoding() ) );
        }
        double fValue;
        if( mrSane.GetOptionValue( mnCurrentOption, fValue, mnCurrentElement ) )
        {
            sprintf( pBuf, "%g", fValue );
            maQuantumRangeBox.SelectEntry( String( pBuf, gsl_getSystemTextEncoding() ) );
        }
        maQuantumRangeBox.Show( TRUE );
        String aText( mrSane.GetOptionName( mnCurrentOption ) );
        aText += ' ';
        aText += mrSane.GetOptionUnitName( mnCurrentOption );
        maOptionDescTxt.SetText( aText );
        maOptionDescTxt.Show( TRUE );
    }
}

void SaneDlg::EstablishNumericOption()
{
    BOOL bSuccess;
    double fValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, fValue );
    if( ! bSuccess )
        return;

    char pBuf[256];
    String aText( mrSane.GetOptionName( mnCurrentOption ) );
    aText += ' ';
    aText += mrSane.GetOptionUnitName( mnCurrentOption );
    if( mfMin != mfMax )
    {
        sprintf( pBuf, " < %g ; %g >", mfMin, mfMax );
        aText += String( pBuf, gsl_getSystemTextEncoding() );
    }
    maOptionDescTxt.SetText( aText );
    maOptionDescTxt.Show( TRUE );
    sprintf( pBuf, "%g", fValue );
    maNumericEdit.SetText( String( pBuf, gsl_getSystemTextEncoding() ) );
    maNumericEdit.Show( TRUE );
}

void SaneDlg::EstablishButtonOption()
{
    maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
    maOptionDescTxt.Show( TRUE );
    maButtonOption.Show( TRUE );
}

#define RECT_SIZE_PIX 7

void SaneDlg::MouseMove( const MouseEvent& rMEvt )
{
    if( mbIsDragging )
    {
        Point aMousePos = rMEvt.GetPosPixel();
        // move into valid area
        Point aLogicPos = GetLogicPos( aMousePos );
        aMousePos = GetPixelPos( aLogicPos );
        switch( meDragDirection )
        {
            case TopLeft:       maTopLeft = aMousePos; break;
            case Top:           maTopLeft.Y() = aMousePos.Y(); break;
            case TopRight:
                maTopLeft.Y() = aMousePos.Y();
                maBottomRight.X() = aMousePos.X();
                break;
            case Right:         maBottomRight.X() = aMousePos.X(); break;
            case BottomRight:   maBottomRight = aMousePos; break;
            case Bottom:        maBottomRight.Y() = aMousePos.Y(); break;
            case BottomLeft:
                maTopLeft.X() = aMousePos.X();
                maBottomRight.Y() = aMousePos.Y();
                break;
            case Left:          maTopLeft.X() = aMousePos.X(); break;
            default: break;
        }
        int nSwap;
        if( maTopLeft.X() > maBottomRight.X() )
        {
            nSwap = maTopLeft.X();
            maTopLeft.X() = maBottomRight.X();
            maBottomRight.X() = nSwap;
        }
        if( maTopLeft.Y() > maBottomRight.Y() )
        {
            nSwap = maTopLeft.Y();
            maTopLeft.Y() = maBottomRight.Y();
            maBottomRight.Y() = nSwap;
        }
        DrawDrag();
        UpdateScanArea( FALSE );
    }
    ModalDialog::MouseMove( rMEvt );
}

void SaneDlg::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMousePixel = rMEvt.GetPosPixel();

    if( ! mbIsDragging )
    {
        int nMiddleX = ( maBottomRight.X() - maTopLeft.X() ) / 2 - RECT_SIZE_PIX/2 + maTopLeft.X();
        int nMiddleY = ( maBottomRight.Y() - maTopLeft.Y() ) / 2 - RECT_SIZE_PIX/2 + maTopLeft.Y();
        if( aMousePixel.Y() >= maTopLeft.Y() &&
            aMousePixel.Y() < maTopLeft.Y() + RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = TopLeft;
                aMousePixel = maTopLeft;
                mbIsDragging = TRUE;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Top;
                aMousePixel.Y() = maTopLeft.Y();
                mbIsDragging = TRUE;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = TopRight;
                aMousePixel = Point( maBottomRight.X(), maTopLeft.Y() );
                mbIsDragging = TRUE;
            }
        }
        else if( aMousePixel.Y() >= nMiddleY &&
                 aMousePixel.Y() < nMiddleY + RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = Left;
                aMousePixel.X() = maTopLeft.X();
                mbIsDragging = TRUE;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = Right;
                aMousePixel.X() = maBottomRight.X();
                mbIsDragging = TRUE;
            }
        }
        else if( aMousePixel.Y() <= maBottomRight.Y() &&
                 aMousePixel.Y() > maBottomRight.Y() - RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = BottomLeft;
                aMousePixel = Point( maTopLeft.X(), maBottomRight.Y() );
                mbIsDragging = TRUE;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Bottom;
                aMousePixel.Y() = maBottomRight.Y();
                mbIsDragging = TRUE;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = BottomRight;
                aMousePixel = maBottomRight;
                mbIsDragging = TRUE;
            }
        }
    }
    if( mbIsDragging )
    {
        SetPointerPosPixel( aMousePixel );
        DrawDrag();
    }
    ModalDialog::MouseButtonDown( rMEvt );
}

void SaneDlg::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( mbIsDragging )
    {
        UpdateScanArea( TRUE );
    }
    mbIsDragging = FALSE;

    ModalDialog::MouseButtonUp( rMEvt );
}

void SaneDlg::DrawRectangles( Point& rUL, Point& rBR )
{
    int nMiddleX, nMiddleY;
    Point aBL, aUR;

    aUR = Point( rBR.X(), rUL.Y() );
    aBL = Point( rUL.X(), rBR.Y() );
    nMiddleX = ( rBR.X() - rUL.X() ) / 2 + rUL.X();
    nMiddleY = ( rBR.Y() - rUL.Y() ) / 2 + rUL.Y();

    DrawLine( rUL, aBL );
    DrawLine( aBL, rBR );
    DrawLine( rBR, aUR );
    DrawLine( aUR, rUL );
    DrawRect( Rectangle( rUL, Size( RECT_SIZE_PIX,RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( aBL, Size( RECT_SIZE_PIX, -RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( rBR, Size( -RECT_SIZE_PIX, -RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( aUR, Size( -RECT_SIZE_PIX, RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( Point( nMiddleX - RECT_SIZE_PIX/2, rUL.Y() ), Size( RECT_SIZE_PIX, RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( Point( nMiddleX - RECT_SIZE_PIX/2, rBR.Y() ), Size( RECT_SIZE_PIX, -RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( Point( rUL.X(), nMiddleY - RECT_SIZE_PIX/2 ), Size( RECT_SIZE_PIX, RECT_SIZE_PIX ) ) );
    DrawRect( Rectangle( Point( rBR.X(), nMiddleY - RECT_SIZE_PIX/2 ), Size( -RECT_SIZE_PIX, RECT_SIZE_PIX ) ) );
}

void SaneDlg::DrawDrag()
{
    static Point aLastUL, aLastBR;

    RasterOp eROP = GetRasterOp();
    SetRasterOp( ROP_INVERT );
    SetMapMode( MapMode( MAP_PIXEL ) );

    if( mbDragDrawn )
        DrawRectangles( aLastUL, aLastBR );

    aLastUL = maTopLeft;
    aLastBR = maBottomRight;
    DrawRectangles( maTopLeft, maBottomRight );

    mbDragDrawn = TRUE;
    SetRasterOp( eROP );
    SetMapMode( maMapMode );
}

Point SaneDlg::GetPixelPos( const Point& rIn )
{
    Point aConvert(
        ( ( rIn.X() * PREVIEW_WIDTH ) /
          ( maMaxBottomRight.X() - maMinTopLeft.X() ) )
        + PREVIEW_UPPER_LEFT,
        ( ( rIn.Y() * PREVIEW_HEIGHT )
          / ( maMaxBottomRight.Y() - maMinTopLeft.Y() ) )
        + PREVIEW_UPPER_TOP );

    return LogicToPixel( aConvert, maMapMode );
}

Point SaneDlg::GetLogicPos( const Point& rIn )
{
    Point aConvert = PixelToLogic( rIn, maMapMode );
    aConvert.X() -= PREVIEW_UPPER_LEFT;
    aConvert.Y() -= PREVIEW_UPPER_TOP;
    if( aConvert.X() < 0 )
        aConvert.X() = 0;
    if( aConvert.X() >= PREVIEW_WIDTH )
        aConvert.X() = PREVIEW_WIDTH-1;
    if( aConvert.Y() < 0 )
        aConvert.Y() = 0;
    if( aConvert.Y() >= PREVIEW_HEIGHT )
        aConvert.Y() = PREVIEW_HEIGHT-1;

    aConvert.X() *= ( maMaxBottomRight.X() - maMinTopLeft.X() );
    aConvert.X() /= PREVIEW_WIDTH;
    aConvert.Y() *= ( maMaxBottomRight.Y() - maMinTopLeft.Y() );
    aConvert.Y() /= PREVIEW_HEIGHT;
    return aConvert;
}

void SaneDlg::UpdateScanArea( BOOL bSend )
{
    Point aUL = GetLogicPos( maTopLeft );
    Point aBR = GetLogicPos( maBottomRight );

    maLeftField.SetValue( aUL.X() );
    maTopField.SetValue( aUL.Y() );
    maRightField.SetValue( aBR.X() );
    maBottomField.SetValue( aBR.Y() );

    if( ! bSend )
        return;

    if( mrSane.IsOpen() )
    {
        SetAdjustedNumericalValue( "tl-x", (double)aUL.X() );
        SetAdjustedNumericalValue( "tl-y", (double)aUL.Y() );
        SetAdjustedNumericalValue( "br-x", (double)aBR.X() );
        SetAdjustedNumericalValue( "br-y", (double)aBR.Y() );
    }
}

BOOL SaneDlg::LoadState()
{
#ifdef USE_SAVE_STATE
    int i;

    if( ! Sane::IsSane() )
        return FALSE;

    const char* pEnv = getenv("HOME");
    String aFileName( pEnv ? pEnv : "", gsl_getSystemTextEncoding() );
    aFileName += String( RTL_CONSTASCII_USTRINGPARAM( "/.so_sane_state" ) );
    Config aConfig( aFileName );
    if( ! aConfig.HasGroup( "SANE" ) )
        return FALSE;

    aConfig.SetGroup( "SANE" );
    ByteString aString = aConfig.ReadKey( "SO_LastSaneDevice" );
    for( i = 0; i < Sane::CountDevices() && ! aString.Equals( ByteString( Sane::GetName( i ), gsl_getSystemTextEncoding() ) ); i++ ) ;
    if( i == Sane::CountDevices() )
        return FALSE;

    mrSane.Close();
    mrSane.Open( aString.GetBuffer() );

    DisableOption();
    InitFields();

    if( mrSane.IsOpen() )
    {
        int iMax = aConfig.GetKeyCount();
        for( i = 0; i < iMax; i++ )
        {
            aString = aConfig.GetKeyName( i );
            ByteString aValue = aConfig.ReadKey( i );
            int nOption = mrSane.GetOptionByName( aString.GetBuffer() );
            if( nOption != -1 )
            {
                if( aValue.CompareTo( "BOOL=", 5 ) == COMPARE_EQUAL )
                {
                    aValue.Erase( 0, 5 );
                    BOOL aBOOL = (BOOL)aValue.ToInt32();
                    mrSane.SetOptionValue( nOption, aBOOL );
                }
                else if( aValue.CompareTo( "STRING=", 7 ) == COMPARE_EQUAL )
                {
                    aValue.Erase( 0, 7 );
                    mrSane.SetOptionValue( nOption, String( aValue, gsl_getSystemTextEncoding() ) );
                }
                else if( aValue.CompareTo( "NUMERIC=", 8 ) == COMPARE_EQUAL )
                {
                    aValue.Erase( 0, 8 );
                    int nMax = aValue.GetTokenCount( ':' );
                    double fValue=0.0;
                    for( int n = 0; n < nMax ; n++ )
                    {
                        ByteString aSub = aValue.GetToken( n, ':' );
                        sscanf( aSub.GetBuffer(), "%lg", &fValue );
                        SetAdjustedNumericalValue( aString.GetBuffer(), fValue, n );
                    }
                }
            }
        }
    }

    DisableOption();
    InitFields();

    return TRUE;
#else
    return FALSE;
#endif
}

void SaneDlg::SaveState()
{
#ifdef USE_SAVE_STATE
    if( ! Sane::IsSane() )
        return;

    const char* pEnv = getenv( "HOME" );
    String aFileName( pEnv ? pEnv : "", gsl_getSystemTextEncoding() );
    aFileName.AppendAscii( "/.so_sane_state" );

    Config aConfig( aFileName );
    aConfig.DeleteGroup( "SANE" );
    aConfig.SetGroup( "SANE" );
    aConfig.WriteKey( "SO_LastSANEDevice", ByteString( maDeviceBox.GetSelectEntry(), RTL_TEXTENCODING_UTF8 ) );

#ifdef SAVE_ALL_STATES
    for( int i = 1; i < mrSane.CountOptions(); i++ )
    {
        String aOption=mrSane.GetOptionName( i );
        SANE_Value_Type nType = mrSane.GetOptionType( i );
        switch( nType )
        {
            case SANE_TYPE_BOOL:
            {
                BOOL bValue;
                if( mrSane.GetOptionValue( i, bValue ) )
                {
                    ByteString aString( "BOOL=" );
                    aString += (ULONG)bValue;
                    aConfig.WriteKey( aOption, aString );
                }
            }
            break;
            case SANE_TYPE_STRING:
            {
                String aString( "STRING=" );
                String aValue;
                if( mrSane.GetOptionValue( i, aValue ) )
                {
                    aString += aValue;
                    aConfig.WriteKey( aOption, aString );
                }
            }
            break;
            case SANE_TYPE_FIXED:
            case SANE_TYPE_INT:
            {
                String aString( "NUMERIC=" );
                double fValue;
                char buf[256];
                for( int n = 0; n < mrSane.GetOptionElements( i ); n++ )
                {
                    if( ! mrSane.GetOptionValue( i, fValue, n ) )
                        break;
                    if( n > 0 )
                        aString += ":";
                    sprintf( buf, "%lg", fValue );
                    aString += buf;
                }
                if( n >= mrSane.GetOptionElements( i ) )
                    aConfig.WriteKey( aOption, aString );
            }
            break;
            default:
                break;
        }
     }
#else
    static char* pSaveOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y"
    };
    for( int i = 0;
         i < (sizeof(pSaveOptions)/sizeof(pSaveOptions[0]));
         i++ )
    {
        ByteString aOption = pSaveOptions[i];
        int nOption = mrSane.GetOptionByName( pSaveOptions[i] );
        if( nOption > -1 )
        {
            SANE_Value_Type nType = mrSane.GetOptionType( nOption );
            switch( nType )
            {
                case SANE_TYPE_BOOL:
                {
                    BOOL bValue;
                    if( mrSane.GetOptionValue( nOption, bValue ) )
                    {
                        ByteString aString( "BOOL=" );
                        aString += (ULONG)bValue;
                        aConfig.WriteKey( aOption, aString );
                    }
                }
                break;
                case SANE_TYPE_STRING:
                {
                    ByteString aString( "STRING=" );
                    ByteString aValue;
                    if( mrSane.GetOptionValue( nOption, aValue ) )
                    {
                        aString += aValue;
                        aConfig.WriteKey( aOption, aString );
                    }
                }
                break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    ByteString aString( "NUMERIC=" );
                    double fValue;
                    char buf[256];
                    for( int n = 0; n < mrSane.GetOptionElements( nOption ); n++ )
                    {
                        if( ! mrSane.GetOptionValue( nOption, fValue, n ) )
                            break;
                        if( n > 0 )
                            aString += ":";
                        sprintf( buf, "%lg", fValue );
                        aString += buf;
                    }
                    if( n >= mrSane.GetOptionElements( nOption ) )
                        aConfig.WriteKey( aOption, aString );
                }
                break;
                default:
                    break;
            }
        }
    }
#endif
#endif
}

BOOL SaneDlg::SetAdjustedNumericalValue(
    const char* pOption,
    double fValue,
    int nElement )
{
    int nOption;
    if( ! Sane::IsSane() || ! mrSane.IsOpen() || ( nOption = mrSane.GetOptionByName( pOption ) ) == -1 )
        return FALSE;

    if( nElement < 0 || nElement >= mrSane.GetOptionElements( nOption ) )
        return FALSE;

    double* pValues = NULL;
    int nValues;
    if( ( nValues = mrSane.GetRange( nOption, pValues ) ) < 0 )
        return FALSE;

#ifdef DEBUG
    fprintf( stderr, "SaneDlg::SetAdjustedNumericalValue( \"%s\", %lg ) ",
             pOption, fValue );
#endif

    if( nValues )
    {
        int nNearest = 0;
        double fNearest = 1e6;
        for( int i = 0; i < nValues; i++ )
        {
            if( fabs( fValue - pValues[ i ] ) < fNearest )
            {
                fNearest = fabs( fValue - pValues[ i ] );
                nNearest = i;
            }
        }
        fValue = pValues[ nNearest ];
    }
    else
    {
        if( fValue < pValues[0] )
            fValue = pValues[0];
        if( fValue > pValues[1] )
            fValue = pValues[1];
    }
    delete pValues;
    mrSane.SetOptionValue( nOption, fValue, nElement );
#ifdef DEBUG
    fprintf( stderr, "yields %lg\n", fValue );
#endif


    return TRUE;
}
