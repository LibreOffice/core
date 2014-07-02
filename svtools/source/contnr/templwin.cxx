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

#include <config_folders.h>

#include "templwin.hxx"
#include <svtools/templdlg.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/langhelp.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <vcl/xtextedt.hxx>
#include <svl/inettype.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/templatefoldercache.hxx>
#include <svtools/imgdef.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/settings.hxx>
#include <svtools/svtools.hrc>
#include "templwin.hrc"
#include <svtools/helpid.hrc>
#include <unotools/viewoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <org/freedesktop/PackageKit/SyncDbusSessionHelper.hpp>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <vcl/waitobj.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <vcl/svapp.hxx>
#include <vcl/split.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/DocumentInfoPreview.hxx>
#include <vcl/mnemonic.hxx>

#include <ucbhelper/content.hxx>
#include <comphelper/string.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::view;
using namespace svtools;

#define aSeparatorStr   "----------------------------------"

#define ICONWIN_ID          2
#define FILEWIN_ID          3
#define FRAMEWIN_ID         4

#define ICON_POS_NEWDOC     0
#define ICON_POS_TEMPLATES  1
#define ICON_POS_MYDOCS     2
#define ICON_POS_SAMPLES    3

#define VIEWSETTING_NEWFROMTEMPLATE     "NewFromTemplate"
#define VIEWSETTING_SELECTEDGROUP       "SelectedGroup"
#define VIEWSETTING_SELECTEDVIEW        "SelectedView"
#define VIEWSETTING_SPLITRATIO          "SplitRatio"
#define VIEWSETTING_LASTFOLDER          "LastFolder"

struct FolderHistory
{
    OUString        m_sURL;
    sal_uLong       m_nGroup;

    FolderHistory( const OUString& _rURL, sal_Int32 _nGroup ) :
        m_sURL( _rURL ), m_nGroup( _nGroup ) {}
};

typedef ::std::vector< OUString > NewDocList_Impl;

// class SvtDummyHeaderBar_Impl ------------------------------------------

void SvtDummyHeaderBar_Impl::UpdateBackgroundColor()
{
    SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetWindowColor() ) );
}

SvtDummyHeaderBar_Impl::SvtDummyHeaderBar_Impl( Window* pPar ) : Window( pPar )
{
    SetSizePixel( HeaderBar( this, 0 ).CalcWindowSizePixel() ); // HeaderBar used only to calculate size

    UpdateBackgroundColor();
}

SvtDummyHeaderBar_Impl::~SvtDummyHeaderBar_Impl()
{
}

void SvtDummyHeaderBar_Impl::DataChanged( const DataChangedEvent& r )
{
    Window::DataChanged( r );
    if( r.GetType() == DATACHANGED_SETTINGS )
        UpdateBackgroundColor();
}

// class SvtIconWindow_Impl ----------------------------------------------

SvtIconWindow_Impl::SvtIconWindow_Impl( Window* pParent ) :

    Window( pParent, WB_DIALOGCONTROL | WB_BORDER | WB_3DLOOK ),

    aDummyHeaderBar( this ),
    aIconCtrl( this, WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /*!WB_NOSELECTION |*/
                     WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN ),
    aNewDocumentRootURL( "private:newdoc" ),
    aMyDocumentsRootURL( SvtPathOptions().GetWorkPath() ),
    aSamplesFolderRootURL( SvtPathOptions().
        SubstituteVariable( "$(insturl)/" LIBO_SHARE_FOLDER "/samples/$(vlang)" ) ),
    nMaxTextLength( 0 )

