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
#ifndef _SFXDOCVOR_HXX
#define _SFXDOCVOR_HXX


#include <vcl/dialog.hxx>
#include <svtools/treelistbox.hxx>

#include <sfx2/objsh.hxx>
#include "orgmgr.hxx"

//=========================================================================

class SfxDocumentTemplates;
class Path;

//=========================================================================

#ifndef _SFX_HXX

class SfxOrganizeDlg_Impl;

class SfxOrganizeListBox_Impl : public SvTreeListBox
{
    enum BMPTYPE            { BMPTYPE_FOLDER, BMPTYPE_DOC };

friend class SfxOrganizeDlg_Impl;

    Image                   aOpenedFolderBmp;
    Image                   aClosedFolderBmp;
    Image                   aOpenedDocBmp;
    Image                   aClosedDocBmp;

    SfxOrganizeMgr*         pMgr;
    SfxOrganizeDlg_Impl*    pDlg;

    static sal_Bool             bDropMoveOk;

    DECL_LINK( OnAsyncExecuteDrop, ExecuteDropEvent* );

protected:
    virtual sal_Bool EditingEntry( SvLBoxEntry* pEntry, Selection & );
    virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText );
    virtual sal_Bool NotifyMoving(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, sal_uIntPtr &);
    virtual sal_Bool NotifyCopying(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, sal_uIntPtr &);
    virtual void RequestingChildren( SvLBoxEntry* pParent );
    virtual long ExpandingHdl();
    virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );

        using SvTreeListBox::ExecuteDrop;
    // new d&d
    virtual DragDropMode    NotifyStartDrag( TransferDataContainer&, SvLBoxEntry* );
    virtual sal_Bool            NotifyAcceptDrop( SvLBoxEntry* );
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void            DragFinished( sal_Int8 nDropAction );

public:
        using SvListView::Select;
    enum DataEnum   { VIEW_TEMPLATES, VIEW_FILES } eViewType;

    SfxOrganizeListBox_Impl( SfxOrganizeDlg_Impl* pDlg, Window* pParent, WinBits, DataEnum );

    DataEnum    GetViewType() const { return eViewType; }
    void        SetViewType(DataEnum eType) { eViewType = eType; }

    void SetMgr(SfxOrganizeMgr *pM) { pMgr = pM; }
    void Reset();
    inline void SetBitmaps(
                    const Image &rOFolderBmp, const Image &rCFolderBmp, const Image &rODocBmp, const Image &rCDocBmp
                    );
    const Image &GetClosedBmp(sal_uInt16 nLevel) const;
    const Image &GetOpenedBmp(sal_uInt16 nLevel) const;

    virtual PopupMenu*  CreateContextMenu();

private:
    sal_Bool IsStandard_Impl( SvLBoxEntry *) const;
    sal_Bool MoveOrCopyTemplates(SvTreeListBox *pSourceBox,
                            SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent,
                            sal_uIntPtr &rIdx,
                            sal_Bool bCopy);
    sal_Bool MoveOrCopyContents(SvTreeListBox *pSourceBox,
                            SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent,
                            sal_uIntPtr &rIdx,
                            sal_Bool bCopy);
    inline sal_uInt16       GetDocLevel() const;
    SfxObjectShellRef   GetObjectShell( const Path& );
    sal_Bool                IsUniqName_Impl( const String &rText,
                                         SvLBoxEntry* pParent, SvLBoxEntry* pEntry = 0 ) const;
    sal_uInt16              GetLevelCount_Impl( SvLBoxEntry* pParent ) const;

    SvLBoxEntry*        InsertEntryByBmpType( const XubString& rText, BMPTYPE eBmpType,
                            SvLBoxEntry* pParent = NULL, sal_Bool bChildrenOnDemand = sal_False,
                            sal_uIntPtr nPos = LIST_APPEND, void* pUserData = NULL );
};

#endif // _SFX_HXX

//=========================================================================

class SfxTemplateOrganizeDlg : public ModalDialog
{
friend class SfxOrganizeListBox_Impl;

    class SfxOrganizeDlg_Impl *pImp;

public:
    SfxTemplateOrganizeDlg(Window * pParent, SfxDocumentTemplates* = 0);
    ~SfxTemplateOrganizeDlg();

#define RET_EDIT_STYLE       100

    virtual short Execute();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
