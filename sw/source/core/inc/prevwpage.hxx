/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef _PREVIEWPAGES_HXX
#define _PREVIEWPAGES_HXX

// classes <Point>, <Size> and <Rectangle>
#include <tools/gen.hxx>

class SwPageFrm;

/** data structure for a preview page in the current preview layout
*/
struct PrevwPage
{
    const SwPageFrm*  pPage;
    bool        bVisible;
    Size        aPageSize;
    Point       aPrevwWinPos;
    Point       aLogicPos;
    Point       aMapOffset;

    inline PrevwPage();
};

inline PrevwPage::PrevwPage()
    : pPage( 0 ),
      bVisible( false ),
      aPageSize( Size(0,0) ),
      aPrevwWinPos( Point(0,0) ),
      aLogicPos( Point(0,0) ),
      aMapOffset( Point(0,0) )
{};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