{
    aDummyHeaderBar.Show();

    aIconCtrl.SetAccessibleName( OUString( "Groups" ) );
    aIconCtrl.SetHelpId( HID_TEMPLATEDLG_ICONCTRL );
    aIconCtrl.SetChoiceWithCursor( true );
    aIconCtrl.SetSelectionMode( SINGLE_SELECTION );
    aIconCtrl.Show();

    // detect the root URL of templates
    Reference< XDocumentTemplates > xTemplates( frame::DocumentTemplates::create(::comphelper::getProcessComponentContext()) );

    Reference < XContent > aRootContent = xTemplates->getContent();
    Reference < XCommandEnvironment > aCmdEnv;

    if ( aRootContent.is() )
        aTemplateRootURL = aRootContent->getIdentifier()->getContentIdentifier();

    // insert the categories
    // "New Document"
    Image aImage( SvtResId( IMG_SVT_NEWDOC ) );
    nMaxTextLength = aImage.GetSizePixel().Width();
    OUString aEntryStr = SVT_RESSTR(STR_SVT_NEWDOC);
    SvxIconChoiceCtrlEntry* pEntry =
        aIconCtrl.InsertEntry( aEntryStr, aImage, ICON_POS_NEWDOC );
    pEntry->SetUserData( new OUString(aNewDocumentRootURL) );
    pEntry->SetQuickHelpText( SVT_RESSTR(STR_SVT_NEWDOC_HELP) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    long nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    // "Templates"
    if( !aTemplateRootURL.isEmpty() )
    {
        aEntryStr = SVT_RESSTR(STR_SVT_TEMPLATES);
        pEntry = aIconCtrl.InsertEntry(
            aEntryStr, Image( SvtResId( IMG_SVT_TEMPLATES ) ), ICON_POS_TEMPLATES );
        pEntry->SetUserData( new OUString(aTemplateRootURL) );
        pEntry->SetQuickHelpText(SVT_RESSTR(STR_SVT_TEMPLATES_HELP));
        DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
        nTemp = pEntry->GetBoundRect().GetSize().Width();
        if (nTemp > nMaxTextLength)
            nMaxTextLength = nTemp;
    }

    // "My Documents"
    aEntryStr = SVT_RESSTR(STR_SVT_MYDOCS);
    pEntry = aIconCtrl.InsertEntry(
        aEntryStr, Image( SvtResId( IMG_SVT_MYDOCS ) ), ICON_POS_MYDOCS );
    pEntry->SetUserData( new OUString(aMyDocumentsRootURL) );
    pEntry->SetQuickHelpText( SVT_RESSTR(STR_SVT_MYDOCS_HELP) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if( nTemp > nMaxTextLength )
        nMaxTextLength = nTemp;

    // "Samples"
    aEntryStr = SVT_RESSTR(STR_SVT_SAMPLES);
    pEntry = aIconCtrl.InsertEntry(
        aEntryStr, Image( SvtResId( IMG_SVT_SAMPLES ) ), ICON_POS_SAMPLES );
    pEntry->SetUserData( new OUString(aSamplesFolderRootURL) );
    pEntry->SetQuickHelpText( SVT_RESSTR(STR_SVT_SAMPLES_HELP));
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    aIconCtrl.CreateAutoMnemonics();
}

SvtIconWindow_Impl::~SvtIconWindow_Impl()
{
    for ( sal_uLong i = 0; i < aIconCtrl.GetEntryCount(); ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( i );
        delete (OUString*)pEntry->GetUserData();
    }
}

SvxIconChoiceCtrlEntry* SvtIconWindow_Impl::GetEntry( const OUString& rURL ) const
{
    SvxIconChoiceCtrlEntry* pEntry = NULL;
    for ( sal_uLong i = 0; i < aIconCtrl.GetEntryCount(); ++i )
    {
        SvxIconChoiceCtrlEntry* pTemp = aIconCtrl.GetEntry( i );
        OUString *pURL = (OUString*)pTemp->GetUserData();
        if ( (*pURL) == rURL )
        {
            pEntry = pTemp;
            break;
        }
    }

    return pEntry;
}

void SvtIconWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    Size aHeaderSize = aDummyHeaderBar.GetSizePixel();
    aHeaderSize.Width() = aWinSize.Width();
    aDummyHeaderBar.SetSizePixel( aHeaderSize );
    long nHeaderHeight = aHeaderSize.Height();
    aWinSize.Height() -= nHeaderHeight;
    aIconCtrl.SetPosSizePixel( Point( 0, nHeaderHeight ), aWinSize );
    aIconCtrl.ArrangeIcons();
}

OUString SvtIconWindow_Impl::GetCursorPosIconURL() const
{
    OUString aURL;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetCursor( );
    if ( pEntry )
        aURL = *static_cast<OUString*>(pEntry->GetUserData());
    return aURL;

}

OUString SvtIconWindow_Impl::GetSelectedIconURL() const
{
    sal_uLong nPos;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetSelectedEntry( nPos );
    OUString aURL;
    if ( pEntry )
        aURL = *static_cast<OUString*>(pEntry->GetUserData());
    return aURL;
}

OUString SvtIconWindow_Impl::GetIconText( const OUString& rURL ) const
{
    OUString aText;
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( rURL );
    if ( pEntry )
        aText = MnemonicGenerator::EraseAllMnemonicChars( pEntry->GetText() );
    return aText;
}

void SvtIconWindow_Impl::InvalidateIconControl()
{
    aIconCtrl.Invalidate();
}

sal_uLong SvtIconWindow_Impl::GetCursorPos() const
{
    sal_uLong nPos = ~sal_uLong(0);

    SvxIconChoiceCtrlEntry* pCursorEntry = aIconCtrl.GetCursor( );
    if ( pCursorEntry )
        nPos = aIconCtrl.GetEntryListPos( pCursorEntry );

    return nPos;
}

sal_uLong SvtIconWindow_Impl::GetSelectEntryPos() const
{
    sal_uLong nPos;
    if ( !aIconCtrl.GetSelectedEntry( nPos ) )
        nPos = ~sal_uLong(0);
    return nPos;
}

void SvtIconWindow_Impl::SetCursorPos( sal_uLong nPos )
{
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( nPos );
    aIconCtrl.SetCursor( pEntry );
    aIconCtrl.Invalidate();
    aIconCtrl.Update();
}

void SvtIconWindow_Impl::SetFocus()
{
    aIconCtrl.GrabFocus();
}

long SvtIconWindow_Impl::CalcHeight() const
{
    // calculate the required height of the IconControl
    long nHeight = 0;
    sal_uLong nCount = aIconCtrl.GetEntryCount();
    if ( nCount > 0 )
        // bottom of the last icon
        nHeight = aIconCtrl.GetEntry(nCount-1)->GetBoundRect().Bottom();

    // + headerbar height
    nHeight += aDummyHeaderBar.GetSizePixel().Height();

    return nHeight;
}

bool SvtIconWindow_Impl::IsRootURL( const OUString& rURL ) const
{
    return  rURL == aNewDocumentRootURL ||
            rURL == aTemplateRootURL ||
            rURL == aMyDocumentsRootURL ||
            rURL == aSamplesFolderRootURL;
}

sal_uLong SvtIconWindow_Impl::GetRootPos( const OUString& rURL ) const
{
    sal_uLong nPos = ~sal_uLong(0);
    if ( rURL.startsWith(aNewDocumentRootURL) )
        nPos = 0;
    else if ( rURL.startsWith(aTemplateRootURL) )
        nPos = 1;
    else if ( rURL.startsWith(aMyDocumentsRootURL) )
        nPos = 2;
    else if ( rURL.startsWith(aSamplesFolderRootURL) )
        nPos = 3;
    else if ( aMyDocumentsRootURL.startsWith(rURL) )
        nPos = 2;
    else
    {
        DBG_WARNING( "SvtIconWindow_Impl::GetRootPos(): invalid position" );
        nPos = 2;
    }

    return nPos;
}

void SvtIconWindow_Impl::UpdateIcons()
{
    aIconCtrl.GetEntry( ICON_POS_NEWDOC )->SetImage(
        Image( SvtResId( IMG_SVT_NEWDOC ) ) );
    aIconCtrl.GetEntry( ICON_POS_TEMPLATES )->SetImage(
        Image( SvtResId( IMG_SVT_TEMPLATES ) ) );
    aIconCtrl.GetEntry( ICON_POS_MYDOCS )->SetImage(
        Image( SvtResId( IMG_SVT_MYDOCS ) ) );
    aIconCtrl.GetEntry( ICON_POS_SAMPLES )->SetImage(
        Image( SvtResId( IMG_SVT_SAMPLES ) ) );
}

void SvtIconWindow_Impl::SelectFolder(sal_Int32 nFolderPosition)
{
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( nFolderPosition );
    if(pEntry)
    {
        aIconCtrl.SetCursor( pEntry );
        aIconCtrl.GetClickHdl().Call(&aIconCtrl);
    }
}

// class SvtFileViewWindow_Impl -----------------------------------------_

SvtFileViewWindow_Impl::SvtFileViewWindow_Impl( SvtTemplateWindow* pParent ) :

    Window( pParent, WB_DIALOGCONTROL | WB_TABSTOP | WB_BORDER | WB_3DLOOK ),

    rParent             ( *pParent ),
    aFileView           ( this, WB_TABSTOP | WB_3DLOOK, FILEVIEW_SHOW_NONE | FILEVIEW_SHOW_ONLYTITLE ),
    bIsTemplateFolder   ( false )

{
    aFileView.SetStyle( aFileView.GetStyle() | WB_DIALOGCONTROL | WB_TABSTOP );
    aFileView.SetHelpId( HID_TEMPLATEDLG_FILEVIEW );
    aFileView.Show();
    aFileView.EnableAutoResize();
    aFileView.EnableContextMenu( false );
    aFileView.EnableDelete( false );
}

SvtFileViewWindow_Impl::~SvtFileViewWindow_Impl()
{
}

void GetMenuEntry_Impl
(
    Sequence< PropertyValue >& aDynamicMenuEntry,
    OUString& rTitle,
    OUString& rURL,
    OUString& rFrame,
    OUString& rImageId
)
{
    for ( int i = 0; i < aDynamicMenuEntry.getLength(); i++ )
    {
        if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_URL )
            aDynamicMenuEntry[i].Value >>= rURL;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TITLE )
            aDynamicMenuEntry[i].Value >>= rTitle;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER )
            aDynamicMenuEntry[i].Value >>= rImageId;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TARGETNAME )
            aDynamicMenuEntry[i].Value >>= rFrame;
    }
}

