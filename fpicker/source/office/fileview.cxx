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
#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <vcl/treelistentry.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/querydelete.hxx>
#include <vcl/event.hxx>
#include <vcl/headbar.hxx>
#include <vcl/svtabbx.hxx>
#include <svtools/strings.hrc>
#include <bitmaps.hlst>
#include <toolkit/helper/vclunohelper.hxx>
#include "contentenumeration.hxx"
#include <vcl/AccessibleBrowseBoxObjType.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <algorithm>
#include <vector>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <rtl/math.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <salhelper/timer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dialog.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svtools/urlfilter.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include "fileview.hxx"
#include "iconview.hxx"

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

class ViewTabListBox_Impl
{
private:
    Reference< XCommandEnvironment >    mxCmdEnv;
    std::unique_ptr<weld::TreeView> mxTreeView;
    std::unique_ptr<VclBuilder> mxBuilder;
    VclPtr<PopupMenu> mxMenu;

    ::osl::Mutex            maMutex;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    OUString const          msAccessibleDescText;
    OUString const          msFolder;
    OUString const          msFile;
    sal_uInt32              mnSearchIndex;
    bool                    mbEnableDelete          : 1;
    bool const              mbShowHeader;

    void            DeleteEntries();
    void            DoQuickSearch( sal_Unicode rChar );
    bool            Kill( const OUString& rURL );

public:
    ViewTabListBox_Impl(std::unique_ptr<weld::TreeView> xTreeView, weld::Window* pTopLevel, SvtFileView_Impl* pParent, FileViewFlags nFlags);
    virtual ~ViewTabListBox_Impl();

    std::unique_ptr<weld::TreeIter> make_iterator() const { return mxTreeView->make_iterator(); }
    void insert(const OUString &rEntry, const OUString& rId, const OUString& rImage, weld::TreeIter& rIter)
    {
        mxTreeView->insert(nullptr, -1, &rEntry, &rId, &rImage, nullptr, nullptr, false, &rIter);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        mxTreeView->append(rId, rStr, rImage);
    }

    void scroll_to_row(const weld::TreeIter& rIter) { mxTreeView->scroll_to_row(rIter); }
    void set_cursor(int nPos) { mxTreeView->set_cursor(nPos); }
    bool get_cursor(weld::TreeIter* pIter) const { return mxTreeView->get_cursor(pIter); }

    OUString get_id(const weld::TreeIter& rIter) { return mxTreeView->get_id(rIter); }

    void connect_row_activated(const Link<weld::TreeView&, bool>& rLink) { mxTreeView->connect_row_activated(rLink); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { mxTreeView->connect_changed(rLink); }

    int n_children() const { return mxTreeView->n_children(); }

    void freeze() { mxTreeView->freeze(); }
    void thaw() { mxTreeView->thaw(); }

    void show() { mxTreeView->show(); }
    void hide() { mxTreeView->hide(); }
    bool get_visible() const { return mxTreeView->get_visible(); }

    void grab_focus() { mxTreeView->grab_focus(); }
    bool has_focus() const { return mxTreeView->has_focus(); }

    void set_help_id(const OString& rHelpId) { mxTreeView->set_help_id(rHelpId); }
    OString get_help_id() const { return mxTreeView->get_help_id(); }

//TODO    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
//TODO    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    void            ClearAll();

    void            EnableDelete( bool bEnable ) { mbEnableDelete = bEnable; }

    const Reference< XCommandEnvironment >& GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK(ResetQuickSearch_Impl, Timer *, void);

//TODO    virtual VclPtr<PopupMenu> CreateContextMenu() override;
//TODO    virtual void        ExecuteContextMenuAction( sal_uInt16 nSelectedPopentry ) override;
};


//= SvtFileView_Impl


class SvtFileView_Impl  :public ::svt::IEnumerationResultHandler
{
protected:
    SvtFileView*                        m_pAntiImpl;
    Link<weld::TreeView&,void>          m_aSelectHandler;

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

    weld::Window*           m_pTopLevel;
    VclPtr<SvTreeListBox>               mpCurView;
    std::unique_ptr<ViewTabListBox_Impl> mxView;
    VclPtr<IconView>                    mxIconView;
    sal_uInt16              mnSortColumn;
    bool                    mbAscending     : 1;
    bool const              mbOnlyFolder    : 1;
    sal_Int16               mnSuspendSelectCallback : 1;
    bool                    mbIsFirstResort : 1;

