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

#ifndef INCLUDED_SW_INC_POSTITMGR_HXX
#define INCLUDED_SW_INC_POSTITMGR_HXX

#include <list>
#include <vector>
#include <editeng/outlobj.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/link.hxx>
#include <swrect.hxx>
#include <unotools/configitem.hxx>
#include <unotools/options.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <SidebarWindowsTypes.hxx>
#include <svl/lstner.hxx>
#include <vcl/vclptr.hxx>
#include <LibreOfficeKit/LibreOfficeKitTypes.h>

class OutputDevice;
class SwWrtShell;
class SwDoc;
class SwView;
class SwPostItField;
class SwFormatField;
class SwField;
class SfxBroadcaster;
class SfxHint;
class SwEditWin;
class Color;
class SfxItemPool;
class SfxItemSet;
class SvxSearchItem;
class SvxLanguageItem;
namespace sw { namespace annotation {
    class SwAnnotationWin;
}}
namespace sw { namespace sidebarwindows {
    class SwSidebarWin;
    class SwFrameSidebarWinContainer;
}}
class SwSidebarItem;
class SwFrame;
namespace vcl { class Window; }
struct ImplSVEvent;
class OutlinerSearchable;

#define COL_NOTES_SIDEPANE_ARROW_ENABLED    RGB_COLORDATA(0,0,0)
#define COL_NOTES_SIDEPANE_ARROW_DISABLED   RGB_COLORDATA(172,168,153)

typedef std::list<SwSidebarItem*> SwSidebarItem_list;
typedef std::list<SwSidebarItem*>::iterator SwSidebarItem_iterator;

struct SwPostItPageItem
{
    bool bScrollbar;
    sw::sidebarwindows::SidebarPosition eSidebarPosition;
    long lOffset;
    SwRect mPageRect;
    SwSidebarItem_list* mList;
    SwPostItPageItem(): bScrollbar(false), eSidebarPosition( sw::sidebarwindows::SidebarPosition::LEFT ), lOffset(0)
    {
        mList = new SwSidebarItem_list;
    }
    ~SwPostItPageItem()
    {
        mList->clear();
        delete mList;
    }

};

struct FieldShadowState
{
    const SwPostItField* mpShadowField;
    bool bCursor;
    bool bMouse;
    FieldShadowState(): mpShadowField(nullptr),bCursor(false),bMouse(false)
    {
    }
};

class SwNoteProps: public utl::ConfigItem
{
    private:
        bool bIsShowAnchor;

        virtual void ImplCommit() override;

    public:
        SwNoteProps()
            : ConfigItem(OUString("Office.Writer/Notes"))
            , bIsShowAnchor(false)
        {
            const css::uno::Sequence< OUString >& rNames = GetPropertyNames();
            css::uno::Sequence< css::uno::Any > aValues = GetProperties(rNames);
            const css::uno::Any* pValues = aValues.getConstArray();
            SAL_WARN_IF(aValues.getLength() != rNames.getLength(), "sw", "GetProperties failed");
            if (aValues.getLength())
                    pValues[0]>>=bIsShowAnchor;
        }

        bool IsShowAnchor()
        {
            return bIsShowAnchor;
        }
        static css::uno::Sequence< OUString >& GetPropertyNames()
        {
            static css::uno::Sequence< OUString > aNames;
            if(!aNames.getLength())
            {
                aNames.realloc(1);
                OUString* pNames = aNames.getArray();
                pNames[0] = "ShowAnkor";
            }
            return aNames;
        }

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
};

class SwPostItMgr: public SfxListener
{
    private:
        SwView*                         mpView;
        SwWrtShell*                     mpWrtShell;
        VclPtr<SwEditWin>               mpEditWin;
        std::list< SwSidebarItem*>      mvPostItFields;
        std::vector<SwPostItPageItem*>  mPages;
        ImplSVEvent *                   mnEventId;
        bool                            mbWaitingForCalcRects;
        VclPtr<sw::sidebarwindows::SwSidebarWin> mpActivePostIt;
        bool                            mbLayout;
        long                            mbLayoutHeight;
        bool                            mbLayouting;
        bool                            mbReadOnly;
        bool                            mbDeleteNote;
        FieldShadowState                mShadowState;
        OutlinerParaObject*             mpAnswer;
        bool                            mbIsShowAnchor;

        // data structure to collect the <SwSidebarWin> instances for certain <SwFrame> instances.
        sw::sidebarwindows::SwFrameSidebarWinContainer* mpFrameSidebarWinContainer;

        typedef std::list<sw::sidebarwindows::SwSidebarWin*>::iterator  SwSidebarWin_iterator;

        void            AddPostIts(bool bCheckExistance = true,bool bFocus = true);
        void            RemoveSidebarWin();
        void            PreparePageContainer();
        void            Scroll(const long lScroll,const unsigned long aPage );
        void            AutoScroll(const sw::sidebarwindows::SwSidebarWin* pPostIt,const unsigned long aPage );
        bool            ScrollbarHit(const unsigned long aPage,const Point &aPoint);
        bool            LayoutByPage( std::list<sw::sidebarwindows::SwSidebarWin*> &aVisiblePostItList,
                                      const Rectangle& rBorder,
                                      long lNeededHeight);
        void            CheckForRemovedPostIts();
        bool            ArrowEnabled(sal_uInt16 aDirection,unsigned long aPage) const;
        bool            BorderOverPageBorder(unsigned long aPage) const;
        bool            HasScrollbars() const;
        void            Focus(SfxBroadcaster& rBC);

