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

#include <vcl/treelistbox.hxx>
#include <svtools/iconview.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <vcl/treelistentry.hxx>
#include <svtools/fileview.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/imagemgr.hxx>
#include <vcl/headbar.hxx>
#include <vcl/svtabbx.hxx>
#include <svtools/strings.hrc>
#include <bitmaps.hlst>
#include <vcl/viewdataentry.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "contentenumeration.hxx"
#include <vcl/AccessibleBrowseBoxObjType.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XAnyCompareFactory.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/io/XPersist.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/layout.hxx>
#include <rtl/math.hxx>
#include <tools/config.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <salhelper/timer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/dialog.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svl/urlfilter.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::comphelper;
using ::svt::SortingData_Impl;
using ::svt::FolderDescriptor;

#define ALL_FILES_FILTER    "*.*"

#define COLUMN_TITLE        1
#define COLUMN_TYPE         2
#define COLUMN_SIZE         3
#define COLUMN_DATE         4

#define ROW_HEIGHT                17    // the height of a row has to be a little higher than the bitmap
#define QUICK_SEARCH_TIMEOUT    1500    // time in mSec before the quicksearch string will be reset

enum class FileViewFlags
{
    NONE               = 0x00,
    ONLYFOLDER         = 0x01,
    MULTISELECTION     = 0x02,
    SHOW_TYPE          = 0x04,
    SHOW_NONE          = 0x20,
};
namespace o3tl
{
    template<> struct typed_flags<FileViewFlags> : is_typed_flags<FileViewFlags, 0x27> {};
}

namespace
{

    //= CallbackTimer

    class CallbackTimer : public ::salhelper::Timer
    {
    protected:
        SvtFileView_Impl* const m_pTimeoutHandler;

    public:
        explicit CallbackTimer( SvtFileView_Impl* _pHandler ) : m_pTimeoutHandler( _pHandler ) { }

    protected:
        virtual void SAL_CALL onShot() override;
    };


}


void FilterMatch::createWildCardFilterList(const OUString& _rFilterList,::std::vector< WildCard >& _rFilters)
{
    if( _rFilterList.getLength() )
    {
        // filter is given
        sal_Int32 nIndex = 0;
        OUString sToken;
        do
        {
            sToken = _rFilterList.getToken( 0, ';', nIndex );
            if ( !sToken.isEmpty() )
            {
                _rFilters.emplace_back( sToken.toAsciiUpperCase() );
            }
        }
        while ( nIndex >= 0 );
    }
    else
    {
        // no filter is given -> match all
        _rFilters.emplace_back("*" );
    }
}

class ViewTabListBox_Impl : public SvHeaderTabListBox
{
private:
    Reference< XCommandEnvironment >    mxCmdEnv;
    std::unique_ptr<VclBuilder> mxBuilder;
    VclPtr<PopupMenu> mxMenu;

    ::osl::Mutex            maMutex;
    VclPtr<HeaderBar>       mpHeaderBar;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    OUString const          msAccessibleDescText;
    OUString const          msFolder;
    OUString const          msFile;
    sal_uInt32              mnSearchIndex;
    bool                    mbResizeDisabled        : 1;
    bool                    mbAutoResize            : 1;
    bool                    mbEnableDelete          : 1;
    bool const              mbShowHeader;

    void            DeleteEntries();
    void            DoQuickSearch( sal_Unicode rChar );
    bool            Kill( const OUString& rURL );

protected:
    virtual bool     DoubleClickHdl() override;
    virtual OUString GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos = -1 ) const override;

public:
    ViewTabListBox_Impl( vcl::Window* pParentWin, SvtFileView_Impl* pParent, FileViewFlags nFlags );
    virtual ~ViewTabListBox_Impl() override;
    virtual void dispose() override;

    virtual void    Resize() override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    void            ClearAll();
    HeaderBar*      GetHeaderBar() const { return mpHeaderBar; }

    void            EnableAutoResize() { mbAutoResize = true; }
    void            EnableDelete( bool bEnable ) { mbEnableDelete = bEnable; }

    const Reference< XCommandEnvironment >& GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK(ResetQuickSearch_Impl, Timer *, void);

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    virtual void        ExecuteContextMenuAction( sal_uInt16 nSelectedPopentry ) override;
};


//= SvtFileView_Impl


class SvtFileView_Impl  :public ::svt::IEnumerationResultHandler
{
protected:
    VclPtr<SvtFileView>                 mpAntiImpl;
    Link<SvTreeListBox*,void>           m_aSelectHandler;

    ::rtl::Reference< ::svt::FileViewContentEnumerator >
                                        m_xContentEnumerator;
    Link<void*,void>                    m_aCurrentAsyncActionHandler;
    ::osl::Condition                    m_aAsyncActionFinished;
    ::rtl::Reference< ::salhelper::Timer > m_xCancelAsyncTimer;
    ::svt::EnumerationResult            m_eAsyncActionResult;
    bool                                m_bRunningAsyncAction;
    bool                                m_bAsyncActionCancelled;

public:

    ::std::vector< std::unique_ptr<SortingData_Impl> >  maContent;
    ::osl::Mutex                        maMutex;

    VclPtr<SvTreeListBox>               mpCurView;
    VclPtr<ViewTabListBox_Impl>         mpView;
    VclPtr<IconView>                    mpIconView;
    sal_uInt16              mnSortColumn;
    bool                    mbAscending     : 1;
    bool const              mbOnlyFolder    : 1;
    sal_Int16               mnSuspendSelectCallback : 1;
    bool                    mbIsFirstResort : 1;

    IntlWrapper const       aIntlWrapper;

    OUString                maViewURL;
    OUString                maCurrentFilter;
    Image const             maFolderImage;
    Link<SvtFileView*,void> maOpenDoneLink;
    Reference< XCommandEnvironment >    mxCmdEnv;

    SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > const & xEnv,
                                              FileViewFlags nFlags,
                                              bool bOnlyFolder );
    virtual                ~SvtFileView_Impl();

    void                    Clear();

    FileViewResult          GetFolderContent_Impl(
        const OUString& rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const css::uno::Sequence< OUString >& rBlackList );

    FileViewResult          GetFolderContent_Impl(
        const FolderDescriptor& _rFolder,
        const FileViewAsyncAction* pAsyncDescriptor,
        const css::uno::Sequence< OUString >& rBlackList );
    void                    FilterFolderContent_Impl( const OUString &rFilter );
    void                    CancelRunningAsyncAction();

    void                    OpenFolder_Impl();
    static void             ReplaceTabWithString( OUString& aValue );
    void                    CreateDisplayText_Impl();
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    OUString                FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    sal_uLong               GetEntryPos( const OUString& rURL );

    void                    SetViewMode( FileViewMode eMode );

    inline void             EnableDelete( bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, bool bAscending );
    bool                    SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             bool bWrapAround );

    void                    SetSelectHandler( const Link<SvTreeListBox*,void>& _rHdl );

    void                    InitSelection();
    void                    ResetCursor();

    inline void             EndEditing();

    void                    onTimeout();

protected:
    DECL_LINK( SelectionMultiplexer, SvTreeListBox*, void );

    // IEnumerationResultHandler overridables
    virtual void        enumerationDone( ::svt::EnumerationResult eResult ) override;
            void        implEnumerationSuccess();
};

inline void SvtFileView_Impl::EnableDelete( bool bEnable )
{
    mpView->EnableDelete( bEnable );
}

inline void SvtFileView_Impl::EndEditing()
{
    if ( mpCurView->IsEditingActive() )
        mpCurView->EndEditing();
}

