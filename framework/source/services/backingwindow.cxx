/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backingwindow.cxx,v $
 * $Revision: 1.12 $
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

// autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "backingwindow.hxx"
#include "framework.hrc"
#include "classes/fwkresid.hxx"

#include "vcl/metric.hxx"
#include "vcl/mnemonic.hxx"
#include "vcl/menu.hxx"
#include "vcl/svapp.hxx"

#include "tools/urlobj.hxx"

#include "svtools/dynamicmenuoptions.hxx"
#include "svtools/imagemgr.hxx"
#include "svtools/svtools.hrc"

#include "comphelper/processfactory.hxx"
#include "comphelper/sequenceashashmap.hxx"

#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace framework;

#define WRITER_URL      "private:factory/swriter"
#define CALC_URL        "private:factory/scalc"
#define IMPRESS_WIZARD_URL     "private:factory/simpress?slot=6686"
#define DRAW_URL        "private:factory/sdraw"
#define BASE_URL        "private:factory/sdatabase?Interactive"
#define MATH_URL        "private:factory/smath"
#define TEMPLATE_URL    "slot:5500"
#define OPEN_URL        ".uno:Open"

DecoToolBox::DecoToolBox( Window* pParent, WinBits nStyle ) :
    ToolBox( pParent, nStyle )
{
        SetBackground();
        SetPaintTransparent( TRUE );
}

void DecoToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( rDCEvt.GetFlags() & SETTINGS_STYLE )
    {
        calcMinSize();
        SetBackground();
        SetPaintTransparent( TRUE );
    }
}

void DecoToolBox::calcMinSize()
{
    ToolBox aTbx( GetParent() );
    USHORT nItems = GetItemCount();
    for( USHORT i = 0; i < nItems; i++ )
    {
        USHORT nId = GetItemId( i );
        aTbx.InsertItem( nId, GetItemImage( nId ) );
    }
    aTbx.SetOutStyle( TOOLBOX_STYLE_FLAT );
    maMinSize = aTbx.CalcWindowSizePixel();
}

Size DecoToolBox::getMinSize()
{
    return maMinSize;
}


