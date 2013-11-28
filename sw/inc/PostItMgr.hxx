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



#ifndef _POSTITMGR_HXX
#define _POSTITMGR_HXX

#include <list>
#include <vector>
#include <editeng/outlobj.hxx>
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/debug.hxx>
#include <swrect.hxx>
#include <unotools/configitem.hxx>
#include <unotools/options.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <SidebarWindowsTypes.hxx>
#include <svl/lstner.hxx>

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
namespace sw { namespace annotation {
    class SwAnnotationWin;
}}
namespace sw { namespace sidebarwindows {
    class SwSidebarWin;
    class SwFrmSidebarWinContainer;
}}
class SwSidebarItem;
class SwFrm;
class Window;

#define SORT_POS    1
#define SORT_AUTHOR 2
#define SORT_DATE   3

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
    SwPostItPageItem(): bScrollbar(false),lOffset(0)
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
    const SwPostItField* mpShadowFld;
    bool bCursor;
    bool bMouse;
    FieldShadowState(): mpShadowFld(0),bCursor(false),bMouse(false)
    {
    }
};

class SwNoteProps: public utl::ConfigItem
{
    private:
        bool bIsShowAnchor;
    public:
        SwNoteProps()
            : ConfigItem(::rtl::OUString::createFromAscii("Office.Writer/Notes"))
            , bIsShowAnchor(false)
        {
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rNames = GetPropertyNames();
                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > aValues = GetProperties(rNames);
                const ::com::sun::star::uno::Any* pValues = aValues.getConstArray();
               DBG_ASSERT(aValues.getLength() == rNames.getLength(), "GetProperties failed");
                if (aValues.getLength())
                pValues[0]>>=bIsShowAnchor;
        }

        bool IsShowAnchor()
        {
            return bIsShowAnchor;
        }
            ::com::sun::star::uno::Sequence< ::rtl::OUString >& GetPropertyNames()
            {
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > aNames;
                 if(!aNames.getLength())
                 {
                         static const char* aPropNames[] =
                         {
                            "ShowAnkor"
                           };
                         const int nCount = sizeof(aPropNames)/sizeof(const char*);
                         aNames.realloc(nCount);
                        ::rtl::OUString* pNames = aNames.getArray();
                     for(int i = 0; i < nCount; i++)
                            pNames[i] = ::rtl::OUString::createFromAscii(aPropNames[i]);
                 }
                 return aNames;
            }

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

class SwPostItMgr: public SfxListener
{
    private:
        SwView*                         mpView;
        SwWrtShell*                     mpWrtShell;
        SwEditWin*                      mpEditWin;
        std::list< SwSidebarItem*>      mvPostItFlds;
        std::vector<SwPostItPageItem*>  mPages;
        sal_uLong                           mnEventId;
        bool                            mbWaitingForCalcRects;
        sw::sidebarwindows::SwSidebarWin* mpActivePostIt;
        bool                            mbLayout;
        long                            mbLayoutHeight;
        long                            mbLayouting;
        bool                            mbReadOnly;
        bool                            mbDeleteNote;
        FieldShadowState                mShadowState;
        OutlinerParaObject*             mpAnswer;
        bool                            mbIsShowAnchor;

        // data structure to collect the <SwSidebarWin> instances for certain <SwFrm> instances.
        sw::sidebarwindows::SwFrmSidebarWinContainer* mpFrmSidebarWinContainer;

        typedef std::list<sw::sidebarwindows::SwSidebarWin*>::iterator  SwSidebarWin_iterator;

