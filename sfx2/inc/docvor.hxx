/*************************************************************************
 *
 *  $RCSfile: docvor.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:22 $
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

class SfxOrganizeListBox_Impl: public SvTreeListBox
{
friend class SfxOrganizeDlg_Impl;

    SfxOrganizeMgr *pMgr;
    Image aOpenedFolderBmp;
    Image aClosedFolderBmp;
    Image aOpenedDocBmp;
    Image aClosedDocBmp;
    SfxOrganizeDlg_Impl *pDlg;
    static BOOL bDropMoveOk;

protected:
    virtual BOOL EditingEntry( SvLBoxEntry* pEntry, Selection & );
    virtual BOOL EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );
    virtual BOOL NotifyQueryDrop(SvLBoxEntry *);
    virtual BOOL NotifyMoving(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, ULONG &);
    virtual BOOL NotifyCopying(SvLBoxEntry *pSource,
                            SvLBoxEntry* pTarget,
                            SvLBoxEntry *&pNewParent, ULONG &);
    virtual void RequestingChilds( SvLBoxEntry* pParent );
    virtual long ExpandingHdl();
#if SUPD < 306
    virtual BOOL Drop( const DropEvent& rEvt );
    virtual BOOL QueryDrop( const DropEvent& rEvt );
#else
    virtual BOOL Drop( DropEvent& rEvt );
    virtual BOOL QueryDrop( DropEvent& rEvt );
#endif
    virtual DragDropMode NotifyBeginDrag(SvLBoxEntry *);
    virtual BOOL Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );
    virtual void Command( const CommandEvent& rCEvt );

public:
    enum DataEnum {
        VIEW_TEMPLATES,
        VIEW_FILES
    } eViewType;
    SfxOrganizeListBox_Impl(SfxOrganizeDlg_Impl *pDlg,
                            Window *pParent, WinBits, DataEnum);

    DataEnum GetViewType() const { return eViewType; }
    void SetViewType(DataEnum eType) { eViewType = eType; }

    void SetMgr(SfxOrganizeMgr *pM) { pMgr = pM; }
    void Reset();
    void SetBitmaps(const Image &rOFolderBitmap,
                    const Image &rCFolderBitmap,
                    const Image &rODocBitmap,
                    const Image &rCDocBitmap) {
        aOpenedFolderBmp = rOFolderBitmap;
        aClosedFolderBmp = rCFolderBitmap;
        aOpenedDocBmp = rODocBitmap;
        aClosedDocBmp = rCDocBitmap;
    }
    const Image &GetClosedBmp(USHORT nLevel) const;
    const Image &GetOpenedBmp(USHORT nLevel) const;

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
    inline USHORT   GetDocLevel() const;
    SfxObjectShellRef GetObjectShell(const Path &);
    BOOL   IsUniqName_Impl(const String &rText, SvLBoxEntry* pParent, SvLBoxEntry *pEntry = 0) const;
    USHORT GetLevelCount_Impl(SvLBoxEntry* pParent) const;
};

#endif
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
