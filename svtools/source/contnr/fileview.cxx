/*************************************************************************
 *
 *  $RCSfile: fileview.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2001-09-18 08:13:57 $
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

#include <string>
#include "fileview.hxx"
#include "svtdata.hxx"
#include "imagemgr.hxx"
#include "headbar.hxx"
#include "svtabbx.hxx"

#include "svtools.hrc"
#include "fileview.hrc"

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
#ifndef  _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef  _COM_SUN_STAR_UCB_CONTENTCREATIONEXCEPTION_HPP_
#include <com/sun/star/ucb/ContentCreationException.hpp>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

#ifndef _VECTOR_
#include <vector>
#endif
#ifndef _ALGORITHM_
#include <algorithm>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
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

#define ALL_FILES_FILTER    "*.*"

#define COLUMN_TITLE        1
#define COLUMN_TYPE         2
#define COLUMN_SIZE         3
#define COLUMN_DATE         4

#define ROW_TITLE           1
#define ROW_SIZE            2
#define ROW_DATE_MOD        3
#define ROW_DATE_CREATE     4
#define ROW_IS_FOLDER       5
#define ROW_TARGET_URL      6

DECLARE_LIST( StringList_Impl, OUString* );

#define ROW_HEIGHT  17  // the height of a row has to be a little higher than the bitmap
#define QUICK_SEARCH_TIMEOUT    1500    // time in mSec before the quicksearch string will be reseted

// structs   -------------------------------------------------------------

struct SortingData_Impl
{
    DateTime    maModDate;
    OUString    maTitle;
    OUString    maLowerTitle;
    OUString    maType;
    OUString    maTargetURL;
    OUString    maImageURL;
    OUString    maDisplayText;
    Image       maImage;
    sal_Int64   maSize;
    sal_Bool    mbIsFolder;
};

// class ViewTabListBox_Impl ---------------------------------------------

class ViewTabListBox_Impl : public SvHeaderTabListBox
{
private:
    Reference< XCommandEnvironment >    mxCmdEnv;

    ::osl::Mutex            maMutex;
    HeaderBar*              mpHeaderBar;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    sal_uInt32              mnSearchIndex;
    sal_Bool                mbResizeDisabled        : 1;
    sal_Bool                mbAutoResize            : 1;
    sal_Bool                mbContextMenuEnabled    : 1;
    sal_Bool                mbEnableDelete          : 1;

    DECL_LINK( HeaderSelect_Impl, HeaderBar * );
    DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );

    void            DeleteEntries();
    void            DoQuickSearch( const xub_Unicode& rChar );
    sal_Bool        Kill( const OUString& rURL );

public:
                    ViewTabListBox_Impl( Window* pParentWin,
                                         SvtFileView_Impl* pParent,
                                         sal_Int16 nFlags );
                   ~ViewTabListBox_Impl();

    virtual void    Resize();
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual BOOL    EditedEntry( SvLBoxEntry* pEntry,
                                 const XubString& rNewText );

    void            ClearAll();
    HeaderBar*      GetHeaderBar() const { return mpHeaderBar; }

    void            EnableAutoResize() { mbAutoResize = sal_True; }
    void            EnableContextMenu( sal_Bool bEnable ) { mbContextMenuEnabled = bEnable; }
    void            EnableDelete( sal_Bool bEnable ) { mbEnableDelete = bEnable; }

    Reference< XCommandEnvironment >    GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK( ResetQuickSearch_Impl, Timer * );
};

// class SvtFileView_Impl ---------------------------------------------

class SvtFileView_Impl
{
public:

    ::std::vector< SortingData_Impl* >  maContent;
    ::osl::Mutex                        maMutex;

    ViewTabListBox_Impl*    mpView;
    sal_uInt16              mnSortColumn;
    sal_Bool                mbAscending     : 1;
    sal_Bool                mbOnlyFolder    : 1;
    String                  maViewURL;
    String                  maAllFilter;
    String                  maCurrentFilter;
    Image                   maFolderImage;
    Link                    maOpenDoneLink;

                            SvtFileView_Impl( Window* pParent,
                                              sal_Int16 nFlags,
                                              sal_Bool bOnlyFolder );
                           ~SvtFileView_Impl();

    void                    Clear();

    void                    GetFolderContent_Impl( const String& rFolder );
    void                    FilterFolderContent_Impl( const OUString &rFilter );

    void                    OpenFolder_Impl();
    void                    CreateDisplayText_Impl();
    void                    CreateVector_Impl( const Sequence < OUString > &rList );
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    void                    FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    ULONG                   GetEntryPos( const OUString& rURL );

    void                    EnableContextMenu( sal_Bool bEnable ) { mpView->EnableContextMenu( bEnable ); }
    void                    EnableDelete( sal_Bool bEnable ) { mpView->EnableDelete( bEnable ); }

    void                    Resort_Impl( sal_Int16 nColumn, sal_Bool bAscending );
    sal_Bool                SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             sal_Bool bWrapAround );
};


// functions -------------------------------------------------------------

#define CONVERT_DATETIME( aUnoDT, aToolsDT ) \
    aToolsDT = DateTime( Date( aUnoDT.Day, aUnoDT.Month, aUnoDT.Year ), \
                         Time( aUnoDT.Hours, aUnoDT.Minutes, aUnoDT.Seconds, aUnoDT.HundredthSeconds ) );

void AppendDateTime_Impl( const ::com::sun::star::util::DateTime& rDT,
                          const Locale& rLocale, String& rRow )
{
    DateTime aDT;
    CONVERT_DATETIME( rDT, aDT );
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), rLocale );
    String aDateStr = aLocaleWrapper.getDate( aDT );
    aDateStr += String::CreateFromAscii( ", " );
    aDateStr += aLocaleWrapper.getTime( aDT );
    rRow += aDateStr;
}

OUString CreateExactSizeText_Impl( sal_Int64 nSize )
{
    double fSize( ( double ) nSize );
    int nDec;

    ULONG nMega = 1024 * 1024;
    ULONG nGiga = nMega * 1024;

    String aUnitStr = ' ';

    if ( nSize < 10000 )
    {
        aUnitStr += String( SvtResId( STR_SVT_BYTES ) );
        nDec = 0;
    }
    else if ( nSize < nMega )
    {
        fSize /= 1024;
        aUnitStr += String( SvtResId( STR_SVT_KB ) );
        nDec = 1;
    }
    else if ( nSize < nGiga )
    {
        fSize /= nMega;
        aUnitStr += String( SvtResId( STR_SVT_MB ) );
        nDec = 2;
    }
    else
    {
        fSize /= nGiga;
        aUnitStr += String( SvtResId( STR_SVT_GB ) );
        nDec = 3;
    }

    String aSizeStr;
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    SolarMath::DoubleToString( aSizeStr, fSize, 'F', nDec, aLocaleWrapper.getNumDecimalSep().GetChar(0) );
    aSizeStr += aUnitStr;

    return aSizeStr;
}

// -----------------------------------------------------------------------
// class ViewTabListBox_Impl ---------------------------------------------
// -----------------------------------------------------------------------

ViewTabListBox_Impl::ViewTabListBox_Impl( Window* pParentWin,
                                          SvtFileView_Impl* pParent,
                                          sal_Int16 nFlags ) :

    SvHeaderTabListBox( pParentWin, WB_TABSTOP ),

    mpHeaderBar     ( NULL ),
    mpParent        ( pParent ),
    mnSearchIndex   ( 0 ),
    mbResizeDisabled( sal_False ),
    mbAutoResize    ( sal_False ),
    mbContextMenuEnabled ( sal_True ),
    mbEnableDelete  ( sal_True )

{
    Size aBoxSize = pParentWin->GetSizePixel();
    mpHeaderBar = new HeaderBar( pParentWin, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), mpHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = ( HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE );
    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        mpHeaderBar->InsertItem( COLUMN_TITLE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 180, nBits | HIB_UPARROW );
        mpHeaderBar->InsertItem( COLUMN_TYPE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TYPE ) ), 140, nBits );
        mpHeaderBar->InsertItem( COLUMN_SIZE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_SIZE ) ), 80, nBits );
        mpHeaderBar->InsertItem( COLUMN_DATE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_DATE ) ), 500, nBits );
    }
    else
        mpHeaderBar->InsertItem( COLUMN_TITLE, String( SvtResId( STR_SVT_FILEVIEW_COLUMN_TITLE ) ), 600, nBits );

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

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetTimeoutHdl( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
    Reference< XInteractionHandler > xInteractionHandler = Reference< XInteractionHandler > (
               xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uui.InteractionHandler") ) ), UNO_QUERY );

    mxCmdEnv = new CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

}

// -----------------------------------------------------------------------

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    maResetQuickSearch.Stop();

    delete mpHeaderBar;
}

// -----------------------------------------------------------------------

IMPL_LINK( ViewTabListBox_Impl, HeaderSelect_Impl, HeaderBar*, pBar )
{
    USHORT nItemID = pBar ? pBar->GetCurItemId() : 0;

/*    if ( pBar && pBar->GetCurItemId() != ITEMID_TYPE )
*/      return 0;