namespace
{
    // functions -------------------------------------------------------------

    OUString CreateExactSizeText( sal_Int64 nSize )
    {
        double fSize( static_cast<double>(nSize) );
        int nDec;

        long nMega = 1024 * 1024;
        long nGiga = nMega * 1024;

        OUString aUnitStr(' ');

        if ( nSize < 10000 )
        {
            aUnitStr += SvtResId(STR_SVT_BYTES );
            nDec = 0;
        }
        else if ( nSize < nMega )
        {
            fSize /= 1024;
            aUnitStr += SvtResId(STR_SVT_KB);
            nDec = 1;
        }
        else if ( nSize < nGiga )
        {
            fSize /= nMega;
            aUnitStr += SvtResId(STR_SVT_MB);
            nDec = 2;
        }
        else
        {
            fSize /= nGiga;
            aUnitStr += SvtResId(STR_SVT_GB);
            nDec = 3;
        }

        OUString aSizeStr( ::rtl::math::doubleToUString( fSize,
                    rtl_math_StringFormat_F, nDec,
                    SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]) );
        aSizeStr += aUnitStr;

        return aSizeStr;
    }
}

ViewTabListBox_Impl::ViewTabListBox_Impl( vcl::Window* pParentWin,
                                          SvtFileView_Impl* pParent,
                                          FileViewFlags nFlags ) :

    SvHeaderTabListBox( pParentWin, WB_TABSTOP ),

    mpHeaderBar         ( nullptr ),
    mpParent            ( pParent ),
    msAccessibleDescText( SvtResId(STR_SVT_ACC_DESC_FILEVIEW) ),
    msFolder            ( SvtResId(STR_SVT_ACC_DESC_FOLDER) ),
    msFile              ( SvtResId(STR_SVT_ACC_DESC_FILE) ),
    mnSearchIndex       ( 0 ),
    mbResizeDisabled    ( false ),
    mbAutoResize        ( false ),
    mbEnableDelete      ( false ),
    mbShowHeader        ( !(nFlags & FileViewFlags::SHOW_NONE) )
{
    Size aBoxSize = pParentWin->GetSizePixel();
    mpHeaderBar = VclPtr<HeaderBar>::Create( pParentWin, WB_BUTTONSTYLE | WB_BOTTOMBORDER );
    mpHeaderBar->SetPosSizePixel( Point( 0, 0 ), mpHeaderBar->CalcWindowSizePixel() );

    HeaderBarItemBits nBits = HeaderBarItemBits::LEFT | HeaderBarItemBits::CLICKABLE;

    long aTabPositions[] = { 20, 180, 320, 400, 600 };
    SetTabs(SAL_N_ELEMENTS(aTabPositions), aTabPositions, MapUnit::MapPixel);
    SetTabJustify(2, SvTabJustify::AdjustRight); // column "Size"

    mpHeaderBar->InsertItem(COLUMN_TITLE, SvtResId(STR_SVT_FILEVIEW_COLUMN_TITLE), 180, nBits | HeaderBarItemBits::UPARROW);
    if (nFlags & FileViewFlags::SHOW_TYPE)
    {
        mpHeaderBar->InsertItem(COLUMN_TYPE, SvtResId(STR_SVT_FILEVIEW_COLUMN_TYPE), 140, nBits);
    }
    mpHeaderBar->InsertItem(COLUMN_SIZE, SvtResId(STR_SVT_FILEVIEW_COLUMN_SIZE), 80, nBits);
    mpHeaderBar->InsertItem(COLUMN_DATE, SvtResId(STR_SVT_FILEVIEW_COLUMN_DATE), 500, nBits);

    Size aHeadSize = mpHeaderBar->GetSizePixel();
    SetPosSizePixel( Point( 0, aHeadSize.Height() ),
                     Size( aBoxSize.Width(), aBoxSize.Height() - aHeadSize.Height() ) );
    InitHeaderBar( mpHeaderBar );
    SetHighlightRange();
    SetEntryHeight( ROW_HEIGHT );
    if (nFlags & FileViewFlags::MULTISELECTION)
        SetSelectionMode( SelectionMode::Multiple );

    Show();
    if( mbShowHeader )
        mpHeaderBar->Show();

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetInvokeHandler( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, VCLUnoHelper::GetInterface(GetParentDialog())), UNO_QUERY_THROW );

    mxCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    EnableContextMenuHandling();
}

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
    disposeOnce();
}

void ViewTabListBox_Impl::dispose()
{
    maResetQuickSearch.Stop();

    mxMenu.disposeAndClear();
    mxBuilder.reset();

    mpHeaderBar.disposeAndClear();
    SvHeaderTabListBox::dispose();
}

IMPL_LINK_NOARG(ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer *, void)
{
    ::osl::MutexGuard aGuard( maMutex );

    maQuickSearchText.clear();
    mnSearchIndex = 0;
}


void ViewTabListBox_Impl::Resize()
{
    SvTabListBox::Resize();
    Size aBoxSize = Control::GetParent()->GetOutputSizePixel();

    if ( mbResizeDisabled || !aBoxSize.Width() )
        return;

    Size aBarSize;
    if ( mbShowHeader )
    {
        aBarSize = mpHeaderBar->GetSizePixel();
        aBarSize.setWidth( mbAutoResize ? aBoxSize.Width() : GetSizePixel().Width() );
        mpHeaderBar->SetSizePixel( aBarSize );
    }

    if ( mbAutoResize )
    {
        mbResizeDisabled = true;
        SetPosSizePixel( Point( 0, aBarSize.Height() ),
                        Size( aBoxSize.Width(), aBoxSize.Height() - aBarSize.Height() ) );
        mbResizeDisabled = false;
    }
}


void ViewTabListBox_Impl::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;

    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if ( 0 == rKeyCode.GetModifier() )
    {
        if ( rKeyCode.GetCode() == KEY_RETURN )
        {
            ResetQuickSearch_Impl( nullptr );
            GetDoubleClickHdl().Call( this );
            bHandled = true;
        }
        else if ( ( rKeyCode.GetCode() == KEY_DELETE ) &&
                  mbEnableDelete )
        {
            ResetQuickSearch_Impl( nullptr );
            DeleteEntries();
            bHandled = true;
        }
        else if ( ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_NUM ) ||
                  ( rKEvt.GetKeyCode().GetGroup() == KEYGROUP_ALPHA ) )
        {
            DoQuickSearch( rKEvt.GetCharCode() );
            bHandled = true;
        }
    }

    if ( !bHandled )
    {
        ResetQuickSearch_Impl( nullptr );
        SvHeaderTabListBox::KeyInput( rKEvt );
    }
}


