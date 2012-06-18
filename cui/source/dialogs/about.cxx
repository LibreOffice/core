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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>
#include <sfx2/sfxuno.hxx>
#include "about.hxx"
#include <sfx2/sfxdefs.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfxcommands.h>
#include "about.hrc"
#include <dialmgr.hxx>
#include <svtools/svtools.hrc>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

// defines ---------------------------------------------------------------

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

// -----------------------------------------------------------------------

static void layoutFixedText( FixedText &rControl, const Point& aPos, Size &aSize, const long nTextWidth )
{
    aSize = rControl.GetSizePixel();
    // change the width
    aSize.Width() = nTextWidth;
    // set Position and Size, to calculate the minimum size
    // this will update the Height
    rControl.SetPosSizePixel( aPos, aSize );
    aSize = rControl.CalcMinimumSize();
    // update the size with the right Height
    rControl.SetSizePixel( aSize );
}

static void layoutEdit( Edit &rControl, const Point& aPos, Size &aSize, const long nTextWidth )
{
    aSize = rControl.GetSizePixel();
    // change the width
    aSize.Width() = nTextWidth;
    // set Position and Size, to calculate the minimum size
    // this will update the Height
    rControl.SetPosSizePixel( aPos, aSize );
    aSize = rControl.CalcMinimumSize();
    // update the size with the right Height
    rControl.SetSizePixel( aSize );
}

// -----------------------------------------------------------------------

AboutDialog::AboutDialog( Window* pParent, const ResId& rId ) :
    SfxModalDialog( pParent, rId ),
    maOKButton( this, ResId( RID_CUI_ABOUT_BTN_OK, *rId.GetResMgr() ) ),
    maVersionText( this, ResId( RID_CUI_ABOUT_FTXT_VERSION, *rId.GetResMgr() ) ),
    maBuildInfoEdit( this, ResId( RID_CUI_ABOUT_FTXT_BUILDDATA, *rId.GetResMgr() ) ),
    maCopyrightEdit( this, ResId( RID_CUI_ABOUT_FTXT_COPYRIGHT, *rId.GetResMgr() ) ),
    maCreditsLink( this, ResId( RID_CUI_ABOUT_FTXT_WELCOME_LINK, *rId.GetResMgr() )  ),
    maCopyrightTextStr( ResId( RID_CUI_ABOUT_STR_COPYRIGHT, *rId.GetResMgr() ) )
{
    // load image from module path
    maAppLogo = SfxApplication::GetApplicationLogo();

    InitControls();

    // set links
    maCreditsLink.SetClickHdl( LINK( this, AboutDialog, OpenLinkHdl_Impl ) );

    FreeResource();

    SetHelpId( CMD_SID_ABOUT );
}

// -----------------------------------------------------------------------

AboutDialog::~AboutDialog()
{
}

// -----------------------------------------------------------------------

void AboutDialog::InitControls()
{
    // apply font, background et al.
    ApplyStyleSettings();

    // set strings
    maCopyrightEdit.SetText( maCopyrightTextStr );
    maBuildInfoEdit.SetText( GetBuildVersionString() );
    maCreditsLink.SetURL( maCreditsLink.GetText() );

    // determine size and position of the dialog & elements
    Size aDlgSize;
    LayoutControls( aDlgSize );

    // Change the width of the dialog
    SetOutputSizePixel( aDlgSize );
}

// -----------------------------------------------------------------------

void AboutDialog::ApplyStyleSettings()
{
    // Transparenter Font
    Font aFont = GetFont();
    aFont.SetTransparent( sal_True );
    SetFont( aFont );

    // set for background and text the correct system color
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    Color aWhiteCol( rSettings.GetWindowColor() );
    Wallpaper aWall( aWhiteCol );
    SetBackground( aWall );
    Font aNewFont( maCopyrightEdit.GetFont() );
    aNewFont.SetTransparent( sal_True );

    maVersionText.SetFont( aNewFont );
    maCopyrightEdit.SetFont( aNewFont );

    maVersionText.SetBackground();
    maCopyrightEdit.SetBackground();
    maBuildInfoEdit.SetBackground();
    maCreditsLink.SetBackground();

    Color aTextColor( rSettings.GetWindowTextColor() );
    maVersionText.SetControlForeground( aTextColor );
    maCopyrightEdit.SetControlForeground( aTextColor );
    maBuildInfoEdit.SetControlForeground( aTextColor );
    maCreditsLink.SetControlForeground();

    Size aSmaller = aNewFont.GetSize();
    aSmaller.Width() = (long) (aSmaller.Width() * 0.75);
    aSmaller.Height() = (long) (aSmaller.Height() * 0.75);
    aNewFont.SetSize( aSmaller );

    maBuildInfoEdit.SetFont( aNewFont );

    // the following is a hack to force the MultiLineEdit update its settings
    // in order to reflect the Font
    // See
    //      Window::SetControlFont
    //      MultiLineEdit::StateChanged
    //      MultiLineEdit::ImplInitSettings
    // TODO Override SetFont in MultiLineEdit and do the following,
    // otherwise SetFont has no effect at all!
    aSmaller = PixelToLogic( aSmaller, MAP_POINT );
    aNewFont.SetSize( aSmaller );
    maBuildInfoEdit.SetControlFont( aNewFont );
}

// -----------------------------------------------------------------------

