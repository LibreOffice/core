/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXDOCVOR_HXX
#define _SFXDOCVOR_HXX


#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <svtools/svtreebx.hxx>

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

    Image                   aOpenedFolderBmpHC;
    Image                   aClosedFolderBmpHC;
    Image                   aOpenedDocBmpHC;
    Image                   aClosedDocBmpHC;

    SfxOrganizeMgr*         pMgr;
    SfxOrganizeDlg_Impl*    pDlg;

    static sal_Bool             bDropMoveOk;

    DECL_LINK( OnAsyncExecuteDrop, ExecuteDropEvent* );

protected:
    virtual sal_Bool EditingEntry( SvLBoxEntry* pEntry, Selection & );
    virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );
    virtual sal_Bool NotifyMoving(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, sal_uIntPtr &);
    virtual sal_Bool NotifyCopying(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, sal_uIntPtr &);
    virtual void RequestingChilds( SvLBoxEntry* pParent );
    virtual long ExpandingHdl();
    virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );

        using SvLBox::ExecuteDrop;
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
                    const Image &rOFolderBmp, const Image &rCFolderBmp, const Image &rODocBmp, const Image &rCDocBmp,
                    const Image &rOFolderBmpHC, const Image &rCFolderBmpHC, const Image &rODocBmpHC, const Image &rCDocBmpHC );
    const Image &GetClosedBmp(sal_uInt16 nLevel) const;
    const Image &GetOpenedBmp(sal_uInt16 nLevel) const;

    virtual PopupMenu*  CreateContextMenu();

private:
    sal_Bool IsStandard_Impl( SvLBoxEntry *) const;
    sal_Bool MoveOrCopyTemplates(SvLBox *pSourceBox,
                            SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent,
                            sal_uIntPtr &rIdx,
                            sal_Bool bCopy);
    sal_Bool MoveOrCopyContents(SvLBox *pSourceBox,
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
                            SvLBoxEntry* pParent = NULL, sal_Bool bChildsOnDemand = sal_False,
                            sal_uIntPtr nPos = LIST_APPEND, void* pUserData = NULL );
};

#endif // _SFX_HXX

//=========================================================================

class SfxTemplateOrganizeDlg : public ModalDialog
{
friend class SfxOrganizeListBox_Impl;

    class SfxOrganizeDlg_Impl *pImp;

//  virtual void    DataChanged( const DataChangedEvent& rDCEvt );
public:
    SfxTemplateOrganizeDlg(Window * pParent, SfxDocumentTemplates* = 0);
    ~SfxTemplateOrganizeDlg();

#define RET_EDIT_STYLE       100

    virtual short Execute();
};

#endif
