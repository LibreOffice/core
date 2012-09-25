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

#include <comphelper/processfactory.hxx>
#include <dialmgr.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sfx2/sfxcommands.h>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/sfxuno.hxx>
#include <svtools/filter.hxx>
#include <svtools/svtools.hrc>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/graph.hxx>
#include <vcl/imagerepository.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/uno/Any.h>

#include "about.hxx"
#include "about.hrc"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

/* On Windows/OS2, all the three files have .txt extension
   and the README file name is in lowercase
   Readme files are localized and have the locale in their file name:
   README_de README_en-US
*/
#if defined(WNT) || defined(OS2)
#define FILE_EXTENSION  ".txt"
#define README_FILE     "readme"
#else
#define FILE_EXTENSION
#define README_FILE     "README"
#endif
#define LICENSE_FILE    "LICENSE" FILE_EXTENSION
#define NOTICE_FILE     "NOTICE"  FILE_EXTENSION

// Dir where the files are located
#define BRAND_DIR_SHARE_README  "${BRAND_BASE_DIR}/share/readme/"

using namespace com::sun::star;

namespace
{

    static void lcl_layoutFixedText( FixedText &rControl,
                                     const Point& aPos,
                                     Size &aSize,
                                     const long nTextWidth )
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

    static void lcl_layoutEdit( Edit &rControl,
                                const Point& aPos,
                                Size &aSize,
                                const long nTextWidth )
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

    static  void lcl_readTxtFile( const rtl::OUString &rFile, rtl::OUString &sText )
    {
        rtl::OUString sFile( rFile );
        rtl::Bootstrap::expandMacros( sFile );
        osl::File aFile(sFile);
        if ( aFile.open(OpenFlag_Read) == osl::FileBase::E_None )
        {
            osl::DirectoryItem aItem;
            osl::DirectoryItem::get(sFile, aItem);

            osl::FileStatus aStatus(FileStatusMask_FileSize);
            aItem.getFileStatus(aStatus);

            sal_uInt64 nBytesRead = 0;
            sal_uInt64 nPosition = 0;
            sal_uInt32 nBytes = (sal_uInt32)aStatus.getFileSize();

            sal_Char *pBuffer = new sal_Char[nBytes];

            while ( aFile.read( pBuffer + nPosition,
                                nBytes-nPosition,
                                nBytesRead ) == osl::FileBase::E_None
                    && nPosition + nBytesRead < nBytes)
            {
                nPosition += nBytesRead;
            }

            OSL_ENSURE( nBytes < STRING_MAXLEN, "Text file has too much bytes!" );
            if ( nBytes > STRING_MAXLEN )
                nBytes = STRING_MAXLEN - 1;

            sText = rtl::OUString( pBuffer,
                                nBytes,
                                RTL_TEXTENCODING_UTF8,
                                OSTRING_TO_OUSTRING_CVTFLAGS
                                | RTL_TEXTTOUNICODE_FLAGS_GLOBAL_SIGNATURE);
            delete[] pBuffer;
        }
    }

    class ReadmeDialog;

    class ReadmeTabPage : public TabPage
    {
    private:
        MultiLineEdit maText;
        String        msText;

    public:
        ReadmeTabPage(Window *pParent, const String &sText);
        ~ReadmeTabPage();

        void Adjust(const Size &aSz, const Size &a6Size);
    };

    ReadmeTabPage::ReadmeTabPage(Window *pParent, const String &sText)
        : TabPage(pParent, CUI_RES( RID_CUI_README_TBPAGE))
        ,maText( this, CUI_RES( RID_CUI_README_TBPAGE_EDIT ))
        ,msText( sText )
    {
        FreeResource();

        maText.SetText(msText);
        maText.Show();
    }

    ReadmeTabPage::~ReadmeTabPage()
    {
    }

    void ReadmeTabPage::Adjust(const Size &aSz, const Size &a6Size)
    {
        long nDlgMargin  = a6Size.Width() * 2;
        long nCtrlMargin = a6Size.Height() * 2;
        maText.SetPosPixel( Point(a6Size.Width(), a6Size.Height()) );
        maText.SetSizePixel( Size(aSz.Width() - nDlgMargin, aSz.Height() - nCtrlMargin) );
    }

    class ReadmeDialog : public ModalDialog
    {
    private:
        TabControl      maTabCtrl;
        OKButton        maBtnOK;

        ReadmeTabPage  *maReadmeTabPage;
        ReadmeTabPage  *maLicenseTabPage;
        ReadmeTabPage  *maNoticeTabPage;

