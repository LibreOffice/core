/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postithelper.hxx,v $
 * $Revision: 1.2.118.3 $
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
#include <svtools/brdcst.hxx>

class SwTxtFld;
class SwRootFrm;
class SwPostIt;
class String;
class SwMarginWin;
class SwPostItMgr;

struct SwPosition;

typedef sal_Int64 SwPostItBits;

struct SwLayoutInfo
{
    SwRect mPosition;
    SwRect mPageFrame;
    SwRect mPagePrtArea;
    unsigned long mnPageNumber;
    bool mbMarginSide;
    USHORT mRedlineAuthor;
};

namespace SwPostItHelper
{
    enum SwLayoutStatus
    {
        INVISIBLE, VISIBLE, INSERTED, DELETED, NONE, HIDDEN
    };

    SwLayoutStatus getLayoutInfos( std::vector< SwLayoutInfo >&, SwPosition& );
    SwLayoutStatus getLayoutInfos( std::vector< SwLayoutInfo >&, SwTxtFld* );
    long getLayoutHeight( const SwRootFrm* pRoot );
    void setSidebarChanged( SwRootFrm* pRoot, bool bBrowseMode );
    unsigned long getPageInfo( SwRect& rPageFrm, const SwRootFrm* , const Point& );
}

class SwMarginItem
{
public:
    SwMarginWin* pPostIt;
    bool bShow;
    bool bFocus;
    bool bMarginSide;
    SwRect mPos;
    SwRect mFramePos;
    SwRect mPagePos;
    unsigned long mnPageNumber;
    SwPostItHelper::SwLayoutStatus mLayoutStatus;
    USHORT mRedlineAuthor;
    SwMarginItem(bool aShow, bool aFocus)
        : pPostIt(0),
        bShow(aShow),
        bFocus(aFocus),
        bMarginSide(false),
        mnPageNumber(1),
        mLayoutStatus( SwPostItHelper::INVISIBLE ),
        mRedlineAuthor(0)
    {}
    virtual ~SwMarginItem(){}
    virtual SwPosition GetPosition() = 0;
    virtual bool UseElement() = 0;
    virtual SwFmtFld* GetFmtFld() = 0;
    virtual SfxBroadcaster* GetBroadCaster() const = 0;
    virtual SwMarginWin* GetMarginWindow(Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits) = 0;
};
/*
class SwRedCommentItem: public SwMarginItem
{
private:
    SwRedline* pRedline;
public:

    SwRedCommentItem( SwRedline* pRed, bool aShow, bool aFocus)
        : SwMarginItem(aShow,aFocus),
        pRedline(pRed) {}
    virtual ~SwRedCommentItem() {}
    virtual SwPosition GetPosition();
    virtual bool UseElement();
    virtual SwFmtFld* GetFmtFld() {return 0; }
    virtual SfxBroadcaster* GetBroadCaster() const { return dynamic_cast<SfxBroadcaster *> (pRedline); }
    virtual SwMarginWin* GetMarginWindow(Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits);
};
*/

class SwPostItItem: public SwMarginItem
{
    private:
    SwFmtFld* pFmtFld;

    public:
    SwPostItItem( SwFmtFld* p, bool aShow, bool aFocus)
        : SwMarginItem(aShow,aFocus) ,
        pFmtFld(p)
    {
    }
    virtual ~SwPostItItem() {}
    virtual SwPosition GetPosition();
    virtual bool UseElement();
    virtual SwFmtFld* GetFmtFld() {return pFmtFld;}
    virtual SfxBroadcaster* GetBroadCaster() const { return dynamic_cast<SfxBroadcaster *> (pFmtFld); }
    virtual SwMarginWin* GetMarginWindow(Window* pParent, WinBits nBits,SwPostItMgr* aMgr,SwPostItBits aBits);
};

#endif // _POSTITHELPER_HXX
