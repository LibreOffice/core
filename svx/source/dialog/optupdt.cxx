/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optupdt.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-21 17:07:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

#include "optupdt.hxx"
#include "optupdt.hrc"

#include "dialmgr.hxx"
#include "dialogs.hrc"


#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef  _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif

#ifndef  _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#ifndef  _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#include <osl/file.hxx>
#include <osl/security.hxx>

namespace beans = ::com::sun::star::beans;
namespace container = ::com::sun::star::container;
namespace dialogs = ::com::sun::star::ui::dialogs;
namespace frame = ::com::sun::star::frame;
namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;
namespace util = ::com::sun::star::util;


// define ----------------------------------------------------------------

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

// class SvxOnlineUpdateTabPage --------------------------------------------------

SvxOnlineUpdateTabPage::SvxOnlineUpdateTabPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_ONLINEUPDATE ), rSet ),
        m_aOptionsLine( this, ResId( FL_OPTIONS ) ),
        m_aAutoCheckCheckBox( this, ResId( CB_AUTOCHECK ) ),
        m_aEveryDayButton( this, ResId( RB_EVERYDAY ) ),
        m_aEveryWeekButton( this, ResId( RB_EVERYWEEK ) ),
        m_aEveryMonthButton( this, ResId( RB_EVERYMONTH ) ),
        m_aCheckNowButton( this, ResId( PB_CHECKNOW ) ),
        m_aAutoDownloadCheckBox( this, ResId( CB_AUTODOWNLOAD ) ),
        m_aDestPathLabel( this, ResId( FT_DESTPATHLABEL ) ),
        m_aDestPath( this, ResId( FT_DESTPATH ) ),
        m_aChangePathButton( this, ResId( PB_CHANGEPATH ) )
{
    FreeResource();

    m_aAutoCheckCheckBox.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, AutoCheckHdl_Impl ) );
    m_aCheckNowButton.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, CheckNowHdl_Impl ) );
    m_aAutoDownloadCheckBox.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, AutoDownloadHdl_Impl ) );
    m_aChangePathButton.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, FileDialogHdl_Impl ) );

    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    m_xUpdateAccess = uno::Reference < container::XNameReplace >(
        xFactory->createInstance( UNISTRING( "com.sun.star.setup.UpdateCheckConfig" ) ),
        uno::UNO_QUERY_THROW );

    sal_Bool bDownloadSupported = sal_Bool();
    m_xUpdateAccess->getByName( UNISTRING( "DownloadSupported") ) >>= bDownloadSupported;

    WinBits nStyle = m_aDestPath.GetStyle();
    nStyle |= WB_PATHELLIPSIS;
    m_aDestPath.SetStyle(nStyle);

    m_aAutoDownloadCheckBox.Show(bDownloadSupported);
    m_aDestPathLabel.Show(bDownloadSupported);
    m_aDestPath.Show(bDownloadSupported);
    m_aChangePathButton.Show(bDownloadSupported);
}

// -----------------------------------------------------------------------

SvxOnlineUpdateTabPage::~SvxOnlineUpdateTabPage()
{
}

// -----------------------------------------------------------------------

SfxTabPage*
SvxOnlineUpdateTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxOnlineUpdateTabPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------


BOOL SvxOnlineUpdateTabPage::FillItemSet( SfxItemSet& )
{
    BOOL bModified = FALSE;

    sal_Bool bValue;
    sal_Int64 nValue;

    if( m_aAutoCheckCheckBox.GetSavedValue() != m_aAutoCheckCheckBox.IsChecked() )
    {
        bValue = (TRUE == m_aAutoCheckCheckBox.IsChecked());
        m_xUpdateAccess->replaceByName( UNISTRING("AutoCheckEnabled"), uno::makeAny( bValue ) );
        bModified = TRUE;
    }

    nValue = 0;
    if( TRUE == m_aEveryDayButton.IsChecked() )
    {
        if( FALSE == m_aEveryDayButton.GetSavedValue() )
            nValue = 86400;
    }
    else if( TRUE == m_aEveryWeekButton.IsChecked() )
    {
        if( FALSE == m_aEveryWeekButton.GetSavedValue() )
            nValue = 604800;
    }
    else if( TRUE == m_aEveryMonthButton.IsChecked() )
    {
        if( FALSE == m_aEveryMonthButton.GetSavedValue() )
            nValue = 2592000;
    }

    if( nValue > 0 )
    {
        m_xUpdateAccess->replaceByName( UNISTRING("CheckInterval"), uno::makeAny( nValue ) );
        bModified = TRUE;
    }

    if( m_aAutoDownloadCheckBox.GetSavedValue() != m_aAutoDownloadCheckBox.IsChecked() )
    {
        bValue = (TRUE == m_aAutoDownloadCheckBox.IsChecked());
        m_xUpdateAccess->replaceByName( UNISTRING("AutoDownloadEnabled"), uno::makeAny( bValue ) );
        bModified = TRUE;
    }

    rtl::OUString sValue, aURL;
    m_xUpdateAccess->getByName( UNISTRING("DownloadDestination") ) >>= sValue;

    if( ( osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(m_aDestPath.GetText(), aURL) ) &&
        ( ! aURL.equals( sValue ) ) )
    {
        m_xUpdateAccess->replaceByName( UNISTRING("DownloadDestination"), uno::makeAny( aURL ) );
        bModified = TRUE;
    }

    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
        xChangesBatch->commitChanges();

    return bModified;
}

