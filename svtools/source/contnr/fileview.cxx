/*************************************************************************
 *
 *  $RCSfile: fileview.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: pb $ $Date: 2001-07-11 08:55:33 $
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

#include "fileview.hxx"
#include "svtdata.hxx"
#include "imagemgr.hxx"
#include "headbar.hxx"
#include "svtabbx.hxx"

#include "svtools.hrc"

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPAREFACTORY_HPP_
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::comphelper;
using namespace ::rtl;
using namespace ::ucb;

DECLARE_LIST( StringList_Impl, OUString* );

#define ROW_HEIGHT  17  // the height of a row has to be a little higher than the bitmap

// functions -------------------------------------------------------------

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

void AppendDateTime_Impl( const ::com::sun::star::util::DateTime& rDT,
                          const ::com::sun::star::lang::Locale& rLocale, String& rRow )
{
    DateTime aDT;
    CONVERT_DATETIME( rDT, aDT );
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), rLocale );
    String aDateStr = aLocaleWrapper.getDate( aDT );
    aDateStr += String::CreateFromAscii( ", " );
    aDateStr += aLocaleWrapper.getTime( aDT );
    rRow += aDateStr;
}

String CreateExactSizeText_Impl( ULONG nSize )
{
    String aUnitStr = ' ';
    aUnitStr += String( SvtResId( STR_SVT_BYTES ) );
    double fSize = nSize;
    int nDec = 0;
    ULONG nMega = 1024 * 1024;
    ULONG nGiga = nMega * 1024;

    if ( nSize >= 10000 && nSize < nMega )
    {
        fSize /= 1024;
        aUnitStr = ' ';
        aUnitStr += String( SvtResId( STR_SVT_KB ) );
        nDec = 0;
    }
    else if ( nSize >= nMega && nSize < nGiga )
    {
        fSize /= nMega;
        aUnitStr = ' ';
        aUnitStr += String( SvtResId( STR_SVT_MB ) );
        nDec = 2;
    }
    else if ( nSize >= nGiga )
    {
        fSize /= nGiga;
        aUnitStr = ' ';
        aUnitStr += String( SvtResId( STR_SVT_GB ) );
        nDec = 3;
    }
    String aSizeStr;
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    SolarMath::DoubleToString(  aSizeStr, fSize, 'F', nDec, aLocaleWrapper.getNumDecimalSep().GetChar(0) );
    aSizeStr += aUnitStr;
    return aSizeStr;
}

// -----------------------------------------------------------------------

Sequence < OUString > GetFolderContentProperties_Impl( const String& rFolder, const ::com::sun::star::lang::Locale& rLocale, sal_Bool bFolder )
{
    StringList_Impl* pProperties = NULL;
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );
    try
    {
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
                   xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );

        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ),
                      new CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() ) );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(6);
        OUString* pProps = aProps.getArray();
        pProps[0] = OUString::createFromAscii( "Title" );
        pProps[1] = OUString::createFromAscii( "ContentType" );
        pProps[2] = OUString::createFromAscii( "Size" );
        pProps[3] = OUString::createFromAscii( "DateModified" );
        pProps[4] = OUString::createFromAscii( "IsFolder" );
        pProps[5] = OUString::createFromAscii( "TargetURL" );

        try
        {
            Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
            ResultSetInclude eInclude = bFolder ? INCLUDE_FOLDERS_AND_DOCUMENTS : INCLUDE_DOCUMENTS_ONLY;
            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            Reference < com::sun::star::ucb::XAnyCompareFactory > xFactory;
            Reference < XMultiServiceFactory > xMgr = getProcessServiceFactory();
            Reference < com::sun::star::ucb::XSortedDynamicResultSetFactory > xSRSFac(
                xMgr->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), UNO_QUERY );

            Sequence< com::sun::star::ucb::NumberedSortingInfo > aSortInfo( 2 );
            com::sun::star::ucb::NumberedSortingInfo* pInfo = aSortInfo.getArray();
            pInfo[ 0 ].ColumnIndex = 5;
            pInfo[ 0 ].Ascending   = sal_False;
            pInfo[ 1 ].ColumnIndex = 1;
            pInfo[ 1 ].Ascending   = sal_True;

            Reference< com::sun::star::ucb::XDynamicResultSet > xDynamicResultSet;
            xDynamicResultSet =
                xSRSFac->createSortedDynamicResultSet( xDynResultSet, aSortInfo, xFactory );
            if ( xDynamicResultSet.is() )
            {
                sal_Int16 nCaps = xDynamicResultSet->getCapabilities();
                xResultSet = xDynamicResultSet->getStaticResultSet();
            }

//          if ( xDynResultSet.is() )
//              xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( ::com::sun::star::uno::Exception& )
        {
            DBG_ERRORFILE( "createCursor: Any other exception" );
        }

        if ( xResultSet.is() )
        {
            pProperties = new StringList_Impl;
            Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            ULONG nFolderPos = LIST_APPEND;

            try
            {
                while ( xResultSet->next() )
                {
                    String aTitle( xRow->getString(1) );
                    String aType( xRow->getString(2) );
                    sal_Int64 nSize = xRow->getLong(3);
                    ::com::sun::star::util::DateTime aDT = xRow->getTimestamp(4);
                    sal_Bool bFolder = xRow->getBoolean(5);
                    String aTargetURL( xRow->getString(6) );
                    sal_Bool bTarget = aTargetURL.Len() > 0;
                    String aContentURL( xContentAccess->queryContentIdentifierString() );

                    if ( bTarget &&
                         INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                    {
                        Content aCnt( aTargetURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
                        aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= nSize;
                        aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDT;
                    }
                    String aRow = aTitle;
                    aRow += '\t';
//!                 aRow += aType;
//!                 aRow += '\t';
                    aRow += String::CreateFromInt64( nSize );
                    aRow += '\t';
                    AppendDateTime_Impl( aDT, rLocale, aRow );
                    aRow += '\t';
                    if ( aTargetURL.Len() > 0 )
                        aRow += aTargetURL;
                    else
                        aRow += aContentURL;
                    aRow += '\t';
                    aRow += bFolder ? '1' : '0';
                    OUString* pRow = new OUString( aRow );
                    ULONG nPos = LIST_APPEND;
                    if ( bFolder )
                    {
                        if ( LIST_APPEND == nFolderPos )
                            nFolderPos = 0;
                        else
                            nFolderPos++;
                        nPos = nFolderPos;
                    }
                    pProperties->Insert( pRow, nPos );
                }
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "GetFolderContents: CommandAbortedException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }

    if ( pProperties )
    {
        ULONG nCount = pProperties->Count();
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( ULONG i = 0; i < nCount; ++i )
        {
            OUString* pProperty = pProperties->GetObject(i);
            pRet[i] = *( pProperty );
            delete pProperty;
        }
        delete pProperties;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

// class ViewTabListBox_Impl ---------------------------------------------

class ViewTabListBox_Impl : public SvHeaderTabListBox
{
private:
    HeaderBar*      mpHeaderBar;
    sal_Bool        mbResizeDisabled;
    sal_Bool        mbAutoResize;

    DECL_LINK( HeaderSelect_Impl, HeaderBar * );
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );

public:
    ViewTabListBox_Impl( Window* pParent, sal_Int16 nFlags );
    ~ViewTabListBox_Impl();

    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKEvt );

    void            ClearAll();
    void            EnableAutoResize() { mbAutoResize = sal_True; }
};

// -----------------------------------------------------------------------

ViewTabListBox_Impl::ViewTabListBox_Impl( Window* pParent, sal_Int16 nFlags ) :

    SvHeaderTabListBox( pParent, 0 ),

    mpHeaderBar     ( NULL ),
    mbResizeDisabled( sal_False ),
    mbAutoResize    ( sal_False )

{
    Size aBoxSize = pParent->GetSizePixel();
    mpHeaderBar = new HeaderBar( pParent, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), mpHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = ( HIB_LEFT | HIB_VCENTER );
    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        mpHeaderBar->InsertItem( 1, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 180, nBits );
        mpHeaderBar->InsertItem( 2, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TYPE ) ), 140, nBits );
        mpHeaderBar->InsertItem( 3, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_SIZE ) ), 80, nBits );
        mpHeaderBar->InsertItem( 4, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_DATE ) ), 500, nBits );
    }
    else
        mpHeaderBar->InsertItem( 1, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 600, nBits );

    mpHeaderBar->SetSelectHdl( LINK( this, ViewTabListBox_Impl, HeaderSelect_Impl ) );
    mpHeaderBar->SetEndDragHdl( LINK( this, ViewTabListBox_Impl, HeaderEndDrag_Impl ) );

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                     Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    InitHeaderBar( mpHeaderBar );
    SetHighlightRange();
    SetEntryHeight( ROW_HEIGHT );

    Show();
    mpHeaderBar->Show();
}

// -----------------------------------------------------------------------

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    delete mpHeaderBar;
}

// -----------------------------------------------------------------------

IMPL_LINK( ViewTabListBox_Impl, HeaderSelect_Impl, HeaderBar*, pBar )
{
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( ViewTabListBox_Impl, HeaderEndDrag_Impl, HeaderBar*, pBar )
{
    if ( pBar && !pBar->GetCurItemId() )
        return 0;

    if ( !mpHeaderBar->IsItemMode() )
    {
        Size aSize;
        USHORT nTabs = mpHeaderBar->GetItemCount();
        long nTmpSize = 0;

        for ( USHORT i = 1; i <= nTabs; ++i )
        {
            long nWidth = mpHeaderBar->GetItemSize(i);
            aSize.Width() =  nWidth + nTmpSize;
            nTmpSize += nWidth;
            SetTab( i, aSize.Width(), MAP_PIXEL );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::Resize()
{
    SvTabListBox::Resize();
    Size aBoxSize = Control::GetParent()->GetOutputSizePixel();

    if ( mbResizeDisabled || !aBoxSize.Width() )
        return;

    Size aBarSize = mpHeaderBar->GetSizePixel();
    aBarSize.Width() = mbAutoResize ? aBoxSize.Width() : GetSizePixel().Width();
    mpHeaderBar->SetSizePixel( aBarSize );

    if ( mbAutoResize )
    {
        mbResizeDisabled = sal_True;
        Point aPos = GetPosPixel();
        SetPosSizePixel( Point( 0, aBarSize.Height() ),
                         Size( aBoxSize.Width(), aBoxSize.Height() - aBarSize.Height() ) );
        mbResizeDisabled = sal_False;
    }
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::KeyInput( const KeyEvent& rKEvt )
{
    if ( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
        GetDoubleClickHdl().Call( this );
    else
        SvHeaderTabListBox::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::ClearAll()
{
    for ( USHORT i = 0; i < GetEntryCount(); ++i )
        delete (SvtContentEntry*)GetEntry(i)->GetUserData();
    Clear();
}

// class SvtFileView -----------------------------------------------------

void SvtFileView::OpenFolder( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aContents )
{
    mpView->ClearAll();
    const ::rtl::OUString* pFileProperties  = aContents.getConstArray();
    UINT32 i, nCount = aContents.getLength();
    sal_Bool bExecFilter = ( maCurrentFilter.Len() > 0 ) && ( maCurrentFilter != maAllFilter );
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pFileProperties[i] );
        // extract columns
        String aTitle, aSize, aDate, aURL, aImageURL;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aSize = aRow.GetToken( 0, '\t', nIdx );
        aDate = aRow.GetToken( 0, '\t', nIdx );
        aURL = aRow.GetToken( 0, '\t', nIdx );
        sal_Unicode cFolder = aRow.GetToken( 0, '\t', nIdx ).GetChar(0);
        sal_Bool bIsFolder = ( '1' == cFolder );
        if ( nIdx != STRING_NOTFOUND )
            aImageURL = aRow.GetToken( 0, '\t', nIdx );

        if ( mbOnlyFolder && !bIsFolder )
            continue;

        // build new row
        String aNewRow = aTitle;
        aNewRow += '\t';
        aNewRow += SvFileInformationManager::GetDescription( aURL );
        aNewRow += '\t';
        if ( !bIsFolder && aSize.Len() > 0 )
        {
            // folder haven't a size
            BigInt nSize( aSize );
            aNewRow += CreateExactSizeText_Impl( nSize );
        }
        aNewRow += '\t';
        aNewRow += aDate;
        // detect image
        sal_Bool bDoInsert = sal_True;
        Image aImage;
        if ( bIsFolder )
            aImage = maFolderImage;
        else
        {
            INetURLObject aObj( aImageURL.Len() > 0 ? aImageURL : aURL );
            aImage = SvFileInformationManager::GetImage( aObj, FALSE );

            if ( bExecFilter )
            {
                bDoInsert = sal_False;
                String aExtension = aObj.getExtension();
                aExtension.ToLowerAscii();
                USHORT nCount = maCurrentFilter.GetTokenCount();

                for ( USHORT i = 0; i < nCount; ++i )
                {
                    String aFilter = maCurrentFilter.GetToken(i).Copy(2);
                    if ( aFilter == aExtension )
                    {
                        bDoInsert = sal_True;
                        break;
                    }
                }
            }
        }

        if ( bDoInsert )
        {
            // insert entry and set user data
            SvLBoxEntry* pEntry = mpView->InsertEntry( aNewRow, aImage, aImage );
            SvtContentEntry* pUserData = new SvtContentEntry( aURL, bIsFolder );
            pEntry->SetUserData( pUserData );
        }
    }

    mpView->SelectAll( FALSE );
    SvLBoxEntry* pFirst = mpView->First();
    if ( pFirst )
        mpView->SetCursor( pFirst, TRUE );
}

// -----------------------------------------------------------------------

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId,
                          sal_Bool bOnlyFolder, sal_Bool bMultiSelection ) :

    Control( pParent, rResId ),

    mbOnlyFolder    ( bOnlyFolder ),
    mbAutoResize    ( sal_False ),
    maFolderImage   ( SvtResId( IMG_SVT_FOLDER ) )

{
    sal_Int8 nFlags = FILEVIEW_SHOW_ALL;
    if ( bOnlyFolder )
        nFlags |= FILEVIEW_ONLYFOLDER;
    if ( bMultiSelection )
        nFlags |= FILEVIEW_MULTISELECTION;
    mpView = new ViewTabListBox_Impl( this, nFlags );
    long pTabs[] = { 5, 20, 180, 320, 400, 600 };
    mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    mpView->SetTabJustify( 2, AdjustRight ); // column "Size"
    maAllFilter = String::CreateFromAscii( "*.*" );
    if ( bMultiSelection )
        mpView->SetSelectionMode( MULTIPLE_SELECTION );
}

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId, sal_Int8 nFlags ) :

    Control( pParent, rResId ),

    mbOnlyFolder    ( ( nFlags & FILEVIEW_ONLYFOLDER ) == FILEVIEW_ONLYFOLDER ),
    mbAutoResize    ( sal_False ),
    maFolderImage   ( SvtResId( IMG_SVT_FOLDER ) )

{
    mpView = new ViewTabListBox_Impl( this, nFlags );
    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        long pTabs[] = { 5, 20, 180, 320, 400, 600 };
        mpView->SetTabs( &pTabs[0], MAP_PIXEL );
        mpView->SetTabJustify( 2, AdjustRight ); // column "Size"
    }
    else
    {
        // show only title
        long pTabs[] = { 2, 20, 600 };
        mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    }

    maAllFilter = String::CreateFromAscii( "*.*" );
    if ( ( nFlags & FILEVIEW_MULTISELECTION ) == FILEVIEW_MULTISELECTION )
        mpView->SetSelectionMode( MULTIPLE_SELECTION );
}

// -----------------------------------------------------------------------

SvtFileView::~SvtFileView()
{
    delete mpView;
}

// -----------------------------------------------------------------------

String SvtFileView::GetURL( SvLBoxEntry* pEntry ) const
{
    String aURL;
    if ( pEntry && pEntry->GetUserData() )
        aURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;
    return aURL;
}

// -----------------------------------------------------------------------

String SvtFileView::GetCurrentURL() const
{
    String aURL;
    SvLBoxEntry* pEntry = mpView->FirstSelected();
    if ( pEntry && pEntry->GetUserData() )
        aURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;
    return aURL;
}

// -----------------------------------------------------------------------

void SvtFileView::CreateNewFolder( const String& rNewFolder )
{
    INetURLObject aObj( maViewURL );
    aObj.insertName( rNewFolder, false, INetURLObject::LAST_SEGMENT,
                     true, INetURLObject::ENCODE_ALL );
    String aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
    if ( ::utl::UCBContentHelper::MakeFolder( aURL ) )
    {
        String aEntry = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
        SvLBoxEntry* pEntry = mpView->InsertEntry( aEntry, maFolderImage, maFolderImage );
        SvtContentEntry* pUserData = new SvtContentEntry( aURL, TRUE );
        pEntry->SetUserData( pUserData );
        mpView->MakeVisible( pEntry );
    }
    else
    {
        String aPath;
        if ( !::utl::LocalFileHelper::ConvertURLToSystemPath( aURL, aPath ) )
            aPath = aURL;
        String aErrorText( SvtResId( STR_SVT_FILEVIEW_ERR_MAKEFOLDER ) );
        String aVar( RTL_CONSTASCII_USTRINGPARAM("%1") );
        aErrorText.SearchAndReplace( aVar, aPath );
        ErrorBox aBox( this, WB_OK, aErrorText );
        aBox.Execute();
    }
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::HasPreviousLevel( String& rParentURL ) const
{
    sal_Bool bRet = sal_False;
    Content aCnt( maViewURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
    Reference< XContent > xContent( aCnt.get() );
    Reference< com::sun::star::container::XChild > xChild( xContent, UNO_QUERY );
    if ( xChild.is() )
    {
        Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
        if ( xParent.is() )
        {
            rParentURL = String( xParent->getIdentifier()->getContentIdentifier() );
            bRet = ( rParentURL.Len() > 0 && rParentURL != maViewURL );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::PreviousLevel( String& rNewURL )
{
    sal_Bool bRet = sal_False;
    if ( HasPreviousLevel( rNewURL ) )
    {
        Initialize( rNewURL, maCurrentFilter );
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void SvtFileView::SetHelpId( sal_uInt32 nHelpId )
{
    mpView->SetHelpId( nHelpId );
}

// -----------------------------------------------------------------------

void SvtFileView::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    mpView->SetSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

void SvtFileView::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}

// -----------------------------------------------------------------------

void SvtFileView::Initialize( const String& rURL, const String& rFilter )
{
    maViewURL = rURL;
    maCurrentFilter = rFilter;
    maCurrentFilter.ToLowerAscii();
    Sequence< ::rtl::OUString > aFileProperties =
        GetFolderContentProperties_Impl( maViewURL, Application::GetSettings().GetLocale(), sal_True );
    OpenFolder( aFileProperties );
    maOpenDoneLink.Call( this );
}

// -----------------------------------------------------------------------

void SvtFileView::Initialize( const String& rURL, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aContents )
{
    maViewURL = rURL;
    maCurrentFilter = maAllFilter;
    OpenFolder( aContents );
    maOpenDoneLink.Call( this );
}

// -----------------------------------------------------------------------

void SvtFileView::ExecuteFilter( const String& rFilter )
{
    maCurrentFilter = rFilter;
    maCurrentFilter.ToLowerAscii();
    Sequence< ::rtl::OUString > aFileProperties =
        GetFolderContentProperties_Impl( maViewURL, Application::GetSettings().GetLocale(), sal_True );
    OpenFolder( aFileProperties );
}

// -----------------------------------------------------------------------

void SvtFileView::SetNoSelection()
{
    mpView->SelectAll( FALSE );
}

// -----------------------------------------------------------------------

void SvtFileView::SetFocusInView()
{
    mpView->GrabFocus();
}

// -----------------------------------------------------------------------

void SvtFileView::ResetCursor()
{
    // deselect
    SvLBoxEntry* pEntry = mpView->FirstSelected();
    if ( pEntry )
        mpView->Select( pEntry, FALSE );
    // set cursor to the first entry
    mpView->SetCursor( mpView->First(), TRUE );
    mpView->Update();
}

// -----------------------------------------------------------------------

void SvtFileView::SetSelectHdl( const Link& rHdl )
{
    mpView->SetSelectHdl( rHdl );
}

// -----------------------------------------------------------------------

void SvtFileView::SetDoubleClickHdl( const Link& rHdl )
{
    mpView->SetDoubleClickHdl( rHdl );
}

// -----------------------------------------------------------------------

ULONG SvtFileView::GetSelectionCount() const
{
    return mpView->GetSelectionCount();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::FirstSelected() const
{
    return mpView->FirstSelected();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::NextSelected( SvLBoxEntry* pEntry ) const
{
    return mpView->NextSelected( pEntry );
}

// -----------------------------------------------------------------------

void SvtFileView::EnableAutoResize()
{
    mpView->EnableAutoResize();
}

// -----------------------------------------------------------------------

void SvtFileView::SetFocus()
{
    mpView->GrabFocus();
}

