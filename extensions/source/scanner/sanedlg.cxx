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


#include <stdio.h>
#include <stdlib.h>
#include <tools/config.hxx>

#include <vcl/msgbox.hxx>
#include <sanedlg.hxx>
#include <sanedlg.hrc>
#include <grid.hxx>
#include <math.h>
#include <sal/macros.h>
#include <rtl/strbuf.hxx>

ResId SaneResId( sal_uInt32 nID )
{
    static ResMgr* pResMgr = ResMgr::CreateResMgr( "scn" );
    return ResId( nID, *pResMgr );
}

SaneDlg::SaneDlg( Window* pParent, Sane& rSane, bool bScanEnabled ) :
        ModalDialog( pParent, SaneResId( RID_SANE_DIALOG ) ),
        mrSane( rSane ),
        mbIsDragging( sal_False ),
        mbScanEnabled( bScanEnabled ),
        mbDragDrawn( sal_False ),
        maMapMode( MAP_APPFONT ),
        maOKButton( this, SaneResId( RID_SCAN_OK ) ),
        maCancelButton( this, SaneResId( RID_SCAN_CANCEL ) ),
        maDeviceInfoButton( this, SaneResId( RID_DEVICEINFO_BTN ) ),
        maPreviewButton( this, SaneResId( RID_PREVIEW_BTN ) ),
        maScanButton( this, SaneResId( RID_SCAN_BTN ) ),
        maButtonOption( this, SaneResId( RID_SCAN_BUTTON_OPTION_BTN ) ),
        maOptionsTxt( this, SaneResId( RID_SCAN_OPTION_TXT ) ),
        maOptionTitle( this, SaneResId( RID_SCAN_OPTIONTITLE_TXT ) ),
        maOptionDescTxt( this, SaneResId( RID_SCAN_OPTION_DESC_TXT ) ),
        maVectorTxt( this, SaneResId( RID_SCAN_NUMERIC_VECTOR_TXT ) ),
        maScanLeftTxt( this, SaneResId( RID_SCAN_LEFT_TXT ) ),
        maLeftField( this, SaneResId( RID_SCAN_LEFT_BOX ) ),
        maScanTopTxt( this, SaneResId( RID_SCAN_TOP_TXT ) ),
        maTopField( this, SaneResId( RID_SCAN_TOP_BOX ) ),
        maRightTxt( this, SaneResId( RID_SCAN_RIGHT_TXT ) ),
        maRightField( this, SaneResId( RID_SCAN_RIGHT_BOX ) ),
        maBottomTxt( this, SaneResId( RID_SCAN_BOTTOM_TXT ) ),
        maBottomField( this, SaneResId( RID_SCAN_BOTTOM_BOX ) ),
        maDeviceBoxTxt( this, SaneResId( RID_DEVICE_BOX_TXT ) ),
        maDeviceBox( this, SaneResId( RID_DEVICE_BOX ) ),
        maReslTxt( this, SaneResId( RID_SCAN_RESOLUTION_TXT ) ),
        maReslBox( this, SaneResId( RID_SCAN_RESOLUTION_BOX ) ),
        maAdvancedTxt( this, SaneResId( RID_SCAN_ADVANCED_TXT ) ),
        maAdvancedBox( this, SaneResId( RID_SCAN_ADVANCED_BOX ) ),
        maVectorBox( this, SaneResId( RID_SCAN_NUMERIC_VECTOR_BOX ) ),
        maQuantumRangeBox( this, SaneResId( RID_SCAN_QUANTUM_RANGE_BOX ) ),
        maStringRangeBox( this, SaneResId( RID_SCAN_STRING_RANGE_BOX ) ),
        maPreviewBox( this, SaneResId( RID_PREVIEW_BOX ) ),
        maAreaBox( this, SaneResId( RID_SCANAREA_BOX ) ),
        maBoolCheckBox( this, SaneResId( RID_SCAN_BOOL_OPTION_BOX ) ),
        maStringEdit( this, SaneResId( RID_SCAN_STRING_OPTION_EDT ) ),
        maNumericEdit( this, SaneResId( RID_SCAN_NUMERIC_OPTION_EDT ) ),
        maOptionBox( this, SaneResId( RID_SCAN_OPTION_BOX ) ),
        mpRange( 0 ),
        doScan( false )
{
    if( Sane::IsSane() )
    {
        InitDevices(); // opens first sane device
        DisableOption();
        InitFields();
    }

    maDeviceInfoButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maPreviewButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
    maScanButton.SetClickHdl( LINK( this, SaneDlg, ClickBtnHdl ) );
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
    maOptionBox.SetStyle( maOptionBox.GetStyle()|
                          WB_HASLINES           |
                          WB_HASBUTTONS         |
                          WB_NOINITIALSELECTION |
                          WB_HASBUTTONSATROOT   |
                          WB_HASLINESATROOT
                        );
    FreeResource();
}

