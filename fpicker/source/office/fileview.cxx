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

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <svtools/svtresid.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/querydelete.hxx>
#include <svtools/strings.hrc>
#include <bitmaps.hlst>
#include <toolkit/helper/vclunohelper.hxx>
#include "contentenumeration.hxx"
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
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
#include <o3tl/typed_flags_set.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <salhelper/timer.hxx>
#include <svtools/urlfilter.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/svapp.hxx>
#include <vcl/AccessibleBrowseBoxObjType.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/headbar.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/settings.hxx>
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
    MULTISELECTION     = 0x02,
    SHOW_TYPE          = 0x04,
    SHOW_NONE          = 0x20,
};
namespace o3tl
{
    template<> struct typed_flags<FileViewFlags> : is_typed_flags<FileViewFlags, 0x26> {};
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
    std::unique_ptr<weld::TreeIter> mxScratchIter;

    ::osl::Mutex            maMutex;
    SvtFileView_Impl*       mpParent;
    Timer                   maResetQuickSearch;
    OUString                maQuickSearchText;
    OUString const          msAccessibleDescText;
    sal_uInt32              mnSearchIndex;
    bool                    mbEnableDelete;
    bool                    mbEditing;
    bool const              mbShowHeader;
    bool const              mbShowType;

    void            DeleteEntries();
    void            DoQuickSearch( sal_Unicode rChar );
    bool            Kill( const OUString& rURL );

public:
    ViewTabListBox_Impl(std::unique_ptr<weld::TreeView> xTreeView, weld::Window* pTopLevel, SvtFileView_Impl* pParent, FileViewFlags nFlags);
    virtual ~ViewTabListBox_Impl();

    std::unique_ptr<weld::TreeIter> make_iterator() const { return mxTreeView->make_iterator(); }
    void insert(const OUString &rEntry, const OUString& rId, const OUString& rImage, weld::TreeIter& rIter)
    {
        mxTreeView->insert(nullptr, -1, &rEntry, &rId, nullptr, nullptr, &rImage, false, &rIter);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rSize, const OUString& rDate, const OUString& rImage)
    {
        mxTreeView->insert(nullptr, -1, &rStr, &rId, nullptr, nullptr, &rImage, false, mxScratchIter.get());
        mxTreeView->set_text(*mxScratchIter, rSize, 1);
        mxTreeView->set_text(*mxScratchIter, rDate, 2);
    }

    void scroll_to_row(const weld::TreeIter& rIter) { mxTreeView->scroll_to_row(rIter); }
    void set_cursor(int nPos) { mxTreeView->set_cursor(nPos); }
    void set_cursor(const weld::TreeIter& rIter) { mxTreeView->set_cursor(rIter); }
    bool get_cursor(weld::TreeIter* pIter) const { return mxTreeView->get_cursor(pIter); }
    bool get_iter_first(weld::TreeIter& rIter) const { return mxTreeView->get_iter_first(rIter); }
    bool get_selected(weld::TreeIter* pIter) const { return mxTreeView->get_selected(pIter); }

    void unselect_all() { mxTreeView->unselect_all(); }

    OUString get_id(const weld::TreeIter& rIter) { return mxTreeView->get_id(rIter); }

    void connect_row_activated(const Link<weld::TreeView&, bool>& rLink) { mxTreeView->connect_row_activated(rLink); }
    void connect_changed(const Link<weld::TreeView&, void>& rLink) { mxTreeView->connect_changed(rLink); }

    int n_children() const { return mxTreeView->n_children(); }

    void freeze() { mxTreeView->freeze(); }
    void thaw() { mxTreeView->thaw(); }

    void show() { mxTreeView->show(); }
    void hide() { mxTreeView->hide(); }
    bool get_visible() const { return mxTreeView->get_visible(); }

    int count_selected_rows() const { return mxTreeView->count_selected_rows(); }

    void grab_focus() { mxTreeView->grab_focus(); }
    bool has_focus() const { return mxTreeView->has_focus(); }

    void set_help_id(const OString& rHelpId) { mxTreeView->set_help_id(rHelpId); }
    OString get_help_id() const { return mxTreeView->get_help_id(); }

    bool IsEditingActive() const { return mbEditing; }
    void end_editing() { mxTreeView->end_editing(); mbEditing = false; }