VclPtr<PopupMenu> ViewTabListBox_Impl::CreateContextMenu()
{
    bool bEnableDelete = mbEnableDelete;
    bool bEnableRename = true;

    if ( bEnableDelete || bEnableRename )
    {
        sal_Int32 nSelectedEntries = GetSelectionCount();
        bEnableDelete &= nSelectedEntries > 0;
        bEnableRename &= nSelectedEntries == 1;
    }

    if ( bEnableDelete || bEnableRename )
    {
        SvTreeListEntry* pEntry = FirstSelected();
        while ( pEntry )
        {
            ::ucbhelper::Content aCnt;
            try
            {
                OUString aURL( static_cast< SvtContentEntry * >(
                    pEntry->GetUserData() )->maURL );
                aCnt = ::ucbhelper::Content( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );
            }
            catch( Exception const & )
            {
                bEnableDelete = bEnableRename = false;
            }

            if ( bEnableDelete )
            {
                try
                {
                    Reference< XCommandInfo > aCommands = aCnt.getCommands();
                    if ( aCommands.is() )
                        bEnableDelete = aCommands->hasCommandByName( "delete" );
                    else
                        bEnableDelete = false;
                }
                catch( Exception const & )
                {
                    bEnableDelete = false;
                }
            }

            if ( bEnableRename )
            {
                try
                {
                    Reference< XPropertySetInfo > aProps = aCnt.getProperties();
                    if ( aProps.is() )
                    {
                        Property aProp = aProps->getPropertyByName("Title");
                        bEnableRename
                            = !( aProp.Attributes & PropertyAttribute::READONLY );
                    }
                    else
                        bEnableRename = false;
                }
                catch( Exception const & )
                {
                    bEnableRename = false;
                }
            }

            pEntry = ( bEnableDelete || bEnableRename )
                ? NextSelected( pEntry )
                : nullptr;
        }
    }

    if ( bEnableDelete || bEnableRename )
    {
        mxMenu.disposeAndClear();
        mxBuilder.reset(new VclBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svt/ui/fileviewmenu.ui", ""));
        mxMenu.set(mxBuilder->get_menu("menu"));
        mxMenu->EnableItem(mxMenu->GetItemId("delete"), bEnableDelete);
        mxMenu->EnableItem(mxMenu->GetItemId("rename"), bEnableRename);
        mxMenu->RemoveDisabledEntries( true, true );
        return mxMenu;
    }

    return nullptr;
}

void ViewTabListBox_Impl::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    if (nSelectedPopupEntry == mxMenu->GetItemId("delete"))
        DeleteEntries();
    else if (nSelectedPopupEntry == mxMenu->GetItemId("rename"))
        EditEntry( FirstSelected() );
}

void ViewTabListBox_Impl::ClearAll()
{
    for ( sal_uLong i = 0; i < GetEntryCount(); ++i )
        delete static_cast<SvtContentEntry*>(GetEntry(i)->GetUserData());
    Clear();
}


void ViewTabListBox_Impl::DeleteEntries()
{
    short eResult = svtools::QUERYDELETE_YES;
    SvTreeListEntry* pEntry = FirstSelected();
    OUString aURL;

    while ( pEntry )
    {
        SvTreeListEntry *pCurEntry = pEntry;
        pEntry = NextSelected( pEntry );

        if ( pCurEntry->GetUserData() )
            aURL = static_cast<SvtContentEntry*>(pCurEntry->GetUserData())->maURL;

        if ( aURL.isEmpty() )
            continue;

        bool canDelete = true;
        try
        {
            ::ucbhelper::Content aCnt( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );
            Reference< XCommandInfo > aCommands = aCnt.getCommands();
            if ( aCommands.is() )
                canDelete = aCommands->hasCommandByName( "delete" );
            else
                canDelete = false;
        }
        catch( Exception const & )
        {
            canDelete = false;
        }

        if (!canDelete)
            continue; // process next entry

        if ( eResult != svtools::QUERYDELETE_ALL )
        {
            INetURLObject aObj( aURL );
            svtools::QueryDeleteDlg_Impl aDlg(GetFrameWeld(), aObj.GetName(INetURLObject::DecodeMechanism::WithCharset));

            if ( GetSelectionCount() > 1 )
                aDlg.EnableAllButton();

            eResult = aDlg.run();
        }

        if ( ( eResult == svtools::QUERYDELETE_ALL ) ||
             ( eResult == svtools::QUERYDELETE_YES ) )
        {
            if ( Kill( aURL ) )
            {
                delete static_cast<SvtContentEntry*>(pCurEntry->GetUserData());
                GetModel()->Remove( pCurEntry );
                mpParent->EntryRemoved( aURL );
            }
        }
    }
}


bool ViewTabListBox_Impl::EditedEntry( SvTreeListEntry* pEntry,
                                 const OUString& rNewText )
{
    bool bRet = false;

    OUString aURL;
    SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    if ( pData )
        aURL = pData->maURL;

    if ( aURL.isEmpty() )
        return bRet;

    try
    {
        OUString aPropName( "Title" );
        bool canRename = true;
        ::ucbhelper::Content aContent( aURL, mxCmdEnv, comphelper::getProcessComponentContext() );

        try
        {
            Reference< XPropertySetInfo > aProps = aContent.getProperties();
            if ( aProps.is() )
            {
                Property aProp = aProps->getPropertyByName( aPropName );
                canRename = !( aProp.Attributes & PropertyAttribute::READONLY );
            }
            else
            {
                canRename = false;
            }
        }
        catch ( Exception const & )
        {
            canRename = false;
        }

        if ( canRename )
        {
            Any aValue;
            aValue <<= rNewText;
            aContent.setPropertyValue( aPropName, aValue );
            mpParent->EntryRenamed( aURL, rNewText );

            if (pData)
                pData->maURL = aURL;

            pEntry->SetUserData( pData );

            bRet = true;
        }
    }
    catch( Exception const & )
    {
    }

    return bRet;
}


void ViewTabListBox_Impl::DoQuickSearch( sal_Unicode rChar )
{
    ::osl::MutexGuard aGuard( maMutex );

    maResetQuickSearch.Stop();

    OUString    aLastText = maQuickSearchText;
    sal_uInt32  aLastPos = mnSearchIndex;

    maQuickSearchText += OUString(rChar).toAsciiLowerCase();

    bool bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, false );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUStringLiteral1(rChar) ) )
    {
        mnSearchIndex = aLastPos + 1;
        maQuickSearchText = aLastText;
        bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, true );
    }

    if ( bFound )
    {
        SvTreeListEntry* pEntry = GetEntry( mnSearchIndex );
        if ( pEntry )
        {
            SelectAll( false );
            Select( pEntry );
            SetCurEntry( pEntry );
            MakeVisible( pEntry );
        }
    }

    maResetQuickSearch.Start();
}


bool ViewTabListBox_Impl::DoubleClickHdl()
{
    SvHeaderTabListBox::DoubleClickHdl();
    return false;
        // this means "do no additional handling". Especially this means that the SvImpLBox does not
        // recognize that the entry at the double click position change after the handler call (which is
        // the case if in the handler, our content was replaced)
        // If it _would_ recognize this change, it would take this as a reason to select the entry, again
        // - which is not what in the case of content replace
        // (I really doubt that this behaviour of the SvImpLBox does make any sense at all, but
        // who knows ...)
}

OUString ViewTabListBox_Impl::GetAccessibleObjectDescription( ::vcl::AccessibleBrowseBoxObjType _eType, sal_Int32 _nPos ) const
{
    OUString sRet = SvHeaderTabListBox::GetAccessibleObjectDescription( _eType, _nPos );
    if ( ::vcl::BBTYPE_TABLECELL == _eType )
    {
        sal_Int32 nRow = -1;
        const sal_uInt16 nColumnCount = GetColumnCount();
        if (nColumnCount > 0)
            nRow = _nPos / nColumnCount;
        SvTreeListEntry* pEntry = GetEntry( nRow );
        if ( pEntry )
        {
            SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());
            if ( pData )
            {
                const OUString sVar1( "%1" );
                const OUString sVar2( "%2" );
                OUString aText( msAccessibleDescText );
                aText = aText.replaceAll( sVar1, pData->mbIsFolder ? msFolder : msFile );
                aText = aText.replaceAll( sVar2, pData->maURL );
                sRet += aText;
            }
        }
    }

    return sRet;
}