    IntlWrapper const       aIntlWrapper;

    OUString                maViewURL;
    OUString                maCurrentFilter;
    OUString                maFolderImage;
    Link<SvtFileView*,void> maOpenDoneLink;
    Reference< XCommandEnvironment >    mxCmdEnv;

    SvtFileView_Impl(SvtFileView* pAntiImpl, weld::Window* pTopLevel,
                     std::unique_ptr<weld::TreeView> xTreeView,
                     Reference < XCommandEnvironment > const & xEnv,
                     FileViewFlags nFlags,
                     bool bOnlyFolder);

    virtual ~SvtFileView_Impl();

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

    int                     GetEntryPos( const OUString& rURL );

    void                    SetViewMode( FileViewMode eMode );

    inline void             EnableDelete( bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, bool bAscending );
    bool                    SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             bool bWrapAround );

    void                    SetSelectHandler( const Link<weld::TreeView&,void>& rHdl );

    void                    InitSelection();
    void                    ResetCursor();

    inline void             EndEditing();

    void                    onTimeout();

    void                    grab_focus()
    {
        if (mxView->get_visible())
            mxView->grab_focus();
        else
            mxIconView->GrabFocus();
    }

    bool                    has_focus() const
    {
        return mxView->has_focus() || mxIconView->HasFocus();
    }

protected:
    DECL_LINK(SelectionMultiplexer, weld::TreeView&, void);

    // IEnumerationResultHandler overridables
    virtual void        enumerationDone( ::svt::EnumerationResult eResult ) override;
            void        implEnumerationSuccess();
};

inline void SvtFileView_Impl::EnableDelete( bool bEnable )
{
    mxView->EnableDelete( bEnable );
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

ViewTabListBox_Impl::ViewTabListBox_Impl(std::unique_ptr<weld::TreeView> xTreeView,
                                         weld::Window* pTopLevel,
                                         SvtFileView_Impl* pParent,
                                         FileViewFlags nFlags)
    : mxTreeView(std::move(xTreeView))
    , mpParent( pParent )
    , msAccessibleDescText( SvtResId(STR_SVT_ACC_DESC_FILEVIEW) )
    , msFolder( SvtResId(STR_SVT_ACC_DESC_FOLDER) )
    , msFile( SvtResId(STR_SVT_ACC_DESC_FILE) )
    , mnSearchIndex( 0 )
    , mbEnableDelete( false )
    , mbShowHeader( !(nFlags & FileViewFlags::SHOW_NONE) )
{
#if 0
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
#endif

    if (nFlags & FileViewFlags::MULTISELECTION)
        mxTreeView->set_selection_mode(SelectionMode::Multiple);

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetInvokeHandler( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, pTopLevel->GetXWindow()), UNO_QUERY_THROW);

    mxCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

//TODO    EnableContextMenuHandling();
}

ViewTabListBox_Impl::~ViewTabListBox_Impl()
{
}

IMPL_LINK_NOARG(ViewTabListBox_Impl, ResetQuickSearch_Impl, Timer *, void)
{
    ::osl::MutexGuard aGuard( maMutex );

    maQuickSearchText.clear();
    mnSearchIndex = 0;
}

#if 0
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
#endif

#if 0
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
#endif

#if 0
void ViewTabListBox_Impl::ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry )
{
    if (nSelectedPopupEntry == mxMenu->GetItemId("delete"))
        DeleteEntries();
    else if (nSelectedPopupEntry == mxMenu->GetItemId("rename"))
        EditEntry( FirstSelected() );
}
#endif

void ViewTabListBox_Impl::ClearAll()
{
    for (int i = 0, nCount = mxTreeView->n_children(); i < nCount; ++i)
        delete reinterpret_cast<SvtContentEntry*>(mxTreeView->get_id(i).toInt64());
    mxTreeView->clear();
}

