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


#ifndef _POSTITHELPER_HXX
#define _POSTITHELPER_HXX

#include <swrect.hxx>
#include <fmtfld.hxx>
#include <redline.hxx>
#include <vector>
#include <vcl/window.hxx>
#include <SidebarWindowsTypes.hxx>
#include <svl/brdcst.hxx>

class SwTxtFld;
class SwRootFrm;
class SwPostIt;
class String;
class SwPostItMgr;
class SwEditWin;
namespace sw { namespace sidebarwindows {
    class SwSidebarWin;
} }

struct SwPosition;

typedef sal_Int64 SwPostItBits;

struct SwLayoutInfo
{
    const SwFrm* mpAnchorFrm;
    SwRect mPosition;

    // optional start of the annotation
    sal_uLong mnStartNodeIdx;
    xub_StrLen mnStartContent;

    SwRect mPageFrame;
    SwRect mPagePrtArea;
    unsigned long mnPageNumber;

    sw::sidebarwindows::SidebarPosition meSidebarPosition;

    sal_uInt16 mRedlineAuthor;

    SwLayoutInfo()
        : mpAnchorFrm(0)
        , mPosition()
        , mnStartNodeIdx( 0 )
        , mnStartContent( STRING_NOTFOUND )
        , mPageFrame()
        , mPagePrtArea()
        , mnPageNumber(1)
        , meSidebarPosition(sw::sidebarwindows::SIDEBAR_NONE)
        , mRedlineAuthor(0)
    {}
};

namespace SwPostItHelper
{
    enum SwLayoutStatus
    {
        INVISIBLE, VISIBLE, INSERTED, DELETED, NONE, HIDDEN
    };

    SwLayoutStatus getLayoutInfos(
        SwLayoutInfo& o_rInfo,
        const SwPosition& rAnchorPos,
        const SwPosition* pAnnotationStartPos = NULL );

    long getLayoutHeight( const SwRootFrm* pRoot );
    void setSidebarChanged( SwRootFrm* pRoot, bool bBrowseMode );
    unsigned long getPageInfo( SwRect& rPageFrm, const SwRootFrm* , const Point& );
}

class SwSidebarItem
{
public:
    sw::sidebarwindows::SwSidebarWin* pPostIt;
    bool bShow;
    bool bFocus;

    SwPostItHelper::SwLayoutStatus mLayoutStatus;
    SwLayoutInfo maLayoutInfo;

    SwSidebarItem( const bool aShow,
                   const bool aFocus)
        : pPostIt(0)
        , bShow(aShow)
        , bFocus(aFocus)
        , mLayoutStatus( SwPostItHelper::INVISIBLE )
        , maLayoutInfo()
    {
    }

    virtual ~SwSidebarItem()
    {
    }

    virtual SwPosition GetAnchorPosition() const = 0;
    virtual bool UseElement() = 0;
    virtual const SwFmtFld& GetFmtFld() const = 0;
    virtual const SfxBroadcaster* GetBroadCaster() const = 0;
    virtual sw::sidebarwindows::SwSidebarWin* GetSidebarWindow( SwEditWin& rEditWin,
                                                                WinBits nBits,
                                                                SwPostItMgr& aMgr,
                                                                SwPostItBits aBits) = 0;
};
/*
class SwRedCommentItem: public SwSidebarItem
{
private:
    SwRedline* pRedline;
public:

    SwRedCommentItem( SwRedline* pRed, bool aShow, bool aFocus)
        : SwSidebarItem(aShow,aFocus),
        pRedline(pRed) {}
    virtual ~SwRedCommentItem() {}
    virtual SwPosition GetAnchorPosition() const;
    virtual bool UseElement();
    virtual SwFmtFld* GetFmtFld() const {return 0; }
    virtual SfxBroadcaster* GetBroadCaster() const { return dynamic_cast<SfxBroadcaster *> (pRedline); }
    virtual sw::sidebarwindows::SwSidebarWin* GetSidebarWindow( SwEditWin& rEditWin,
                                                                WinBits nBits,
                                                                SwPostItMgr& aMgr,
                                                                SwPostItBits aBits);
};
*/

class SwAnnotationItem: public SwSidebarItem
{
public:
    SwAnnotationItem(
        SwFmtFld& rFmtFld,
        const bool bShow,
        const bool bFocus)
        : SwSidebarItem( bShow, bFocus )
        , mrFmtFld( rFmtFld )
    {
    }

    virtual ~SwAnnotationItem()
    {
    }

    virtual SwPosition GetAnchorPosition() const;
    virtual bool UseElement();
    virtual const SwFmtFld& GetFmtFld() const
    {
        return mrFmtFld;
    }
    virtual const SfxBroadcaster* GetBroadCaster() const
    {
        return dynamic_cast<const SfxBroadcaster *> (&mrFmtFld);
    }
    virtual sw::sidebarwindows::SwSidebarWin* GetSidebarWindow(
        SwEditWin& rEditWin,
        WinBits nBits,
        SwPostItMgr& aMgr,
        SwPostItBits aBits );

private:
    SwFmtFld& mrFmtFld;
};

#endif // _POSTITHELPER_HXX
