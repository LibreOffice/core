/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: postithelper.hxx,v $
 * $Revision: 1.2 $
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
#include <vector>

class SwTxtFld;
class SwRootFrm;
class SwFmtFld;
class SwPostIt;

struct SwPosition;

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
        INVISIBLE, VISIBLE, INSERTED, DELETED
    };

    SwLayoutStatus getLayoutInfos( std::vector< SwLayoutInfo >&, SwPosition& );
    SwLayoutStatus getLayoutInfos( std::vector< SwLayoutInfo >&, SwTxtFld* );
    long getLayoutHeight( const SwRootFrm* pRoot );
    void setSidebarChanged( SwRootFrm* pRoot, bool bBrowseMode );
    unsigned long getPageInfo( SwRect& rPageFrm, const SwRootFrm* , const Point& );
}

struct SwPostItItem
{
    bool bShow;
    bool bFocus;
    bool bMarginSide;
    SwFmtFld* pFmtFld;
    SwPostIt* pPostIt;
    SwRect mPos;
    SwRect mFramePos;
    SwRect mPagePos;
    unsigned long mnPageNumber;
    SwPostItHelper::SwLayoutStatus mLayoutStatus;
    USHORT mRedlineAuthor;
    SwPostItItem( SwFmtFld* p, bool aShow, bool aFocus)
        : bShow(aShow),
        bFocus(aFocus),
        bMarginSide(false),
        pFmtFld(p),
        pPostIt(0),
        mnPageNumber(1),
        mLayoutStatus( SwPostItHelper::INVISIBLE ),
        mRedlineAuthor(0)
    {
    }
};

#endif // _POSTITHELPER_HXX