        DECL_LINK( ActivatePageHdl, TabControl * );
        DECL_LINK( DeactivatePageHdl, TabControl * );

    public:
        ReadmeDialog( Window* );
        ~ReadmeDialog();
    };

    ReadmeDialog::ReadmeDialog( Window * pParent )
        : ModalDialog( pParent, CUI_RES( RID_CUI_README_DLG ) )
        , maTabCtrl( this, CUI_RES(RID_CUI_README_TBCTL) )
        , maBtnOK( this, CUI_RES(RID_CUI_README_OKBTN) )
        , maReadmeTabPage(0)
        , maLicenseTabPage(0)
        , maNoticeTabPage(0)
    {
        FreeResource();

        maTabCtrl.Show();

        // Notice and License are not localized
        const rtl::OUString sLicense( RTL_CONSTASCII_USTRINGPARAM( BRAND_DIR_SHARE_README LICENSE_FILE ) );
        const rtl::OUString sNotice( RTL_CONSTASCII_USTRINGPARAM(  BRAND_DIR_SHARE_README NOTICE_FILE ) );

        // get localized README
        rtl::OUStringBuffer aBuff;
        lang::Locale aLocale = Application::GetSettings().GetUILocale();
        aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( BRAND_DIR_SHARE_README README_FILE "_" ) );
        aBuff.append( aLocale.Language );
        if ( aLocale.Country.getLength() )
        {
            aBuff.append( sal_Unicode( '-') );
            aBuff.append( aLocale.Country );
            if ( aLocale.Variant.getLength() )
            {
                aBuff.append( sal_Unicode( '-' ) );
                aBuff.append( aLocale.Variant );
            }
        }
#if defined(WNT) || defined(OS2)
        aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( FILE_EXTENSION ) );
#endif

        rtl::OUString sReadmeTxt, sLicenseTxt, sNoticeTxt;
        lcl_readTxtFile( aBuff.makeStringAndClear(), sReadmeTxt );
        lcl_readTxtFile( sLicense, sLicenseTxt );
        lcl_readTxtFile( sNotice, sNoticeTxt );

        maReadmeTabPage = new ReadmeTabPage( &maTabCtrl, sReadmeTxt );
        maLicenseTabPage = new ReadmeTabPage( &maTabCtrl, sLicenseTxt );
        maNoticeTabPage = new ReadmeTabPage( &maTabCtrl, sNoticeTxt );

        maTabCtrl.SetTabPage( RID_CUI_READMEPAGE, maReadmeTabPage );
        maTabCtrl.SetTabPage( RID_CUI_LICENSEPAGE, maLicenseTabPage );
        maTabCtrl.SetTabPage( RID_CUI_NOTICEPAGE, maNoticeTabPage );

        maTabCtrl.SelectTabPage( RID_CUI_READMEPAGE );

        Size aTpSz  = maReadmeTabPage->GetOutputSizePixel();
        Size a6Size = maReadmeTabPage->LogicToPixel( Size( 6, 6 ), MAP_APPFONT );

        maReadmeTabPage->Adjust( aTpSz, a6Size );
        maLicenseTabPage->Adjust( aTpSz, a6Size );
        maNoticeTabPage->Adjust( aTpSz, a6Size );

        Size aDlgSize = GetOutputSizePixel();
        Size aOkBtnSz = maBtnOK.GetSizePixel();
        Point aOKPnt( aDlgSize.Width() / 2 - aOkBtnSz.Width() / 2 , maBtnOK.GetPosPixel().Y() );
        maBtnOK.SetPosPixel( aOKPnt );
    }

    ReadmeDialog::~ReadmeDialog()
    {
        delete maReadmeTabPage;
        delete maLicenseTabPage;
        delete maNoticeTabPage;
    }
}

// -----------------------------------------------------------------------