Sequence< OUString > SvtFileViewWindow_Impl::GetNewDocContents() const
{
    NewDocList_Impl aNewDocs;
    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;
    aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );

    OUString aTitle;
    OUString aURL;
    OUString aImageURL;
    OUString aTargetFrame;

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    OUString sSeparator( "private:separator" );
    OUString sSlotURL( "slot:5500" );

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry_Impl( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageURL );
        if( aURL == sSlotURL )
            continue;
        if( aURL == sSeparator )
        {
            OUString aSeparator( aSeparatorStr );
            aNewDocs.push_back( aSeparator );
        }
        else
        {
            // title
            OUString aRow = MnemonicGenerator::EraseAllMnemonicChars( aTitle );
            aRow += "\t";
            // no type
            aRow += "\t";
            // no size
            aRow += "\t";
            // no date
            aRow += "\t";
            // url
            aRow += aURL;
            aRow += "\t";
            // folder == false
            aRow += "0";
            // image url?
            if ( !aImageURL.isEmpty() )
            {
                aRow += "\t";
                aRow += aImageURL;
            }

            aNewDocs.push_back( aRow );
        }
    }

    nCount = aNewDocs.size();
    Sequence < OUString > aRet( nCount );
    OUString* pRet = aRet.getArray();
    for ( i = 0; i < nCount; ++i )
    {
        pRet[i] = aNewDocs[i];
    }

    return aRet;
}

void SvtFileViewWindow_Impl::Resize()
{
    aFileView.SetSizePixel(GetOutputSizePixel());
}

OUString SvtFileViewWindow_Impl::GetSelectedFile() const
{
    return aFileView.GetCurrentURL();
}

void SvtFileViewWindow_Impl::OpenFolder( const OUString& rURL )
{
    aFolderURL = rURL;

    rParent.SetPrevLevelButtonState( rURL );

    if ( INetURLObject( rURL ).GetProtocol() == INET_PROT_PRIVATE )
    {
        aFileView.EnableNameReplacing( false );
        aFileView.Initialize( GetNewDocContents() );
    }
    else
    {
        sal_Int32 nSampFoldLen = aSamplesFolderURL.getLength();
        aFileView.EnableNameReplacing(
                    nSampFoldLen && rURL.startsWith( aSamplesFolderURL ) );
        aFileView.Initialize( rURL, "", NULL );
    }
    aNewFolderLink.Call( this );
}

bool SvtFileViewWindow_Impl::HasPreviousLevel( OUString& rURL ) const
{
    INetURLObject aViewObj( aFileView.GetViewURL() );
    INetURLObject aRootObj( aCurrentRootURL );
    INetURLObject aMyDocObj( aMyDocumentsURL );

    return ( ( aViewObj != aRootObj || aRootObj == aMyDocObj ) && aFileView.GetParentURL( rURL ) );
}

OUString SvtFileViewWindow_Impl::GetFolderTitle() const
{
    OUString aTitle;
    ::utl::UCBContentHelper::GetTitle( aFolderURL, &aTitle );
    return aTitle;
}

void SvtFileViewWindow_Impl::SetFocus()
{
    aFileView.SetFocus();
}

// class SvtDocInfoTable_Impl --------------------------------------------

SvtDocInfoTable_Impl::SvtDocInfoTable_Impl() :

    ResStringArray( SvtResId( STRARY_SVT_DOCINFO ) )

{
}


OUString SvtDocInfoTable_Impl::GetString( long nId ) const
{
    sal_uInt32 nPos( FindIndex( nId ) );

    if ( RESARRAY_INDEX_NOTFOUND != nPos )
        return ResStringArray::GetString( nPos );

    return OUString();
}

// class SvtFrameWindow_Impl ---------------------------------------------

SvtFrameWindow_Impl::SvtFrameWindow_Impl( Window* pParent )
    : Window(pParent)
    , bDocInfo(false)
{
    // create windows and frame
    pEditWin = new ODocumentInfoPreview( this ,WB_LEFT | WB_VSCROLL | WB_READONLY | WB_BORDER | WB_3DLOOK);
    pTextWin = new Window( this );
    m_xFrame = Frame::create( ::comphelper::getProcessComponentContext() );
    xWindow = VCLUnoHelper::GetInterface( pTextWin );
    m_xFrame->initialize( xWindow );

    // create docinfo instance
    m_xDocProps.set( document::DocumentProperties::create(::comphelper::getProcessComponentContext()) );

    pEmptyWin = new Window( this, WB_BORDER | WB_3DLOOK );
}

SvtFrameWindow_Impl::~SvtFrameWindow_Impl()
{
    delete pEditWin;
    delete pEmptyWin;
    m_xFrame->dispose();
}

void SvtFrameWindow_Impl::ViewEditWin()
{
    pEmptyWin->Hide();
    xWindow->setVisible( sal_False );
    pTextWin->Hide();
    pEditWin->Show();
}

void SvtFrameWindow_Impl::ViewTextWin()
{
    pEmptyWin->Hide();
    pEditWin->Hide();
    xWindow->setVisible( sal_True );
    pTextWin->Show();
}

void SvtFrameWindow_Impl::ViewEmptyWin()
{
    xWindow->setVisible( sal_False );
    pTextWin->Hide();
    pEditWin->Hide();
    pEmptyWin->Show();
}

void SvtFrameWindow_Impl::ViewNonEmptyWin()
{
    if( bDocInfo )
        ViewEditWin();
    else
        ViewTextWin();
}

void SvtFrameWindow_Impl::ShowDocInfo( const OUString& rURL )
{
    try
    {
        uno::Reference < task::XInteractionHandler2 > xInteractionHandler(
            task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), 0) );
        uno::Sequence < beans::PropertyValue> aProps(1);
        aProps[0].Name = "InteractionHandler";
        aProps[0].Value <<= xInteractionHandler;
        m_xDocProps->loadFromMedium( rURL, aProps );
        pEditWin->fill( m_xDocProps, rURL );
    }
    catch ( UnknownPropertyException& ) {}
    catch ( Exception& ) {}
}

void SvtFrameWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    pEditWin->SetSizePixel( aWinSize );
    pTextWin->SetSizePixel( aWinSize );
    pEmptyWin->SetSizePixel( aWinSize );
}