        sal_Int32       GetInitialAnchorDistance() const;
        sal_Int32       GetScrollSize() const;
        sal_Int32       GetSpaceBetween() const;
        void            SetReadOnlyState();
        DECL_LINK_TYPED( CalcHdl, void*, void);

        sw::sidebarwindows::SwSidebarWin* GetSidebarWin(const SfxBroadcaster* pBroadcaster) const;

        void            InsertItem( SfxBroadcaster* pItem, bool bCheckExistance, bool bFocus);
        void            RemoveItem( SfxBroadcaster* pBroadcast );

        void            Sort();

    public:
        SwPostItMgr(SwView* aDoc);
        virtual ~SwPostItMgr();

        typedef std::list< SwSidebarItem* >::const_iterator const_iterator;
        const_iterator begin()  const { return mvPostItFields.begin(); }
        const_iterator end()    const { return mvPostItFields.end();  }

        void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

        void LayoutPostIts();
        bool CalcRects();

        void MakeVisible( const sw::sidebarwindows::SwSidebarWin* pPostIt,
                          long aPage = -1);

        bool ShowScrollbar(const unsigned long aPage) const;
        bool HasNotes() const ;
        bool ShowNotes() const;
        bool IsShowAnchor() { return mbIsShowAnchor;}
        unsigned long GetSidebarWidth(bool bPx = false) const;
        unsigned long GetSidebarBorderWidth(bool bPx = false) const;
        unsigned long GetNoteWidth();

        void PrepareView(bool bIgnoreCount = false);

        void CorrectPositions();

        void SetLayout() { mbLayout = true; };
        void Delete(const OUString& aAuthor);
        void Delete();

        void ExecuteFormatAllDialog(SwView& rView);
        void FormatAll(const SfxItemSet &rNewAttr);

        void Hide( const OUString& rAuthor );
        void Hide();
        void Show();

        void Rescale();

        Rectangle GetBottomScrollRect(const unsigned long aPage) const;
        Rectangle GetTopScrollRect(const unsigned long aPage) const;

        bool IsHit(const Point &aPointPixel);
        /// Get the matching window that is responsible for handling mouse events of rPointLogic, if any.
        vcl::Window* IsHitSidebarWindow(const Point& rPointLogic);
        Color GetArrowColor(sal_uInt16 aDirection,unsigned long aPage) const;

        sw::annotation::SwAnnotationWin* GetAnnotationWin(const SwPostItField* pField) const;

        sw::sidebarwindows::SwSidebarWin* GetNextPostIt( sal_uInt16 aDirection,
                                                         sw::sidebarwindows::SwSidebarWin* aPostIt);
        long GetNextBorder();

        sw::sidebarwindows::SwSidebarWin* GetActiveSidebarWin() { return mpActivePostIt; }
        void SetActiveSidebarWin( sw::sidebarwindows::SwSidebarWin* p);
        bool HasActiveSidebarWin() const;
        bool HasActiveAnnotationWin() const;
        void GrabFocusOnActiveSidebarWin();
        void UpdateDataOnActiveSidebarWin();
        void DeleteActiveSidebarWin();
        void HideActiveSidebarWin();
        void ToggleInsModeOnActiveSidebarWin();

        sal_Int32 GetMinimumSizeWithMeta() const;
        sal_Int32 GetSidebarScrollerHeight() const;

        void SetShadowState(const SwPostItField* pField,bool bCursor = true);

        void SetSpellChecking();

        static Color           GetColorDark(sal_uInt16 aAuthorIndex);
        static Color           GetColorLight(sal_uInt16 aAuthorIndex);
        static Color           GetColorAnchor(sal_uInt16 aAuthorIndex);

        void                RegisterAnswer(OutlinerParaObject* pAnswer) { mpAnswer = pAnswer;}
        OutlinerParaObject* IsAnswer() {return mpAnswer;}
        void CheckMetaText();

        sal_uInt16 Replace(SvxSearchItem* pItem);
        sal_uInt16 SearchReplace(const SwFormatField &pField, const css::util::SearchOptions& rSearchOptions,bool bSrchForward);
        sal_uInt16 FinishSearchReplace(const css::util::SearchOptions& rSearchOptions,bool bSrchForward);

        void AssureStdModeAtShell();

        void ConnectSidebarWinToFrame( const SwFrame& rFrame,
                                     const SwFormatField& rFormatField,
                                     sw::sidebarwindows::SwSidebarWin& rSidebarWin );
        void DisconnectSidebarWinFromFrame( const SwFrame& rFrame,
                                          sw::sidebarwindows::SwSidebarWin& rSidebarWin );
        bool HasFrameConnectedSidebarWins( const SwFrame& rFrame );
        vcl::Window* GetSidebarWinForFrameByIndex( const SwFrame& rFrame,
                                            const sal_Int32 nIndex );
        void GetAllSidebarWinForFrame( const SwFrame& rFrame,
                                     std::vector< vcl::Window* >* pChildren );

        void DrawNotesForPage(OutputDevice *pOutDev, sal_uInt32 nPage);
        void PaintTile(OutputDevice& rRenderContext, const Rectangle& rRect);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