BackingWindow::BackingWindow( Window* i_pParent ) :
    Window( i_pParent, FwkResId( DLG_BACKING ) ),
    maWelcome( this, WB_LEFT ),
    maProduct( this, WB_LEFT ),
    maCreateText( this, WB_LEFT ),
    maWriterText( this, WB_WORDBREAK | WB_VCENTER ),
    maWriterButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maCalcText( this, WB_WORDBREAK | WB_VCENTER ),
    maCalcButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maImpressText( this, WB_WORDBREAK | WB_VCENTER ),
    maImpressButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maDrawText( this, WB_WORDBREAK | WB_VCENTER ),
    maDrawButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maDBText( this, WB_WORDBREAK | WB_VCENTER ),
    maDBButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maMathText( this, WB_WORDBREAK | WB_VCENTER ),
    maMathButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maTemplateText( this, WB_WORDBREAK | WB_VCENTER ),
    maTemplateButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maOpenText( this, WB_WORDBREAK | WB_VCENTER ),
    maOpenButton( this, WB_CENTER | WB_BEVELBUTTON ),
    maToolbox( this, WB_DIALOGCONTROL ),
    maWelcomeString( FwkResId( STR_BACKING_WELCOME ) ),
    maProductString( FwkResId( STR_BACKING_WELCOMEPRODUCT ) ),
    maCreateString( FwkResId( STR_BACKING_CREATE ) ),
    maOpenString( FwkResId( STR_BACKING_FILE ) ),
    maTemplateString( FwkResId( STR_BACKING_TEMPLATE ) ),
    maButtonImageSize( 10, 10 ),
    mbInitControls( false ),
    mpAccExec( NULL )
{
    mnColumnWidth[0] = mnColumnWidth[1] = 0;

    // get icon images from vcl resource and set them on the appropriate buttons
    loadImage( FwkResId( BMP_BACKING_WRITER ), maWriterButton );
    loadImage( FwkResId( BMP_BACKING_CALC ), maCalcButton );
    loadImage( FwkResId( BMP_BACKING_IMPRESS ), maImpressButton );
    loadImage( FwkResId( BMP_BACKING_DRAW ), maDrawButton );
    loadImage( FwkResId( BMP_BACKING_DATABASE ), maDBButton );
    loadImage( FwkResId( BMP_BACKING_FORMULA ), maMathButton );
    loadImage( FwkResId( BMP_BACKING_OPENFILE ), maOpenButton );
    loadImage( FwkResId( BMP_BACKING_OPENTEMPLATE ), maTemplateButton );

    BitmapEx aExtImage( FwkResId( BMP_BACKING_EXT ) );
    String aExtHelpText( FwkResId( STR_BACKING_EXTHELP ) );
    BitmapEx aRegImage( FwkResId( BMP_BACKING_REG ) );
    String aRegHelpText( FwkResId( STR_BACKING_REGHELP ) );
    BitmapEx aInfoImage( FwkResId( BMP_BACKING_INFO ) );
    String aInfoHelpText( FwkResId( STR_BACKING_INFOHELP ) );
    BitmapEx aTplRepImage( FwkResId( BMP_BACKING_TPLREP ) );
    String aTplRepHelpText( FwkResId( STR_BACKING_TPLREP ) );

    // clean up resource stack
    FreeResource();

    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    // add some breathing space for the images
    maButtonImageSize.Width() += 12;
    maButtonImageSize.Height() += 12;

    // force tab cycling in toolbox
    maToolbox.SetStyle( maToolbox.GetStyle() | WB_FORCETABCYCLE );

    // insert toolbox items
    maToolbox.InsertItem( nItemId_TplRep, Image( aTplRepImage ) );
    maToolbox.SetItemText( nItemId_TplRep, aTplRepHelpText );
    maToolbox.SetQuickHelpText( nItemId_TplRep, aTplRepHelpText );
    maToolbox.SetItemCommand( nItemId_TplRep, String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:TemplateRepository" ) ) );
    maToolbox.ShowItem( nItemId_TplRep );

    maToolbox.InsertItem( nItemId_Extensions, Image( aExtImage ) );
    maToolbox.SetQuickHelpText( nItemId_Extensions, aExtHelpText );
    maToolbox.SetItemText( nItemId_Extensions, aExtHelpText );
    maToolbox.SetItemCommand( nItemId_Extensions, String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:Extensions" ) ) );
    maToolbox.ShowItem( nItemId_Extensions );

    maToolbox.InsertItem( nItemId_Reg, Image( aRegImage ) );
    maToolbox.SetQuickHelpText( nItemId_Reg, aRegHelpText );
    maToolbox.SetItemText( nItemId_Reg, aRegHelpText );
    maToolbox.SetItemCommand( nItemId_Reg, String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:Register" ) ) );
    maToolbox.ShowItem( nItemId_Reg );

    maToolbox.InsertItem( nItemId_Info, Image( aInfoImage ) );
    maToolbox.SetItemText( nItemId_Info, aInfoHelpText );
    maToolbox.SetQuickHelpText( nItemId_Info, aInfoHelpText );
    maToolbox.SetItemCommand( nItemId_Info, String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:Info" ) ) );
    maToolbox.ShowItem( nItemId_Info );

    // get dispatch provider
    mxDesktop = Reference<XDesktop>( comphelper::getProcessServiceFactory()->createInstance(
                                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                                        UNO_QUERY );
    if( mxDesktop.is() )
        mxDesktopDispatchProvider = Reference< XDispatchProvider >( mxDesktop, UNO_QUERY );

    maWriterButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:WriterButton" ) ) ) );
    maCalcButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:CalcButton" ) ) ) );
    maImpressButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:ImpressButton" ) ) ) );
    maDrawButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:DrawButton" ) ) ) );
    maDBButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:DBButton" ) ) ) );
    maMathButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:MathButton" ) ) ) );
    maTemplateButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:TemplateButton" ) ) ) );
    maOpenButton.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:OpenButton" ) ) ) );
    maToolbox.SetSmartHelpId( SmartId( String( RTL_CONSTASCII_USTRINGPARAM( ".HelpId:StartCenter:Toolbox" ) ) ) );

    // init background
    initBackground();
}


BackingWindow::~BackingWindow()
{
    delete mpAccExec;
}

class ImageContainerRes : public Resource
{
    public:
    ImageContainerRes( const ResId& i_rId ) : Resource( i_rId ) {}
    ~ImageContainerRes() { FreeResource(); }
};

void BackingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( rDCEvt.GetFlags() & SETTINGS_STYLE )
    {
        initBackground();
    }
}

void BackingWindow::initBackground()
{
    SetBackground( GetSettings().GetStyleSettings().GetWorkspaceGradient() );

    bool bDark = GetSettings().GetStyleSettings().GetWindowColor().IsDark();
    maWelcomeTextColor = maLabelTextColor =  bDark ? Color( COL_WHITE ) : Color( 0x26, 0x35, 0x42 );
    Color aTextBGColor( bDark ? COL_BLACK : COL_WHITE );

    // select image set
    ImageContainerRes aRes( FwkResId( bDark ? RES_BACKING_IMAGES_HC : RES_BACKING_IMAGES ) );

    // scale middle segment
    Size aMiddleSize;
    if( !! maBackgroundMiddle )
        aMiddleSize = maBackgroundMiddle.GetSizePixel();
    // load middle segment
    maBackgroundMiddle = BitmapEx( FwkResId( BMP_BACKING_BACKGROUND_MIDDLE ) );
    // and scale it to previous size
    if( aMiddleSize.Width() && aMiddleSize.Height() )
        maBackgroundMiddle.Scale( aMiddleSize );

    if( GetSettings().GetLayoutRTL() )
    {
        // replace images by RTL versions
        maBackgroundLeft = BitmapEx( FwkResId( BMP_BACKING_BACKGROUND_RTL_RIGHT ) );
        maBackgroundRight = BitmapEx( FwkResId( BMP_BACKING_BACKGROUND_RTL_LEFT) );
    }
    else
    {
        maBackgroundLeft = BitmapEx( FwkResId( BMP_BACKING_BACKGROUND_LEFT ) );
        maBackgroundRight = BitmapEx( FwkResId( BMP_BACKING_BACKGROUND_RIGHT ) );
    }

    maWelcome.SetControlForeground( maWelcomeTextColor );
    maWelcome.SetControlBackground( aTextBGColor );
    maProduct.SetControlForeground( maWelcomeTextColor );
    maProduct.SetControlBackground( aTextBGColor );
    maCreateText.SetControlForeground( maLabelTextColor );
    maCreateText.SetControlBackground( aTextBGColor );
    maWriterText.SetControlForeground( maLabelTextColor );
    maWriterText.SetControlBackground( aTextBGColor );
    maCalcText.SetControlForeground( maLabelTextColor );
    maCalcText.SetControlBackground( aTextBGColor );
    maImpressText.SetControlForeground( maLabelTextColor );
    maImpressText.SetControlBackground( aTextBGColor );
    maDrawText.SetControlForeground( maLabelTextColor );
    maDrawText.SetControlBackground( aTextBGColor );
    maDBText.SetControlForeground( maLabelTextColor );
    maDBText.SetControlBackground( aTextBGColor );
    maMathText.SetControlForeground( maLabelTextColor );
    maMathText.SetControlBackground( aTextBGColor );
    maTemplateText.SetControlForeground( maLabelTextColor );
    maTemplateText.SetControlBackground( aTextBGColor );
    maOpenText.SetControlForeground( maLabelTextColor );
    maOpenText.SetControlBackground( aTextBGColor );
}

void BackingWindow::initControls()
{
    if( mbInitControls )
        return;

    mbInitControls = true;

    // calculate dialog size
    // begin with background bitmap
    maControlRect = Rectangle( Point(), maBackgroundLeft.GetSizePixel() );
    maControlRect.Left() += nShadowLeft;
    maControlRect.Right() -= nShadowRight;
    maControlRect.Top() += nShadowTop;
    maControlRect.Bottom() -= nShadowBottom;

    long nYPos = 0;
    // set bigger welcome string
    maWelcome.SetText( maWelcomeString );
    maTextFont = GetSettings().GetStyleSettings().GetLabelFont();
    maTextFont.SetSize( Size( 0, 18 ) );
    maTextFont.SetWeight( WEIGHT_BOLD );
    maWelcome.SetFont( maTextFont );
    // get metric to get correct width factor and adjust
    long nW = (maWelcome.GetFontMetric().GetWidth()*95)/100;
    maTextFont.SetSize( Size( nW, 18 ) );

    maWelcome.SetFont( maTextFont );
    maWelcome.SetControlFont( maTextFont );
    maWelcomeSize = Size( maWelcome.GetTextWidth( maWelcomeString ), maWelcome.GetTextHeight() );
    maWelcomeSize.Width() = (maWelcomeSize.Width() * 20)/19;

    nYPos += (maWelcomeSize.Height()*3)/2;

    if( maControlRect.GetWidth() < nBtnPos + maWelcomeSize.Width() + 20 )
        maControlRect.Right() = maControlRect.Left() + maWelcomeSize.Width() + nBtnPos + 20;

    maWelcome.Show();

    nYPos += maWelcomeSize.Height();

    // set product string
    maTextFont.SetSize( Size( 0, 30 ) );
    maProduct.SetFont( maTextFont );

    // get metric to get correct width factor and adjust
    nW = (maProduct.GetFontMetric().GetWidth()*95)/100;
    maTextFont.SetSize( Size( nW, 28 ) );

    maProduct.SetFont( maTextFont );
    maProduct.SetControlFont( maTextFont );
    maProduct.SetText( maProductString );
    maProductSize = Size( maProduct.GetTextWidth( maProductString ), maProduct.GetTextHeight() );
    maProductSize.Width() = (maProductSize.Width() * 20)/19;

    if( maControlRect.GetWidth() < maProductSize.Width() + nBtnPos + 10 )
        maControlRect.Right() = maControlRect.Left() + maProductSize.Width() + nBtnPos + 10;

    maProduct.Show();

    nYPos += (maProductSize.Height()*3)/2;

    // set a slighly larger font than normal labels on the texts
    maTextFont.SetSize( Size( 0, 11 ) );
    maTextFont.SetWeight( WEIGHT_NORMAL );

    maCreateText.SetText( maCreateString );
    maCreateText.SetFont( maTextFont );
    maCreateText.SetControlFont( maTextFont );
    maCreateSize = Size( maCreateText.GetTextWidth( maCreateString ), maCreateText.GetTextHeight() );
    maCreateText.Show();

    nYPos += (maCreateSize.Height()*3)/2;

    // collect the URLs of the entries in the File/New menu
    SvtModuleOptions    aModuleOptions;
    std::set< rtl::OUString > aFileNewAppsAvailable;
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aNewMenu = aOpt.GetMenu( E_NEWMENU );
    const rtl::OUString sURLKey( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );

    const Sequence< PropertyValue >* pNewMenu = aNewMenu.getConstArray();
    const Sequence< PropertyValue >* pNewMenuEnd = aNewMenu.getConstArray() + aNewMenu.getLength();
    for ( ; pNewMenu != pNewMenuEnd; ++pNewMenu )
    {
        comphelper::SequenceAsHashMap aEntryItems( *pNewMenu );
        rtl::OUString sURL( aEntryItems.getUnpackedValueOrDefault( sURLKey, rtl::OUString() ) );
        if ( sURL.getLength() )
            aFileNewAppsAvailable.insert( sURL );
    }

    // create mnemonics on the fly, preregister the mnemonics of the menu
    MnemonicGenerator aMnemns;
    maTemplateString = MnemonicGenerator::EraseAllMnemonicChars( maTemplateString );
    maOpenString = MnemonicGenerator::EraseAllMnemonicChars( maOpenString );

    SystemWindow* pSysWin = GetSystemWindow();
    if( pSysWin )
    {
        MenuBar* pMBar = pSysWin->GetMenuBar();
        if( pMBar )
        {
            for( USHORT i = 0; i < pMBar->GetItemCount(); i++ )
            {
                USHORT nItemId = pMBar->GetItemId( i );
                String aItemText( pMBar->GetItemText( nItemId ) );
                if( aItemText.Len() )
                    aMnemns.RegisterMnemonic( aItemText );
            }
        }
    }

    // layout the buttons
    layoutButtonAndText( WRITER_URL, 0, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SWRITER,
                         maWriterButton, maWriterText, aMnemns );
    layoutButtonAndText( CALC_URL, 1, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SCALC,
                         maCalcButton, maCalcText, aMnemns );
    nYPos += maButtonImageSize.Height() + 10;
    layoutButtonAndText( IMPRESS_WIZARD_URL, 0, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SIMPRESS,
                         maImpressButton, maImpressText, aMnemns );
    layoutButtonAndText( DRAW_URL, 1, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SDRAW,
                         maDrawButton, maDrawText, aMnemns );
    nYPos += maButtonImageSize.Height() + 10;
    layoutButtonAndText( BASE_URL, 0, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SDATABASE,
                         maDBButton, maDBText, aMnemns );
    layoutButtonAndText( MATH_URL, 1, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SMATH,
                         maMathButton, maMathText, aMnemns );

    nYPos += 3*maButtonImageSize.Height() / 2;
    layoutButtonAndText( NULL, -1, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SWRITER,
                         maTemplateButton, maTemplateText, aMnemns, maTemplateString );

    nYPos += 10;
    layoutButtonAndText( NULL, -1, aFileNewAppsAvailable,
                         aModuleOptions, SvtModuleOptions::E_SWRITER,
                         maOpenButton, maOpenText, aMnemns, maOpenString );
    nYPos += 10;

    DBG_ASSERT( nYPos < maControlRect.GetHeight(), "misformatting !" );
    if( mnColumnWidth[0] + mnColumnWidth[1] + nBtnPos + 20 > maControlRect.GetWidth() )
        maControlRect.Right() = maControlRect.Left() + mnColumnWidth[0] + mnColumnWidth[1] + nBtnPos + 20;

    maToolbox.SetSelectHdl( LINK( this, BackingWindow, ToolboxHdl ) );
    maToolbox.Show();

    // scale middle map to formatted width
    Size aMiddleSegmentSize( maControlRect.GetSize().Width() + nShadowLeft + nShadowRight,
                             maBackgroundMiddle.GetSizePixel().Height() );

    long nLW = maBackgroundLeft.GetSizePixel().Width();
    long nRW = maBackgroundRight.GetSizePixel().Width();
    if( aMiddleSegmentSize.Width() > nLW + nRW )
    {
        aMiddleSegmentSize.Width() -= nLW;
        aMiddleSegmentSize.Width() -= nRW;
        maBackgroundMiddle.Scale( aMiddleSegmentSize );
    }
    else
        maBackgroundMiddle = BitmapEx();

    Resize();
}

void BackingWindow::loadImage( const ResId& i_rId, ImageButton& i_rButton )
{
    BitmapEx aBmp( i_rId );
    Size aImgSize( aBmp.GetSizePixel() );
    if( aImgSize.Width() > maButtonImageSize.Width() )
        maButtonImageSize.Width() = aImgSize.Width();
    if( aImgSize.Height() > maButtonImageSize.Height() )
        maButtonImageSize.Height() = aImgSize.Height();
    i_rButton.SetModeImage( aBmp );
}

void BackingWindow::layoutButtonAndText(
                          const char* i_pURL, int nColumn,
                          const std::set<rtl::OUString>& i_rURLS,
                          SvtModuleOptions& i_rOpt, SvtModuleOptions::EModule i_eMod,
                          ImageButton& i_rBtn, FixedText& i_rText,
                          MnemonicGenerator& i_rMnemns,
                          const String& i_rStr
                          )
{
    rtl::OUString aURL( rtl::OUString::createFromAscii( i_pURL ? i_pURL : "" ) );
    // setup button
    i_rBtn.SetPaintTransparent( TRUE );
    i_rBtn.SetClickHdl( LINK( this, BackingWindow, ClickHdl ) );
    if( i_pURL && (! i_rOpt.IsModuleInstalled( i_eMod ) || i_rURLS.find( aURL ) == i_rURLS.end()) )
    {
        i_rBtn.Enable( FALSE );
    }

    // setup text
    i_rText.SetFont( maTextFont );
    i_rText.SetControlFont( maTextFont );
    String aText( i_rStr.Len() ? i_rStr : SvFileInformationManager::GetDescription( INetURLObject( aURL ) ) );
    i_rMnemns.CreateMnemonic( aText );
    i_rText.SetText( aText );

    long nTextWidth = i_rText.GetTextWidth( i_rText.GetText() );
    i_rText.SetPaintTransparent( TRUE );

    nTextWidth += maButtonImageSize.Width() + 30;
    if( nColumn >= 0 && nColumn < static_cast<int>(sizeof(mnColumnWidth)/sizeof(mnColumnWidth[0])) )
    {
        if( nTextWidth > mnColumnWidth[nColumn] )
            mnColumnWidth[nColumn] = nTextWidth;
    }

    // show the controls
    i_rBtn.Show();
    i_rText.Show();
}

void BackingWindow::Paint( const Rectangle& )
{
    bool bDark = GetSettings().GetStyleSettings().GetWindowColor().IsDark();

    Color aBackColor( bDark ? COL_BLACK : COL_WHITE );

    // fill control rect
    SetLineColor();
    SetFillColor( aBackColor );
    DrawRect( maControlRect );

    // draw bitmap
    if( GetSettings().GetLayoutRTL() )
    {
        Point aTL( maControlRect.TopLeft() );
        aTL.X() -= nShadowRight;
        aTL.Y() -= nShadowTop;
        DrawBitmapEx( aTL, maBackgroundLeft );
        aTL.X() += maBackgroundLeft.GetSizePixel().Width();
        if( !!maBackgroundMiddle )
        {
            DrawBitmapEx( aTL, maBackgroundMiddle );
            aTL.X() += maBackgroundMiddle.GetSizePixel().Width();
        }
        DrawBitmapEx( aTL, maBackgroundRight );
    }
    else
    {
        Point aTL( maControlRect.TopLeft() );
        aTL.X() -= nShadowLeft;
        aTL.Y() -= nShadowTop;
        DrawBitmapEx( aTL, maBackgroundLeft );
        aTL.X() += maBackgroundLeft.GetSizePixel().Width();
        if( !!maBackgroundMiddle )
        {
            DrawBitmapEx( aTL, maBackgroundMiddle );
            aTL.X() += maBackgroundMiddle.GetSizePixel().Width();
        }
        DrawBitmapEx( aTL, maBackgroundRight );
    }
}

long BackingWindow::Notify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if( ! mpAccExec )
        {
            mpAccExec = svt::AcceleratorExecute::createAcceleratorHelper();
            mpAccExec->init( comphelper::getProcessServiceFactory(), mxFrame);
        }

        const KeyEvent* pEvt = rNEvt.GetKeyEvent();
        if( pEvt && mpAccExec->execute(pEvt->GetKeyCode()) )
            return 1;
    }
    return Window::Notify( rNEvt );
}

void BackingWindow::setOwningFrame( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame )
{
    mxFrame = xFrame;
    if( ! mbInitControls )
        initControls();
}

void BackingWindow::Resize()
{
    Size aWindowSize( GetSizePixel() );
    Size aControlSize = maControlRect.GetSize();
    maControlRect = Rectangle( Point( (aWindowSize.Width() - aControlSize.Width()) / 2,
                                      (aWindowSize.Height() - aControlSize.Height()) / 2 ),
                               aControlSize );
    long nYPos = maControlRect.Top();
    nYPos += (maWelcomeSize.Height()*3)/2;
    maWelcome.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ),
                                Size( maControlRect.GetWidth() - nBtnPos - 5, (maWelcomeSize.Height()*20)/19 ) );
    nYPos += maWelcomeSize.Height();
    maProduct.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), Size( maControlRect.GetWidth() - nBtnPos - 5, (maProductSize.Height()*20)/19 ) );
    nYPos += (maProductSize.Height()*3)/2;

    maCreateText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ),
                                  Size( maControlRect.GetWidth() - nBtnPos - 5, maCreateSize.Height() ) );

    nYPos += (maCreateSize.Height()*3)/2;

    maWriterButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), maButtonImageSize );
    maWriterText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10, nYPos ),
                                  Size( mnColumnWidth[0] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    maCalcButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + mnColumnWidth[0], nYPos ), maButtonImageSize );
    maCalcText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10 + mnColumnWidth[0], nYPos ),
                                  Size( mnColumnWidth[1] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    nYPos += maButtonImageSize.Height() + 10;
    maImpressButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), maButtonImageSize );
    maImpressText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10, nYPos ),
                                  Size( mnColumnWidth[0] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    maDrawButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + mnColumnWidth[0], nYPos ), maButtonImageSize );
    maDrawText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10 + mnColumnWidth[0], nYPos ),
                                  Size( mnColumnWidth[1] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    nYPos += maButtonImageSize.Height() + 10;
    maDBButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), maButtonImageSize );
    maDBText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10, nYPos ),
                                  Size( mnColumnWidth[0] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    maMathButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + mnColumnWidth[0], nYPos ), maButtonImageSize );
    maMathText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10 + mnColumnWidth[0], nYPos ),
                                    Size( mnColumnWidth[1] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );

    nYPos += 3*maButtonImageSize.Height()/2;
    maTemplateButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), maButtonImageSize );
    maTemplateText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10, nYPos ),
                                Size( mnColumnWidth[0]+mnColumnWidth[1] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    nYPos += maButtonImageSize.Height() + 10;
    maOpenButton.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos, nYPos ), maButtonImageSize );
    maOpenText.SetPosSizePixel( Point( maControlRect.Left() + nBtnPos + maButtonImageSize.Width() + 10, nYPos ),
                                Size( mnColumnWidth[0]+mnColumnWidth[1] - maButtonImageSize.Width() - 10, maButtonImageSize.Height() ) );
    nYPos += maButtonImageSize.Height() + 10;

    maToolbox.calcMinSize();
    Size aTBSize( maToolbox.getMinSize() );
    maToolbox.SetPosSizePixel( Point( maControlRect.Right() - aTBSize.Width() - 10,
                                      maControlRect.Bottom() - aTBSize.Height() - 10 ),
                               aTBSize );
}