void SvtFrameWindow_Impl::OpenFile( const OUString& rURL, bool bPreview, bool bIsTemplate, bool bAsTemplate )
{
    if ( bPreview )
        aCurrentURL = rURL;

    ViewNonEmptyWin();
    pEditWin->clear();

    if ( !rURL.isEmpty() && bPreview && m_xDocProps.is() )
        ShowDocInfo( rURL );

    if ( rURL.isEmpty() )
    {
        m_xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
        ViewEmptyWin();
    }
    else if ( !::utl::UCBContentHelper::IsFolder( rURL ) )
    {
        com::sun::star::util::URL aURL;
        aURL.Complete = rURL;
        Reference< com::sun::star::util::XURLTransformer > xTrans(
                    com::sun::star::util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aURL );

        OUString aTarget;
        Reference < XDispatchProvider > xProv( m_xFrame, UNO_QUERY_THROW );
        if ( bPreview )
            aTarget = "_self";
        else
        {
            // can be removed if the database application change its URL
            if ( !rURL.startsWith( "service:" ) )
                // service URL has no default target
                aTarget = "_default";
            xProv = Reference < XDispatchProvider >( Desktop::create(::comphelper::getProcessComponentContext() ),
                UNO_QUERY_THROW );
        }

        Reference < XDispatch > xDisp = xProv.is() ?
            xProv->queryDispatch( aURL, aTarget, 0 ) : Reference < XDispatch >();

        if ( xDisp.is() )
        {
            if ( bPreview )
            {
                if ( m_aOpenURL != aURL.Complete )
                {
                    WaitObject aWaitCursor( GetParent() );
                    // disabling must be done here, does not work in ctor because
                    // execute of the dialog will overwrite it
                    // ( own execute method would help )
                    pTextWin->EnableInput( false, true );
                    if ( pTextWin->IsReallyVisible() )
                    {
                        sal_Bool    b = sal_True;
                        Sequence < PropertyValue > aArgs( 4 );
                        aArgs[0].Name = "Preview";
                        aArgs[0].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[1].Name = "ReadOnly";
                        aArgs[1].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[2].Name = "AsTemplate";    // prevents getting an empty URL with getURL()!

                        uno::Reference < task::XInteractionHandler2 > xInteractionHandler(
                            task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), 0) );
                        aArgs[3].Name = "InteractionHandler";
                        aArgs[3].Value <<= xInteractionHandler;

                        b = sal_False;
                        aArgs[2].Value.setValue( &b, ::getBooleanCppuType() );
                        xDisp->dispatch( aURL, aArgs );

                        OUString                                                aDispURL;
                        Reference< ::com::sun::star::frame::XController >       xCtrl = m_xFrame->getController();
                        if( xCtrl.is() )
                        {
                            Reference< ::com::sun::star::frame::XModel >        xMdl = xCtrl->getModel();
                            if( xMdl.is() )
                                aDispURL = xMdl->getURL();
                        }

                        if( aDispURL != aURL.Complete )
                        {
                            m_xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
                            ViewEmptyWin();
                            m_aOpenURL = "";
                        }
                        else
                            m_aOpenURL = aDispURL;
                    }
                }
            }
            else if ( bIsTemplate )
            {
                Sequence < PropertyValue > aArgs( 1 );
                aArgs[0].Name = "AsTemplate";
                aArgs[0].Value <<= bAsTemplate;
                xDisp->dispatch( aURL, aArgs );
                m_aOpenURL = "";
            }
            else
            {
                Sequence < PropertyValue > aArgs;
                xDisp->dispatch( aURL, aArgs );
                m_aOpenURL = "";
            }
        }
    }
}

void SvtFrameWindow_Impl::ToggleView( bool bDI )
{
    bDocInfo = bDI;

    // view is set properly in OpenFile()

    OpenFile( aCurrentURL, true, false, false );
}

// class SvtTemplateWindow -----------------------------------------------

SvtTemplateWindow::SvtTemplateWindow( Window* pParent ) :

    Window( pParent, WB_DIALOGCONTROL ),

    aFileViewTB             ( this, SvtResId( TB_SVT_FILEVIEW ) ),
    aFrameWinTB             ( this, SvtResId( TB_SVT_FRAMEWIN ) ),
    aSplitWin               ( this, WB_DIALOGCONTROL | WB_NOSPLITDRAW ),
    pHistoryList            ( NULL )

{
    // create windows
    pIconWin = new SvtIconWindow_Impl( this );
    pFileWin = new SvtFileViewWindow_Impl( this );
    pFileWin->SetMyDocumentsURL( pIconWin->GetMyDocumentsRootURL() );
    pFileWin->SetSamplesFolderURL( pIconWin->GetSamplesFolderURL() );
    pFrameWin = new SvtFrameWindow_Impl( this );

    // set handlers
    pIconWin->SetClickHdl( LINK( this, SvtTemplateWindow, IconClickHdl_Impl ) );
    pFileWin->SetSelectHdl( LINK( this, SvtTemplateWindow, FileSelectHdl_Impl ) );
    pFileWin->SetDoubleClickHdl( LINK( this, SvtTemplateWindow, FileDblClickHdl_Impl ) );
    pFileWin->SetNewFolderHdl( LINK( this, SvtTemplateWindow, NewFolderHdl_Impl ) );

    // create the split items
    aSplitWin.SetAlign( WINDOWALIGN_LEFT );
    long nWidth = pIconWin->GetMaxTextLength() * 8 / 7 + 1; // extra space for border
    aSplitWin.InsertItem( ICONWIN_ID, pIconWin, nWidth, SPLITWINDOW_APPEND, 0, SWIB_FIXED );
    aSplitWin.InsertItem( FILEWIN_ID, pFileWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    aSplitWin.InsertItem( FRAMEWIN_ID, pFrameWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    aSplitWin.SetSplitHdl( LINK( this, SvtTemplateWindow, ResizeHdl_Impl ) );

    // show the windows
    pIconWin->Show();
    pFileWin->Show();
    pFrameWin->Show();
    aSplitWin.Show();

    // initialize the timers
    aSelectTimer.SetTimeout( 200 );
    aSelectTimer.SetTimeoutHdl( LINK( this, SvtTemplateWindow, TimeoutHdl_Impl ) );

    // initialize the toolboxes and then show them
    InitToolBoxes();
    aFileViewTB.Show();
    aFrameWinTB.Show();

    ReadViewSettings( );

    Application::PostUserEvent( LINK( this, SvtTemplateWindow, ResizeHdl_Impl ) );
}



SvtTemplateWindow::~SvtTemplateWindow()
{
    WriteViewSettings( );

    delete pIconWin;
    delete pFileWin;
    delete pFrameWin;
    if ( pHistoryList )
    {
        for ( size_t i = 0, n = pHistoryList->size(); i < n; ++i )
            delete (*pHistoryList)[ i ];
        pHistoryList->clear();
        delete pHistoryList;
    }
}



IMPL_LINK_NOARG(SvtTemplateWindow , IconClickHdl_Impl)
{
    OUString aURL = pIconWin->GetSelectedIconURL();
    if ( aURL.isEmpty() )
        aURL = pIconWin->GetCursorPosIconURL();
    if ( pFileWin->GetRootURL() != aURL )
    {
        pFileWin->OpenRoot( aURL );
        pIconWin->InvalidateIconControl();
        aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, false );
    }
    return 0;
}



IMPL_LINK_NOARG(SvtTemplateWindow , FileSelectHdl_Impl)
{
    aSelectTimer.Start();
    return 0;
}



IMPL_LINK_NOARG(SvtTemplateWindow , FileDblClickHdl_Impl)
{
    if ( aSelectTimer.IsActive() )
        aSelectTimer.Stop();

    OUString aURL = pFileWin->GetSelectedFile();
    if ( !aURL.isEmpty() )
    {
        if ( ::utl::UCBContentHelper::IsFolder( aURL ) )
            pFileWin->OpenFolder( aURL );
        else
            aDoubleClickHdl.Call( this );
    }

    return 0;
}



IMPL_LINK_NOARG(SvtTemplateWindow , NewFolderHdl_Impl)
{
    pFrameWin->OpenFile( "", true, false, false );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, false );

    OUString sURL = pFileWin->GetFolderURL();
    sal_uLong nPos = pIconWin->GetRootPos( sURL );
    AppendHistoryURL( sURL, nPos );

    aNewFolderHdl.Call( this );
    return 0;
}