void ViewTabListBox_Impl::DeleteEntries()
{
    short eResult = svtools::QUERYDELETE_YES;

    mxTreeView->selected_foreach([this, &eResult](weld::TreeIter& rCurEntry){
        OUString aURL;
        if (!mxTreeView->get_id(rCurEntry).isEmpty())
            aURL = reinterpret_cast<SvtContentEntry*>(mxTreeView->get_id(rCurEntry).toInt64())->maURL;
        if (aURL.isEmpty())
        {
            mxTreeView->unselect(rCurEntry);
            return false;
        }

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
        {
            mxTreeView->unselect(rCurEntry);
            return false; // process next entry
        }

        if ( eResult != svtools::QUERYDELETE_ALL )
        {
            INetURLObject aObj( aURL );
            svtools::QueryDeleteDlg_Impl aDlg(
                mxTreeView.get(), aObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset));

            if (mxTreeView->count_selected_rows() > 1)
                aDlg.EnableAllButton();

            eResult = aDlg.run();
        }

        if ( ( eResult == svtools::QUERYDELETE_ALL ) ||
             ( eResult == svtools::QUERYDELETE_YES ) )
        {
            if ( Kill( aURL ) )
            {
                delete reinterpret_cast<SvtContentEntry*>(mxTreeView->get_id(rCurEntry).toInt64());
                mpParent->EntryRemoved( aURL );
            }
        }

        return false;
    });

    mxTreeView->remove_selection();
}

#if 0
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
#endif

#if 0
void ViewTabListBox_Impl::DoQuickSearch( sal_Unicode rChar )
{
    ::osl::MutexGuard aGuard( maMutex );

    maResetQuickSearch.Stop();

    OUString    aLastText = maQuickSearchText;
    sal_uInt32  aLastPos = mnSearchIndex;

    maQuickSearchText += OUString(rChar).toAsciiLowerCase();

    bool bFound = mpParent->SearchNextEntry( mnSearchIndex, maQuickSearchText, false );

    if ( !bFound && ( aLastText.getLength() == 1 ) &&
         ( aLastText == OUStringChar(rChar) ) )
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
            mxTreeView->unselect_all();
            Select( pEntry );
            SetCurEntry( pEntry );
            MakeVisible( pEntry );
        }
    }

    maResetQuickSearch.Start();
}
#endif

#if 0
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
#endif

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

SvtFileView::SvtFileView( weld::Window* pTopLevel, std::unique_ptr<weld::TreeView> xTreeView,
                          bool bOnlyFolder, bool bMultiSelection, bool bShowType )
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
        InteractionHandler::createWithParent(xContext, pTopLevel->GetXWindow()), UNO_QUERY_THROW);
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mpImpl.reset(new SvtFileView_Impl(this, pTopLevel, std::move(xTreeView), xCmdEnv, nFlags, bOnlyFolder));
//TODO    mpImpl->mxView->ForbidEmptyText();

#if 0
    HeaderBar* pHeaderBar = mpImpl->mxView->GetHeaderBar();
    pHeaderBar->SetSelectHdl( LINK( this, SvtFileView, HeaderSelect_Impl ) );
    pHeaderBar->SetEndDragHdl( LINK( this, SvtFileView, HeaderEndDrag_Impl ) );
#endif
}

void SvtFileView::grab_focus()
{
    mpImpl->grab_focus();
}

bool SvtFileView::has_focus() const
{
    return mpImpl->has_focus();
}

SvtFileView::~SvtFileView()
{
}

#if 0
Size SvtFileView::GetOptimalSize() const
{
    return LogicToPixel(Size(208, 50), MapMode(MapUnit::MapAppFont));
}
#endif

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

    std::unique_ptr<weld::TreeIter> xEntry = mpImpl->mxView->make_iterator();
    SvtContentEntry* pUserData = new SvtContentEntry( rUrl, true );
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
    mpImpl->mxView->insert(sEntry, sId, mpImpl->maFolderImage, *xEntry);
    mpImpl->mxView->scroll_to_row(*xEntry);

#if 0
    SvTreeListEntry* pEntry2 = mpImpl->mxIconView->InsertEntry( sEntry.getToken( 0, '\t' ), mpImpl->maFolderImage, mpImpl->maFolderImage );
    SvtContentEntry* pUserData2 = new SvtContentEntry( rUrl, true );
    pEntry2->SetUserData( pUserData2 );
    mpImpl->mxIconView->MakeVisible( pEntry2 );
#endif
}

