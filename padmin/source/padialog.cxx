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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "padialog.hrc"
#include "helper.hxx"
#include "padialog.hxx"
#include "adddlg.hxx"
#include "prtsetup.hxx"

#include "vcl/msgbox.hxx"
#include "vcl/print.hxx"
#include "vcl/gradient.hxx"
#include "vcl/bitmap.hxx"
#include "vcl/lineinfo.hxx"
#include "vcl/svapp.hxx"
#include "vcl/event.hxx"
#include "vcl/printerinfomanager.hxx"

#include "tools/stream.hxx"
#include "tools/color.hxx"
#include "tools/urlobj.hxx"

#include "osl/file.hxx"

#include "rtl/ustrbuf.hxx"
#include <sal/macros.h>

#include "unotools/localedatawrapper.hxx"
#include "unotools/configitem.hxx"
#include "unotools/configmgr.hxx"

#include "com/sun/star/awt/Size.hpp"

using namespace psp;
using namespace padmin;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


PADialog* PADialog::Create( Window* pParent, sal_Bool bAdmin )
{
    return new PADialog( pParent, bAdmin );
}

PADialog::PADialog( Window* pParent, sal_Bool /*bAdmin*/ ) :
        ModalDialog( pParent, PaResId( RID_PADIALOG ) ),
        m_aDevicesLB( this, PaResId( RID_PA_LB_DEV ) ),
        m_aConfPB( this, PaResId( RID_PA_BTN_CONF ) ),
        m_aRenamePB( this, PaResId( RID_PA_BTN_RENAME ) ),
        m_aStdPB( this, PaResId( RID_PA_BTN_STD ) ),
        m_aRemPB( this, PaResId( RID_PA_BTN_DEL ) ),
        m_aTestPagePB( this, PaResId( RID_PA_TESTPAGE ) ),
        m_aPrintersFL( this, PaResId( RID_PA_FL_PRINTERS ) ),
        m_aDriverTxt( this, PaResId( RID_PA_TXT_DRIVER ) ),
        m_aDriver( this, PaResId( RID_PA_TXT_DRIVER_STRING ) ),
        m_aLocationTxt( this, PaResId( RID_PA_TXT_LOCATION ) ),
        m_aLocation( this, PaResId( RID_PA_TXT_LOCATION_STRING ) ),
        m_aCommandTxt( this, PaResId( RID_PA_TXT_COMMAND ) ),
        m_aCommand( this, PaResId( RID_PA_TXT_COMMAND_STRING ) ),
        m_aCommentTxt( this, PaResId( RID_PA_TXT_COMMENT ) ),
        m_aComment( this, PaResId( RID_PA_TXT_COMMENT_STRING ) ),
        m_aCUPSFL( this, PaResId( RID_PA_FL_CUPSUSAGE ) ),
        m_aCUPSCB( this, PaResId( RID_PA_CB_CUPSUSAGE ) ),
        m_aSepButtonFL( this, PaResId( RID_PA_FL_SEPBUTTON ) ),
        m_aAddPB( this, PaResId( RID_PA_BTN_ADD ) ),
        m_aCancelButton( this, PaResId( RID_PA_BTN_CANCEL ) ),
        m_aDefPrt( PaResId( RID_PA_STR_DEFPRT ) ),
        m_aRenameStr( PaResId( RID_PA_STR_RENAME ) ),
        m_rPIManager( PrinterInfoManager::get() )
{
    FreeResource();
    updateSettings();
    Init();
}

void PADialog::updateSettings()
{
    m_aPrinterImg = Image( BitmapEx( PaResId( RID_BMP_SMALL_PRINTER ) ) );
    m_aFaxImg     = Image( BitmapEx( PaResId( RID_BMP_SMALL_FAX     ) ) );
    m_aPdfImg     = Image( BitmapEx( PaResId( RID_BMP_SMALL_PDF     ) ) );
}