bool ViewTabListBox_Impl::Kill( const OUString& rContent )
{
    bool bRet = true;

    try
    {
        ::ucbhelper::Content aCnt( rContent, mxCmdEnv, comphelper::getProcessComponentContext() );
        aCnt.executeCommand( "delete", makeAny( true ) );
    }
    catch( css::ucb::CommandAbortedException const & )
    {
        SAL_INFO( "svtools.contnr", "CommandAbortedException" );
        bRet = false;
    }
    catch( Exception const & )
    {
        SAL_INFO( "svtools.contnr", "Any other exception" );
        bRet = false;
    }

    return bRet;
}

SvtFileView::SvtFileView( vcl::Window* pParent, WinBits nBits,
                          bool bOnlyFolder, bool bMultiSelection, bool bShowType ) :

    Control( pParent, nBits )
{
    FileViewFlags nFlags = FileViewFlags::NONE;
    if ( bOnlyFolder )
        nFlags |= FileViewFlags::ONLYFOLDER;
    if ( bMultiSelection )
        nFlags |= FileViewFlags::MULTISELECTION;
    if ( bShowType )
        nFlags |= FileViewFlags::SHOW_TYPE;

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, VCLUnoHelper::GetInterface(GetParentDialog())), UNO_QUERY_THROW );
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mpImpl.reset( new SvtFileView_Impl( this, xCmdEnv, nFlags, bOnlyFolder ) );
    mpImpl->mpView->ForbidEmptyText();

    HeaderBar* pHeaderBar = mpImpl->mpView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvtFileView, HeaderEndDrag_Impl ) );
}

SvtFileView::~SvtFileView()
{
    disposeOnce();
}

void SvtFileView::dispose()
{
    mpImpl.reset();
    Control::dispose();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSvtFileView(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;

    bool bDropdown = BuilderUtils::extractDropdown(rMap);

    if (bDropdown)
        nBits |= WB_DROPDOWN;

    rRet = VclPtr<SvtFileView>::Create(pParent, nBits, true, true);
}

Size SvtFileView::GetOptimalSize() const
{
    return LogicToPixel(Size(208, 50), MapMode(MapUnit::MapAppFont));
}

void SvtFileView::SetViewMode( FileViewMode eMode )
{
    mpImpl->SetViewMode( eMode );
}

OUString SvtFileView::GetURL( SvTreeListEntry const * pEntry )
{
    OUString aURL;
    if ( pEntry && pEntry->GetUserData() )
        aURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;
    return aURL;
}


OUString SvtFileView::GetCurrentURL() const
{
    OUString aURL;
    SvTreeListEntry* pEntry = mpImpl->mpCurView->FirstSelected();
    if ( pEntry && pEntry->GetUserData() )
        aURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;
    return aURL;
}


void SvtFileView::CreatedFolder( const OUString& rUrl, const OUString& rNewFolder )
{
    OUString sEntry = mpImpl->FolderInserted( rUrl, rNewFolder );

    SvTreeListEntry* pEntry = mpImpl->mpView->InsertEntry( sEntry, mpImpl->maFolderImage, mpImpl->maFolderImage );
    SvtContentEntry* pUserData = new SvtContentEntry( rUrl, true );
    pEntry->SetUserData( pUserData );
    mpImpl->mpView->MakeVisible( pEntry );

    SvTreeListEntry* pEntry2 = mpImpl->mpIconView->InsertEntry( sEntry.getToken( 0, '\t' ), mpImpl->maFolderImage, mpImpl->maFolderImage );
    SvtContentEntry* pUserData2 = new SvtContentEntry( rUrl, true );
    pEntry2->SetUserData( pUserData2 );
    mpImpl->mpIconView->MakeVisible( pEntry2 );
}


FileViewResult SvtFileView::PreviousLevel( const FileViewAsyncAction* pAsyncDescriptor )
{
    FileViewResult eResult = eFailure;

    OUString sParentURL;
    if ( GetParentURL( sParentURL ) )
        eResult = Initialize( sParentURL, mpImpl->maCurrentFilter, pAsyncDescriptor, mpBlackList );

    return eResult;
}


bool SvtFileView::GetParentURL( OUString& rParentURL ) const
{
    bool bRet = false;
    try
    {
        ::ucbhelper::Content aCnt( mpImpl->maViewURL, mpImpl->mxCmdEnv, comphelper::getProcessComponentContext() );
        Reference< XContent > xContent( aCnt.get() );
        Reference< css::container::XChild > xChild( xContent, UNO_QUERY );
        if ( xChild.is() )
        {
            Reference< XContent > xParent( xChild->getParent(), UNO_QUERY );
            if ( xParent.is() )
            {
                rParentURL = xParent->getIdentifier()->getContentIdentifier();
                bRet = !rParentURL.isEmpty() && rParentURL != mpImpl->maViewURL;
            }
        }
    }
    catch( Exception const & )
    {
        // perhaps an unknown url protocol (e.g. "private:newdoc")
    }

    return bRet;
}


const OString& SvtFileView::GetHelpId( ) const
{
    return mpImpl->mpView->GetHelpId( );
}


void SvtFileView::SetHelpId( const OString& rHelpId )
{
    mpImpl->mpView->SetHelpId( rHelpId );
}


void SvtFileView::SetSizePixel( const Size& rNewSize )
{
    Control::SetSizePixel( rNewSize );
    mpImpl->mpView->SetSizePixel( rNewSize );
    mpImpl->mpIconView->SetSizePixel( rNewSize );
}


void SvtFileView::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosPixel( rNewPos );
    SetSizePixel( rNewSize );
}


bool SvtFileView::Initialize( const css::uno::Reference< css::ucb::XContent>& _xContent  )
{
    WaitObject aWaitCursor( this );

    mpImpl->Clear();
    ::ucbhelper::Content aContent(_xContent, mpImpl->mxCmdEnv, comphelper::getProcessComponentContext() );
    FileViewResult eResult = mpImpl->GetFolderContent_Impl( FolderDescriptor( aContent ), nullptr, css::uno::Sequence< OUString >() );
    OSL_ENSURE( eResult != eStillRunning, "SvtFileView::Initialize: this was expected to be synchronous!" );
    if ( eResult != eSuccess )
        return false;

    mpImpl->FilterFolderContent_Impl( OUString() );

    mpImpl->SortFolderContent_Impl(); // possibly not necessary!!!!!!!!!!
    mpImpl->CreateDisplayText_Impl();
    mpImpl->OpenFolder_Impl();

    mpImpl->maOpenDoneLink.Call( this );
    return true;
}


FileViewResult SvtFileView::Initialize(
    const OUString& rURL,
    const OUString& rFilter,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList  )
{
    WaitObject aWaitCursor( this );
    mpBlackList = rBlackList;

    OUString sPushURL( mpImpl->maViewURL );

    mpImpl->maViewURL = rURL;
    FileViewResult eResult = ExecuteFilter( rFilter, pAsyncDescriptor );
    switch ( eResult )
    {
    case eFailure:
    case eTimeout:
        mpImpl->maViewURL = sPushURL;
        return eResult;

    case eStillRunning:
        OSL_ENSURE( pAsyncDescriptor, "SvtFileView::Initialize: we told it to read synchronously!" );
        [[fallthrough]];
    case eSuccess:
        return eResult;
    }

    OSL_FAIL( "SvtFileView::Initialize: unreachable!" );
    return eFailure;
}