IMPL_LINK_NOARG(SvtTemplateWindow , TimeoutHdl_Impl)
{
    aSelectHdl.Call( this );
    OUString sURL = pFileWin->GetSelectedFile();
    bool bIsNewDoc = ( pIconWin->GetSelectEntryPos() == ICON_POS_NEWDOC );
    bool bIsFile = ( !sURL.isEmpty() && !::utl::UCBContentHelper::IsFolder( sURL ) &&
                         INetURLObject( sURL ).GetProtocol() != INET_PROT_PRIVATE && !bIsNewDoc );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, bIsFile );
    aFrameWinTB.EnableItem( TI_DOCTEMPLATE_PREVIEW, !bIsNewDoc );

    if ( bIsFile )
        pFrameWin->OpenFile( sURL, true, false, false );
    else if ( bIsNewDoc && aFrameWinTB.IsItemChecked( TI_DOCTEMPLATE_PREVIEW ) )
    {
        aFrameWinTB.CheckItem( TI_DOCTEMPLATE_DOCINFO );
        DoAction( TI_DOCTEMPLATE_DOCINFO );
    }
    return 0;
}



IMPL_LINK ( SvtTemplateWindow , ClickHdl_Impl, ToolBox *, pToolBox )
{
    DoAction( pToolBox->GetCurItemId() );
    return 0;
}



IMPL_LINK_NOARG(SvtTemplateWindow , ResizeHdl_Impl)
{
    Resize();
    return 0;
}



void SvtTemplateWindow::PrintFile( const OUString& rURL )
{
    // open the file readonly and hidden
    Sequence < PropertyValue > aArgs( 2 );
    aArgs[0].Name = "ReadOnly";
    aArgs[0].Value <<= sal_True;
    aArgs[1].Name = "Hidden";
    aArgs[1].Value <<= sal_True;

    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference < XModel > xModel( xDesktop->loadComponentFromURL(
        rURL, "_blank", 0, aArgs ), UNO_QUERY );
    if ( xModel.is() )
    {
        // print
        Reference < XPrintable > xPrintable( xModel, UNO_QUERY );
        if ( xPrintable.is() )
            xPrintable->print( Sequence < PropertyValue >() );
    }
}



void SvtTemplateWindow::AppendHistoryURL( const OUString& rURL, sal_uLong nGroup )
{
    bool bInsert = true;
    if ( !pHistoryList )
        pHistoryList = new HistoryList_Impl;
    else if ( pHistoryList->size() > 0 )
    {
        FolderHistory* pLastEntry = pHistoryList->back();
        bInsert = ( rURL != pLastEntry->m_sURL);
    }

    if ( bInsert )
    {
        FolderHistory* pEntry = new FolderHistory( rURL, nGroup );
        pHistoryList->push_back( pEntry );
        aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->size() > 1 );
    }
}



void SvtTemplateWindow::OpenHistory()
{
    delete pHistoryList->back();
    pHistoryList->pop_back();
    FolderHistory* pEntry = pHistoryList->back();
    pHistoryList->pop_back();
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->size() > 1 );
    pFileWin->OpenFolder( pEntry->m_sURL );
    pIconWin->SetCursorPos( pEntry->m_nGroup );
    delete pEntry;
}



void SvtTemplateWindow::DoAction( sal_uInt16 nAction )
{
    switch( nAction )
    {
        case TI_DOCTEMPLATE_BACK :
        {
            if ( pHistoryList && pHistoryList->size() > 1 )
                OpenHistory();
            break;
        }

        case TI_DOCTEMPLATE_PREV :
        {
            OUString aURL;
            if ( pFileWin->HasPreviousLevel( aURL ) )
                pFileWin->OpenFolder( aURL );
            break;
        }

        case TI_DOCTEMPLATE_PRINT :
        {
            OUString sPrintFile( pFileWin->GetSelectedFile() );
            if ( !sPrintFile.isEmpty() )
                PrintFile( sPrintFile );
            break;
        }

        case TI_DOCTEMPLATE_DOCINFO :
        case TI_DOCTEMPLATE_PREVIEW :
        {
            pFrameWin->ToggleView( TI_DOCTEMPLATE_DOCINFO == nAction );
            break;
        }
    }
}



void SvtTemplateWindow::InitToolBoxes()
{
    InitToolBoxImages();

    Size aSize = aFileViewTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFileViewTB.SetPosSizePixel( Point( 0, 2 ), aSize );
    aSize = aFrameWinTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFrameWinTB.SetPosSizePixel( Point( pFrameWin->GetPosPixel().X() + 2, 2 ), aSize );

    bool bFlat = ( SvtMiscOptions().GetToolboxStyle() == TOOLBOX_STYLE_FLAT );
    if ( bFlat )
    {
        aFileViewTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
        aFrameWinTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
    }

    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, false );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, false );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, false );

    Link aLink = LINK( this, SvtTemplateWindow, ClickHdl_Impl );
    aFileViewTB.SetClickHdl( aLink );
    aFrameWinTB.SetClickHdl( aLink );
}



void SvtTemplateWindow::InitToolBoxImages()
{
    SvtMiscOptions aMiscOpt;
    bool bLarge = aMiscOpt.AreCurrentSymbolsLarge();

    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_BACK, Image( SvtResId(
        bLarge ? IMG_SVT_DOCTEMPLATE_BACK_LARGE
               : IMG_SVT_DOCTEMPLATE_BACK_SMALL ) ) );
    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_PREV, Image( SvtResId(
        bLarge ? IMG_SVT_DOCTEMPLATE_PREV_LARGE
               : IMG_SVT_DOCTEMPLATE_PREV_SMALL ) ) );
    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_PRINT, Image( SvtResId(
        bLarge ? IMG_SVT_DOCTEMPLATE_PRINT_LARGE
               : IMG_SVT_DOCTEMPLATE_PRINT_SMALL ) ) );

    aFrameWinTB.SetItemImage( TI_DOCTEMPLATE_DOCINFO, Image( SvtResId(
        bLarge ? IMG_SVT_DOCTEMPLATE_DOCINFO_LARGE
               : IMG_SVT_DOCTEMPLATE_DOCINFO_SMALL ) ) );
    aFrameWinTB.SetItemImage( TI_DOCTEMPLATE_PREVIEW, Image( SvtResId(
        bLarge ? IMG_SVT_DOCTEMPLATE_PREVIEW_LARGE
               : IMG_SVT_DOCTEMPLATE_PREVIEW_SMALL ) ) );
}



