/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExtensionNotPresent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 08:11:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_dbaccess.hxx"

#include "ExtensionNotPresent.hxx"
#include "ExtensionNotPresent.hrc"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "dbu_resource.hrc"
#include "moduledbu.hxx"
#include <vcl/msgbox.hxx>
#include <svx/globlmn.hrc>
#include <svx/svxids.hrc>
#include <connectivity/dbconversion.hxx>
#include <svtools/syslocale.hxx>
#include "UITools.hxx"

#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <unotools/confignode.hxx>
// #include <toolkit/unohlp.hxx>

// #include "uistrings.hrc"

namespace dbaui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

DBG_NAME( dbu_OExtensionNotPresentDialog )
//========================================================================
// class OExtensionNotPresentDialog
//========================================================================
    OExtensionNotPresentDialog::OExtensionNotPresentDialog( Window* _pParent, uno::Reference< lang::XMultiServiceFactory > _xORB)
            : ModalDialog( _pParent, ModuleRes(RID_EXTENSION_NOT_PRESENT_DLG) )
              ,m_aFI_WARNING(this, ModuleRes(FI_WARNING))
              ,m_aFT_TEXT(this, ModuleRes(FT_TEXT       ) )
              ,m_aPB_DOWNLOAD(this,     ModuleRes(PB_DOWNLOAD))
              ,m_aPB_CANCEL(this,   ModuleRes(PB_CANCEL))
            ,m_xMultiServiceFactory(_xORB)
{
    DBG_CTOR( dbu_OExtensionNotPresentDialog, NULL);

    try
    {
        SvtSysLocale aSysLocale;
        m_nLocale = aSysLocale.GetLocaleData().getLocale();
    }
    catch(uno::Exception&)
    {
    }

    // set a ClickHandler for the 'Download' button
    m_aPB_DOWNLOAD.SetClickHdl( LINK( this, OExtensionNotPresentDialog, Download_Click ) );

    // get message string out of the resource
    String sText =          String( ModuleRes( RID_STR_EXTENSION_NOT_PRESENT ) );
    // String sExtensionName = String( ModuleRes( RID_STR_EXTENSION_NAME ) );
    String sExtensionName = getFromConfigurationExtension("Name");
    sText.SearchAndReplaceAscii("%RPT_EXTENSION_NAME", sExtensionName);

    m_aFT_TEXT.SetText(sText);

    // calulate the size of the text field
    Rectangle aPrimaryRect( Point(0,0), m_aFT_TEXT.GetSizePixel() );
    Rectangle aSuggestedRect( GetTextRect( aPrimaryRect, sText, TEXT_DRAW_MULTILINE | TEXT_DRAW_LEFT ) );

    Size aTempSize = LogicToPixel( Size(LEFT_PADDING + RIGHT_PADDING, 1), MAP_APPFONT); // real pixel size of LEFT and RIGHT_PADDING
    sal_Int32 nWidthWithoutFixedText = aTempSize.getWidth() ;
    sal_Int32 nHeightWithoutFixedText = GetSizePixel().getHeight() - m_aFT_TEXT.GetSizePixel().getHeight();

    Size aNewSize = aSuggestedRect.GetSize();
    m_aFT_TEXT.SetSizePixel( aNewSize );
    sal_Int32 nNewWidth  = nWidthWithoutFixedText  + aSuggestedRect.GetWidth();
    sal_Int32 nNewHeight = nHeightWithoutFixedText + aSuggestedRect.GetHeight();

    // set new window width & height
    Size aDialogWindowSize = GetSizePixel();
    aDialogWindowSize.setWidth(  nNewWidth );
    aDialogWindowSize.setHeight( nNewHeight );
    SetSizePixel(aDialogWindowSize);

    // move Action items
    sal_Int32 nWindowWidth = GetSizePixel().getWidth();
    sal_Int32 nWindowHeight = GetSizePixel().getHeight();

    Size aButtonSize = LogicToPixel( Size(BUTTON_WIDTH, BUTTON_HEIGHT), MAP_APPFONT); // real pixel size of a button

    Point aNewPos = m_aPB_DOWNLOAD.GetPosPixel();
    aNewPos.setX(nWindowWidth / 2 - m_aPB_DOWNLOAD.GetSizePixel().getWidth() - 8);
    aNewPos.setY(nWindowHeight - aButtonSize.getHeight() - 5);
    m_aPB_DOWNLOAD.SetPosPixel(aNewPos );

    aNewPos = m_aPB_CANCEL.GetPosPixel();
    aNewPos.setX(nWindowWidth / 2 + 8);
    aNewPos.setY(nWindowHeight - aButtonSize.getHeight() - 5);
    m_aPB_CANCEL.SetPosPixel(aNewPos );

    m_aFI_WARNING.SetImage(WarningBox::GetStandardImage());

    // set an image in high contrast
    // m_aFI_WARNING.SetModeImage(Image(BMP_EXCEPTION_WARNING_SCH), BMP_COLOR_HIGHCONTRAST);

    // to resize images
    // WinBits aBits = m_aFI_WARNING.GetStyle();
    // aBits |= WB_SCALE;
    // m_aFI_WARNING.SetStyle(aBits);
    //
    // Size aImageSize = m_aFI_WARNING.GetSizePixel();
    // (void) aImageSize;
    // m_aFI_WARNING.Resize();


    Resize();
    FreeResource();
}

