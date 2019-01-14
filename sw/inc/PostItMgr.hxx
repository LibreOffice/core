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

#include <sal/config.h>
#include "swdllapi.h"

#include <cstddef>
#include <memory>
#include <vector>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <tools/link.hxx>
#include "swrect.hxx"
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "SidebarWindowsTypes.hxx"
#include <svl/lstner.hxx>
#include <vcl/vclptr.hxx>

class OutputDevice;
class SwWrtShell;
class SwView;
class SwPostItField;
class SwFormatField;
class SfxBroadcaster;
class SfxHint;
class SwEditWin;
class Color;
class SfxItemSet;
class SvxSearchItem;
namespace sw { namespace annotation {
    class SwAnnotationWin;
}}
namespace sw { namespace sidebarwindows {
    class SwFrameSidebarWinContainer;
}}
class SwSidebarItem;
class SwFrame;
namespace vcl { class Window; }
struct ImplSVEvent;
class OutlinerParaObject;
namespace i18nutil { struct SearchOptions2; }

#define COL_NOTES_SIDEPANE_ARROW_ENABLED    Color(0,0,0)
#define COL_NOTES_SIDEPANE_ARROW_DISABLED   Color(172,168,153)

struct SwPostItPageItem
{
    bool bScrollbar;
    sw::sidebarwindows::SidebarPosition eSidebarPosition;
    long lOffset;
    SwRect mPageRect;
    std::vector<SwSidebarItem*> mvSidebarItems;
    SwPostItPageItem(): bScrollbar(false), eSidebarPosition( sw::sidebarwindows::SidebarPosition::LEFT ), lOffset(0)
    {
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
        bool m_bIsShowAnchor;

        virtual void ImplCommit() override;

    public:
        SwNoteProps()
            : ConfigItem("Office.Writer/Notes")
            , m_bIsShowAnchor(false)
        {
            const css::uno::Sequence< OUString >& rNames = GetPropertyNames();
            css::uno::Sequence< css::uno::Any > aValues = GetProperties(rNames);
            const css::uno::Any* pValues = aValues.getConstArray();
            SAL_WARN_IF(aValues.getLength() != rNames.getLength(), "sw", "GetProperties failed");
            if (aValues.getLength())
                    pValues[0]>>=m_bIsShowAnchor;
        }

        bool IsShowAnchor()
        {
            return m_bIsShowAnchor;
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
        std::vector<std::unique_ptr<SwSidebarItem>>     mvPostItFields;
        std::vector<std::unique_ptr<SwPostItPageItem>>  mPages;
        ImplSVEvent *                   mnEventId;
        bool                            mbWaitingForCalcRects;
        VclPtr<sw::annotation::SwAnnotationWin> mpActivePostIt;
        bool                            mbLayout;
        long                            mbLayoutHeight;
        bool                            mbLayouting;
        bool                            mbReadOnly;
        bool                            mbDeleteNote;
        FieldShadowState                mShadowState;
        OutlinerParaObject*             mpAnswer;
        OUString                        maAnswerText;
        bool                            mbIsShowAnchor;

        // data structure to collect the <SwAnnotationWin> instances for certain <SwFrame> instances.
        std::unique_ptr<sw::sidebarwindows::SwFrameSidebarWinContainer> mpFrameSidebarWinContainer;

        void            AddPostIts(bool bCheckExistence = true,bool bFocus = true);
        void            RemoveSidebarWin();
        void            PreparePageContainer();
        void            Scroll(const long lScroll,const unsigned long aPage );
        void            AutoScroll(const sw::annotation::SwAnnotationWin* pPostIt,const unsigned long aPage );
        bool            ScrollbarHit(const unsigned long aPage,const Point &aPoint);
        bool            LayoutByPage( std::vector<sw::annotation::SwAnnotationWin*> &aVisiblePostItList,
                                      const tools::Rectangle& rBorder,
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
        DECL_LINK( CalcHdl, void*, void);

        sw::annotation::SwAnnotationWin* GetSidebarWin(const SfxBroadcaster* pBroadcaster) const;

        SwSidebarItem*  InsertItem( SfxBroadcaster* pItem, bool bCheckExistence, bool bFocus);
        void            RemoveItem( SfxBroadcaster* pBroadcast );

    public:
        SwPostItMgr(SwView* aDoc);
        virtual ~SwPostItMgr() override;

        typedef std::vector< std::unique_ptr<SwSidebarItem> >::const_iterator const_iterator;
        const_iterator begin()  const { return mvPostItFields.begin(); }
        const_iterator end()    const { return mvPostItFields.end();  }

        void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

        void LayoutPostIts();
        bool CalcRects();

        void MakeVisible( const sw::annotation::SwAnnotationWin* pPostIt);

        bool ShowScrollbar(const unsigned long aPage) const;
        bool HasNotes() const ;
        bool ShowNotes() const;
        bool IsShowAnchor() { return mbIsShowAnchor;}
        unsigned long GetSidebarWidth(bool bPx = false) const;
        unsigned long GetSidebarBorderWidth(bool bPx = false) const;

        void PrepareView(bool bIgnoreCount = false);

        void CorrectPositions();

        void SetLayout() { mbLayout = true; };
        void Delete(const OUString& aAuthor);
        void Delete(sal_uInt32 nPostItId);
        void Delete();

        void ExecuteFormatAllDialog(SwView& rView);
        void FormatAll(const SfxItemSet &rNewAttr);

        void Hide( const OUString& rAuthor );
        void Hide();
        void Show();

        void Rescale();

        tools::Rectangle GetBottomScrollRect(const unsigned long aPage) const;
        tools::Rectangle GetTopScrollRect(const unsigned long aPage) const;

        bool IsHit(const Point &aPointPixel);
        /// Get the matching window that is responsible for handling mouse events of rPointLogic, if any.
        vcl::Window* IsHitSidebarWindow(const Point& rPointLogic);
        Color GetArrowColor(sal_uInt16 aDirection,unsigned long aPage) const;

        sw::annotation::SwAnnotationWin* GetAnnotationWin(const SwPostItField* pField) const;
        sw::annotation::SwAnnotationWin* GetAnnotationWin(const sal_uInt32 nPostItId) const;

        sw::annotation::SwAnnotationWin* GetNextPostIt( sal_uInt16 aDirection,
                                                        sw::annotation::SwAnnotationWin* aPostIt);
        long GetNextBorder();

        sw::annotation::SwAnnotationWin* GetActiveSidebarWin() { return mpActivePostIt; }
        void SetActiveSidebarWin( sw::annotation::SwAnnotationWin* p);
        SW_DLLPUBLIC bool HasActiveSidebarWin() const;
        bool HasActiveAnnotationWin() const;
        void GrabFocusOnActiveSidebarWin();
        SW_DLLPUBLIC void UpdateDataOnActiveSidebarWin();
        void DeleteActiveSidebarWin();
        void HideActiveSidebarWin();
        void ToggleInsModeOnActiveSidebarWin();

        sal_Int32 GetMinimumSizeWithMeta() const;
        sal_Int32 GetSidebarScrollerHeight() const;

        void SetShadowState(const SwPostItField* pField,bool bCursor = true);

        void SetSpellChecking();

        static Color           GetColorDark(std::size_t aAuthorIndex);
        static Color           GetColorLight(std::size_t aAuthorIndex);
        static Color           GetColorAnchor(std::size_t aAuthorIndex);

        void                RegisterAnswer(OutlinerParaObject* pAnswer) { mpAnswer = pAnswer;}
        OutlinerParaObject* IsAnswer() {return mpAnswer;}
        void                RegisterAnswerText(const OUString& aAnswerText) { maAnswerText = aAnswerText; }
        const OUString&     GetAnswerText() { return maAnswerText; }
        void CheckMetaText();

        sal_uInt16 Replace(SvxSearchItem const * pItem);
        sal_uInt16 SearchReplace(const SwFormatField &pField, const i18nutil::SearchOptions2& rSearchOptions,bool bSrchForward);
        sal_uInt16 FinishSearchReplace(const i18nutil::SearchOptions2& rSearchOptions,bool bSrchForward);

        void AssureStdModeAtShell();

        void ConnectSidebarWinToFrame( const SwFrame& rFrame,
                                     const SwFormatField& rFormatField,
                                     sw::annotation::SwAnnotationWin& rSidebarWin );
        void DisconnectSidebarWinFromFrame( const SwFrame& rFrame,
                                          sw::annotation::SwAnnotationWin& rSidebarWin );
        bool HasFrameConnectedSidebarWins( const SwFrame& rFrame );
        vcl::Window* GetSidebarWinForFrameByIndex( const SwFrame& rFrame,
                                            const sal_Int32 nIndex );
        void GetAllSidebarWinForFrame( const SwFrame& rFrame,
                                     std::vector< vcl::Window* >* pChildren );

        void DrawNotesForPage(OutputDevice *pOutDev, sal_uInt32 nPage);
        void PaintTile(OutputDevice& rRenderContext);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