void SvtTemplateWindow::UpdateIcons()
{
    pIconWin->UpdateIcons();
}



bool SvtTemplateWindow::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();
    bool nRet = false;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if ( KEY_BACKSPACE == nCode && !rKeyCode.GetModifier() && pFileWin->HasChildPathFocus() )
        {
            DoAction( TI_DOCTEMPLATE_BACK );
            nRet = true;
        }
        else if ( pIconWin->ProcessKeyEvent( *rNEvt.GetKeyEvent() ) )
        {
            nRet = true;
        }
    }

    return nRet || Window::PreNotify( rNEvt );
}



void SvtTemplateWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( ( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) ||
           ( rDCEvt.GetType() == DATACHANGED_DISPLAY ) ) &&
         ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        // update of the background for the area left of the FileView toolbox
        SetBackground( Wallpaper( GetSettings().GetStyleSettings().GetFaceColor() ) );
        // update of the images of the IconChoiceControl
        UpdateIcons();
        // update of the toolbox images
        InitToolBoxImages();
    }
}


void SvtTemplateWindow::Resize()
{
    long nItemSize = aSplitWin.GetItemSize( ICONWIN_ID );
    long nSplitterWidth = Splitter( this, 0 ).GetSizePixel().Width();

    Point aPos = aFileViewTB.GetPosPixel();
    aPos.X() = nItemSize + nSplitterWidth / 2;
    aFileViewTB.SetPosPixel( aPos );

    Size aWinSize = GetOutputSizePixel();
    long nWidth = aWinSize.Width() - aPos.X();

    nItemSize = nWidth * aSplitWin.GetItemSize( FILEWIN_ID ) / 100;
    aPos.X() = pFrameWin->GetPosPixel().X() + 2;
    aFrameWinTB.SetPosPixel( aPos );

    Size aSize = aFileViewTB.GetSizePixel();
    aSize.Width() = nItemSize;
    aFileViewTB.SetSizePixel( aSize );

    aSize = aFrameWinTB.GetSizePixel();
    aSize.Width() = nWidth - nItemSize;
    aFrameWinTB.SetSizePixel( aSize );

    long nToolBoxHeight = aSize.Height() + aFrameWinTB.GetPosPixel().Y();
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    aSplitWin.SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}



OUString SvtTemplateWindow::GetSelectedFile() const
{
    return pFileWin->GetSelectedFile();
}



bool SvtTemplateWindow::IsFileSelected() const
{
    OUString aURL = pFileWin->GetSelectedFile();
    bool bRet = ( !aURL.isEmpty() && !::utl::UCBContentHelper::IsFolder( aURL ) );
    return bRet;
}



void SvtTemplateWindow::OpenFile( bool bNotAsTemplate )
{
    OUString aURL = pFileWin->GetSelectedFile();
    if ( !aURL.isEmpty() && !::utl::UCBContentHelper::IsFolder( aURL ) )
        pFrameWin->OpenFile( aURL, false, pFileWin->IsTemplateFolder(), !bNotAsTemplate );
}



OUString SvtTemplateWindow::GetFolderTitle() const
{
    OUString sTitle;
    OUString sFolderURL = pFileWin->GetFolderURL();
    if ( pIconWin->IsRootURL( sFolderURL ) )
        sTitle = pIconWin->GetIconText( sFolderURL );
    else
        sTitle = pFileWin->GetFolderTitle();
    return sTitle;
}



OUString SvtTemplateWindow::GetFolderURL() const
{
    return pFileWin->GetFolderURL();
}




void SvtTemplateWindow::SetFocus( bool bIconWin )
{
    if ( bIconWin )
        pIconWin->SetFocus();
    else
        pFileWin->SetFocus();
}



void SvtTemplateWindow::OpenTemplateRoot()
{
    pFileWin->OpenFolder( pIconWin->GetTemplateRootURL() );
}



void SvtTemplateWindow::SetPrevLevelButtonState( const OUString& rURL )
{
    // disable the prev level button on root folder of the icon pane (except My Documents)
    // and on the root of all (file:/// -> count == 0)
    INetURLObject aObj( rURL );
    sal_Int32 nCount = aObj.getSegmentCount();
    bool bEnable =
        ( nCount > 0 &&
            ( !pIconWin->IsRootURL( rURL ) || rURL == pIconWin->GetMyDocumentsRootURL() ) );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, bEnable );
}



void SvtTemplateWindow::ClearHistory()
{
    if( pHistoryList )
    {
        for ( size_t i = 0, n = pHistoryList->size(); i < n; ++i )
            delete (*pHistoryList)[ i ];
        pHistoryList->clear();
    }
}



long SvtTemplateWindow::CalcHeight() const
{
    // toolbox height
    long nHeight = aFileViewTB.GetSizePixel().Height();
    // + iconwin height
    nHeight += pIconWin->CalcHeight();
    // + little offset
    nHeight += 8;
    return nHeight;
}



void SvtTemplateWindow::ReadViewSettings()
{
    // defaults
    sal_Int32 nSelectedGroup    =   ICON_POS_TEMPLATES;
    sal_Int32 nSelectedView     =   TI_DOCTEMPLATE_DOCINFO;
    double nSplitRatio          =   0.5;
    OUString sLastFolder;

    SvtViewOptions aViewSettings( E_DIALOG, VIEWSETTING_NEWFROMTEMPLATE );
    if ( aViewSettings.Exists() )
    {
        // read the settings
        aViewSettings.GetUserItem( VIEWSETTING_SELECTEDGROUP ) >>= nSelectedGroup;
        aViewSettings.GetUserItem( VIEWSETTING_SELECTEDVIEW ) >>= nSelectedView;
        aViewSettings.GetUserItem( VIEWSETTING_SPLITRATIO ) >>= nSplitRatio;
        aViewSettings.GetUserItem( VIEWSETTING_LASTFOLDER ) >>= sLastFolder;
    }
    // normalize
    if ( nSelectedGroup < ICON_POS_NEWDOC )     nSelectedGroup = ICON_POS_NEWDOC;
    if ( nSelectedGroup > ICON_POS_SAMPLES )    nSelectedGroup = ICON_POS_SAMPLES;

    if ( ( TI_DOCTEMPLATE_DOCINFO != nSelectedView ) && ( TI_DOCTEMPLATE_PREVIEW != nSelectedView ) )
        nSelectedView = TI_DOCTEMPLATE_DOCINFO;

    if ( nSplitRatio < 0.2 ) nSplitRatio = 0.2;
    if ( nSplitRatio > 0.8 ) nSplitRatio = 0.8;

    // change our view according to the settings

    // the selected view (details or preview)
    pFrameWin->ToggleView( TI_DOCTEMPLATE_DOCINFO == nSelectedView );
    aFrameWinTB.CheckItem( (sal_uInt16)nSelectedView, true );

    // the split ratio
    sal_Int32 nSplitFileAndFrameSize = aSplitWin.GetItemSize( FILEWIN_ID ) + aSplitWin.GetItemSize( FRAMEWIN_ID );
    sal_Int32 nSplitFileSize = (sal_Int32)(nSplitFileAndFrameSize * nSplitRatio);
    sal_Int32 nSplitFrameSize = nSplitFileAndFrameSize - nSplitFileSize;
    aSplitWin.SetItemSize( FILEWIN_ID, nSplitFileSize );
    aSplitWin.SetItemSize( FRAMEWIN_ID, nSplitFrameSize );
    Resize();

    // the selected folder
    pIconWin->SetCursorPos( nSelectedGroup );

    // open the last folder or the selected group
    if ( !sLastFolder.isEmpty() )
        pFileWin->OpenFolder( sLastFolder );
    else
        IconClickHdl_Impl( NULL );
}



