/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PostItMgr.hxx,v $
 * $Revision: 1.8.84.5 $
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

#ifndef _POSTITMGR_HXX
#define _POSTITMGR_HXX

#include <svtools/lstner.hxx>

#include <list>
#include <vector>
#include <svx/outlobj.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <swrect.hxx>

#include <com/sun/star/util/SearchOptions.hpp>

class SwWrtShell;
class SwDoc;
class SwView;
class SwPostItField;
class SwFmtFld;
class SwField;
class SfxBroadcaster;
class SfxHint;
class SwEditWin;
class Color;
class SvxSearchItem;
class SvxLanguageItem;
class SwPostIt;
class SwMarginWin;
class SwMarginItem;

#define SORT_POS    1
#define SORT_AUTHOR 2
#define SORT_DATE   3

#define COL_NOTES_SIDEPANE_ARROW_ENABLED    RGB_COLORDATA(0,0,0)
#define COL_NOTES_SIDEPANE_ARROW_DISABLED   RGB_COLORDATA(172,168,153)

typedef std::list<SwMarginItem*> SwMarginItem_list;

struct SwPostItPageItem
{
    bool bScrollbar;
    bool bMarginSide;
    long lOffset;
    SwRect mPageRect;
    SwMarginItem_list* mList;
    SwPostItPageItem(): bScrollbar(false),lOffset(0)
    {
        mList = new SwMarginItem_list;
    }
    ~SwPostItPageItem()
    {
        mList->clear();
        delete mList;
    }

};

struct FieldShadowState
{
    const SwPostItField* mpShadowFld;
    bool bCursor;
    bool bMouse;
    FieldShadowState(): mpShadowFld(0),bCursor(false),bMouse(false)
    {
    }
};

typedef std::list<SwMarginItem*>::iterator SwMarginItem_iterator;

class SwPostItMgr: public SfxListener
{
    private:
        SwView*                         mpView;
        SwWrtShell*                     mpWrtShell;
        SwEditWin*                      mpEditWin;
        std::list< SwMarginItem*>       mvPostItFlds;
        std::vector<SwPostItPageItem*>  mPages;
        ULONG                           mnEventId;
        bool                            mbWaitingForCalcRects;
        SwMarginWin*                    mpActivePostIt;
        bool                            mbLayout;
        long                            mbLayoutHeight;
        long                            mbLayouting;
        bool                            mbReadOnly;
        bool                            mbDeleteNote;
        FieldShadowState                mShadowState;
        OutlinerParaObject*             mpAnswer;

        typedef std::list<SwMarginWin*>::iterator   SwMarginWin_iterator;

        void            AddPostIts(bool bCheckExistance = true,bool bFocus = true);
        //void          AddRedlineComments(bool bCheckExistance, bool bFocus);
        void            RemoveMarginWin();
        void            PreparePageContainer();
        void            Scroll(const long lScroll,const unsigned long aPage );
        void            AutoScroll(const SwMarginWin* pPostIt,const unsigned long aPage );
        bool            ScrollbarHit(const unsigned long aPage,const Point &aPoint);
        bool            LayoutByPage(std::list<SwMarginWin*> &aVisiblePostItList,const Rectangle aBorder,long lNeededHeight);
        void            CheckForRemovedPostIts();
        bool            ArrowEnabled(USHORT aDirection,unsigned long aPage) const;
        bool            BorderOverPageBorder(unsigned long aPage) const;
        bool            HasScrollbars() const;
        void            Focus(SfxBroadcaster& rBC);

        sal_Int32       GetInitialAnchorDistance() const;
        sal_Int32       GetScrollSize() const;
        sal_Int32       GetSpaceBetween() const;
        void            SetReadOnlyState();
                        DECL_LINK( CalcHdl, void*);

    protected:

    public:
            SwPostItMgr(SwView* aDoc);
            ~SwPostItMgr();

            typedef std::list< SwMarginItem* >::const_iterator const_iterator;
            const_iterator begin()  const { return mvPostItFlds.begin(); }
            const_iterator end()    const { return mvPostItFlds.end();  }

            void InsertItem( SfxBroadcaster* pItem, bool bCheckExistance, bool bFocus);
            void RemoveItem( SfxBroadcaster* pBroadcast );
            void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

            void LayoutPostIts();
            bool CalcRects();

            void MakeVisible(const SwMarginWin* pPostIt,long aPage = -1);

            bool ShowScrollbar(const unsigned long aPage) const;
            bool HasNotes() const ;
            bool ShowNotes() const;
            unsigned long GetSidebarWidth(bool bPx = false) const;
            unsigned long GetSidebarBorderWidth(bool bPx = false) const;
            unsigned long GetNoteWidth();

            void PrepareView(bool bIgnoreCount = false);

            void CorrectPositions();

            void Sort(const short aType);

            void SetLayout() { mbLayout = true; };
            void Delete(String aAuthor);
            void Delete();

            void Hide( SwPostItField* pPostItField );
            void Hide( const String& rAuthor );
            void Hide();
            void Show();

            void Rescale();

            Rectangle GetBottomScrollRect(const unsigned long aPage) const;
            Rectangle GetTopScrollRect(const unsigned long aPage) const;

            bool IsHit(const Point &aPointPixel);
            Color GetArrowColor(USHORT aDirection,unsigned long aPage) const;

            SwMarginWin* GetNextPostIt(USHORT aDirection, SwMarginWin* aPostIt);
            long GetNextBorder();
            SwMarginWin* GetActivePostIt() { return mpActivePostIt; }
            void      SetActivePostIt( SwMarginWin* p);
            sal_Int32 GetMinimumSizeWithMeta() const;
            sal_Int32 GetSidebarScrollerHeight() const;

            SwMarginWin* GetPostIt(const SfxBroadcaster* pBroadcaster) const;
            SwMarginWin* GetPostIt(SfxBroadcaster* pBroadcaster) const;
            SwPostIt* GetPostIt(const SwPostItField* pFld) const;
            SwPostIt* GetPostIt(SwPostItField* pFld) const;

            void SetShadowState(const SwPostItField* pFld,bool bCursor = true);

            void SetSpellChecking();

            Color           GetColorDark(sal_uInt16 aAuthorIndex);
            Color           GetColorLight(sal_uInt16 aAuthorIndex);
            Color           GetColorAnkor(sal_uInt16 aAuthorIndex);

            bool                ShowPreview(const SwField* pFld,SwFmtFld*& pFmtFld) const;

            void                RegisterAnswer(OutlinerParaObject* pAnswer) { mpAnswer = pAnswer;}
            OutlinerParaObject* IsAnswer() {return mpAnswer;}

            sal_uInt16 Replace(SvxSearchItem* pItem);
            void StartSearchAndReplace(const SvxSearchItem& rSearchItem);
            sal_uInt16 SearchReplace(const SwFmtFld &pFld, const ::com::sun::star::util::SearchOptions& rSearchOptions,bool bSrchForward);
            sal_uInt16 FinishSearchReplace(const ::com::sun::star::util::SearchOptions& rSearchOptions,bool bSrchForward);
};

#endif
