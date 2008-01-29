/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpusrlst.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:43:36 $
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

#ifndef SC_TPUSRLST_HXX
#define SC_TPUSRLST_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

//========================================================================
// Benutzerdefinierte Listen:

class ScUserList;
class ScDocument;
class ScViewData;
class ScRangeUtil;

class ScTpUserLists : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rAttrSet );
    static  USHORT*     GetRanges       ();
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreAttrs );
    virtual void        Reset           ( const SfxItemSet& rCoreAttrs );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
            ScTpUserLists( Window*              pParent,
                           const SfxItemSet&    rArgSet );
            ~ScTpUserLists();

private:
    FixedText       aFtLists;
    ListBox         aLbLists;
    FixedText       aFtEntries;
    MultiLineEdit   aEdEntries;
    FixedText       aFtCopyFrom;
    Edit            aEdCopyFrom;
    PushButton      aBtnNew;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnCopy;

    const String    aStrQueryRemove;
    const String    aStrNew;
    const String    aStrCancel;
    const String    aStrAdd;
    const String    aStrModify;
    const String    aStrCopyList;
    const String    aStrCopyFrom;
    const String    aStrCopyErr;

    const USHORT    nWhichUserLists;
    ScUserList*     pUserLists;

    ScDocument*     pDoc;
    ScViewData*     pViewData;
    ScRangeUtil*    pRangeUtil;
    String          aStrSelectedArea;

    BOOL            bModifyMode;
    BOOL            bCancelMode;
    BOOL            bCopyDone;
    USHORT          nCancelPos;

#ifdef _TPUSRLST_CXX
private:
    void    Init                ();
    USHORT  UpdateUserListBox   ();
    void    UpdateEntries       ( USHORT nList );
    void    MakeListStr         ( String& rListStr );
    void    AddNewList          ( const String& rEntriesStr );
    void    RemoveList          ( USHORT nList );
    void    ModifyList          ( USHORT        nSelList,
                                  const String& rEntriesStr );
    void    CopyListFromArea    ( const ScRefAddress& rStartPos,
                                  const ScRefAddress& rEndPos );

    // Handler:
    DECL_LINK( LbSelectHdl,     ListBox* );
    DECL_LINK( BtnClickHdl,     PushButton* );
    DECL_LINK( EdEntriesModHdl, MultiLineEdit* );
#endif
};



#endif // SC_TPUSRLST_HXX