FileViewResult SvtFileView::PreviousLevel( const FileViewAsyncAction* pAsyncDescriptor )
{
    FileViewResult eResult = eFailure;

    OUString sParentURL;
    if ( GetParentURL( sParentURL ) )
        eResult = Initialize( sParentURL, mpImpl->maCurrentFilter, pAsyncDescriptor, maBlackList );

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

OString SvtFileView::get_help_id() const
{
    return mpImpl->mxView->get_help_id();
}

void SvtFileView::set_help_id(const OString& rHelpId)
{
    mpImpl->mxView->set_help_id(rHelpId);
}

FileViewResult SvtFileView::Initialize(
    const OUString& rURL,
    const OUString& rFilter,
    const FileViewAsyncAction* pAsyncDescriptor,
    const css::uno::Sequence< OUString >& rBlackList )
{
    weld::WaitObject aWaitCursor(mpImpl->m_pTopLevel);
    maBlackList = rBlackList;

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
    FileViewResult eResult = mpImpl->GetFolderContent_Impl(mpImpl->maViewURL, pAsyncDescriptor, maBlackList);
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

#if 0
void SvtFileView::GetFocus()
{
    Control::GetFocus();
    if ( mpImpl && mpImpl->mpCurView )
        mpImpl->mpCurView->GrabFocus();
}
#endif

void SvtFileView::SetSelectHdl(const Link<weld::TreeView&,void>& rHdl)
{
    mpImpl->SetSelectHandler(rHdl);
}

void SvtFileView::SetDoubleClickHdl(const Link<weld::TreeView&,bool>& rHdl)
{
    mpImpl->mxView->connect_row_activated( rHdl );
//TODO    mpImpl->mxIconView->SetDoubleClickHdl( rHdl );
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

#if 0
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
#endif

#if 0
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
        mpImpl->mxView->SetTab( i, aSize.Width(), MapUnit::MapPixel );
    }
}
#endif

OUString SvtFileView::GetConfigString() const
{
    OUString sRet;
#if 0
    HeaderBar* pBar = mpImpl->mxView->GetHeaderBar();
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
#endif
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

void SvtFileView::SetConfigString( const OUString& /*rCfgStr*/ )
{
#if 0
    HeaderBar* pBar = mpImpl->mxView->GetHeaderBar();
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
#endif
}

#if 0
void SvtFileView::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::Enable )
        Invalidate();
    Control::StateChanged( nStateChange );
}
#endif

// class SvtFileView_Impl
SvtFileView_Impl::SvtFileView_Impl(SvtFileView* pAntiImpl, weld::Window* pTopLevel,
                                   std::unique_ptr<weld::TreeView> xTreeView,
                                   Reference < XCommandEnvironment > const & xEnv,
                                   FileViewFlags nFlags, bool bOnlyFolder)
    : m_pAntiImpl                ( pAntiImpl )
    , m_eAsyncActionResult       ( ::svt::EnumerationResult::ERROR )
    , m_bRunningAsyncAction      ( false )
    , m_bAsyncActionCancelled    ( false )
    , m_pTopLevel                ( pTopLevel )
    , mxView(new ViewTabListBox_Impl(std::move(xTreeView), pTopLevel, this, nFlags))
    , mnSortColumn               ( COLUMN_TITLE )
    , mbAscending                ( true )
    , mbOnlyFolder               ( bOnlyFolder )
    , mnSuspendSelectCallback    ( 0 )
    , mbIsFirstResort            ( true )
    , aIntlWrapper               ( Application::GetSettings().GetLanguageTag() )
    , maFolderImage              (RID_BMP_FOLDER)
    , mxCmdEnv ( xEnv )
{
//TODO    mpCurView = mxView;
//TODO    mxIconView = VclPtr<IconView>::Create( m_pAntiImpl, WB_TABSTOP );
    mxIconView->Hide();
//TODO    mxView->EnableCellFocus();
}

SvtFileView_Impl::~SvtFileView_Impl()
{
    Clear();
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
        mxView->GetCommandEnvironment(), maContent, maMutex));
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

        // now wait. Note that if we didn't get a pAsyncDescriptor, then this is an infinite wait.
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
        mxView->ClearAll();
        mxIconView->ClearAll();
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

IMPL_LINK(SvtFileView_Impl, SelectionMultiplexer, weld::TreeView&, rSource, void)
{
    if (!mnSuspendSelectCallback)
        m_aSelectHandler.Call(rSource);
}

