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
#include <svtools/svtools.hrc>
#include "templwin.hrc"
#include <svtools/helpid.hrc>
#include <unotools/viewoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include "unotools/configmgr.hxx"
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
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

#define SPLITSET_ID         0
#define COLSET_ID           1
#define ICONWIN_ID          2
#define FILEWIN_ID          3
#define FRAMEWIN_ID         4

#define ICON_POS_NEWDOC     0
#define ICON_POS_TEMPLATES  1
#define ICON_POS_MYDOCS     2
#define ICON_POS_SAMPLES    3

#define ASCII_STR(s)                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define VIEWSETTING_NEWFROMTEMPLATE     ASCII_STR("NewFromTemplate")
#define VIEWSETTING_SELECTEDGROUP       ASCII_STR("SelectedGroup")
#define VIEWSETTING_SELECTEDVIEW        ASCII_STR("SelectedView")
#define VIEWSETTING_SPLITRATIO          ASCII_STR("SplitRatio")
#define VIEWSETTING_LASTFOLDER          ASCII_STR("LastFolder")

struct FolderHistory
{
    String      m_sURL;
    sal_uLong       m_nGroup;

    FolderHistory( const String& _rURL, sal_Int32 _nGroup ) :
        m_sURL( _rURL ), m_nGroup( _nGroup ) {}
};

typedef ::std::vector< ::rtl::OUString* > NewDocList_Impl;

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
    aNewDocumentRootURL( ASCII_STR("private:newdoc") ),
    aMyDocumentsRootURL( SvtPathOptions().GetWorkPath() ),
    aSamplesFolderRootURL( SvtPathOptions().
        SubstituteVariable( String( ASCII_STR("$(insturl)/share/samples/$(vlang)") ) ) ),
    nMaxTextLength( 0 )