//  HeaderBarItemBits nBits = mpHeaderBar->GetItemBits( ITEMID_TYPE );
    HeaderBarItemBits nBits = mpHeaderBar->GetItemBits( nItemID );
    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );
    SvSortMode eMode = SortAscending;

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
        eMode = SortDescending;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }
    mpHeaderBar->SetItemBits( nItemID, nBits );
/*  mpHeaderBar->SetItemBits( ITEMID_TYPE, nBits );
    SvTreeList* pModel = pPathBox->GetModel();
    pModel->SetSortMode( eMode );
    pModel->Resort();
*/  return 1;
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

IMPL_LINK( ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer*, pTimer )
{
    ::osl::MutexGuard aGuard( maMutex );

    maQuickSearchText = OUString();
    mnSearchIndex = 0;

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
    {
        ResetQuickSearch_Impl( NULL );
        GetDoubleClickHdl().Call( this );
    }
    else if ( ( rKEvt.GetKeyCode().GetCode() == KEY_DELETE ) &&
              mbEnableDelete )
    {
        ResetQuickSearch_Impl( NULL );
        DeleteEntries();
    }
    else if ( ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_NUM ) ||
              ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_ALPHA ) )
    {
        DoQuickSearch( rKEvt.GetCharCode() );
    }
    else
    {
        ResetQuickSearch_Impl( NULL );
        SvHeaderTabListBox::KeyInput( rKEvt );
    }
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::Command( const CommandEvent& rCEvt )
{
    if ( ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU ) && mbContextMenuEnabled )
    {
        Point nPos = rCEvt.GetMousePosPixel();
        SvLBoxEntry* pEntry = GetEntry( nPos );
        if ( pEntry )
        {
            SelectAll( FALSE );
            Select( pEntry, TRUE );

            PopupMenu aMenu( SvtResId( RID_FILEVIEW_CONTEXTMENU ) );
            USHORT nId = aMenu.Execute( this, nPos );
            switch ( nId )
            {
                case MID_FILEVIEW_DELETE :
                    DeleteEntries();
                    break;
                case MID_FILEVIEW_RENAME :
                    EditEntry( pEntry );
                    break;
            }
        }
    }
    else
        SvHeaderTabListBox::Command( rCEvt );
}

// -----------------------------------------------------------------------

void ViewTabListBox_Impl::ClearAll()
{
    for ( USHORT i = 0; i < GetEntryCount(); ++i )
        delete (SvtContentEntry*)GetEntry(i)->GetUserData();
    Clear();
}

