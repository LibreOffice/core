/*************************************************************************
 *
 *  $RCSfile: templwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pb $ $Date: 2001-05-08 04:27:46 $
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

#include "templwin.hxx"
#include "templdlg.hxx"
#include "svtdata.hxx"
#include "pathoptions.hxx"

#include "svtools.hrc"
#include "templwin.hrc"
#include "helpid.hrc"

#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDOCUMENTTEMPLATES_HPP_
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XLOCALIZABLE_HPP_
#include <com/sun/star/lang/XLocalizable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif

#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/split.hxx>
#include <vcl/msgbox.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::view;

#define SPLITSET_ID         0
#define COLSET_ID           1
#define ICONWIN_ID          2
#define FILEWIN_ID          3
#define FRAMEWIN_ID         4

// class SvtIconWindow_Impl ----------------------------------------------

SvtIconWindow_Impl::SvtIconWindow_Impl( Window* pParent ) :

    Window( pParent ),

    aIconCtrl( this, WB_3DLOOK | WB_ICON | WB_NOCOLUMNHEADER |
                     WB_HIGHLIGHTFRAME | WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP ),
    nMaxTextLength( 0 )

{
    aIconCtrl.SetStyle( WB_3DLOOK | WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                        WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN );
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    Wallpaper aWallpaper ( rStyles.GetWorkspaceColor() );
    aIconCtrl.SetBackground( aWallpaper );
    aIconCtrl.SetFontColorToBackground ();
    aIconCtrl.SetHelpId( HID_TEMPLATEDLG_ICONCTRL );
    aIconCtrl.Show();

    // detect the root URL of templates
    ::rtl::OUString aService = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DocumentTemplates" ) );
    Reference< XDocumentTemplates > xTemplates(
        ::comphelper::getProcessServiceFactory()->createInstance( aService ), UNO_QUERY );

    if ( xTemplates.is() )
    {
        AllSettings aSettings;
        Locale aLocale = aSettings.GetLocale();

        Reference< XLocalizable > xLocalizable( xTemplates, UNO_QUERY );
        xLocalizable->setLocale( aLocale );

        Reference < XContent > aRootContent = xTemplates->getContent();
        Reference < XCommandEnvironment > aCmdEnv;

        if ( aRootContent.is() )
            aTemplateRootURL = aRootContent->getIdentifier()->getContentIdentifier();
    }

    // insert the categories
    // "New Document"
    String aEntryStr = String( SvtResId( STR_SVT_NEWDOC ) );
    nMaxTextLength = GetTextWidth( aEntryStr );
    SvxIconChoiceCtrlEntry* pEntry =
        aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_NEWDOC ) ) );
    String* pURL = new String();
    pEntry->SetUserData( pURL );
    long nTemp = 0;

    // "Templates"
    if ( aTemplateRootURL.Len() > 0 )
    {
        aEntryStr = String( SvtResId( STR_SVT_TEMPLATES ) );
        nTemp = GetTextWidth( aEntryStr );
        if ( nTemp > nMaxTextLength )
            nMaxTextLength = nTemp;
        pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_TEMPLATES ) ) );
        pURL = new String( aTemplateRootURL );
        pEntry->SetUserData( pURL );
    }

    // "My Documents"
    aEntryStr = String( SvtResId( STR_SVT_MYDOCS ) );
    nTemp = GetTextWidth( aEntryStr );
    if ( nTemp > nMaxTextLength )
        nMaxTextLength = nTemp;
    pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_MYDOCS ) ) );
    pURL = new String( SvtPathOptions().GetWorkPath() );
    pEntry->SetUserData( pURL );

    // "Samples"
    aEntryStr = String( SvtResId( STR_SVT_SAMPLES ) );
    nTemp = GetTextWidth( aEntryStr );
    if ( nTemp > nMaxTextLength )
        nMaxTextLength = nTemp;
    pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_SAMPLES ) ) );
    String aPath( RTL_CONSTASCII_USTRINGPARAM("$(insturl)/share/samples/$(vlang)") );
    pURL = new String( SvtPathOptions().SubstituteVariable( aPath ) );
    pEntry->SetUserData( pURL );
}

SvtIconWindow_Impl::~SvtIconWindow_Impl()
{
    for ( ULONG i = 0; i < aIconCtrl.GetEntryCount(); ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( i );
        delete (String*)pEntry->GetUserData();
    }
}

void SvtIconWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    aIconCtrl.SetSizePixel( aWinSize );
    aIconCtrl.ArrangeIcons();
}

String SvtIconWindow_Impl::GetSelectedIconURL() const
{
    ULONG nPos;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetSelectedEntry( nPos );
    String aURL = *(String*)pEntry->GetUserData();
    return aURL;
}

// class SvtFileViewWindow_Impl -----------------------------------------_

SvtFileViewWindow_Impl::SvtFileViewWindow_Impl( Window* pParent ) :

    Window( pParent ),

    aFileView           ( this, SvtResId( CTRL_FILEVIEW ), sal_False, sal_False ),
    bIsTemplateFolder   ( sal_False )

{
    aFileView.SetHelpId( HID_TEMPLATEDLG_FILEVIEW );
    aFileView.Show();
    aFileView.SetPosPixel( Point( 0, 0 ) );
}

SvtFileViewWindow_Impl::~SvtFileViewWindow_Impl()
{
}

void SvtFileViewWindow_Impl::Resize()
{
    Size aWinSize = GetSizePixel();
    aFileView.SetSizePixel( aWinSize );
}

String SvtFileViewWindow_Impl::GetSelectedFile() const
{
    return aFileView.GetCurrentURL();
}

void SvtFileViewWindow_Impl::OpenFolder( const String& rURL )
{
    bIsTemplateFolder = ( INetURLObject( rURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER );
    aFileView.Initialize( rURL, String() );
    aNewFolderLink.Call( this );
}

sal_Bool SvtFileViewWindow_Impl::HasPreviousLevel( String& rURL ) const
{
    return ( INetURLObject( aFileView.GetViewURL() ) != INetURLObject( aCurrentRootURL ) &&
             aFileView.HasPreviousLevel( rURL ) );
}

// class SvtFrameWindow_Impl ---------------------------------------------

SvtFrameWindow_Impl::SvtFrameWindow_Impl( Window* pParent ) :

    Window( pParent )

{
    pTextWin = new Window( this );
    pTextWin->Show();
    xFrame = ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > (
            ::comphelper::getProcessServiceFactory()->createInstance(
            String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame") ) ), ::com::sun::star::uno::UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( pTextWin ) );
}

SvtFrameWindow_Impl::~SvtFrameWindow_Impl()
{
    xFrame->dispose();
}

void SvtFrameWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    pTextWin->SetSizePixel( aWinSize );
}

void SvtFrameWindow_Impl::OpenFile( const String& rURL, sal_Bool bPreview, sal_Bool bAsTemplate )
{
    if ( rURL.Len() == 0 || !::utl::UCBContentHelper::IsFolder( rURL ) )
    {
        URL aURL;
        aURL.Complete = rURL;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer" ) ) ), UNO_QUERY );
        xTrans->parseStrict( aURL );

        String aTarget;
        if ( bPreview )
            aTarget = String( RTL_CONSTASCII_USTRINGPARAM("_self") );
        else
            aTarget = String( RTL_CONSTASCII_USTRINGPARAM("_blank") );
        Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
        Reference < XDispatch > xDisp = xProv.is() ?
            xProv->queryDispatch( aURL, aTarget, 0 ) : Reference < XDispatch >();

        if ( xDisp.is() )
        {
            if ( bPreview )
            {
                Sequence < PropertyValue > aArgs( 2 );
                aArgs[0].Name = String( RTL_CONSTASCII_USTRINGPARAM("Preview") );
                aArgs[0].Value <<= sal_True;
                aArgs[1].Name = String( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
                aArgs[1].Value <<= sal_True;
                xDisp->dispatch( aURL, aArgs );
            }
            else if ( bAsTemplate )
            {
                Sequence < PropertyValue > aArgs( 1 );
                aArgs[0].Name = String( RTL_CONSTASCII_USTRINGPARAM("AsTemplate") );
                aArgs[0].Value <<= sal_True;
                xDisp->dispatch( aURL, aArgs );
            }
            else
                xDisp->dispatch( aURL, Sequence < PropertyValue >() );
        }
    }
}

// class SvtTemplateWindow -----------------------------------------------

SvtTemplateWindow::SvtTemplateWindow( Window* pParent ) :

    Window( pParent, WB_3DLOOK ),

    aToolBox        ( this, SvtResId( 1001 ) ),
    aSplitWin       ( this, WB_DOCKBORDER )

{
    pIconWin = new SvtIconWindow_Impl( this );
    pIconWin->SetClickHdl( LINK( this, SvtTemplateWindow, IconClickHdl_Impl ) );
    pIconWin->Show();
    pFileWin = new SvtFileViewWindow_Impl( this );
    pFileWin->SetSelectHdl( LINK( this, SvtTemplateWindow, FileSelectHdl_Impl ) );
    pFileWin->SetDoubleClickHdl( LINK( this, SvtTemplateWindow, FileDblClickHdl_Impl ) );
    pFileWin->SetNewFolderHdl( LINK( this, SvtTemplateWindow, NewFolderHdl_Impl ) );
    pFileWin->Show();
    pFrameWin = new SvtFrameWindow_Impl( this );
    pFrameWin->Show();

    aSplitWin.SetAlign( WINDOWALIGN_LEFT );
    long nWidth = ( pIconWin->GetMaxTextLength() * 3 / 2 );
    aSplitWin.InsertItem( ICONWIN_ID, pIconWin, nWidth, SPLITWINDOW_APPEND, 0, SWIB_FIXED );
    aSplitWin.InsertItem( FILEWIN_ID, pFileWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    aSplitWin.InsertItem( FRAMEWIN_ID, pFrameWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );

    SetSizePixel( Size( 1000, 600 ) );

    aResetTimer.SetTimeout( 100 );
    aResetTimer.SetTimeoutHdl( LINK( this, SvtTemplateWindow, ResetHdl_Impl ) );
    aSelectTimer.SetTimeout( 200 );
    aSelectTimer.SetTimeoutHdl( LINK( this, SvtTemplateWindow, TimeoutHdl_Impl ) );

    Size aSize = aToolBox.CalcWindowSizePixel();
    aToolBox.SetPosSizePixel( Point(), aSize );
    aToolBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
    aToolBox.SetClickHdl( LINK( this, SvtTemplateWindow, ClickHdl_Impl ) );
    aToolBox.EnableItem( TI_DOCTEMPLATE_PREV, FALSE );
    aToolBox.Show();

    aSplitWin.Show();
    String aRootURL = pIconWin->GetTemplateRootURL();
    if ( aRootURL.Len() > 0 )
        pFileWin->OpenRoot( aRootURL );
}

SvtTemplateWindow::~SvtTemplateWindow()
{
    delete pIconWin;
    delete pFileWin;
    delete pFrameWin;
}

IMPL_LINK ( SvtTemplateWindow , IconClickHdl_Impl, SvtIconChoiceCtrl *, pCtrl )
{
    String aURL = pIconWin->GetSelectedIconURL();
    pFileWin->OpenRoot( aURL );
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , FileSelectHdl_Impl, SvtFileView *, pView )
{
    aSelectTimer.Start();
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , FileDblClickHdl_Impl, SvtFileView *, pView )
{
    if ( aSelectTimer.IsActive() )
        aSelectTimer.Stop();

    String aURL = pFileWin->GetSelectedFile();
    if ( ::utl::UCBContentHelper::IsFolder( aURL ) )
    {
        pFileWin->OpenFolder( aURL );
        aResetTimer.Start();
    }
    else
    {
        pFrameWin->OpenFile( aURL, sal_False, pFileWin->IsTemplateFolder() );
        aDoubleClickHdl.Call( this );
    }
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , NewFolderHdl_Impl, SvtFileView *, pView )
{
    String aTemp;
    aToolBox.EnableItem( TI_DOCTEMPLATE_PREV, pFileWin->HasPreviousLevel( aTemp ) );
    pFrameWin->OpenFile( String(), sal_True, sal_False );
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , ResetHdl_Impl, Timer *, EMPTYARG )
{
    pFileWin->ResetCursor();
    aSelectHdl.Call( this );
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    aSelectHdl.Call( this );
    String aURL = pFileWin->GetSelectedFile();
    sal_Bool bIsFolder = ( aURL.Len() == 0 || ::utl::UCBContentHelper::IsFolder( aURL ) );
    aToolBox.EnableItem( TI_DOCTEMPLATE_PRINT, !bIsFolder );
    if ( !bIsFolder )
        pFrameWin->OpenFile( aURL, sal_True, sal_False );
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , ClickHdl_Impl, ToolBox *, EMPTYARG )
{
    switch( aToolBox.GetCurItemId() )
    {
        case TI_DOCTEMPLATE_BACK :
            break;

        case TI_DOCTEMPLATE_PREV :
        {
            String aURL;
            if ( pFileWin->HasPreviousLevel( aURL ) )
                pFileWin->OpenFolder( aURL );
            break;
        }

        case TI_DOCTEMPLATE_PRINT :
        {
            PrintFile( pFileWin->GetSelectedFile() );
            break;
        }
    }

    return 0;
}

void SvtTemplateWindow::PrintFile( const String& rURL )
{
    // open the file readonly and hidden
    Sequence < PropertyValue > aArgs( 2 );
    aArgs[0].Name = String( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
    aArgs[0].Value <<= sal_True;
    aArgs[1].Name = String( RTL_CONSTASCII_USTRINGPARAM("Hidden") );
    aArgs[1].Value <<= sal_True;

    Reference < XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );
    Reference < XModel > xModel( xDesktop->loadComponentFromURL( rURL, ::rtl::OUString::createFromAscii("_blank"), 0, aArgs ), UNO_QUERY );
    if ( xModel.is() )
    {
        // print
        Reference < XPrintable > xPrintable( xModel, UNO_QUERY );
        if ( xPrintable.is() )
            xPrintable->print( Sequence < PropertyValue >() );
    }
}

void SvtTemplateWindow::Resize()
{
    long nIconWinWidth = aSplitWin.GetItemSize( ICONWIN_ID );
    nIconWinWidth += ( Splitter( this, 0 ).GetSizePixel().Width() / 2);
    Size aWinSize = GetOutputSizePixel();
    Size aSize = aToolBox.GetSizePixel();
    aSize.Width() = aWinSize.Width() - nIconWinWidth;
    aToolBox.SetSizePixel( aSize );
    Point aPos = aToolBox.GetPosPixel();
    aPos.X() = nIconWinWidth;
    aToolBox.SetPosPixel( aPos );

    long nToolBoxHeight = aSize.Height();
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    aSplitWin.SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}

sal_Bool SvtTemplateWindow::IsFileSelected() const
{
    String aURL = pFileWin->GetSelectedFile();
    sal_Bool bRet = ( aURL.Len() > 0 && !::utl::UCBContentHelper::IsFolder( aURL ) );
    return bRet;
}

// class SvtDocumentTemplateDialog ---------------------------------------

SvtDocumentTemplateDialog::SvtDocumentTemplateDialog( Window* pParent ) :

    ModalDialog( pParent, SvtResId( DLG_DOCTEMPLATE ) ),

    aLine       ( this, ResId( FL_DOCTEMPLATE ) ),
    aManageBtn  ( this, ResId( BTN_DOCTEMPLATE_MANAGE ) ),
    aEditBtn    ( this, ResId( BTN_DOCTEMPLATE_EDIT ) ),
    aOKBtn      ( this, ResId( BTN_DOCTEMPLATE_OPEN ) ),
    aCancelBtn  ( this, ResId( BTN_DOCTEMPLATE_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_DOCTEMPLATE_HELP ) )

{
    FreeResource();

    pWin = new SvtTemplateWindow( this );
    Size aSize = GetOutputSizePixel();
    Point aPos = aLine.GetPosPixel();
    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    aSize.Height() = aPos.Y() - a6Size.Height();

    pWin->SetSelectHdl( LINK( this, SvtDocumentTemplateDialog, SelectHdl_Impl ) );
    pWin->SetDoubleClickHdl( LINK( this, SvtDocumentTemplateDialog, DoubleClickHdl_Impl ) );

    pWin->SetPosSizePixel( Point( 0, 0 ), aSize );
    pWin->Show();

    SelectHdl_Impl( NULL );
}

SvtDocumentTemplateDialog::~SvtDocumentTemplateDialog()
{
    delete pWin;
}

IMPL_LINK ( SvtDocumentTemplateDialog , SelectHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    sal_Bool bEnable = pWin->IsFileSelected();
    aEditBtn.Enable( bEnable );
    aOKBtn.Enable( bEnable );
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , DoubleClickHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    EndDialog( RET_OK );
    return 0;
}