    void selected_foreach(const std::function<bool(weld::TreeIter&)>& func)
    {
        mxTreeView->selected_foreach(func);
    }

    weld::TreeView* getWidget() const
    {
        return mxTreeView.get();
    }

//TODO    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

    void            ClearAll();

    void            EnableDelete( bool bEnable ) { mbEnableDelete = bEnable; }
    bool            TypeColumnVisible() const { return mbShowType; }

    const Reference< XCommandEnvironment >& GetCommandEnvironment() const { return mxCmdEnv; }

    DECL_LINK(ResetQuickSearch_Impl, Timer *, void);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(EditingEntryHdl, const weld::TreeIter&, bool);
    typedef std::pair<const weld::TreeIter&, OUString> IterString;
    DECL_LINK(EditedEntryHdl, const IterString&, bool);

    void            ExecuteContextMenuAction(const OString& rSelectedPopentry);
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
    static OUString         ReplaceTabWithString(const OUString& rValue);
    void                    CreateDisplayText_Impl();
    void                    SortFolderContent_Impl();

    void                    EntryRemoved( const OUString& rURL );
    void                    EntryRenamed( OUString& rURL,
                                          const OUString& rName );
    const SortingData_Impl& FolderInserted( const OUString& rURL,
                                            const OUString& rTitle );

    int                     GetEntryPos( const OUString& rURL );

    void                    SetViewMode( FileViewMode eMode );

    inline void             EnableDelete( bool bEnable );

    void                    Resort_Impl( sal_Int16 nColumn, bool bAscending );
    bool                    SearchNextEntry( sal_uInt32 &nIndex,
                                             const OUString& rTitle,
                                             bool bWrapAround );

    void                    SetSelectHandler( const Link<weld::TreeView&,void>& rHdl );

    void                    ResetCursor();

    void EndEditing()
    {
        if (mxView->IsEditingActive())
            mxView->end_editing();
    }

    void onTimeout();

    void grab_focus()
    {
        if (mxView->get_visible())
            mxView->grab_focus();
        else
            mxIconView->GrabFocus();
    }

    bool has_focus() const
    {
        return mxView->has_focus() || mxIconView->HasFocus();
    }

    int GetSortColumn() const
    {
        sal_uInt16 nOldSortID = mnSortColumn;
        // skip "TYPE"
        if (!mxView->TypeColumnVisible() && nOldSortID != COLUMN_TITLE)
            --nOldSortID;
        return nOldSortID - 1;
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
    , mxScratchIter(mxTreeView->make_iterator())
    , mpParent( pParent )
    , mnSearchIndex( 0 )
    , mbEnableDelete( false )
    , mbEditing( false )
    , mbShowHeader( !(nFlags & FileViewFlags::SHOW_NONE) )
    , mbShowType(nFlags & FileViewFlags::SHOW_TYPE)
{
    Size aSize(mxTreeView->get_approximate_digit_width() * 50,
               mxTreeView->get_height_rows(7));

    mxTreeView->set_size_request(aSize.Width(), aSize.Height());
//TODO icons    mxTreeView->set_size_request(aSize.Width(), aSize.Height());

#if 0
    if (nFlags & FileViewFlags::SHOW_TYPE)
    {
        mpHeaderBar->InsertItem(COLUMN_TYPE, SvtResId(STR_SVT_FILEVIEW_COLUMN_TYPE), 140, nBits);
    }
#endif

    std::vector<int> aWidths;
    aWidths.push_back(180);
    aWidths.push_back(80);
    aWidths.push_back(100);
    mxTreeView->set_column_fixed_widths(aWidths);

    if (nFlags & FileViewFlags::MULTISELECTION)
        mxTreeView->set_selection_mode(SelectionMode::Multiple);

    maResetQuickSearch.SetTimeout( QUICK_SEARCH_TIMEOUT );
    maResetQuickSearch.SetInvokeHandler( LINK( this, ViewTabListBox_Impl, ResetQuickSearch_Impl ) );

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, pTopLevel->GetXWindow()), UNO_QUERY_THROW);

    mxCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mxTreeView->connect_popup_menu(LINK(this, ViewTabListBox_Impl, CommandHdl));
    mxTreeView->connect_editing_started(LINK( this, ViewTabListBox_Impl, EditingEntryHdl));
    mxTreeView->connect_editing_done(LINK( this, ViewTabListBox_Impl, EditedEntryHdl));
}

