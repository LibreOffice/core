/*************************************************************************
 *
 *  $RCSfile: templwin.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-07 14:37:11 $
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
#include "dynamicmenuoptions.hxx"
#include "xtextedt.hxx"
#include "txtattr.hxx"
#include "inettype.hxx"

#include "svtools.hrc"
#include "templwin.hrc"
#include "helpid.hrc"

#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
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
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSIST_HPP_
#include <com/sun/star/io/XPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
//!using namespace ::com::sun::star::util;
using namespace ::com::sun::star::view;

extern ::rtl::OUString CreateExactSizeText_Impl( sal_Int64 nSize ); // fileview.cxx

#define SPLITSET_ID         0
#define COLSET_ID           1
#define ICONWIN_ID          2
#define FILEWIN_ID          3
#define FRAMEWIN_ID         4

#define ICON_POS_NEWDOC     0
#define ICON_POS_TEMPLATES  1
#define ICON_POS_MYDOCS     2
#define ICON_POS_SAMPLES    3

DECLARE_LIST( NewDocList_Impl, ::rtl::OUString* );
DECLARE_LIST( HistoryList_Impl, String* );

enum SvtDocInfoType
{
    STRING_TYPE = 0,
    DATE_TYPE,
    SIZE_TYPE
};

struct SvtDocInfoMapping_Impl
{
    const sal_Char* _pPropName;
    USHORT          _nStringId;
    SvtDocInfoType  _eType;
};

static SvtDocInfoMapping_Impl __READONLY_DATA DocInfoMap_Impl[] =
{
    "Title",        DI_TITLE,           STRING_TYPE,
    "Author",       DI_FROM,            STRING_TYPE,
    "Size",         DI_SIZE,            SIZE_TYPE,
    "CreationDate", DI_DATE,            DATE_TYPE,
    "Keywords",     DI_KEYWORDS,        STRING_TYPE,
    "Description",  DI_DESCRIPTION,     STRING_TYPE,
    "MIMEType",     DI_MIMETYPE,        STRING_TYPE,
    "ModifyDate",   DI_MODIFIEDDATE,    DATE_TYPE,
    "ModifiedBy",   DI_MODIFIEDBY,      STRING_TYPE,
    "PrintDate",    DI_PRINTDATE,       DATE_TYPE,
    "PrintedBy",    DI_PRINTBY,         STRING_TYPE,
    "Theme",        DI_THEME,           STRING_TYPE,
    NULL,           0,                  STRING_TYPE
};

// class SvtIconWindow_Impl ----------------------------------------------

SvtIconWindow_Impl::SvtIconWindow_Impl( Window* pParent ) :

    Window( pParent ),

    aHeaderBar( this, 0 ),
    aIconCtrl( this, WB_3DLOOK | WB_ICON | WB_NOCOLUMNHEADER |
                     WB_HIGHLIGHTFRAME | WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP ),
    nMaxTextLength( 0 )

{
    Size aNewSize = aHeaderBar.CalcWindowSizePixel();
    aHeaderBar.SetSizePixel( aNewSize );
    aHeaderBar.SetBackground( Wallpaper( Color( COL_WHITE ) ) );
    aHeaderBar.Show();

    aIconCtrl.SetStyle( WB_3DLOOK | WB_ICON | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                        WB_NOSELECTION | WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN );
      aIconCtrl.SetHelpId( HID_TEMPLATEDLG_ICONCTRL );
    aIconCtrl.SetChoiceWithCursor( TRUE );
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
    Image aImage( SvtResId( IMG_SVT_NEWDOC ) );
    nMaxTextLength = aImage.GetSizePixel().Width();
    String aEntryStr = String( SvtResId( STR_SVT_NEWDOC ) );
    SvxIconChoiceCtrlEntry* pEntry =
        aIconCtrl.InsertEntry( aEntryStr, aImage, ICON_POS_NEWDOC );
    String* pURL = new String( RTL_CONSTASCII_USTRINGPARAM("private:newdoc") );
    pEntry->SetUserData( pURL );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    long nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    // "Templates"
    if ( aTemplateRootURL.Len() > 0 )
    {
        aEntryStr = String( SvtResId( STR_SVT_TEMPLATES ) );
        pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_TEMPLATES ) ), ICON_POS_TEMPLATES );
        pURL = new String( aTemplateRootURL );
        pEntry->SetUserData( pURL );
        DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
        nTemp = pEntry->GetBoundRect().GetSize().Width();
        if (nTemp > nMaxTextLength)
            nMaxTextLength = nTemp;
    }

    // "My Documents"
    aEntryStr = String( SvtResId( STR_SVT_MYDOCS ) );
    pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_MYDOCS ) ), ICON_POS_MYDOCS );
    pURL = new String( SvtPathOptions().GetWorkPath() );
    pEntry->SetUserData( pURL );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    // "Samples"
    aEntryStr = String( SvtResId( STR_SVT_SAMPLES ) );
    pEntry = aIconCtrl.InsertEntry( aEntryStr, Image( SvtResId( IMG_SVT_SAMPLES ) ), ICON_POS_SAMPLES );
    String aPath( RTL_CONSTASCII_USTRINGPARAM("$(insturl)/share/samples/$(vlang)") );
    pURL = new String( SvtPathOptions().SubstituteVariable( aPath ) );
    pEntry->SetUserData( pURL );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;
}

SvtIconWindow_Impl::~SvtIconWindow_Impl()
{
    for ( ULONG i = 0; i < aIconCtrl.GetEntryCount(); ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( i );
        delete (String*)pEntry->GetUserData();
    }
}

SvxIconChoiceCtrlEntry* SvtIconWindow_Impl::GetEntry( const String& rURL ) const
{
    SvxIconChoiceCtrlEntry* pEntry = NULL;
    for ( ULONG i = 0; i < aIconCtrl.GetEntryCount(); ++i )
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
    Size aHeaderSize = aHeaderBar.GetSizePixel();
    aHeaderSize.Width() = aWinSize.Width();
    aHeaderBar.SetSizePixel( aHeaderSize );
    aHeaderBar.SetItemSize( HBI_CATEGORY, aWinSize.Width() );
    long nHeaderHeight = aHeaderSize.Height();
    aWinSize.Height() -= nHeaderHeight;
    aIconCtrl.SetPosSizePixel( Point( 0, nHeaderHeight ), aWinSize );
    aIconCtrl.ArrangeIcons();
}

String SvtIconWindow_Impl::GetSelectedIconURL() const
{
    ULONG nPos;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetSelectedEntry( nPos );
    String aURL = *(String*)pEntry->GetUserData();
    return aURL;
}

String SvtIconWindow_Impl::GetSelectedIconText() const
{
    ULONG nPos;
    String aText = aIconCtrl.GetSelectedEntry( nPos )->GetText();
    return aText;
}

String SvtIconWindow_Impl::GetIconText( const String& rURL ) const
{
    String aText;
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( rURL );
    if ( pEntry )
        aText = pEntry->GetText();
    return aText;
}

void SvtIconWindow_Impl::InvalidateIconControl()
{
    aIconCtrl.Invalidate();
}

void SvtIconWindow_Impl::SetCursorPos( ULONG nPos )
{
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetEntry( nPos );
    aIconCtrl.SetCursor( pEntry );
}

void SvtIconWindow_Impl::SetFocus()
{
    aIconCtrl.GrabFocus();
}

// class SvtFileViewWindow_Impl -----------------------------------------_

SvtFileViewWindow_Impl::SvtFileViewWindow_Impl( Window* pParent ) :

    Window( pParent ),

    aFileView           ( this, SvtResId( CTRL_FILEVIEW ), FILEVIEW_SHOW_TITLE ),
    bIsTemplateFolder   ( sal_False )

{
    aFileView.SetHelpId( HID_TEMPLATEDLG_FILEVIEW );
    aFileView.Show();
    aFileView.SetPosPixel( Point( 0, 0 ) );
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
::com::sun::star::uno::Sequence< ::rtl::OUString > SvtFileViewWindow_Impl::GetNewDocContents() const
{
    NewDocList_Impl aNewDocs;
    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;
    aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aImageURL;
    ::rtl::OUString aTargetFrame;

    UINT32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry_Impl( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageURL );

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" ) ) ||
             aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:5500" ) ) )
            continue;
        else
        {
            // title
            String aRow = String( aTitle ).EraseAllChars( '~' );
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
            if ( aImageURL.getLength() > 0 )
            {
                aRow += '\t';
                aRow += String( aImageURL );
            }

            ::rtl::OUString* pRow = new ::rtl::OUString( aRow );
            aNewDocs.Insert( pRow, LIST_APPEND );
        }
    }

    nCount = aNewDocs.Count();
    Sequence < ::rtl::OUString > aRet( nCount );
    ::rtl::OUString* pRet = aRet.getArray();
    for ( i = 0; i < nCount; ++i )
    {
        ::rtl::OUString* pNewDoc = aNewDocs.GetObject(i);
        pRet[i] = *( pNewDoc );
        delete pNewDoc;
    }

    return aRet;
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
    aFolderURL = rURL;
    INetProtocol eProt = INetURLObject( rURL ).GetProtocol();
    bIsTemplateFolder = ( eProt == INET_PROT_VND_SUN_STAR_HIER );
    if ( eProt == INET_PROT_PRIVATE )
        aFileView.Initialize( rURL, GetNewDocContents() );
    else
        aFileView.Initialize( rURL, String() );
    aNewFolderLink.Call( this );
}

sal_Bool SvtFileViewWindow_Impl::HasPreviousLevel( String& rURL ) const
{
    return ( INetURLObject( aFileView.GetViewURL() ) != INetURLObject( aCurrentRootURL ) &&
             aFileView.HasPreviousLevel( rURL ) );
}

String SvtFileViewWindow_Impl::GetFolderTitle() const
{
    String aTitle;
    ::utl::UCBContentHelper::GetTitle( aFolderURL, aTitle );
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

const String& SvtDocInfoTable_Impl::GetString( long nId ) const
{
    USHORT nPos = FindIndex( nId );

    if ( RESARRAY_INDEX_NOTFOUND != nPos )
        return ResStringArray::GetString( nPos );
    else
        return aEmptyString;
}

// class SvtExtendedMultiLineEdit ----------------------------------------

SvtExtendedMultiLineEdit_Impl::SvtExtendedMultiLineEdit_Impl( Window* pParent ) :

    ExtMultiLineEdit( pParent, WB_LEFT | WB_VSCROLL | WB_READONLY )

{
    SetLeftMargin( 10 );
}

void SvtExtendedMultiLineEdit_Impl::Resize()
{
    ExtMultiLineEdit::Resize();
}

void SvtExtendedMultiLineEdit_Impl::InsertEntry( const String& rTitle, const String& rValue )
{
    String aText( '\n' );
    aText += rTitle;
    aText += ':';
    InsertText( aText );
    ULONG nPara = GetParagraphCount() - 1;
    SetAttrib( TextAttribFontWeight( WEIGHT_BOLD ), nPara, 0, aText.Len() );

    aText = '\n';
    aText += rValue;
    InsertText( aText );
    nPara = GetParagraphCount() - 1;
    SetAttrib( TextAttribFontWeight( WEIGHT_NORMAL ), nPara, 0, aText.Len() );

    InsertText( String( '\n' ) );
}

// class SvtFrameWindow_Impl ---------------------------------------------

SvtFrameWindow_Impl::SvtFrameWindow_Impl( Window* pParent ) :

    Window( pParent )

{
    // detect application language
    eLangType = SvtPathOptions().GetLanguageType();

    // create windows and frame
    pEditWin = new SvtExtendedMultiLineEdit_Impl( this );
    pEditWin->EnableCursor( FALSE );
    pTextWin = new Window( this );
    xFrame = ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > (
            ::comphelper::getProcessServiceFactory()->createInstance(
            String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame") ) ), ::com::sun::star::uno::UNO_QUERY );
    xWindow = VCLUnoHelper::GetInterface( pTextWin );
    xFrame->initialize( xWindow );

    // create docinfo instance
    xDocInfo = ::com::sun::star::uno::Reference < ::com::sun::star::io::XPersist > (
               ::comphelper::getProcessServiceFactory()->createInstance(
               String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.DocumentProperties") ) ), ::com::sun::star::uno::UNO_QUERY );
}

SvtFrameWindow_Impl::~SvtFrameWindow_Impl()
{
    delete pEditWin;
    xFrame->dispose();
}


void SvtFrameWindow_Impl::ShowDocInfo( const String& rURL )
{
    try
    {
        xDocInfo->read( rURL );
        Reference< XPropertySet > aPropSet( xDocInfo, UNO_QUERY );

        Reference< XMultiPropertySet > aMultiSet( xDocInfo, UNO_QUERY );
        if ( aMultiSet.is() )
        {
            String aText;
            Reference< XPropertySetInfo > aInfoSet = aMultiSet->getPropertySetInfo();
            if ( aInfoSet.is() )
            {
                Sequence< Property > aProps = aInfoSet->getProperties();
                const Property* pProps  = aProps.getConstArray();
                sal_uInt32 nCount = aProps.getLength();

                for ( sal_uInt32 i = 0; i < nCount; ++i )
                {
                    aText += String( pProps[i].Name );
                    aText += '\n';
                }
            }
        }

        if ( aPropSet.is() )
        {
            pEditWin->SetAutoScroll( FALSE );
            USHORT nIndex = 0;
            rtl::OUString aStringValue;
            ::com::sun::star::util::DateTime aDateValue;
            while ( DocInfoMap_Impl[ nIndex ]._pPropName )
            {
                SvtDocInfoType eInfoType = DocInfoMap_Impl[ nIndex ]._eType;
                Any aValue;
                if ( eInfoType != SIZE_TYPE )
                    aValue = aPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( DocInfoMap_Impl[ nIndex ]._pPropName ) );

                switch ( eInfoType )
                {
                    case STRING_TYPE :
                    {
                        if ( ( aValue >>= aStringValue ) && aStringValue.getLength() > 0 )
                        {
                            String aValueStr;
                            if ( DI_MIMETYPE == DocInfoMap_Impl[ nIndex ]._nStringId )
                            {
                                INetContentType eTypeID = INetContentTypes::GetContentTypeFromURL( rURL );
                                aValueStr = INetContentTypes::GetPresentation( eTypeID, eLangType );
                                if ( aValueStr.Len() == 0 )
                                    aValueStr = String( aStringValue );
                            }
                            else
                                aValueStr = String( aStringValue );
                            pEditWin->InsertEntry( aInfoTable.GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), aValueStr );
                        }
                        break;
                    }

                    case DATE_TYPE :
                    {
                        if ( aValue >>= aDateValue )
                        {
                            DateTime aToolsDT =
                                DateTime( Date( aDateValue.Day, aDateValue.Month, aDateValue.Year ),
                                Time( aDateValue.Hours, aDateValue.Minutes, aDateValue.Seconds, aDateValue.HundredthSeconds ) );
                            if ( aToolsDT.IsValid() )
                            {
                                LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
                                String aDateStr = aLocaleWrapper.getDate( aToolsDT );
                                aDateStr += String::CreateFromAscii( ", " );
                                aDateStr += aLocaleWrapper.getTime( aToolsDT );
                                pEditWin->InsertEntry( aInfoTable.GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), aDateStr );
                            }
                        }
                        break;
                    }

                    case SIZE_TYPE :
                    {
                        // size
                        ULONG nDocSize = ::utl::UCBContentHelper::GetSize( rURL );
                        pEditWin->InsertEntry( aInfoTable.GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), CreateExactSizeText_Impl( nDocSize ) );
                        break;
                    }
                }

                ++nIndex;
            }

            pEditWin->SetSelection( Selection( 0, 0 ) );
            pEditWin->SetAutoScroll( TRUE );
        }

        // info fields
        Reference< XNameContainer > aNameCnt( xDocInfo, UNO_QUERY );
        if ( aNameCnt.is() )
        {
            Sequence< ::rtl::OUString > aNameList = aNameCnt->getElementNames();
            const ::rtl::OUString* pNames  = aNameList.getConstArray();
            sal_uInt32 nCount = aNameList.getLength();

            for ( sal_uInt32 i = 0; i < nCount; ++i )
            {
                ::rtl::OUString aName = pNames[i], aStrVal;
                Any aValue = aNameCnt->getByName( aName );
                if ( ( aValue >>= aStrVal ) && aStrVal.getLength() > 0 )
                    pEditWin->InsertEntry( String( aName ), String( aStrVal ) );
            }
        }
    }
    catch ( ::com::sun::star::io::IOException& ) {}
    catch ( UnknownPropertyException& ) {}
    catch ( Exception& ) {}
}

void SvtFrameWindow_Impl::Resize()
{
    Size aWinSize = GetOutputSizePixel();
    pEditWin->SetSizePixel( aWinSize );
    pTextWin->SetSizePixel( aWinSize );
}

void SvtFrameWindow_Impl::OpenFile( const String& rURL, sal_Bool bPreview, sal_Bool bIsTemplate, sal_Bool bAsTemplate )
{
    if ( bPreview )
        aCurrentURL = rURL;

    pEditWin->Clear();

    if ( rURL.Len() > 0 && bPreview && xDocInfo.is() )
        ShowDocInfo( rURL );

    if ( rURL.Len() == 0 )
    {
        xFrame->setComponent( Reference < com::sun::star::awt::XWindow >(), Reference < XController >() );
    }
    else if ( !::utl::UCBContentHelper::IsFolder( rURL ) )
    {
        com::sun::star::util::URL aURL;
        aURL.Complete = rURL;
        Reference < com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer" ) ) ), UNO_QUERY );
        xTrans->parseStrict( aURL );

        String aTarget;
        Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
        if ( bPreview )
            aTarget = String( RTL_CONSTASCII_USTRINGPARAM("_self") );
        else
        {
            aTarget = String( RTL_CONSTASCII_USTRINGPARAM("_blank") );
            xProv = Reference < XDispatchProvider >( ::comphelper::getProcessServiceFactory()->createInstance(
                String( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ) ), UNO_QUERY );
        }

        Reference < XDispatch > xDisp = xProv.is() ?
            xProv->queryDispatch( aURL, aTarget, 0 ) : Reference < XDispatch >();

        if ( xDisp.is() )
        {
            if ( bPreview )
            {
                // disabling must be done here, does not work in ctor because
                // execute of the dialog will overwrite it
                // ( own execute method would help )
                pTextWin->EnableInput( FALSE, TRUE );
                if ( pTextWin->IsReallyVisible() )
                {
                    Sequence < PropertyValue > aArgs( 2 );
                    aArgs[0].Name = String( RTL_CONSTASCII_USTRINGPARAM("Preview") );
                    aArgs[0].Value <<= sal_True;
                    aArgs[1].Name = String( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") );
                    aArgs[1].Value <<= sal_True;
                    xDisp->dispatch( aURL, aArgs );
                }
            }
            else if ( bIsTemplate || bAsTemplate )
            {
                Sequence < PropertyValue > aArgs( 1 );
                aArgs[0].Name = String( RTL_CONSTASCII_USTRINGPARAM("AsTemplate") );
                aArgs[0].Value <<= bAsTemplate;
                xDisp->dispatch( aURL, aArgs );
            }
            else
                xDisp->dispatch( aURL, Sequence < PropertyValue >() );
        }
    }
}

void SvtFrameWindow_Impl::ToggleView( sal_Bool bDocInfo )
{
    if ( bDocInfo )
    {
        xWindow->setVisible( sal_False );
        pTextWin->Hide();
        pEditWin->Show();
    }
    else
    {
        pEditWin->Hide();
        xWindow->setVisible( sal_True );
        pTextWin->Show();
    }

    OpenFile( aCurrentURL, sal_True, sal_False, sal_False );
}

// class SvtTemplateWindow -----------------------------------------------

SvtTemplateWindow::SvtTemplateWindow( Window* pParent ) :

    Window( pParent, WB_3DLOOK ),

    aFileViewTB             ( this, SvtResId( TB_SVT_FILEVIEW ) ),
    aFrameWinTB             ( this, SvtResId( TB_SVT_FRAMEWIN ) ),
    aSplitWin               ( this, WB_DOCKBORDER | WB_FLATSPLITDRAW ),
    pHistoryList            ( NULL )

{
    // create windows
    pIconWin = new SvtIconWindow_Impl( this );
    pFileWin = new SvtFileViewWindow_Impl( this );
    pFrameWin = new SvtFrameWindow_Impl( this );

    // set handlers
    pIconWin->SetClickHdl( LINK( this, SvtTemplateWindow, IconClickHdl_Impl ) );
    pFileWin->SetSelectHdl( LINK( this, SvtTemplateWindow, FileSelectHdl_Impl ) );
    pFileWin->SetDoubleClickHdl( LINK( this, SvtTemplateWindow, FileDblClickHdl_Impl ) );
    pFileWin->SetNewFolderHdl( LINK( this, SvtTemplateWindow, NewFolderHdl_Impl ) );

    // create the split items
    aSplitWin.SetAlign( WINDOWALIGN_LEFT );
    long nWidth = pIconWin->GetMaxTextLength() * 8 / 7;
    aSplitWin.InsertItem( ICONWIN_ID, pIconWin, nWidth, SPLITWINDOW_APPEND, 0, SWIB_FIXED );
    aSplitWin.InsertItem( FILEWIN_ID, pFileWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    aSplitWin.InsertItem( FRAMEWIN_ID, pFrameWin, 50, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    aSplitWin.SetSplitHdl( LINK( this, SvtTemplateWindow, SplitHdl_Impl ) );

    // show the windows
    pIconWin->Show();
    pFileWin->Show();
    pFrameWin->Show();
    aSplitWin.Show();

    // initialize the timers
    aResetTimer.SetTimeout( 100 );
    aResetTimer.SetTimeoutHdl( LINK( this, SvtTemplateWindow, ResetHdl_Impl ) );
    aSelectTimer.SetTimeout( 200 );
    aSelectTimer.SetTimeoutHdl( LINK( this, SvtTemplateWindow, TimeoutHdl_Impl ) );

    // initialize the toolboxes
    Size aSize = aFileViewTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFileViewTB.SetPosSizePixel( Point( 0, 2 ), aSize );
    aFileViewTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
    aSize = aFrameWinTB.CalcWindowSizePixel();
    aSize.Height() += 4;
    aFrameWinTB.SetPosSizePixel( Point( 0, 2 ), aSize );
    aFrameWinTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
    Link aLink = LINK( this, SvtTemplateWindow, ClickHdl_Impl );

    aFileViewTB.SetClickHdl( aLink );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, FALSE );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, FALSE );
    aFileViewTB.Show();

    aFrameWinTB.SetClickHdl( aLink );
    aFrameWinTB.CheckItem( TI_DOCTEMPLATE_DOCINFO, TRUE );
    aFrameWinTB.Show();

    pFrameWin->ToggleView( sal_True );

    // open the template icon for default
    String aRootURL = pIconWin->GetTemplateRootURL();
    if ( aRootURL.Len() > 0 )
    {
        pFileWin->OpenRoot( aRootURL );
        pIconWin->SetCursorPos( ICON_POS_TEMPLATES );
    }
}

SvtTemplateWindow::~SvtTemplateWindow()
{
    delete pIconWin;
    delete pFileWin;
    delete pFrameWin;
    if ( pHistoryList )
    {
        for ( UINT32 i = 0; i < pHistoryList->Count(); ++i )
            delete pHistoryList->GetObject(i);
        delete pHistoryList;
    }
}

IMPL_LINK ( SvtTemplateWindow , IconClickHdl_Impl, SvtIconChoiceCtrl *, pCtrl )
{
    String aURL = pIconWin->GetSelectedIconURL();
    pFileWin->OpenRoot( aURL );
    pIconWin->InvalidateIconControl();
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
        aDoubleClickHdl.Call( this );
    }
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , NewFolderHdl_Impl, SvtFileView *, pView )
{
    String aTemp;
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, pFileWin->HasPreviousLevel( aTemp ) );
    pFrameWin->OpenFile( String(), sal_True, sal_False, sal_False );
    AppendHistoryURL( pFileWin->GetFolderURL() );
    aNewFolderHdl.Call( this );
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
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, !bIsFolder );
    if ( !bIsFolder && INetURLObject( aURL ).GetProtocol() != INET_PROT_PRIVATE )
        pFrameWin->OpenFile( aURL, sal_True, sal_False, sal_False );
    return 0;
}

IMPL_LINK ( SvtTemplateWindow , ClickHdl_Impl, ToolBox *, pToolBox )
{
    USHORT nId = pToolBox->GetCurItemId();
    switch( nId )
    {
        case TI_DOCTEMPLATE_BACK :
            OpenHistory();
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

        case TI_DOCTEMPLATE_DOCINFO :
        case TI_DOCTEMPLATE_PREVIEW :
        {
            pFrameWin->ToggleView( TI_DOCTEMPLATE_DOCINFO == nId );
            break;
        }
    }

    return 0;
}

IMPL_LINK ( SvtTemplateWindow , SplitHdl_Impl, SplitWindow *, EMPTYARG )
{
    Resize();
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

void SvtTemplateWindow::AppendHistoryURL( const String& rURL )
{
    if ( !pHistoryList )
        pHistoryList = new HistoryList_Impl;
    String* pEntry = new String( rURL );
    pHistoryList->Insert( pEntry, LIST_APPEND );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->Count() > 1 );
}

void SvtTemplateWindow::OpenHistory()
{
    DBG_ASSERT( pHistoryList && pHistoryList->Count() > 1, "invalid history list" );
    String* pEntry = pHistoryList->Remove( pHistoryList->Count() - 1 );
    pEntry = pHistoryList->Remove( pHistoryList->Count() - 1 );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->Count() > 1 );
    String aURL( *pEntry );
    delete pEntry;
    pFileWin->OpenFolder( aURL );
}

long SvtTemplateWindow::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();
    long nRet = 0;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nCode = rKeyCode.GetCode();

        if ( KEY_TAB == nCode )
        {
            if ( pIconWin->HasChildPathFocus() )
            {
                if ( !rKeyCode.GetModifier() )
                {
                    pFileWin->SetFocus();
                    nRet = 1;
                }
                else if ( rKeyCode.IsShift() )
                {
                    aSendFocusHdl.Call( this );
                    nRet = 1;
                }
            }
            else if ( pFileWin->HasChildPathFocus() )
            {
                if ( !rKeyCode.GetModifier() )
                {
                    aSendFocusHdl.Call( this );
                    nRet = 1;
                }
                else if ( rKeyCode.IsShift() )
                {
                    pIconWin->SetFocus();
                    nRet = 1;
                }
            }
        }
    }

    return nRet ? nRet : Window::PreNotify( rNEvt );
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
    aPos.X() += nItemSize;
    aFrameWinTB.SetPosPixel( aPos );

    Size aSize = aFileViewTB.GetSizePixel();
    aSize.Width() = nItemSize;
    aFileViewTB.SetSizePixel( aSize );

    aSize = aFrameWinTB.GetSizePixel();
    aSize.Width() = nWidth - nItemSize;
    aFrameWinTB.SetSizePixel( aSize );

    long nToolBoxHeight = aSize.Height();
    aSize = aWinSize;
    aSize.Height() -= nToolBoxHeight;
    aSplitWin.SetPosSizePixel( Point( 0, nToolBoxHeight  ), aSize );
}

String SvtTemplateWindow::GetSelectedFile() const
{
    return pFileWin->GetSelectedFile();
}

sal_Bool SvtTemplateWindow::IsFileSelected() const
{
    String aURL = pFileWin->GetSelectedFile();
    sal_Bool bRet = ( aURL.Len() > 0 && !::utl::UCBContentHelper::IsFolder( aURL ) );
    return bRet;
}

void SvtTemplateWindow::OpenFile( sal_Bool bNotAsTemplate )
{
    String aURL = pFileWin->GetSelectedFile();
    if ( aURL.Len() > 0 && !::utl::UCBContentHelper::IsFolder( aURL ) )
        pFrameWin->OpenFile( aURL, sal_False, pFileWin->IsTemplateFolder(), !bNotAsTemplate );
}

String SvtTemplateWindow::GetFolderTitle() const
{
    String aTitle, aTemp;
    if ( !pFileWin->HasPreviousLevel( aTemp ) )
        aTitle = pIconWin->GetIconText( pFileWin->GetRootURL() );
    else
        aTitle = pFileWin->GetFolderTitle();
    return aTitle;
}

void SvtTemplateWindow::SetFocus( sal_Bool bIconWin )
{
    if ( bIconWin )
        pIconWin->SetFocus();
    else
        pFileWin->SetFocus();
}

// struct SvtTmplDlg_Impl ------------------------------------------------

struct SvtTmplDlg_Impl
{
    SvtTemplateWindow*  pWin;
    String              aTitle;
    Timer               aUpdateTimer;
    sal_Bool            bSelectNoOpen;

    SvtTmplDlg_Impl( Window* pParent ) : pWin( new SvtTemplateWindow( pParent ) ), bSelectNoOpen( sal_False ) {}
    ~SvtTmplDlg_Impl() { delete pWin; }
};

// class SvtDocumentTemplateDialog ---------------------------------------

SvtDocumentTemplateDialog::SvtDocumentTemplateDialog( Window* _pParent, SelectOnly ) :
    ModalDialog( _pParent, SvtResId( DLG_DOCTEMPLATE ) ),

    aLine       ( this, ResId( FL_DOCTEMPLATE ) ),
    aManageBtn  ( this, ResId( BTN_DOCTEMPLATE_MANAGE ) ),
    aEditBtn    ( this, ResId( BTN_DOCTEMPLATE_EDIT ) ),
    aOKBtn      ( this, ResId( BTN_DOCTEMPLATE_OPEN ) ),
    aCancelBtn  ( this, ResId( BTN_DOCTEMPLATE_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_DOCTEMPLATE_HELP ) ),
    pImpl       ( NULL )
{
    FreeResource();
    InitImpl( );

    // no editing of templates
    aEditBtn.Hide();

    pImpl->bSelectNoOpen = sal_True;
}

SvtDocumentTemplateDialog::SvtDocumentTemplateDialog( Window* pParent ) :

    ModalDialog( pParent, SvtResId( DLG_DOCTEMPLATE ) ),

    aLine       ( this, ResId( FL_DOCTEMPLATE ) ),
    aManageBtn  ( this, ResId( BTN_DOCTEMPLATE_MANAGE ) ),
    aEditBtn    ( this, ResId( BTN_DOCTEMPLATE_EDIT ) ),
    aOKBtn      ( this, ResId( BTN_DOCTEMPLATE_OPEN ) ),
    aCancelBtn  ( this, ResId( BTN_DOCTEMPLATE_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_DOCTEMPLATE_HELP ) ),
    pImpl       ( NULL )
{
    FreeResource();
    InitImpl( );
}


void SvtDocumentTemplateDialog::InitImpl( )
{
    pImpl = new SvtTmplDlg_Impl( this );

    pImpl->aTitle = GetText();

    aManageBtn.SetClickHdl( LINK( this, SvtDocumentTemplateDialog, OrganizerHdl_Impl ) );
    Link aLink = LINK( this, SvtDocumentTemplateDialog, OKHdl_Impl );
    aEditBtn.SetClickHdl( aLink );
    aOKBtn.SetClickHdl( aLink );

    pImpl->pWin->SetSelectHdl( LINK( this, SvtDocumentTemplateDialog, SelectHdl_Impl ) );
    pImpl->pWin->SetDoubleClickHdl( LINK( this, SvtDocumentTemplateDialog, DoubleClickHdl_Impl ) );
    pImpl->pWin->SetNewFolderHdl( LINK( this, SvtDocumentTemplateDialog, NewFolderHdl_Impl ) );
    pImpl->pWin->SetSendFocusHdl( LINK( this, SvtDocumentTemplateDialog, SendFocusHdl_Impl ) );

    Size aSize = GetOutputSizePixel();
    Point aPos = aLine.GetPosPixel();
    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    aSize.Height() = aPos.Y() - a6Size.Height();
    aSize.Width() -= a6Size.Width();
    pImpl->pWin->SetPosSizePixel( Point( a6Size.Width() / 2, 0 ), aSize );
    pImpl->pWin->Show();

    SelectHdl_Impl( NULL );
    NewFolderHdl_Impl( NULL );

    pImpl->aUpdateTimer.SetTimeout( 100 );
    pImpl->aUpdateTimer.SetTimeoutHdl( LINK( this, SvtDocumentTemplateDialog, UpdateHdl_Impl ) );
    pImpl->aUpdateTimer.Start();
}

SvtDocumentTemplateDialog::~SvtDocumentTemplateDialog()
{
    delete pImpl;
}

sal_Bool SvtDocumentTemplateDialog::IsFileSelected( ) const
{
    return pImpl->pWin->IsFileSelected();
}

String SvtDocumentTemplateDialog::GetSelectedFileURL( ) const
{
    return pImpl->pWin->GetSelectedFile();
}

IMPL_LINK ( SvtDocumentTemplateDialog , SelectHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    sal_Bool bEnable = pImpl->pWin->IsFileSelected();
    aEditBtn.Enable( bEnable && pImpl->pWin->IsTemplateFolderOpen() );
    aOKBtn.Enable( bEnable );
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , DoubleClickHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    if ( !pImpl->bSelectNoOpen )
        pImpl->pWin->OpenFile( !pImpl->pWin->IsTemplateFolderOpen() );

    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , NewFolderHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    String aNewTitle( pImpl->aTitle );
    aNewTitle += String( RTL_CONSTASCII_USTRINGPARAM(" - ") );
    aNewTitle += pImpl->pWin->GetFolderTitle();
    SetText( aNewTitle );

    SelectHdl_Impl( NULL );
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , SendFocusHdl_Impl, SvtTemplateWindow *, EMPTYARG )
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
        if ( !pImpl->bSelectNoOpen )
            pImpl->pWin->OpenFile( &aEditBtn == pBtn );
        EndDialog( RET_OK );
    }
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , OrganizerHdl_Impl, PushButton *, pBtn )
{
    Window* pOldDefWin = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop =
            Reference < ::com::sun::star::frame::XFramesSupplier >( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop") ) ), UNO_QUERY );
    Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop->getActiveFrame() );
    if ( !xFrame.is() )
        xFrame = Reference < ::com::sun::star::frame::XFrame >( xDesktop, UNO_QUERY );

    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("slot:5540") );
    Reference < com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer") ) ), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < ::com::sun::star::frame::XDispatch > xDisp;
    xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

    if ( xDisp.is() )
    {
        Sequence<PropertyValue> aArgs(1);
        PropertyValue* pArg = aArgs.getArray();
        pArg[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Referer") );
        pArg[0].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("private:user") );
        xDisp->dispatch( aTargetURL, aArgs );
    }

    Application::SetDefDialogParent( pOldDefWin );
    return 0;
}

IMPL_LINK ( SvtDocumentTemplateDialog , UpdateHdl_Impl, Timer *, EMPTYARG )
{
    pImpl->pWin->SetFocus( sal_False );
    UpdateDocumentTemplates_Impl();
    return 0;
}

void SvtDocumentTemplateDialog::UpdateDocumentTemplates_Impl()
{
    ::rtl::OUString aService = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.DocumentTemplates" ) );
    Reference< XDocumentTemplates > xTemplates(
        ::comphelper::getProcessServiceFactory()->createInstance( aService ), UNO_QUERY );
    if ( xTemplates.is() )
    {
        WaitObject aWaitCursor( this );
        xTemplates->update();
    }
}

long SvtDocumentTemplateDialog::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();
    long nRet = 0;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nCode = rKeyCode.GetCode();

        if ( KEY_TAB == nCode )
        {
            if ( aHelpBtn.HasChildPathFocus() )
            {
                if ( !rKeyCode.GetModifier() )
                {
                    pImpl->pWin->SetFocus( sal_True );
                    nRet = 1;
                }
            }
            else if ( ( aEditBtn.IsEnabled() && aEditBtn.HasChildPathFocus() ) ||
                      ( !aEditBtn.IsEnabled() && aOKBtn.IsEnabled() && aOKBtn.HasChildPathFocus() ) ||
                      ( !aEditBtn.IsEnabled() && !aOKBtn.IsEnabled() && aCancelBtn.HasChildPathFocus() ) )
            {
                if ( rKeyCode.IsShift() )
                {
                    pImpl->pWin->SetFocus( sal_False );
                    nRet = 1;
                }
            }
        }
    }

    return nRet ? nRet : ModalDialog::PreNotify( rNEvt );
}

