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
#ifndef INCLUDED_SVTOOLS_FILEVIEW_HXX
#define INCLUDED_SVTOOLS_FILEVIEW_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/ucb/XContent.hpp>
#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <rtl/ustring.hxx>

// class SvtFileView -----------------------------------------------------

#define FILEVIEW_ONLYFOLDER         0x0001
#define FILEVIEW_MULTISELECTION     0x0002

#define FILEVIEW_SHOW_ONLYTITLE     0x0010
#define FILEVIEW_SHOW_NONE          0x0020

class ViewTabListBox_Impl;
class SvtFileView_Impl;
class SvTreeListEntry;
class HeaderBar;

/// the result of an action in the FileView
enum FileViewResult
{
    eSuccess,
    eFailure,
    eTimeout,
    eStillRunning
};

/// describes parameters for doing an action on the FileView asynchronously
struct FileViewAsyncAction
{
    sal_uInt32  nMinTimeout;    /// minimum time to wait for a result, in milliseconds
    sal_uInt32  nMaxTimeout;    /// maximum time to wait for a result, in milliseconds, until eTimeout is returned
    Link        aFinishHandler; /// the handler to be called when the action is finished. Called in every case, no matter of the result

    FileViewAsyncAction()
    {
        nMinTimeout = nMaxTimeout = 0;
    }
};

class SVT_DLLPUBLIC SvtFileView : public Control
{
private:
    SvtFileView_Impl*       mpImp;
    bool                    bSortColumn;

    ::com::sun::star::uno::Sequence< OUString > mpBlackList;

    DECL_DLLPRIVATE_LINK(               HeaderSelect_Impl, HeaderBar * );
    DECL_DLLPRIVATE_LINK(               HeaderEndDrag_Impl, HeaderBar * );

protected:
    virtual void GetFocus() SAL_OVERRIDE;

public:
    SvtFileView( Window* pParent, const ResId& rResId, bool bOnlyFolder, bool bMultiSelection );
    SvtFileView( Window* pParent, const ResId& rResId, sal_uInt8 nFlags );
    ~SvtFileView();

    const OUString&         GetViewURL() const;
    OUString                GetURL( SvTreeListEntry* pEntry ) const;
    OUString                GetCurrentURL() const;

    bool                    GetParentURL( OUString& _rParentURL ) const;
    void                    CreatedFolder( const OUString& rUrl, const OUString& rNewFolder );

    void                    SetHelpId( const OString& rHelpId );
    const OString&          GetHelpId( ) const;
    void                    SetSizePixel( const Size& rNewSize ) SAL_OVERRIDE;
    virtual void            SetPosSizePixel( const Point& rNewPos, const Size& rNewSize ) SAL_OVERRIDE;
    void                    SetSortColumn( bool bValue ) { bSortColumn = bValue; }
    bool                    GetSortColumn() { return bSortColumn; }

    /** initialize the view with the content of a folder given by URL, and aply an immediate filter

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
                                const ::com::sun::star::uno::Sequence< OUString >& rBlackList
                            );

    FileViewResult          Initialize(
                                const OUString& rFolderURL,
                                const OUString& rFilter,
                                const FileViewAsyncAction* pAsyncDescriptor );
    /** initialze the view with a sequence of contents, which have already been obtained elsewhere

        This method will never return <member>eStillRunning</member>, since it will fill the
        view synchronously
    */
    bool                    Initialize( const ::com::sun::star::uno::Sequence< OUString >& aContents );

    /** initializes the view with the content of a folder given by an UCB content
    */
    bool                    Initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent,
                                        const OUString& rFilter );

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

    void                    SetSelectHdl( const Link& rHdl );
    void                    SetDoubleClickHdl( const Link& rHdl );
    void                    SetOpenDoneHdl( const Link& rHdl );

    sal_uLong                   GetSelectionCount() const;
    SvTreeListEntry*            FirstSelected() const;
    SvTreeListEntry*            NextSelected( SvTreeListEntry* pEntry ) const;
    void                    EnableAutoResize();
    void                    SetFocus();

    void                    EnableContextMenu( bool bEnable );
    void                    EnableDelete( bool bEnable );
    void                    EnableNameReplacing( bool bEnable = true );
                                // translate folder names or display doc-title instead of file name
                                // EnableContextMenu( sal_True )/EnableDelete(sal_True) disable name replacing!

                            // save and load column size and sort order
    OUString                GetConfigString() const;
    void                    SetConfigString( const OUString& rCfgStr );

    void                    EndInplaceEditing( bool _bCancel );

protected:
    virtual void            StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
};

// struct SvtContentEntry ------------------------------------------------

struct SvtContentEntry
{
    bool     mbIsFolder;
    OUString maURL;

    SvtContentEntry( const OUString& rURL, bool bIsFolder ) :
        mbIsFolder( bIsFolder ), maURL( rURL ) {}
};

namespace svtools {


// QueryDeleteDlg_Impl


enum QueryDeleteResult_Impl
{
    QUERYDELETE_CANCEL = RET_CANCEL,
    QUERYDELETE_YES = RET_YES,
    QUERYDELETE_NO = RET_NO,
    QUERYDELETE_ALL = -1
};

class SVT_DLLPUBLIC QueryDeleteDlg_Impl : public MessageDialog
{
private:
    PushButton* m_pAllButton;
public:

    QueryDeleteDlg_Impl(Window* pParent, const OUString& rName);

    void EnableAllButton() { m_pAllButton->Enable(true); }
};

}

#endif // INCLUDED_SVTOOLS_FILEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