void PADialog::Init()
{
    // #i79787# initially ensure printer discovery has ended
    m_rPIManager.checkPrintersChanged( true );
    m_aCUPSCB.Check( m_rPIManager.isCUPSDisabled() );

    UpdateDevice();
    UpdateText();

    m_aRemPB.Enable( sal_False );

    m_aDevicesLB.SetDoubleClickHdl( LINK( this, PADialog, DoubleClickHdl ) );
    m_aDevicesLB.SetSelectHdl( LINK( this, PADialog, SelectHdl ) );
    m_aStdPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aRemPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aConfPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aRenamePB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aTestPagePB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aAddPB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );
    m_aDevicesLB.setDelPressedLink( LINK( this, PADialog, DelPressedHdl ) );
    m_aCUPSCB.SetClickHdl( LINK( this, PADialog, ClickBtnHdl ) );

    // at this point no actual changes will be  written
    // but the write will have checked whether any writeable config exists
    if( ! m_rPIManager.writePrinterConfig() )
    {
        m_aAddPB.Enable( sal_False );
        m_aRemPB.Enable( sal_False );
        m_aConfPB.Enable( sal_False );
        m_aRenamePB.Enable( sal_False );
        m_aStdPB.Enable( sal_False );
        m_aCUPSCB.Enable( sal_False );
        ErrorBox aBox( GetParent(), WB_OK | WB_DEF_OK, String( PaResId( RID_ERR_NOWRITE ) ) );
        aBox.Execute();
    }
}

PADialog::~PADialog()
{
    m_rPIManager.writePrinterConfig();
    freePadminRC();
}

long PADialog::Notify( NotifyEvent& rEv )
{
    if( IsVisible() &&
        (rEv.GetType() == EVENT_GETFOCUS || rEv.GetType() == EVENT_LOSEFOCUS )
      )
    {
        if( m_rPIManager.checkPrintersChanged( true ) )
        {
            String aSelectEntry = m_aDevicesLB.GetSelectEntry();
            UpdateDevice();
            UpdateText();
            m_aDevicesLB.SelectEntry( aSelectEntry );
        }
    }
    return ModalDialog::Notify( rEv );
}

void PADialog::DataChanged( const DataChangedEvent& rEv )
{
    ModalDialog::DataChanged( rEv );
    if( (rEv.GetType() == DATACHANGED_SETTINGS) &&
        (rEv.GetFlags() & SETTINGS_STYLE) )
    {
        updateSettings();
        // push the new images into the listbox
        UpdateDevice();
    }
}

String PADialog::getSelectedDevice()
{
    int nPos = m_aDevicesLB.GetSelectEntryPos();
    int nLen = (int)(sal_IntPtr)m_aDevicesLB.GetEntryData( nPos );
    return m_aDevicesLB.GetEntry( nPos ).copy( 0, nLen );
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
    else if( static_cast<Button*>(pButton) == &m_aCUPSCB )
    {
        m_rPIManager.setCUPSDisabled( m_aCUPSCB.IsChecked() );
            UpdateDevice();
            UpdateText();
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
            m_aRemPB.Enable( sal_False );
        else
            m_aRemPB.Enable( sal_True );
        UpdateText();
    }
    return 0;
}

void PADialog::UpdateDefPrt()
{
    m_rPIManager.setDefaultPrinter( getSelectedDevice() );

    UpdateDevice();
    UpdateText();

    if( m_aRemPB.HasFocus() )
        m_aDevicesLB.GetFocus();
    m_aRemPB.Enable( sal_False );
}

void PADialog::UpdateText()
{
    OUString aDev( getSelectedDevice() );
    if( !aDev.isEmpty() )
    {
        const PrinterInfo& rInfo = m_rPIManager.getPrinterInfo( aDev );

        OUString aDriver = rInfo.m_aPrinterName + " (" + rInfo.m_aDriverName + ")";

        m_aDriver.SetText( aDriver );
        m_aCommand.SetText( rInfo.m_aCommand );
        m_aComment.SetText( rInfo.m_aComment );
        m_aLocation.SetText( rInfo.m_aLocation );
    }
    else // nothing selected
    {
        String aEmpty;
        m_aDriver.SetText( aEmpty );
        m_aCommand.SetText( aEmpty );
        m_aComment.SetText( aEmpty );
        m_aLocation.SetText( aEmpty );
    }
}

static Point project( const Point& rPoint )
{
    const double angle_x = M_PI / 6.0;
    const double angle_z = M_PI / 6.0;

    // transform planar coordinates to 3d
    double x = rPoint.X();
    double y = rPoint.Y();
    //double z = 0;

    // rotate around X axis
    double x1 = x;
    double y1 = y * cos( angle_x );
    double z1 = y * sin( angle_x );

    // rotate around Z axis
    double x2 = x1 * cos( angle_z ) + y1 * sin( angle_z );
    //double y2 = y1 * cos( angle_z ) - x1 * sin( angle_z );
    double z2 = z1;

    return Point( (sal_Int32)x2, (sal_Int32)z2 );
}