AboutDialog::AboutDialog( Window* pParent, const ResId& rId ) :
    SfxModalDialog( pParent, rId ),
    maOKButton( this, ResId( RID_CUI_ABOUT_BTN_OK, *rId.GetResMgr() ) ),
    maReadmeButton( this, ResId( RID_CUI_ABOUT_BTN_README, *rId.GetResMgr() ) ),
    maVersionText( this, ResId( RID_CUI_ABOUT_FTXT_VERSION, *rId.GetResMgr() ) ),
    maBuildInfoEdit( this, ResId( RID_CUI_ABOUT_FTXT_BUILDDATA, *rId.GetResMgr() ) ),
    maCopyrightEdit( this, ResId( RID_CUI_ABOUT_FTXT_COPYRIGHT, *rId.GetResMgr() ) ),
    maCreditsLink( this, ResId( RID_CUI_ABOUT_FTXT_WELCOME_LINK, *rId.GetResMgr() )  ),
    maCopyrightTextStr( ResId( RID_CUI_ABOUT_STR_COPYRIGHT, *rId.GetResMgr() ) )
{
    bool bLoad = vcl::ImageRepository::loadBrandingImage(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("about")),
            maAppLogo );
    OSL_ENSURE( bLoad, "Can't load about image");

    bLoad = vcl::ImageRepository::loadBrandingImage(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("logo")),
            maMainLogo );
    OSL_ENSURE( bLoad, "Can't load logo image");

    InitControls();

    // set links
    maReadmeButton.SetClickHdl( LINK( this, AboutDialog, ShowReadme_Impl ) );
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
    Color aWindowColor( rSettings.GetWindowColor() );
    Wallpaper aWall( aWindowColor );
    SetBackground( aWall );
    Font aNewFont( maCopyrightEdit.GetFont() );
    aNewFont.SetTransparent( sal_True );

    maVersionText.SetFont( aNewFont );
    maCopyrightEdit.SetFont( aNewFont );

    maVersionText.SetBackground(aWall);
    maCopyrightEdit.SetBackground(aWall);
    maBuildInfoEdit.SetBackground(aWall);
    maCreditsLink.SetBackground(aWall);

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
    Size aMainLogoSz = maMainLogo.GetSizePixel();
    Size aAppLogoSiz = maAppLogo.GetSizePixel();

    long nLeftOffset = aMainLogoSz.Width();

    aDlgSize = GetOutputSizePixel();

    Size a6Size      = maVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nDlgMargin  = a6Size.Width() * 2;
    long nCtrlMargin = a6Size.Height() * 2;
    long nTextWidth  = aAppLogoSiz.Width() - nDlgMargin;
    long nY          = aAppLogoSiz.Height() + a6Size.Height();

    aDlgSize.Width() = nLeftOffset + a6Size.Width() + aAppLogoSiz.Width();

    Point aPos( nLeftOffset + a6Size.Width(), nY );
    Size aSize;
    // layout fixed text control
    lcl_layoutFixedText( maVersionText, aPos, aSize, nTextWidth );
    nY += aSize.Height() + a6Size.Height();

    // Multiline edit with Build info
    aPos.Y() = nY;
    lcl_layoutEdit( maBuildInfoEdit, aPos, aSize, nTextWidth );
    nY += aSize.Height() + a6Size.Height();

    // Multiline edit with Copyright-Text
    aPos.Y() = nY;
    lcl_layoutEdit( maCopyrightEdit, aPos, aSize, nTextWidth );
    nY += aSize.Height() + a6Size.Height();

    // Hyperlink
    aPos.Y() = nY;
    lcl_layoutFixedText( maCreditsLink, aPos, aSize, nTextWidth );
    nY += aSize.Height();

    nY = std::max( nY, aMainLogoSz.Height() );
    nY += nCtrlMargin;

    // logos position
    maMainLogoPos = Point( 0, nY / 2 - aMainLogoSz.Height() / 2 );
    maAppLogoPos = Point( nLeftOffset + a6Size.Width(), 0 );

    // OK-Button-Position (at the bottom and centered)
    Size aOKSiz = maOKButton.GetSizePixel();
    Point aOKPnt( ( aDlgSize.Width() - aOKSiz.Width() ) - a6Size.Width(), nY );
    maOKButton.SetPosPixel( aOKPnt );

    maReadmeButton.SetPosPixel( Point(a6Size.Width(), nY) );

    aDlgSize.Height() = aOKPnt.Y() + aOKSiz.Height() + a6Size.Width();
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
    DrawImage( maMainLogoPos, maMainLogo );
    DrawImage( maAppLogoPos, maAppLogo );

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
            uno::Reference< uno::XComponentContext > xContext =
                ::comphelper::getProcessComponentContext();
            uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShell(
                xContext->getServiceManager()->createInstanceWithContext(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" ) ),
                    xContext ),
                uno::UNO_QUERY_THROW );
            if ( xSystemShell.is() )
                xSystemShell->execute( sURL, rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    return 0;
}

IMPL_LINK ( AboutDialog, ShowReadme_Impl, PushButton*, EMPTYARG )
{
    ReadmeDialog aDlg( this );
    aDlg.Execute();

    return 0;
}