IMPL_LINK_NOARG(ViewTabListBox_Impl, EditingEntryHdl, const weld::TreeIter&, bool)
{
    return mbEditing;
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

IMPL_LINK(ViewTabListBox_Impl, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    bool bEnableDelete = mbEnableDelete;
    bool bEnableRename = true;

    int nCount = 0;
    mxTreeView->selected_foreach([this, &nCount, &bEnableDelete, &bEnableRename](weld::TreeIter& rEntry){
        ++nCount;

        ::ucbhelper::Content aCnt;
        try
        {
            OUString aURL(reinterpret_cast<SvtContentEntry*>(
                mxTreeView->get_id(rEntry).toInt64())->maURL);
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

        bool bStop = !bEnableDelete && !bEnableRename;
        return bStop;
    });

    if (nCount == 0)
        bEnableDelete = false;
    if (nCount != 1)
        bEnableRename = false;

    if (bEnableDelete || bEnableRename)
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxTreeView.get(), "svt/ui/fileviewmenu.ui"));
        auto xContextMenu = xBuilder->weld_menu("menu");
        xContextMenu->set_visible("delete", bEnableDelete);
        xContextMenu->set_visible("rename", bEnableRename);
        OString sCommand(xContextMenu->popup_at_rect(mxTreeView.get(), tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
        ExecuteContextMenuAction(sCommand);
    }

    return true;
}

void ViewTabListBox_Impl::ExecuteContextMenuAction(const OString& rSelectedPopupEntry)
{
    if (rSelectedPopupEntry == "delete")
        DeleteEntries();
    else if (rSelectedPopupEntry == "rename")
    {
        std::unique_ptr<weld::TreeIter> xEntry = mxTreeView->make_iterator();
        if (mxTreeView->get_selected(xEntry.get()))
        {
            mbEditing = true;
            mxTreeView->start_editing(*xEntry);
        }
    }
}

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

IMPL_LINK(ViewTabListBox_Impl, EditedEntryHdl, const IterString&, rIterString, bool)
{
    mbEditing = false;

    const weld::TreeIter& rEntry = rIterString.first;
    OUString sNewText = rIterString.second;

    if (sNewText.isEmpty())
        return false;

    bool bRet = false;

    OUString aURL;
    SvtContentEntry* pData = reinterpret_cast<SvtContentEntry*>(mxTreeView->get_id(rEntry).toInt64());

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
            aValue <<= sNewText;
            aContent.setPropertyValue( aPropName, aValue );
            mpParent->EntryRenamed(aURL, sNewText);

            if (pData)
                pData->maURL = aURL;

            mxTreeView->set_id(rEntry, OUString::number(reinterpret_cast<sal_Int64>(pData)));

            bRet = true;
        }
    }
    catch( Exception const & )
    {
    }

    return bRet;
}

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
    if ( bMultiSelection )
        nFlags |= FileViewFlags::MULTISELECTION;
    if ( bShowType )
        nFlags |= FileViewFlags::SHOW_TYPE;

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
        InteractionHandler::createWithParent(xContext, pTopLevel->GetXWindow()), UNO_QUERY_THROW);
    Reference < XCommandEnvironment > xCmdEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    mpImpl.reset(new SvtFileView_Impl(this, pTopLevel, std::move(xTreeView), xCmdEnv, nFlags, bOnlyFolder));

    weld::TreeView* pView = mpImpl->mxView->getWidget();
    pView->connect_column_clicked(LINK(this, SvtFileView, HeaderSelect_Impl));
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

void SvtFileView::SetViewMode( FileViewMode eMode )
{
    mpImpl->SetViewMode( eMode );
}

OUString SvtFileView::GetURL(const weld::TreeIter& rEntry) const
{
    //TODO the icon
    SvtContentEntry* pEntry = reinterpret_cast<SvtContentEntry*>(mpImpl->mxView->get_id(rEntry).toInt64());
    if (pEntry)
        return pEntry->maURL;
    return OUString();
}