FileViewResult SvtFileView::ExecuteFilter( const OUString& rFilter, const FileViewAsyncAction* pAsyncDescriptor )
{
    mpImpl->maCurrentFilter = rFilter.toAsciiLowerCase();

    mpImpl->Clear();
    FileViewResult eResult = mpImpl->GetFolderContent_Impl( mpImpl->maViewURL, pAsyncDescriptor, mpBlackList );
    OSL_ENSURE( ( eResult != eStillRunning ) || pAsyncDescriptor, "SvtFileView::ExecuteFilter: we told it to read synchronously!" );
    return eResult;
}

void SvtFileView::CancelRunningAsyncAction()
{
    mpImpl->CancelRunningAsyncAction();
}

void SvtFileView::SetNoSelection()
{
    mpImpl->mpCurView->SelectAll( false );
}


void SvtFileView::GetFocus()
{
    Control::GetFocus();
    if ( mpImpl && mpImpl->mpCurView )
        mpImpl->mpCurView->GrabFocus();
}


void SvtFileView::SetSelectHdl( const Link<SvTreeListBox*,void>& rHdl )
{
    mpImpl->SetSelectHandler( rHdl );
}


void SvtFileView::SetDoubleClickHdl( const Link<SvTreeListBox*,bool>& rHdl )
{
    mpImpl->mpView->SetDoubleClickHdl( rHdl );
    mpImpl->mpIconView->SetDoubleClickHdl( rHdl );
}


sal_uLong SvtFileView::GetSelectionCount() const
{
    return mpImpl->mpCurView->GetSelectionCount();
}


SvTreeListEntry* SvtFileView::FirstSelected() const
{
    return mpImpl->mpCurView->FirstSelected();
}


SvTreeListEntry* SvtFileView::NextSelected( SvTreeListEntry* pEntry ) const
{
    return mpImpl->mpCurView->NextSelected( pEntry );
}

void SvtFileView::EnableAutoResize()
{
    mpImpl->mpView->EnableAutoResize();
}

const OUString& SvtFileView::GetViewURL() const
{
    return mpImpl->maViewURL;
}

void SvtFileView::SetOpenDoneHdl( const Link<SvtFileView*,void>& rHdl )
{
    mpImpl->maOpenDoneLink = rHdl;
}

void SvtFileView::EnableDelete( bool bEnable )
{
    mpImpl->EnableDelete( bEnable );
}

void SvtFileView::EndInplaceEditing()
{
    return mpImpl->EndEditing();
}

IMPL_LINK( SvtFileView, HeaderSelect_Impl, HeaderBar*, pBar, void )
{
    DBG_ASSERT( pBar, "no headerbar" );
    sal_uInt16 nItemID = pBar->GetCurItemId();

    HeaderBarItemBits nBits;

    // clear the arrow of the recently used column
    if ( nItemID != mpImpl->mnSortColumn )
    {
        if ( !nItemID )
        {
            // first call -> remove arrow from title column,
            // because another column is the sort column
            nItemID = mpImpl->mnSortColumn;
            mpImpl->mnSortColumn = COLUMN_TITLE;
        }
        nBits = pBar->GetItemBits( mpImpl->mnSortColumn );
        nBits &= ~HeaderBarItemBits( HeaderBarItemBits::UPARROW | HeaderBarItemBits::DOWNARROW );
        pBar->SetItemBits( mpImpl->mnSortColumn, nBits );
    }

    nBits = pBar->GetItemBits( nItemID );

    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );

    if ( bUp )
    {
        nBits &= ~HeaderBarItemBits::UPARROW;
        nBits |= HeaderBarItemBits::DOWNARROW;
    }
    else
    {
        nBits &= ~HeaderBarItemBits::DOWNARROW;
        nBits |= HeaderBarItemBits::UPARROW;
    }

    pBar->SetItemBits( nItemID, nBits );
    mpImpl->Resort_Impl( nItemID, !bUp );
}


IMPL_LINK( SvtFileView, HeaderEndDrag_Impl, HeaderBar*, pBar, void )
{
    if ( pBar->IsItemMode() )
        return;

    Size aSize;
    sal_uInt16 nTabs = pBar->GetItemCount();
    long nTmpSize = 0;

    for ( sal_uInt16 i = 1; i <= nTabs; ++i )
    {
        long nWidth = pBar->GetItemSize(i);
        aSize.setWidth(  nWidth + nTmpSize );
        nTmpSize += nWidth;
        mpImpl->mpView->SetTab( i, aSize.Width(), MapUnit::MapPixel );
    }
}


OUString SvtFileView::GetConfigString() const
{
    OUString sRet;
    HeaderBar* pBar = mpImpl->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    // sort order
    sRet += OUString::number( mpImpl->mnSortColumn ) + ";";
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImpl->mnSortColumn );
    bool bUp = ( ( nBits & HeaderBarItemBits::UPARROW ) == HeaderBarItemBits::UPARROW );
    sRet += bUp ? OUString("1") : OUString("0");
    sRet += ";";

    sal_uInt16 nCount = pBar->GetItemCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        sal_uInt16 nId = pBar->GetItemId(i);
        sRet += OUString::number( nId )
                + ";"
                + OUString::number( pBar->GetItemSize( nId ) )
                + ";";
    }

    sRet = comphelper::string::stripEnd(sRet, ';');
    return sRet;
}

::std::vector< SvtContentEntry > SvtFileView::GetContent()
{
    ::std::vector< SvtContentEntry > aContent;

    for(auto const& elem : mpImpl->maContent)
    {
        SvtContentEntry aEntry( elem->maTargetURL, elem->mbIsFolder );
        aContent.push_back( aEntry );
    }

    return aContent;
}

void SvtFileView::SetConfigString( const OUString& rCfgStr )
{
    HeaderBar* pBar = mpImpl->mpView->GetHeaderBar();
    DBG_ASSERT( pBar, "invalid headerbar" );

    sal_Int32 nIdx = 0;
    mpImpl->mnSortColumn = static_cast<sal_uInt16>(rCfgStr.getToken( 0, ';', nIdx ).toInt32());
    bool bUp = static_cast<bool>(static_cast<sal_uInt16>(rCfgStr.getToken( 0, ';', nIdx ).toInt32()));
    HeaderBarItemBits nBits = pBar->GetItemBits( mpImpl->mnSortColumn );

    if ( bUp )
    {
        nBits &= ~HeaderBarItemBits::UPARROW;
        nBits |= HeaderBarItemBits::DOWNARROW;
    }
    else
    {
        nBits &= ~HeaderBarItemBits::DOWNARROW;
        nBits |= HeaderBarItemBits::UPARROW;
    }
    pBar->SetItemBits( mpImpl->mnSortColumn, nBits );

    while ( nIdx != -1 )
    {
        sal_uInt16 nItemId = static_cast<sal_uInt16>(rCfgStr.getToken( 0, ';', nIdx ).toInt32());
        pBar->SetItemSize( nItemId, rCfgStr.getToken( 0, ';', nIdx ).toInt32() );
    }

    HeaderSelect_Impl( pBar );
    HeaderEndDrag_Impl( pBar );
}


void SvtFileView::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::Enable )
        Invalidate();
    Control::StateChanged( nStateChange );
}


// class SvtFileView_Impl


SvtFileView_Impl::SvtFileView_Impl( SvtFileView* pAntiImpl, Reference < XCommandEnvironment > const & xEnv, FileViewFlags nFlags, bool bOnlyFolder )

    :mpAntiImpl                 ( pAntiImpl )
    ,m_eAsyncActionResult       ( ::svt::EnumerationResult::ERROR )
    ,m_bRunningAsyncAction      ( false )
    ,m_bAsyncActionCancelled    ( false )
    ,mnSortColumn               ( COLUMN_TITLE )
    ,mbAscending                ( true )
    ,mbOnlyFolder               ( bOnlyFolder )
    ,mnSuspendSelectCallback    ( 0 )
    ,mbIsFirstResort            ( true )
    ,aIntlWrapper               ( Application::GetSettings().GetLanguageTag() )
    ,maFolderImage              (StockImage::Yes, RID_BMP_FOLDER)
    ,mxCmdEnv ( xEnv )