void AboutDialog::LayoutControls( Size& aDlgSize )
{
    Size aAppLogoSiz = maAppLogo.GetSizePixel();

    aDlgSize = GetOutputSizePixel();
    aDlgSize.Width() = aAppLogoSiz.Width();

    Size a6Size      = maVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nY          = aAppLogoSiz.Height() + ( a6Size.Height() * 2 );
    long nDlgMargin  = a6Size.Width() * 2;
    long nCtrlMargin = a6Size.Height() * 2;
    long nTextWidth  = aDlgSize.Width() - ( nDlgMargin * 2 );

    Point aPos( nDlgMargin, nY );
    Size aSize;
    // layout fixed text control
    layoutFixedText( maVersionText, aPos, aSize, nTextWidth );
    // set the next control closer
    nY += aSize.Height() + (nCtrlMargin / 2);

    // Multiline edit with Build info
    aPos.Y() = nY;
    layoutEdit( maBuildInfoEdit, aPos, aSize, nTextWidth );
    nY += aSize.Height() + nCtrlMargin;

    // Multiline edit with Copyright-Text
    aPos.Y() = nY;
    layoutEdit( maCopyrightEdit, aPos, aSize, nTextWidth );
    // set the next control closer
    nY += aSize.Height() + (nCtrlMargin/2);

    // Hyperlink
    aPos.Y() = nY;
    layoutFixedText( maCreditsLink, aPos, aSize, nTextWidth );
    nY += aSize.Height() + nCtrlMargin;

    // OK-Button-Position (at the bottom and centered)
    Size aOKSiz = maOKButton.GetSizePixel();
    Point aOKPnt( ( aDlgSize.Width() - aOKSiz.Width() ) / 2, nY );
    maOKButton.SetPosPixel( aOKPnt );

    aDlgSize.Height() = aOKPnt.Y() + aOKSiz.Height() + nCtrlMargin;
}

// -----------------------------------------------------------------------

const rtl::OUString AboutDialog::GetBuildId() const
{
    rtl::OUString sDefault;

    // Get buildid from version[rc|.ini]
    rtl::OUString sBuildId( utl::Bootstrap::getBuildIdData( sDefault ) );
    OSL_ENSURE( sBuildId.getLength() > 0, "No BUILDID in bootstrap file" );
    rtl::OUStringBuffer sBuildIdBuff( sBuildId );

    // Get ProductSource from version[rc|.ini]
    rtl::OUString sProductSource( utl::Bootstrap::getProductSource( sDefault ) );
    OSL_ENSURE( sProductSource.getLength() > 0, "No ProductSource in bootstrap file" );

    // the product source is something like "AOO340",
    // while the build id is something like "340m1(Build:9590)"
    // For better readability, strip the duplicate ProductMajor ("340").
    if ( sProductSource.getLength() )
    {
        bool bMatchingUPD =
                ( sProductSource.getLength() >= 3 )
            &&  ( sBuildId.getLength() >= 3 )
            &&  ( sProductSource.copy( sProductSource.getLength() - 3 ) == sBuildId.copy( 0, 3 ) );
        OSL_ENSURE( bMatchingUPD, "BUILDID and ProductSource do not match in their UPD" );
        if ( bMatchingUPD )
            sProductSource = sProductSource.copy( 0, sProductSource.getLength() - 3 );

        // prepend the product source
        sBuildIdBuff.insert( 0, sProductSource );
    }

    return sBuildIdBuff.makeStringAndClear();
}

// -----------------------------------------------------------------------

const rtl::OUString AboutDialog::GetBuildVersionString() const
{
    rtl::OUStringBuffer aBuildString( GetBuildId() );
    rtl::OUString sRevision( utl::Bootstrap::getRevisionInfo() );

    if ( sRevision.getLength() > 0 )
    {
        aBuildString.appendAscii( RTL_CONSTASCII_STRINGPARAM( "  -  Rev. " ) );
        aBuildString.append( sRevision );
    }

#ifdef BUILD_VER_STRING
    rtl::OUString sBuildVer( RTL_CONSTASCII_USTRINGPARAM( STRINGIFY( BUILD_VER_STRING ) ) );
    if ( sBuildVer.getLength() > 0 )
    {
        aBuildString.append( sal_Unicode( '\n' ) );
        aBuildString.append( sBuildVer );
    }
#endif

    return aBuildString.makeStringAndClear();
}

// -----------------------------------------------------------------------

sal_Bool AboutDialog::Close()
{
    EndDialog( RET_OK );
    return( sal_False );
}

// -----------------------------------------------------------------------

void AboutDialog::Paint( const Rectangle& rRect )
{
    SetClipRegion( rRect );
    Point aPos( 0, 0 );
    DrawImage( aPos, maAppLogo );

    return;
}

// -----------------------------------------------------------------------

IMPL_LINK ( AboutDialog, OpenLinkHdl_Impl, svt::FixedHyperlink*, EMPTYARG )
{
    ::rtl::OUString sURL( maCreditsLink.GetURL() );
    if ( sURL.getLength() > 0 )
    {
        try
        {
            com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xSMGR =
                ::comphelper::getProcessServiceFactory();
            com::sun::star::uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShell(
                xSMGR->createInstance( ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" ) ) ),
                com::sun::star::uno::UNO_QUERY_THROW );
            if ( xSystemShell.is() )
                xSystemShell->execute( sURL, ::rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
        }
        catch( const com::sun::star::uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    return 0;
}
