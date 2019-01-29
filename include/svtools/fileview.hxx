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
#ifndef INCLUDED_VCL_FILEVIEW_HXX
#define INCLUDED_VCL_FILEVIEW_HXX

#include <memory>
#include <svtools/svtdllapi.h>
#include <com/sun/star/uno/Sequence.h>
#include <vcl/ctrl.hxx>
#include <vcl/weld.hxx>
#include <rtl/ustring.hxx>

namespace com :: sun :: star :: ucb { class XContent; }

// class SvtFileView -----------------------------------------------------

class SvtFileView_Impl;
class SvTreeListEntry;
class HeaderBar;
struct SvtContentEntry;
class SvTreeListBox;

/// the result of an action in the FileView
enum FileViewResult
{
    eSuccess,
    eFailure,
    eTimeout,
    eStillRunning
};

enum FileViewMode
{
    eDetailedList,
    eIcon
};

/// describes parameters for doing an action on the FileView asynchronously
struct FileViewAsyncAction
{
    sal_uInt32  nMinTimeout;    /// minimum time to wait for a result, in milliseconds
    sal_uInt32  nMaxTimeout;    /// maximum time to wait for a result, in milliseconds, until eTimeout is returned
    Link<void*,void>  aFinishHandler; /// the handler to be called when the action is finished. Called in every case, no matter of the result

    FileViewAsyncAction()
    {
        nMinTimeout = nMaxTimeout = 0;
    }
};

class SVT_DLLPUBLIC SvtFileView : public Control
{
private:
    std::unique_ptr<SvtFileView_Impl> mpImpl;
    css::uno::Sequence< OUString >    mpBlackList;

    DECL_DLLPRIVATE_LINK( HeaderSelect_Impl, HeaderBar*, void );
    DECL_DLLPRIVATE_LINK( HeaderEndDrag_Impl, HeaderBar*, void );

protected:
    virtual void GetFocus() override;

public:
    SvtFileView( vcl::Window* pParent, WinBits nBits, bool bOnlyFolder, bool bMultiSelection, bool bShowType = true );
    virtual ~SvtFileView() override;
    virtual void dispose() override;

    virtual Size GetOptimalSize() const override;

    void                    SetViewMode( FileViewMode eMode );

    const OUString&         GetViewURL() const;
    static OUString         GetURL( SvTreeListEntry const * pEntry );
    OUString                GetCurrentURL() const;

    bool                    GetParentURL( OUString& _rParentURL ) const;
    void                    CreatedFolder( const OUString& rUrl, const OUString& rNewFolder );

    void                    SetHelpId( const OString& rHelpId );
    const OString&          GetHelpId( ) const;
    void                    SetSizePixel( const Size& rNewSize ) override;
    virtual void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) override;

    /** initialize the view with the content of a folder given by URL, and apply an immediate filter

        @param rFolderURL
            the URL of the folder whose content is to be read
        @param rFilter
            the initial filter to be applied
        @param pAsyncDescriptor
            If not <NULL/>, this struct describes the parameters for doing the
            action asynchronously.
    */
    FileViewResult          Initialize(
                                const OUString& rFolderURL,
                                const OUString& rFilter,
                                const FileViewAsyncAction* pAsyncDescriptor,
                                const css::uno::Sequence< OUString >& rBlackList
                            );

    /** initializes the view with the content of a folder given by an UCB content
    */
    bool                    Initialize( const css::uno::Reference< css::ucb::XContent>& _xContent );

    /** reads the current content of the current folder again, and applies the given filter to it

        Note 1: The folder is really read a second time. This implies that any new elements (which were
        not present when you called Initialize the last time) are now displayed.

        Note 2: This method must not be called when you previously initialized the view from a sequence
        of strings, or a UNO content object.

        @param rFilter
            the filter to be applied
        @param pAsyncDescriptor
            If not <NULL/>, this struct describes the parameters for doing the
            action asynchronously.
    */
    FileViewResult          ExecuteFilter(
                                const OUString& rFilter,
                                const FileViewAsyncAction* pAsyncDescriptor
                            );

    /** cancels a running async action (if any)

        @seealso Initialize
        @seealso ExecuteFilter
        @seealso FileViewAsyncAction
    */
    void                    CancelRunningAsyncAction();

    /** initializes the view with the parent folder of the current folder

        @param rNewURL
            the URL of the folder which we just navigated to
        @param pAsyncDescriptor
            If not <NULL/>, this struct describes the parameters for doing the
            action asynchronously.
    */
    FileViewResult          PreviousLevel(
                                const FileViewAsyncAction* pAsyncDescriptor
                            );

    void                    SetNoSelection();

    void                    SetSelectHdl( const Link<SvTreeListBox*,void>& rHdl );
    void                    SetDoubleClickHdl( const Link<SvTreeListBox*,bool>& rHdl );
    void                    SetOpenDoneHdl( const Link<SvtFileView*,void>& rHdl );

    sal_uLong               GetSelectionCount() const;
    SvTreeListEntry*        FirstSelected() const;
    SvTreeListEntry*        NextSelected( SvTreeListEntry* pEntry ) const;
    void                    EnableAutoResize();

    void                    EnableDelete( bool bEnable );

                            // save and load column size and sort order
    OUString                GetConfigString() const;
    void                    SetConfigString( const OUString& rCfgStr );

    void                    EndInplaceEditing();

    ::std::vector< SvtContentEntry > GetContent();

protected:
    virtual void            StateChanged( StateChangedType nStateChange ) override;
};

// struct SvtContentEntry ------------------------------------------------

struct SvtContentEntry
{
    bool const     mbIsFolder;
    OUString maURL;

    SvtContentEntry( const OUString& rURL, bool bIsFolder ) :
        mbIsFolder( bIsFolder ), maURL( rURL ) {}
};

namespace svtools {


// QueryDeleteDlg_Impl


enum QueryDeleteResult_Impl
{
    QUERYDELETE_YES = RET_YES,
    QUERYDELETE_ALL = 101
};

class SVT_DLLPUBLIC QueryDeleteDlg_Impl :  public weld::MessageDialogController
{
private:
    std::unique_ptr<weld::Button> m_xAllButton;
public:

    QueryDeleteDlg_Impl(weld::Widget* pParent, const OUString& rName);
    virtual ~QueryDeleteDlg_Impl() override;

    void EnableAllButton() { m_xAllButton->set_sensitive(true); }
};

}

#endif // INCLUDED_VCL_FILEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