void SvtTemplateWindow::WriteViewSettings()
{
    // collect
    Sequence< NamedValue > aSettings(4);

    // the selected group
    aSettings[0].Name   =   VIEWSETTING_SELECTEDGROUP;
    pIconWin->SetFocus();
    aSettings[0].Value  <<= (sal_Int32)pIconWin->GetCursorPos( );

    // the selected view mode
    aSettings[1].Name   =   VIEWSETTING_SELECTEDVIEW;
    aSettings[1].Value  <<= sal_Int32( aFrameWinTB.IsItemChecked( TI_DOCTEMPLATE_DOCINFO ) ? TI_DOCTEMPLATE_DOCINFO : TI_DOCTEMPLATE_PREVIEW );

    // the split ratio
    aSettings[2].Name   =   VIEWSETTING_SPLITRATIO;
    sal_Int32 nSplitFileSize = aSplitWin.GetItemSize( FILEWIN_ID );
    sal_Int32 nSplitFileAndFrameSize = nSplitFileSize + aSplitWin.GetItemSize( FRAMEWIN_ID );
    aSettings[2].Value  <<= double( 1.0 * nSplitFileSize / nSplitFileAndFrameSize );

    // last folder
    aSettings[3].Name   =   VIEWSETTING_LASTFOLDER;
    aSettings[3].Value  <<= OUString( pFileWin->GetFolderURL() );

    // write
    SvtViewOptions aViewSettings( E_DIALOG, VIEWSETTING_NEWFROMTEMPLATE );
    aViewSettings.SetUserData( aSettings );
}

void SvtTemplateWindow::SelectFolder(sal_Int32 nFolderPosition)
{
    pIconWin->SelectFolder(nFolderPosition);
}

struct SvtTmplDlg_Impl
{
    SvtTemplateWindow*  pWin;
    OUString            aTitle;
    Timer               aUpdateTimer;
    bool            bSelectNoOpen;

    uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;


    SvtTmplDlg_Impl( Window* pParent ) : pWin( new SvtTemplateWindow( pParent ) ) ,bSelectNoOpen( false ) {}

    ~SvtTmplDlg_Impl() { delete pWin; }
};

SvtDocumentTemplateDialog::SvtDocumentTemplateDialog( Window* pParent ) :

    ModalDialog( pParent, SvtResId( DLG_DOCTEMPLATE ) ),

    aMoreTemplatesLink  ( this, SvtResId( FT_DOCTEMPLATE_LINK ) ),
    aLine               ( this, SvtResId( FL_DOCTEMPLATE ) ),
    aManageBtn          ( this, SvtResId( BTN_DOCTEMPLATE_MANAGE ) ),
    aPackageBtn         ( this, SvtResId( BTN_DOCTEMPLATE_PACKAGE ) ),
    aEditBtn            ( this, SvtResId( BTN_DOCTEMPLATE_EDIT ) ),
    aOKBtn              ( this, SvtResId( BTN_DOCTEMPLATE_OPEN ) ),
    aCancelBtn          ( this, SvtResId( BTN_DOCTEMPLATE_CANCEL ) ),
    aHelpBtn            ( this, SvtResId( BTN_DOCTEMPLATE_HELP ) ),
    pImpl               ( NULL )
{
    FreeResource();
    InitImpl( );
}



void SvtDocumentTemplateDialog::InitImpl( )
{
    pImpl = new SvtTmplDlg_Impl( this );
    pImpl->aTitle = GetText();

    bool bHideLink = ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
                    == SvtExtendedSecurityOptions::OPEN_NEVER );
    if ( !bHideLink )
         {
    aMoreTemplatesLink.SetURL( "http://templates.libreoffice.org/" );
    aMoreTemplatesLink.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, OpenLinkHdl_Impl ) );
    }
    else
       aMoreTemplatesLink.Hide();

    aManageBtn.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, OrganizerHdl_Impl ) );
    // Only enable the Package Button, if the service is available
    try
    {
        using namespace org::freedesktop::PackageKit;
        Reference< XSyncDbusSessionHelper > xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()), UNO_QUERY);
        aPackageBtn.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, PackageHdl_Impl ) );
        aPackageBtn.Enable(xSyncDbusSessionHelper.is());
    }
    catch (Exception & e)
    {
        SAL_INFO(
            "svtools.contnr",
            "disable Install Template Pack, caught " << e.Message);
        aPackageBtn.Enable(false);
    }
    Link aLink = LINK( this, SvtDocumentTemplateDialog, OKHdl_Impl );
    aEditBtn.SetClickHdl( aLink );
    aOKBtn.SetClickHdl( aLink );

    pImpl->pWin->SetSelectHdl( LINK( this, SvtDocumentTemplateDialog, SelectHdl_Impl ) );
    pImpl->pWin->SetDoubleClickHdl( LINK( this, SvtDocumentTemplateDialog, DoubleClickHdl_Impl ) );
    pImpl->pWin->SetNewFolderHdl( LINK( this, SvtDocumentTemplateDialog, NewFolderHdl_Impl ) );
    pImpl->pWin->SetSendFocusHdl( LINK( this, SvtDocumentTemplateDialog, SendFocusHdl_Impl ) );

    // dynamic height adjustment
    long nHeight = pImpl->pWin->CalcHeight();

    Size aSize = GetOutputSizePixel();
    Point aPos = aMoreTemplatesLink.GetPosPixel();
    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    if ( bHideLink )
        aPos.Y() += aMoreTemplatesLink.GetSizePixel().Height();
    else
        aPos.Y() -= a6Size.Height();
    long nDelta = aPos.Y() - nHeight;
    aSize.Height() -= nDelta;
    SetOutputSizePixel( aSize );

    aSize.Height() = nHeight;
    aSize.Width() -= ( a6Size.Width() * 2 );
    pImpl->pWin->SetPosSizePixel( Point( a6Size.Width(), 0 ), aSize );

    aPos = aMoreTemplatesLink.GetPosPixel();
    aPos.Y() -= nDelta;
    aMoreTemplatesLink.SetPosPixel( aPos );
    aPos = aLine.GetPosPixel();
    aPos.Y() -= nDelta;
    aLine.SetPosPixel( aPos );
    aPos = aManageBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aManageBtn.SetPosPixel( aPos );
    aPos = aEditBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aEditBtn.SetPosPixel( aPos );
    aPos = aOKBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aOKBtn.SetPosPixel( aPos );
    aPos = aCancelBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aCancelBtn.SetPosPixel( aPos );
    aPos = aHelpBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aHelpBtn.SetPosPixel( aPos );
    aPos = aPackageBtn.GetPosPixel();
    aPos.Y() -= nDelta;
    aPackageBtn.SetPosPixel( aPos );

    pImpl->pWin->Show();

    SelectHdl_Impl( NULL );
    NewFolderHdl_Impl( NULL );

    UpdateHdl_Impl( NULL );
}