{
    aDummyHeaderBar.Show();

    aIconCtrl.SetAccessibleName( String( RTL_CONSTASCII_USTRINGPARAM("Groups") ) );
      aIconCtrl.SetHelpId( HID_TEMPLATEDLG_ICONCTRL );
    aIconCtrl.SetChoiceWithCursor( sal_True );
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
    String aEntryStr = SVT_RESSTR(STR_SVT_NEWDOC);
    SvxIconChoiceCtrlEntry* pEntry =
        aIconCtrl.InsertEntry( aEntryStr, aImage, ICON_POS_NEWDOC );
    pEntry->SetUserData( new String( aNewDocumentRootURL ) );
    pEntry->SetQuickHelpText( SVT_RESSTR(STR_SVT_NEWDOC_HELP) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    long nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    // "Templates"
    if( aTemplateRootURL.Len() > 0 )
    {
        aEntryStr = SVT_RESSTR(STR_SVT_TEMPLATES);
        pEntry = aIconCtrl.InsertEntry(
            aEntryStr, Image( SvtResId( IMG_SVT_TEMPLATES ) ), ICON_POS_TEMPLATES );
        pEntry->SetUserData( new String( aTemplateRootURL ) );
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
    pEntry->SetUserData( new String( aMyDocumentsRootURL ) );
    pEntry->SetQuickHelpText( SVT_RESSTR(STR_SVT_MYDOCS_HELP) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if( nTemp > nMaxTextLength )
        nMaxTextLength = nTemp;

    // "Samples"
    aEntryStr = SVT_RESSTR(STR_SVT_SAMPLES);
    pEntry = aIconCtrl.InsertEntry(
        aEntryStr, Image( SvtResId( IMG_SVT_SAMPLES ) ), ICON_POS_SAMPLES );
    pEntry->SetUserData( new String( aSamplesFolderRootURL ) );
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
        delete (String*)pEntry->GetUserData();
    }
}

SvxIconChoiceCtrlEntry* SvtIconWindow_Impl::GetEntry( const String& rURL ) const
{
    SvxIconChoiceCtrlEntry* pEntry = NULL;
    for ( sal_uLong i = 0; i < aIconCtrl.GetEntryCount(); ++i )
    {
        SvxIconChoiceCtrlEntry* pTemp = aIconCtrl.GetEntry( i );
        String aURL( *( (String*)pTemp->GetUserData() ) );
        if ( aURL == rURL )
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

String SvtIconWindow_Impl::GetCursorPosIconURL() const
{
    String aURL;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetCursor( );
    if ( pEntry )
        aURL = *static_cast<String*>(pEntry->GetUserData());
    return aURL;

}

String SvtIconWindow_Impl::GetSelectedIconURL() const
{
    sal_uLong nPos;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetSelectedEntry( nPos );
    String aURL;
    if ( pEntry )
        aURL = *static_cast<String*>(pEntry->GetUserData());
    return aURL;
}

String SvtIconWindow_Impl::GetIconText( const String& rURL ) const
{
    String aText;
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

sal_Bool SvtIconWindow_Impl::IsRootURL( const String& rURL ) const
{
    return  rURL == aNewDocumentRootURL ||
            rURL == aTemplateRootURL ||
            rURL == aMyDocumentsRootURL ||
            rURL == aSamplesFolderRootURL;
}

sal_uLong SvtIconWindow_Impl::GetRootPos( const String& rURL ) const
{
    sal_uLong nPos = ~sal_uLong(0);
    if ( aNewDocumentRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 0;
    else if ( aTemplateRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 1;
    else if ( aMyDocumentsRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 2;
    else if ( aSamplesFolderRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 3;
    else if ( rURL.Match( aMyDocumentsRootURL ) == STRING_MATCH )
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
    aFileView           ( this, SvtResId( CTRL_FILEVIEW ), FILEVIEW_SHOW_NONE | FILEVIEW_SHOW_ONLYTITLE ),
    bIsTemplateFolder   ( sal_False )

{
    aFileView.SetStyle( aFileView.GetStyle() | WB_DIALOGCONTROL | WB_TABSTOP );
    aFileView.SetHelpId( HID_TEMPLATEDLG_FILEVIEW );
    aFileView.Show();
    aFileView.EnableAutoResize();
    aFileView.EnableContextMenu( sal_False );
    aFileView.EnableDelete( sal_False );
}

SvtFileViewWindow_Impl::~SvtFileViewWindow_Impl()
{
}

void GetMenuEntry_Impl
(
    Sequence< PropertyValue >& aDynamicMenuEntry,
    ::rtl::OUString& rTitle,
    ::rtl::OUString& rURL,
    ::rtl::OUString& rFrame,
    ::rtl::OUString& rImageId
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

Sequence< ::rtl::OUString > SvtFileViewWindow_Impl::GetNewDocContents() const
{
    NewDocList_Impl aNewDocs;
    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;
    aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aImageURL;
    ::rtl::OUString aTargetFrame;

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    ::rtl::OUString sSeparator( ASCII_STR("private:separator") );
    ::rtl::OUString sSlotURL( ASCII_STR("slot:5500") );

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry_Impl( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageURL );
        if( aURL == sSlotURL )
            continue;
        if( aURL == sSeparator )
        {
            String aSeparator( ASCII_STR( aSeparatorStr ) );
            ::rtl::OUString* pSeparator = new ::rtl::OUString( aSeparator );
            aNewDocs.push_back( pSeparator );
        }
        else
        {
            // title
            String aRow = MnemonicGenerator::EraseAllMnemonicChars( String( aTitle ) );
            aRow += '\t';
            // no type
            aRow += '\t';
            // no size
            aRow += '\t';
            // no date
            aRow += '\t';
            // url
            aRow += String( aURL );
            aRow += '\t';
            // folder == false
            aRow += '0';
            // image url?
            if ( !aImageURL.isEmpty() )
            {
                aRow += '\t';
                aRow += String( aImageURL );
            }

            ::rtl::OUString* pRow = new ::rtl::OUString( aRow );
            aNewDocs.push_back( pRow );
        }
    }

    nCount = aNewDocs.size();
    Sequence < ::rtl::OUString > aRet( nCount );
    ::rtl::OUString* pRet = aRet.getArray();
    for ( i = 0; i < nCount; ++i )
    {
        ::rtl::OUString* pNewDoc = aNewDocs[i];
        pRet[i] = *( pNewDoc );
        delete pNewDoc;
    }

    return aRet;
}

void SvtFileViewWindow_Impl::Resize()
{
    aFileView.SetSizePixel(GetOutputSizePixel());
}

String SvtFileViewWindow_Impl::GetSelectedFile() const
{
    return aFileView.GetCurrentURL();
}

void SvtFileViewWindow_Impl::OpenFolder( const String& rURL )
{
    aFolderURL = rURL;

    rParent.SetPrevLevelButtonState( rURL );

    aFileView.SetUrlFilter( &aURLFilter );

    INetProtocol eProt = INetURLObject( rURL ).GetProtocol();
    bIsTemplateFolder = ( eProt == INET_PROT_VND_SUN_STAR_HIER );
    bool isNewDocumentFolder = ( eProt == INET_PROT_PRIVATE );

    aURLFilter.enableFilter( !bIsTemplateFolder && !isNewDocumentFolder );

    if ( isNewDocumentFolder )
    {
        aFileView.EnableNameReplacing( sal_False );
        aFileView.Initialize( GetNewDocContents() );
    }
    else
    {
        xub_StrLen nSampFoldLen = aSamplesFolderURL.Len();
        aFileView.EnableNameReplacing(
                    nSampFoldLen && rURL.CompareTo( aSamplesFolderURL, nSampFoldLen ) == COMPARE_EQUAL );
        aFileView.Initialize( rURL, String(), NULL );
    }
    aNewFolderLink.Call( this );
}

sal_Bool SvtFileViewWindow_Impl::HasPreviousLevel( String& rURL ) const
{
    INetURLObject aViewObj( aFileView.GetViewURL() );
    INetURLObject aRootObj( aCurrentRootURL );
    INetURLObject aMyDocObj( aMyDocumentsURL );

    return ( ( aViewObj != aRootObj || aRootObj == aMyDocObj ) && aFileView.GetParentURL( rURL ) );
}

String SvtFileViewWindow_Impl::GetFolderTitle() const
{
    rtl::OUString aTitle;
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
// -----------------------------------------------------------------------

rtl::OUString SvtDocInfoTable_Impl::GetString( long nId ) const
{
    sal_uInt32 nPos( FindIndex( nId ) );

    if ( RESARRAY_INDEX_NOTFOUND != nPos )
        return ResStringArray::GetString( nPos );

    return rtl::OUString();
}

// class SvtFrameWindow_Impl ---------------------------------------------

SvtFrameWindow_Impl::SvtFrameWindow_Impl( Window* pParent ) :

    Window( pParent )

{
    // detect application language
    aLocale= SvtPathOptions().GetLocale();

    // create windows and frame
    pEditWin = new ODocumentInfoPreview( this ,WB_LEFT | WB_VSCROLL | WB_READONLY | WB_BORDER | WB_3DLOOK);
    pTextWin = new Window( this );
    xFrame = Reference < XFrame > ( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.frame.Frame") ), UNO_QUERY );
    xWindow = VCLUnoHelper::GetInterface( pTextWin );
    xFrame->initialize( xWindow );

    // create docinfo instance
    m_xDocProps.set( document::DocumentProperties::create(::comphelper::getProcessComponentContext()) );

    pEmptyWin = new Window( this, WB_BORDER | WB_3DLOOK );
}

SvtFrameWindow_Impl::~SvtFrameWindow_Impl()
{
    delete pEditWin;
    delete pEmptyWin;
    xFrame->dispose();
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

void SvtFrameWindow_Impl::ShowDocInfo( const String& rURL )
{
    try
    {
        uno::Reference < task::XInteractionHandler > xInteractionHandler( ::comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.InteractionHandler" )) ), uno::UNO_QUERY );
        uno::Sequence < beans::PropertyValue> aProps(1);
        aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
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

void SvtFrameWindow_Impl::OpenFile( const String& rURL, sal_Bool bPreview, sal_Bool bIsTemplate, sal_Bool bAsTemplate )
{
    if ( bPreview )
        aCurrentURL = rURL;

    ViewNonEmptyWin();
    pEditWin->clear();

    if ( rURL.Len() > 0 && bPreview && m_xDocProps.is() )
        ShowDocInfo( rURL );

    if ( rURL.Len() == 0 )
    {
        xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
        ViewEmptyWin();
    }
    else if ( !::utl::UCBContentHelper::IsFolder( rURL ) )
    {
        com::sun::star::util::URL aURL;
        aURL.Complete = rURL;
        Reference< com::sun::star::util::XURLTransformer > xTrans(
                    com::sun::star::util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
        xTrans->parseStrict( aURL );

        String aTarget;
        Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
        if ( bPreview )
            aTarget = ASCII_STR("_self");
        else
        {
            // can be removed if the database application change its URL
            String sServiceScheme( RTL_CONSTASCII_USTRINGPARAM( "service:" ) );
            if ( rURL.Match( sServiceScheme ) != sServiceScheme.Len() )
                // service URL has no default target
                aTarget = ASCII_STR("_default");
            xProv = Reference < XDispatchProvider >( ::comphelper::getProcessServiceFactory()->
                createInstance( ASCII_STR("com.sun.star.frame.Desktop") ), UNO_QUERY );
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
                    pTextWin->EnableInput( sal_False, sal_True );
                    if ( pTextWin->IsReallyVisible() )
                    {
                        sal_Bool    b = sal_True;
                        Sequence < PropertyValue > aArgs( 4 );
                        aArgs[0].Name = ASCII_STR("Preview");
                        aArgs[0].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[1].Name = ASCII_STR("ReadOnly");
                        aArgs[1].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[2].Name = ASCII_STR("AsTemplate");    // prevents getting an empty URL with getURL()!

                        uno::Reference < task::XInteractionHandler > xInteractionHandler( ::comphelper::getProcessServiceFactory()->createInstance(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.task.InteractionHandler" )) ), uno::UNO_QUERY );
                        aArgs[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ));
                        aArgs[3].Value <<= xInteractionHandler;

                        b = sal_False;
                        aArgs[2].Value.setValue( &b, ::getBooleanCppuType() );
                        xDisp->dispatch( aURL, aArgs );

                        ::rtl::OUString                                         aDispURL;
                        Reference< ::com::sun::star::frame::XController >       xCtrl = xFrame->getController();
                        if( xCtrl.is() )
                        {
                            Reference< ::com::sun::star::frame::XModel >        xMdl = xCtrl->getModel();
                            if( xMdl.is() )
                                aDispURL = xMdl->getURL();
                        }

                        if( aDispURL != aURL.Complete )
                        {
                            xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
                            ViewEmptyWin();
                            m_aOpenURL = rtl::OUString();
                        }
                        else
                            m_aOpenURL = aDispURL;
                    }
                }
            }
            else if ( bIsTemplate )
            {
                Sequence < PropertyValue > aArgs( 1 );
                aArgs[0].Name = ASCII_STR("AsTemplate");
                aArgs[0].Value <<= bAsTemplate;
                xDisp->dispatch( aURL, aArgs );
                m_aOpenURL = rtl::OUString();
            }
            else
            {
                Sequence < PropertyValue > aArgs;
                xDisp->dispatch( aURL, aArgs );
                m_aOpenURL = rtl::OUString();
            }
        }
    }
}

void SvtFrameWindow_Impl::ToggleView( sal_Bool bDI )
{
    bDocInfo = bDI;

    // view is set properly in OpenFile()

    OpenFile( aCurrentURL, sal_True, sal_False, sal_False );
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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , IconClickHdl_Impl)
{
    String aURL = pIconWin->GetSelectedIconURL();
    if ( !aURL.Len() )
        aURL = pIconWin->GetCursorPosIconURL();
    if ( pFileWin->GetRootURL() != aURL )
    {
        pFileWin->OpenRoot( aURL );
        pIconWin->InvalidateIconControl();
        aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, sal_False );
    }
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , FileSelectHdl_Impl)
{
    aSelectTimer.Start();
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , FileDblClickHdl_Impl)
{
    if ( aSelectTimer.IsActive() )
        aSelectTimer.Stop();

    String aURL = pFileWin->GetSelectedFile();
    if ( aURL.Len() > 0 )
    {
        if ( ::utl::UCBContentHelper::IsFolder( aURL ) )
            pFileWin->OpenFolder( aURL );
        else
            aDoubleClickHdl.Call( this );
    }

    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , NewFolderHdl_Impl)
{
    pFrameWin->OpenFile( String(), sal_True, sal_False, sal_False );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, sal_False );

    String sURL = pFileWin->GetFolderURL();
    sal_uLong nPos = pIconWin->GetRootPos( sURL );
    AppendHistoryURL( sURL, nPos );

    aNewFolderHdl.Call( this );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , TimeoutHdl_Impl)
{
    aSelectHdl.Call( this );
    String sURL = pFileWin->GetSelectedFile();
    sal_Bool bIsNewDoc = ( pIconWin->GetSelectEntryPos() == ICON_POS_NEWDOC );
    sal_Bool bIsFile = ( sURL.Len() != 0 && !::utl::UCBContentHelper::IsFolder( sURL ) &&
                         INetURLObject( sURL ).GetProtocol() != INET_PROT_PRIVATE && !bIsNewDoc );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, bIsFile );
    aFrameWinTB.EnableItem( TI_DOCTEMPLATE_PREVIEW, !bIsNewDoc );

    if ( bIsFile )
        pFrameWin->OpenFile( sURL, sal_True, sal_False, sal_False );
    else if ( bIsNewDoc && aFrameWinTB.IsItemChecked( TI_DOCTEMPLATE_PREVIEW ) )
    {
        aFrameWinTB.CheckItem( TI_DOCTEMPLATE_DOCINFO );
        DoAction( TI_DOCTEMPLATE_DOCINFO );
    }
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , ClickHdl_Impl, ToolBox *, pToolBox )
{
    DoAction( pToolBox->GetCurItemId() );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtTemplateWindow , ResizeHdl_Impl)
{
    Resize();
    return 0;
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::PrintFile( const String& rURL )
{
    // open the file readonly and hidden
    Sequence < PropertyValue > aArgs( 2 );
    aArgs[0].Name = ASCII_STR("ReadOnly");
    aArgs[0].Value <<= sal_True;
    aArgs[1].Name = ASCII_STR("Hidden");
    aArgs[1].Value <<= sal_True;

    Reference < XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XModel > xModel( xDesktop->loadComponentFromURL(
        rURL, ASCII_STR("_blank"), 0, aArgs ), UNO_QUERY );
    if ( xModel.is() )
    {
        // print
        Reference < XPrintable > xPrintable( xModel, UNO_QUERY );
        if ( xPrintable.is() )
            xPrintable->print( Sequence < PropertyValue >() );
    }
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::AppendHistoryURL( const String& rURL, sal_uLong nGroup )
{
    sal_Bool bInsert = sal_True;
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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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
            String aURL;
            if ( pFileWin->HasPreviousLevel( aURL ) )
                pFileWin->OpenFolder( aURL );
            break;
        }

        case TI_DOCTEMPLATE_PRINT :
        {
            String sPrintFile( pFileWin->GetSelectedFile() );
            if ( sPrintFile.Len() > 0 )
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

// ------------------------------------------------------------------------

void SvtTemplateWindow::InitToolBoxes()
{
    InitToolBoxImages();

    Size aSize = aFileViewTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFileViewTB.SetPosSizePixel( Point( 0, 2 ), aSize );
    aSize = aFrameWinTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFrameWinTB.SetPosSizePixel( Point( pFrameWin->GetPosPixel().X() + 2, 2 ), aSize );

    sal_Bool bFlat = ( SvtMiscOptions().GetToolboxStyle() == TOOLBOX_STYLE_FLAT );
    if ( bFlat )
    {
        aFileViewTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
        aFrameWinTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
    }

    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, sal_False );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, sal_False );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, sal_False );

    Link aLink = LINK( this, SvtTemplateWindow, ClickHdl_Impl );
    aFileViewTB.SetClickHdl( aLink );
    aFrameWinTB.SetClickHdl( aLink );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::InitToolBoxImages()
{
    SvtMiscOptions aMiscOpt;
    sal_Bool bLarge = aMiscOpt.AreCurrentSymbolsLarge();

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

// ------------------------------------------------------------------------

void SvtTemplateWindow::UpdateIcons()
{
    pIconWin->UpdateIcons();
}

// ------------------------------------------------------------------------

long SvtTemplateWindow::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();
    long nRet = 0;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if ( KEY_BACKSPACE == nCode && !rKeyCode.GetModifier() && pFileWin->HasChildPathFocus() )
        {
            DoAction( TI_DOCTEMPLATE_BACK );
            nRet = 1;
        }
        else if ( pIconWin->ProcessKeyEvent( *rNEvt.GetKeyEvent() ) )
        {
            nRet = 1;
        }
    }

    return nRet ? nRet : Window::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------------

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
// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

String SvtTemplateWindow::GetSelectedFile() const
{
    return pFileWin->GetSelectedFile();
}

// ------------------------------------------------------------------------

sal_Bool SvtTemplateWindow::IsFileSelected() const
{
    String aURL = pFileWin->GetSelectedFile();
    sal_Bool bRet = ( aURL.Len() > 0 && !::utl::UCBContentHelper::IsFolder( aURL ) );
    return bRet;
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::OpenFile( sal_Bool bNotAsTemplate )
{
    String aURL = pFileWin->GetSelectedFile();
    if ( aURL.Len() > 0 && !::utl::UCBContentHelper::IsFolder( aURL ) )
        pFrameWin->OpenFile( aURL, sal_False, pFileWin->IsTemplateFolder(), !bNotAsTemplate );
}

// ------------------------------------------------------------------------

String SvtTemplateWindow::GetFolderTitle() const
{
    String sTitle;
    String sFolderURL = pFileWin->GetFolderURL();
    if ( pIconWin->IsRootURL( sFolderURL ) )
        sTitle = pIconWin->GetIconText( sFolderURL );
    else
        sTitle = pFileWin->GetFolderTitle();
    return sTitle;
}

// ------------------------------------------------------------------------

String SvtTemplateWindow::GetFolderURL() const
{
    return pFileWin->GetFolderURL();
}


// ------------------------------------------------------------------------

void SvtTemplateWindow::SetFocus( sal_Bool bIconWin )
{
    if ( bIconWin )
        pIconWin->SetFocus();
    else
        pFileWin->SetFocus();
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::OpenTemplateRoot()
{
    pFileWin->OpenFolder( pIconWin->GetTemplateRootURL() );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::SetPrevLevelButtonState( const String& rURL )
{
    // disable the prev level button on root folder of the icon pane (except My Documents)
    // and on the root of all (file:/// -> count == 0)
    INetURLObject aObj( rURL );
    sal_Int32 nCount = aObj.getSegmentCount();
    sal_Bool bEnable =
        ( nCount > 0 &&
            ( !pIconWin->IsRootURL( rURL ) || rURL == pIconWin->GetMyDocumentsRootURL() ) );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, bEnable );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::ClearHistory()
{
    if( pHistoryList )
    {
        for ( size_t i = 0, n = pHistoryList->size(); i < n; ++i )
            delete (*pHistoryList)[ i ];
        pHistoryList->clear();
    }
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

void SvtTemplateWindow::ReadViewSettings()
{
    // defaults
    sal_Int32 nSelectedGroup    =   ICON_POS_TEMPLATES;
    sal_Int32 nSelectedView     =   TI_DOCTEMPLATE_DOCINFO;
    double nSplitRatio          =   0.5;
    ::rtl::OUString sLastFolder;

    SvtViewOptions aViewSettings( E_DIALOG, VIEWSETTING_NEWFROMTEMPLATE );
    if ( aViewSettings.Exists() )
    {
        // read the settings
        Sequence< NamedValue > aSettings = aViewSettings.GetUserData( );

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
    aFrameWinTB.CheckItem( (sal_uInt16)nSelectedView, sal_True );

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

// ------------------------------------------------------------------------

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
    aSettings[3].Value  <<= ::rtl::OUString( pFileWin->GetFolderURL() );

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
    String              aTitle;
    Timer               aUpdateTimer;
    sal_Bool            bSelectNoOpen;

    uno::Reference< util::XOfficeInstallationDirectories > m_xOfficeInstDirs;


    SvtTmplDlg_Impl( Window* pParent ) : pWin( new SvtTemplateWindow( pParent ) ) ,bSelectNoOpen( sal_False ) {}

    ~SvtTmplDlg_Impl() { delete pWin; }
};

SvtDocumentTemplateDialog::SvtDocumentTemplateDialog( Window* pParent ) :

    ModalDialog( pParent, SvtResId( DLG_DOCTEMPLATE ) ),

    aMoreTemplatesLink  ( this, SvtResId( FT_DOCTEMPLATE_LINK ) ),
    aLine               ( this, SvtResId( FL_DOCTEMPLATE ) ),
    aManageBtn          ( this, SvtResId( BTN_DOCTEMPLATE_MANAGE ) ),
    aEditBtn            ( this, SvtResId( BTN_DOCTEMPLATE_EDIT ) ),
    aOKBtn              ( this, SvtResId( BTN_DOCTEMPLATE_OPEN ) ),
    aCancelBtn          ( this, SvtResId( BTN_DOCTEMPLATE_CANCEL ) ),
    aHelpBtn            ( this, SvtResId( BTN_DOCTEMPLATE_HELP ) ),
    pImpl               ( NULL )
{
    FreeResource();
    InitImpl( );
}

// ------------------------------------------------------------------------

void SvtDocumentTemplateDialog::InitImpl( )
{
    pImpl = new SvtTmplDlg_Impl( this );
    pImpl->aTitle = GetText();

    bool bHideLink = ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
                    == SvtExtendedSecurityOptions::OPEN_NEVER );
    if ( !bHideLink )
         {
    aMoreTemplatesLink.SetURL( String(
        RTL_CONSTASCII_USTRINGPARAM( "http://templates.libreoffice.org/" ) ) );
    aMoreTemplatesLink.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, OpenLinkHdl_Impl ) );
    }
    else
       aMoreTemplatesLink.Hide();

    aManageBtn.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, OrganizerHdl_Impl ) );
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

    pImpl->pWin->Show();

    SelectHdl_Impl( NULL );
    NewFolderHdl_Impl( NULL );

    UpdateHdl_Impl( NULL );
}

// ------------------------------------------------------------------------

SvtDocumentTemplateDialog::~SvtDocumentTemplateDialog()
{
    delete pImpl;
}

// ------------------------------------------------------------------------

sal_Bool SvtDocumentTemplateDialog::CanEnableEditBtn() const
{
    sal_Bool bEnable = sal_False;

    ::rtl::OUString aFolderURL = pImpl->pWin->GetFolderURL();
    if ( pImpl->pWin->IsFileSelected() && !aFolderURL.isEmpty() )
    {
        ::rtl::OUString aFileTargetURL = pImpl->pWin->GetSelectedFile();
        bEnable = !aFileTargetURL.isEmpty();
    }

    return bEnable;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtDocumentTemplateDialog , SelectHdl_Impl)
{
    aEditBtn.Enable( pImpl->pWin->IsTemplateFolderOpen() && CanEnableEditBtn() );
    aOKBtn.Enable( pImpl->pWin->IsFileSelected() );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtDocumentTemplateDialog , DoubleClickHdl_Impl)
{
    EndDialog( RET_OK );

    if ( !pImpl->bSelectNoOpen )
        pImpl->pWin->OpenFile( !pImpl->pWin->IsTemplateFolderOpen() );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtDocumentTemplateDialog , NewFolderHdl_Impl)
{
    String aNewTitle( pImpl->aTitle );
    aNewTitle += String( ASCII_STR(" - ") );
    aNewTitle += pImpl->pWin->GetFolderTitle();
    SetText( aNewTitle );

    SelectHdl_Impl( NULL );
    return 0;
}

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtDocumentTemplateDialog , OrganizerHdl_Impl)
{
    Window* pOldDefWin = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    Reference < XFramesSupplier > xDesktop = Reference < XFramesSupplier >(
        ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XFrame > xFrame( xDesktop->getActiveFrame() );
    if ( !xFrame.is() )
        xFrame = Reference < XFrame >( xDesktop, UNO_QUERY );

    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = ASCII_STR("slot:5540");
    Reference< com::sun::star::util::XURLTransformer > xTrans(
                com::sun::star::util::URLTransformer::create( ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < XDispatch > xDisp;
    xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

    if ( xDisp.is() )
    {
        Sequence<PropertyValue> aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = ASCII_STR("Referer");
        pArg[0].Value <<= ASCII_STR("private:user");
        xDisp->dispatch( aTargetURL, aArgs );
    }

    Application::SetDefDialogParent( pOldDefWin );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtDocumentTemplateDialog, UpdateHdl_Impl, Timer*, _pEventSource )
{
    pImpl->pWin->SetFocus( sal_False );
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

// ------------------------------------------------------------------------

IMPL_LINK_NOARG(SvtDocumentTemplateDialog, OpenLinkHdl_Impl)
{
    ::rtl::OUString sURL( aMoreTemplatesLink.GetURL() );
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
                xSystemShell->execute( sURL, ::rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
            EndDialog( RET_CANCEL );
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
    return 0;
}

void SvtDocumentTemplateDialog::SelectTemplateFolder()
{
    pImpl->pWin->SelectFolder(ICON_POS_TEMPLATES);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