{
    mpView = VclPtr<ViewTabListBox_Impl>::Create( mpAntiImpl, this, nFlags );
    mpCurView = mpView;
    mpIconView = VclPtr<IconView>::Create( mpAntiImpl, WB_TABSTOP );
    mpIconView->Hide();
    mpView->EnableCellFocus();
}


SvtFileView_Impl::~SvtFileView_Impl()
{
    Clear();
    mpView.disposeAndClear();
    mpCurView.clear();
    mpIconView.disposeAndClear();
    mpAntiImpl.clear();
}


void SvtFileView_Impl::Clear()
{
    ::osl::MutexGuard aGuard( maMutex );

    maContent.clear();
}


FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const OUString& rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList )
{
    ::osl::ClearableMutexGuard aGuard( maMutex );
    INetURLObject aFolderObj( rFolder );
    DBG_ASSERT( aFolderObj.GetProtocol() != INetProtocol::NotValid, "Invalid URL!" );

    FolderDescriptor aFolder( aFolderObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    aGuard.clear();
    return GetFolderContent_Impl( aFolder, pAsyncDescriptor, rBlackList );
}


FileViewResult SvtFileView_Impl::GetFolderContent_Impl(
    const FolderDescriptor& _rFolder,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList )
{
    DBG_TESTSOLARMUTEX();
    ::osl::ClearableMutexGuard aGuard( maMutex );

    OSL_ENSURE( !m_xContentEnumerator.is(), "SvtFileView_Impl::GetFolderContent_Impl: still running another enumeration!" );
    m_xContentEnumerator.set(new ::svt::FileViewContentEnumerator(
        mpView->GetCommandEnvironment(), maContent, maMutex));
        // TODO: should we cache and re-use this thread?

    if ( !pAsyncDescriptor )
    {
        ::svt::EnumerationResult eResult = m_xContentEnumerator->enumerateFolderContentSync( _rFolder, rBlackList );
        if ( ::svt::EnumerationResult::SUCCESS == eResult )
        {
            implEnumerationSuccess();
            m_xContentEnumerator.clear();
            return eSuccess;
        }
        m_xContentEnumerator.clear();
        return eFailure;
    }

    m_bRunningAsyncAction = true;
    m_bAsyncActionCancelled = false;
    m_eAsyncActionResult = ::svt::EnumerationResult::ERROR;
    m_aAsyncActionFinished.reset();

    // don't (yet) set m_aCurrentAsyncActionHandler to pTimeout->aFinishHandler.
    // By definition, this handler *only* gets called when the result cannot be obtained
    // during the minimum wait time, so it is only set below, when needed.
    m_aCurrentAsyncActionHandler = Link<void*,void>();

    // minimum time to wait
    std::unique_ptr< TimeValue > pTimeout( new TimeValue );
    sal_Int32 nMinTimeout = pAsyncDescriptor->nMinTimeout;
    OSL_ENSURE( nMinTimeout > 0, "SvtFileView_Impl::GetFolderContent_Impl: invalid minimum timeout!" );
    if ( nMinTimeout <= 0 )
        nMinTimeout = sal_Int32( 1000 );
    pTimeout->Seconds = nMinTimeout / 1000L;
    pTimeout->Nanosec = ( nMinTimeout % 1000L ) * 1000000L;

    m_xContentEnumerator->enumerateFolderContent( _rFolder, this );

    // wait until the enumeration is finished
    // for this, release our own mutex (which is used by the enumerator thread)
    aGuard.clear();

    ::osl::Condition::Result eResult = ::osl::Condition::result_ok;
    {
        // also release the SolarMutex. Not all code which is needed during the enumeration
        // is Solar-Thread-Safe, in particular there is some code which needs to access
        // string resources (and our resource system relies on the SolarMutex :()
        SolarMutexReleaser aSolarRelease;

        // now wait. Note that if we didn't get an pAsyncDescriptor, then this is an infinite wait.
        eResult = m_aAsyncActionFinished.wait( pTimeout.get() );
    }

    ::osl::MutexGuard aGuard2( maMutex );
    if ( ::osl::Condition::result_timeout == eResult )
    {
        // maximum time to wait
        OSL_ENSURE(!m_xCancelAsyncTimer,
                   "SvtFileView_Impl::GetFolderContent_Impl: there's still a previous timer!");
        m_xCancelAsyncTimer.set(new CallbackTimer(this));
        sal_Int32 nMaxTimeout = pAsyncDescriptor->nMaxTimeout;
        OSL_ENSURE( nMaxTimeout > nMinTimeout,
            "SvtFileView_Impl::GetFolderContent_Impl: invalid maximum timeout!" );
        if ( nMaxTimeout <= nMinTimeout )
            nMaxTimeout = nMinTimeout + 5000;
        m_xCancelAsyncTimer->setRemainingTime( salhelper::TTimeValue( nMaxTimeout - nMinTimeout ) );
            // we already waited for nMinTimeout milliseconds, so take this into account
        m_xCancelAsyncTimer->start();

        m_aCurrentAsyncActionHandler = pAsyncDescriptor->aFinishHandler;
        DBG_ASSERT( m_aCurrentAsyncActionHandler.IsSet(), "SvtFileView_Impl::GetFolderContent_Impl: nobody interested when it's finished?" );
        mpView->ClearAll();
        mpIconView->ClearAll();
        return eStillRunning;
    }

    m_bRunningAsyncAction = false;
    switch ( m_eAsyncActionResult )
    {
    case ::svt::EnumerationResult::SUCCESS:
        return eSuccess;

    case ::svt::EnumerationResult::ERROR:
        return eFailure;
    }

    SAL_WARN( "svtools.contnr", "SvtFileView_Impl::GetFolderContent_Impl: unreachable!" );
    return eFailure;
}


void SvtFileView_Impl::FilterFolderContent_Impl( const OUString &rFilter )
{
    if ( rFilter.isEmpty() || ( rFilter == ALL_FILES_FILTER ) )
        // when replacing names, there is always something to filter (no view of ".nametranslation.table")
        return;

    ::osl::MutexGuard aGuard( maMutex );

    if ( maContent.empty() )
        return;

    // collect the filter tokens
    ::std::vector< WildCard > aFilters;
    FilterMatch::createWildCardFilterList(rFilter,aFilters);


    // do the filtering
    maContent.erase(std::remove_if(maContent.begin(), maContent.end(),
        [&aFilters](const std::unique_ptr<SortingData_Impl>& rxContent) {
            if (rxContent->mbIsFolder)
                return false;
            // normalize the content title (we always match case-insensitive)
            // 91872 - 11.09.2001 - frank.schoenheit@sun.com
            OUString sCompareString = rxContent->GetFileName(); // filter works on file name, not on title!
            return std::none_of(aFilters.begin(), aFilters.end(), FilterMatch(sCompareString));
        }),
        maContent.end());
}


IMPL_LINK( SvtFileView_Impl, SelectionMultiplexer, SvTreeListBox*, _pSource, void )
{
    if (!mnSuspendSelectCallback)
        m_aSelectHandler.Call( _pSource );
}


void SvtFileView_Impl::SetSelectHandler( const Link<SvTreeListBox*,void>& _rHdl )
{
    m_aSelectHandler = _rHdl;

    Link<SvTreeListBox*,void> aMasterHandler;
    if ( m_aSelectHandler.IsSet() )
        aMasterHandler = LINK( this, SvtFileView_Impl, SelectionMultiplexer );

    mpView->SetSelectHdl( aMasterHandler );
    mpIconView->SetSelectHdl( aMasterHandler );
}


void SvtFileView_Impl::InitSelection()
{
    mpCurView->SelectAll( false );
    SvTreeListEntry* pFirst = mpCurView->First();
    if ( pFirst )
        mpCurView->SetCursor( pFirst, true );
}


void SvtFileView_Impl::OpenFolder_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    mpView->SetUpdateMode( false );
    mpIconView->SetUpdateMode( false );
    mpView->ClearAll();
    mpIconView->ClearAll();

    for (auto const& elem : maContent)
    {
        if ( mbOnlyFolder && ! elem->mbIsFolder )
            continue;

        // insert entry and set user data
        SvTreeListEntry* pEntry = mpView->InsertEntry( elem->maDisplayText,
                                                   elem->maImage,
                                                   elem->maImage );

        SvTreeListEntry* pEntry2 = mpIconView->InsertEntry( elem->maDisplayText.getToken( 0, '\t' ),
                                                   elem->maImage, elem->maImage );

        SvtContentEntry* pUserData = new SvtContentEntry( elem->maTargetURL,
                                                          elem->mbIsFolder );
        SvtContentEntry* pUserData2 = new SvtContentEntry( elem->maTargetURL,
                                                          elem->mbIsFolder );

        pEntry->SetUserData( pUserData );
        pEntry2->SetUserData( pUserData2 );
    }

    InitSelection();

    ++mnSuspendSelectCallback;
    mpView->SetUpdateMode( true );
    mpIconView->SetUpdateMode( true );
    --mnSuspendSelectCallback;

    ResetCursor();
}