IMPL_LINK( BackingWindow, ToolboxHdl, void*, EMPTYARG )
{
    const char* pNodePath = NULL;
    const char* pNode = NULL;

    switch( maToolbox.GetCurItemId() )
    {
    case nItemId_Extensions:
        pNodePath = "/org.openoffice.Office.Common/Help/StartCenter";
        pNode = "AddFeatureURL";
        break;
    case nItemId_Reg:
        try
        {
            // create the Desktop component which can load components
            Reference < lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
            if( xFactory.is() )
            {
                Reference< task::XJobExecutor > xProductRegistration(
                    xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.setup.ProductRegistration" ) ) ),
                    UNO_QUERY_THROW );

                 // tell it that the user wants to register
                 xProductRegistration->trigger( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RegistrationRequired" ) ) );
            }
        }
        catch( const Exception& )
        {
        }
        break;
    case nItemId_Info:
        pNodePath = "/org.openoffice.Office.Common/Help/StartCenter";
        pNode = "InfoURL";
        break;
    case nItemId_TplRep:
        pNodePath = "/org.openoffice.Office.Common/Help/StartCenter";
        pNode = "TemplateRepositoryURL";
        break;
    default:
        break;
    }
    if( pNodePath && pNode )
    {
        try
        {
            Reference<lang::XMultiServiceFactory> xConfig( comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider"))),
                UNO_QUERY);
            if( xConfig.is() )
            {
                Sequence<Any> args(1);
                PropertyValue val(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("nodepath") ),
                    0,
                    Any(rtl::OUString::createFromAscii(pNodePath)),
                    PropertyState_DIRECT_VALUE);
                args.getArray()[0] <<= val;
                Reference<container::XNameAccess> xNameAccess(
                    xConfig->createInstanceWithArguments(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                        args), UNO_QUERY);
                if( xNameAccess.is() )
                {
                    rtl::OUString sURL;
                    //throws css::container::NoSuchElementException, css::lang::WrappedTargetException
                    Any value( xNameAccess->getByName(rtl::OUString::createFromAscii(pNode)) );
                    sURL = value.get<rtl::OUString> ();

                    Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                        comphelper::getProcessServiceFactory()->createInstance(
                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" ) ) ),
                        UNO_QUERY_THROW);
                    //throws css::lang::IllegalArgumentException, css::system::SystemShellExecuteException
                    xSystemShellExecute->execute( sURL, rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::DEFAULTS);
                }
            }
        }
        catch (Exception& )
        {
        }
    }

    return 0;
}

