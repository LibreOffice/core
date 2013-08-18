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
    SwRect mPageFrame;
    SwRect mPagePrtArea;
    unsigned long mnPageNumber;
    sw::sidebarwindows::SidebarPosition meSidebarPosition;
    sal_uInt16 mRedlineAuthor;

    SwLayoutInfo()
        : mpAnchorFrm(0)
        , mPosition()
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

    SwLayoutStatus getLayoutInfos( std::vector< SwLayoutInfo >&, SwPosition& );
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
    {}
    virtual ~SwSidebarItem(){}
    virtual SwPosition GetAnchorPosition() const = 0;
    virtual bool UseElement() = 0;
    virtual SwFmtFld* GetFmtFld() const = 0;
    virtual SfxBroadcaster* GetBroadCaster() const = 0;
    virtual sw::sidebarwindows::SwSidebarWin* GetSidebarWindow( SwEditWin& rEditWin,
                                                                WinBits nBits,
                                                                SwPostItMgr& aMgr,
                                                                SwPostItBits aBits) = 0;
};

class SwAnnotationItem: public SwSidebarItem
{
    private:
    SwFmtFld* pFmtFld;

    public:
    SwAnnotationItem( SwFmtFld* p,
                      const bool aShow,
                      const bool aFocus)
        : SwSidebarItem( aShow, aFocus )
        , pFmtFld(p)
    {}
    virtual ~SwAnnotationItem() {}
    virtual SwPosition GetAnchorPosition() const;
    virtual bool UseElement();
    virtual SwFmtFld* GetFmtFld() const {return pFmtFld;}
    virtual SfxBroadcaster* GetBroadCaster() const { return dynamic_cast<SfxBroadcaster *> (pFmtFld); }
    virtual sw::sidebarwindows::SwSidebarWin* GetSidebarWindow( SwEditWin& rEditWin,
                                                                WinBits nBits,
                                                                SwPostItMgr& aMgr,
                                                                SwPostItBits aBits);
};

#endif // _POSTITHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