//------------------------------------------------------------------------
OExtensionNotPresentDialog::~OExtensionNotPresentDialog()
{
    DBG_DTOR( dbu_OExtensionNotPresentDialog, NULL);
}
// -----------------------------------------------------------------------------
short OExtensionNotPresentDialog::Execute()
{
    DBG_CHKTHIS( dbu_OExtensionNotPresentDialog,NULL);
    short nRet = ModalDialog::Execute();
    // RET_OK
    // RET_NO
    return nRet;
}

//------------------------------------------------------------------------------

uno::Reference< com::sun::star::system::XSystemShellExecute > OExtensionNotPresentDialog::getShellExecuter() const
{
    uno::Reference<com::sun::star::system::XSystemShellExecute> xExecuter( m_xMultiServiceFactory->createInstance( UNISTRING( "com.sun.star.system.SystemShellExecute" )), uno::UNO_QUERY_THROW);
    return xExecuter;
}
// -----------------------------------------------------------------------------
rtl::OUString OExtensionNotPresentDialog::getFromConfigurationExtension(rtl::OUString const& _sPropertyName) const
{
    // get the URL to open in a browser from Configuration
    static const ::rtl::OUString sConfigName( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.ReportDesign/Extension" ) );

    ::utl::OConfigurationTreeRoot aConfiguration( ::utl::OConfigurationTreeRoot::createWithServiceFactory( m_xMultiServiceFactory, sConfigName ) );

    rtl::OUString aValue;
    aConfiguration.getNodeValue( _sPropertyName ) >>= aValue;
    return aValue;
}

// -----------------------------------------------------------------------------
rtl::OUString OExtensionNotPresentDialog::getFromConfigurationExtension(rtl::OString const& _sPropertyName) const
{
    return getFromConfigurationExtension(rtl::OStringToOUString( _sPropertyName, RTL_TEXTENCODING_UTF8) );
}

// -----------------------------------------------------------------------------
// handle the click on the download button
IMPL_LINK( OExtensionNotPresentDialog, Download_Click, PushButton*, EMPTYARG )
{
    try
    {
        EndDialog( TRUE );

        rtl::OUString suDownloadURL = getFromConfigurationExtension("DownloadURL");
        if (suDownloadURL.getLength() == 0)
        {
            // fallback
            suDownloadURL = UNISTRING("http://extensions.services.openoffice.org");
        }

        // open such URL in a browser
        uno::Reference< com::sun::star::system::XSystemShellExecute > xShellExecute( getShellExecuter() );
        xShellExecute->execute( suDownloadURL, ::rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
    }
    catch(uno::Exception)
    {
        OSL_ENSURE(0,"Exception caught!");
    }
    return 0;
}

// =============================================================================
} // rptui
// =============================================================================