IMPL_LINK( BackingWindow, ClickHdl, Button*, pButton )
{
    // dispatch the appropriate URL and end the dialog
    if( pButton == &maWriterButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(WRITER_URL) ) );
    else if( pButton == &maCalcButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(CALC_URL) ) );
    else if( pButton == &maImpressButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(IMPRESS_WIZARD_URL) ) );
    else if( pButton == &maDrawButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(DRAW_URL) ) );
    else if( pButton == &maDBButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(BASE_URL) ) );
    else if( pButton == &maMathButton )
        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(MATH_URL) ) );
    else if( pButton == &maOpenButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< com::sun::star::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = rtl::OUString::createFromAscii("Referer");
        pArg[0].Value <<= rtl::OUString::createFromAscii("private:user");

        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(OPEN_URL) ), rtl::OUString(), xFrame, aArgs );
    }
    else if( pButton == &maTemplateButton )
    {
        Reference< XDispatchProvider > xFrame( mxFrame, UNO_QUERY );

        Sequence< com::sun::star::beans::PropertyValue > aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = rtl::OUString::createFromAscii("Referer");
        pArg[0].Value <<= rtl::OUString::createFromAscii("private:user");

        dispatchURL( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(TEMPLATE_URL) ), rtl::OUString(), xFrame, aArgs );
    }
    return 0;
}