        void            AddPostIts(bool bCheckExistance = true,bool bFocus = true);
        //void          AddRedlineComments(bool bCheckExistance, bool bFocus);
        void            RemoveSidebarWin();
        void            PreparePageContainer();
        void            Scroll(const long lScroll,const unsigned long aPage );
        void            AutoScroll(const sw::sidebarwindows::SwSidebarWin* pPostIt,const unsigned long aPage );
        bool            ScrollbarHit(const unsigned long aPage,const Point &aPoint);
        bool            LayoutByPage( std::list<sw::sidebarwindows::SwSidebarWin*> &aVisiblePostItList,
                                      const Rectangle aBorder,
                                      long lNeededHeight);
        void            CheckForRemovedPostIts();
            bool                ArrowEnabled(sal_uInt16 aDirection,unsigned long aPage) const;
            bool                BorderOverPageBorder(unsigned long aPage) const;
            bool                HasScrollbars() const;
        void            Focus(SfxBroadcaster& rBC);

            sal_Int32           GetInitialAnchorDistance() const;
            sal_Int32           GetScrollSize() const;
            sal_Int32           GetSpaceBetween() const;
        void            SetReadOnlyState();
                    DECL_LINK( CalcHdl, void*);

        sw::sidebarwindows::SwSidebarWin* GetSidebarWin(const SfxBroadcaster* pBroadcaster) const;

        void InsertItem( SfxBroadcaster* pItem, bool bCheckExistance, bool bFocus);
        void RemoveItem( SfxBroadcaster* pBroadcast );

        void Sort(const short aType);

    public:
            SwPostItMgr(SwView* aDoc);
            ~SwPostItMgr();

            typedef std::list< SwSidebarItem* >::const_iterator const_iterator;
            const_iterator begin()  const { return mvPostItFlds.begin(); }
            const_iterator end()    const { return mvPostItFlds.end();  }

            void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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
            void Delete(String aAuthor);
            void Delete();

#if 0
            void Hide( SwPostItField* pPostItField );
#endif
            void Hide( const String& rAuthor );
            void Hide();
            void Show();

            void Rescale();

            Rectangle GetBottomScrollRect(const unsigned long aPage) const;
            Rectangle GetTopScrollRect(const unsigned long aPage) const;

            bool IsHit(const Point &aPointPixel);
            Color GetArrowColor(sal_uInt16 aDirection,unsigned long aPage) const;

            sw::annotation::SwAnnotationWin* GetAnnotationWin(const SwPostItField* pFld) const;

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

            void SetShadowState(const SwPostItField* pFld,bool bCursor = true);

            void SetSpellChecking();

            Color           GetColorDark(sal_uInt16 aAuthorIndex);
            Color           GetColorLight(sal_uInt16 aAuthorIndex);
            Color           GetColorAnchor(sal_uInt16 aAuthorIndex);


            void                RegisterAnswer(OutlinerParaObject* pAnswer) { mpAnswer = pAnswer;}
            OutlinerParaObject* IsAnswer() {return mpAnswer;}
            void CheckMetaText();
            void StartSpelling();

            sal_uInt16 Replace(SvxSearchItem* pItem);
            sal_uInt16 SearchReplace(const SwFmtFld &pFld, const ::com::sun::star::util::SearchOptions& rSearchOptions,bool bSrchForward);
            sal_uInt16 FinishSearchReplace(const ::com::sun::star::util::SearchOptions& rSearchOptions,bool bSrchForward);

            // get the PostIt window by index
            sal_Int32 GetPostItCount() {return mvPostItFlds.size();};
            void AssureStdModeAtShell();

            void ConnectSidebarWinToFrm( const SwFrm& rFrm,
                                         const SwFmtFld& rFmtFld,
                                         sw::sidebarwindows::SwSidebarWin& rSidebarWin );
            void DisconnectSidebarWinFromFrm( const SwFrm& rFrm,
                                              sw::sidebarwindows::SwSidebarWin& rSidebarWin );
            bool HasFrmConnectedSidebarWins( const SwFrm& rFrm );
            Window* GetSidebarWinForFrmByIndex( const SwFrm& rFrm,
                                                const sal_Int32 nIndex );
            void GetAllSidebarWinForFrm( const SwFrm& rFrm,
                                         std::vector< Window* >* pChildren );
};

#endif