static Color approachColor( const Color& rFrom, const Color& rTo )
{
    Color aColor;
    sal_uInt8 nDiff;
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

class SpaPrinterController : public vcl::PrinterController
{
public:
    SpaPrinterController( const boost::shared_ptr<Printer>& i_pPrinter )
    : vcl::PrinterController( i_pPrinter )
    {}
    virtual ~SpaPrinterController()
    {}

    virtual int getPageCount() const { return 1; }
    virtual Sequence< PropertyValue > getPageParameters( int i_nPage ) const;
    virtual void printPage( int i_nPage ) const;
    virtual void jobFinished( com::sun::star::view::PrintableState );
};

Sequence< PropertyValue > SpaPrinterController::getPageParameters( int ) const
{
    Sequence< PropertyValue > aRet( 1 );

    Size aPageSize( getPrinter()->GetPaperSizePixel() );
    aPageSize = getPrinter()->PixelToLogic( aPageSize, MapMode( MAP_100TH_MM ) );

    awt::Size aSize;
    aSize.Width = aPageSize.Width();
    aSize.Height = aPageSize.Height();
    aRet[0].Value = makeAny(aSize);

    return aRet;
}

void SpaPrinterController::printPage( int ) const
{
    const double DELTA = 5.0;

    boost::shared_ptr<Printer> pPrinter( getPrinter() );

    PrinterInfo aInfo( psp::PrinterInfoManager::get().getPrinterInfo( pPrinter->GetName() ) );
    const PPDParser* pPrintParser = aInfo.m_pParser;

    MapMode aMapMode( MAP_100TH_MM );

    Bitmap aButterfly( PaResId( RID_BUTTERFLY ) );

    pPrinter->SetMapMode( aMapMode );

    Size aPaperSize=pPrinter->GetOutputSize();
    Point aCenter( aPaperSize.Width()/2-300,
                   aPaperSize.Height() - aPaperSize.Width()/2 );
    Point aP1( aPaperSize.Width()/48, 0), aP2( aPaperSize.Width()/40, 0 ), aPoint;

    pPrinter->DrawRect( Rectangle( Point( 0,0 ), aPaperSize ) );
    pPrinter->DrawRect( Rectangle( Point( 100,100 ),
                                    Size( aPaperSize.Width()-200,
                                          aPaperSize.Height()-200 ) ) );
    pPrinter->DrawRect( Rectangle( Point( 200,200 ),
                                    Size( aPaperSize.Width()-400,
                                          aPaperSize.Height()-400 ) ) );
    pPrinter->DrawRect( Rectangle( Point( 300,300 ),
                                    Size( aPaperSize.Width()-600,
                                          aPaperSize.Height()-600 ) ) );

    Font aFont( String( "Courier" ), Size( 0, 400 ) );
    aFont.SetWeight( WEIGHT_NORMAL );
    aFont.SetItalic( ITALIC_NONE );
    pPrinter->SetFont( aFont );

    OUStringBuffer aPrintText(1024);
    long nWidth = 0, nMaxWidth = 0;
    String aToken;

    static const struct
    {
            const char* const pDirect;
            sal_uInt16 nResId;
    } aResIds[] =
        {
            { NULL, RID_TXT_TESTPAGE_NAME },
            { NULL, RID_TXT_TESTPAGE_MODEL },
            { "PPD", 0 },
            { NULL, RID_TXT_TESTPAGE_QUEUE },
            { NULL, RID_TXT_TESTPAGE_COMMENT },
            { NULL, RID_TXT_TESTPAGE_DATE },
            { NULL, RID_TXT_TESTPAGE_TIME }
        };

    for( unsigned int i = 0; i < SAL_N_ELEMENTS(aResIds); i++ )
    {
        if( aResIds[i].pDirect )
            aToken = OUString::createFromAscii(aResIds[i].pDirect);
        else
            aToken = String( PaResId( aResIds[i].nResId ) );
        nMaxWidth = ( nWidth = pPrinter->GetTextWidth( aToken ) ) > nMaxWidth ? nWidth : nMaxWidth;
        aPrintText.append( aToken );
        aPrintText.append( (sal_Unicode)'\n' );
    };

    pPrinter->DrawText( Rectangle( Point( 1000, 1000 ),
                                    Size( aPaperSize.Width() - 2000,
                                          aPaperSize.Height() - 4000 ) ),
                          aPrintText.makeStringAndClear(),
                          TEXT_DRAW_MULTILINE );

    AllSettings aSettings( Application::GetSettings() );
    const LocaleDataWrapper& rLocaleWrapper( aSettings.GetLocaleDataWrapper() );

    aPrintText.appendAscii( ": " );
    aPrintText.append( pPrinter->GetName() );
    aPrintText.appendAscii( "\n: " );
    if( pPrintParser )
        aPrintText.append( pPrintParser->getPrinterName() );
    aPrintText.appendAscii( "\n: " );
    INetURLObject aDriverPath( pPrintParser ? pPrintParser->getFilename() : String( "<undef>" ),
                               INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    aPrintText.append( aDriverPath.GetName() );
    aPrintText.appendAscii( "\n: " );
    aPrintText.append( aInfo.m_aCommand );
    aPrintText.appendAscii( "\n: " );
    aPrintText.append( aInfo.m_aComment );
    aPrintText.appendAscii( "\n: " );
    aPrintText.append( rLocaleWrapper.getDate( Date( Date::SYSTEM ) ) );
    aPrintText.appendAscii( "\n: " );
    aPrintText.append( rLocaleWrapper.getTime( Time( Time::SYSTEM ) ) );

    pPrinter->DrawText( Rectangle( Point( 1100 + nMaxWidth, 1000 ),
                                    Size( aPaperSize.Width() - 2100 - nMaxWidth,
                                          aPaperSize.Height() - 4000 ) ),
                         aPrintText.makeStringAndClear(),
                         TEXT_DRAW_MULTILINE );

    pPrinter->DrawBitmap( Point( aPaperSize.Width() - 4000, 1000 ),
                           Size( 3000,3000 ),
                           aButterfly );
    pPrinter->SetFillColor();
    pPrinter->DrawRect( Rectangle( Point( aPaperSize.Width() - 4000, 1000 ),
                                     Size( 3000,3000 ) ) );

    Color aWhite( 0xff, 0xff, 0xff );
    Color aBlack( 0, 0, 0 );
    Color aLightRed( 0xff, 0, 0 );
    Color aDarkRed( 0x40, 0, 0 );
    Color aLightBlue( 0, 0, 0xff );
    Color aDarkBlue( 0,0,0x40 );
    Color aLightGreen( 0, 0xff, 0 );
    Color aDarkGreen( 0, 0x40, 0 );

    Gradient aGradient( GradientStyle_LINEAR, aBlack, aWhite );
    aGradient.SetAngle( 900 );
    pPrinter->DrawGradient( Rectangle( Point( 1000, 5500 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkRed );
    aGradient.SetEndColor( aLightBlue );
    pPrinter->DrawGradient( Rectangle( Point( 1000, 6300 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkBlue );
    aGradient.SetEndColor( aLightGreen );
    pPrinter->DrawGradient( Rectangle( Point( 1000, 7100 ),
                                        Size( aPaperSize.Width() - 2000,
                                              500 ) ), aGradient );
    aGradient.SetStartColor( aDarkGreen );
    aGradient.SetEndColor( aLightRed );
    pPrinter->DrawGradient( Rectangle( Point( 1000, 7900 ),
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
        pPrinter->SetLineColor( aLineColor );

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

        pPrinter->DrawLine( project( aP1 ) + aCenter,
                             project( aP2 ) + aCenter,
                             aLineInfo );
        aPoint.X() = (int)((((double)aP1.X())*cosd - ((double)aP1.Y())*sind)*factor);
        aPoint.Y() = (int)((((double)aP1.Y())*cosd + ((double)aP1.X())*sind)*factor);
        aP1 = aPoint;
        aPoint.X() = (int)((((double)aP2.X())*cosd - ((double)aP2.Y())*sind)*factor);
        aPoint.Y() = (int)((((double)aP2.Y())*cosd + ((double)aP2.X())*sind)*factor);
        aP2 = aPoint;
    }
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
    fprintf( stderr, "%d lines\n",n );
#endif
}

void SpaPrinterController::jobFinished( com::sun::star::view::PrintableState )
{
    String aInfoString( PaResId( RID_PA_TXT_TESTPAGE_PRINTED ) );
    InfoBox aInfoBox( NULL, aInfoString );
    aInfoBox.SetText( String( PaResId( RID_BXT_TESTPAGE ) ) );
    aInfoBox.Execute();
}

void PADialog::PrintTestPage()
{
    const OUString sPrinter( getSelectedDevice() );

    boost::shared_ptr<Printer> pPrinter( new Printer( sPrinter ) );

    if( pPrinter->GetName() != sPrinter )
    {
        String aString( PaResId( RID_ERR_NOPRINTER ) );
        aString.SearchAndReplaceAscii( "%s", sPrinter );

        ErrorBox aErrorBox( this, WB_OK | WB_DEF_OK, aString );
        aErrorBox.SetText( String( PaResId( RID_BXT_ENVIRONMENT ) ) );
        aErrorBox.Execute();
        return;
    }

    boost::shared_ptr<vcl::PrinterController> pController( new SpaPrinterController( pPrinter ) );
    JobSetup aJobSetup( pPrinter->GetJobSetup() );
    aJobSetup.SetValue( "IsQuickJob", "true" );
    Printer::PrintJob( pController, aJobSetup );
}

void PADialog::AddDevice()
{
    AddPrinterDialog aDlg( this );

    if( aDlg.Execute() )
        UpdateDevice();
}

void PADialog::RemDevice()
{
    OUString aPrinter( getSelectedDevice() );
    OUString aDefPrinter( m_rPIManager.getDefaultPrinter() );
    // do not remove the default printer
    if( aPrinter.equals( aDefPrinter ) )
        return;

    if( ! m_rPIManager.removePrinter( aPrinter ) )
    {
        String aText( PaResId( RID_ERR_PRINTERNOTREMOVABLE ) );
        aText.SearchAndReplace( String( "%s" ), aPrinter );
        ErrorBox aBox( this, WB_OK | WB_DEF_OK, aText );
        aBox.Execute();
        return;
    }
    m_aPrinters.remove( aPrinter );

    m_aDevicesLB.RemoveEntry( m_aDevicesLB.GetSelectEntryPos() );
    for (int i=0; i < m_aDevicesLB.GetEntryCount(); ++i)
    {
        if (m_aDevicesLB.GetEntry(i).equals(aDefPrinter))
        {
            m_aDevicesLB.SelectEntryPos( i, sal_True );
            UpdateText();
            break;
        }
    }

    m_aDevicesLB.GetFocus();

    if( m_aDevicesLB.GetEntryCount() < 2 )
        m_aRemPB.Enable( sal_False );
}

void PADialog::ConfigureDevice()
{
    OUString aPrinter(getSelectedDevice());

    if (aPrinter.isEmpty())
        return;

    PrinterInfo aInfo( m_rPIManager.getPrinterInfo( aPrinter ) );
    RTSDialog aDialog( aInfo, aPrinter, true, this );

    if( aDialog.Execute() )
        m_rPIManager.changePrinterInfo( aPrinter, aDialog.getSetup() );

    UpdateText();
}

void PADialog::RenameDevice()
{
    String aPrinter( getSelectedDevice() );
    OUString aOldPrinter( aPrinter );

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
            if( !aToken.isEmpty() )
            {
                if( aToken.compareToAscii( "autoqueue" ) == 0 )
                    bAutoQueue = true;
                else if( aToken.startsWith("pdf=") )
                    bPdf = true;
                else if( aToken.startsWith("fax") )
                    bFax = true;
            }
        }
        if( bAutoQueue )
            continue;

        OUString aEntry( *it );
        if( *it == m_rPIManager.getDefaultPrinter() )
        {
            aEntry += " (" + OUString( m_aDefPrt ) + ")";
        }
        int nPos =
            m_aDevicesLB.InsertEntry( aEntry,
                                      bFax ? m_aFaxImg :
                                      bPdf ? m_aPdfImg : m_aPrinterImg
                                      );
        m_aDevicesLB.SetEntryData( nPos, (void*)(sal_IntPtr)it->getLength() );
        if( *it == m_rPIManager.getDefaultPrinter() )
        {
            m_aDevicesLB.SelectEntryPos( nPos );
            UpdateText();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