void SvtFileView_Impl::ResetCursor()
{
    // deselect
    SvTreeListEntry* pEntry = mpCurView->FirstSelected();
    if ( pEntry )
        mpCurView->Select( pEntry, false );
    // set cursor to the first entry
    mpCurView->SetCursor( mpCurView->First(), true );
    mpCurView->Update();
}


void SvtFileView_Impl::CancelRunningAsyncAction()
{
    DBG_TESTSOLARMUTEX();
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_xContentEnumerator.is() )
        return;

    m_bAsyncActionCancelled = true;
    m_xContentEnumerator->cancel();
    m_bRunningAsyncAction = false;

    m_xContentEnumerator.clear();
    if ( m_xCancelAsyncTimer.is() && m_xCancelAsyncTimer->isTicking() )
        m_xCancelAsyncTimer->stop();
    m_xCancelAsyncTimer.clear();
}


void SvtFileView_Impl::onTimeout()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( maMutex );
    if ( !m_bRunningAsyncAction )
        // there might have been a race condition while we waited for the mutex
        return;

    CancelRunningAsyncAction();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( eTimeout ) );
        m_aCurrentAsyncActionHandler = Link<void*,void>();
    }
}


void SvtFileView_Impl::enumerationDone( ::svt::EnumerationResult eResult )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( maMutex );

    m_xContentEnumerator.clear();
    if ( m_xCancelAsyncTimer.is() && m_xCancelAsyncTimer->isTicking() )
        m_xCancelAsyncTimer->stop();
    m_xCancelAsyncTimer.clear();

    if ( m_bAsyncActionCancelled )
        // this is to prevent race conditions
        return;

    m_eAsyncActionResult = eResult;
    m_bRunningAsyncAction = false;

    m_aAsyncActionFinished.set();

    if ( svt::EnumerationResult::SUCCESS == eResult )
        implEnumerationSuccess();

    if ( m_aCurrentAsyncActionHandler.IsSet() )
    {
        Application::PostUserEvent( m_aCurrentAsyncActionHandler, reinterpret_cast< void* >( m_eAsyncActionResult ) );
        m_aCurrentAsyncActionHandler = Link<void*,void>();
    }
}


void SvtFileView_Impl::implEnumerationSuccess()
{
    FilterFolderContent_Impl( maCurrentFilter );
    SortFolderContent_Impl();
    CreateDisplayText_Impl();
    OpenFolder_Impl();
    maOpenDoneLink.Call( mpAntiImpl );
}


void SvtFileView_Impl::ReplaceTabWithString( OUString& aValue )
{
    OUString const aTab( "\t" );
    OUString const aTabString( "%09" );
    sal_Int32 iPos;

    while ( ( iPos = aValue.indexOf( aTab ) ) >= 0 )
       aValue = aValue.replaceAt( iPos, 1, aTabString );
}


void SvtFileView_Impl::CreateDisplayText_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString const aTab( "\t" );
    OUString const aDateSep( ", " );

    for (auto const& elem : maContent)
    {
        // title, type, size, date
        OUString aValue = elem->GetTitle();
        ReplaceTabWithString( aValue );
        aValue += aTab + elem->maType + aTab;
        // folders don't have a size
        if ( ! elem->mbIsFolder )
            aValue += CreateExactSizeText( elem->maSize );
        aValue += aTab;
        // set the date, but volumes have no date
        if ( ! elem->mbIsFolder || ! elem->mbIsVolume )
        {
            SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
            aValue += rLocaleData.getDate( elem->maModDate )
                    + aDateSep
                    + rLocaleData.getTime( elem->maModDate, false );
        }
        elem->maDisplayText = aValue;

        // detect image
        if ( elem->mbIsFolder )
        {
            ::svtools::VolumeInfo aVolInfo( elem->mbIsVolume, elem->mbIsRemote,
                                            elem->mbIsRemoveable, elem->mbIsFloppy,
                                            elem->mbIsCompactDisc );
            elem->maImage = SvFileInformationManager::GetFolderImage( aVolInfo );
        }
        else
            elem->maImage = SvFileInformationManager::GetFileImage( INetURLObject( elem->maTargetURL ) );
    }
}

void SvtFileView_Impl::Resort_Impl( sal_Int16 nColumn, bool bAscending )
{
    // TODO: IconView ()
    ::osl::MutexGuard aGuard( maMutex );

    if ( ( nColumn == mnSortColumn ) &&
         ( bAscending == mbAscending ) )
         return;

    // reset the quick search index
    mpView->ResetQuickSearch_Impl( nullptr );

    OUString aEntryURL;
    SvTreeListEntry* pEntry = mpView->GetCurEntry();
    if ( pEntry && pEntry->GetUserData() )
        aEntryURL = static_cast<SvtContentEntry*>(pEntry->GetUserData())->maURL;

    mnSortColumn = nColumn;
    mbAscending = bAscending;

    SortFolderContent_Impl();
    OpenFolder_Impl();

    if ( !mbIsFirstResort )
    {
        sal_uLong nPos = GetEntryPos( aEntryURL );
        if ( nPos < mpView->GetEntryCount() )
        {
            pEntry = mpView->GetEntry( nPos );

            ++mnSuspendSelectCallback;  // #i15668#
            mpView->SetCurEntry( pEntry );
            --mnSuspendSelectCallback;
        }
    }
    else
        mbIsFirstResort = false;
}


static bool                     gbAscending = true;
static sal_Int16                gnColumn = COLUMN_TITLE;
static const CollatorWrapper*   pCollatorWrapper = nullptr;

