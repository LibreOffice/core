/*************************************************************************
 *
 *  $RCSfile: templwin.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:36:25 $
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

#ifndef _SVTOOLS_TEMPLWIN_HXX
#include "templwin.hxx"
#endif
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include "templdlg.hxx"
#endif
#ifndef _SVTOOLS_SVTDATA_HXX
#include "svtdata.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include "pathoptions.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX
#include "dynamicmenuoptions.hxx"
#endif
#ifndef _XTEXTEDT_HXX
#include "xtextedt.hxx"
#endif

#ifndef _INETTYPE_HXX
#include "inettype.hxx"
#endif
#ifndef _SVTOOLS_IMAGEMGR_HXX
#include "imagemgr.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include "miscopt.hxx"
#endif
#ifndef SFX2_TEMPLATEFOLDERCACHE_HXX
#include "templatefoldercache.hxx"
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include "imgdef.hxx"
#endif
#ifndef _TXTATTR_HXX
#include "txtattr.hxx"
#endif
#ifndef _SVTOOLS_HRC
#include "svtools.hrc"
#endif
#ifndef _SVTOOLS_TEMPLWIN_HRC
#include "templwin.hrc"
#endif
#ifndef _SVT_HELPID_HRC
#include "helpid.hrc"
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include "pathoptions.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include "viewoptions.hxx"
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif

#include "unotools/configmgr.hxx"

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
#include "DocumentInfoPreview.hxx"
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::view;
using namespace svtools;

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

#define ASCII_STR(s)                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define VIEWSETTING_NEWFROMTEMPLATE     ASCII_STR("NewFromTemplate")
#define VIEWSETTING_SELECTEDGROUP       ASCII_STR("SelectedGroup")
#define VIEWSETTING_SELECTEDVIEW        ASCII_STR("SelectedView")
#define VIEWSETTING_SPLITRATIO          ASCII_STR("SplitRatio")
#define VIEWSETTING_LASTFOLDER          ASCII_STR("LastFolder")

struct FolderHistory
{
    String      m_sURL;
    ULONG       m_nGroup;

    FolderHistory( const String& _rURL, sal_Int32 _nGroup ) :
        m_sURL( _rURL ), m_nGroup( _nGroup ) {}
};

DECLARE_LIST( HistoryList_Impl, FolderHistory* );
DECLARE_LIST( NewDocList_Impl, ::rtl::OUString* );

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
        {"Title",       DI_TITLE,           STRING_TYPE},
        {"Author",      DI_FROM,            STRING_TYPE},
        {"Size",        DI_SIZE,            SIZE_TYPE},
        {"CreationDate",DI_DATE,            DATE_TYPE},
        {"Keywords",    DI_KEYWORDS,        STRING_TYPE},
        {"Description", DI_DESCRIPTION,     STRING_TYPE},
        {"MIMEType",    DI_MIMETYPE,        STRING_TYPE},
        {"ModifyDate",  DI_MODIFIEDDATE,    DATE_TYPE},
        {"ModifiedBy",  DI_MODIFIEDBY,      STRING_TYPE},
        {"PrintDate",   DI_PRINTDATE,       DATE_TYPE},
        {"PrintedBy",   DI_PRINTBY,         STRING_TYPE},
        {"Theme",       DI_THEME,           STRING_TYPE},
        {NULL,          0,                  STRING_TYPE}
};


ODocumentInfoPreview::ODocumentInfoPreview( Window* pParent ,WinBits _nBits) : Window(pParent,WB_DIALOGCONTROL)
{
    m_pEditWin = new SvtExtendedMultiLineEdit_Impl(this,_nBits);
    m_pEditWin->Show();
    m_pEditWin->EnableCursor( FALSE );
    m_pInfoTable = new SvtDocInfoTable_Impl();
    // detect application language
    m_aLocale = SvtPathOptions().GetLocale();
}
// -----------------------------------------------------------------------------
ODocumentInfoPreview::~ODocumentInfoPreview()
{
    delete m_pEditWin;
    delete m_pInfoTable;
}
// -----------------------------------------------------------------------------
void ODocumentInfoPreview::Resize()
{
    Size aOutputSize( GetOutputSize() );
    m_pEditWin->SetPosSizePixel( Point(0,0),aOutputSize);
}
// -----------------------------------------------------------------------------
void ODocumentInfoPreview::Clear()
{
    m_pEditWin->Clear();
}
// -----------------------------------------------------------------------------
void ODocumentInfoPreview::fill(const Reference< XPropertySet>& _xDocInfo,const String& rURL)
{
    Reference< XPropertySet > aPropSet( _xDocInfo, UNO_QUERY );

    Reference< XMultiPropertySet > aMultiSet( _xDocInfo, UNO_QUERY );
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
        m_pEditWin->SetAutoScroll( FALSE );
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
                            if ( eTypeID != CONTENT_TYPE_APP_OCTSTREAM )
                                aValueStr = INetContentTypes::GetPresentation( eTypeID, m_aLocale );
                            else
                                aValueStr = SvFileInformationManager::GetDescription( rURL );
                            if ( aValueStr.Len() == 0 )
                                aValueStr = String( aStringValue );
                        }
                        else
                            aValueStr = String( aStringValue );
                        m_pEditWin->InsertEntry( m_pInfoTable->GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), aValueStr );
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
                            aDateStr += String( RTL_CONSTASCII_STRINGPARAM(", ") );
                            aDateStr += aLocaleWrapper.getTime( aToolsDT );
                            m_pEditWin->InsertEntry( m_pInfoTable->GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), aDateStr );
                        }
                    }
                    break;
                }

                case SIZE_TYPE :
                {
                    // size
                    ULONG nDocSize = ::utl::UCBContentHelper::GetSize( rURL );
                    m_pEditWin->InsertEntry( m_pInfoTable->GetString( DocInfoMap_Impl[ nIndex ]._nStringId ), CreateExactSizeText_Impl( nDocSize ) );
                    break;
                }
            }

            ++nIndex;
        }

        m_pEditWin->SetSelection( Selection( 0, 0 ) );
        m_pEditWin->SetAutoScroll( TRUE );
    }
}
// -----------------------------------------------------------------------------
void ODocumentInfoPreview::InsertEntry( const String& rTitle, const String& rValue )
{
    m_pEditWin->InsertEntry( rTitle, rValue);
}
// -----------------------------------------------------------------------------

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
    nMaxTextLength( 0 ),
    aNewDocumentRootURL( ASCII_STR("private:newdoc") ),
    aMyDocumentsRootURL( SvtPathOptions().GetWorkPath() ),
    aSamplesFolderRootURL( SvtPathOptions().
        SubstituteVariable( String( ASCII_STR("$(insturl)/share/samples/$(vlang)") ) ) )

{
    aDummyHeaderBar.Show();

      aIconCtrl.SetHelpId( HID_TEMPLATEDLG_ICONCTRL );
    aIconCtrl.SetChoiceWithCursor( TRUE );
    aIconCtrl.SetSelectionMode( SINGLE_SELECTION );
    aIconCtrl.Show();

    // detect the root URL of templates
    Reference< XDocumentTemplates > xTemplates( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.frame.DocumentTemplates") ), UNO_QUERY );

    if ( xTemplates.is() )
    {
        Reference < XContent > aRootContent = xTemplates->getContent();
        Reference < XCommandEnvironment > aCmdEnv;

        if ( aRootContent.is() )
            aTemplateRootURL = aRootContent->getIdentifier()->getContentIdentifier();
    }

    // insert the categories
    // "New Document"
    sal_Bool bHiContrast = GetBackground().GetColor().IsDark();
    Image aImage( SvtResId( bHiContrast ? IMG_SVT_NEWDOC_HC : IMG_SVT_NEWDOC ) );
    nMaxTextLength = aImage.GetSizePixel().Width();
    String aEntryStr = String( SvtResId( STR_SVT_NEWDOC ) );
    SvxIconChoiceCtrlEntry* pEntry =
        aIconCtrl.InsertEntry( aEntryStr, aImage, ICON_POS_NEWDOC );
    pEntry->SetUserData( new String( aNewDocumentRootURL ) );
    pEntry->SetQuickHelpText( String( SvtResId( STR_SVT_NEWDOC_HELP ) ) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    long nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    // "Templates"
    if( aTemplateRootURL.Len() > 0 )
    {
        aEntryStr = String( SvtResId( STR_SVT_TEMPLATES ) );
        pEntry = aIconCtrl.InsertEntry(
            aEntryStr, Image( SvtResId( bHiContrast ? IMG_SVT_TEMPLATES_HC : IMG_SVT_TEMPLATES ) ), ICON_POS_TEMPLATES );
        pEntry->SetUserData( new String( aTemplateRootURL ) );
        pEntry->SetQuickHelpText( String( SvtResId( STR_SVT_TEMPLATES_HELP ) ) );
        DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
        nTemp = pEntry->GetBoundRect().GetSize().Width();
        if (nTemp > nMaxTextLength)
            nMaxTextLength = nTemp;
    }

    // "My Documents"
    aEntryStr = String( SvtResId( STR_SVT_MYDOCS ) );
    pEntry = aIconCtrl.InsertEntry(
        aEntryStr, Image( SvtResId( bHiContrast ? IMG_SVT_MYDOCS_HC : IMG_SVT_MYDOCS ) ), ICON_POS_MYDOCS );
    pEntry->SetUserData( new String( aMyDocumentsRootURL ) );
    pEntry->SetQuickHelpText( String( SvtResId( STR_SVT_MYDOCS_HELP ) ) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if( nTemp > nMaxTextLength )
        nMaxTextLength = nTemp;

    // "Samples"
    aEntryStr = String( SvtResId( STR_SVT_SAMPLES ) );
    pEntry = aIconCtrl.InsertEntry(
        aEntryStr, Image( SvtResId( bHiContrast ? IMG_SVT_SAMPLES_HC : IMG_SVT_SAMPLES ) ), ICON_POS_SAMPLES );
    pEntry->SetUserData( new String( aSamplesFolderRootURL ) );
    pEntry->SetQuickHelpText( String( SvtResId( STR_SVT_SAMPLES_HELP ) ) );
    DBG_ASSERT( !pEntry->GetBoundRect().IsEmpty(), "empty rectangle" );
    nTemp = pEntry->GetBoundRect().GetSize().Width();
    if (nTemp > nMaxTextLength)
        nMaxTextLength = nTemp;

    aIconCtrl.CreateAutoMnemonics();
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
    ULONG nPos;
    SvxIconChoiceCtrlEntry* pEntry = aIconCtrl.GetSelectedEntry( nPos );
    String aURL;
    if ( pEntry )
        aURL = *static_cast<String*>(pEntry->GetUserData());
    return aURL;
}

String SvtIconWindow_Impl::GetSelectedIconText() const
{
    ULONG nPos;
    return MnemonicGenerator::EraseAllMnemonicChars( aIconCtrl.GetSelectedEntry( nPos )->GetText() );
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

ULONG SvtIconWindow_Impl::GetCursorPos() const
{
    // FIXME -Wall cannot really return -1 actually returns high values.
    ULONG nPos = -1;

    SvxIconChoiceCtrlEntry* pCursorEntry = aIconCtrl.GetCursor( );
    if ( pCursorEntry )
        nPos = aIconCtrl.GetEntryListPos( pCursorEntry );

    return nPos;
}

ULONG SvtIconWindow_Impl::GetSelectEntryPos() const
{
    ULONG nPos;
    if ( !aIconCtrl.GetSelectedEntry( nPos ) )
        nPos = -1;
    return nPos;
}

void SvtIconWindow_Impl::SetCursorPos( ULONG nPos )
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
    ULONG nCount = aIconCtrl.GetEntryCount();
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

ULONG SvtIconWindow_Impl::GetRootPos( const String& rURL ) const
{
    ULONG nPos = -1;
    if ( aNewDocumentRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 0;
    else if ( aTemplateRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 1;
    else if ( aMyDocumentsRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 2;
    else if ( aSamplesFolderRootURL.Match( rURL ) == STRING_MATCH )
        nPos = 3;
    else
    {
        DBG_ERRORFILE( "SvtIconWindow_Impl::GetRootPos(): invalid position" );
    }

    return nPos;
}

void SvtIconWindow_Impl::UpdateIcons( sal_Bool _bHiContrast )
{
    aIconCtrl.GetEntry( ICON_POS_NEWDOC )->SetImage(
        Image( SvtResId( _bHiContrast ? IMG_SVT_NEWDOC_HC : IMG_SVT_NEWDOC ) ) );
    aIconCtrl.GetEntry( ICON_POS_TEMPLATES )->SetImage(
        Image( SvtResId( _bHiContrast ? IMG_SVT_TEMPLATES_HC : IMG_SVT_TEMPLATES ) ) );
    aIconCtrl.GetEntry( ICON_POS_MYDOCS )->SetImage(
        Image( SvtResId( _bHiContrast ? IMG_SVT_MYDOCS_HC : IMG_SVT_MYDOCS ) ) );
    aIconCtrl.GetEntry( ICON_POS_SAMPLES )->SetImage(
        Image( SvtResId( _bHiContrast ? IMG_SVT_SAMPLES_HC : IMG_SVT_SAMPLES ) ) );
}
/* -----------------27.11.2002 16:58-----------------
 *
 * --------------------------------------------------*/
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