OUString SvtFileView::GetCurrentURL() const
{
    //TODO the icon
    OUString aURL;
    std::unique_ptr<weld::TreeIter> xEntry = mpImpl->mxView->make_iterator();
    if (mpImpl->mxView->get_selected(xEntry.get()))
    {
        SvtContentEntry* pEntry = reinterpret_cast<SvtContentEntry*>(mpImpl->mxView->get_id(*xEntry).toInt64());
        if (pEntry)
            aURL = pEntry->maURL;
    }
    return aURL;
}

void SvtFileView::CreatedFolder( const OUString& rUrl, const OUString& rNewFolder )
{
    const SortingData_Impl& rEntry = mpImpl->FolderInserted( rUrl, rNewFolder );

    std::unique_ptr<weld::TreeIter> xEntry = mpImpl->mxView->make_iterator();
    SvtContentEntry* pUserData = new SvtContentEntry( rUrl, true );
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
    mpImpl->mxView->insert(rEntry.maDisplayName, sId, mpImpl->maFolderImage, *xEntry);
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
    mpImpl->mxView->unselect_all();
//TODO    mpImpl->mxIconView->SelectAll( false );
}

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
    //TODO icons
    if (mpImpl->mxView->get_visible())
        return mpImpl->mxView->count_selected_rows();
    return mpImpl->mpCurView->GetSelectionCount();
}

SvtContentEntry* SvtFileView::FirstSelected() const
{
    if (mpImpl->mxView->get_visible())
    {
        SvtContentEntry* pRet = nullptr;
        std::unique_ptr<weld::TreeIter> xEntry = mpImpl->mxView->make_iterator();
        if (mpImpl->mxView->get_selected(xEntry.get()))
            pRet = reinterpret_cast<SvtContentEntry*>(mpImpl->mxView->get_id(*xEntry).toInt64());
        return pRet;
    }
    else
    {
//        return mpImpl->mxIconView->FirstSelected();
        return nullptr;
    }
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

IMPL_LINK(SvtFileView, HeaderSelect_Impl, int, nColumn, void)
{
    sal_uInt16 nItemID = nColumn + 1;
    // skip "TYPE"
    if (!mpImpl->mxView->TypeColumnVisible() && nItemID != COLUMN_TITLE)
        ++nItemID;

    weld::TreeView* pView = mpImpl->mxView->getWidget();
    bool bSortAtoZ = mpImpl->mbAscending;

    //set new arrow positions in headerbar
    if (nItemID != mpImpl->mnSortColumn)
    {
        // remove old indicator, new will be created in OpenFolder_Impl
        pView->set_sort_indicator(TRISTATE_INDET, mpImpl->GetSortColumn());
    }
    else
        bSortAtoZ = !bSortAtoZ;

    mpImpl->Resort_Impl(nItemID, bSortAtoZ);
}

OUString SvtFileView::GetConfigString() const
{
    // sort order
    OUString sRet = OUString::number( mpImpl->mnSortColumn ) + ";";

    weld::TreeView* pView = mpImpl->mxView->getWidget();
    bool bUp = pView->get_sort_indicator(mpImpl->GetSortColumn()) == TRISTATE_TRUE;
    sRet += bUp ? OUString("1") : OUString("0");
    sRet += ";";

    sal_uInt16 nCount = mpImpl->mxView->TypeColumnVisible() ? 4 : 3;
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        sal_uInt16 nId = i + 1;
        // skip "TYPE"
        if (!mpImpl->mxView->TypeColumnVisible() && nId != COLUMN_TITLE)
            ++nId;

        sRet += OUString::number( nId )
                + ";"
                + OUString::number(pView->get_column_width(i))
                + ";";
    }

    return comphelper::string::stripEnd(sRet, ';');
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
#endif
}

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
//TODO    mxIconView->Hide();
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
//        mxIconView->ClearAll();
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

void SvtFileView_Impl::OpenFolder_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    mxView->freeze();
//TODO    mxIconView->SetUpdateMode( false );
    mxView->ClearAll();
//    mxIconView->ClearAll();

    for (auto const& elem : maContent)
    {
        if ( mbOnlyFolder && ! elem->mbIsFolder )
            continue;

        // insert entry and set user data
        SvtContentEntry* pUserData = new SvtContentEntry( elem->maTargetURL,
                                                          elem->mbIsFolder );
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pUserData)));
        mxView->append(sId, elem->maDisplayName, elem->maDisplaySize, elem->maDisplayDate, elem->maImage);