// -----------------------------------------------------------------------
void ViewTabListBox_Impl::DeleteEntries()
{
    svtools::QueryDeleteResult_Impl eResult = svtools::QUERYDELETE_YES;
    SvLBoxEntry* pEntry = FirstSelected();
    String aURL;

    while ( pEntry && ( eResult != svtools::QUERYDELETE_CANCEL ) )
    {
        SvLBoxEntry *pCurEntry = pEntry;
        pEntry = NextSelected( pEntry );

        if ( pCurEntry->GetUserData() )
            aURL = ( (SvtContentEntry*)pCurEntry->GetUserData() )->maURL;

        if ( !aURL.Len() )
            return;

        INetURLObject aObj( aURL );

        if ( eResult != svtools::QUERYDELETE_ALL )
        {
            svtools::QueryDeleteDlg_Impl aDlg( NULL, aObj.GetName( INetURLObject::DECODE_WITH_CHARSET ) );

            if ( GetSelectionCount() > 1 )
                aDlg.EnableAllButton();

            if ( aDlg.Execute() == RET_OK )
                eResult = aDlg.GetResult();
            else
                eResult = svtools::QUERYDELETE_CANCEL;
        }

        if ( ( eResult == svtools::QUERYDELETE_ALL ) ||
             ( eResult == svtools::QUERYDELETE_YES ) )
        {
            if ( Kill( aURL ) )
            {
                delete (SvtContentEntry*)pCurEntry->GetUserData();
                GetModel()->Remove( pCurEntry );
                mpParent->EntryRemoved( aURL );
            }
        }
    }
}

// -----------------------------------------------------------------------
BOOL ViewTabListBox_Impl::EditedEntry( SvLBoxEntry* pEntry,
                                 const XubString& rNewText )
{
    BOOL bRet = FALSE;

    OUString aURL;
    SvtContentEntry* pData = (SvtContentEntry*)pEntry->GetUserData();

    if ( pData )
        aURL = OUString( pData->maURL );

    if ( ! aURL.getLength() )
        return bRet;

    try
    {
        Content aContent( aURL, mxCmdEnv );

        OUString aPropName = OUString::createFromAscii( "Title" );
        Any aValue;
        aValue <<= OUString( rNewText );
        aContent.setPropertyValue( aPropName, aValue );
        mpParent->EntryRenamed( aURL, rNewText );

        pData->maURL = aURL;
        pEntry->SetUserData( pData );

        bRet = TRUE;
    }
    catch( ::com::sun::star::ucb::ContentCreationException ) {}
    catch( ::com::sun::star::ucb::CommandAbortedException ) {}
    catch( ::com::sun::star::uno::Exception ) {}

    return bRet;
}

// -----------------------------------------------------------------------
void ViewTabListBox_Impl::DoQuickSearch( const xub_Unicode& rChar )
{
    ::osl::MutexGuard aGuard( maMutex );

    maResetQuickSearch.Stop();

    OUString    aLastText = maQuickSearchText;
    sal_uInt32  aLastPos = mnSearchIndex;
    sal_Bool    bFound = sal_False;

    maQuickSearchText += OUString( rChar ).toAsciiLowerCase();

    bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_False );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUString( rChar ) ) )
    {
        mnSearchIndex = aLastPos + 1;
        maQuickSearchText = aLastText;
        bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, sal_True );
    }

    if ( bFound )
    {
        SvLBoxEntry* pEntry = GetEntry( mnSearchIndex );
        SelectAll( FALSE );
        Select( pEntry );
        SetCurEntry( pEntry );
        MakeVisible( pEntry );
    }
    else
        Sound::Beep();

    maResetQuickSearch.Start();
}

// -----------------------------------------------------------------------
sal_Bool ViewTabListBox_Impl::Kill( const OUString& rContent )
{
    sal_Bool bRet = sal_True;

    try
    {
        Content aCnt( rContent, mxCmdEnv );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_WARNING( "CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_WARNING( "Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}




// -----------------------------------------------------------------------
// class SvtFileView -----------------------------------------------------
// -----------------------------------------------------------------------

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId,
                          sal_Bool bOnlyFolder, sal_Bool bMultiSelection ) :

    Control( pParent, rResId )
{
    sal_Int8 nFlags = FILEVIEW_SHOW_ALL;
    if ( bOnlyFolder )
        nFlags |= FILEVIEW_ONLYFOLDER;
    if ( bMultiSelection )
        nFlags |= FILEVIEW_MULTISELECTION;

    mpImp = new SvtFileView_Impl( this, nFlags, bOnlyFolder );

    long pTabs[] = { 5, 20, 180, 320, 400, 600 };
    mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    mpImp->mpView->SetTabJustify( 2, AdjustRight ); // column "Size"

    if ( bMultiSelection )
        mpImp->mpView->SetSelectionMode( MULTIPLE_SELECTION );

    HeaderBar *pHeaderBar = mpImp->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
}

SvtFileView::SvtFileView( Window* pParent, const ResId& rResId, sal_Int8 nFlags ) :

    Control( pParent, rResId )
{
    mpImp = new SvtFileView_Impl( this, nFlags,
                                  ( nFlags & FILEVIEW_ONLYFOLDER ) == FILEVIEW_ONLYFOLDER );

    if ( ( nFlags & FILEVIEW_SHOW_ALL ) == FILEVIEW_SHOW_ALL )
    {
        long pTabs[] = { 5, 20, 180, 320, 400, 600 };
        mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
        mpImp->mpView->SetTabJustify( 2, AdjustRight ); // column "Size"
    }
    else
    {
        // show only title
        long pTabs[] = { 2, 20, 600 };
        mpImp->mpView->SetTabs( &pTabs[0], MAP_PIXEL );
    }

    if ( ( nFlags & FILEVIEW_MULTISELECTION ) == FILEVIEW_MULTISELECTION )
        mpImp->mpView->SetSelectionMode( MULTIPLE_SELECTION );

    HeaderBar *pHeaderBar = mpImp->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
}

// -----------------------------------------------------------------------

SvtFileView::~SvtFileView()
{
    delete mpImp;
}

// -----------------------------------------------------------------------

void SvtFileView::OpenFolder( const Sequence< OUString >& aContents )
{
    mpImp->mpView->ClearAll();
    const OUString* pFileProperties  = aContents.getConstArray();
    UINT32 i, nCount = aContents.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        String aRow( pFileProperties[i] );
        // extract columns
        // the columns are: title, type, size, date, target url, is folder, image url
        String aTitle, aType, aSize, aDate, aURL, aImageURL;
        xub_StrLen nIdx = 0;
        aTitle = aRow.GetToken( 0, '\t', nIdx );
        aType = aRow.GetToken( 0, '\t', nIdx );
        aSize = aRow.GetToken( 0, '\t', nIdx );
        aDate = aRow.GetToken( 0, '\t', nIdx );
        aURL = aRow.GetToken( 0, '\t', nIdx );
        sal_Unicode cFolder = aRow.GetToken( 0, '\t', nIdx ).GetChar(0);
        sal_Bool bIsFolder = ( '1' == cFolder );
        if ( nIdx != STRING_NOTFOUND )
            aImageURL = aRow.GetToken( 0, '\t', nIdx );

        if ( mpImp->mbOnlyFolder && !bIsFolder )
            continue;

        // build new row
        String aNewRow = aTitle;
        aNewRow += '\t';
        aNewRow += aType;
        aNewRow += '\t';
        aNewRow += aSize;
        aNewRow += '\t';
        aNewRow += aDate;
        // detect image
        sal_Bool bDoInsert = sal_True;
        Image aImage;

        if ( bIsFolder )
            aImage = mpImp->maFolderImage;
        else
        {
            INetURLObject aObj( aImageURL.Len() > 0 ? aImageURL : aURL );
            aImage = SvFileInformationManager::GetImage( aObj, FALSE );
        }

        if ( bDoInsert )
        {
            // insert entry and set user data
            SvLBoxEntry* pEntry = mpImp->mpView->InsertEntry( aNewRow, aImage, aImage );
            SvtContentEntry* pUserData = new SvtContentEntry( aURL, bIsFolder );
            pEntry->SetUserData( pUserData );
        }
    }

    mpImp->mpView->SelectAll( FALSE );
    SvLBoxEntry* pFirst = mpImp->mpView->First();
    if ( pFirst )
        mpImp->mpView->SetCursor( pFirst, TRUE );
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
    SvLBoxEntry* pEntry = mpImp->mpView->FirstSelected();
    if ( pEntry && pEntry->GetUserData() )
        aURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;
    return aURL;
}