SaneDlg::~SaneDlg()
{
    mrSane.SetReloadOptionsHdl( maOldLink );
}

short SaneDlg::Execute()
{
    if( ! Sane::IsSane() )
    {
        ErrorBox aErrorBox( NULL, WB_OK | WB_DEF_OK,
                            String( SaneResId( RID_SANE_NOSANELIB_TXT ) ) );
        aErrorBox.Execute();
        return sal_False;
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

    int nOption, i, nValue;
    double fValue;
    sal_Bool bSuccess = sal_False;
    const char *ppSpecialOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y",
        "preview"
    };

    mbDragEnable = sal_True;
    maReslBox.Clear();
    maMinTopLeft = Point( 0, 0 );
    maMaxBottomRight = Point( PREVIEW_WIDTH,  PREVIEW_HEIGHT );
    maScanButton.Show( mbScanEnabled );

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
            maReslBox.Enable( sal_True );

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
                    // Can only select 75 and 2400 dpi in Scanner dialogue
                    // scanner allows random setting of dpi resolution, a slider might be useful
                    // support that
                    // workaround: offer at least some more standard dpi resolution between
                    // min and max value
                    int bGot300 = 0;
                    for ( int nRes = (long) pDouble[0] * 2; nRes < (long) pDouble[1]; nRes = nRes * 2 )
                    {
                        if ( !bGot300 && nRes > 300 ) {
                            nRes = 300; bGot300 = 1;
                        }
                        maReslBox.InsertValue(nRes);
                    }
                    maReslBox.InsertValue( (long)pDouble[1] );
                }
                if( pDouble )
                    delete [] pDouble;
            }
            else
                maReslBox.Enable( sal_False );
        }
    }
    else
        maReslBox.Enable( sal_False );

    // set scan area
    for( i = 0; i < 4; i++ )
    {
        char const *pOptionName = NULL;
        MetricField* pField = NULL;
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
        nOption = pOptionName ? mrSane.GetOptionByName( pOptionName ) : -1;
        bSuccess = sal_False;
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
                    pField->SetCustomUnitText(rtl::OUString("Pixel"));
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
                    delete [] pDouble;
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
            pField->Enable( sal_True );
        }
        else
        {
            mbDragEnable = sal_False;
            pField->SetMin( 0 );
            switch( i ) {
                case 0:
                    maMinTopLeft.X() = 0;
                    maTopLeft.X() = 0;
                    pField->SetMax( PREVIEW_WIDTH );
                    pField->SetValue( 0 );
                    break;
                case 1:
                    maMinTopLeft.Y() = 0;
                    maTopLeft.Y() = 0;
                    pField->SetMax( PREVIEW_HEIGHT );
                    pField->SetValue( 0 );
                    break;
                case 2:
                    maMaxBottomRight.X() = PREVIEW_WIDTH;
                    maBottomRight.X() = PREVIEW_WIDTH;
                    pField->SetMax( PREVIEW_WIDTH );
                    pField->SetValue( PREVIEW_WIDTH );
                    break;
                case 3:
                    maMaxBottomRight.Y() = PREVIEW_HEIGHT;
                    maBottomRight.Y() = PREVIEW_HEIGHT;
                    pField->SetMax( PREVIEW_HEIGHT );
                    pField->SetValue( PREVIEW_HEIGHT );
                    break;
            }
            pField->Enable( sal_False );
        }
    }
    maTopLeft = GetPixelPos( maTopLeft );
    maBottomRight = GetPixelPos( maBottomRight );
    maPreviewRect = Rectangle( maTopLeft,
                               Size( maBottomRight.X() - maTopLeft.X(),
                                     maBottomRight.Y() - maTopLeft.Y() )
                               );
    // fill OptionBox
    maOptionBox.Clear();
    SvTreeListEntry* pParentEntry = 0;
    sal_Bool bGroupRejected = sal_False;
    for( i = 1; i < mrSane.CountOptions(); i++ )
    {
        String aOption=mrSane.GetOptionName( i );
        sal_Bool bInsertAdvanced =
            mrSane.GetOptionCap( i ) & SANE_CAP_ADVANCED &&
            ! maAdvancedBox.IsChecked() ? sal_False : sal_True;
        if( mrSane.GetOptionType( i ) == SANE_TYPE_GROUP )
        {
            if( bInsertAdvanced )
            {
                aOption = mrSane.GetOptionTitle( i );
                pParentEntry = maOptionBox.InsertEntry( aOption );
                bGroupRejected = sal_False;
            }
            else
                bGroupRejected = sal_True;
        }
        else if( aOption.Len() &&
                 ! ( mrSane.GetOptionCap( i ) &
                     (
                         SANE_CAP_HARD_SELECT |
                         SANE_CAP_INACTIVE
                         ) ) &&
                 bInsertAdvanced && ! bGroupRejected )
        {
            sal_Bool bIsSpecial = sal_False;
            for( size_t n = 0; !bIsSpecial &&
                     n < SAL_N_ELEMENTS(ppSpecialOptions); n++ )
            {
                if( aOption.EqualsAscii( ppSpecialOptions[n] ) )
                    bIsSpecial=sal_True;
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
                                   (sal_Bool)sal_True : (sal_Bool)sal_False );
        }
        else if( pButton == &maButtonOption )
        {

            SANE_Value_Type nType = mrSane.GetOptionType( mnCurrentOption );
            switch( nType )
            {
                case SANE_TYPE_BUTTON:
                    mrSane.ActivateButtonOption( mnCurrentOption );
                    break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    int nElements = mrSane.GetOptionElements( mnCurrentOption );
                    double* x = new double[ nElements ];
                    double* y = new double[ nElements ];
                    for( int i = 0; i < nElements; i++ )
                        x[ i ] = (double)i;
                    mrSane.GetOptionValue( mnCurrentOption, y );

                    GridWindow aGrid( x, y, nElements, this );
                    aGrid.SetText( mrSane.GetOptionName( mnCurrentOption ) );
                    aGrid.setBoundings( 0, mfMin, nElements, mfMax );
                    if( aGrid.Execute() && aGrid.getNewYValues() )
                        mrSane.SetOptionValue( mnCurrentOption, aGrid.getNewYValues() );

                    delete [] x;
                    delete [] y;
                }
                break;
                case SANE_TYPE_BOOL:
                case SANE_TYPE_STRING:
                case SANE_TYPE_GROUP:
                    break;
            }
        }
        else if( pButton == &maAdvancedBox )
        {
            ReloadSaneOptionsHdl( NULL );
        }
    }
    if( pButton == &maOKButton || pButton == &maScanButton )
    {
        double fRes = (double)maReslBox.GetValue();
        SetAdjustedNumericalValue( "resolution", fRes );
        UpdateScanArea( sal_True );
        SaveState();
        EndDialog( mrSane.IsOpen() ? 1 : 0 );
        doScan = (pButton == &maScanButton);
    }
    else if( pButton == &maCancelButton )
    {
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
            rtl::OString aValue(rtl::OUStringToOString(maQuantumRangeBox.GetSelectEntry(),
                osl_getThreadTextEncoding()));
            double fValue = atof(aValue.getStr());
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
        int nOption = mrSane.GetOptionByName(rtl::OUStringToOString(aOption,
            osl_getThreadTextEncoding()).getStr());
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
                    int nElements = mrSane.GetOptionElements( mnCurrentOption );
                    mnCurrentElement = 0;
                    if( nConstraint == SANE_CONSTRAINT_RANGE ||
                        nConstraint == SANE_CONSTRAINT_WORD_LIST )
                        EstablishQuantumRange();
                    else
                    {
                        mfMin = mfMax = 0.0;
                        EstablishNumericOption();
                    }
                    if( nElements > 1 )
                    {
                        if( nElements <= 10 )
                        {
                            maVectorBox.SetValue( 1 );
                            maVectorBox.SetMin( 1 );
                            maVectorBox.SetMax(
                                mrSane.GetOptionElements( mnCurrentOption ) );
                            maVectorBox.Show( sal_True );
                            maVectorTxt.Show( sal_True );
                        }
                        else
                        {
                            DisableOption();
                            // bring up dialog only on button click
                            EstablishButtonOption();
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
                maReslBox.SetValue( (sal_uLong)fRes );
            }
        }
        else if( pEdit == &maNumericEdit )
        {
            double fValue;
            char pBuf[256];
            rtl::OString aContents(rtl::OUStringToOString(maNumericEdit.GetText(),
                osl_getThreadTextEncoding()));
            fValue = atof(aContents.getStr());
            if( mfMin != mfMax && ( fValue < mfMin || fValue > mfMax ) )
            {
                if( fValue < mfMin )
                    fValue = mfMin;
                else if( fValue > mfMax )
                fValue = mfMax;
                sprintf( pBuf, "%g", fValue );
                maNumericEdit.SetText( String( pBuf, osl_getThreadTextEncoding() ) );
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
            String aValue( pBuf, osl_getThreadTextEncoding() );
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

IMPL_LINK( SaneDlg, ReloadSaneOptionsHdl, Sane*, /*pSane*/ )
{
    mnCurrentOption = -1;
    mnCurrentElement = 0;
    DisableOption();
    // #92024# preserve preview rect, should only be set
    // initially or in AcquirePreview
    Rectangle aPreviewRect = maPreviewRect;
    InitFields();
    maPreviewRect = aPreviewRect;
    Rectangle aDummyRect( Point( 0, 0 ), GetSizePixel() );
    Paint( aDummyRect );
    return 0;
}

void SaneDlg::AcquirePreview()
{
    if( ! mrSane.IsOpen() )
        return;

    UpdateScanArea( sal_True );
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
        mrSane.SetOptionValue( nOption, (sal_Bool)sal_True );

    BitmapTransporter aTransporter;
    if( ! mrSane.Start( aTransporter ) )
    {
        ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK,
                            String( SaneResId( RID_SANE_SCANERROR_TXT ) ) );
        aErrorBox.Execute();
    }
    else
    {
#if OSL_DEBUG_LEVEL > 1
        aTransporter.getStream().Seek( STREAM_SEEK_TO_END );
        fprintf( stderr, "Previewbitmapstream contains %d bytes\n", (int)aTransporter.getStream().Tell() );
#endif
        aTransporter.getStream().Seek( STREAM_SEEK_TO_BEGIN );
        maPreviewBitmap.Read( aTransporter.getStream(), sal_True );
    }

    SetAdjustedNumericalValue( "resolution", fResl );
    maReslBox.SetValue( (sal_uLong)fResl );

    if( mbDragEnable )
    {
        maPreviewRect = Rectangle( maTopLeft,
                                   Size( maBottomRight.X() - maTopLeft.X(),
                                         maBottomRight.Y() - maTopLeft.Y() )
                                   );
    }
    else
    {
        Size aBMSize( maPreviewBitmap.GetSizePixel() );
        if( aBMSize.Width() > aBMSize.Height() && aBMSize.Width() )
        {
            int nVHeight = (maBottomRight.X() - maTopLeft.X()) * aBMSize.Height() / aBMSize.Width();
            maPreviewRect = Rectangle( Point( maTopLeft.X(), ( maTopLeft.Y() + maBottomRight.Y() )/2 - nVHeight/2 ),
                                       Size( maBottomRight.X() - maTopLeft.X(),
                                             nVHeight ) );
        }
        else if (aBMSize.Height())
        {
            int nVWidth = (maBottomRight.Y() - maTopLeft.Y()) * aBMSize.Width() / aBMSize.Height();
            maPreviewRect = Rectangle( Point( ( maTopLeft.X() + maBottomRight.X() )/2 - nVWidth/2, maTopLeft.Y() ),
                                       Size( nVWidth,
                                             maBottomRight.Y() - maTopLeft.Y() ) );
        }
    }

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

    mbDragDrawn = sal_False;
    DrawDrag();

    ModalDialog::Paint( rRect );
}

void SaneDlg::DisableOption()
{
    maBoolCheckBox.Show( sal_False );
    maStringEdit.Show( sal_False );
    maNumericEdit.Show( sal_False );
    maQuantumRangeBox.Show( sal_False );
    maStringRangeBox.Show( sal_False );
    maButtonOption.Show( sal_False );
    maVectorBox.Show( sal_False );
    maVectorTxt.Show( sal_False );
    maOptionDescTxt.Show( sal_False );
}

void SaneDlg::EstablishBoolOption()
{
    sal_Bool bSuccess, bValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, bValue );
    if( bSuccess )
    {
        maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
        maOptionDescTxt.Show( sal_True );
        maBoolCheckBox.Check( bValue );
        maBoolCheckBox.Show( sal_True );
    }
}

void SaneDlg::EstablishStringOption()
{
    sal_Bool bSuccess;
    rtl::OString aValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, aValue );
    if( bSuccess )
    {
        maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
        maOptionDescTxt.Show( sal_True );
        maStringEdit.SetText(rtl::OStringToOUString(aValue, osl_getThreadTextEncoding()));
        maStringEdit.Show( sal_True );
    }
}

