/*************************************************************************
 *
 *  $RCSfile: acredlin.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 14:34:29 $
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

#ifndef SC_ACREDLIN_HXX
#define SC_ACREDLIN_HXX

#ifndef VCL
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _HEADBAR_HXX //autogen
#include <svtools/headbar.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif


#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVX_ACREDLIN_HXX
#include <svx/ctredlin.hxx>
#endif

#ifndef _SVX_SIMPTABL_HXX
#include <svx/simptabl.hxx>
#endif

#ifndef _SVARRAY_HXX
#define _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif

#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

class ScViewData;
class ScDocument;

#define FLT_DATE_BEFORE     0
#define FLT_DATE_SINCE      1
#define FLT_DATE_EQUAL      2
#define FLT_DATE_NOTEQUAL   3
#define FLT_DATE_BETWEEN    4
#define FLT_DATE_SAVE       5


class ScViewEntryPtr
{
private:
    String*         pAction;
    String*         pPos;
    String*         pAuthor;
    String*         pDate;
    String*         pComment;
    void*           pData;

public:

    String*         GetpAction()    {return pAction; }
    String*         GetpPos()       {return pPos;    }
    String*         GetpAuthor()    {return pAuthor; }
    String*         GetpDate()      {return pDate;   }
    String*         GetpComment()   {return pComment;}
    void*           GetpData()      {return pData;   }

    void        SetpAction (String* pString)    {pAction= pString;}
    void        SetpPos    (String* pString)    {pPos   = pString;}
    void        SetpAuthor (String* pString)    {pAuthor= pString;}
    void        SetpDate   (String* pString)    {pDate  = pString;}
    void        SetpComment(String* pString)    {pComment=pString;}
    void        SetpData   (void*   pdata)      {pData   =pdata;}
};

class ScViewEntryPtrList
{
    ScViewEntryPtrList* pNext;
    ScViewEntryPtrList* pLast;

    ScViewEntryPtr* pData;
};


class ScRedlinData : public RedlinData
{
public:

                    ScRedlinData();
                    ~ScRedlinData();
    USHORT          nTable;
    USHORT          nCol;
    USHORT          nRow;
    ULONG           nActionNo;
    ULONG           nInfo;
    BOOL            bIsRejectable;
    BOOL            bIsAcceptable;
};

typedef long LExpNum;

//@ Expand-Entrys nicht eindeutig, daher gestrichen
//DECLARE_TABLE( ScChgTrackExps, LExpNum)
//==================================================================

class ScAcceptChgDlg : public SfxModelessDialog
{
private:

    Timer                   aSelectionTimer;
    Timer                   aReOpenTimer;
    SvxAcceptChgCtr         aAcceptChgCtr;
    ScViewData*             pViewData;
    ScDocument*             pDoc;
    ScRangeName             aLocalRangeName;
    Selection               theCurSel;
    SvxTPFilter*            pTPFilter;
    SvxTPView*              pTPView;
    SvxRedlinTable*         pTheView;
    Size                    MinSize;
    ScRangeList             aRangeList;
    ScChangeViewSettings    aChangeViewSet;
    String                  aStrInsertCols;
    String                  aStrInsertRows;
    String                  aStrInsertTabs;
    String                  aStrDeleteCols;
    String                  aStrDeleteRows;
    String                  aStrDeleteTabs;
    String                  aStrMove;
    String                  aStrContent;
    String                  aStrReject;
    String                  aUnknown;
    String                  aStrAllAccepted;
    String                  aStrAllRejected;
    String                  aStrNoEntry;
    String                  aStrContentWithChild;
    String                  aStrChildContent;
    String                  aStrChildOrgContent;
    String                  aStrEmpty;
    Bitmap                  aExpBmp;
    Bitmap                  aCollBmp;
    Bitmap                  aCloseBmp;
    Bitmap                  aOpenBmp;
    Bitmap                  aEndBmp;
    Bitmap                  aErrorBmp;
    ULONG                   nAcceptCount;
    ULONG                   nRejectCount;
    BOOL                    bAcceptEnableFlag;
    BOOL                    bRejectEnableFlag;
    BOOL                    bNeedsUpdate;
    BOOL                    bIgnoreMsg;
    BOOL                    bNoSelection;
    BOOL                    bHasFilterEntry;
    BOOL                    bUseColor;
    //ScChgTrackExps            aExpandArray;

    void            Init();
    void            InitFilter();
    void            SetMyStaticData();

    DECL_LINK( FilterHandle, SvxTPFilter* );
    DECL_LINK( RefHandle, SvxTPFilter* );
    DECL_LINK( FilterModified, SvxTPFilter* );
    DECL_LINK( MinSizeHandle, SvxAcceptChgCtr*);
    DECL_LINK( RejectHandle, SvxTPView*);
    DECL_LINK( AcceptHandle, SvxTPView*);
    DECL_LINK( RejectAllHandle, SvxTPView*);
    DECL_LINK( AcceptAllHandle, SvxTPView*);
    DECL_LINK( ExpandingHandle, SvxRedlinTable*);
    DECL_LINK( SelectHandle, SvxRedlinTable*);
    DECL_LINK( RefInfoHandle, String*);

    DECL_LINK( UpdateSelectionHdl, Timer*);
    DECL_LINK( ChgTrackModHdl, ScChangeTrack*);
    DECL_LINK( CommandHdl, Control*);
    DECL_LINK( ReOpenTimerHdl, Timer*);
    DECL_LINK( ColCompareHdl, SvSortData*);



protected:

    virtual void    Resize();
    virtual BOOL    Close();

    void            RejectFiltered();
    void            AcceptFiltered();

    BOOL            IsValidAction(const ScChangeAction* pScChangeAction);

    String*         MakeTypeString(ScChangeActionType eType);

    SvLBoxEntry*    InsertChangeAction(const ScChangeAction* pScChangeAction,ScChangeActionState eState,
                                    SvLBoxEntry* pParent=NULL,BOOL bDelMaster=FALSE,
                                    BOOL bDisabled=FALSE,ULONG nPos=LIST_APPEND);

    SvLBoxEntry*    InsertFilteredAction(const ScChangeAction* pScChangeAction,ScChangeActionState eState,
                                    SvLBoxEntry* pParent=NULL,BOOL bDelMaster=FALSE,
                                    BOOL bDisabled=FALSE,ULONG nPos=LIST_APPEND);


    SvLBoxEntry*    InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                              SvLBoxEntry* pParent,ULONG nSpecial);

    void            GetDependents( const ScChangeAction* pScChangeAction,
                                ScChangeActionTable& aActionTable,
                                SvLBoxEntry* pEntry);

    BOOL            InsertContentChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent);

    BOOL            InsertAcceptedORejected(SvLBoxEntry* pParent);

    BOOL            InsertDeletedChilds(const ScChangeAction *pChangeAction, ScChangeActionTable* pActionTable,
                                        SvLBoxEntry* pParent);

    BOOL            InsertChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent);

    void            AppendChanges(ScChangeTrack* pChanges,ULONG nStartAction, ULONG nEndAction,
                                    ULONG nPos=LIST_APPEND);

    void            RemoveEntrys(ULONG nStartAction,ULONG nEndAction);
    void            UpdateEntrys(ScChangeTrack* pChgTrack, ULONG nStartAction,ULONG nEndAction);

    void            UpdateView();
    void            ClearView();

    BOOL            Expand(ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                            SvLBoxEntry* pEntry, BOOL bFilter=FALSE);

public:
                    ScAcceptChgDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                               ScViewData*      ptrViewData);

                    ~ScAcceptChgDlg();

    void            ReInit(ScViewData* ptrViewData);

    virtual long    PreNotify( NotifyEvent& rNEvt );

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const;

};


#endif // SC_NAMEDLG_HXX

