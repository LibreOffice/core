/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docvor.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:52:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXDOCVOR_HXX
#define _SFXDOCVOR_HXX


#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include "objsh.hxx"
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

    static BOOL             bDropMoveOk;

    DECL_LINK( OnAsyncExecuteDrop, ExecuteDropEvent* );

protected:
    virtual BOOL EditingEntry( SvLBoxEntry* pEntry, Selection & );
    virtual BOOL EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );
    virtual BOOL NotifyMoving(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, ULONG &);
    virtual BOOL NotifyCopying(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, ULONG &);
    virtual void RequestingChilds( SvLBoxEntry* pParent );
    virtual long ExpandingHdl();
    virtual BOOL Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );

    // new d&d
    virtual DragDropMode    NotifyStartDrag( TransferDataContainer&, SvLBoxEntry* );
    virtual BOOL            NotifyAcceptDrop( SvLBoxEntry* );
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void            DragFinished( sal_Int8 nDropAction );

public:
    enum DataEnum   { VIEW_TEMPLATES, VIEW_FILES } eViewType;

    SfxOrganizeListBox_Impl( SfxOrganizeDlg_Impl* pDlg, Window* pParent, WinBits, DataEnum );

    DataEnum    GetViewType() const { return eViewType; }
    void        SetViewType(DataEnum eType) { eViewType = eType; }

    void SetMgr(SfxOrganizeMgr *pM) { pMgr = pM; }
    void Reset();
    inline void SetBitmaps(
                    const Image &rOFolderBmp, const Image &rCFolderBmp, const Image &rODocBmp, const Image &rCDocBmp,
                    const Image &rOFolderBmpHC, const Image &rCFolderBmpHC, const Image &rODocBmpHC, const Image &rCDocBmpHC );
    const Image &GetClosedBmp(USHORT nLevel) const;
    const Image &GetOpenedBmp(USHORT nLevel) const;

    virtual PopupMenu*  CreateContextMenu();

private:
    BOOL IsStandard_Impl( SvLBoxEntry *) const;
    BOOL MoveOrCopyTemplates(SvLBox *pSourceBox,
                            SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent,
                            ULONG &rIdx,
                            BOOL bCopy);
    BOOL MoveOrCopyContents(SvLBox *pSourceBox,
                            SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent,
                            ULONG &rIdx,
                            BOOL bCopy);
    inline USHORT       GetDocLevel() const;
    SfxObjectShellRef   GetObjectShell( const Path& );
    BOOL                IsUniqName_Impl( const String &rText,
                                         SvLBoxEntry* pParent, SvLBoxEntry* pEntry = 0 ) const;
    USHORT              GetLevelCount_Impl( SvLBoxEntry* pParent ) const;

    SvLBoxEntry*        InsertEntryByBmpType( const XubString& rText, BMPTYPE eBmpType,
                            SvLBoxEntry* pParent = NULL, BOOL bChildsOnDemand = FALSE,
                            ULONG nPos = LIST_APPEND, void* pUserData = NULL );
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