SvtFileViewWindow_Impl::SvtFileViewWindow_Impl( SvtTemplateWindow* pParent, const String& rSamplesFolderURL ) :

    Window( pParent, WB_DIALOGCONTROL | WB_TABSTOP | WB_BORDER | WB_3DLOOK ),

    aFileView           ( this, SvtResId( CTRL_FILEVIEW ), FILEVIEW_SHOW_TITLE ),
    bIsTemplateFolder   ( sal_False ),
    aSamplesFolderURL   ( rSamplesFolderURL ),
    rParent( *pParent )

{
    aFileView.SetStyle( aFileView.GetStyle() | WB_DIALOGCONTROL | WB_TABSTOP );
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
Sequence< ::rtl::OUString > SvtFileViewWindow_Impl::GetNewDocContents() const
{
    NewDocList_Impl aNewDocs;
    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;
    aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aImageURL;
    ::rtl::OUString aTargetFrame;

    UINT32 i, nCount = aDynamicMenuEntries.getLength();
    ::rtl::OUString sSeparator( ASCII_STR("private:separator") );
    ::rtl::OUString sSlotURL( ASCII_STR("slot:5500") );

    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry_Impl( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageURL );

        if ( aURL == sSeparator || aURL == sSlotURL )
            continue;
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
    Size aWinSize = GetOutputSizePixel();

    static int  x = 0;
    static int  y = 0;

    aWinSize.nA += x;
    aWinSize.nB += y;
    aFileView.SetSizePixel( aWinSize );
}

String SvtFileViewWindow_Impl::GetSelectedFile() const
{
    return aFileView.GetCurrentURL();
}

void SvtFileViewWindow_Impl::OpenFolder( const String& rURL )
{
    aFolderURL = rURL;

    rParent.SetPrevLevelButtonState( rURL );

    INetProtocol eProt = INetURLObject( rURL ).GetProtocol();
    bIsTemplateFolder = ( eProt == INET_PROT_VND_SUN_STAR_HIER );
    if ( eProt == INET_PROT_PRIVATE )
    {
        aFileView.EnableNameReplacing( sal_False );
        aFileView.Initialize( rURL, GetNewDocContents() );
    }
    else
    {
        xub_StrLen nSampFoldLen = aSamplesFolderURL.Len();
        aFileView.EnableNameReplacing(
                    nSampFoldLen? rURL.CompareTo( aSamplesFolderURL, nSampFoldLen ) == COMPARE_EQUAL : sal_False );
        aFileView.Initialize( rURL, String() );
    }
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
// -----------------------------------------------------------------------------
// class SvtExtendedMultiLineEdit_Impl --------------------------------------------
SvtExtendedMultiLineEdit_Impl::SvtExtendedMultiLineEdit_Impl( Window* pParent,WinBits _nBits ) :

    ExtMultiLineEdit( pParent, _nBits )

{
    SetLeftMargin( 10 );
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------

const String& SvtDocInfoTable_Impl::GetString( long nId ) const
{
    USHORT nPos = FindIndex( nId );

    if ( RESARRAY_INDEX_NOTFOUND != nPos )
        return ResStringArray::GetString( nPos );
    else
        return aEmptyString;
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
    xDocInfo = Reference < XPersist > ( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.document.DocumentProperties") ), UNO_QUERY );

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
        xDocInfo->read( rURL );

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
    catch ( IOException& ) {}
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
    pEditWin->Clear();

    if ( rURL.Len() > 0 && bPreview && xDocInfo.is() )
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
        Reference < com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->
            createInstance( ASCII_STR("com.sun.star.util.URLTransformer" ) ), UNO_QUERY );
        xTrans->parseStrict( aURL );

        String aTarget;
        Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
        if ( bPreview )
            aTarget = ASCII_STR("_self");
        else
        {
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
                    pTextWin->EnableInput( FALSE, TRUE );
                    if ( pTextWin->IsReallyVisible() )
                    {
                        sal_Bool    b = sal_True;
                        Sequence < PropertyValue > aArgs( 3 );
                        aArgs[0].Name = ASCII_STR("Preview");
                        aArgs[0].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[1].Name = ASCII_STR("ReadOnly");
                        aArgs[1].Value.setValue( &b, ::getBooleanCppuType() );
                        aArgs[2].Name = ASCII_STR("AsTemplate");    // prevents getting an empty URL with getURL()!
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
                xDisp->dispatch( aURL, Sequence < PropertyValue >() );
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
    pFileWin = new SvtFileViewWindow_Impl( this, pIconWin->GetSamplesFolderURL() );
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
        for ( UINT32 i = 0; i < pHistoryList->Count(); ++i )
            delete pHistoryList->GetObject(i);
        delete pHistoryList;
    }
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , IconClickHdl_Impl, SvtIconChoiceCtrl *, pCtrl )
{
    String aURL = pIconWin->GetSelectedIconURL();
    if ( !aURL.Len() )
        aURL = pIconWin->GetCursorPosIconURL();
    if ( pFileWin->GetRootURL() != aURL )
    {
        pFileWin->OpenRoot( aURL );
        pIconWin->InvalidateIconControl();
        aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, FALSE );
    }
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , FileSelectHdl_Impl, SvtFileView *, pView )
{
    aSelectTimer.Start();
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , FileDblClickHdl_Impl, SvtFileView *, pView )
{
    if ( aSelectTimer.IsActive() )
        aSelectTimer.Stop();

    String aURL = pFileWin->GetSelectedFile();
    if ( ::utl::UCBContentHelper::IsFolder( aURL ) )
    {
        pFileWin->OpenFolder( aURL );
    }
    else
    {
        aDoubleClickHdl.Call( this );
    }
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , NewFolderHdl_Impl, SvtFileView *, pView )
{
    pFrameWin->OpenFile( String(), sal_True, sal_False, sal_False );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, FALSE );

    String sURL = pFileWin->GetFolderURL();
    ULONG nPos = pIconWin->GetRootPos( sURL );
    AppendHistoryURL( sURL, nPos );

    aNewFolderHdl.Call( this );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    aSelectHdl.Call( this );
    String aURL = pFileWin->GetSelectedFile();
    sal_Bool bIsFile = ( aURL.Len() != 0 && !::utl::UCBContentHelper::IsFolder( aURL ) &&
                         INetURLObject( aURL ).GetProtocol() != INET_PROT_PRIVATE );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, bIsFile );
    if ( bIsFile )
        pFrameWin->OpenFile( aURL, sal_True, sal_False, sal_False );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , ClickHdl_Impl, ToolBox *, pToolBox )
{
    DoAction( pToolBox->GetCurItemId() );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtTemplateWindow , ResizeHdl_Impl, SplitWindow *, EMPTYARG )
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

void SvtTemplateWindow::AppendHistoryURL( const String& rURL, ULONG nGroup )
{
    sal_Bool bInsert = sal_True;
    if ( !pHistoryList )
        pHistoryList = new HistoryList_Impl;
    else if ( pHistoryList->Count() > 0 )
    {
        FolderHistory* pLastEntry = pHistoryList->GetObject( pHistoryList->Count() - 1 );
        bInsert = ( rURL != pLastEntry->m_sURL);
    }

    if ( bInsert )
    {
        FolderHistory* pEntry = new FolderHistory( rURL, nGroup );
        pHistoryList->Insert( pEntry, LIST_APPEND );
        aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->Count() > 1 );
    }
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::OpenHistory()
{
    FolderHistory* pEntry = pHistoryList->Remove( pHistoryList->Count() - 1 );
    pEntry = pHistoryList->Remove( pHistoryList->Count() - 1 );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, pHistoryList->Count() > 1 );
    pFileWin->OpenFolder( pEntry->m_sURL );
    pIconWin->SetCursorPos( pEntry->m_nGroup );
    delete pEntry;
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::DoAction( USHORT nAction )
{
    switch( nAction )
    {
        case TI_DOCTEMPLATE_BACK :
        {
            if ( pHistoryList && pHistoryList->Count() > 1 )
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

    BOOL bFlat = ( SvtMiscOptions().GetToolboxStyle() == TOOLBOX_STYLE_FLAT );
    if ( bFlat )
    {
        aFileViewTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
        aFrameWinTB.SetOutStyle( TOOLBOX_STYLE_FLAT );
    }

    aFileViewTB.EnableItem( TI_DOCTEMPLATE_BACK, FALSE );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, FALSE );
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PRINT, FALSE );

    Link aLink = LINK( this, SvtTemplateWindow, ClickHdl_Impl );
    aFileViewTB.SetClickHdl( aLink );
    aFrameWinTB.SetClickHdl( aLink );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::InitToolBoxImages()
{
    SvtMiscOptions aMiscOpt;
    BOOL bLarge = ( aMiscOpt.GetSymbolSet() == SFX_SYMBOLS_LARGE );
    sal_Bool bHiContrast = aFileViewTB.GetBackground().GetColor().IsDark();

    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_BACK, Image( SvtResId(
        bLarge ? bHiContrast ? IMG_SVT_DOCTEMPL_HC_BACK_LARGE : IMG_SVT_DOCTEMPLATE_BACK_LARGE
               : bHiContrast ? IMG_SVT_DOCTEMPL_HC_BACK_SMALL : IMG_SVT_DOCTEMPLATE_BACK_SMALL ) ) );
    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_PREV, Image( SvtResId(
        bLarge ? bHiContrast ? IMG_SVT_DOCTEMPL_HC_PREV_LARGE : IMG_SVT_DOCTEMPLATE_PREV_LARGE
               : bHiContrast ? IMG_SVT_DOCTEMPL_HC_PREV_SMALL : IMG_SVT_DOCTEMPLATE_PREV_SMALL ) ) );
    aFileViewTB.SetItemImage( TI_DOCTEMPLATE_PRINT, Image( SvtResId(
        bLarge ? bHiContrast ? IMG_SVT_DOCTEMPL_HC_PRINT_LARGE : IMG_SVT_DOCTEMPLATE_PRINT_LARGE
               : bHiContrast ? IMG_SVT_DOCTEMPL_HC_PRINT_SMALL : IMG_SVT_DOCTEMPLATE_PRINT_SMALL ) ) );

    aFrameWinTB.SetItemImage( TI_DOCTEMPLATE_DOCINFO, Image( SvtResId(
        bLarge ? bHiContrast ? IMG_SVT_DOCTEMPL_HC_DOCINFO_LARGE : IMG_SVT_DOCTEMPLATE_DOCINFO_LARGE
               : bHiContrast ? IMG_SVT_DOCTEMPL_HC_DOCINFO_SMALL : IMG_SVT_DOCTEMPLATE_DOCINFO_SMALL ) ) );
    aFrameWinTB.SetItemImage( TI_DOCTEMPLATE_PREVIEW, Image( SvtResId(
        bLarge ? bHiContrast ? IMG_SVT_DOCTEMPL_HC_PREVIEW_LARGE : IMG_SVT_DOCTEMPLATE_PREVIEW_LARGE
               : bHiContrast ? IMG_SVT_DOCTEMPL_HC_PREVIEW_SMALL : IMG_SVT_DOCTEMPLATE_PREVIEW_SMALL ) ) );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::UpdateIcons()
{
    pIconWin->UpdateIcons( aFileViewTB.GetBackground().GetColor().IsDark() );
}

// ------------------------------------------------------------------------

long SvtTemplateWindow::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();
    long nRet = 0;

    if ( EVENT_KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        USHORT nCode = rKeyCode.GetCode();

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
    aFileViewTB.EnableItem( TI_DOCTEMPLATE_PREV, !pIconWin->IsRootURL( rURL ) );
}

// ------------------------------------------------------------------------

void SvtTemplateWindow::ClearHistory()
{
    if( pHistoryList )
        pHistoryList->Clear();
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
    aFrameWinTB.CheckItem( (sal_uInt16)nSelectedView, TRUE );

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
    if ( sLastFolder.getLength() > 0 )
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
/* -----------------27.11.2002 17:20-----------------
 *
 * --------------------------------------------------*/

void SvtTemplateWindow::SelectFolder(sal_Int32 nFolderPosition)
{
    pIconWin->SelectFolder(nFolderPosition);
}
// struct SvtTmplDlg_Impl ------------------------------------------------

struct SvtTmplDlg_Impl
{
    SvtTemplateWindow*  pWin;
    String              aTitle;
    Timer               aUpdateTimer;
    sal_Bool            bSelectNoOpen;

    SvtTmplDlg_Impl( Window* pParent ) : pWin( new SvtTemplateWindow( pParent ) ) ,bSelectNoOpen( sal_False ) {}

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

// ------------------------------------------------------------------------

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

// ------------------------------------------------------------------------

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

    // dynamic height adjustment
    long nHeight = pImpl->pWin->CalcHeight();

    Size aSize = GetOutputSizePixel();
    Point aPos = aLine.GetPosPixel();
    Size a6Size = LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    aPos.Y() -= a6Size.Height();
    long nDelta = aPos.Y() - nHeight;
    aSize.Height() -= nDelta;
    SetOutputSizePixel( aSize );

    aSize.Height() = nHeight;
    aSize.Width() -= a6Size.Width();
    pImpl->pWin->SetPosSizePixel( Point( a6Size.Width() / 2, 0 ), aSize );

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

sal_Bool SvtDocumentTemplateDialog::IsFileSelected( ) const
{
    return pImpl->pWin->IsFileSelected();
}

// ------------------------------------------------------------------------

String SvtDocumentTemplateDialog::GetSelectedFileURL( ) const
{
    return pImpl->pWin->GetSelectedFile();
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtDocumentTemplateDialog , SelectHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    sal_Bool bEnable = pImpl->pWin->IsFileSelected();
    aEditBtn.Enable( bEnable && pImpl->pWin->IsTemplateFolderOpen() );
    aOKBtn.Enable( bEnable );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtDocumentTemplateDialog , DoubleClickHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    EndDialog( RET_OK );

    if ( !pImpl->bSelectNoOpen )
        pImpl->pWin->OpenFile( !pImpl->pWin->IsTemplateFolderOpen() );
    return 0;
}

// ------------------------------------------------------------------------

IMPL_LINK ( SvtDocumentTemplateDialog , NewFolderHdl_Impl, SvtTemplateWindow *, EMPTYARG )
{
    String aNewTitle( pImpl->aTitle );
    aNewTitle += String( ASCII_STR(" - ") );
    aNewTitle += pImpl->pWin->GetFolderTitle();
    SetText( aNewTitle );

    SelectHdl_Impl( NULL );
    return 0;
}

// ------------------------------------------------------------------------

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

IMPL_LINK ( SvtDocumentTemplateDialog , OrganizerHdl_Impl, PushButton *, pBtn )
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
    Reference < com::sun::star::util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.util.URLTransformer") ), UNO_QUERY );
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
    Reference< XDocumentTemplates > xTemplates( ::comphelper::getProcessServiceFactory()->
        createInstance( ASCII_STR("com.sun.star.frame.DocumentTemplates") ), UNO_QUERY );
    if ( xTemplates.is() )
    {
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
    }
    return 0;
}
/* -----------------27.11.2002 16:54-----------------
 *
 * --------------------------------------------------*/
void SvtDocumentTemplateDialog::SelectTemplateFolder()
{
    pImpl->pWin->SelectFolder(ICON_POS_TEMPLATES);
}