/* this function returns true, if aOne is less then aTwo
*/
static bool CompareSortingData_Impl( std::unique_ptr<SortingData_Impl> const & aOne, std::unique_ptr<SortingData_Impl> const & aTwo )
{
    DBG_ASSERT( pCollatorWrapper, "*CompareSortingData_Impl(): Can't work this way!" );

    sal_Int32 nComp;
    bool      bRet = false;
    bool      bEqual = false;

    if ( aOne->mbIsFolder != aTwo->mbIsFolder )
    {
        bRet = aOne->mbIsFolder;

        // !!! pb: #100376# folder always on top
        if ( !gbAscending )
            bRet = !bRet;
    }
    else
    {
        switch ( gnColumn )
        {
            case COLUMN_TITLE:
                // compare case insensitive first
                nComp = pCollatorWrapper->compareString( aOne->GetLowerTitle(), aTwo->GetLowerTitle() );

                if ( nComp == 0 )
                    nComp = pCollatorWrapper->compareString( aOne->GetTitle(), aTwo->GetTitle() );

                if ( nComp < 0 )
                    bRet = true;
                else if ( nComp > 0 )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_TYPE:
                nComp = pCollatorWrapper->compareString( aOne->maType, aTwo->maType );
                if ( nComp < 0 )
                    bRet = true;
                else if ( nComp > 0 )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_SIZE:
                if ( aOne->maSize < aTwo->maSize )
                    bRet = true;
                else if ( aOne->maSize > aTwo->maSize )
                    bRet = false;
                else
                    bEqual = true;
                break;
            case COLUMN_DATE:
                if ( aOne->maModDate < aTwo->maModDate )
                    bRet = true;
                else if ( aOne->maModDate > aTwo->maModDate )
                    bRet = false;
                else
                    bEqual = true;
                break;
            default:
                SAL_INFO( "svtools.contnr", "CompareSortingData_Impl: Compare unknown type!" );
                bRet = false;
        }
    }

    // when the two elements are equal, we must not return sal_True (which would
    // happen if we just return ! ( a < b ) when not sorting ascending )
    if ( bEqual )
        return false;

    return gbAscending ? bRet : !bRet;
}


void SvtFileView_Impl::SortFolderContent_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( maContent.size() > 1 )
    {
        gbAscending = mbAscending;
        gnColumn = mnSortColumn;
        pCollatorWrapper = aIntlWrapper.getCaseCollator();

        std::stable_sort( maContent.begin(), maContent.end(), CompareSortingData_Impl );

        pCollatorWrapper = nullptr;
    }
}


void SvtFileView_Impl::EntryRemoved( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    maContent.erase(std::find_if(maContent.begin(), maContent.end(),
                     [&](const std::unique_ptr<SortingData_Impl> & data) { return data->maTargetURL == rURL; }));
}


void SvtFileView_Impl::EntryRenamed( OUString& rURL,
                                     const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    auto aFoundElem = std::find_if(maContent.begin(), maContent.end(),
                     [&](const std::unique_ptr<SortingData_Impl> & data) { return data->maTargetURL == rURL; });
    if (aFoundElem != maContent.end())
    {
        (*aFoundElem)->SetNewTitle( rTitle );
        OUString aDisplayText = (*aFoundElem)->maDisplayText;
        sal_Int32 nIndex = aDisplayText.indexOf( '\t' );

        if ( nIndex > 0 )
            (*aFoundElem)->maDisplayText = aDisplayText.replaceAt( 0, nIndex, rTitle );

        INetURLObject aURLObj( rURL );
        aURLObj.SetName( rTitle, INetURLObject::EncodeMechanism::All );

        rURL = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        (*aFoundElem)->maTargetURL = rURL;
    }
}


OUString SvtFileView_Impl::FolderInserted( const OUString& rURL, const OUString& rTitle )
{
    ::osl::MutexGuard aGuard( maMutex );

    std::unique_ptr<SortingData_Impl> pData(new SortingData_Impl);

    pData->SetNewTitle( rTitle );
    pData->maSize        = 0;
    pData->mbIsFolder    = true;
    pData->maTargetURL   = rURL;

    ::svtools::VolumeInfo aVolInfo;
    pData->maType = SvFileInformationManager::GetFolderDescription( aVolInfo );
    pData->maImage = SvFileInformationManager::GetFolderImage( aVolInfo );

    OUString aValue;
    OUString const aTab( "\t" );
    OUString const aDateSep( ", " );

    // title, type, size, date
    aValue = pData->GetTitle();
    ReplaceTabWithString( aValue );
    aValue += aTab + pData->maType + aTab;
    // folders don't have a size
    aValue += aTab;
    // set the date
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    aValue += rLocaleData.getDate( pData->maModDate )
            + aDateSep
            + rLocaleData.getTime( pData->maModDate );

    pData->maDisplayText = aValue;
    maContent.push_back( std::move(pData) );

    return aValue;
}


sal_uLong SvtFileView_Impl::GetEntryPos( const OUString& rURL )
{
    ::osl::MutexGuard aGuard( maMutex );

    auto aFoundElem = std::find_if(maContent.begin(), maContent.end(),
          [&](const std::unique_ptr<SortingData_Impl> & data) { return data->maTargetURL == rURL; });
    return aFoundElem != maContent.end()?std::distance(maContent.begin(), aFoundElem):0;
}


void SvtFileView_Impl::SetViewMode( FileViewMode eMode )
{
    switch ( eMode )
    {
        case eDetailedList:
            mpCurView = mpView;
            mpView->Show();
            mpView->GetHeaderBar()->Show();
            mpIconView->Hide();
            break;

        case eIcon:
            mpCurView = mpIconView;
            mpView->Hide();
            mpView->GetHeaderBar()->Hide();
            mpIconView->Show();
            break;

        default:
            mpCurView = mpView;
            mpView->Show();
            mpView->GetHeaderBar()->Show();
            mpIconView->Hide();
    };
}


bool SvtFileView_Impl::SearchNextEntry( sal_uInt32& nIndex, const OUString& rTitle, bool bWrapAround )
{
    ::osl::MutexGuard aGuard( maMutex );

    sal_uInt32 nEnd = maContent.size();
    sal_uInt32 nStart = nIndex;
    while ( nIndex < nEnd )
    {
        SortingData_Impl* pData = maContent[ nIndex ].get();
        if ( pData->GetLowerTitle().startsWith( rTitle ) )
            return true;
        ++nIndex;
    }

    if ( bWrapAround )
    {
        nIndex = 0;
        while ( nIndex < nEnd && nIndex <= nStart )
        {
            SortingData_Impl* pData = maContent[ nIndex ].get();
            if ( pData->GetLowerTitle().startsWith( rTitle ) )
                return true;
            ++nIndex;
        }
    }

    return false;
}


namespace svtools {

QueryDeleteDlg_Impl::QueryDeleteDlg_Impl(weld::Widget* pParent, const OUString& rName)
    : MessageDialogController(pParent, "svt/ui/querydeletedialog.ui", "QueryDeleteDialog")
    , m_xAllButton(m_xBuilder->weld_button("all"))
{
    // display specified texts
    m_xDialog->set_secondary_text(m_xDialog->get_secondary_text().replaceFirst("%s", rName));
}

QueryDeleteDlg_Impl::~QueryDeleteDlg_Impl()
{
}

}

namespace {
    void SAL_CALL CallbackTimer::onShot()
    {
        OSL_ENSURE( m_pTimeoutHandler, "CallbackTimer::onShot: nobody interested in?" );
        SvtFileView_Impl* pHandler( m_pTimeoutHandler );
        if ( pHandler )
            pHandler->onTimeout();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