//TODO        SvTreeListEntry* pEntry2 = mxIconView->InsertEntry( elem->maDisplayText.getToken( 0, '\t' ),
//TODO                                                   elem->maImage, elem->maImage );

//        SvtContentEntry* pUserData2 = new SvtContentEntry( elem->maTargetURL,
//                                                          elem->mbIsFolder );

//        pEntry2->SetUserData( pUserData2 );
    }

    ++mnSuspendSelectCallback;
    mxView->thaw();

    //set sort indicator
    weld::TreeView* pView = mxView->getWidget();
    pView->set_sort_indicator(mbAscending ? TRISTATE_TRUE : TRISTATE_FALSE, GetSortColumn());

//TODO    mxIconView->SetUpdateMode( true );
    --mnSuspendSelectCallback;

    ResetCursor();
}

void SvtFileView_Impl::ResetCursor()
{
    if (mxView->get_visible())
    {
        mxView->unselect_all();
        std::unique_ptr<weld::TreeIter> xFirst = mxView->make_iterator();
        if (mxView->get_iter_first(*xFirst))
            mxView->set_cursor(*xFirst);
    }
    else
    {
        // deselect
        SvTreeListEntry* pEntry = mpCurView->FirstSelected();
        if ( pEntry )
            mpCurView->Select( pEntry, false );
        // set cursor to the first entry
        mpCurView->SetCursor( mpCurView->First(), true );
        mpCurView->Update();
    }
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

OUString SvtFileView_Impl::ReplaceTabWithString(const OUString& rValue)
{
    OUString const aTab( "\t" );
    OUString const aTabString( "%09" );

    sal_Int32 iPos;
    OUString aValue(rValue);
    while ( ( iPos = aValue.indexOf( aTab ) ) >= 0 )
        aValue = aValue.replaceAt( iPos, 1, aTabString );
    return aValue;
}

void SvtFileView_Impl::CreateDisplayText_Impl()
{
    ::osl::MutexGuard aGuard( maMutex );

    OUString const aDateSep( ", " );

    for (auto const& elem : maContent)
    {
        // title, type, size, date
        elem->maDisplayName = ReplaceTabWithString(elem->GetTitle());
        // folders don't have a size
        if ( ! elem->mbIsFolder )
            elem->maDisplaySize = CreateExactSizeText( elem->maSize );
        // set the date, but volumes have no date
        if ( ! elem->mbIsFolder || ! elem->mbIsVolume )
        {
            SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
            elem->maDisplayDate = rLocaleData.getDate( elem->maModDate )
                                  + aDateSep
                                  + rLocaleData.getTime( elem->maModDate, false );
        }

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
        (*aFoundElem)->maDisplayName = ReplaceTabWithString(rTitle);

        INetURLObject aURLObj( rURL );
        aURLObj.setName( rTitle, INetURLObject::EncodeMechanism::All );

        rURL = aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        (*aFoundElem)->maTargetURL = rURL;
    }
}

const SortingData_Impl& SvtFileView_Impl::FolderInserted( const OUString& rURL, const OUString& rTitle )
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

    OUString const aDateSep( ", " );

    // title, type, size, date
    pData->maDisplayName = ReplaceTabWithString(pData->GetTitle());
    // set the date
    SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleData = aSysLocale.GetLocaleData();
    pData->maDisplayDate = rLocaleData.getDate( pData->maModDate )
                           + aDateSep
                           + rLocaleData.getTime( pData->maModDate );

    maContent.push_back( std::move(pData) );

    return *maContent.back();
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
//            mxIconView->Hide();
            break;

        case eIcon:
//            mpCurView = mxIconView;
            mxView->hide();
//            mxIconView->Show();
            break;

        default:
//TODO            mpCurView = mxView;
            mxView->show();
//            mxIconView->Hide();
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

void SvtFileView::selected_foreach(const std::function<bool(weld::TreeIter&)>& func)
{
    if (mpImpl->mxView->get_visible())
        mpImpl->mxView->selected_foreach(func);
#if 0
    else
        mxIconView->GrabFocus();
#endif
}

weld::Widget* SvtFileView::identifier() const
{
    return mpImpl->mxView->getWidget();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