void SaneDlg::EstablishStringRange()
{
    const char** ppStrings = mrSane.GetStringConstraint( mnCurrentOption );
    maStringRangeBox.Clear();
    for( int i = 0; ppStrings[i] != 0; i++ )
        maStringRangeBox.InsertEntry( String( ppStrings[i], osl_getThreadTextEncoding() ) );
    rtl::OString aValue;
    mrSane.GetOptionValue( mnCurrentOption, aValue );
    maStringRangeBox.SelectEntry(rtl::OStringToOUString(aValue, osl_getThreadTextEncoding()));
    maStringRangeBox.Show( sal_True );
    maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
    maOptionDescTxt.Show( sal_True );
}

void SaneDlg::EstablishQuantumRange()
{
    if( mpRange )
    {
        delete [] mpRange;
        mpRange = 0;
    }
    int nValues = mrSane.GetRange( mnCurrentOption, mpRange );
    if( nValues == 0 )
    {
        mfMin = mpRange[ 0 ];
        mfMax = mpRange[ 1 ];
        delete [] mpRange;
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
            maQuantumRangeBox.InsertEntry( String( pBuf, osl_getThreadTextEncoding() ) );
        }
        double fValue;
        if( mrSane.GetOptionValue( mnCurrentOption, fValue, mnCurrentElement ) )
        {
            sprintf( pBuf, "%g", fValue );
            maQuantumRangeBox.SelectEntry( String( pBuf, osl_getThreadTextEncoding() ) );
        }
        maQuantumRangeBox.Show( sal_True );
        String aText( mrSane.GetOptionName( mnCurrentOption ) );
        aText += ' ';
        aText += mrSane.GetOptionUnitName( mnCurrentOption );
        maOptionDescTxt.SetText( aText );
        maOptionDescTxt.Show( sal_True );
    }
}