Window* BackingWindow::GetParentLabelFor( const Window* pLabel ) const
{
    const Window* pRet = NULL;

    if( pLabel == &maWriterText )
        pRet = &maWriterButton;
    else if( pLabel == &maCalcText )
        pRet = &maCalcButton;
    else if( pLabel == &maImpressText )
        pRet = &maImpressButton;
    else if( pLabel == &maDrawText )
        pRet = &maDrawButton;
    else if( pLabel == &maDBText )
        pRet = &maDBButton;
    else if( pLabel == &maMathText )
        pRet = &maMathButton;
    else if( pLabel == &maTemplateText )
        pRet = &maTemplateButton;
    else if( pLabel == &maOpenText )
        pRet = &maOpenButton;

    return const_cast<Window*>(pRet);
}

Window* BackingWindow::GetParentLabeledBy( const Window* pLabeled ) const
{
    const Window *pRet = NULL;

    if( pLabeled == &maWriterButton )
        pRet = &maWriterText;
    else if( pLabeled == &maCalcButton )
        pRet = &maCalcText;
    else if( pLabeled == &maImpressButton )
        pRet = &maImpressText;
    else if( pLabeled == &maDrawButton )
        pRet = &maDrawText;
    else if( pLabeled == &maDBButton )
        pRet = &maDBText;
    else if( pLabeled == &maMathButton )
        pRet = &maMathText;
    else if( pLabeled == &maTemplateButton )
        pRet = &maTemplateText;
    else if( pLabeled == &maOpenButton )
        pRet = &maOpenText;

    return const_cast<Window*>(pRet);
}

