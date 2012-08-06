/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