SvtDocumentTemplateDialog::~SvtDocumentTemplateDialog()
{
    delete pImpl;
}



bool SvtDocumentTemplateDialog::CanEnableEditBtn() const
{
    bool bEnable = false;

    OUString aFolderURL = pImpl->pWin->GetFolderURL();
    if ( pImpl->pWin->IsFileSelected() && !aFolderURL.isEmpty() )
    {
        OUString aFileTargetURL = pImpl->pWin->GetSelectedFile();
        bEnable = !aFileTargetURL.isEmpty();
    }

    return bEnable;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog , SelectHdl_Impl)
{
    aEditBtn.Enable( pImpl->pWin->IsTemplateFolderOpen() && CanEnableEditBtn() );
    aOKBtn.Enable( pImpl->pWin->IsFileSelected() );
    return 0;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog , DoubleClickHdl_Impl)
{
    EndDialog( RET_OK );

    if ( !pImpl->bSelectNoOpen )
        pImpl->pWin->OpenFile( !pImpl->pWin->IsTemplateFolderOpen() );
    return 0;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog , NewFolderHdl_Impl)
{
    OUString aNewTitle( pImpl->aTitle );
    aNewTitle += " - ";
    aNewTitle += pImpl->pWin->GetFolderTitle();
    SetText( aNewTitle );

    SelectHdl_Impl( NULL );
    return 0;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog , SendFocusHdl_Impl)
{
    if ( pImpl->pWin->HasIconWinFocus() )
        aHelpBtn.GrabFocus();
    else
    {
        if ( aEditBtn.IsEnabled() )
            aEditBtn.GrabFocus();
        else if ( aOKBtn.IsEnabled() )
            aOKBtn.GrabFocus();
        else
            aCancelBtn.GrabFocus();
    }

    return 0;
}



IMPL_LINK ( SvtDocumentTemplateDialog , OKHdl_Impl, PushButton *, pBtn )
{
    if ( pImpl->pWin->IsFileSelected() )
    {
        EndDialog( RET_OK );

        if ( !pImpl->bSelectNoOpen )
            pImpl->pWin->OpenFile( &aEditBtn == pBtn );
    }
    return 0;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog , OrganizerHdl_Impl)
{
    Window* pOldDefWin = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
    Reference < XFrame > xFrame( xDesktop->getActiveFrame() );
    if ( !xFrame.is() )
        xFrame = xDesktop;

    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = "slot:5540";
    Reference< com::sun::star::util::XURLTransformer > xTrans(
                com::sun::star::util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < XDispatch > xDisp;
    xDisp = xProv->queryDispatch( aTargetURL, OUString(), 0 );

    if ( xDisp.is() )
    {
        Sequence<PropertyValue> aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = "Referer";
        pArg[0].Value <<= OUString("private:user");
        xDisp->dispatch( aTargetURL, aArgs );
    }

    Application::SetDefDialogParent( pOldDefWin );
    return 0;
}


IMPL_LINK_NOARG(SvtDocumentTemplateDialog, PackageHdl_Impl)
{
    try
    {
        using namespace org::freedesktop::PackageKit;
        Reference< XSyncDbusSessionHelper > xSyncDbusSessionHelper(SyncDbusSessionHelper::create(comphelper::getProcessComponentContext()), UNO_QUERY);
        Sequence< OUString > vPackages(1);
        vPackages[0] = "libreoffice-templates";
        OUString sInteraction("");
        xSyncDbusSessionHelper->InstallPackageNames(0, vPackages, sInteraction);
    }
    catch (Exception & e)
    {
        SAL_INFO(
            "svtools.contnr",
            "trying Install Template Pack, caught " << e.Message);
    }
    return 0;
}



IMPL_LINK ( SvtDocumentTemplateDialog, UpdateHdl_Impl, Timer*, _pEventSource )
{
    pImpl->pWin->SetFocus( false );
    Reference< XDocumentTemplates > xTemplates( frame::DocumentTemplates::create(::comphelper::getProcessComponentContext()) );
    if ( _pEventSource )
    {   // it was no direct call, which means it was triggered by the timer, which means we alread checked the necessity
        WaitObject aWaitCursor( this );
        xTemplates->update();
        if ( pImpl->pWin->IsTemplateFolderOpen() )
        {
            pImpl->pWin->ClearHistory();
            pImpl->pWin->OpenTemplateRoot();
        }
    }
    else
    {
        // check if we really need to do the update
        ::svt::TemplateFolderCache aCache;
        if ( aCache.needsUpdate() )
        {   // yes -> do it asynchronous (it will take a noticeable time)

            // (but first store the current state)
            aCache.storeState();

            // start the timer for the async update
            pImpl->aUpdateTimer.SetTimeout( 300 );
            pImpl->aUpdateTimer.SetTimeoutHdl( LINK( this, SvtDocumentTemplateDialog, UpdateHdl_Impl ) );
            pImpl->aUpdateTimer.Start();
        }
    }
    return 0;
}



IMPL_LINK_NOARG(SvtDocumentTemplateDialog, OpenLinkHdl_Impl)
{
    OUString sURL( aMoreTemplatesLink.GetURL() );
    if ( !sURL.isEmpty() )
    {
        localizeWebserviceURI(sURL);
        try
        {
            uno::Reference< uno::XComponentContext > xContext =
                ::comphelper::getProcessComponentContext();
            uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShell(
                com::sun::star::system::SystemShellExecute::create(xContext) );
            if ( xSystemShell.is() )
                xSystemShell->execute( sURL, OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
            EndDialog( RET_CANCEL );
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    return 0;
}

void SvtDocumentTemplateDialog::SelectTemplateFolder()
{
    pImpl->pWin->SelectFolder(ICON_POS_TEMPLATES);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