void SvtFileView_Impl::SetSelectHandler(const Link<weld::TreeView&,void>& rHdl)
{
    m_aSelectHandler = rHdl;

    Link<weld::TreeView&,void> aMasterHandler;
    if (m_aSelectHandler.IsSet())
        aMasterHandler = LINK(this, SvtFileView_Impl, SelectionMultiplexer);

    mxView->connect_changed( aMasterHandler );
//TODO    mxIconView->SetSelectHdl( aMasterHandler );
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

    mxView->freeze();
//TODO    mxIconView->SetUpdateMode( false );
    mxView->ClearAll();
    mxIconView->ClearAll();

    for (auto const& elem : maContent)
    {
        if ( mbOnlyFolder && ! elem->mbIsFolder )
            continue;

        // insert entry and set user data
        SvtContentEntry* pUserData = new SvtContentEntry( elem->maTargetURL,
                                                          elem->mbIsFolder );
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
        mxView->append(sId, elem->maDisplayText, elem->maImage);

//TODO        SvTreeListEntry* pEntry2 = mxIconView->InsertEntry( elem->maDisplayText.getToken( 0, '\t' ),
//TODO                                                   elem->maImage, elem->maImage );

//        SvtContentEntry* pUserData2 = new SvtContentEntry( elem->maTargetURL,
//                                                          elem->mbIsFolder );

//        pEntry2->SetUserData( pUserData2 );
    }

    InitSelection();

    ++mnSuspendSelectCallback;
    mxView->thaw();
//TODO    mxIconView->SetUpdateMode( true );
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
    maOpenDoneLink.Call( m_pAntiImpl );
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
            elem->maImage = SvFileInformationManager::GetFolderImageId(aVolInfo);
        }
        else
            elem->maImage = SvFileInformationManager::GetFileImageId(INetURLObject(elem->maTargetURL));
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
    mxView->ResetQuickSearch_Impl( nullptr );

    std::unique_ptr<weld::TreeIter> xEntry(mxView->make_iterator());
    bool bEntry = mxView->get_cursor(xEntry.get());

    OUString aEntryURL;
    if (bEntry && !mxView->get_id(*xEntry).isEmpty())
        aEntryURL = reinterpret_cast<SvtContentEntry*>(mxView->get_id(*xEntry).toInt64())->maURL;

    mnSortColumn = nColumn;
    mbAscending = bAscending;

    SortFolderContent_Impl();
    OpenFolder_Impl();

    if ( !mbIsFirstResort )
    {
        int nPos = GetEntryPos( aEntryURL );
        if (nPos != -1 && nPos < mxView->n_children())
        {
            ++mnSuspendSelectCallback;  // #i15668#
            mxView->set_cursor(nPos);
            --mnSuspendSelectCallback;
        }
    }
    else
        mbIsFirstResort = false;
}

static bool                     gbAscending = true;
static sal_Int16                gnColumn = COLUMN_TITLE;
static const CollatorWrapper*   pCollatorWrapper = nullptr;

/* this function returns true, if aOne is less than aTwo
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
        aURLObj.setName( rTitle, INetURLObject::EncodeMechanism::All );

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
    pData->maImage = SvFileInformationManager::GetFolderImageId( aVolInfo );

    OUString aValue;
    OUString const aTab( "\t" );
    OUString const aDateSep( ", " );

    // title, type, size, date
    aValue = pData->GetTitle();
    ReplaceTabWithString( aValue );
    aValue += aTab + pData->maType + aTab +
        // folders don't have a size
        aTab;
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

int SvtFileView_Impl::GetEntryPos(const OUString& rURL)
{
    ::osl::MutexGuard aGuard( maMutex );

    auto aFoundElem = std::find_if(maContent.begin(), maContent.end(),
          [&](const std::unique_ptr<SortingData_Impl> & data) { return data->maTargetURL == rURL; });
    return aFoundElem != maContent.end() ? std::distance(maContent.begin(), aFoundElem) : -1;
}

void SvtFileView_Impl::SetViewMode( FileViewMode eMode )
{
    switch ( eMode )
    {
        case eDetailedList:
//TODO            mpCurView = mxView;
            mxView->show();
            mxIconView->Hide();
            break;

        case eIcon:
            mpCurView = mxIconView;
            mxView->hide();
            mxIconView->Show();
            break;

        default:
//TODO            mpCurView = mxView;
            mxView->show();
            mxIconView->Hide();
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