// -----------------------------------------------------------------------

void SvxOnlineUpdateTabPage::Reset( const SfxItemSet& )
{
    sal_Bool bValue = sal_Bool();
    m_xUpdateAccess->getByName( UNISTRING("AutoCheckEnabled") ) >>= bValue;

    m_aAutoCheckCheckBox.Check(bValue);
    m_aEveryDayButton.Enable(bValue);
    m_aEveryWeekButton.Enable(bValue);
    m_aEveryMonthButton.Enable(bValue);

    sal_Int64 nValue = 0;
    m_xUpdateAccess->getByName( UNISTRING("CheckInterval") ) >>= nValue;

    if( nValue == 86400 )
        m_aEveryDayButton.Check();
    else if( nValue == 604800 )
        m_aEveryWeekButton.Check();
    else
        m_aEveryMonthButton.Check();

    m_aAutoCheckCheckBox.SaveValue();
    m_aEveryDayButton.SaveValue();
    m_aEveryWeekButton.SaveValue();
    m_aEveryMonthButton.SaveValue();

    m_xUpdateAccess->getByName( UNISTRING("AutoDownloadEnabled") ) >>= bValue;
    m_aAutoDownloadCheckBox.Check(bValue);
    m_aDestPathLabel.Enable(bValue);
    m_aDestPath.Enable(bValue);
    m_aChangePathButton.Enable(bValue);

    rtl::OUString sValue, aPath;
    m_xUpdateAccess->getByName( UNISTRING("DownloadDestination") ) >>= sValue;

    if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sValue, aPath) )
        m_aDestPath.SetText(aPath);

    m_aAutoDownloadCheckBox.SaveValue();
}

// -----------------------------------------------------------------------

void SvxOnlineUpdateTabPage::FillUserData()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxOnlineUpdateTabPage, AutoCheckHdl_Impl, CheckBox *, pBox )
{
    BOOL bEnabled = pBox->IsChecked();

    m_aEveryDayButton.Enable(bEnabled);
    m_aEveryWeekButton.Enable(bEnabled);
    m_aEveryMonthButton.Enable(bEnabled);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxOnlineUpdateTabPage, AutoDownloadHdl_Impl, CheckBox *, pBox )
{
    BOOL bEnabled = pBox->IsChecked();

    m_aDestPathLabel.Enable(bEnabled);
    m_aDestPath.Enable(bEnabled);
    m_aChangePathButton.Enable(bEnabled);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxOnlineUpdateTabPage, FileDialogHdl_Impl, PushButton *, EMPTYARG )
{
    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference < dialogs::XFolderPicker > xFolderPicker(
        xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) ) ),
        uno::UNO_QUERY );

    rtl::OUString aURL;
    if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath(m_aDestPath.GetText(), aURL) )
        osl::Security().getHomeDir(aURL);

    xFolderPicker->setDisplayDirectory( aURL );
    sal_Int16 nRet = xFolderPicker->execute();

    if ( dialogs::ExecutableDialogResults::OK == nRet )
    {
        rtl::OUString aFolder;
        if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(xFolderPicker->getDirectory(), aFolder))
            m_aDestPath.SetText( aFolder );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxOnlineUpdateTabPage, CheckNowHdl_Impl, PushButton *, EMPTYARG )
{
    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            xFactory->createInstance( UNISTRING( "com.sun.star.configuration.ConfigurationProvider" )),
            uno::UNO_QUERY_THROW);

        beans::PropertyValue aProperty;
        aProperty.Name  = UNISTRING( "nodepath" );
        aProperty.Value = uno::makeAny( UNISTRING("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] = uno::makeAny( aProperty );

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                UNISTRING("com.sun.star.configuration.ConfigurationAccess"), aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName(UNISTRING("URL")) >>= aURL.Complete;

        uno::Reference < util::XURLTransformer > xTransformer(
            xFactory->createInstance(  UNISTRING( "com.sun.star.util.URLTransformer" ) ),
            uno::UNO_QUERY_THROW );

        xTransformer->parseStrict(aURL);

        uno::Reference < frame::XDesktop > xDesktop(
            xFactory->createInstance(  UNISTRING( "com.sun.star.frame.Desktop" ) ),
            uno::UNO_QUERY_THROW );

        uno::Reference< frame::XDispatchProvider > xDispatchProvider(
            xDesktop->getCurrentFrame(), uno::UNO_QUERY );

        uno::Reference< frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, rtl::OUString(), 0);

        if( xDispatch.is() )
            xDispatch->dispatch(aURL, uno::Sequence< beans::PropertyValue > ());

    }
    catch( const uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return 0;
}