void SaneDlg::EstablishNumericOption()
{
    sal_Bool bSuccess;
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
        aText += String( pBuf, osl_getThreadTextEncoding() );
    }
    maOptionDescTxt.SetText( aText );
    maOptionDescTxt.Show( sal_True );
    sprintf( pBuf, "%g", fValue );
    maNumericEdit.SetText( String( pBuf, osl_getThreadTextEncoding() ) );
    maNumericEdit.Show( sal_True );
}

void SaneDlg::EstablishButtonOption()
{
    maOptionDescTxt.SetText( mrSane.GetOptionName( mnCurrentOption ) );
    maOptionDescTxt.Show( sal_True );
    maButtonOption.Show( sal_True );
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
        UpdateScanArea( sal_False );
    }
    ModalDialog::MouseMove( rMEvt );
}

void SaneDlg::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point aMousePixel = rMEvt.GetPosPixel();

    if( ! mbIsDragging  && mbDragEnable )
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
                mbIsDragging = sal_True;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Top;
                aMousePixel.Y() = maTopLeft.Y();
                mbIsDragging = sal_True;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = TopRight;
                aMousePixel = Point( maBottomRight.X(), maTopLeft.Y() );
                mbIsDragging = sal_True;
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
                mbIsDragging = sal_True;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = Right;
                aMousePixel.X() = maBottomRight.X();
                mbIsDragging = sal_True;
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
                mbIsDragging = sal_True;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Bottom;
                aMousePixel.Y() = maBottomRight.Y();
                mbIsDragging = sal_True;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = BottomRight;
                aMousePixel = maBottomRight;
                mbIsDragging = sal_True;
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
        UpdateScanArea( sal_True );
    }
    mbIsDragging = sal_False;

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

    if( ! mbDragEnable )
        return;

    RasterOp eROP = GetRasterOp();
    SetRasterOp( ROP_INVERT );
    SetMapMode( MapMode( MAP_PIXEL ) );

    if( mbDragDrawn )
        DrawRectangles( aLastUL, aLastBR );

    aLastUL = maTopLeft;
    aLastBR = maBottomRight;
    DrawRectangles( maTopLeft, maBottomRight );

    mbDragDrawn = sal_True;
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