// -----------------------------------------------------------------------

void SvtFileView::CreateNewFolder( const String& rNewFolder )
{
    INetURLObject aObj( mpImp->maViewURL );
    aObj.insertName( rNewFolder, false, INetURLObject::LAST_SEGMENT,
                     true, INetURLObject::ENCODE_ALL );
    String aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
    if ( ::utl::UCBContentHelper::MakeFolder( aURL ) )
    {
        String aEntry = aObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
        SvLBoxEntry* pEntry = mpImp->mpView->InsertEntry( aEntry, mpImp->maFolderImage, mpImp->maFolderImage );
        SvtContentEntry* pUserData = new SvtContentEntry( aURL, TRUE );
        pEntry->SetUserData( pUserData );
        mpImp->FolderInserted( aURL, aEntry );

        mpImp->mpView->MakeVisible( pEntry );
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
    try
    {
        Content aCnt( mpImp->maViewURL, Reference< XCommandEnvironment > () );
        Reference< XContent > xContent( aCnt.get() );
        Reference< com::sun::star::container::XChild > xChild( xContent, UNO_QUERY );
        if ( xChild.is() )
        {
            Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                rParentURL = String( xParent->getIdentifier()->getContentIdentifier() );
                bRet = ( rParentURL.Len() > 0 && rParentURL != mpImp->maViewURL );
            }
        }
    }
    catch( ::com::sun::star::uno::Exception )
    {
        // perhaps an unkown url protocol (e.g. "private:newdoc")
    }

    return bRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtFileView::PreviousLevel( String& rNewURL )
{
    sal_Bool bRet = sal_False;
    if ( HasPreviousLevel( rNewURL ) )
    {
        Initialize( rNewURL, mpImp->maCurrentFilter );
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void SvtFileView::SetHelpId( sal_uInt32 nHelpId )
{
    mpImp->mpView->SetHelpId( nHelpId );
}

// -----------------------------------------------------------------------

void SvtFileView::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    mpImp->mpView->SetSizePixel( rNewSize );
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
    WaitObject aWaitCursor( this );

    mpImp->maViewURL = rURL;
    ExecuteFilter( rFilter );
    mpImp->maOpenDoneLink.Call( this );
}

// -----------------------------------------------------------------------

void SvtFileView::Initialize( const String& rURL, const Sequence< OUString >& aContents )
{
    WaitObject aWaitCursor( this );

    mpImp->maViewURL = rURL;
    mpImp->maCurrentFilter = mpImp->maAllFilter;

    mpImp->Clear();
    mpImp->CreateVector_Impl( aContents );
    mpImp->SortFolderContent_Impl();

    mpImp->OpenFolder_Impl();

    mpImp->maOpenDoneLink.Call( this );
}

// -----------------------------------------------------------------------

void SvtFileView::ExecuteFilter( const String& rFilter )
{
    mpImp->maCurrentFilter = rFilter;
    mpImp->maCurrentFilter.ToLowerAscii();

    mpImp->Clear();
    mpImp->GetFolderContent_Impl( mpImp->maViewURL );
    mpImp->FilterFolderContent_Impl( rFilter );

    mpImp->SortFolderContent_Impl();
    mpImp->CreateDisplayText_Impl();
    mpImp->OpenFolder_Impl();
}

// -----------------------------------------------------------------------

void SvtFileView::SetNoSelection()
{
    mpImp->mpView->SelectAll( FALSE );
}

// -----------------------------------------------------------------------

void SvtFileView::GetFocus()
{
    Control::GetFocus();
    mpImp->mpView->GrabFocus();
}

// -----------------------------------------------------------------------

void SvtFileView::ResetCursor()
{
    // deselect
    SvLBoxEntry* pEntry = mpImp->mpView->FirstSelected();
    if ( pEntry )
        mpImp->mpView->Select( pEntry, FALSE );
    // set cursor to the first entry
    mpImp->mpView->SetCursor( mpImp->mpView->First(), TRUE );
    mpImp->mpView->Update();
}

// -----------------------------------------------------------------------

void SvtFileView::SetSelectHdl( const Link& rHdl )
{
    mpImp->mpView->SetSelectHdl( rHdl );
}

// -----------------------------------------------------------------------

void SvtFileView::SetDoubleClickHdl( const Link& rHdl )
{
    mpImp->mpView->SetDoubleClickHdl( rHdl );
}

// -----------------------------------------------------------------------

ULONG SvtFileView::GetSelectionCount() const
{
    return mpImp->mpView->GetSelectionCount();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::FirstSelected() const
{
    return mpImp->mpView->FirstSelected();
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvtFileView::NextSelected( SvLBoxEntry* pEntry ) const
{
    return mpImp->mpView->NextSelected( pEntry );
}

// -----------------------------------------------------------------------

void SvtFileView::EnableAutoResize()
{
    mpImp->mpView->EnableAutoResize();
}

// -----------------------------------------------------------------------

void SvtFileView::SetFocus()
{
    mpImp->mpView->GrabFocus();
}

// -----------------------------------------------------------------------
const String& SvtFileView::GetViewURL() const
{
    return mpImp->maViewURL;
}

// -----------------------------------------------------------------------
void SvtFileView::SetOpenDoneHdl( const Link& rHdl )
{
    mpImp->maOpenDoneLink = rHdl;
}

// -----------------------------------------------------------------------
void SvtFileView::EnableContextMenu( sal_Bool bEnable )
{
    mpImp->EnableContextMenu( bEnable );
}

// -----------------------------------------------------------------------
void SvtFileView::EnableDelete( sal_Bool bEnable )
{
    mpImp->EnableDelete( bEnable );
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
IMPL_LINK( SvtFileView, HeaderSelect_Impl, HeaderBar*, pBar )
{
    if ( !pBar )
        return 0;

    USHORT nItemID = pBar->GetCurItemId();

    HeaderBarItemBits nBits;

    // clear the arrow of the recently used column
    if ( nItemID != mpImp->mnSortColumn )
    {
        nBits = pBar->GetItemBits( mpImp->mnSortColumn );
        nBits &= ~( HIB_UPARROW | HIB_DOWNARROW );
        pBar->SetItemBits( mpImp->mnSortColumn, nBits );
    }

    nBits = pBar->GetItemBits( nItemID );

    BOOL bUp = ( ( nBits & HIB_UPARROW ) == HIB_UPARROW );

    if ( bUp )
    {
        nBits &= ~HIB_UPARROW;
        nBits |= HIB_DOWNARROW;
    }
    else
    {
        nBits &= ~HIB_DOWNARROW;
        nBits |= HIB_UPARROW;
    }

    pBar->SetItemBits( nItemID, nBits );

    mpImp->Resort_Impl( nItemID, !bUp );

    return 1;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
SvtFileView_Impl::SvtFileView_Impl( Window* pParent,
                                    sal_Int16 nFlags,
                                    sal_Bool bOnlyFolder ) :
    mnSortColumn    ( COLUMN_TITLE ),
    mbAscending     ( sal_True ),
    mbOnlyFolder    ( bOnlyFolder ),
    maFolderImage   ( SvtResId( IMG_SVT_FOLDER ) )
{
    maAllFilter = String::CreateFromAscii( "*.*" );

    mpView = new ViewTabListBox_Impl( pParent, this, nFlags );
}

// -----------------------------------------------------------------------
SvtFileView_Impl::~SvtFileView_Impl()
{
    Clear();

    delete mpView;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::Clear()
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
        delete (*aIt);

    maContent.clear();
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::GetFolderContent_Impl( const String& rFolder )
{
    ::osl::MutexGuard aGuard( maMutex );

    SortingData_Impl* pData;

    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    try
    {
        Content aCnt( aFolderObj.GetMainURL( INetURLObject::NO_DECODE ), mpView->GetCommandEnvironment() );
        Reference< XResultSet > xResultSet;
        Sequence< OUString > aProps(6);

        aProps[0] = OUString::createFromAscii( "Title" );
        aProps[1] = OUString::createFromAscii( "Size" );
        aProps[2] = OUString::createFromAscii( "DateModified" );
        aProps[3] = OUString::createFromAscii( "DateCreated" );
        aProps[4] = OUString::createFromAscii( "IsFolder" );
        aProps[5] = OUString::createFromAscii( "TargetURL" );

        try
        {
            Reference< com::sun::star::ucb::XDynamicResultSet > xDynResultSet;
            ResultSetInclude eInclude = INCLUDE_FOLDERS_AND_DOCUMENTS;
            xDynResultSet = aCnt.createDynamicCursor( aProps, eInclude );

            if ( xDynResultSet.is() )
                xResultSet = xDynResultSet->getStaticResultSet();
        }
        catch( CommandAbortedException& )
        {
            DBG_ERRORFILE( "createCursor: CommandAbortedException" );
        }
        catch( ::com::sun::star::uno::Exception& e )
        {
            e; // make compiler happy
        }

        if ( xResultSet.is() )
        {
            Reference< com::sun::star::sdbc::XRow > xRow( xResultSet, UNO_QUERY );
            Reference< com::sun::star::ucb::XContentAccess > xContentAccess( xResultSet, UNO_QUERY );

            try
            {
                while ( xResultSet->next() )
                {
                    pData = new SortingData_Impl;

                    ::com::sun::star::util::DateTime aDT = xRow->getTimestamp( ROW_DATE_MOD );
                    if ( xRow->wasNull() )
                        aDT = xRow->getTimestamp( ROW_DATE_CREATE );

                    OUString aContentURL = xContentAccess->queryContentIdentifierString();
                    OUString aTargetURL = xRow->getString( ROW_TARGET_URL );
                    sal_Bool bTarget = aTargetURL.getLength() > 0;

                    pData->maTitle    = xRow->getString( ROW_TITLE );
                    pData->maLowerTitle = pData->maTitle.toAsciiLowerCase();
                    pData->maSize     = xRow->getLong( ROW_SIZE );
                    pData->mbIsFolder = xRow->getBoolean( ROW_IS_FOLDER );

                    if ( bTarget &&
                         INetURLObject( aContentURL ).GetProtocol() == INET_PROT_VND_SUN_STAR_HIER )
                    {
                        Content aCnt( aTargetURL, Reference< XCommandEnvironment > () );
                        aCnt.getPropertyValue( OUString::createFromAscii( "Size" ) ) >>= pData->maSize;
                        aCnt.getPropertyValue( OUString::createFromAscii( "DateModified" ) ) >>= aDT;
                    }

                    CONVERT_DATETIME( aDT, pData->maModDate );

                    if ( aTargetURL.getLength() )
                        pData->maTargetURL   = aTargetURL;
                    else
                        pData->maTargetURL   = aContentURL;

                    INetURLObject aURLObj( pData->maTargetURL );

                    pData->maType = SvFileInformationManager::GetDescription( aURLObj );

                    maContent.push_back( pData );
                }
            }
            catch( CommandAbortedException& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): CommandAbortedException" );
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                DBG_ERRORFILE( "XContentAccess::next(): Any other exception" );
            }
        }
    }
    catch( CommandAbortedException& )
    {
        DBG_ERRORFILE( "GetFolderContents: CommandAbortedException" );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        DBG_ERRORFILE( "GetFolderContents: Any other exception" );
    }
}
// -----------------------------------------------------------------------
namespace
{
    struct FilterMatch : public ::std::unary_function< bool, WildCard >
    {
    private:
        const String&   m_rCompareString;
    public:
        FilterMatch( const String& _rCompareString ) : m_rCompareString( _rCompareString ) { }

        bool operator()( const WildCard& _rMatcher )
        {
            return _rMatcher.Matches( m_rCompareString ) ? true : false;
        }
    };
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::FilterFolderContent_Impl( const OUString &rFilter )
{
    if ( !rFilter.getLength() ||
         ( rFilter.compareToAscii( ALL_FILES_FILTER ) == COMPARE_EQUAL ) )
        return;

    ::osl::MutexGuard aGuard( maMutex );

    if ( maContent.size() == 0 )
        return;

    // count (estimate) the number of filter tokens
    sal_Int32 nTokens;
    const sal_Unicode* pStart = rFilter.getStr();
    const sal_Unicode* pEnd = pStart + rFilter.getLength();
    while ( pStart != pEnd )
        if ( *pStart++ == ';' )
            ++nTokens;

    // collect the filter tokens
    ::std::vector< WildCard > aFilters;
    aFilters.reserve( nTokens );
    sal_Int32 nIndex = 0;
    ::rtl::OUString sToken;
    do
    {
        sToken = rFilter.getToken( 0, ';', nIndex );
        if ( sToken.getLength() )
        {
            aFilters.push_back( WildCard( sToken.toAsciiUpperCase() ) );
        }
    }
    while ( nIndex >= 0 );

    // do the filtering
    ::std::vector< SortingData_Impl* >::iterator aContentLoop = maContent.begin();
    sal_Bool bDeleteThisOne = sal_True;
    String sCompareString;
    do
    {
        if ( (*aContentLoop)->mbIsFolder == sal_True )
            ++aContentLoop;
        else
        {
            // normalize the content title (we always match case-insensitive)
            // 91872 - 11.09.2001 - frank.schoenheit@sun.com
            sCompareString = (*aContentLoop)->maTitle.toAsciiUpperCase();

            // search for the first filter which matches
            ::std::vector< WildCard >::const_iterator pMatchingFilter =
                ::std::find_if(
                    aFilters.begin(),
                    aFilters.end(),
                    FilterMatch( sCompareString )
                );
            if ( aFilters.end() == pMatchingFilter )
            {
                // none of the filters did match
                delete (*aContentLoop);

                if ( maContent.begin() == aContentLoop )
                {
                    maContent.erase( aContentLoop );
                    aContentLoop = maContent.begin();
                }
                else
                {
                    std::vector< SortingData_Impl* >::iterator aDelete = aContentLoop;
                    --aContentLoop; // move the iterator to a position which is not invalidated by the erase
                    maContent.erase( aDelete );
                    ++aContentLoop; // this is now the next one ....
                }
            }
            else
                ++aContentLoop;
        }
    }
    while ( aContentLoop != maContent.end() );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::OpenFolder_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    mpView->SetUpdateMode( FALSE );
    mpView->ClearAll();

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( mbOnlyFolder && ! (*aIt)->mbIsFolder )
            continue;

        // insert entry and set user data
        SvLBoxEntry* pEntry = mpView->InsertEntry( (*aIt)->maDisplayText,
                                                   (*aIt)->maImage,
                                                   (*aIt)->maImage );

        SvtContentEntry* pUserData = new SvtContentEntry( (*aIt)->maTargetURL,
                                                          (*aIt)->mbIsFolder );
        pEntry->SetUserData( pUserData );
    }

    mpView->SelectAll( FALSE );
    SvLBoxEntry* pFirst = mpView->First();
    if ( pFirst )
        mpView->SetCursor( pFirst, TRUE );

    mpView->SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::CreateDisplayText_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(),
                                      Application::GetSettings().GetLocale() );

    OUString aValue;
    OUString aTab     = OUString::createFromAscii( "\t" );
    OUString aDateSep = OUString::createFromAscii( ", " );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        // title, type, size, date
        aValue = (*aIt)->maTitle;
        aValue += aTab;
        aValue += (*aIt)->maType;
        aValue += aTab;
        // folders don't have a size
        if ( ! (*aIt)->mbIsFolder )
            aValue += CreateExactSizeText_Impl( (*aIt)->maSize );
        aValue += aTab;
        // set the date
        aValue += aLocaleWrapper.getDate( (*aIt)->maModDate );
        aValue += aDateSep;
        aValue += aLocaleWrapper.getTime( (*aIt)->maModDate );

        (*aIt)->maDisplayText = aValue;

        // detect image

        if ( (*aIt)->mbIsFolder )
            (*aIt)->maImage = maFolderImage;
        else
        {
            INetURLObject aObj( (*aIt)->maTargetURL );
            (*aIt)->maImage = SvFileInformationManager::GetImage( aObj, FALSE );
        }
    }
}

// -----------------------------------------------------------------------
// this function converts the sequence of strings into a vector of SortingData
// the string should have the form :
// title \t type \t size \t date \t target url \t is folder \t image url

void SvtFileView_Impl::CreateVector_Impl( const Sequence < OUString > &rList )
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString aTab     = OUString::createFromAscii( "\t" );

    sal_uInt32 nCount = (sal_uInt32) rList.getLength();

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SortingData_Impl*   pEntry = new SortingData_Impl;
        OUString            aValue = rList[i];
        OUString            aDisplayText;
        sal_Int32           nIndex = 0;

        // get the title
        pEntry->maTitle = aValue.getToken( 0, '\t', nIndex );
        pEntry->maLowerTitle = pEntry->maTitle.toAsciiLowerCase();
        aDisplayText = pEntry->maTitle;
        aDisplayText += aTab;

        // get the type
        if ( nIndex >= 0 )
        {
            pEntry->maType = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += pEntry->maType;
        }
        aDisplayText += aTab;

        // get the size
        if ( nIndex >= 0 )
        {
            OUString aSize = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += aSize;

            if ( aSize.getLength() )
                pEntry->maSize = aSize.toInt64();
        }
        aDisplayText += aTab;

        // get the date
        if ( nIndex >= 0 )
        {
            OUString aDate = aValue.getToken( 0, '\t', nIndex );
            aDisplayText += aDate;

            if ( aDate.getLength() )
            {
                DBG_ERRORFILE( "Don't know, how to convert date" );
                ;// convert date string to date
            }
        }
        // get the target url
        if ( nIndex >= 0 )
        {
            pEntry->maTargetURL = aValue.getToken( 0, '\t', nIndex );
        }
        // get the size
        if ( nIndex >= 0 )
        {
            OUString aBool = aValue.getToken( 0, '\t', nIndex );
            if ( aBool.getLength() )
                pEntry->mbIsFolder = aBool.toBoolean();
        }
        // get the image url
        if ( nIndex >= 0 )
        {
            pEntry->maImageURL = aValue.getToken( 0, '\t', nIndex );
        }

        // set the display text
        pEntry->maDisplayText = aDisplayText;

        // detect the image
        if ( pEntry->mbIsFolder )
            pEntry->maImage = maFolderImage;
        else
        {
            INetURLObject aObj( pEntry->maImageURL.getLength() ? pEntry->maImageURL : pEntry->maTargetURL );
            pEntry->maImage = SvFileInformationManager::GetImage( aObj, FALSE );
        }

        maContent.push_back( pEntry );
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::Resort_Impl( sal_Int16 nColumn, sal_Bool bAscending )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( ( nColumn == mnSortColumn ) &&
         ( bAscending == mbAscending ) )
         return;

    // reset the quick search index
    mpView->ResetQuickSearch_Impl( NULL );

    String aEntryURL;
    SvLBoxEntry* pEntry = mpView->GetCurEntry();
    if ( pEntry && pEntry->GetUserData() )
        aEntryURL = ( (SvtContentEntry*)pEntry->GetUserData() )->maURL;

    mnSortColumn = nColumn;
    mbAscending = bAscending;

    SortFolderContent_Impl();
    OpenFolder_Impl();

    if ( aEntryURL.Len() )
    {
        ULONG nPos = GetEntryPos( aEntryURL );
        if ( nPos < mpView->GetEntryCount() )
        {
            pEntry = mpView->GetEntry( nPos );
            mpView->SetCurEntry( pEntry );
        }
    }
}

// -----------------------------------------------------------------------
static sal_Bool     gbAscending = sal_True;
static sal_Int16    gnColumn = COLUMN_TITLE;

/* this functions returns true, if aOne is less then aTwo
*/
sal_Bool CompareSortingData_Impl( SortingData_Impl* const aOne,
                                  SortingData_Impl* const aTwo )
{
    sal_Int32   nComp;
    sal_Bool    bRet;
    sal_Bool    bEqual = sal_False;

    if ( aOne->mbIsFolder != aTwo->mbIsFolder )
        if ( aOne->mbIsFolder )
            bRet = sal_True;
        else
            bRet = sal_False;
    else
    {
        switch ( gnColumn )
        {
        case COLUMN_TITLE:
            // compare case insensitiv first
            nComp = aOne->maLowerTitle.compareTo( aTwo->maLowerTitle );

            if ( nComp == 0 )
                nComp = aOne->maTitle.compareTo( aTwo->maTitle );

            if ( nComp < 0 )
                bRet = sal_True;
            else if ( nComp > 0 )
                bRet = sal_False;
            else
                bEqual = sal_True;
            break;
        case COLUMN_TYPE:
            nComp = aOne->maType.compareTo( aTwo->maType );
            if ( nComp < 0 )
                bRet = sal_True;
            else if ( nComp > 0 )
                bRet = sal_False;
            else
                bEqual = sal_True;
            break;
        case COLUMN_SIZE:
            if ( aOne->maSize < aTwo->maSize )
                bRet = sal_True;
            else if ( aOne->maSize > aTwo->maSize )
                bRet = sal_False;
            else
                bEqual = sal_True;
            break;
        case COLUMN_DATE:
            if ( aOne->maModDate < aTwo->maModDate )
                bRet = sal_True;
            else if ( aOne->maModDate > aTwo->maModDate )
                bRet = sal_False;
            else
                bEqual = sal_True;
            break;
        }
    }

    // when the two elements are equal, we must not return TRUE (which would
    // happen if we just return ! ( a < b ) when not sorting ascending )
    if ( bEqual )
        return sal_False;

    if ( gbAscending )
        return bRet;

    return ! bRet;
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::SortFolderContent_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nSize = maContent.size();

    if ( nSize > 1 )
    {
        gbAscending = mbAscending;
        gnColumn = mnSortColumn;

        std::stable_sort( maContent.begin(),
                          maContent.end(),
                          CompareSortingData_Impl );
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::EntryRemoved( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
        {
            maContent.erase( aIt );
            break;
        }
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::EntryRenamed( OUString& rURL,
                                     const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
        {
            (*aIt)->maTitle = rTitle;
            (*aIt)->maLowerTitle = (*aIt)->maTitle.toAsciiLowerCase();
            OUString aDisplayText = (*aIt)->maDisplayText;
            sal_Int32 nIndex = aDisplayText.indexOf( '\t' );

            if ( nIndex > 0 )
                (*aIt)->maDisplayText = aDisplayText.replaceAt( 0, nIndex, rTitle );

            INetURLObject aURLObj( rURL );
            aURLObj.SetName( rTitle, INetURLObject::ENCODE_ALL );

            rURL = aURLObj.GetMainURL( INetURLObject::NO_DECODE );

            (*aIt)->maTargetURL = rURL;
            break;
        }
    }
}

// -----------------------------------------------------------------------
void SvtFileView_Impl::FolderInserted( const OUString& rURL,
                                       const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    SortingData_Impl* pData = new SortingData_Impl;

    pData->maTitle    = rTitle;
    pData->maLowerTitle = pData->maTitle.toAsciiLowerCase();
    pData->maSize     = 0;
    pData->mbIsFolder = sal_True;
    pData->maTargetURL   = rURL;

    INetURLObject aURLObj( rURL );

    pData->maType = SvFileInformationManager::GetDescription( aURLObj );
    pData->maImage = maFolderImage;

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(),
                                      Application::GetSettings().GetLocale() );

    OUString aValue;
    OUString aTab     = OUString::createFromAscii( "\t" );
    OUString aDateSep = OUString::createFromAscii( ", " );

    // title, type, size, date
    aValue = pData->maTitle;
    aValue += aTab;
    aValue += pData->maType;
    aValue += aTab;
    // folders don't have a size
    aValue += aTab;
    // set the date
    aValue += aLocaleWrapper.getDate( pData->maModDate );
    aValue += aDateSep;
    aValue += aLocaleWrapper.getTime( pData->maModDate );

    pData->maDisplayText = aValue;

    maContent.push_back( pData );
}

// -----------------------------------------------------------------------
ULONG SvtFileView_Impl::GetEntryPos( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::vector< SortingData_Impl* >::iterator aIt;
    ULONG   nPos = 0;

    for ( aIt = maContent.begin(); aIt != maContent.end(); aIt++ )
    {
        if ( (*aIt)->maTargetURL == rURL )
            return nPos;
        nPos += 1;
    }

    return nPos;
}

// -----------------------------------------------------------------------
sal_Bool SvtFileView_Impl::SearchNextEntry( sal_uInt32 &nIndex,
                                            const OUString& rTitle,
                                            sal_Bool bWrapAround )
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nEnd = maContent.size();
    sal_uInt32 nStart = nIndex;

    while ( nIndex < nEnd )
    {
        SortingData_Impl* pData = maContent[ nIndex ];
        if ( rTitle.compareTo( pData->maLowerTitle, rTitle.getLength() ) == 0 )
            return sal_True;
        nIndex += 1;
    }

    if ( bWrapAround )
    {
        nIndex = 0;
        while ( nIndex <= nStart )
        {
            SortingData_Impl* pData = maContent[ nIndex ];
            if ( rTitle.compareTo( pData->maLowerTitle, rTitle.getLength() ) == 0 )
                return sal_True;
            nIndex += 1;
        }
    }

    return sal_False;
}



namespace svtools {

// -----------------------------------------------------------------------
// QueryDeleteDlg_Impl
// -----------------------------------------------------------------------

QueryDeleteDlg_Impl::QueryDeleteDlg_Impl
(
    Window* pParent,
    const String& rName      // Eintragsname
) :

    ModalDialog( pParent, SvtResId( DLG_SVT_QUERYDELETE ) ),

    _aEntryLabel    ( this, ResId( TXT_ENTRY ) ),
    _aEntry     ( this, ResId( TXT_ENTRYNAME ) ),
    _aQueryMsg  ( this, ResId( TXT_QUERYMSG ) ),
    _aYesButton ( this, ResId( BTN_YES ) ),
    _aAllButton ( this, ResId( BTN_ALL ) ),
    _aNoButton  ( this, ResId( BTN_NO ) ),
    _aCancelButton( this, ResId( BTN_CANCEL ) )

{
    FreeResource();

    // Handler
    Link aLink( STATIC_LINK( this, QueryDeleteDlg_Impl, ClickLink ) );
    _aYesButton.SetClickHdl( aLink );
    _aAllButton.SetClickHdl( aLink );
    _aNoButton.SetClickHdl( aLink );

    // Anzeige der spezifizierten Texte

    WinBits nTmpStyle = _aEntry.GetStyle();
    nTmpStyle |= WB_PATHELLIPSIS;
    _aEntry.SetStyle( nTmpStyle );
    _aEntry.SetText( rName );
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK( QueryDeleteDlg_Impl, ClickLink, PushButton*, pBtn )

/*  [Beschreibung]

    Die Methode wertet das Resultat der Abfrage aus.
*/

{
    if ( pBtn == &pThis->_aYesButton )
        pThis->_eResult = QUERYDELETE_YES;
    else if ( pBtn == &pThis->_aNoButton )
        pThis->_eResult = QUERYDELETE_NO;
    else if ( pBtn == &pThis->_aAllButton )
        pThis->_eResult = QUERYDELETE_ALL;
    else if ( pBtn == &pThis->_aCancelButton )
        pThis->_eResult = QUERYDELETE_CANCEL;

    pThis->EndDialog( RET_OK );

    return 0;
}

}
