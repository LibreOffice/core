/*************************************************************************
 *
 *  $RCSfile: padialog.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 14:16:06 $
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
#include <unistd.h>
#include <string.h>
#include <math.h>

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <vcl/gradient.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <vcl/lineinfo.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _PAD_PADIALOG_HRC_
#include <padialog.hrc>
#endif
#ifndef _PAD_FONTENTRY_HXX_
#include <fontentry.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif
#ifndef _PSP_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif
#ifndef _PAD_PADIALOG_HXX_
#include <padialog.hxx>
#endif
#ifndef _PAD_ADDDLG_HXX_
#include <adddlg.hxx>
#endif
#ifndef _PAD_PRTSETUP_HXX_
#include <prtsetup.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

using namespace psp;
using namespace rtl;
using namespace padmin;
using namespace osl;

PADialog* PADialog::Create( Window* pParent, BOOL bAdmin )
{
    return new PADialog( pParent, bAdmin );
}

PADialog::PADialog( Window* pParent, BOOL bAdmin ) :
        ModalDialog( pParent, PaResId( RID_PADIALOG ) ),
        m_aAddPB( this, PaResId( RID_PA_BTN_ADD ) ),
        m_aCancelButton( this, PaResId( RID_PA_BTN_CANCEL ) ),
        m_aDevicesLB( this, PaResId( RID_PA_LB_DEV ) ),
        m_aStdPB( this, PaResId( RID_PA_BTN_STD ) ),
        m_aRemPB( this, PaResId( RID_PA_BTN_DEL ) ),
        m_aPrintersFL( this, PaResId( RID_PA_FL_PRINTERS ) ),
        m_aFontsPB( this, PaResId( RID_PA_BTN_FONTS ) ),
        m_aConfPB( this, PaResId( RID_PA_BTN_CONF ) ),
        m_aRenamePB( this, PaResId( RID_PA_BTN_RENAME ) ),
        m_aTestPagePB( this, PaResId( RID_PA_TESTPAGE ) ),
        m_aDriverTxt( this, PaResId( RID_PA_TXT_DRIVER ) ),
        m_aSepButtonFL( this, PaResId( RID_PA_FL_SEPBUTTON ) ),
        m_aDriver( this, PaResId( RID_PA_TXT_DRIVER_STRING ) ),
        m_aLocationTxt( this, PaResId( RID_PA_TXT_LOCATION ) ),
        m_aLocation( this, PaResId( RID_PA_TXT_LOCATION_STRING ) ),
        m_aCommandTxt( this, PaResId( RID_PA_TXT_COMMAND ) ),
        m_aCommand( this, PaResId( RID_PA_TXT_COMMAND_STRING ) ),
        m_aCommentTxt( this, PaResId( RID_PA_TXT_COMMENT ) ),
        m_aComment( this, PaResId( RID_PA_TXT_COMMENT_STRING ) ),
        m_aDefPrt( PaResId( RID_PA_STR_DEFPRT ) ),
        m_aRenameStr( PaResId( RID_PA_STR_RENAME ) ),
        m_aPrinterImg( Bitmap( PaResId( RID_BMP_SMALL_PRINTER ) ), Color( 0xff, 0x00, 0xff ) ),
        m_aFaxImg( Bitmap( PaResId( RID_BMP_SMALL_FAX ) ), Color( 0xff, 0x00, 0xff ) ),
        m_aPdfImg( Bitmap( PaResId( RID_BMP_SMALL_PDF ) ), Color( 0xff, 0x00, 0xff ) ),
        m_pPrinter( 0 ),
        m_rPIManager( PrinterInfoManager::get() )
{
    Init();
    FreeResource();
}

void PADialog::Init()
{
    UpdateDevice();
    UpdateText();

    m_aRemPB.Enable( FALSE );

    m_aDevicesLB.SetDoubleClickHdl( LINK( this, PADialog, DoubleClickHdl ) );
    m_aDevicesLB.SetSelectHdl( LINK( this, PADialog, SelectHdl ) );
    m_aStdPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aRemPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aConfPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aRenamePB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aTestPagePB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aFontsPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aAddPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aDevicesLB.setDelPressedLink( LINK( this, PADialog, DelPressedHdl ) );

    ::psp::PrintFontManager& rFontManager( ::psp::PrintFontManager::get() );
    if( ! rFontManager.checkImportPossible() )
        m_aFontsPB.Enable( FALSE );
}

PADialog::~PADialog()
{
    m_rPIManager.writePrinterConfig();
    freePadminRC();
}

String PADialog::getSelectedDevice()
{
    int nPos = m_aDevicesLB.GetSelectEntryPos();
    int nLen = (int)m_aDevicesLB.GetEntryData( nPos );
    return m_aDevicesLB.GetEntry( nPos ).Copy( 0, nLen );
}

IMPL_LINK( PADialog, DelPressedHdl, ListBox*, pBox )
{
    if( pBox == &m_aDevicesLB && m_aRemPB.IsEnabled() )
        ClickBtnHdl( &m_aRemPB );
    return 0;
}

IMPL_LINK( PADialog, ClickBtnHdl, PushButton*, pButton )
{
    if( pButton == &m_aStdPB )
        UpdateDefPrt();
    else if( pButton == &m_aRemPB && AreYouSure( this, RID_QUERY_REMOVEPRINTER ) )
        RemDevice();
    else if( pButton == &m_aConfPB )
        ConfigureDevice();
    else if( pButton == &m_aRenamePB )
        RenameDevice();
    else if( pButton == &m_aTestPagePB )
        PrintTestPage();
    else if( pButton == &m_aAddPB )
        AddDevice();
    else if( pButton == &m_aFontsPB )
    {
        FontNameDlg aDialog( this );
        aDialog.Execute();
    }

    return 0;
}

IMPL_LINK( PADialog, DoubleClickHdl, ListBox*, pListBox )
{
    if( pListBox == &m_aDevicesLB )
        UpdateDefPrt();
    return 0;
}

IMPL_LINK( PADialog, SelectHdl, ListBox*, pListBox )
{
    if( pListBox == &m_aDevicesLB )
    {
        String sSelect = getSelectedDevice();
        String sDefPrt = m_rPIManager.getDefaultPrinter();
        if( sDefPrt == sSelect || ! m_rPIManager.removePrinter( sSelect, true ) )
            m_aRemPB.Enable( FALSE );
        else
            m_aRemPB.Enable( TRUE );
        UpdateText();
    }
    return 0;
}

IMPL_LINK( PADialog, EndPrintHdl, void*, pNil )
{
    String aInfoString( PaResId( RID_PA_TXT_TESTPAGE_PRINTED ) );
    InfoBox aInfoBox( this, aInfoString );
    aInfoBox.SetText( String( PaResId( RID_BXT_TESTPAGE ) ) );
    aInfoBox.Execute();

    delete m_pPrinter;
    m_pPrinter = NULL;
    return 0;
}

void PADialog::UpdateDefPrt()
{
    m_rPIManager.setDefaultPrinter( getSelectedDevice() );

    UpdateDevice();
    UpdateText();

    if( m_aRemPB.HasFocus() )
        m_aDevicesLB.GetFocus();
    m_aRemPB.Enable( FALSE );
}

void PADialog::UpdateText()
{
    const PrinterInfo& rInfo = m_rPIManager.getPrinterInfo( getSelectedDevice() );
    String aDriver( rInfo.m_aPrinterName );
    aDriver.AppendAscii( " (" );
    aDriver += String( rInfo.m_aDriverName );
    aDriver.Append( ')' );
    m_aDriver.SetText( aDriver );
    m_aCommand.SetText( rInfo.m_aCommand );
    m_aComment.SetText( rInfo.m_aComment );
    m_aLocation.SetText( rInfo.m_aLocation );
}

static Point project( const Point& rPoint )
{
    const double angle_x = M_PI / 6.0;
    const double angle_z = M_PI / 6.0;

    // transform planar coordinates to 3d
    double x = rPoint.X();
    double y = rPoint.Y();
    double z = 0;

    // rotate around X axis
    double x1 = x;
    double y1 = y * cos( angle_x );
    double z1 = y * sin( angle_x );

    // rotate around Z axis
    double x2 = x1 * cos( angle_z ) + y1 * sin( angle_z );
    double y2 = y1 * cos( angle_z ) - x1 * sin( angle_z );
    double z2 = z1;

    return Point( x2, z2 );
}

static Color approachColor( const Color& rFrom, const Color& rTo )
{
    Color aColor;
    UINT8 nDiff;
    // approach red
    if( rFrom.GetRed() < rTo.GetRed() )
    {
        nDiff = rTo.GetRed() - rFrom.GetRed();
        aColor.SetRed( rFrom.GetRed() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetRed() > rTo.GetRed() )
    {
        nDiff = rFrom.GetRed() - rTo.GetRed();
        aColor.SetRed( rFrom.GetRed() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetRed( rFrom.GetRed() );

    // approach Green
    if( rFrom.GetGreen() < rTo.GetGreen() )
    {
        nDiff = rTo.GetGreen() - rFrom.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetGreen() > rTo.GetGreen() )
    {
        nDiff = rFrom.GetGreen() - rTo.GetGreen();
        aColor.SetGreen( rFrom.GetGreen() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetGreen( rFrom.GetGreen() );

    // approach blue
    if( rFrom.GetBlue() < rTo.GetBlue() )
    {
        nDiff = rTo.GetBlue() - rFrom.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() + ( nDiff < 10 ? nDiff : 10 ) );
    }
    else if( rFrom.GetBlue() > rTo.GetBlue() )
    {
        nDiff = rFrom.GetBlue() - rTo.GetBlue();
        aColor.SetBlue( rFrom.GetBlue() - ( nDiff < 10 ? nDiff : 10 ) );
    }
    else
        aColor.SetBlue( rFrom.GetBlue() );

    return aColor;
}

#define DELTA 5.0
void PADialog::PrintTestPage()
{
    if( m_pPrinter ) // already printing; user pressed button twice
        return;

    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    String sPrinter( getSelectedDevice() );

    m_pPrinter = new Printer( sPrinter );

    PrinterInfo aInfo( m_rPIManager.getPrinterInfo( sPrinter ) );
    const PPDParser* pPrintParser = aInfo.m_pParser;

    MapMode aMapMode( MAP_100TH_MM );

    Bitmap aButterfly( PaResId( RID_BUTTERFLY ) );

    m_pPrinter->SetMapMode( aMapMode );
    m_pPrinter->SetEndPrintHdl( LINK( this, PADialog, EndPrintHdl ) );
    String aJobName( String( RTL_CONSTASCII_USTRINGPARAM( "StarOffice Test" ) ) );
    if( m_pPrinter->GetName() != sPrinter || ! m_pPrinter->StartJob( aJobName ) )
    {
        String aString( PaResId( RID_ERR_NOPRINTER ) );
        aString.SearchAndReplaceAscii( "%s", sPrinter );

        ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK, aString );
        aErrorBox.SetText( String( PaResId( RID_BXT_ENVIRONMENT ) ) );
        aErrorBox.Execute();
        delete m_pPrinter;
        m_pPrinter = 0;
        return;
    }
    m_pPrinter->StartPage();

    Size aPaperSize=m_pPrinter->GetOutputSize();
    Point aCenter( aPaperSize.Width()/2-300,
                   aPaperSize.Height() - aPaperSize.Width()/2 );
    Point aP1( aPaperSize.Width()/48, 0), aP2( aPaperSize.Width()/40, 0 ), aPoint;

    m_pPrinter->DrawRect( Rectangle( Point( 0,0 ), aPaperSize ) );
    m_pPrinter->DrawRect( Rectangle( Point( 100,100 ),
                                    Size( aPaperSize.Width()-200,
                                          aPaperSize.Height()-200 ) ) );
    m_pPrinter->DrawRect( Rectangle( Point( 200,200 ),
                                    Size( aPaperSize.Width()-400,
                                          aPaperSize.Height()-400 ) ) );
    m_pPrinter->DrawRect( Rectangle( Point( 300,300 ),
                                    Size( aPaperSize.Width()-600,
                                          aPaperSize.Height()-600 ) ) );

    Font aFont( m_pPrinter->GetFont() );
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Courier" ) ) );
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetItalic( ITALIC_NONE );
    m_pPrinter->SetFont( aFont );

    String aPrintText;
    long nWidth = 0, nMaxWidth = 0;
    String aToken;

    aToken = String( PaResId( RID_TXT_TESTPAGE_NAME ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;
    aPrintText += '\n';
    aToken = String( PaResId( RID_TXT_TESTPAGE_MODEL ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;
    aPrintText += '\n';
    aPrintText.AppendAscii( "PPD" );
    aPrintText += '\n';
    aToken = String( PaResId( RID_TXT_TESTPAGE_QUEUE ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;
    aPrintText += '\n';
    aToken = String( PaResId( RID_TXT_TESTPAGE_COMMENT ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;
    aPrintText += '\n';
    aToken = String( PaResId( RID_TXT_TESTPAGE_DATE ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;
    aPrintText += '\n';
    aToken = String( PaResId( RID_TXT_TESTPAGE_TIME ) );
    nMaxWidth = ( nWidth = m_pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
    aPrintText += aToken;

    m_pPrinter->DrawText( Rectangle( Point( 1000, 2000 ),
                                    Size( aPaperSize.Width() - 2000,
                                          aPaperSize.Height() - 4000 ) ),
                         aPrintText,
                         TEXT_DRAW_MULTILINE );

    AllSettings aSettings( Application::GetSettings() );
    const LocaleDataWrapper& rLocaleWrapper( aSettings.GetLocaleDataWrapper() );

    aPrintText = String( RTL_CONSTASCII_USTRINGPARAM( ": " ) );
    aPrintText += sPrinter;
    aPrintText.AppendAscii( "\n: " );
    aPrintText += pPrintParser->getPrinterName();
    aPrintText.AppendAscii( "\n: " );
    INetURLObject aDriverPath( pPrintParser->getFilename(), INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    aPrintText += aDriverPath.GetName();
    aPrintText.AppendAscii( "\n: " );
    aPrintText += String( aInfo.m_aCommand );
    aPrintText.AppendAscii( "\n: " );
    aPrintText += String( aInfo.m_aComment );
    aPrintText.AppendAscii( "\n: " );
    aPrintText += rLocaleWrapper.getDate( Date() );
    aPrintText.AppendAscii( "\n: " );
    aPrintText += rLocaleWrapper.getTime( Time() );

    m_pPrinter->DrawText( Rectangle( Point( 1100 + nMaxWidth, 2000 ),
                                    Size( aPaperSize.Width() - 2100 - nMaxWidth,
                                          aPaperSize.Height() - 4000 ) ),
                         aPrintText,
                         TEXT_DRAW_MULTILINE );

    m_pPrinter->DrawBitmap( Point( aPaperSize.Width() - 4000, 1000 ),
                           Size( 3000,3000 ),
                           aButterfly );
    m_pPrinter->SetFillColor();
    m_pPrinter->DrawRect( Rectangle( Point( aPaperSize.Width() - 4000, 1000 ),
                                     Size( 3000,3000 ) ) );

    Color aWhite( 0xff, 0xff, 0xff );
    Color aBlack( 0, 0, 0 );
    Color aLightRed( 0xff, 0, 0 );
    Color aDarkRed( 0x40, 0, 0 );
    Color aLightBlue( 0, 0, 0xff );
    Color aDarkBlue( 0,0,0x40 );
    Color aLightGreen( 0, 0xff, 0 );
    Color aDarkGreen( 0, 0x40, 0 );

    Gradient aGradient( GRADIENT_LINEAR, aBlack, aWhite );
    aGradient.SetAngle( 900 );
    m_pPrinter->DrawGradient( Rectangle( Point( 1000, 5500 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkRed );
    aGradient.SetEndColor( aLightBlue );
    m_pPrinter->DrawGradient( Rectangle( Point( 1000, 6300 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkBlue );
    aGradient.SetEndColor( aLightGreen );
    m_pPrinter->DrawGradient( Rectangle( Point( 1000, 7100 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkGreen );
    aGradient.SetEndColor( aLightRed );
    m_pPrinter->DrawGradient( Rectangle( Point( 1000, 7900 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );



    LineInfo aLineInfo( LINE_SOLID, 200 );
    double sind = sin( DELTA*M_PI/180.0 );
    double cosd = cos( DELTA*M_PI/180.0 );
    double factor = 1 + (DELTA/1000.0);
    int n=0;
    Color aLineColor( 0, 0, 0 );
    Color aApproachColor( 0, 0, 200 );
    while ( aP2.X() < aCenter.X() && n++ < 680 )
    {
        aLineInfo.SetWidth( n/3 );
        aLineColor = approachColor( aLineColor, aApproachColor );
        m_pPrinter->SetLineColor( aLineColor );

        // switch aproach color
        if( aApproachColor.IsRGBEqual( aLineColor ) )
        {
            if( aApproachColor.GetRed() )
                aApproachColor = Color( 0, 0, 200 );
            else if( aApproachColor.GetGreen() )
                aApproachColor = Color( 200, 0, 0 );
            else
                aApproachColor = Color( 0, 200, 0 );
        }

        m_pPrinter->DrawLine( project( aP1 ) + aCenter,
                             project( aP2 ) + aCenter,
                             aLineInfo );
        aPoint.X() = (((double)aP1.X())*cosd - ((double)aP1.Y())*sind)*factor;
        aPoint.Y() = (((double)aP1.Y())*cosd + ((double)aP1.X())*sind)*factor;
        aP1 = aPoint;
        aPoint.X() = (((double)aP2.X())*cosd - ((double)aP2.Y())*sind)*factor;
        aPoint.Y() = (((double)aP2.Y())*cosd + ((double)aP2.X())*sind)*factor;
        aP2 = aPoint;
    }
#if defined DEBUG || defined DBG_UTIL
    fprintf( stderr, "%d lines\n",n );
#endif
    m_pPrinter->EndPage();
    m_pPrinter->EndJob();
}

void PADialog::AddDevice()
{
    AddPrinterDialog aDlg( this );

    if( aDlg.Execute() )
        UpdateDevice();
}

void PADialog::RemDevice()
{
    String aPrinter( getSelectedDevice() );
    String aDefPrinter( m_rPIManager.getDefaultPrinter() );
    // do not remove the default printer
    if( aPrinter.Equals( aDefPrinter ) )
        return;

    if( ! m_rPIManager.removePrinter( aPrinter ) )
    {
        String aText( PaResId( RID_ERR_PRINTERNOTREMOVEABLE ) );
        aText.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ), aPrinter );
        ErrorBox aBox( this, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
        return;
    }
    m_aPrinters.remove( aPrinter );

    m_aDevicesLB.RemoveEntry( m_aDevicesLB.GetSelectEntryPos() );
    for( int i=0; i < m_aDevicesLB.GetEntryCount(); i++ )
    {
        if( m_aDevicesLB.GetEntry( i ).CompareTo( aDefPrinter, aDefPrinter.Len() ) == COMPARE_EQUAL )
        {
            m_aDevicesLB.SelectEntryPos( i, TRUE );
            UpdateText();
            break;
        }
    }

    m_aDevicesLB.GetFocus();

    if( m_aDevicesLB.GetEntryCount() < 2 )
        m_aRemPB.Enable( FALSE );
}

void PADialog::ConfigureDevice()
{
    String aPrinter( getSelectedDevice() );

    if( ! aPrinter.Len() )
        return;

    PrinterInfo aInfo( m_rPIManager.getPrinterInfo( aPrinter ) );
    RTSDialog aDialog( aInfo, aPrinter, true, this );

    if( aDialog.Execute() )
        m_rPIManager.changePrinterInfo( aPrinter, aDialog.getSetup() );

    UpdateText();
}

void PADialog::RenameDevice()
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    String aPrinter( getSelectedDevice() );
    OUString aOldPrinter( aPrinter );
    int nSelectEntryPos = m_aDevicesLB.GetSelectEntryPos();

    if( ! aPrinter.Len() )
        return;

    String aTmpString( PaResId( RID_QRY_PRTNAME ) );
    QueryString aQuery( this,
                        aTmpString,
                        aPrinter );
    aQuery.SetText( m_aRenameStr );
    aQuery.Execute();

    if( aPrinter.Len() )
    {
        PrinterInfo aInfo( m_rPIManager.getPrinterInfo( aOldPrinter ) );
        aInfo.m_aPrinterName = aPrinter;
        if( m_rPIManager.addPrinter( aPrinter, aInfo.m_aDriverName ) )
        {
            bool bWasDefault = m_rPIManager.getDefaultPrinter() == aOldPrinter;
            m_aPrinters.push_back( aPrinter );
            if( m_rPIManager.removePrinter( aOldPrinter ) )
                m_aPrinters.remove( aOldPrinter );
            m_rPIManager.changePrinterInfo( aPrinter, aInfo );
            if( bWasDefault )
            {
                m_rPIManager.setDefaultPrinter( aPrinter );
                UpdateDefPrt();
            }
            UpdateDevice();
        }
    }
}

void PADialog::UpdateDevice()
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    m_aDevicesLB.Clear();

    m_rPIManager.listPrinters( m_aPrinters );
    ::std::list< OUString >::iterator it;
    for( it = m_aPrinters.begin(); it != m_aPrinters.end(); ++it )
    {
        const PrinterInfo& rInfo( m_rPIManager.getPrinterInfo( *it ) );
        sal_Int32 nIndex = 0;
        bool bAutoQueue = false;
        bool bFax = false;
        bool bPdf = false;
        while( nIndex != -1 && ! bAutoQueue )
        {
            OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
            if( aToken.getLength() )
            {
                if( aToken.compareToAscii( "autoqueue" ) == 0 )
                    bAutoQueue = true;
                else if( aToken.compareToAscii( "pdf=", 4 ) == 0 )
                    bPdf = true;
                else if( aToken.compareToAscii( "fax", 3 ) == 0 )
                    bFax = true;
            }
        }
        if( bAutoQueue )
            continue;

        String aEntry( *it );
        if( *it == m_rPIManager.getDefaultPrinter() )
        {
            aEntry.AppendAscii( " (" );
            aEntry += m_aDefPrt;
            aEntry.AppendAscii( ")" );
        }
        int nPos =
            m_aDevicesLB.InsertEntry( aEntry,
                                      bFax ? m_aFaxImg :
                                      bPdf ? m_aPdfImg : m_aPrinterImg
                                      );
        m_aDevicesLB.SetEntryData( nPos, (void*)it->getLength() );
        if( *it == m_rPIManager.getDefaultPrinter() )
        {
            m_aDevicesLB.SelectEntryPos( nPos );
            UpdateText();
        }
    }
}