void SaneDlg::UpdateScanArea( sal_Bool bSend )
{
    if( ! mbDragEnable )
        return;

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

sal_Bool SaneDlg::LoadState()
{
    int i;

    if( ! Sane::IsSane() )
        return sal_False;

    const char* pEnv = getenv("HOME");
    String aFileName( pEnv ? pEnv : "", osl_getThreadTextEncoding() );
    aFileName += String( RTL_CONSTASCII_USTRINGPARAM( "/.so_sane_state" ) );
    Config aConfig( aFileName );
    if( ! aConfig.HasGroup( "SANE" ) )
        return sal_False;

    aConfig.SetGroup( "SANE" );
    rtl::OString aString = aConfig.ReadKey( "SO_LastSaneDevice" );
    for( i = 0; i < Sane::CountDevices() && !aString.equals(rtl::OUStringToOString(Sane::GetName(i), osl_getThreadTextEncoding())); i++ ) ;
    if( i == Sane::CountDevices() )
        return sal_False;

    mrSane.Close();
    mrSane.Open( aString.getStr() );

    DisableOption();
    InitFields();

    if( mrSane.IsOpen() )
    {
        int iMax = aConfig.GetKeyCount();
        for (i = 0; i < iMax; ++i)
        {
            aString = aConfig.GetKeyName( i );
            rtl::OString aValue = aConfig.ReadKey( i );
            int nOption = mrSane.GetOptionByName( aString.getStr() );
            if( nOption == -1 )
                continue;

            if (aValue.matchL(RTL_CONSTASCII_STRINGPARAM("BOOL=")))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("BOOL="));
                sal_Bool aBOOL = (sal_Bool)aValue.toInt32();
                mrSane.SetOptionValue( nOption, aBOOL );
            }
            else if (aValue.matchL(RTL_CONSTASCII_STRINGPARAM("STRING=")))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("STRING="));
                mrSane.SetOptionValue(nOption,rtl::OStringToOUString(aValue, osl_getThreadTextEncoding()) );
            }
            else if (aValue.matchL(RTL_CONSTASCII_STRINGPARAM("NUMERIC=")))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("NUMERIC="));

                sal_Int32 nIndex = 0;
                int n = 0;
                do
                {
                    rtl::OString aSub = aValue.getToken(0, ':', nIndex);
                    double fValue=0.0;
                    sscanf(aSub.getStr(), "%lg", &fValue);
                    SetAdjustedNumericalValue(aString.getStr(), fValue, n++);
                }
                while ( nIndex >= 0 );
            }
        }
    }

    DisableOption();
    InitFields();

    return sal_True;
}

