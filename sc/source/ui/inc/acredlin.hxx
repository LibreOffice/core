/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_ACREDLIN_HXX
#define SC_ACREDLIN_HXX

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif
#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <svtools/headbar.hxx>
#include <svtools/svtabbx.hxx>


#include "rangenam.hxx"
#include "anyrefdg.hxx"
#include <vcl/lstbox.hxx>

#ifndef _SVX_ACREDLIN_HXX
#include <svx/ctredlin.hxx>
#endif
#include <svx/simptabl.hxx>

#ifndef _SVARRAY_HXX
#define _SVARRAY_HXX
#include <svl/svarray.hxx>
#endif
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include <vcl/timer.hxx>

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
    SCTAB           nTable;
    SCCOL           nCol;
    SCROW           nRow;
    sal_uLong           nActionNo;
    sal_uLong           nInfo;
    sal_Bool            bIsRejectable;
    sal_Bool            bIsAcceptable;
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
    SvxRedlinTable*         pTheView; // PB 2006/02/02 #i48648 now SvHeaderTabListBox
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
    sal_uLong                   nAcceptCount;
    sal_uLong                   nRejectCount;
    sal_Bool                    bAcceptEnableFlag;
    sal_Bool                    bRejectEnableFlag;
    sal_Bool                    bNeedsUpdate;
    sal_Bool                    bIgnoreMsg;
    sal_Bool                    bNoSelection;
    sal_Bool                    bHasFilterEntry;
    sal_Bool                    bUseColor;
    //ScChgTrackExps            aExpandArray;

    void            Init();
    void            InitFilter();
//UNUSED2008-05  void           SetMyStaticData();

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
    virtual sal_Bool    Close();

    void            RejectFiltered();
    void            AcceptFiltered();

    sal_Bool            IsValidAction(const ScChangeAction* pScChangeAction);

    String*         MakeTypeString(ScChangeActionType eType);

    SvLBoxEntry*    InsertChangeAction(const ScChangeAction* pScChangeAction,ScChangeActionState eState,
                                    SvLBoxEntry* pParent=NULL,sal_Bool bDelMaster=sal_False,
                                    sal_Bool bDisabled=sal_False,sal_uLong nPos=LIST_APPEND);

    SvLBoxEntry*    InsertFilteredAction(const ScChangeAction* pScChangeAction,ScChangeActionState eState,
                                    SvLBoxEntry* pParent=NULL,sal_Bool bDelMaster=sal_False,
                                    sal_Bool bDisabled=sal_False,sal_uLong nPos=LIST_APPEND);


    SvLBoxEntry*    InsertChangeActionContent(const ScChangeActionContent* pScChangeAction,
                                              SvLBoxEntry* pParent,sal_uLong nSpecial);

    void            GetDependents( const ScChangeAction* pScChangeAction,
                                ScChangeActionTable& aActionTable,
                                SvLBoxEntry* pEntry);

    sal_Bool            InsertContentChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent);

    sal_Bool            InsertAcceptedORejected(SvLBoxEntry* pParent);

    sal_Bool            InsertDeletedChilds(const ScChangeAction *pChangeAction, ScChangeActionTable* pActionTable,
                                        SvLBoxEntry* pParent);

    sal_Bool            InsertChilds(ScChangeActionTable* pActionTable,SvLBoxEntry* pParent);

    void            AppendChanges(ScChangeTrack* pChanges,sal_uLong nStartAction, sal_uLong nEndAction,
                                    sal_uLong nPos=LIST_APPEND);

    void            RemoveEntrys(sal_uLong nStartAction,sal_uLong nEndAction);
    void            UpdateEntrys(ScChangeTrack* pChgTrack, sal_uLong nStartAction,sal_uLong nEndAction);

    void            UpdateView();
    void            ClearView();

    sal_Bool            Expand(ScChangeTrack* pChanges,const ScChangeAction* pScChangeAction,
                            SvLBoxEntry* pEntry, sal_Bool bFilter=sal_False);

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

