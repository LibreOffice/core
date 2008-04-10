/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PostItMgr.hxx,v $
 * $Revision: 1.4 $
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

#include <tools/link.hxx>
#include <swrect.hxx>

class SwWrtShell;
class SwDoc;
class SwView;
class SwPostItField;
class SwFmtFld;
class SfxBroadcaster;
class SfxHint;
class SwPostIt;
class SwEditWin;
class Color;

#define SORT_POS    1
#define SORT_AUTHOR 2
#define SORT_DATE   3

#define COL_NOTES_SIDEPANE_ARROW_ENABLED    RGB_COLORDATA(0,0,0)
#define COL_NOTES_SIDEPANE_ARROW_DISABLED   RGB_COLORDATA(172,168,153)


struct SwPostItItem
{
    bool bShow;
    bool bFocus;
    SwFmtFld* pFmtFld;
    SwPostIt* pPostIt;
    SwRect mPos;
    SwRect mFramePos;
    SwRect mPagePos;
    SwPostItItem( SwFmtFld* p, bool aShow, bool aFocus)
        : bShow(aShow),
        bFocus(aFocus),
        pFmtFld(p),
        pPostIt(0)
    {
    }
};

typedef std::list<SwPostItItem*> SwPostItItem_list;

struct SwPostItPageItem
{
    bool bScrollbar;
    bool bMarginSide;
    long lOffset;
    SwRect mPageRect;
    SwPostItItem_list* mList;
    SwPostItPageItem(): bScrollbar(false),lOffset(0)
    {
        mList = new SwPostItItem_list;
    }
    ~SwPostItPageItem()
    {
        mList->clear();
        delete mList;
    }

};

class SwPostItMgr: public SfxListener
{
    private:
        SwView*                         mpView;
        SwWrtShell*                     mpWrtShell;
        SwEditWin*                      mpEditWin;
        std::list< SwPostItItem*>       mvPostItFlds;
        std::vector<SwPostItPageItem*>  mPages;
        ULONG                           mnEventId;
        bool                            mbWaitingForCalcRects;
        SwPostIt*                       mpActivePostIt;
        bool                            mbLayout;
        long                            mbLayoutHeight;
        long                            mbLayouting;
        bool                            mbDeletingSeveral;

        typedef std::list<SwPostItItem*>::iterator  SwPostItItem_iterator;
        typedef std::list<SwPostIt*>::iterator      SwPostIt_iterator;

        void            AddPostIts(bool bCheckExistance = true,bool bFocus = true);
        void            RemovePostIts();
        void            PreparePageContainer();
        void            Scroll(const long lScroll,const unsigned long aPage );
        void            AutoScroll(const SwPostIt* pPostIt,const unsigned long aPage );
        bool            ScrollbarHit(const unsigned long aPage,const Point &aPoint);
        bool            LayoutByPage(std::list<SwPostIt*> &aVisiblePostItList,const Rectangle aBorder,long lNeededHeight);
        void            CheckForRemovedPostIts();
        bool            ArrowEnabled(USHORT aDirection,unsigned long aPage) const;
        bool            BorderOverPageBorder(unsigned long aPage) const;
        bool            HasScrollbars() const;
        sal_Int32       GetInitialAnchorDistance() const;
        sal_Int32       GetScrollSize() const;
        sal_Int32       GetSpaceBetween() const;
        void            SetReadOnlyState();
                        DECL_LINK( CalcHdl, void*);

    protected:

    public:
            SwPostItMgr(SwView* aDoc);
            ~SwPostItMgr();

            typedef std::list< SwPostItItem* >::const_iterator const_iterator;
            const_iterator begin()  const { return mvPostItFlds.begin(); }
            const_iterator end()    const { return mvPostItFlds.end();  }

            void InsertFld( SwFmtFld* aField, bool bCheckExistance, bool bFocus);
            void RemoveFld( SfxBroadcaster* pFld );
            void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

            void LayoutPostIts();
            bool CalcRects();

            void AutoScroll(const SwPostIt* pPostIt);
            bool ShowScrollbar(const unsigned long aPage) const;
            bool HasNotes() const ;
            bool ShowNotes() const;
            unsigned long GetSidebarWidth(bool bPx = false) const;
            unsigned long GetSidebarBorderWidth(bool bPx = false) const;
            unsigned long GetNoteWidth();

            void PrepareView(bool bIgnoreCount = false);

            void CorrectPositions();
            void SetColors(SwPostIt* pPostIt, SwPostItField* pFld);

            void Sort(const short aType);

            void SetLayout() { mbLayout = true; };
            void Delete(String aAuthor);
            void Delete();

            void Hide(SwPostItField* aPostItField, bool All = false);
            void Hide();
            void Show();

            void Rescale();

            Rectangle GetBottomScrollRect(const unsigned long aPage) const;
            Rectangle GetTopScrollRect(const unsigned long aPage) const;

            bool IsHit(const Point &aPointPixel);
            Color GetArrowColor(USHORT aDirection,unsigned long aPage) const;

            SwPostIt* GetNextPostIt(USHORT aDirection, SwPostIt* aPostIt);
            long GetNextBorder();
            SwFmtFld* GetFmtFld(SwPostIt* mpPostIt);
            SwPostIt* GetActivePostIt() { return mpActivePostIt; }
            void      SetActivePostIt( SwPostIt* p);
            sal_Int32 GetMinimumSizeWithMeta() const;
            sal_Int32 GetSidebarScrollerHeight() const;

            void SetSpellChecking(bool bEnable);
};

#endif