void SaneDlg::SaveState()
{
    if( ! Sane::IsSane() )
        return;

    const char* pEnv = getenv( "HOME" );
    String aFileName( pEnv ? pEnv : "", osl_getThreadTextEncoding() );
    aFileName.AppendAscii( "/.so_sane_state" );

    Config aConfig( aFileName );
    aConfig.DeleteGroup( "SANE" );
    aConfig.SetGroup( "SANE" );
    aConfig.WriteKey( "SO_LastSANEDevice",
        rtl::OUStringToOString(maDeviceBox.GetSelectEntry(), RTL_TEXTENCODING_UTF8) );

    static char const* pSaveOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y"
    };
    for( size_t i = 0; i < SAL_N_ELEMENTS(pSaveOptions); ++i )
    {
        rtl::OString aOption = pSaveOptions[i];
        int nOption = mrSane.GetOptionByName( pSaveOptions[i] );
        if( nOption > -1 )
        {
            SANE_Value_Type nType = mrSane.GetOptionType( nOption );
            switch( nType )
            {
                case SANE_TYPE_BOOL:
                {
                    sal_Bool bValue;
                    if( mrSane.GetOptionValue( nOption, bValue ) )
                    {
                        rtl::OStringBuffer aString(RTL_CONSTASCII_STRINGPARAM(
                            "BOOL="));
                        aString.append(static_cast<sal_Int32>(bValue));
                        aConfig.WriteKey(aOption, aString.makeStringAndClear());
                    }
                }
                break;
                case SANE_TYPE_STRING:
                {
                    rtl::OString aValue;
                    if( mrSane.GetOptionValue( nOption, aValue ) )
                    {
                        rtl::OStringBuffer aString(RTL_CONSTASCII_STRINGPARAM("STRING="));
                        aString.append(aValue);
                        aConfig.WriteKey( aOption, aString.makeStringAndClear() );
                    }
                }
                break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    rtl::OStringBuffer aString(RTL_CONSTASCII_STRINGPARAM("NUMERIC="));
                    double fValue;
                    char buf[256];
                    int n;

                    for( n = 0; n < mrSane.GetOptionElements( nOption ); n++ )
                    {
                        if( ! mrSane.GetOptionValue( nOption, fValue, n ) )
                            break;
                        if( n > 0 )
                            aString.append(':');
                        sprintf( buf, "%lg", fValue );
                        aString.append(buf);
                    }
                    if( n >= mrSane.GetOptionElements( nOption ) )
                        aConfig.WriteKey( aOption, aString.makeStringAndClear() );
                }
                break;
                default:
                    break;
            }
        }
    }
}

sal_Bool SaneDlg::SetAdjustedNumericalValue(
    const char* pOption,
    double fValue,
    int nElement )
{
    int nOption;
    if( ! Sane::IsSane() || ! mrSane.IsOpen() || ( nOption = mrSane.GetOptionByName( pOption ) ) == -1 )
        return sal_False;

    if( nElement < 0 || nElement >= mrSane.GetOptionElements( nOption ) )
        return sal_False;

    double* pValues = NULL;
    int nValues;
    if( ( nValues = mrSane.GetRange( nOption, pValues ) ) < 0 )
        return sal_False;

#if OSL_DEBUG_LEVEL > 1
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
    delete [] pValues;
    mrSane.SetOptionValue( nOption, fValue, nElement );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "yields %lg\n", fValue );
#endif


    return sal_True;
}

bool SaneDlg::getDoScan()
{
    return doScan;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