struct ImplDelayedDispatch
{
    Reference< XDispatch >      xDispatch;
    com::sun::star::util::URL   aDispatchURL;
    Sequence< PropertyValue >   aArgs;

    ImplDelayedDispatch( const Reference< XDispatch >& i_xDispatch,
                         const com::sun::star::util::URL& i_rURL,
                         const Sequence< PropertyValue >& i_rArgs )
    : xDispatch( i_xDispatch ),
      aDispatchURL( i_rURL ),
      aArgs( i_rArgs )
    {
    }
    ~ImplDelayedDispatch() {}
};

static long implDispatchDelayed( void*, void* pArg )
{
    struct ImplDelayedDispatch* pDispatch = reinterpret_cast<ImplDelayedDispatch*>(pArg);
    try
    {
        pDispatch->xDispatch->dispatch( pDispatch->aDispatchURL, pDispatch->aArgs );
    }
    catch( Exception )
    {
    }

    // clean up
    delete pDispatch;

    return 0;
}

void BackingWindow::dispatchURL( const rtl::OUString& i_rURL,
                                 const rtl::OUString& rTarget,
                                 const Reference< XDispatchProvider >& i_xProv,
                                 const Sequence< PropertyValue >& i_rArgs )
{
    // if no special dispatch provider is given, get the desktop
    Reference< XDispatchProvider > xProvider( i_xProv.is() ? i_xProv : mxDesktopDispatchProvider );

    // check for dispatch provider
    if( !xProvider.is())
        return;

    // get an URL transformer to clean up the URL
    com::sun::star::util::URL aDispatchURL;
    aDispatchURL.Complete = i_rURL;

    Reference < com::sun::star::util::XURLTransformer > xURLTransformer(
        comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer") ),
        com::sun::star::uno::UNO_QUERY );
    if ( xURLTransformer.is() )
    {
        try
        {
            // clean up the URL
            xURLTransformer->parseStrict( aDispatchURL );
            // get a Dispatch for the URL and target
            Reference< XDispatch > xDispatch(
                xProvider->queryDispatch( aDispatchURL, rTarget, 0 )
                );
            // dispatch the URL
            if ( xDispatch.is() )
            {
                ImplDelayedDispatch* pDisp = new ImplDelayedDispatch( xDispatch, aDispatchURL, i_rArgs );
                ULONG nEventId = 0;
                if( ! Application::PostUserEvent( nEventId, Link( NULL, implDispatchDelayed ), pDisp ) )
                    delete pDisp; // event could not be posted for unknown reason, at least don't leak
            }
        }
        catch ( com::sun::star::uno::RuntimeException& )
        {
            throw;
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }
}

