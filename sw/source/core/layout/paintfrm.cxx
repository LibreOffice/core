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

#include <vcl/lazydelete.hxx>
#include <vcl/gradient.hxx>
#include <sfx2/progress.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/framelink.hxx>
#include <svx/xflgrit.hxx>
#include <tgrditem.hxx>
#include <switerator.hxx>
#include <fmtsrnd.hxx>
#include <fmtclds.hxx>
#include <comcore.hrc>
#include <swmodule.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <viewimp.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <frmtool.hxx>
#include <viewopt.hxx>
#include <dview.hxx>
#include <dcontact.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <notxtfrm.hxx>
#include <layact.hxx>
#include <pagedesc.hxx>
#include <ptqueue.hxx>
#include <noteurl.hxx>
#include <virtoutp.hxx>
#include <lineinfo.hxx>
#include <dbg_lay.hxx>
#include <docsh.hxx>
#include <svx/svdogrp.hxx>
#include <sortedobjs.hxx>
#include <EnhancedPDFExportHelper.hxx>
#include <bodyfrm.hxx>
#include <hffrm.hxx>
#include <colfrm.hxx>
// <--
// --> OD #i76669#
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
// <--

#include <ndole.hxx>
#include <svx/charthelper.hxx>
#include <PostItMgr.hxx>
#include <vcl/svapp.hxx>

#define COL_NOTES_SIDEPANE                  RGB_COLORDATA(230,230,230)
#define COL_NOTES_SIDEPANE_BORDER           RGB_COLORDATA(200,200,200)
#define COL_NOTES_SIDEPANE_SCROLLAREA       RGB_COLORDATA(230,230,220)

#include <svtools/borderhelper.hxx> //Need for svtools::DrawLine

#include "pagefrm.hrc"
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <svx/unoapi.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <vector>
#include <algorithm>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>

using namespace ::editeng;
using namespace ::com::sun::star;
using ::drawinglayer::primitive2d::BorderLinePrimitive2D;
using ::std::pair;
using ::std::make_pair;

//subsidiary lines enabled?
#define IS_SUBS_TABLE \
    (pGlobalShell->GetViewOptions()->IsTable() && \
    !pGlobalShell->GetViewOptions()->IsPagePreview()&&\
    !pGlobalShell->GetViewOptions()->IsReadonly()&&\
    !pGlobalShell->GetViewOptions()->IsFormView() &&\
     SwViewOption::IsTableBoundaries())
//other subsidiary lines enabled?
#define IS_SUBS (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
        !pGlobalShell->GetViewOptions()->IsReadonly() && \
        !pGlobalShell->GetViewOptions()->IsFormView() &&\
         SwViewOption::IsDocBoundaries())
//subsidiary lines for sections
#define IS_SUBS_SECTION (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
                         !pGlobalShell->GetViewOptions()->IsReadonly()&&\
                         !pGlobalShell->GetViewOptions()->IsFormView() &&\
                          SwViewOption::IsSectionBoundaries())
#define IS_SUBS_FLYS (!pGlobalShell->GetViewOptions()->IsPagePreview() && \
                      !pGlobalShell->GetViewOptions()->IsReadonly()&&\
                      !pGlobalShell->GetViewOptions()->IsFormView() &&\
                       SwViewOption::IsObjectBoundaries())

//Class declaration; here because they are only used in this file

#define SUBCOL_PAGE     0x01    //Helplines of the page
#define SUBCOL_TAB      0x08    //Helplines inside tables
#define SUBCOL_FLY      0x10    //Helplines inside fly frames
#define SUBCOL_SECT     0x20    //Helplines inside sections

// Classes collecting the border lines and help lines
class SwLineRect : public SwRect
{
    Color aColor;
    SvxBorderStyle  nStyle;
    const SwTabFrm *pTab;
          sal_uInt8     nSubColor;  //colorize subsidiary lines
          bool      bPainted;   //already painted?
          sal_uInt8     nLock;      //To distinguish the line and the hell layer.
public:
    SwLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderStyle nStyle,
                const SwTabFrm *pT , const sal_uInt8 nSCol );

    const Color         *GetColor() const { return &aColor;}
    SvxBorderStyle       GetStyle() const { return nStyle; }
    const SwTabFrm      *GetTab()   const { return pTab;  }
    void  SetPainted()                    { bPainted = true; }
    void  Lock( sal_Bool bLock )              { if ( bLock )
                                                ++nLock;
                                            else if ( nLock )
                                                --nLock;
                                          }
    bool  IsPainted()               const { return bPainted; }
    bool  IsLocked()                const { return nLock != 0;  }
    sal_uInt8  GetSubColor()                const { return nSubColor;}

    bool MakeUnion( const SwRect &rRect );
};

class SwLineRects : public std::vector< SwLineRect >
{
    size_t nLastCount;  //avoid unnecessary cycles in PaintLines
public:
    SwLineRects() : nLastCount( 0 ) {}
    void AddLineRect( const SwRect& rRect,  const Color *pColor, const SvxBorderStyle nStyle,
                      const SwTabFrm *pTab, const sal_uInt8 nSCol );
    void ConnectEdges( OutputDevice *pOut );
    void PaintLines  ( OutputDevice *pOut );
    void LockLines( sal_Bool bLock );

    //Limit lines to 100
    bool isFull() const { return this->size()>100 ? true : false; }
};

class SwSubsRects : public SwLineRects
{
    void RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects );
public:
    void PaintSubsidiary( OutputDevice *pOut, const SwLineRects *pRects );

    inline void Ins( const SwRect &rRect, const sal_uInt8 nSCol );
};

class BorderLines
{
    typedef ::comphelper::SequenceAsVector<
        ::rtl::Reference<BorderLinePrimitive2D> > Lines_t;
    Lines_t m_Lines;
public:
    void AddBorderLine(::rtl::Reference<BorderLinePrimitive2D> const& xLine);
    drawinglayer::primitive2d::Primitive2DSequence GetBorderLines_Clear()
    {
        ::comphelper::SequenceAsVector<
            ::drawinglayer::primitive2d::Primitive2DReference> lines;
        for (Lines_t::const_iterator it = m_Lines.begin(); it != m_Lines.end();
                ++it)
        {
            lines.push_back(it->get());
        }
        m_Lines.clear();
        return lines.getAsConstList();
    }
};

//----------------- End of classes for border lines ----------------------

static ViewShell *pGlobalShell = 0;

//Only repaint the Fly content as well as the background of the Fly content if
//a metafile is taken of the Fly.
static sal_Bool bFlyMetafile = sal_False;
static OutputDevice *pFlyMetafileOut = 0;

//Retouch for transparent Flys is done by the background of the Flys.
//The Fly itself should certainly not be spared out. See PaintBackground and
//lcl_SubtractFlys()
static SwFlyFrm *pRetoucheFly  = 0;
static SwFlyFrm *pRetoucheFly2 = 0;

// Sizes of a pixel and the corresponding halves. Will be reset when
// entering SwRootFrm::Paint
static long nPixelSzW = 0, nPixelSzH = 0;
static long nHalfPixelSzW = 0, nHalfPixelSzH = 0;
static long nMinDistPixelW = 0, nMinDistPixelH = 0;

// Current zoom factor
static double aScaleX = 1.0;
static double aScaleY = 1.0;
static double aMinDistScale = 0.73;
static double aEdgeScale = 0.5;

// The borders will be collected in pLines during the Paint and later
// possibly merge them.
// The help lines will be collected and merged in pSubsLines. These will
// be compared with pLines before the work in order to avoid help lines
// to hide borders.
// bTablines is sal_True during the Paint of a table.
static BorderLines *g_pBorderLines = 0;
static SwLineRects *pLines = 0;
static SwSubsRects *pSubsLines = 0;
// global variable for sub-lines of body, header, footer, section and footnote frames.
static SwSubsRects *pSpecSubsLines = 0;

static SfxProgress *pProgress = 0;

static SwFlyFrm *pFlyOnlyDraw = 0;

//So the flys can also be painted right for the hack.
static sal_Bool bTableHack = sal_False;

//To optimize the expensive RetouchColor determination
Color aGlobalRetoucheColor;

// Set borders alignment statics.
// adjustment for 'small' twip-to-pixel relations:
// For 'small' twip-to-pixel relations (less then 2:1)
// values of <nHalfPixelSzW> and <nHalfPixelSzH> are set to ZERO.
void SwCalcPixStatics( OutputDevice *pOut )
{
    // determine 'small' twip-to-pixel relation
    sal_Bool bSmallTwipToPxRelW = sal_False;
    sal_Bool bSmallTwipToPxRelH = sal_False;
    {
        Size aCheckTwipToPxRelSz( pOut->PixelToLogic( Size( 100, 100 )) );
        if ( (aCheckTwipToPxRelSz.Width()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelW = sal_True;
        }
        if ( (aCheckTwipToPxRelSz.Height()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelH = sal_True;
        }
    }

    Size aSz( pOut->PixelToLogic( Size( 1,1 )) );

    nPixelSzW = aSz.Width();
    if( !nPixelSzW )
        nPixelSzW = 1;
    nPixelSzH = aSz.Height();
    if( !nPixelSzH )
        nPixelSzH = 1;

    // consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelW )
    {
        nHalfPixelSzW = nPixelSzW / 2 + 1;
    }
    else
    {
        nHalfPixelSzW = 0;
    }
    // consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelH )
    {
        nHalfPixelSzH = nPixelSzH / 2 + 1;
    }
    else
    {
        nHalfPixelSzH = 0;
    }

    nMinDistPixelW = nPixelSzW * 2 + 1;
    nMinDistPixelH = nPixelSzH * 2 + 1;

    const MapMode &rMap = pOut->GetMapMode();
    aScaleX = rMap.GetScaleX();
    aScaleY = rMap.GetScaleY();
}

//To be able to save the statics so the paint is more or lees reentrant.
class SwSavePaintStatics
{
    sal_Bool            bSFlyMetafile;
    ViewShell          *pSGlobalShell;
    OutputDevice       *pSFlyMetafileOut;
    SwFlyFrm           *pSRetoucheFly,
                       *pSRetoucheFly2,
                       *pSFlyOnlyDraw;
    BorderLines        *pBLines;
    SwLineRects        *pSLines;
    SwSubsRects        *pSSubsLines;
    SwSubsRects*        pSSpecSubsLines;
    SfxProgress        *pSProgress;
    long                nSPixelSzW,
                        nSPixelSzH,
                        nSHalfPixelSzW,
                        nSHalfPixelSzH,
                        nSMinDistPixelW,
                        nSMinDistPixelH;
    Color               aSGlobalRetoucheColor;
    double              aSScaleX,
                        aSScaleY;
public:
    SwSavePaintStatics();
    ~SwSavePaintStatics();
};

SwSavePaintStatics::SwSavePaintStatics() :
    bSFlyMetafile       ( bFlyMetafile      ),
    pSGlobalShell       ( pGlobalShell      ),
    pSFlyMetafileOut    ( pFlyMetafileOut   ),
    pSRetoucheFly       ( pRetoucheFly      ),
    pSRetoucheFly2      ( pRetoucheFly2     ),
    pSFlyOnlyDraw       ( pFlyOnlyDraw      ),
    pBLines             ( g_pBorderLines    ),
    pSLines             ( pLines            ),
    pSSubsLines         ( pSubsLines        ),
    pSSpecSubsLines     ( pSpecSubsLines    ),
    pSProgress          ( pProgress         ),
    nSPixelSzW          ( nPixelSzW         ),
    nSPixelSzH          ( nPixelSzH         ),
    nSHalfPixelSzW      ( nHalfPixelSzW     ),
    nSHalfPixelSzH      ( nHalfPixelSzH     ),
    nSMinDistPixelW     ( nMinDistPixelW    ),
    nSMinDistPixelH     ( nMinDistPixelH    ),
    aSGlobalRetoucheColor( aGlobalRetoucheColor ),
    aSScaleX            ( aScaleX           ),
    aSScaleY            ( aScaleY           )
{
    bFlyMetafile = sal_False;
    pFlyMetafileOut = 0;
    pRetoucheFly  = 0;
    pRetoucheFly2 = 0;
    nPixelSzW = nPixelSzH =
    nHalfPixelSzW = nHalfPixelSzH =
    nMinDistPixelW = nMinDistPixelH = 0;
    aScaleX = aScaleY = 1.0;
    aMinDistScale = 0.73;
    aEdgeScale = 0.5;
    g_pBorderLines = 0;
    pLines = 0;
    pSubsLines = 0;
    pSpecSubsLines = 0L;
    pProgress = 0;
}

SwSavePaintStatics::~SwSavePaintStatics()
{
    pGlobalShell       = pSGlobalShell;
    bFlyMetafile       = bSFlyMetafile;
    pFlyMetafileOut    = pSFlyMetafileOut;
    pRetoucheFly       = pSRetoucheFly;
    pRetoucheFly2      = pSRetoucheFly2;
    pFlyOnlyDraw       = pSFlyOnlyDraw;
    g_pBorderLines     = pBLines;
    pLines             = pSLines;
    pSubsLines         = pSSubsLines;
    pSpecSubsLines     = pSSpecSubsLines;
    pProgress          = pSProgress;
    nPixelSzW          = nSPixelSzW;
    nPixelSzH          = nSPixelSzH;
    nHalfPixelSzW      = nSHalfPixelSzW;
    nHalfPixelSzH      = nSHalfPixelSzH;
    nMinDistPixelW     = nSMinDistPixelW;
    nMinDistPixelH     = nSMinDistPixelH;
    aGlobalRetoucheColor = aSGlobalRetoucheColor;
    aScaleX            = aSScaleX;
    aScaleY            = aSScaleY;
}

//----------------- Implementation for the table borders --------------

/**
 * Check whether the two primitve can be merged
 *
 * @param[in]   mergeA  A primitive start and end position
 * @param[in]   mergeB  B primitive start and end position
 * @return      1       if A and B can be merged to a primite staring with A, ending with B
 *              2       if A and B can be merged to a primite staring with B, ending with A
 *              0       if A and B can't be merged
**/
static sal_uInt8 lcl_TryMergeLines(
    pair<double, double> const mergeA,
    pair<double, double> const mergeB)
{
    double const fMergeGap(nPixelSzW + nHalfPixelSzW); // NOT static!
    // A is above/before B
    if( mergeA.second <= mergeB.second &&
        mergeA.second + fMergeGap >= mergeB.first )
    {
        return 1;
    }
    // B is above/before A
    else if( mergeB.second <= mergeA.second &&
             mergeB.second + fMergeGap >= mergeA.first )
    {
        return 2;
    }
    return 0;
}

/**
 * Make a new primitive from the two input borderline primitive
 *
 * @param[in]   rLine       starting primitive
 * @param[in]   rOther      ending primitive
 * @param[in]   rStart      starting point of merged primitive
 * @param[in]   rEnd        ending point of merged primitive
 * @return      merged primitive
**/
static ::rtl::Reference<BorderLinePrimitive2D>
lcl_MergeBorderLines(
    BorderLinePrimitive2D const& rLine, BorderLinePrimitive2D const& rOther,
    basegfx::B2DPoint const& rStart, basegfx::B2DPoint const& rEnd)
{
    return new BorderLinePrimitive2D(rStart, rEnd,
                rLine.getLeftWidth(),
                rLine.getDistance(),
                rLine.getRightWidth(),
                rLine.getExtendLeftStart(),
                rOther.getExtendLeftEnd(),
                rLine.getExtendRightStart(),
                rOther.getExtendRightEnd(),
                rLine.getRGBColorLeft(),
                rLine.getRGBColorGap(),
                rLine.getRGBColorRight(),
                rLine.hasGapColor(),
                rLine.getStyle());
}

/**
 * Merge the two borderline if possible.
 *
 * @param[in]   rThis   one borderline primitive
 * @param[in]   rOther  other borderline primitive
 * @return      merged borderline including the two input primitive, if they can be merged
 *              0, otherwise
**/
static ::rtl::Reference<BorderLinePrimitive2D>
lcl_TryMergeBorderLine(BorderLinePrimitive2D const& rThis,
                       BorderLinePrimitive2D const& rOther)
{
    assert(rThis.getEnd().getX() >= rThis.getStart().getX());
    assert(rThis.getEnd().getY() >= rThis.getStart().getY());
    assert(rOther.getEnd().getX() >= rOther.getStart().getX());
    assert(rOther.getEnd().getY() >= rOther.getStart().getY());
    double thisHeight = rThis.getEnd().getY() - rThis.getStart().getY();
    double thisWidth  = rThis.getEnd().getX() - rThis.getStart().getX();
    double otherHeight = rOther.getEnd().getY() -  rOther.getStart().getY();
    double otherWidth  = rOther.getEnd().getX() -  rOther.getStart().getX();
    // check for same orientation, same line width, same style and matching colors
    if (    ((thisHeight > thisWidth) == (otherHeight > otherWidth))
        &&  (rThis.getLeftWidth()     == rOther.getLeftWidth())
        &&  (rThis.getDistance()      == rOther.getDistance())
        &&  (rThis.getRightWidth()    == rOther.getRightWidth())
        &&  (rThis.getStyle()         == rOther.getStyle())
        &&  (rThis.getRGBColorLeft()  == rOther.getRGBColorLeft())
        &&  (rThis.getRGBColorRight() == rOther.getRGBColorRight())
        &&  (rThis.hasGapColor()      == rOther.hasGapColor())
        &&  (!rThis.hasGapColor() ||
             (rThis.getRGBColorGap()  == rOther.getRGBColorGap())))
    {
        int nRet = 0;
        if (thisHeight > thisWidth) // vertical line
        {
            if (rThis.getStart().getX() == rOther.getStart().getX())
            {
                assert(rThis.getEnd().getX() == rOther.getEnd().getX());
                nRet = lcl_TryMergeLines(
                    make_pair(rThis.getStart().getY(), rThis.getEnd().getY()),
                    make_pair(rOther.getStart().getY(),rOther.getEnd().getY()));
            }
        }
        else // horizontal line
        {
            if (rThis.getStart().getY() == rOther.getStart().getY())
            {
                assert(rThis.getEnd().getY() == rOther.getEnd().getY());
                nRet = lcl_TryMergeLines(
                    make_pair(rThis.getStart().getX(), rThis.getEnd().getX()),
                    make_pair(rOther.getStart().getX(),rOther.getEnd().getX()));
            }
        }

        // The merged primitive starts with rThis and ends with rOther
        if (nRet == 1)
        {
            basegfx::B2DPoint const start(
                rThis.getStart().getX(), rThis.getStart().getY());
            basegfx::B2DPoint const end(
                rOther.getEnd().getX(), rOther.getEnd().getY());
            return lcl_MergeBorderLines(rThis, rOther, start, end);
        }
        // The merged primitive starts with rOther and ends with rThis
        else if(nRet == 2)
        {
            basegfx::B2DPoint const start(
                rOther.getStart().getX(), rOther.getStart().getY());
            basegfx::B2DPoint const end(
                rThis.getEnd().getX(), rThis.getEnd().getY());
            return lcl_MergeBorderLines(rOther, rThis, start, end);
        }
    }
    return 0;
}

void BorderLines::AddBorderLine(
        rtl::Reference<BorderLinePrimitive2D> const& xLine)
{
    for (Lines_t::reverse_iterator it = m_Lines.rbegin(); it != m_Lines.rend();
         ++it)
    {
        ::rtl::Reference<BorderLinePrimitive2D> const xMerged =
            lcl_TryMergeBorderLine(**it, *xLine);
        if (xMerged.is())
        {
            *it = xMerged; // replace existing line with merged
            return;
        }
    }
    m_Lines.push_back(xLine);
}


SwLineRect::SwLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderStyle nStyl,
                        const SwTabFrm *pT, const sal_uInt8 nSCol ) :
    SwRect( rRect ),
    nStyle( nStyl ),
    pTab( pT ),
    nSubColor( nSCol ),
    bPainted( false ),
    nLock( 0 )
{
    if ( pCol != NULL )
        aColor = *pCol;
}

bool SwLineRect::MakeUnion( const SwRect &rRect )
{
    // It has already been tested outside, whether the rectangles have
    // the same orientation (horizontal or vertical), color, etc.
    if ( Height() > Width() ) //Vertical line
    {
        if ( Left()  == rRect.Left() && Width() == rRect.Width() )
        {
            // Merge when there is no gap between the lines
            const long nAdd = nPixelSzW + nHalfPixelSzW;
            if ( Bottom() + nAdd >= rRect.Top() &&
                 Top()    - nAdd <= rRect.Bottom()  )
            {
                Bottom( std::max( Bottom(), rRect.Bottom() ) );
                Top   ( std::min( Top(),    rRect.Top()    ) );
                return true;
            }
        }
    }
    else
    {
        if ( Top()  == rRect.Top() && Height() == rRect.Height() )
        {
            // Merge when there is no gap between the lines
            const long nAdd = nPixelSzW + nHalfPixelSzW;
            if ( Right() + nAdd >= rRect.Left() &&
                 Left()  - nAdd <= rRect.Right() )
            {
                Right( std::max( Right(), rRect.Right() ) );
                Left ( std::min( Left(),  rRect.Left()  ) );
                return true;
            }
        }
    }
    return false;
}

void SwLineRects::AddLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderStyle nStyle,
                               const SwTabFrm *pTab, const sal_uInt8 nSCol )
{
    //Rueckwaerts durch, weil Linien die zusammengefasst werden koennen i.d.R.
    //im gleichen Kontext gepaintet werden.

    //Loop backwards because lines which can be combined, can usually be painted
    //in the same context.

    for (SwLineRects::reverse_iterator it = this->rbegin(); it != this->rend();
         ++it)
    {
        SwLineRect &rLRect = (*it);
        // Test for the orientation, color, table
        if ( rLRect.GetTab() == pTab &&
             !rLRect.IsPainted() && rLRect.GetSubColor() == nSCol &&
             (rLRect.Height() > rLRect.Width()) == (rRect.Height() > rRect.Width()) &&
             ((!rLRect.GetColor() && !pCol) ||
              (rLRect.GetColor() && pCol && *rLRect.GetColor() == *pCol)) )
        {
            if ( rLRect.MakeUnion( rRect ) )
                return;
        }
    }
    this->push_back( SwLineRect( rRect, pCol, nStyle, pTab, nSCol ) );
}

void SwLineRects::ConnectEdges( OutputDevice *pOut )
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        // I'm not doing anything for a too small zoom
        if ( aScaleX < aEdgeScale || aScaleY < aEdgeScale )
            return;
    }

    static const long nAdd = 20;

    std::vector<SwLineRect*> aCheck;

    for (size_t i = 0; i < this->size(); ++i)
    {
        SwLineRect &rL1 = (*this)[i];
        if ( !rL1.GetTab() || rL1.IsPainted() || rL1.IsLocked() )
            continue;

        aCheck.clear();

        const bool bVert = rL1.Height() > rL1.Width();
        long nL1a, nL1b, nL1c, nL1d;

        if ( bVert )
        {
            nL1a = rL1.Top();   nL1b = rL1.Left();
            nL1c = rL1.Right(); nL1d = rL1.Bottom();
        }
        else
        {
            nL1a = rL1.Left();   nL1b = rL1.Top();
            nL1c = rL1.Bottom(); nL1d = rL1.Right();
        }

        // Collect all lines to possibly link with i1
        for (SwLineRects::iterator it2 = this->begin(); it2 != this->end(); ++it2)
        {
            SwLineRect &rL2 = (*it2);
            if ( rL2.GetTab() != rL1.GetTab() ||
                 rL2.IsPainted()              ||
                 rL2.IsLocked()               ||
                 (bVert == (rL2.Height() > rL2.Width())) )
                continue;

            long nL2a, nL2b, nL2c, nL2d;
            if ( bVert )
            {
                nL2a = rL2.Top();   nL2b = rL2.Left();
                nL2c = rL2.Right(); nL2d = rL2.Bottom();
            }
            else
            {
                nL2a = rL2.Left();   nL2b = rL2.Top();
                nL2c = rL2.Bottom(); nL2d = rL2.Right();
            }

            if ( (nL1a - nAdd < nL2d && nL1d + nAdd > nL2a) &&
                  ((nL1b >  nL2b && nL1c        < nL2c) ||
                   (nL1c >= nL2c && nL1b - nAdd < nL2c) ||
                   (nL1b <= nL2b && nL1c + nAdd > nL2b)) )
            {
                aCheck.push_back( &rL2 );
            }
        }
        if ( aCheck.size() < 2 )
            continue;

        bool bRemove = false;

        // For each line test all following ones.
        for ( sal_uInt16 k = 0; !bRemove && k < aCheck.size(); ++k )
        {
            SwLineRect &rR1 = *aCheck[k];

            for ( sal_uInt16 k2 = k+1; !bRemove && k2 < aCheck.size(); ++k2 )
            {
                SwLineRect &rR2 = *aCheck[k2];
                if ( bVert )
                {
                    SwLineRect *pLA = 0;
                    SwLineRect *pLB = 0;
                    if ( rR1.Top() < rR2.Top() )
                    {
                        pLA = &rR1; pLB = &rR2;
                    }
                    else if ( rR1.Top() > rR2.Top() )
                    {
                        pLA = &rR2; pLB = &rR1;
                    }
                    // are k1 and k2 describing a double line?
                    if ( pLA && pLA->Bottom() + 60 > pLB->Top() )
                    {
                        if ( rL1.Top() < pLA->Top() )
                        {
                            if ( rL1.Bottom() == pLA->Bottom() )
                                continue;    //Small mistake (where?)

                            SwRect aIns( rL1 );
                            aIns.Bottom( pLA->Bottom() );
                            if ( !rL1.IsInside( aIns ) )
                                continue;
                            this->push_back( SwLineRect( aIns, rL1.GetColor(),
                                        table::BorderLineStyle::SOLID,
                                        rL1.GetTab(), SUBCOL_TAB ) );
                            if ( isFull() )
                            {
                                --i;
                                k = aCheck.size();
                                break;
                            }
                        }

                        if ( rL1.Bottom() > pLB->Bottom() )
                            rL1.Top( pLB->Top() );  // extend i1 on the top
                        else
                            bRemove = true;     //stopping, remove i1
                    }
                }
                else
                {
                    SwLineRect *pLA = 0;
                    SwLineRect *pLB = 0;
                    if ( rR1.Left() < rR2.Left() )
                    {
                        pLA = &rR1; pLB = &rR2;
                    }
                    else if ( rR1.Left() > rR2.Left() )
                    {
                        pLA = &rR2; pLB = &rR1;
                    }
                    // Is it double line?
                    if ( pLA && pLA->Right() + 60 > pLB->Left() )
                    {
                        if ( rL1.Left() < pLA->Left() )
                        {
                            if ( rL1.Right() == pLA->Right() )
                                continue;    //small error

                            SwRect aIns( rL1 );
                            aIns.Right( pLA->Right() );
                            if ( !rL1.IsInside( aIns ) )
                                continue;
                            this->push_back( SwLineRect( aIns, rL1.GetColor(),
                                        table::BorderLineStyle::SOLID,
                                        rL1.GetTab(), SUBCOL_TAB ) );
                            if ( isFull() )
                            {
                                --i;
                                k = aCheck.size();
                                break;
                            }
                        }
                        if ( rL1.Right() > pLB->Right() )
                            rL1.Left( pLB->Left() );
                        else
                            bRemove = true;
                    }
                }
            }
        }
        if ( bRemove )
        {
            this->erase(this->begin() + i);
            --i;
        }
    }
}

inline void SwSubsRects::Ins( const SwRect &rRect, const sal_uInt8 nSCol )
{
    // Lines that are shorted than the largest line width won't be inserted
    if ( rRect.Height() > DEF_LINE_WIDTH_4 || rRect.Width() > DEF_LINE_WIDTH_4 )
    {
        this->push_back(
            SwLineRect(rRect, 0, table::BorderLineStyle::SOLID, 0, nSCol));
    }
}

void SwSubsRects::RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects )
{
    // All help lines that are covered by any border will be removed or split

    for (size_t i = 0; i < this->size(); ++i)
    {
        // get a copy instead of a reference, because an <insert> may destroy
        // the object due to a necessary array resize.
        const SwLineRect aSubsLineRect = SwLineRect((*this)[i]);

        // add condition <aSubsLineRect.IsLocked()> in order to consider only
        // border lines, which are *not* locked.
        if ( aSubsLineRect.IsPainted() ||
             aSubsLineRect.IsLocked() )
            continue;

        const bool bVerticalSubs = aSubsLineRect.Height() > aSubsLineRect.Width();
        SwRect aSubsRect( aSubsLineRect );
        if ( bVerticalSubs )
        {
            aSubsRect.Left  ( aSubsRect.Left()  - (nPixelSzW+nHalfPixelSzW) );
            aSubsRect.Right ( aSubsRect.Right() + (nPixelSzW+nHalfPixelSzW) );
        }
        else
        {
            aSubsRect.Top   ( aSubsRect.Top()    - (nPixelSzH+nHalfPixelSzH) );
            aSubsRect.Bottom( aSubsRect.Bottom() + (nPixelSzH+nHalfPixelSzH) );
        }
        for (SwLineRects::const_iterator itK = rRects.begin(); itK != rRects.end(); ++itK)
        {
            const SwLineRect &rLine = *itK;

            // do *not* consider painted or locked border lines.
            // #i1837# - locked border lines have to be considered.
            if ( rLine.IsLocked () )
                continue;

            if ( !bVerticalSubs == ( rLine.Height() > rLine.Width() ) ) //same direction?
                continue;

            if ( aSubsRect.IsOver( rLine ) )
            {
                if ( bVerticalSubs ) // Vertical?
                {
                    if ( aSubsRect.Left()  <= rLine.Right() &&
                         aSubsRect.Right() >= rLine.Left() )
                    {
                        long nTmp = rLine.Top()-(nPixelSzH+1);
                        if ( aSubsLineRect.Top() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Bottom( nTmp );
                            this->push_back( SwLineRect( aNewSubsRect, 0, aSubsLineRect.GetStyle(), 0,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        nTmp = rLine.Bottom()+nPixelSzH+1;
                        if ( aSubsLineRect.Bottom() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Top( nTmp );
                            this->push_back( SwLineRect( aNewSubsRect, 0, aSubsLineRect.GetStyle(), 0,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        this->erase(this->begin() + i);
                        --i;
                        break;
                    }
                }
                else                                    //horizontal
                {
                    if ( aSubsRect.Top() <= rLine.Bottom() &&
                         aSubsRect.Bottom() >= rLine.Top() )
                    {
                        long nTmp = rLine.Left()-(nPixelSzW+1);
                        if ( aSubsLineRect.Left() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Right( nTmp );
                            this->push_back( SwLineRect( aNewSubsRect, 0, aSubsLineRect.GetStyle(), 0,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        nTmp = rLine.Right()+nPixelSzW+1;
                        if ( aSubsLineRect.Right() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Left( nTmp );
                            this->push_back(  SwLineRect( aNewSubsRect, 0, aSubsLineRect.GetStyle(), 0,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        this->erase(this->begin() + i);
                        --i;
                        break;
                    }
                }
            }
        }
    }
}

void SwLineRects::LockLines( sal_Bool bLock )
{
    for (SwLineRects::iterator it = this->begin(); it != this->end(); ++it)
       (*it).Lock( bLock );
}

static void lcl_DrawDashedRect( OutputDevice * pOut, SwLineRect & rLRect )
{
    double nHalfLWidth = rLRect.Height(  );
    if ( nHalfLWidth > 1 )
    {
        nHalfLWidth = nHalfLWidth / 2;
    }
    else
    {
        nHalfLWidth = 1;
    }

    long startX = rLRect.Left(  );
    long startY = rLRect.Top(  ) + static_cast<long>(nHalfLWidth);
    long endX = rLRect.Left(  ) + rLRect.Width(  );
    long endY = rLRect.Top(  ) + static_cast<long>(nHalfLWidth);

    if ( rLRect.Height(  ) > rLRect.Width(  ) )
    {
        nHalfLWidth = rLRect.Width(  );
        if ( nHalfLWidth > 1 )
        {
            nHalfLWidth = nHalfLWidth / 2;
        }
        else
        {
            nHalfLWidth = 1;
        }
        startX = rLRect.Left(  ) + static_cast<long>(nHalfLWidth);
        startY = rLRect.Top(  );
        endX = rLRect.Left(  ) + static_cast<long>(nHalfLWidth);
        endY = rLRect.Top(  ) + rLRect.Height(  );
    }

    svtools::DrawLine( *pOut, Point( startX, startY ), Point( endX, endY ),
            sal_uInt32( nHalfLWidth * 2 ), rLRect.GetStyle( ) );
}

void SwLineRects::PaintLines( OutputDevice *pOut )
{
    // Paint the borders. Sadly two passes are needed.
    // Once for the inside and once for the outside edges of tables
    if ( this->size() != nLastCount )
    {
        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pOut );

        pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
        pOut->SetFillColor();
        pOut->SetLineColor();
        ConnectEdges( pOut );
        const Color *pLast = 0;

        bool bPaint2nd = false;
        size_t nMinCount = this->size();

        for ( size_t i = 0; i < size(); ++i )
        {
            SwLineRect &rLRect = operator[](i);

            if ( rLRect.IsPainted() )
                continue;

            if ( rLRect.IsLocked() )
            {
                nMinCount = std::min( nMinCount, i );
                continue;
            }

            // Paint it now or in the second pass?
            bool bPaint = true;
            if ( rLRect.GetTab() )
            {
                if ( rLRect.Height() > rLRect.Width() )
                {
                    // Vertical edge, overlapping with the table edge?
                    SwTwips nLLeft  = rLRect.Left()  - 30,
                            nLRight = rLRect.Right() + 30,
                            nTLeft  = rLRect.GetTab()->Frm().Left() + rLRect.GetTab()->Prt().Left(),
                            nTRight = rLRect.GetTab()->Frm().Left() + rLRect.GetTab()->Prt().Right();
                    if ( (nTLeft >= nLLeft && nTLeft <= nLRight) ||
                         (nTRight>= nLLeft && nTRight<= nLRight) )
                        bPaint = false;
                }
                else
                {
                    // Horizontal edge, overlapping with the table edge?
                    SwTwips nLTop    = rLRect.Top()    - 30,
                            nLBottom = rLRect.Bottom() + 30,
                            nTTop    = rLRect.GetTab()->Frm().Top()  + rLRect.GetTab()->Prt().Top(),
                            nTBottom = rLRect.GetTab()->Frm().Top()  + rLRect.GetTab()->Prt().Bottom();
                    if ( (nTTop    >= nLTop && nTTop      <= nLBottom) ||
                         (nTBottom >= nLTop && nTBottom <= nLBottom) )
                        bPaint = false;
                }
            }
            if ( bPaint )
            {
                if ( !pLast || *pLast != *rLRect.GetColor() )
                {
                    pLast = rLRect.GetColor();

                    sal_uLong nOldDrawMode = pOut->GetDrawMode();
                    if( pGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                        pOut->SetDrawMode( 0 );

                    pOut->SetLineColor( *pLast );
                    pOut->SetFillColor( *pLast );
                    pOut->SetDrawMode( nOldDrawMode );
                }

                if( !rLRect.IsEmpty() )
                    lcl_DrawDashedRect( pOut, rLRect );
                rLRect.SetPainted();
            }
            else
                bPaint2nd = true;
        }
        if ( bPaint2nd )
        {
            for ( size_t i = 0; i < size(); ++i )
            {
                SwLineRect &rLRect = operator[](i);
                if ( rLRect.IsPainted() )
                    continue;

                if ( rLRect.IsLocked() )
                {
                    nMinCount = std::min( nMinCount, i );
                    continue;
                }

                if ( !pLast || *pLast != *rLRect.GetColor() )
                {
                    pLast = rLRect.GetColor();

                    sal_uLong nOldDrawMode = pOut->GetDrawMode();
                    if( pGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    {
                        pOut->SetDrawMode( 0 );
                    }

                    pOut->SetFillColor( *pLast );
                    pOut->SetDrawMode( nOldDrawMode );
                }
                if( !rLRect.IsEmpty() )
                    lcl_DrawDashedRect( pOut, rLRect );
                rLRect.SetPainted();
            }
        }
        nLastCount = nMinCount;
        pOut->Pop();
    }
}

void SwSubsRects::PaintSubsidiary( OutputDevice *pOut,
                                   const SwLineRects *pRects )
{
    if ( !this->empty() )
    {
        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pOut );

        // Remove all help line that are almost covered (tables)
        for (SwSubsRects::size_type i = 0; i != this->size(); ++i)
        {
            SwLineRect &rLi = (*this)[i];
            const bool bVerticalSubs = rLi.Height() > rLi.Width();

            for (SwSubsRects::size_type k = i + 1; k != this->size(); ++k)
            {
                SwLineRect &rLk = (*this)[k];
                if ( rLi.SSize() == rLk.SSize() )
                {
                    if ( bVerticalSubs == ( rLk.Height() > rLk.Width() ) )
                    {
                        if ( bVerticalSubs )
                        {
                            long nLi = rLi.Right();
                            long nLk = rLk.Right();
                            if ( rLi.Top() == rLk.Top() &&
                                 ((nLi < rLk.Left() && nLi+21 > rLk.Left()) ||
                                  (nLk < rLi.Left() && nLk+21 > rLi.Left())))
                            {
                                this->erase(this->begin() + k);
                                // don't continue with inner loop any more:
                                // the array may shrink!
                                --i;
                                break;
                            }
                        }
                        else
                        {
                            long nLi = rLi.Bottom();
                            long nLk = rLk.Bottom();
                            if ( rLi.Left() == rLk.Left() &&
                                 ((nLi < rLk.Top() && nLi+21 > rLk.Top()) ||
                                  (nLk < rLi.Top() && nLk+21 > rLi.Top())))
                            {
                                this->erase(this->begin() + k);
                                // don't continue with inner loop any more:
                                // the array may shrink!
                                --i;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if ( pRects && (!pRects->empty()) )
            RemoveSuperfluousSubsidiaryLines( *pRects );

        if ( !this->empty() )
        {
            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetLineColor();

            // Reset draw mode in high contrast mode in order to get fill color
            // set at output device. Recover draw mode after draw of lines.
            // Necessary for the subsidiary lines painted by the fly frames.
            sal_uLong nOldDrawMode = pOut->GetDrawMode();
            if( pGlobalShell->GetWin() &&
                Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            {
                pOut->SetDrawMode( 0 );
            }

            for (SwSubsRects::iterator it = this->begin(); it != this->end();
                 ++it)
            {
                SwLineRect &rLRect = (*it);
                // Add condition <!rLRect.IsLocked()> to prevent paint of locked subsidiary lines.
                if ( !rLRect.IsPainted() &&
                     !rLRect.IsLocked() )
                {
                    const Color *pCol = 0;
                    switch ( rLRect.GetSubColor() )
                    {
                        case SUBCOL_PAGE: pCol = &SwViewOption::GetDocBoundariesColor(); break;
                        case SUBCOL_FLY: pCol = &SwViewOption::GetObjectBoundariesColor(); break;
                        case SUBCOL_TAB: pCol = &SwViewOption::GetTableBoundariesColor(); break;
                        case SUBCOL_SECT: pCol = &SwViewOption::GetSectionBoundColor(); break;
                    }

                    if ( pOut->GetFillColor() != *pCol )
                        pOut->SetFillColor( *pCol );
                    pOut->DrawRect( rLRect.SVRect() );

                    rLRect.SetPainted();
                }
            }

            pOut->SetDrawMode( nOldDrawMode );

            pOut->Pop();
        }
    }
}

//-------------------------------------------------------------------------
//Diverse Functions die in diesem File so verwendet werden.

// Note: function <SwAlignRect(..)> also used outside this file.
// Correction: adjust rectangle on pixel level in order
//          to assure, that the border 'leaves its original pixel', if it has to.
//          No prior adjustments for odd relation between pixel and twip.
void SwAlignRect( SwRect &rRect, const ViewShell *pSh )
{
    if( !rRect.HasArea() )
        return;

    // Assure that view shell (parameter <pSh>) exists, if the output device
    // is taken from this view shell --> no output device, no alignment.
    // Output device taken from view shell <pSh>, if <bFlyMetafile> not set.
    if ( !bFlyMetafile && !pSh )
    {
        return;
    }

    const OutputDevice *pOut = bFlyMetafile ?
                        pFlyMetafileOut : pSh->GetOut();

    // Hold original rectangle in pixel
    const Rectangle aOrgPxRect = pOut->LogicToPixel( rRect.SVRect() );
    // Determine pixel-center rectangle in twip
    const SwRect aPxCenterRect( pOut->PixelToLogic( aOrgPxRect ) );

    // Perform adjustments on pixel level.
    SwRect aAlignedPxRect( aOrgPxRect );
    if ( rRect.Top() > aPxCenterRect.Top() )
    {
        // 'leave pixel overlapping on top'
        aAlignedPxRect.Top( aAlignedPxRect.Top() + 1 );
    }

    if ( rRect.Bottom() < aPxCenterRect.Bottom() )
    {
        // 'leave pixel overlapping on bottom'
        aAlignedPxRect.Bottom( aAlignedPxRect.Bottom() - 1 );
    }

    if ( rRect.Left() > aPxCenterRect.Left() )
    {
        // 'leave pixel overlapping on left'
        aAlignedPxRect.Left( aAlignedPxRect.Left() + 1 );
    }

    if ( rRect.Right() < aPxCenterRect.Right() )
    {
        // 'leave pixel overlapping on right'
        aAlignedPxRect.Right( aAlignedPxRect.Right() - 1 );
    }

    // Consider negative width/height check, if aligned SwRect has negative width/height.
    // If Yes, adjust it to width/height = 0 twip.
    // NOTE: A SwRect with negative width/height can occur, if the width/height
    //     of the given SwRect in twip was less than a pixel in twip and that
    //     the alignment calculates that the aligned SwRect should not contain
    //     the pixels the width/height is on.
    if ( aAlignedPxRect.Width() < 0 )
    {
        aAlignedPxRect.Width(0);
    }
    if ( aAlignedPxRect.Height() < 0 )
    {
        aAlignedPxRect.Height(0);
    }
    // Consider zero width/height for converting a rectangle from
    // pixel to logic it needs a width/height. Thus, set width/height
    // to one, if it's zero and correct this on the twip level after the conversion.
    bool bZeroWidth = false;
    if ( aAlignedPxRect.Width() == 0 )
    {
        aAlignedPxRect.Width(1);
        bZeroWidth = true;
    }
    bool bZeroHeight = false;
    if ( aAlignedPxRect.Height() == 0 )
    {
        aAlignedPxRect.Height(1);
        bZeroHeight = true;
    }

    rRect = pOut->PixelToLogic( aAlignedPxRect.SVRect() );

    // Consider zero width/height and adjust calculated aligned twip rectangle.
    // Reset width/height to zero; previous negative width/height haven't to be considered.
    if ( bZeroWidth )
    {
        rRect.Width(0);
    }
    if ( bZeroHeight )
    {
        rRect.Height(0);
    }
}

/** Helper method for twip adjustments on pixel base

    method compares the x- or y-pixel position of two twip-point. If the x-/y-pixel
    positions are the same, the x-/y-pixel position of the second twip point is
    adjusted by a given amount of pixels.

    @author OD
*/
static void lcl_CompPxPosAndAdjustPos( const OutputDevice&  _rOut,
                                const Point&         _rRefPt,
                                Point&               _rCompPt,
                                const bool       _bChkXPos,
                                const sal_Int8       _nPxAdjustment )
{
    const Point aRefPxPt = _rOut.LogicToPixel( _rRefPt );
    Point aCompPxPt = _rOut.LogicToPixel( _rCompPt );

    if ( _bChkXPos )
    {
        if ( aCompPxPt.X() == aRefPxPt.X() )
        {
            aCompPxPt.X() += _nPxAdjustment ;
            const Point aAdjustedCompPt = _rOut.PixelToLogic( aCompPxPt );
            _rCompPt.X() = aAdjustedCompPt.X();
        }
    }
    else
    {
        if ( aCompPxPt.Y() == aRefPxPt.Y() )
        {
            aCompPxPt.Y() += _nPxAdjustment ;
            const Point aAdjustedCompPt = _rOut.PixelToLogic( aCompPxPt );
            _rCompPt.Y() = aAdjustedCompPt.Y();
        }
    }
}

/** Method to pixel-align rectangle for drawing graphic object

    Because for drawing a graphic left-top-corner and size coordinations are
    used, these coordinations have to be determined on pixel level.
    Thus, convert rectangle to pixel and then convert left-top-corner and
    size of pixel rectangle back to logic.
    This calculation is necessary, because there exists a different between
    the convert from logic to pixel of a normal rectangle with its left-top-
    and right-bottom-corner and the same convert of the same rectangle
    with left-top-corner and size.
    Call this method before each <GraphicObject.Draw(...)>

    @author OD
*/
void SwAlignGrfRect( SwRect *pGrfRect, const OutputDevice &rOut )
{
    Rectangle aPxRect = rOut.LogicToPixel( pGrfRect->SVRect() );
    pGrfRect->Pos( rOut.PixelToLogic( aPxRect.TopLeft() ) );
    pGrfRect->SSize( rOut.PixelToLogic( aPxRect.GetSize() ) );
}

static long lcl_AlignWidth( const long nWidth )
{
    if ( nWidth )
    {
        const long nW = nWidth % nPixelSzW;

        if ( !nW || nW > nHalfPixelSzW )
            return std::max(1L, nWidth - nHalfPixelSzW);
    }
    return nWidth;
}

static long lcl_AlignHeight( const long nHeight )
{
    if ( nHeight )
    {
        const long nH = nHeight % nPixelSzH;

        if ( !nH || nH > nHalfPixelSzH )
            return std::max(1L, nHeight - nHalfPixelSzH);
    }
    return nHeight;
}

static long lcl_MinHeightDist( const long nDist )
{
    if ( aScaleX < aMinDistScale || aScaleY < aMinDistScale )
        return nDist;
    return ::lcl_AlignHeight( std::max( nDist, nMinDistPixelH ));
}

//Calculate PrtArea plus surrounding plus shadow.
static void lcl_CalcBorderRect( SwRect &rRect, const SwFrm *pFrm,
                                        const SwBorderAttrs &rAttrs,
                                        const bool bShadow )
{
    // Special handling for cell frames.
    // The printing area of a cell frame is completely enclosed in the frame area
    // and a cell frame has no shadow. Thus, for cell frames the calculated
    // area equals the frame area.
    // Notes: Borders of cell frames in R2L text direction will switch its side
    //        - left border is painted on the right; right border on the left.
    //        See <lcl_PaintLeftLine> and <lcl_PaintRightLine>.
    if( pFrm->IsSctFrm() )
    {
        rRect = pFrm->Prt();
        rRect.Pos() += pFrm->Frm().Pos();
    }
    else if ( pFrm->IsCellFrm() )
        rRect = pFrm->Frm();
    else
    {
        rRect = pFrm->Prt();
        rRect.Pos() += pFrm->Frm().Pos();

        if ( rAttrs.IsLine() || rAttrs.IsBorderDist() ||
             (bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE) )
        {
            //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
            SwRectFn fnRect = pFrm->IsVertical() ? ( pFrm->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

            const SvxBoxItem &rBox = rAttrs.GetBox();
            const bool bTop = 0 != (pFrm->*fnRect->fnGetTopMargin)();
            if ( bTop )
            {
                SwTwips nDiff = rBox.GetTop() ?
                    rBox.CalcLineSpace( BOX_LINE_TOP ) :
                    ( rAttrs.IsBorderDist() ?
                      // Increase of distance by one twip is incorrect.
                      rBox.GetDistance( BOX_LINE_TOP ) : 0 );
                if( nDiff )
                    (rRect.*fnRect->fnSubTop)( nDiff );
            }

            const bool bBottom = 0 != (pFrm->*fnRect->fnGetBottomMargin)();
            if ( bBottom )
            {
                SwTwips nDiff = 0;
                // #i29550#
                if ( pFrm->IsTabFrm() &&
                     ((SwTabFrm*)pFrm)->IsCollapsingBorders() )
                {
                    // For collapsing borders, we have to add the height of
                    // the height of the last line
                    nDiff = ((SwTabFrm*)pFrm)->GetBottomLineSize();
                }
                else
                {
                    nDiff = rBox.GetBottom() ?
                    rBox.CalcLineSpace( BOX_LINE_BOTTOM ) :
                    ( rAttrs.IsBorderDist() ?
                      // Increase of distance by one twip is incorrect.
                      rBox.GetDistance( BOX_LINE_BOTTOM ) : 0 );
                }
                if( nDiff )
                    (rRect.*fnRect->fnAddBottom)( nDiff );
            }

            if ( rBox.GetLeft() )
                (rRect.*fnRect->fnSubLeft)( rBox.CalcLineSpace( BOX_LINE_LEFT ) );
            else if ( rAttrs.IsBorderDist() )
                 // Increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnSubLeft)( rBox.GetDistance( BOX_LINE_LEFT ) );

            if ( rBox.GetRight() )
                (rRect.*fnRect->fnAddRight)( rBox.CalcLineSpace( BOX_LINE_RIGHT ) );
            else if ( rAttrs.IsBorderDist() )
                 // Increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnAddRight)( rBox.GetDistance( BOX_LINE_RIGHT ) );

            if ( bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                const SvxShadowItem &rShadow = rAttrs.GetShadow();
                if ( bTop )
                    (rRect.*fnRect->fnSubTop)(rShadow.CalcShadowSpace(SHADOW_TOP));
                (rRect.*fnRect->fnSubLeft)(rShadow.CalcShadowSpace(SHADOW_LEFT));
                if ( bBottom )
                    (rRect.*fnRect->fnAddBottom)
                                    (rShadow.CalcShadowSpace( SHADOW_BOTTOM ));
                (rRect.*fnRect->fnAddRight)(rShadow.CalcShadowSpace(SHADOW_RIGHT));
            }
        }
    }

    ::SwAlignRect( rRect, pGlobalShell );
}

static void lcl_SubtractFlys( const SwFrm *pFrm, const SwPageFrm *pPage,
                           const SwRect &rRect, SwRegionRects &rRegion )
{
    const SwSortedObjs& rObjs = *pPage->GetSortedObjs();
    const SwFlyFrm* pSelfFly = pFrm->IsInFly() ? pFrm->FindFlyFrm() : pRetoucheFly2;
    if ( !pRetoucheFly )
        pRetoucheFly = pRetoucheFly2;

    for ( sal_uInt16 j = 0; (j < rObjs.Count()) && !rRegion.empty(); ++j )
    {
        const SwAnchoredObject* pAnchoredObj = rObjs[j];
        const SdrObject* pSdrObj = pAnchoredObj->GetDrawObj();

        // Do not consider invisible objects
        if ( !pPage->GetFmt()->GetDoc()->IsVisibleLayerId( pSdrObj->GetLayer() ) )
            continue;

        if ( !pAnchoredObj->ISA(SwFlyFrm) )
            continue;

        const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);

        if ( pSelfFly == pFly || pRetoucheFly == pFly || !rRect.IsOver( pFly->Frm() ) )
            continue;

        if ( !pFly->GetFmt()->GetPrint().GetValue() &&
                (OUTDEV_PRINTER == pGlobalShell->GetOut()->GetOutDevType() ||
                pGlobalShell->IsPreView()))
            continue;

        const bool bLowerOfSelf = pSelfFly && pFly->IsLowerOf( pSelfFly );

        //For character bound Flys only examine those Flys in which it is not
        //anchored itself.
        //Why only for character bound ones you may ask? It never makes sense to
        //subtract frames in which it is anchored itself right?
        if ( pSelfFly && pSelfFly->IsLowerOf( pFly ) )
            continue;

        //Any why does it not apply for the RetoucheFly too?
        if ( pRetoucheFly && pRetoucheFly->IsLowerOf( pFly ) )
            continue;

#if OSL_DEBUG_LEVEL > 0
        //Flys who are anchored inside their own one, must have a bigger OrdNum
        //or be character bound.
        if ( pSelfFly && bLowerOfSelf )
        {
            OSL_ENSURE( pFly->IsFlyInCntFrm() ||
                    pSdrObj->GetOrdNumDirect() > pSelfFly->GetVirtDrawObj()->GetOrdNumDirect(),
                    "Fly with wrong z-Order" );
        }
#endif

        bool bStopOnHell = true;
        if ( pSelfFly )
        {
            const SdrObject *pTmp = pSelfFly->GetVirtDrawObj();
            if ( pSdrObj->GetLayer() == pTmp->GetLayer() )
            {
                if ( pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    //In the same layer we only observe those that are above.
                    continue;
            }
            else
            {
                if ( !bLowerOfSelf && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //From other layers we are only interested in non
                    //transparent ones or those that are internal
                    continue;
                bStopOnHell = false;
            }
        }
        if ( pRetoucheFly )
        {
            const SdrObject *pTmp = pRetoucheFly->GetVirtDrawObj();
            if ( pSdrObj->GetLayer() == pTmp->GetLayer() )
            {
                if ( pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    //In the same layer we only observe those that are above.
                    continue;
            }
            else
            {
                if ( !pFly->IsLowerOf( pRetoucheFly ) && !pFly->GetFmt()->GetOpaque().GetValue() )
                    //From other layers we are only interested in non
                    //transparent ones or those that are internal
                    continue;
                bStopOnHell = false;
            }
        }

        //If the content of the Fly is transparent, we subtract it only if it's
        //contained in the hell layer.
        const IDocumentDrawModelAccess* pIDDMA = pFly->GetFmt()->getIDocumentDrawModelAccess();
        bool bHell = pSdrObj->GetLayer() == pIDDMA->GetHellId();
        if ( (bStopOnHell && bHell) ||
             /// Change internal order of condition
             ///    first check "!bHell", then "..->Lower()" and "..->IsNoTxtFrm()"
             ///    have not to be performed, if frame is in "Hell"
             ( !bHell && pFly->Lower() && pFly->Lower()->IsNoTxtFrm() &&
               ( ((SwNoTxtFrm*)pFly->Lower())->IsTransparent() ||
                 ((SwNoTxtFrm*)pFly->Lower())->HasAnimation() ||
                 pFly->GetFmt()->GetSurround().IsContour()
               )
             )
           )
            continue;

        // Own if-statements for transparent background/shadow of fly frames
        // in order to handle special conditions.
        if ( pFly->IsBackgroundTransparent() )
        {
            // Background <pFly> is transparent drawn. Thus normally, its region
            // have not to be substracted from given region.
            // But, if method is called for a fly frame and
            // <pFly> is a direct lower of this fly frame and
            // <pFly> inherites its transparent background brush from its parent,
            // then <pFly> frame area have to be subtracted from given region.
            // NOTE: Because in Status Quo transparent backgrounds can only be
            //     assigned to fly frames, the handle of this special case
            //     avoids drawing of transparent areas more than once, if
            //     a fly frame inherites a transparent background from its
            //     parent fly frame.
            if ( pFrm->IsFlyFrm() &&
                 (pFly->GetAnchorFrm()->FindFlyFrm() == pFrm) &&
                 static_cast<const SwFlyFrmFmt*>(pFly->GetFmt())->IsBackgroundBrushInherited()
               )
            {
                SwRect aRect;
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFly );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                ::lcl_CalcBorderRect( aRect, pFly, rAttrs, true );
                rRegion -= aRect;
                continue;
            }
            else
            {
                continue;
            }
        }
        if ( pFly->IsShadowTransparent() )
        {
            continue;
        }

        if ( bHell && pFly->GetAnchorFrm()->IsInFly() )
        {
            //So the border won't get dismantled by the background of the other
            //Fly.
            SwRect aRect;
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFly );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            ::lcl_CalcBorderRect( aRect, pFly, rAttrs, true );
            rRegion -= aRect;
        }
        else
        {
            SwRect aRect( pFly->Prt() );
            aRect += pFly->Frm().Pos();
            rRegion -= aRect;
        }
    }
    if ( pRetoucheFly == pRetoucheFly2 )
        pRetoucheFly = 0;
}

//---------------- Output for BrushItem ----------------

/** lcl_DrawGraphicBackgrd - local help method to draw a background for a graphic

    Under certain circumstances we have to draw a background for a graphic.
    This method takes care of the conditions and draws the background with the
    corresponding color.
    Method introduced for bug fix #103876# in order to optimize drawing tiled
    background graphics. Previously, this code was integrated in method
    <lcl_DrawGraphic>.
    Method implemented as a inline, checking the conditions and calling method
    method <lcl_implDrawGraphicBackgrd(..)> for the intrinsic drawing.

    @author OD

    @param _rBackgrdBrush
    background brush contain the color the background has to be drawn.

    @param _pOut
    output device the background has to be drawn in.

    @param _rPaintRect
    paint retangle in the output device, which has to be drawn with the background.
    rectangle have to be aligned by method ::SwAlignRect

    @param _rGraphicObj
    graphic object, for which the background has to be drawn. Used for checking
    the transparency of its bitmap, its type and if the graphic is drawn transparent

    @param _bNumberingGraphic
    boolean indicating that graphic is used as a numbering.

    @param _bBackgrdAlreadyDrawn
    boolean (optional; default: false) indicating, if the background is already drawn.
*/
static void lcl_implDrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                 OutputDevice* _pOut,
                                 const SwRect& _rAlignedPaintRect,
                                 const GraphicObject& _rGraphicObj )
{
    /// determine color of background
    ///     If color of background brush is not "no fill"/"auto fill" or
    ///     <bFlyMetafile> is set, use color of background brush, otherwise
    ///     use global retouche color.
    const Color aColor( ( (_rBackgrdBrush.GetColor() != COL_TRANSPARENT) || bFlyMetafile )
                        ? _rBackgrdBrush.GetColor()
                        : aGlobalRetoucheColor );

    /// determine, if background color have to be drawn transparent
    /// and calculate transparency percent value
    sal_Int8 nTransparencyPercent = 0;
    bool bDrawTransparent = false;
    if ( aColor.GetTransparency() != 0 )
    ///     background color is transparent --> draw transparent.
    {
        bDrawTransparent = true;
        nTransparencyPercent = (aColor.GetTransparency()*100 + 0x7F)/0xFF;
    }
    else if ( (_rGraphicObj.GetAttr().GetTransparency() != 0) &&
                (_rBackgrdBrush.GetColor() == COL_TRANSPARENT) )
    ///     graphic is drawn transparent and background color is
    ///     "no fill"/"auto fill" --> draw transparent
    {
        bDrawTransparent = true;
        nTransparencyPercent = (_rGraphicObj.GetAttr().GetTransparency()*100 + 0x7F)/0xFF;
    }

    if ( bDrawTransparent )
    {
        /// draw background transparent
        if( _pOut->GetFillColor() != aColor.GetRGBColor() )
            _pOut->SetFillColor( aColor.GetRGBColor() );
        PolyPolygon aPoly( _rAlignedPaintRect.SVRect() );
        _pOut->DrawTransparent( aPoly, nTransparencyPercent );
    }
    else
    {
        /// draw background opaque
        if ( _pOut->GetFillColor() != aColor )
            _pOut->SetFillColor( aColor );
        _pOut->DrawRect( _rAlignedPaintRect.SVRect() );
    }
}

static inline void lcl_DrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                    OutputDevice* _pOut,
                                    const SwRect& _rAlignedPaintRect,
                                    const GraphicObject& _rGraphicObj,
                                    bool _bNumberingGraphic,
                                    bool _bBackgrdAlreadyDrawn = false )
{
    /// draw background with background color, if
    ///     (1) graphic is not used as a numbering AND
    ///     (2) background is not already drawn AND
    ///     (3) intrinsic graphic is transparent OR intrinsic graphic doesn't exists
    if ( !_bNumberingGraphic &&
         !_bBackgrdAlreadyDrawn &&
         ( _rGraphicObj.IsTransparent() || _rGraphicObj.GetType() == GRAPHIC_NONE  )
       )
    {
        lcl_implDrawGraphicBackgrd( _rBackgrdBrush, _pOut, _rAlignedPaintRect, _rGraphicObj );
    }
}

/// Note: the transparency of the background graphic
///     is saved in SvxBrushItem.GetGraphicObject(<shell>).GetAttr().Set/GetTransparency()
///     and is considered in the drawing of the graphic.
///     Thus, to provide transparent background graphic for text frames nothing
///     has to be coded.
/// Use align rectangle for drawing graphic
/// Pixel-align coordinations for drawing graphic.
/// Outsource code for drawing background of the graphic
///     with a background color in method <lcl_DrawGraphicBackgrd>
///     Also, change type of <bGrfNum> and <bClip> from <sal_Bool> to <bool>.
static void lcl_DrawGraphic( const SvxBrushItem& rBrush, OutputDevice *pOut,
                      ViewShell &rSh, const SwRect &rGrf, const SwRect &rOut,
                      bool bClip, bool bGrfNum,
                      bool bBackgrdAlreadyDrawn = false )
                      /// add parameter <bBackgrdAlreadyDrawn> to indicate
                      /// that the background is already drawn.
{
    /// Calculate align rectangle from parameter <rGrf> and use aligned
    /// rectangle <aAlignedGrfRect> in the following code
    SwRect aAlignedGrfRect = rGrf;
    ::SwAlignRect( aAlignedGrfRect, &rSh );

    /// Change type from <sal_Bool> to <bool>.
    const bool bNotInside = bClip && !rOut.IsInside( aAlignedGrfRect );
    if ( bNotInside )
    {
        pOut->Push( PUSH_CLIPREGION );
        pOut->IntersectClipRegion( rOut.SVRect() );
    }

    //Hier kein Link, wir wollen die Grafik synchron laden!
    ((SvxBrushItem&)rBrush).SetDoneLink( Link() );
    GraphicObject *pGrf = (GraphicObject*)rBrush.GetGraphicObject();

    /// Outsourcing drawing of background with a background color.
    ::lcl_DrawGraphicBackgrd( rBrush, pOut, aAlignedGrfRect, *pGrf, bGrfNum, bBackgrdAlreadyDrawn );

    /// Because for drawing a graphic left-top-corner and size coordinations are
    /// used, these coordinations have to be determined on pixel level.
    ::SwAlignGrfRect( &aAlignedGrfRect, *pOut );
    pGrf->DrawWithPDFHandling( *pOut, aAlignedGrfRect.Pos(), aAlignedGrfRect.SSize() );

    if ( bNotInside )
        pOut->Pop();
} // end of method <lcl_DrawGraphic>

void DrawGraphic( const SvxBrushItem *pBrush,
                              const XFillStyleItem* pFillStyleItem,
                              const XFillGradientItem* pFillGradientItem,
                              OutputDevice *pOutDev,
                              const SwRect &rOrg,
                              const SwRect &rOut,
                              const sal_uInt8 nGrfNum,
                              const sal_Bool bConsiderBackgroundTransparency )
    /// Add 6th parameter to indicate that method should
    ///   consider background transparency, saved in the color of the brush item
{
    ViewShell &rSh = *pGlobalShell;
    bool bReplaceGrfNum = GRFNUM_REPLACE == nGrfNum;
    bool bGrfNum = GRFNUM_NO != nGrfNum;
    Size aGrfSize;
    SvxGraphicPosition ePos = GPOS_NONE;
    if( pBrush && !bReplaceGrfNum )
    {
        if( rSh.GetViewOptions()->IsGraphic() )
        {
            // load graphic directly in PDF import
            // #i68953# - also during print load graphic directly.
            if ( (rSh).GetViewOptions()->IsPDFExport() ||
                 rSh.GetOut()->GetOutDevType() == OUTDEV_PRINTER )
            {
                ((SvxBrushItem*)pBrush)->PurgeMedium();
                ((SvxBrushItem*)pBrush)->SetDoneLink( Link() );
            }
            else
                ((SvxBrushItem*)pBrush)->SetDoneLink( STATIC_LINK(
                                    rSh.GetDoc(), SwDoc, BackgroundDone ) );
            const Graphic* pGrf = pBrush->GetGraphic();
            if( pGrf && GRAPHIC_NONE != pGrf->GetType() )
            {
                ePos = pBrush->GetGraphicPos();
                if( pGrf->IsSupportedGraphic() )
                    // don't the use the specific output device! Bug 94802
                    aGrfSize = ::GetGraphicSizeTwip( *pGrf, 0 );
            }
        }
        else
            bReplaceGrfNum = bGrfNum;
    }

    SwRect aGrf;
    aGrf.SSize( aGrfSize );
    bool bDraw = true;
    bool bRetouche = true;
    switch ( ePos )
    {
    case GPOS_LT:
        aGrf.Pos() = rOrg.Pos();
        break;

    case GPOS_MT:
        aGrf.Pos().Y() = rOrg.Top();
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RT:
        aGrf.Pos().Y() = rOrg.Top();
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_LM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Left();
        break;

    case GPOS_MM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RM:
        aGrf.Pos().Y() = rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2;
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_LB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Left();
        break;

    case GPOS_MB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2;
        break;

    case GPOS_RB:
        aGrf.Pos().Y() = rOrg.Bottom() - aGrfSize.Height();
        aGrf.Pos().X() = rOrg.Right() - aGrfSize.Width();
        break;

    case GPOS_AREA:
        aGrf = rOrg;
        /// In spite the fact that the background graphic have to fill the complete
        /// area, it has been checked, if the graphic will completely fill out
        /// the region to be painted <rOut> and thus, nothing has to be retouched.
        /// For example, this is the case for a fly frame without a background
        /// brush positioned on the border of the page and inherited the
        /// background brush from the page.
        bRetouche = !rOut.IsInside( aGrf );
        break;

    case GPOS_TILED:
        {
            // draw background of tiled graphic before drawing tiled graphic in loop
            // determine graphic object
            GraphicObject* pGraphicObj = const_cast< GraphicObject* >(pBrush->GetGraphicObject());
            // calculate aligned paint rectangle
            SwRect aAlignedPaintRect = rOut;
            ::SwAlignRect( aAlignedPaintRect, &rSh );
            // draw background color for aligned paint rectangle
            lcl_DrawGraphicBackgrd( *pBrush, pOutDev, aAlignedPaintRect, *pGraphicObj, bGrfNum );

            // set left-top-corner of background graphic to left-top-corner of the
            // area, from which the background brush is determined.
            aGrf.Pos() = rOrg.Pos();
            // setup clipping at output device
            pOutDev->Push( PUSH_CLIPREGION );
            pOutDev->IntersectClipRegion( rOut.SVRect() );
            // use new method <GraphicObject::DrawTiled(::)>
            {
                // calculate paint offset
                Point aPaintOffset( aAlignedPaintRect.Pos() - aGrf.Pos() );
                // draw background graphic tiled for aligned paint rectangle
                // #i42643#
                // For PDF export, every draw operation for bitmaps takes a
                // noticeable amount of place (~50 characters). Thus, optimize
                // between tile bitmap size and number of drawing operations here.
                //
                //                  A_out
                // n_chars = k1 *  ---------- + k2 * A_bitmap
                //                  A_bitmap
                //
                // minimum n_chars is obtained for (derive for  A_bitmap,
                // set to 0, take positive solution):
                //                   k1
                // A_bitmap = Sqrt( ---- A_out )
                //                   k2
                //
                // where k1 is the number of chars per draw operation, and
                // k2 is the number of chars per bitmap pixel.
                // This is approximately 50 and 7 for current PDF writer, respectively.
                //
                const double    k1( 50 );
                const double    k2( 7 );
                const Size      aSize( aAlignedPaintRect.SSize() );
                const double    Abitmap( k1/k2 * static_cast<double>(aSize.Width())*aSize.Height() );

                pGraphicObj->DrawTiled( pOutDev,
                                        aAlignedPaintRect.SVRect(),
                                        aGrf.SSize(),
                                        Size( aPaintOffset.X(), aPaintOffset.Y() ),
                                        NULL, GRFMGR_DRAW_STANDARD,
                                        ::std::max( 128, static_cast<int>( sqrt(sqrt( Abitmap)) + .5 ) ) );
            }
            // reset clipping at output device
            pOutDev->Pop();
            // set <bDraw> and <bRetouche> to false, indicating that background
            // graphic and background are already drawn.
            bDraw = bRetouche = false;
        }
        break;

    case GPOS_NONE:
        bDraw = false;
        break;

    default: OSL_ENSURE( !pOutDev, "new Graphic position?" );
    }

    /// init variable <bGrfBackgrdAlreadDrawn> to indicate, if background of
    /// graphic is already drawn or not.
    bool bGrfBackgrdAlreadyDrawn = false;
    if ( bRetouche )
    {
        pOutDev->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
        pOutDev->SetLineColor();

        //     check, if a existing background graphic (not filling the complete
        //     background) is transparent drawn and the background color is
        //     "no fill" respectively "auto fill", if background transparency
        //     has to be considered.
        //     If YES, memorise transparency of background graphic.
        //     check also, if background graphic bitmap is transparent.
        bool bTransparentGrfWithNoFillBackgrd = false;
        sal_Int32 nGrfTransparency = 0;
        bool bGrfIsTransparent = false;
        if ( (ePos != GPOS_NONE) &&
             (ePos != GPOS_TILED) && (ePos != GPOS_AREA)
           )
        {
            GraphicObject *pGrf = (GraphicObject*)pBrush->GetGraphicObject();
            if ( bConsiderBackgroundTransparency )
            {
                GraphicAttr pGrfAttr = pGrf->GetAttr();
                if ( (pGrfAttr.GetTransparency() != 0) &&
                     ( pBrush && (pBrush->GetColor() == COL_TRANSPARENT) )
                   )
                {
                    bTransparentGrfWithNoFillBackgrd = true;
                    nGrfTransparency = pGrfAttr.GetTransparency();
                }
            }
            if ( pGrf->IsTransparent() )
            {
                bGrfIsTransparent = true;
            }
        }

        /// to get color of brush, check background color against COL_TRANSPARENT ("no fill"/"auto fill")
        /// instead of checking, if transparency is not set.
        const Color aColor( pBrush &&
                            ( !(pBrush->GetColor() == COL_TRANSPARENT) ||
                              bFlyMetafile )
                    ? pBrush->GetColor()
                    : aGlobalRetoucheColor );

        /// determine, if background region have to be
        ///     drawn transparent.
        ///     background region has to be drawn transparent, if
        ///         background transparency have to be considered
        ///     AND
        ///       ( background color is transparent OR
        ///         background graphic is transparent and background color is "no fill"
        ///       )
        bool bDrawTransparent = bConsiderBackgroundTransparency &&
                                ( ( aColor.GetTransparency() != 0) ||
                                    bTransparentGrfWithNoFillBackgrd );

        // #i75614# reset draw mode in high contrast mode in order to get fill color set
        const sal_uLong nOldDrawMode = pOutDev->GetDrawMode();
        if ( pGlobalShell->GetWin() &&
             Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            pOutDev->SetDrawMode( 0 );
        }

        /// OD 06.08.2002 #99657# - if background region have to be drawn
        ///     transparent, set only the RGB values of the background color as
        ///     the fill color for the output device.
        if ( bDrawTransparent )
        {
            if( pOutDev->GetFillColor() != aColor.GetRGBColor() )
                pOutDev->SetFillColor( aColor.GetRGBColor() );
        }
        else
        {
            if( pOutDev->GetFillColor() != aColor )
                pOutDev->SetFillColor( aColor );
        }

        // #i75614#
        // restore draw mode
        pOutDev->SetDrawMode( nOldDrawMode );

        /// OD 02.09.2002 #99657#
        if ( bDrawTransparent )
        {
            /// background region have to be drawn transparent.
            /// Thus, create a poly-polygon from the region and draw it with
            /// the corresponding transparency precent.
            PolyPolygon aDrawPoly( rOut.SVRect() );
            if ( aGrf.HasArea() )
            {
                if ( !bGrfIsTransparent )
                {
                    /// substract area of background graphic from draw area
                    /// OD 08.10.2002 #103898# - consider only that part of the
                    ///     graphic area that is overlapping with draw area.
                    SwRect aTmpGrf = aGrf;
                    aTmpGrf.Intersection( rOut );
                    if ( aTmpGrf.HasArea() )
                    {
                        Polygon aGrfPoly( aTmpGrf.SVRect() );
                        aDrawPoly.Insert( aGrfPoly );
                    }
                }
                else
                    bGrfBackgrdAlreadyDrawn = true;
            }
            /// calculate transparency percent:
            /// ( <transparency value[0x01..0xFF]>*100 + 0x7F ) / 0xFF
            /// If there is a background graphic with a background color "no fill"/"auto fill",
            /// the transparency value is taken from the background graphic,
            /// otherwise take the transparency value from the color.
            sal_Int8 nTransparencyPercent = static_cast<sal_Int8>(
              (( bTransparentGrfWithNoFillBackgrd ? nGrfTransparency : aColor.GetTransparency()
               )*100 + 0x7F)/0xFF);
            /// draw poly-polygon transparent
            pOutDev->DrawTransparent( aDrawPoly, nTransparencyPercent );
        }
        else if (!pFillStyleItem || pFillStyleItem->GetValue() != XFILL_GRADIENT || !pFillGradientItem)
        {
            SwRegionRects aRegion( rOut, 4 );
            if ( !bGrfIsTransparent )
                aRegion -= aGrf;
            else
                bGrfBackgrdAlreadyDrawn = true;
            /// loop rectangles of background region, which has to be drawn
            for( sal_uInt16 i = 0; i < aRegion.size(); ++i )
            {
                pOutDev->DrawRect( aRegion[i].SVRect() );
            }
        }
        else
            pOutDev->DrawGradient(rOut.SVRect(), pFillGradientItem->GetGradientValue().VclGradient());
       pOutDev ->Pop();
    }

    if( bDraw && aGrf.IsOver( rOut ) )
        /// OD 02.09.2002 #99657#
        /// add parameter <bGrfBackgrdAlreadyDrawn>
        lcl_DrawGraphic( *pBrush, pOutDev, rSh, aGrf, rOut, true, bGrfNum,
                         bGrfBackgrdAlreadyDrawn );

    if( bReplaceGrfNum )
    {
        const BitmapEx& rBmp = ViewShell::GetReplacementBitmap( false );
        Font aTmp( pOutDev->GetFont() );
        Graphic::DrawEx( pOutDev, aEmptyStr, aTmp, rBmp, rOrg.Pos(), rOrg.SSize() );
    }
}

//------------------------------------------------------------------------

/** local help method for SwRootFrm::Paint(..) - Adjust given rectangle to pixel size

    By OD at 27.09.2002 for #103636#
    In order to avoid paint errors caused by multiple alignments - e.g. method
    ::SwAlignRect(..) - and other changes to the rectangle to be painted,
    this method is called for the rectangle to be painted in order to
    adjust it to the pixel it is overlapping.

    @author OD
*/
static void lcl_AdjustRectToPixelSize( SwRect& io_aSwRect, const OutputDevice &aOut )
{
    /// local constant object of class <Size> to determine number of Twips
    /// representing a pixel.
    const Size aTwipToPxSize( aOut.PixelToLogic( Size( 1,1 )) );

    /// local object of class <Rectangle> in Twip coordinates
    /// calculated from given rectangle aligned to pixel centers.
    const Rectangle aPxCenterRect = aOut.PixelToLogic(
            aOut.LogicToPixel( io_aSwRect.SVRect() ) );

    /// local constant object of class <Rectangle> representing given rectangle
    /// in pixel.
    const Rectangle aOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );

    /// calculate adjusted rectangle from pixel centered rectangle.
    /// Due to rounding differences <aPxCenterRect> doesn't exactly represents
    /// the Twip-centers. Thus, adjust borders by half of pixel width/height plus 1.
    /// Afterwards, adjust calculated Twip-positions of the all borders.
    Rectangle aSizedRect = aPxCenterRect;
    aSizedRect.Left() -= (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Right() += (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Top() -= (aTwipToPxSize.Height()/2 + 1);
    aSizedRect.Bottom() += (aTwipToPxSize.Height()/2 + 1);

    /// adjust left()
    while ( (aOut.LogicToPixel(aSizedRect)).Left() < aOrgPxRect.Left() )
    {
        ++aSizedRect.Left();
    }
    /// adjust right()
    while ( (aOut.LogicToPixel(aSizedRect)).Right() > aOrgPxRect.Right() )
    {
        --aSizedRect.Right();
    }
    /// adjust top()
    while ( (aOut.LogicToPixel(aSizedRect)).Top() < aOrgPxRect.Top() )
    {
        ++aSizedRect.Top();
    }
    /// adjust bottom()
    while ( (aOut.LogicToPixel(aSizedRect)).Bottom() > aOrgPxRect.Bottom() )
    {
        --aSizedRect.Bottom();
    }

    io_aSwRect = SwRect( aSizedRect );

#if OSL_DEBUG_LEVEL > 0
    Rectangle aTestOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );
    Rectangle aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect == aTestNewPxRect,
            "Error in lcl_AlignRectToPixelSize(..): Adjusted rectangle has incorrect position or size");
    /// check Left()
    --aSizedRect.Left();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Left() >= (aTestNewPxRect.Left()+1),
            "Error in lcl_AlignRectToPixelSize(..): Left() not correct adjusted");
    ++aSizedRect.Left();
    /// check Right()
    ++aSizedRect.Right();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Right() <= (aTestNewPxRect.Right()-1),
            "Error in lcl_AlignRectToPixelSize(..): Right() not correct adjusted");
    --aSizedRect.Right();
    /// check Top()
    --aSizedRect.Top();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Top() >= (aTestNewPxRect.Top()+1),
            "Error in lcl_AlignRectToPixelSize(..): Top() not correct adjusted");
    ++aSizedRect.Top();
    /// check Bottom()
    ++aSizedRect.Bottom();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Bottom() <= (aTestNewPxRect.Bottom()-1),
            "Error in lcl_AlignRectToPixelSize(..): Bottom() not correct adjusted");
    --aSizedRect.Bottom();
#endif
}

//
// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES START
//

struct SwLineEntry
{
    SwTwips mnKey;
    SwTwips mnStartPos;
    SwTwips mnEndPos;

    svx::frame::Style maAttribute;

    enum OverlapType { NO_OVERLAP, OVERLAP1, OVERLAP2, OVERLAP3 };

public:
    SwLineEntry( SwTwips nKey,
                 SwTwips nStartPos,
                 SwTwips nEndPos,
                 const svx::frame::Style& rAttribute );

    OverlapType Overlaps( const SwLineEntry& rComp ) const;
};

SwLineEntry::SwLineEntry( SwTwips nKey,
                          SwTwips nStartPos,
                          SwTwips nEndPos,
                          const svx::frame::Style& rAttribute )
    :   mnKey( nKey ),
        mnStartPos( nStartPos ),
        mnEndPos( nEndPos ),
        maAttribute( rAttribute )
{
}

/*

 1. ----------    rOld
       ---------- rNew

 2. ----------    rOld
    ------------- rNew

 3.    -------    rOld
    ------------- rNew

 4. ------------- rOld
       ---------- rNew

 5. ----------    rOld
       ----       rNew

 6. ----------    rOld
    ----------    rNew

 7. ------------- rOld
    ----------    rNew

 8.    ---------- rOld
    ------------- rNew

 9.    ---------- rOld
    ----------    rNew
*/

SwLineEntry::OverlapType SwLineEntry::Overlaps( const SwLineEntry& rNew )  const
{
    SwLineEntry::OverlapType eRet = OVERLAP3;

    if ( mnStartPos >= rNew.mnEndPos || mnEndPos <= rNew.mnStartPos )
        eRet = NO_OVERLAP;

    // 1, 2, 3
    else if ( mnEndPos < rNew.mnEndPos )
        eRet = OVERLAP1;

    // 4, 5, 6, 7
    else if ( mnStartPos <= rNew.mnStartPos && mnEndPos >= rNew.mnEndPos )
        eRet = OVERLAP2;

    // 8, 9
    return eRet;
}

struct lt_SwLineEntry
{
    bool operator()( const SwLineEntry& e1, const SwLineEntry& e2 ) const
    {
        return e1.mnStartPos < e2.mnStartPos;
    }
};

typedef std::set< SwLineEntry, lt_SwLineEntry > SwLineEntrySet;
typedef std::set< SwLineEntry, lt_SwLineEntry >::iterator SwLineEntrySetIter;
typedef std::set< SwLineEntry, lt_SwLineEntry >::const_iterator SwLineEntrySetConstIter;
typedef std::map< SwTwips, SwLineEntrySet > SwLineEntryMap;
typedef std::map< SwTwips, SwLineEntrySet >::iterator SwLineEntryMapIter;
typedef std::map< SwTwips, SwLineEntrySet >::const_iterator SwLineEntryMapConstIter;

class SwTabFrmPainter
{
    SwLineEntryMap maVertLines;
    SwLineEntryMap maHoriLines;
    const SwTabFrm& mrTabFrm;

    void Insert( SwLineEntry&, bool bHori );
    void Insert( const SwFrm& rFrm, const SvxBoxItem& rBoxItem );
    void HandleFrame( const SwLayoutFrm& rFrm );
    void FindStylesForLine( const Point&,
                            const Point&,
                            svx::frame::Style*,
                            bool bHori ) const;

public:
    SwTabFrmPainter( const SwTabFrm& rTabFrm );

    void PaintLines( OutputDevice& rDev, const SwRect& rRect ) const;
};

SwTabFrmPainter::SwTabFrmPainter( const SwTabFrm& rTabFrm )
    : mrTabFrm( rTabFrm )
{
    HandleFrame( rTabFrm );
}

void SwTabFrmPainter::HandleFrame( const SwLayoutFrm& rLayoutFrm )
{
    // Add border lines of cell frames. Skip covered cells. Skip cells
    // in special row span row, which do not have a negative row span:
    if ( rLayoutFrm.IsCellFrm() && !rLayoutFrm.IsCoveredCell() )
    {
        const SwCellFrm* pThisCell = static_cast<const SwCellFrm*>(&rLayoutFrm);
        const SwRowFrm* pRowFrm = static_cast<const SwRowFrm*>(pThisCell->GetUpper());
        const long nRowSpan = pThisCell->GetTabBox()->getRowSpan();
        if ( !pRowFrm->IsRowSpanLine() || nRowSpan > 1 || nRowSpan < -1 )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), &rLayoutFrm );
            const SwBorderAttrs& rAttrs = *aAccess.Get();
            const SvxBoxItem& rBox = rAttrs.GetBox();
            Insert( rLayoutFrm, rBox );
        }
    }

    // Recurse into lower layout frames, but do not recurse into lower tabframes.
    const SwFrm* pLower = rLayoutFrm.Lower();
    while ( pLower )
    {
        const SwLayoutFrm* pLowerLayFrm = dynamic_cast<const SwLayoutFrm*>(pLower);
        if ( pLowerLayFrm && !pLowerLayFrm->IsTabFrm() )
            HandleFrame( *pLowerLayFrm );

        pLower = pLower->GetNext();
    }
}

void SwTabFrmPainter::PaintLines( OutputDevice& rDev, const SwRect& rRect ) const
{
    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, rDev );

    const SwFrm* pTmpFrm = &mrTabFrm;
    const bool bVert = pTmpFrm->IsVertical();

    SwLineEntryMapConstIter aIter = maHoriLines.begin();
    bool bHori = true;

    // color for subsidiary lines:
    const Color& rCol( SwViewOption::GetTableBoundariesColor() );

    // high contrast mode:
    // overrides the color of non-subsidiary lines.
    const Color* pHCColor = 0;
    sal_uLong nOldDrawMode = rDev.GetDrawMode();
    if( pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pHCColor = &SwViewOption::GetFontColor();
        rDev.SetDrawMode( 0 );
    }

    // set clip region:
    rDev.Push( PUSH_CLIPREGION );
    Size aSize( rRect.SSize() );
    // Hack! Necessary, because the layout is not pixel aligned!
    aSize.Width() += nPixelSzW; aSize.Height() += nPixelSzH;
    rDev.SetClipRegion(Region(Rectangle(rRect.Pos(), aSize)));

    // The following stuff if necessary to have the new table borders fit
    // into a ::SwAlignRect adjusted world.
    const SwTwips nTwipXCorr =  bVert ? 0 : std::max( 0L, nHalfPixelSzW - 2 );    // 1 < 2 < 3 ;-)
    const SwTwips nTwipYCorr = !bVert ? 0 : std::max( 0L, nHalfPixelSzW - 2 );    // 1 < 2 < 3 ;-)
    const SwFrm* pUpper = mrTabFrm.GetUpper();
    SwRect aUpper( pUpper->Prt() );
    aUpper.Pos() += pUpper->Frm().Pos();
    SwRect aUpperAligned( aUpper );
    ::SwAlignRect( aUpperAligned, pGlobalShell );

    while ( true )
    {
        if ( bHori && aIter == maHoriLines.end() )
        {
            aIter = maVertLines.begin();
            bHori = false;
        }

        if ( !bHori && aIter == maVertLines.end() )
            break;

        const SwLineEntrySet& rEntrySet = (*aIter).second;
        SwLineEntrySetConstIter aSetIter = rEntrySet.begin();
        while ( aSetIter != rEntrySet.end() )
        {
            const SwLineEntry& rEntry = *aSetIter;
            const svx::frame::Style& rEntryStyle( (*aSetIter).maAttribute );

            Point aStart, aEnd;
            if ( bHori )
            {
                aStart.X() = rEntry.mnStartPos;
                aStart.Y() = rEntry.mnKey;
                aEnd.X() = rEntry.mnEndPos;
                aEnd.Y() = rEntry.mnKey;
            }
            else
            {
                aStart.X() = rEntry.mnKey;
                aStart.Y() = rEntry.mnStartPos;
                aEnd.X() = rEntry.mnKey;
                aEnd.Y() = rEntry.mnEndPos;
            }

            SwRect aRepaintRect( aStart, aEnd );

            // the repaint rectangle has to be moved a bit for the centered lines:
            SwTwips nRepaintRectSize = !rEntryStyle.GetWidth() ? 1 : rEntryStyle.GetWidth();
            if ( bHori )
            {
                aRepaintRect.Height( 2 * nRepaintRectSize );
                aRepaintRect.Pos().Y() -= nRepaintRectSize;
            }
            else
            {
                aRepaintRect.Width( 2 * nRepaintRectSize );
                aRepaintRect.Pos().X() -= nRepaintRectSize;
            }

            if ( rRect.IsOver( aRepaintRect ) )
            {
                svx::frame::Style aStyles[ 7 ];
                aStyles[ 0 ] = rEntryStyle;
                FindStylesForLine( aStart, aEnd, aStyles, bHori );

                // subsidiary lines
                const Color* pTmpColor = 0;
                if ( 0 == aStyles[ 0 ].GetWidth() )
                {
                    if ( IS_SUBS_TABLE && pGlobalShell->GetWin() )
                        aStyles[ 0 ].Set( rCol, rCol, rCol, false, 1, 0, 0 );
                }
                else
                    pTmpColor = pHCColor;

                // The line sizes stored in the line style have to be adjusted as well.
                // This will guarantee that lines with the same twip size will have the
                // same pixel size.
                for ( int i = 0; i < 7; ++i )
                {
                    sal_uInt16 nPrim = aStyles[ i ].Prim();
                    sal_uInt16 nDist = aStyles[ i ].Dist();
                    sal_uInt16 nSecn = aStyles[ i ].Secn();

                    if ( nPrim > 0 )
                        nPrim = (sal_uInt16)( std::max( 1L, nPixelSzH * ( nPrim / nPixelSzH ) ) );
                    if ( nDist > 0 )
                        nDist = (sal_uInt16)( std::max( 1L, nPixelSzH * ( nDist / nPixelSzH ) ) );
                    if ( nSecn > 0 )
                        nSecn = (sal_uInt16)( std::max( 1L, nPixelSzH * ( nSecn / nPixelSzH ) ) );

                    aStyles[ i ].Set( nPrim, nDist, nSecn );
                }

                // The (twip) positions will be adjusted to meet these requirements:
                // 1. The y coordinates are located in the middle of the pixel grid
                // 2. The x coordinated are located at the beginning of the pixel grid
                // This is done, because the horizontal lines are painted "at beginning",
                // whereas the vertical lines are painted "centered". By making the line
                // sizes a multiple of one pixel size, we can assure, that all lines having
                // the same twip size have the same pixel size, independent of their position
                // on the screen.
                Point aPaintStart = rDev.PixelToLogic( rDev.LogicToPixel( aStart ) );
                Point aPaintEnd = rDev.PixelToLogic( rDev.LogicToPixel( aEnd ) );

                if( pGlobalShell->GetWin() )
                {
                    // The table borders do not use SwAlignRect, but all the other frames do.
                    // Therefore we tweak the outer borders a bit to achieve that the outer
                    // borders match the subsidiary lines of the upper:
                    if ( aStart.X() == aUpper.Left() )
                        aPaintStart.X() = aUpperAligned.Left();
                    else if ( aStart.X() == aUpper._Right() )
                        aPaintStart.X() = aUpperAligned._Right();
                    if ( aStart.Y() == aUpper.Top() )
                        aPaintStart.Y() = aUpperAligned.Top();
                    else if ( aStart.Y() == aUpper._Bottom() )
                        aPaintStart.Y() = aUpperAligned._Bottom();

                    if ( aEnd.X() == aUpper.Left() )
                        aPaintEnd.X() = aUpperAligned.Left();
                    else if ( aEnd.X() == aUpper._Right() )
                        aPaintEnd.X() = aUpperAligned._Right();
                    if ( aEnd.Y() == aUpper.Top() )
                        aPaintEnd.Y() = aUpperAligned.Top();
                    else if ( aEnd.Y() == aUpper._Bottom() )
                        aPaintEnd.Y() = aUpperAligned._Bottom();
                }

                // logically vertical lines are painted centered on the line,
                // logically horizontal lines are painted "below" the line
                bool const isBelow((mrTabFrm.IsVertical()) ? !bHori : bHori);
                double const offsetStart = (isBelow)
                    ?   aStyles[0].GetWidth() / 2.0
                    :   std::max<double>(aStyles[1].GetWidth(),
                            aStyles[3].GetWidth()) / 2.0;
                double const offsetEnd = (isBelow)
                    ?   aStyles[0].GetWidth() / 2.0
                    :   std::max<double>(aStyles[4].GetWidth(),
                            aStyles[6].GetWidth()) / 2.0;
                if (mrTabFrm.IsVertical())
                {
                    aPaintStart.X() -= static_cast<long>(offsetStart + 0.5);
                    aPaintEnd.X()   -= static_cast<long>(offsetEnd   + 0.5);
                }
                else
                {
                    aPaintStart.Y() += static_cast<long>(offsetStart + 0.5);
                    aPaintEnd.Y()   += static_cast<long>(offsetEnd   + 0.5);
                }

                aPaintStart.X() -= nTwipXCorr; // nHalfPixelSzW - 2 to assure that we do not leave the pixel
                aPaintEnd.X()   -= nTwipXCorr;
                aPaintStart.Y() -= nTwipYCorr;
                aPaintEnd.Y()   -= nTwipYCorr;

                // Here comes the painting stuff: Thank you, DR, great job!!!
                if ( bHori )
                {
                    mrTabFrm.ProcessPrimitives( svx::frame::CreateBorderPrimitives(
                        aPaintStart,
                        aPaintEnd,
                        aStyles[ 0 ],   // current style
                        aStyles[ 1 ],   // aLFromT
                        aStyles[ 2 ],   // aLFromL
                        aStyles[ 3 ],   // aLFromB
                        aStyles[ 4 ],   // aRFromT
                        aStyles[ 5 ],   // aRFromR
                        aStyles[ 6 ],   // aRFromB
                        pTmpColor
                        )
                    );
                }
                else
                {
                    mrTabFrm.ProcessPrimitives( svx::frame::CreateBorderPrimitives(
                        aPaintEnd,
                        aPaintStart,
                        aStyles[ 0 ],   // current style
                        aStyles[ 4 ],   // aBFromL
                        aStyles[ 5 ],   // aBFromB
                        aStyles[ 6 ],   // aBFromR
                        aStyles[ 1 ],   // aTFromL
                        aStyles[ 2 ],   // aTFromT
                        aStyles[ 3 ],   // aTFromR
                        pTmpColor
                        )
                    );
                }
            }

            ++aSetIter;
        }

        ++aIter;
    }

    // restore output device:
    rDev.Pop();
    rDev.SetDrawMode( nOldDrawMode );
}

// Finds the lines that join the line defined by (StartPoint, EndPoint) in either
// StartPoint or Endpoint. The styles of these lines are required for DR's magic
// line painting functions.
void SwTabFrmPainter::FindStylesForLine( const Point& rStartPoint,
                                         const Point& rEndPoint,
                                         svx::frame::Style* pStyles,
                                         bool bHori ) const
{
    // pStyles[ 1 ] = bHori ? aLFromT : TFromL
    // pStyles[ 2 ] = bHori ? aLFromL : TFromT,
    // pStyles[ 3 ] = bHori ? aLFromB : TFromR,
    // pStyles[ 4 ] = bHori ? aRFromT : BFromL,
    // pStyles[ 5 ] = bHori ? aRFromR : BFromB,
    // pStyles[ 6 ] = bHori ? aRFromB : BFromR,

    SwLineEntryMapConstIter aMapIter = maVertLines.find( rStartPoint.X() );
    OSL_ENSURE( aMapIter != maVertLines.end(), "FindStylesForLine: Error" );
    const SwLineEntrySet& rVertSet = (*aMapIter).second;
    SwLineEntrySetConstIter aIter = rVertSet.begin();

    while ( aIter != rVertSet.end() )
    {
        const SwLineEntry& rEntry = *aIter;
        if ( bHori )
        {
            if ( rStartPoint.Y() == rEntry.mnStartPos )
                pStyles[ 3 ] = rEntry.maAttribute;
            else if ( rStartPoint.Y() == rEntry.mnEndPos )
                pStyles[ 1 ] = rEntry.maAttribute;
        }
        else
        {
            if ( rStartPoint.Y() == rEntry.mnEndPos )
                pStyles[ 2 ] = rEntry.maAttribute;
            else if ( rEndPoint.Y() == rEntry.mnStartPos )
                pStyles[ 5 ] = rEntry.maAttribute;
        }
        ++aIter;
    }

    aMapIter = maHoriLines.find( rStartPoint.Y() );
    OSL_ENSURE( aMapIter != maHoriLines.end(), "FindStylesForLine: Error" );
    const SwLineEntrySet& rHoriSet = (*aMapIter).second;
    aIter = rHoriSet.begin();

    while ( aIter != rHoriSet.end() )
    {
        const SwLineEntry& rEntry = *aIter;
        if ( bHori )
        {
            if ( rStartPoint.X() == rEntry.mnEndPos )
                pStyles[ 2 ] = rEntry.maAttribute;
            else if ( rEndPoint.X() == rEntry.mnStartPos )
                pStyles[ 5 ] = rEntry.maAttribute;
        }
        else
        {
            if ( rStartPoint.X() == rEntry.mnEndPos )
                pStyles[ 1 ] = rEntry.maAttribute;
            else if ( rStartPoint.X() == rEntry.mnStartPos )
                pStyles[ 3 ] = rEntry.maAttribute;
        }
        ++aIter;
    }

    if ( bHori )
    {
        aMapIter = maVertLines.find( rEndPoint.X() );
        OSL_ENSURE( aMapIter != maVertLines.end(), "FindStylesForLine: Error" );
        const SwLineEntrySet& rVertSet2 = (*aMapIter).second;
        aIter = rVertSet2.begin();

        while ( aIter != rVertSet2.end() )
        {
            const SwLineEntry& rEntry = *aIter;
            if ( rEndPoint.Y() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
            else if ( rEndPoint.Y() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
            ++aIter;
        }
    }
    else
    {
        aMapIter = maHoriLines.find( rEndPoint.Y() );
        OSL_ENSURE( aMapIter != maHoriLines.end(), "FindStylesForLine: Error" );
        const SwLineEntrySet& rHoriSet2 = (*aMapIter).second;
        aIter = rHoriSet2.begin();

        while ( aIter != rHoriSet2.end() )
        {
            const SwLineEntry& rEntry = *aIter;
            if ( rEndPoint.X() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
            else if ( rEndPoint.X() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
            ++aIter;
        }
    }
}

// special case: #i9860#
// first line in follow table without repeated headlines
static bool lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines(
        SwTabFrm const& rTabFrm, SwFrm const& rFrm, SvxBoxItem const& rBoxItem)
{
    SwRowFrm const*const pThisRowFrm =
        dynamic_cast<const SwRowFrm*>(rFrm.GetUpper());
    return (pThisRowFrm
        && (pThisRowFrm->GetUpper() == &rTabFrm)
        && rTabFrm.IsFollow()
        && !rTabFrm.GetTable()->GetRowsToRepeat()
        &&  (  !pThisRowFrm->GetPrev()
            || static_cast<const SwRowFrm*>(pThisRowFrm->GetPrev())
                    ->IsRowSpanLine())
        && !rBoxItem.GetTop()
        && rBoxItem.GetBottom());
}

void SwTabFrmPainter::Insert( const SwFrm& rFrm, const SvxBoxItem& rBoxItem )
{
    // build 4 line entries for the 4 borders:
    SwRect aBorderRect = rFrm.Frm();
    if ( rFrm.IsTabFrm() )
    {
        aBorderRect = rFrm.Prt();
        aBorderRect.Pos() += rFrm.Frm().Pos();
    }

    bool const bBottomAsTop(lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines(
                mrTabFrm, rFrm, rBoxItem));
    bool const bVert = mrTabFrm.IsVertical();
    bool const bR2L  = mrTabFrm.IsRightToLeft();

    svx::frame::Style aL( rBoxItem.GetLeft() );
    svx::frame::Style aR( rBoxItem.GetRight() );
    svx::frame::Style aT( rBoxItem.GetTop() );
    svx::frame::Style aB( rBoxItem.GetBottom() );

    aR.MirrorSelf();
    aB.MirrorSelf();

    const SwTwips nLeft   = aBorderRect._Left();
    const SwTwips nRight  = aBorderRect._Right();
    const SwTwips nTop    = aBorderRect._Top();
    const SwTwips nBottom = aBorderRect._Bottom();

    aL.SetRefMode( svx::frame::REFMODE_CENTERED );
    aR.SetRefMode( svx::frame::REFMODE_CENTERED );
    aT.SetRefMode( !bVert ? svx::frame::REFMODE_BEGIN : svx::frame::REFMODE_END );
    aB.SetRefMode( !bVert ? svx::frame::REFMODE_BEGIN : svx::frame::REFMODE_END );

    SwLineEntry aLeft  (nLeft,   nTop,  nBottom,
            (bVert) ? aB                         : ((bR2L) ? aR : aL));
    SwLineEntry aRight (nRight,  nTop,  nBottom,
            (bVert) ? ((bBottomAsTop) ? aB : aT) : ((bR2L) ? aL : aR));
    SwLineEntry aTop   (nTop,    nLeft, nRight,
            (bVert) ? aL                         : ((bBottomAsTop) ? aB : aT));
    SwLineEntry aBottom(nBottom, nLeft, nRight,
            (bVert) ? aR                         : aB);

    Insert( aLeft, false );
    Insert( aRight, false );
    Insert( aTop, true );
    Insert( aBottom, true );
}

void SwTabFrmPainter::Insert( SwLineEntry& rNew, bool bHori )
{
    // get all lines from structure, that have key entry of pLE
    SwLineEntryMap* pLine2 = bHori ? &maHoriLines : &maVertLines;
    const SwTwips nKey = rNew.mnKey;
    SwLineEntryMapIter aMapIter = pLine2->find( nKey );

    SwLineEntrySet* pLineSet = aMapIter != pLine2->end() ? &((*aMapIter).second) : 0;
    if ( !pLineSet )
    {
        SwLineEntrySet aNewSet;
        (*pLine2)[ nKey ] = aNewSet;
        pLineSet = &(*pLine2)[ nKey ];
    }
    SwLineEntrySetIter aIter = pLineSet->begin();

    while ( pLineSet && aIter != pLineSet->end() && rNew.mnStartPos < rNew.mnEndPos )
    {
        const SwLineEntry& rOld = *aIter;
        const SwLineEntry::OverlapType nOverlapType = rOld.Overlaps( rNew );

        const svx::frame::Style& rOldAttr = rOld.maAttribute;
        const svx::frame::Style& rNewAttr = rNew.maAttribute;
        const svx::frame::Style& rCmpAttr = rNewAttr > rOldAttr ? rNewAttr : rOldAttr;

        if ( SwLineEntry::OVERLAP1 == nOverlapType )
        {
            OSL_ENSURE( rNew.mnStartPos >= rOld.mnStartPos, "Overlap type 3? How this?" );

            // new left segment
            const SwLineEntry aLeft( nKey, rOld.mnStartPos, rNew.mnStartPos, rOldAttr );

            // new middle segment
            const SwLineEntry aMiddle( nKey, rNew.mnStartPos, rOld.mnEndPos, rCmpAttr );

            // new right segment
            rNew.mnStartPos = rOld.mnEndPos;

            // update current lines set
            pLineSet->erase( aIter );
            if ( aLeft.mnStartPos   < aLeft.mnEndPos   ) pLineSet->insert( aLeft );
            if ( aMiddle.mnStartPos < aMiddle.mnEndPos ) pLineSet->insert( aMiddle );

            aIter = pLineSet->begin();

            continue; // start over
        }
        else if ( SwLineEntry::OVERLAP2 == nOverlapType )
        {
            // new left segment
            const SwLineEntry aLeft( nKey, rOld.mnStartPos, rNew.mnStartPos, rOldAttr );

            // new middle segment
            const SwLineEntry aMiddle( nKey, rNew.mnStartPos, rNew.mnEndPos, rCmpAttr );

            // new right segment
            const SwLineEntry aRight( nKey, rNew.mnEndPos, rOld.mnEndPos, rOldAttr );

            // update current lines set
            pLineSet->erase( aIter );
            if ( aLeft.mnStartPos < aLeft.mnEndPos ) pLineSet->insert( aLeft );
            if ( aMiddle.mnStartPos < aMiddle.mnEndPos ) pLineSet->insert( aMiddle );
            if ( aRight.mnStartPos < aRight.mnEndPos ) pLineSet->insert( aRight );

            rNew.mnStartPos = rNew.mnEndPos; // rNew should not be inserted!

            break; // we are finished
        }
        else if ( SwLineEntry::OVERLAP3 == nOverlapType )
        {
            // new left segment
            const SwLineEntry aLeft( nKey, rNew.mnStartPos, rOld.mnStartPos, rNewAttr );

            // new middle segment
            const SwLineEntry aMiddle( nKey, rOld.mnStartPos, rNew.mnEndPos, rCmpAttr );

            // new right segment
            const SwLineEntry aRight( nKey, rNew.mnEndPos, rOld.mnEndPos, rOldAttr );

            // update current lines set
            pLineSet->erase( aIter );
            if ( aLeft.mnStartPos < aLeft.mnEndPos ) pLineSet->insert( aLeft );
            if ( aMiddle.mnStartPos < aMiddle.mnEndPos ) pLineSet->insert( aMiddle );
            if ( aRight.mnStartPos < aRight.mnEndPos ) pLineSet->insert( aRight );

            rNew.mnStartPos = rNew.mnEndPos; // rNew should not be inserted!

            break; // we are finished
        }

        ++aIter;
    }

    if ( rNew.mnStartPos < rNew.mnEndPos ) // insert rest
        pLineSet->insert( rNew );
}

//
// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES END
//

// --> OD #i76669#
namespace
{
    class SwViewObjectContactRedirector : public ::sdr::contact::ViewObjectContactRedirector
    {
        private:
            const ViewShell& mrViewShell;

        public:
            SwViewObjectContactRedirector( const ViewShell& rSh )
                : mrViewShell( rSh )
            {};

            virtual ~SwViewObjectContactRedirector()
            {}

            virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
                                    const sdr::contact::ViewObjectContact& rOriginal,
                                    const sdr::contact::DisplayInfo& rDisplayInfo)
            {
                sal_Bool bPaint( sal_True );

                SdrObject* pObj = rOriginal.GetViewContact().TryToGetSdrObject();
                if ( pObj )
                {
                    bPaint = SwFlyFrm::IsPaint( pObj, &mrViewShell );
                }

                if ( !bPaint )
                {
                    return drawinglayer::primitive2d::Primitive2DSequence();
                }

                return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
                                                        rOriginal, rDisplayInfo );
            }
    };

} // end of anonymous namespace
// <--
/*************************************************************************
|*
|*  SwRootFrm::Paint()
|*
|*  Description
|*      Paint once for every visible page which is touched by Rect.
|*      1. Paint borders and backgrounds.
|*      2. Paint the draw layer (frames and drawing objects) that is
|*         below the document (hell).
|*      3. Paint the document content (text)
|*      4. Paint the draw layer that is above the document.
|*************************************************************************/

void
SwRootFrm::Paint(SwRect const& rRect, SwPrintData const*const pPrintData) const
{
    OSL_ENSURE( Lower() && Lower()->IsPageFrm(), "Lower of root is no page." );

    PROTOCOL( this, PROT_FILE_INIT, 0, 0)

    bool bResetRootPaint = false;
    ViewShell *pSh = pCurrShell;

    if ( pSh->GetWin() )
    {
        if ( pSh->GetOut() == pSh->GetWin() && !pSh->GetWin()->IsVisible() )
        {
            return;
        }
        if ( SwRootFrm::bInPaint )
        {
            SwPaintQueue::Add( pSh, rRect );
            return;
        }
    }
    else
        SwRootFrm::bInPaint = bResetRootPaint = true;

    SwSavePaintStatics *pStatics = 0;
    if ( pGlobalShell )
        pStatics = new SwSavePaintStatics();
    pGlobalShell = pSh;

    if( !pSh->GetWin() )
        pProgress = SfxProgress::GetActiveProgress( (SfxObjectShell*) pSh->GetDoc()->GetDocShell() );

    ::SwCalcPixStatics( pSh->GetOut() );
    aGlobalRetoucheColor = pSh->Imp()->GetRetoucheColor();

    //Trigger an action to clear things up if needed.
    //Using this trick we can ensure that all values are valid in all paints -
    //no problems, no special case(s).
    // #i92745#
    // Extend check on certain states of the 'current' <ViewShell> instance to
    // all existing <ViewShell> instances.
    bool bPerformLayoutAction( true );
    {
        ViewShell* pTmpViewShell = pSh;
        do {
            if ( pTmpViewShell->IsInEndAction() ||
                 pTmpViewShell->IsPaintInProgress() ||
                 ( pTmpViewShell->Imp()->IsAction() &&
                   pTmpViewShell->Imp()->GetLayAction().IsActionInProgress() ) )
            {
                bPerformLayoutAction = false;
            }

            pTmpViewShell = static_cast<ViewShell*>(pTmpViewShell->GetNext());
        } while ( bPerformLayoutAction && pTmpViewShell != pSh );
    }
    if ( bPerformLayoutAction )
    {
        ((SwRootFrm*)this)->ResetTurbo();
        SwLayAction aAction( (SwRootFrm*)this, pSh->Imp() );
        aAction.SetPaint( sal_False );
        aAction.SetComplete( sal_False );
        aAction.SetReschedule( pProgress ? sal_True : sal_False );
        aAction.Action();
        ((SwRootFrm*)this)->ResetTurboFlag();
        if ( !pSh->ActionPend() )
            pSh->Imp()->DelRegion();
    }

    SwRect aRect( rRect );
    aRect.Intersection( pSh->VisArea() );

    const bool bExtraData = ::IsExtraData( GetFmt()->GetDoc() );

    pLines = new SwLineRects;   //Container for borders.

    // #104289#. During painting, something (OLE) can
    // load the linguistic, which in turn can cause a reformat
    // of the document. Dangerous! We better set this flag to
    // avoid the reformat.
    const sal_Bool bOldAction = IsCallbackActionEnabled();
    ((SwRootFrm*)this)->SetCallbackActionEnabled( sal_False );

    const SwPageFrm *pPage = pSh->Imp()->GetFirstVisPage();

    const bool bBookMode = pGlobalShell->GetViewOptions()->IsViewLayoutBookMode();
    if ( bBookMode && pPage->GetPrev() && static_cast<const SwPageFrm*>(pPage->GetPrev())->IsEmptyPage() )
        pPage = static_cast<const SwPageFrm*>(pPage->GetPrev());

    // #i68597#
    const bool bGridPainting(pSh->GetWin() && pSh->Imp()->HasDrawView() && pSh->Imp()->GetDrawView()->IsGridVisible());

    // Hide all page break controls before showing them again
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pGlobalShell );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
        SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();
        const SwPageFrm* pHiddenPage = pPage;
        while ( pHiddenPage->GetPrev() != NULL )
        {
            pHiddenPage = static_cast< const SwPageFrm* >( pHiddenPage->GetPrev() );
            SwFrameControlPtr pControl = rMngr.GetControl( PageBreak, pHiddenPage );
            if ( pControl.get() )
                pControl->ShowAll( false );
        }
    }

    // #i76669#
    SwViewObjectContactRedirector aSwRedirector( *pSh );

    while ( pPage )
    {
        const bool bPaintRightShadow =  pPage->IsRightShadowNeeded();
        const bool bPaintLeftShadow = pPage->IsLeftShadowNeeded();
        const bool bRightSidebar = pPage->SidebarPosition() == sw::sidebarwindows::SIDEBAR_RIGHT;

        if ( !pPage->IsEmptyPage() )
        {
            SwRect aPaintRect;
            SwPageFrm::GetBorderAndShadowBoundRect( pPage->Frm(), pSh, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

            if ( aRect.IsOver( aPaintRect ) )
            {
                if ( pSh->GetWin() )
                {
                    pSubsLines = new SwSubsRects;
                    pSpecSubsLines = new SwSubsRects;
                }
                g_pBorderLines = new BorderLines;

                aPaintRect._Intersection( aRect );

                if ( bExtraData &&
                     pSh->GetWin() && pSh->IsInEndAction() )
                {
                    // enlarge paint rectangle to complete page width, subtract
                    // current paint area and invalidate the resulting region.
                    SWRECTFN( pPage )
                    SwRect aPageRectTemp( aPaintRect );
                    (aPageRectTemp.*fnRect->fnSetLeftAndWidth)(
                         (pPage->Frm().*fnRect->fnGetLeft)(),
                         (pPage->Frm().*fnRect->fnGetWidth)() );
                    aPageRectTemp._Intersection( pSh->VisArea() );
                    Region aPageRectRegion( aPageRectTemp.SVRect() );
                    aPageRectRegion.Exclude( aPaintRect.SVRect() );
                    pSh->GetWin()->Invalidate( aPageRectRegion, INVALIDATE_CHILDREN );
                }

                // #i80793#
                // enlarge paint rectangle for objects overlapping the same pixel
                // in all cases and before the DrawingLayer overlay is initialized.
                lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );

                // #i68597#
                // moved paint pre-process for DrawingLayer overlay here since the above
                // code dependent from bExtraData may expand the PaintRect
                {
                    // #i75172# if called from ViewShell::ImplEndAction it sould no longer
                    // really be used but handled by ViewShell::ImplEndAction already
                    const Region aDLRegion(aPaintRect.SVRect());
                    pSh->DLPrePaint2(aDLRegion);
                }

                if(OUTDEV_WINDOW == pGlobalShell->GetOut()->GetOutDevType())
                {
                    /// OD 27.09.2002 #103636# - changed method SwLayVout::Enter(..)
                    /// 2nd parameter is no longer <const> and will be set to the
                    /// rectangle the virtual output device is calculated from <aPaintRect>,
                    /// if the virtual output is used.
                    pVout->Enter( pSh, aPaintRect, !bNoVirDev );

                    /// OD 27.09.2002 #103636# - adjust paint rectangle to pixel size
                    /// Thus, all objects overlapping on pixel level with the unadjusted
                    /// paint rectangle will be considered in the paint.
                    lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );
                }

                // maybe this can be put in the above scope. Since we are not sure, just leave it ATM
                pVout->SetOrgRect( aPaintRect );

                /// OD 29.08.2002 #102450#
                /// determine background color of page for <PaintLayer> method
                /// calls, paint <hell> or <heaven>
                const Color aPageBackgrdColor = pPage->GetDrawBackgrdColor();

                pPage->PaintBaBo( aPaintRect, pPage, sal_True );

                if ( pSh->Imp()->HasDrawView() )
                {
                    pLines->LockLines( sal_True );
                    const IDocumentDrawModelAccess* pIDDMA = pSh->getIDocumentDrawModelAccess();
                    pSh->Imp()->PaintLayer( pIDDMA->GetHellId(),
                                            pPrintData,
                                            pPage->Frm(),
                                            &aPageBackgrdColor,
                                            (pPage->IsRightToLeft() ? true : false),
                                            &aSwRedirector );
                    pLines->PaintLines( pSh->GetOut() );
                    pLines->LockLines( sal_False );
                }

                if ( pSh->GetDoc()->get( IDocumentSettingAccess::BACKGROUND_PARA_OVER_DRAWINGS ) )
                    pPage->PaintBaBo( aPaintRect, pPage, sal_True, /*bOnlyTxtBackground=*/true );

                if( pSh->GetWin() )
                {
                    // collect sub-lines
                    pPage->RefreshSubsidiary( aPaintRect );
                    // paint special sub-lines
                    pSpecSubsLines->PaintSubsidiary( pSh->GetOut(), NULL );
                }

                pPage->Paint( aPaintRect );

                // no paint of page border and shadow, if writer is in place mode.
                if( pSh->GetWin() && pSh->GetDoc()->GetDocShell() &&
                    !pSh->GetDoc()->GetDocShell()->IsInPlaceActive() )
                {
                    SwPageFrm::PaintBorderAndShadow( pPage->Frm(), pSh, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
                    SwPageFrm::PaintNotesSidebar( pPage->Frm(), pSh, pPage->GetPhyPageNum(), bRightSidebar);
                }

                pLines->PaintLines( pSh->GetOut() );
                if ( pSh->GetWin() )
                {
                    pSubsLines->PaintSubsidiary( pSh->GetOut(), pLines );
                    DELETEZ( pSubsLines );
                    DELETEZ( pSpecSubsLines );
                }
                // fdo#42750: delay painting these until after subsidiary lines
                // fdo#45562: delay painting these until after hell layer
                // fdo#47717: but do it before heaven layer
                ProcessPrimitives(g_pBorderLines->GetBorderLines_Clear());

                if ( pSh->Imp()->HasDrawView() )
                {
                    /// OD 29.08.2002 #102450# - add 3rd parameter
                    // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
                    pSh->Imp()->PaintLayer( pSh->GetDoc()->GetHeavenId(),
                                            pPrintData,
                                            pPage->Frm(),
                                            &aPageBackgrdColor,
                                            (pPage->IsRightToLeft() ? true : false),
                                            &aSwRedirector );
                }

                if ( bExtraData )
                    pPage->RefreshExtraData( aPaintRect );

                DELETEZ(g_pBorderLines);
                pVout->Leave();

                // #i68597#
                // needed to move grid painting inside Begin/EndDrawLayer bounds and to change
                // output rect for it accordingly
                if(bGridPainting)
                {
                    SdrPaintView* pPaintView = pSh->Imp()->GetDrawView();
                    SdrPageView* pPageView = pPaintView->GetSdrPageView();
                    pPageView->DrawPageViewGrid(*pSh->GetOut(), aPaintRect.SVRect(), SwViewOption::GetTextGridColor() );
                }

                // #i68597#
                // moved paint post-process for DrawingLayer overlay here, see above
                {
                    pSh->DLPostPaint2(true);
                }
            }

            pPage->PaintDecorators( );
            pPage->PaintBreak();
        }
        else if ( bBookMode && pSh->GetWin() && !pSh->GetDoc()->GetDocShell()->IsInPlaceActive() )
        {
            // paint empty page
            SwRect aPaintRect;
            SwRect aEmptyPageRect( pPage->Frm() );

            // code from vprint.cxx
            const SwPageFrm& rFormatPage = pPage->GetFormatPage();
            aEmptyPageRect.SSize() = rFormatPage.Frm().SSize();

            SwPageFrm::GetBorderAndShadowBoundRect( aEmptyPageRect, pSh, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
            aPaintRect._Intersection( aRect );

            if ( aRect.IsOver( aEmptyPageRect ) )
            {
                // #i75172# if called from ViewShell::ImplEndAction it sould no longer
                // really be used but handled by ViewShell::ImplEndAction already
                {
                    const Region aDLRegion(aPaintRect.SVRect());
                    pSh->DLPrePaint2(aDLRegion);
                }

                if( pSh->GetOut()->GetFillColor() != aGlobalRetoucheColor )
                    pSh->GetOut()->SetFillColor( aGlobalRetoucheColor );

                pSh->GetOut()->SetLineColor(); // OD 20.02.2003 #107369# - no line color
                // OD 20.02.2003 #107369# - use aligned page rectangle
                {
                    SwRect aTmpPageRect( aEmptyPageRect );
                    ::SwAlignRect( aTmpPageRect, pSh );
                    aEmptyPageRect = aTmpPageRect;
                }

                pSh->GetOut()->DrawRect( aEmptyPageRect.SVRect() );

                // paint empty page text
                const Font& rEmptyPageFont = SwPageFrm::GetEmptyPageFont();
                const Font aOldFont( pSh->GetOut()->GetFont() );

                pSh->GetOut()->SetFont( rEmptyPageFont );
                pSh->GetOut()->DrawText( aEmptyPageRect.SVRect(), SW_RESSTR( STR_EMPTYPAGE ),
                                    TEXT_DRAW_VCENTER |
                                    TEXT_DRAW_CENTER |
                                    TEXT_DRAW_CLIP );

                pSh->GetOut()->SetFont( aOldFont );
                // paint shadow and border for empty page
                // OD 19.02.2003 #107369# - use new method to paint page border and
                // shadow
                SwPageFrm::PaintBorderAndShadow( aEmptyPageRect, pSh, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
                SwPageFrm::PaintNotesSidebar( aEmptyPageRect, pSh, pPage->GetPhyPageNum(), bRightSidebar);

                {
                    pSh->DLPostPaint2(true);
                }
            }
        }

        OSL_ENSURE( !pPage->GetNext() || pPage->GetNext()->IsPageFrm(),
                "Neighbour of page is not a page." );
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    DELETEZ( pLines );

    if ( bResetRootPaint )
        SwRootFrm::bInPaint = false;
    if ( pStatics )
        delete pStatics;
    else
    {
        pProgress = 0;
        pGlobalShell = 0;
    }

    ((SwRootFrm*)this)->SetCallbackActionEnabled( bOldAction );
}

/*************************************************************************
|*
|*  SwLayoutFrm::Paint()
|*
|*************************************************************************/

static void lcl_EmergencyFormatFtnCont( SwFtnContFrm *pCont )
{
    //It's possible that the Cont will get destroyed.
    SwCntntFrm *pCnt = pCont->ContainsCntnt();
    while ( pCnt && pCnt->IsInFtn() )
    {
        pCnt->Calc();
        pCnt = pCnt->GetNextCntntFrm();
    }
}

class SwShortCut
{
    SwRectDist fnCheck;
    long nLimit;
public:
    SwShortCut( const SwFrm& rFrm, const SwRect& rRect );
    bool Stop( const SwRect& rRect ) const
        { return (rRect.*fnCheck)( nLimit ) > 0; }
};

SwShortCut::SwShortCut( const SwFrm& rFrm, const SwRect& rRect )
{
    sal_Bool bVert = rFrm.IsVertical();
    sal_Bool bR2L = rFrm.IsRightToLeft();
    if( rFrm.IsNeighbourFrm() && bVert == bR2L )
    {
        if( bVert )
        {
            fnCheck = &SwRect::GetBottomDistance;
            nLimit = rRect.Top();
        }
        else
        {
            fnCheck = &SwRect::GetLeftDistance;
            nLimit = rRect.Left() + rRect.Width();
        }
    }
    else if( bVert == rFrm.IsNeighbourFrm() )
    {
        fnCheck = &SwRect::GetTopDistance;
        nLimit = rRect.Top() + rRect.Height();
    }
    else
    {
        //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        if ( rFrm.IsVertLR() )
        {
               fnCheck = &SwRect::GetLeftDistance;
               nLimit = rRect.Right();
        }
        else
        {
            fnCheck = &SwRect::GetRightDistance;
            nLimit = rRect.Left();
        }
    }
}

void SwLayoutFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    ViewShell *pSh = getRootFrm()->GetCurrShell();

    // #i16816# tagged pdf support
    Frm_Info aFrmInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelper( 0, &aFrmInfo, 0, *pSh->GetOut() );

    const SwFrm *pFrm = Lower();
    if ( !pFrm )
        return;

    SwShortCut aShortCut( *pFrm, rRect );
    sal_Bool bCnt;
    if ( sal_True == (bCnt = pFrm->IsCntntFrm()) )
        pFrm->Calc();

    if ( pFrm->IsFtnContFrm() )
    {
        ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm );
        pFrm = Lower();
    }

    const SwPageFrm *pPage = 0;
    const bool bWin   = pGlobalShell->GetWin() ? true : false;

    while ( IsAnLower( pFrm ) )
    {
        SwRect aPaintRect( pFrm->PaintArea() );
        if( aShortCut.Stop( aPaintRect ) )
            break;
        if ( bCnt && pProgress )
            pProgress->Reschedule();

        //We need to retouch if a frame explicitly requests it.
        //First do the retouch, because this could flatten the borders.
        if ( pFrm->IsRetouche() )
        {
            if ( pFrm->IsRetoucheFrm() && bWin && !pFrm->GetNext() )
            {   if ( !pPage )
                    pPage = FindPageFrm();
               pFrm->Retouche( pPage, rRect );
            }
            pFrm->ResetRetouche();
        }

        if ( rRect.IsOver( aPaintRect ) )
        {
            if ( bCnt && pFrm->IsCompletePaint() &&
                 !rRect.IsInside( aPaintRect ) && GetpApp()->AnyInput( VCL_INPUT_KEYBOARD ) )
            {
                //fix(8104): It may happen, that the processing wasn't complete
                //but some parts of the paragraph were still repainted.
                //This could lead to the situation, that other parts of the
                //paragraph won't be repainted at all. The only solution seems
                //to be an invalidation of the window.
                //To not make it too severe the rectangle is limited by
                //painting the desired part and only invalidating the
                //remaining paragraph parts.
                if ( aPaintRect.Left()  == rRect.Left() &&
                     aPaintRect.Right() == rRect.Right() )
                {
                    aPaintRect.Bottom( rRect.Top() - 1 );
                    if ( aPaintRect.Height() > 0 )
                        pGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( rRect.Bottom() + 1 );
                    aPaintRect.Bottom( pFrm->Frm().Bottom() );
                    if ( aPaintRect.Height() > 0 )
                        pGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( pFrm->Frm().Top() );
                    aPaintRect.Bottom( pFrm->Frm().Bottom() );
                }
                else
                {
                    pGlobalShell->InvalidateWindows( aPaintRect );
                    pFrm = pFrm->GetNext();
                    if ( pFrm && (sal_True == (bCnt = pFrm->IsCntntFrm())) )
                        pFrm->Calc();
                    continue;
                }
            }
            pFrm->ResetCompletePaint();
            aPaintRect._Intersection( rRect );

            pFrm->Paint( aPaintRect );

            if ( Lower() && Lower()->IsColumnFrm() )
            {
                //Paint the column separator line if needed. The page is
                //responsible for the page frame - not the upper.
                const SwFrmFmt *pFmt = GetUpper() && GetUpper()->IsPageFrm()
                                            ? GetUpper()->GetFmt()
                                            : GetFmt();
                const SwFmtCol &rCol = pFmt->GetCol();
                if ( rCol.GetLineAdj() != COLADJ_NONE )
                {
                    if ( !pPage )
                        pPage = pFrm->FindPageFrm();

                    PaintColLines( aPaintRect, rCol, pPage );
                }
            }
        }
        if ( !bCnt && pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm() )
            ::lcl_EmergencyFormatFtnCont( (SwFtnContFrm*)pFrm->GetNext() );

        pFrm = pFrm->GetNext();

        if ( pFrm && (sal_True == (bCnt = pFrm->IsCntntFrm())) )
            pFrm->Calc();
    }
}

static drawinglayer::primitive2d::Primitive2DSequence lcl_CreateDashedIndicatorPrimitive(
        basegfx::B2DPoint aStart, basegfx::B2DPoint aEnd,
        basegfx::BColor aColor )
{
    drawinglayer::primitive2d::Primitive2DSequence aSeq( 1 );

    std::vector< double > aStrokePattern;
    basegfx::B2DPolygon aLinePolygon;
    aLinePolygon.append( aStart );
    aLinePolygon.append( aEnd );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    if ( rSettings.GetHighContrastMode( ) )
    {
        // Only a solid line in high contrast mode
        aColor = rSettings.GetDialogTextColor().getBColor();
    }
    else
    {
        // Get a color for the contrast
        basegfx::BColor aHslLine = basegfx::tools::rgb2hsl( aColor );
        double nLuminance = aHslLine.getZ() * 2.5;
        if ( nLuminance == 0 )
            nLuminance = 0.5;
        else if ( nLuminance >= 1.0 )
            nLuminance = aHslLine.getZ() * 0.4;
        aHslLine.setZ( nLuminance );
        const basegfx::BColor aOtherColor = basegfx::tools::hsl2rgb( aHslLine );

        // Compute the plain line
        drawinglayer::primitive2d::PolygonHairlinePrimitive2D * pPlainLine =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aLinePolygon, aOtherColor );

        aSeq[0] = drawinglayer::primitive2d::Primitive2DReference( pPlainLine );


        // Dashed line in twips
        aStrokePattern.push_back( 40 );
        aStrokePattern.push_back( 40 );

        aSeq.realloc( 2 );
    }

    // Compute the dashed line primitive
    drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D * pLine =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D (
                basegfx::B2DPolyPolygon( aLinePolygon ),
                drawinglayer::attribute::LineAttribute( aColor ),
                drawinglayer::attribute::StrokeAttribute( aStrokePattern ) );

    aSeq[ aSeq.getLength( ) - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pLine );

    return aSeq;
}

void SwPageFrm::PaintBreak( ) const
{
    if ( pGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER  &&
         !pGlobalShell->GetViewOptions()->IsPDFExport() &&
         !pGlobalShell->GetViewOptions()->IsReadonly() &&
         !pGlobalShell->IsPreView() )
    {
        const SwFrm* pBodyFrm = Lower();
        while ( pBodyFrm && !pBodyFrm->IsBodyFrm() )
            pBodyFrm = pBodyFrm->GetNext();

        if ( pBodyFrm )
        {
            const SwLayoutFrm* pLayBody = static_cast< const SwLayoutFrm* >( pBodyFrm );
            const SwFlowFrm *pFlowFrm = pLayBody->ContainsCntnt();

            // Test if the first node is a table
            const SwFrm* pFirstFrm = pLayBody->Lower();
            if ( pFirstFrm && pFirstFrm->IsTabFrm() )
                pFlowFrm = static_cast< const SwTabFrm* >( pFirstFrm );

            SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pGlobalShell );
            if ( pWrtSh )
            {
                SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
                SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();

                if ( pFlowFrm && pFlowFrm->IsPageBreak( sal_True ) )
                    rMngr.SetPageBreakControl( this );
                else
                    rMngr.RemoveControlsByType( PageBreak, this );
            }
        }
        SwLayoutFrm::PaintBreak( );
    }
}

void SwColumnFrm::PaintBreak( ) const
{
    if ( pGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER  &&
         !pGlobalShell->GetViewOptions()->IsPDFExport() &&
         !pGlobalShell->GetViewOptions()->IsReadonly() &&
         !pGlobalShell->IsPreView() )
    {
        const SwFrm* pBodyFrm = Lower();
        while ( pBodyFrm && !pBodyFrm->IsBodyFrm() )
            pBodyFrm = pBodyFrm->GetNext();

        if ( pBodyFrm )
        {
            const SwCntntFrm *pCnt = static_cast< const SwLayoutFrm* >( pBodyFrm )->ContainsCntnt();
            if ( pCnt && pCnt->IsColBreak( sal_True ) )
            {
                // Paint the break only if:
                //    * Not in header footer edition, to avoid conflicts with the
                //      header/footer marker
                //    * Non-printing characters are shown, as this is more consistent
                //      with other formatting marks
                if ( !pGlobalShell->IsShowHeaderFooterSeparator( Header ) &&
                     !pGlobalShell->IsShowHeaderFooterSeparator( Footer ) )
                {
                    SwRect aRect( pCnt->Prt() );
                    aRect.Pos() += pCnt->Frm().Pos();

                    // Draw the line
                    basegfx::B2DPoint aStart( double( aRect.Left() ), aRect.Top() );
                    basegfx::B2DPoint aEnd( double( aRect.Right() ), aRect.Top() );
                    double nWidth = aRect.Width();
                    if ( IsVertical( ) )
                    {
                        aStart = basegfx::B2DPoint( double( aRect.Right() ), double( aRect.Top() ) );
                        aEnd = basegfx::B2DPoint( double( aRect.Right() ), double( aRect.Bottom() ) );
                        nWidth = aRect.Height();
                    }

                    basegfx::BColor aLineColor = SwViewOption::GetPageBreakColor().getBColor();


                    drawinglayer::primitive2d::Primitive2DSequence aSeq =
                        lcl_CreateDashedIndicatorPrimitive( aStart, aEnd, aLineColor );
                    aSeq.realloc( aSeq.getLength( ) + 1 );

                    // Add the text above
                    OUString aBreakText = SW_RESSTR(STR_COLUMN_BREAK);

                    basegfx::B2DVector aFontSize;
                    OutputDevice* pOut = pGlobalShell->GetOut();
                    Font aFont = pOut->GetSettings().GetStyleSettings().GetToolFont();
                    aFont.SetHeight( 8 * 20 );
                    pOut->SetFont( aFont );
                    drawinglayer::attribute::FontAttribute aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(
                            aFontSize, aFont, false, false );

                    Rectangle aTextRect;
                    pOut->GetTextBoundRect( aTextRect, String( aBreakText ) );
                    long nTextOff = ( nWidth - aTextRect.GetWidth() ) / 2;

                    basegfx::B2DHomMatrix aTextMatrix( basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aFontSize.getX(), aFontSize.getY(),
                                aRect.Left() + nTextOff, aRect.Top() ) );
                    if ( IsVertical() )
                    {
                        aTextMatrix = basegfx::B2DHomMatrix( basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix (
                                aFontSize.getX(), aFontSize.getY(), 0.0, M_PI_2,
                                aRect.Right(), aRect.Top() + nTextOff ) );
                    }

                    drawinglayer::primitive2d::TextSimplePortionPrimitive2D * pText =
                            new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                                aTextMatrix,
                                aBreakText, 0, aBreakText.getLength(),
                                std::vector< double >(),
                                aFontAttr,
                                lang::Locale(),
                                aLineColor );
                    aSeq[ aSeq.getLength() - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pText );

                    ProcessPrimitives( aSeq );
                }
            }
        }
    }
}

void SwLayoutFrm::PaintBreak( ) const
{
    const SwFrm* pFrm = Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            static_cast< const SwLayoutFrm*>( pFrm )->PaintBreak( );
        pFrm = pFrm->GetNext();
    }
}

void SwPageFrm::PaintDecorators( ) const
{
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pGlobalShell );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();

        const SwLayoutFrm* pBody = FindBodyCont();
        if ( pBody )
        {
            SwRect aBodyRect( pBody->Frm() );

            if ( pGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER &&
                 !pGlobalShell->GetViewOptions()->IsPDFExport() &&
                 !pGlobalShell->IsPreView() &&
                 !pGlobalShell->GetViewOptions()->IsReadonly() &&
                 !pGlobalShell->GetViewOptions()->getBrowseMode() &&
                 ( pGlobalShell->IsShowHeaderFooterSeparator( Header ) ||
                   pGlobalShell->IsShowHeaderFooterSeparator( Footer ) ) )
            {
                bool bRtl = Application::GetSettings().GetLayoutRTL();
                const SwRect& rVisArea = pGlobalShell->VisArea();
                long nXOff = std::min( aBodyRect.Right(), rVisArea.Right() );
                if ( bRtl )
                    nXOff = std::max( aBodyRect.Left(), rVisArea.Left() );

                // Header
                if ( pGlobalShell->IsShowHeaderFooterSeparator( Header ) )
                {
                    const SwFrm* pHeaderFrm = Lower();
                    if ( !pHeaderFrm->IsHeaderFrm() )
                        pHeaderFrm = NULL;

                    long nHeaderYOff = aBodyRect.Top();
                    Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nHeaderYOff ) );
                    rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, Header, nOutputOff );
                }

                // Footer
                if ( pGlobalShell->IsShowHeaderFooterSeparator( Footer ) )
                {
                    const SwFrm* pFtnContFrm = Lower();
                    while ( pFtnContFrm )
                    {
                        if ( pFtnContFrm->IsFtnContFrm() )
                            aBodyRect.AddBottom( pFtnContFrm->Frm().Bottom() - aBodyRect.Bottom() );
                        pFtnContFrm = pFtnContFrm->GetNext();
                    }

                    long nFooterYOff = aBodyRect.Bottom();
                    Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nFooterYOff ) );
                    rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, Footer, nOutputOff );
                }
            }
        }
    }
}

/** FlyFrm::IsBackgroundTransparent - for feature #99657#

    OD 12.08.2002
    determines, if background of fly frame has to be drawn transparent
    declaration found in /core/inc/flyfrm.cxx
    OD 08.10.2002 #103898# - If the background of the fly frame itself is not
    transparent and the background is inherited from its parent/grandparent,
    the background brush, used for drawing, has to be investigated for transparency.

    @author OD

    @return true, if background is transparent drawn.
*/
sal_Bool SwFlyFrm::IsBackgroundTransparent() const
{
    sal_Bool bBackgroundTransparent = GetFmt()->IsBackgroundTransparent();
    if ( !bBackgroundTransparent &&
         static_cast<const SwFlyFrmFmt*>(GetFmt())->IsBackgroundBrushInherited() )
    {
        const SvxBrushItem* pBackgrdBrush = 0;
        const XFillStyleItem* pFillStyleItem = 0;
        const XFillGradientItem* pFillGradientItem = 0;
        const Color* pSectionTOXColor = 0;
        SwRect aDummyRect;
        if ( GetBackgroundBrush( pBackgrdBrush, pFillStyleItem, pFillGradientItem, pSectionTOXColor, aDummyRect, false) )
        {
            if ( pSectionTOXColor &&
                 (pSectionTOXColor->GetTransparency() != 0) &&
                 (pSectionTOXColor->GetColor() != COL_TRANSPARENT) )
            {
                bBackgroundTransparent = sal_True;
            }
            else if ( pBackgrdBrush )
            {
                if ( (pBackgrdBrush->GetColor().GetTransparency() != 0) &&
                     (pBackgrdBrush->GetColor() != COL_TRANSPARENT) )
                {
                    bBackgroundTransparent = sal_True;
                }
                else
                {
                    const GraphicObject *pTmpGrf =
                            static_cast<const GraphicObject*>(pBackgrdBrush->GetGraphicObject());
                    if ( (pTmpGrf) &&
                         (pTmpGrf->GetAttr().GetTransparency() != 0)
                       )
                    {
                        bBackgroundTransparent = sal_True;
                    }
                }
            }
        }
    }

    return bBackgroundTransparent;
};

/** FlyFrm::IsShadowTransparent - for feature #99657#

    OD 13.08.2002
    determine, if shadow color of fly frame has to be drawn transparent
    declaration found in /core/inc/flyfrm.cxx

    @author OD

    @return true, if shadow color is transparent.
*/
sal_Bool SwFlyFrm::IsShadowTransparent() const
{
    return GetFmt()->IsShadowTransparent();
};

/*************************************************************************
|*
|*  SwFlyFrm::IsPaint()
|*
|*************************************************************************/

sal_Bool SwFlyFrm::IsPaint( SdrObject *pObj, const ViewShell *pSh )
{
    SdrObjUserCall *pUserCall;

    if ( 0 == ( pUserCall = GetUserCall(pObj) ) )
        return sal_True;

    //Attribute dependant, don't paint for printer or PreView
    sal_Bool bPaint =  pFlyOnlyDraw ||
                       ((SwContact*)pUserCall)->GetFmt()->GetPrint().GetValue();
    if ( !bPaint )
        bPaint = pSh->GetWin() && !pSh->IsPreView();

    if ( bPaint )
    {
        //The paint may be prevented by the superior Flys.
        SwFrm *pAnch = 0;
        if ( pObj->ISA(SwFlyDrawObj) ) // i#117962#
        {
            bPaint = false;
        }
        if ( pObj->ISA(SwVirtFlyDrawObj) )
        {
            SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
            if ( pFlyOnlyDraw && pFlyOnlyDraw == pFly )
                return sal_True;

            //Try to avoid displaying the intermediate stage, Flys which don't
            //overlap with the page on which they are anchored won't be
            //painted.
            //HACK: exception: printing of frames in tables, those can overlap
            //a page once in a while when dealing with oversized tables (HTML).
            SwPageFrm *pPage = pFly->FindPageFrm();
            if ( pPage )
            {
                if ( pPage->Frm().IsOver( pFly->Frm() ) )
                    pAnch = pFly->AnchorFrm();
                else if ( bTableHack &&
                          pFly->Frm().Top() >= pFly->GetAnchorFrm()->Frm().Top() &&
                          pFly->Frm().Top() < pFly->GetAnchorFrm()->Frm().Bottom() &&
                          sal_IntPtr(pSh->GetOut()) ==
                          sal_IntPtr(pSh->getIDocumentDeviceAccess()->getPrinter( false ) ) )
                {
                    pAnch = pFly->AnchorFrm();
                }
            }

        }
        else
        {
            // OD 13.10.2003 #i19919# - consider 'virtual' drawing objects
            // OD 2004-03-29 #i26791#
            SwDrawContact* pDrawContact = dynamic_cast<SwDrawContact*>(pUserCall);
            pAnch = pDrawContact ? pDrawContact->GetAnchorFrm(pObj) : NULL;
            if ( pAnch )
            {
                if ( !pAnch->GetValidPosFlag() )
                    pAnch = 0;
                else if ( sal_IntPtr(pSh->GetOut()) == sal_IntPtr(pSh->getIDocumentDeviceAccess()->getPrinter( false )))
                {
                    //HACK: we have to omit some of the objects for printing,
                    //otherwise they would be printed twice.
                    //The objects should get printed if the TableHack is active
                    //right now. Afterwards they must not be printed if the
                    //page over which they float position wise gets printed.
                    const SwPageFrm *pPage = pAnch->FindPageFrm();
                    if ( !bTableHack &&
                         !pPage->Frm().IsOver( pObj->GetCurrentBoundRect() ) )
                        pAnch = 0;
                }
            }
            else
            {
                // OD 02.07.2003 #108784# - debug assert
                if ( !pObj->ISA(SdrObjGroup) )
                {
                    OSL_FAIL( "<SwFlyFrm::IsPaint(..)> - paint of drawing object without anchor frame!?" );
                }
            }
        }
        if ( pAnch )
        {
            if ( pAnch->IsInFly() )
                bPaint = SwFlyFrm::IsPaint( pAnch->FindFlyFrm()->GetVirtDrawObj(),
                                            pSh );
            else if ( pFlyOnlyDraw )
                bPaint = sal_False;
        }
        else
            bPaint = sal_False;
    }
    return bPaint;
}

/*************************************************************************
|*  SwCellFrm::Paint( const SwRect& ) const
|*************************************************************************/
void SwCellFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    if ( GetLayoutRowSpan() >= 1 )
        SwLayoutFrm::Paint( rRect );
}

/*************************************************************************
|*
|*  SwFlyFrm::Paint()
|*
|*************************************************************************/

struct BorderLinesGuard
{
    explicit BorderLinesGuard() : m_pBorderLines(g_pBorderLines)
    {
        g_pBorderLines = new BorderLines;
    }
    ~BorderLinesGuard()
    {
        delete g_pBorderLines;
        g_pBorderLines = m_pBorderLines;
    }
private:
    BorderLines *const m_pBorderLines;
};

void SwFlyFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    //optimize thumbnail generation and store procedure to improve odt saving performance, #i120030#
    ViewShell *pShell = getRootFrm()->GetCurrShell();
    if (pShell && pShell->GetDoc() && pShell->GetDoc()->GetDocShell())
    {
        sal_Bool bInGenerateThumbnail = pShell->GetDoc()->GetDocShell()->IsInGenerateAndStoreThumbnail();
        if (bInGenerateThumbnail)
        {
            SwRect aVisRect = pShell->VisArea();
            if (!aVisRect.IsOver(Frm()))
                return;
        }
    }

    //because of the overlapping of frames and drawing objects the flys have to
    //paint their borders (and those of the internal ones) directly.
    //e.g. #33066#
    pLines->LockLines(sal_True);
    BorderLinesGuard blg; // this should not paint borders added from PaintBaBo

    SwRect aRect( rRect );
    aRect._Intersection( Frm() );

    OutputDevice* pOut = pGlobalShell->GetOut();
    pOut->Push( PUSH_CLIPREGION );
    pOut->SetClipRegion();
    const SwPageFrm* pPage = FindPageFrm();

    const SwNoTxtFrm *pNoTxt = Lower() && Lower()->IsNoTxtFrm()
                                                ? (SwNoTxtFrm*)Lower() : 0;

    bool bIsChart = false; //#i102950# don't paint additional borders for charts
    //check whether we have a chart
    if(pNoTxt)
    {
        const SwNoTxtNode* pNoTNd = dynamic_cast<const SwNoTxtNode*>(pNoTxt->GetNode());
        if( pNoTNd )
        {
            SwOLENode* pOLENd = const_cast<SwOLENode*>(pNoTNd->GetOLENode());
            if( pOLENd && ChartHelper::IsChart( pOLENd->GetOLEObj().GetObject() ) )
                bIsChart = true;
        }
    }

    {
        bool bContour = GetFmt()->GetSurround().IsContour();
        PolyPolygon aPoly;
        if ( bContour )
        {
            // OD 16.04.2003 #i13147# - add 2nd parameter with value <sal_True>
            // to indicate that method is called for paint in order to avoid
            // load of the intrinsic graphic.
            bContour = GetContour( aPoly, sal_True );
        }

        // #i47804# - distinguish complete background paint
        // and margin paint.
        // paint complete background for Writer text fly frames
        bool bPaintCompleteBack( !pNoTxt );
        // paint complete background for transparent graphic and contour,
        // if own background color exists.
        const bool bIsGraphicTransparent = pNoTxt ? pNoTxt->IsTransparent() : false;
        if ( !bPaintCompleteBack &&
             ( bIsGraphicTransparent|| bContour ) )
        {
            const SvxBrushItem &rBack = GetFmt()->GetBackground();
            // OD 07.08.2002 #99657# #GetTransChg#
            //     to determine, if background has to be painted, by checking, if
            //     background color is not COL_TRANSPARENT ("no fill"/"auto fill")
            //     or a background graphic exists.
            bPaintCompleteBack = !(rBack.GetColor() == COL_TRANSPARENT) ||
                                 rBack.GetGraphicPos() != GPOS_NONE;
        }
        // paint of margin needed.
        const bool bPaintMarginOnly( !bPaintCompleteBack &&
                                     Prt().SSize() != Frm().SSize() );

        // #i47804# - paint background of parent fly frame
        // for transparent graphics in layer Hell, if parent fly frame isn't
        // in layer Hell. It's only painted the intersection between the
        // parent fly frame area and the paint area <aRect>
        const IDocumentDrawModelAccess* pIDDMA = GetFmt()->getIDocumentDrawModelAccess();

        if ( bIsGraphicTransparent &&
            GetVirtDrawObj()->GetLayer() == pIDDMA->GetHellId() &&
            GetAnchorFrm()->FindFlyFrm() )
        {
            const SwFlyFrm* pParentFlyFrm = GetAnchorFrm()->FindFlyFrm();
            if ( pParentFlyFrm->GetDrawObj()->GetLayer() !=
                                            pIDDMA->GetHellId() )
            {
                SwFlyFrm* pOldRet = pRetoucheFly2;
                pRetoucheFly2 = const_cast<SwFlyFrm*>(this);

                SwBorderAttrAccess aAccess( SwFrm::GetCache(), pParentFlyFrm );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                SwRect aPaintRect( aRect );
                aPaintRect._Intersection( pParentFlyFrm->Frm() );
                pParentFlyFrm->PaintBackground( aPaintRect, pPage, rAttrs, sal_False, sal_False );

                pRetoucheFly2 = pOldRet;
            }
        }

        if ( bPaintCompleteBack || bPaintMarginOnly )
        {
            //#24926# JP 01.02.96, PaintBaBo is here partially so PaintBorder
            //receives the original Rect but PaintBackground only the limited
            //one.

            // OD 2004-04-23 #116347#
            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetLineColor();

            pPage = FindPageFrm();

            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // paint background
            {
                SwRegionRects aRegion( aRect );
                // #i80822#
                // suppress painting of background in printing area for
                // non-transparent graphics.
                if ( bPaintMarginOnly ||
                     ( pNoTxt && !bIsGraphicTransparent ) )
                {
                    //What we actually want to paint is the small stripe between
                    //PrtArea and outer border.
                    SwRect aTmp( Prt() ); aTmp += Frm().Pos();
                    aRegion -= aTmp;
                }
                if ( bContour )
                {
                    pOut->Push();
                    // #i80822#
                    // apply clip region under the same conditions, which are
                    // used in <SwNoTxtFrm::Paint(..)> to set the clip region
                    // for painting the graphic/OLE. Thus, the clip region is
                    // also applied for the PDF export.
                    ViewShell *pSh = getRootFrm()->GetCurrShell();
                    if ( !pOut->GetConnectMetaFile() || !pSh || !pSh->GetWin() )
                    {
                        pOut->SetClipRegion(Region(aPoly));
                    }
                    for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                        PaintBackground( aRegion[i], pPage, rAttrs, sal_False, sal_True );
                    pOut->Pop();
                }
                else
                    for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                        PaintBackground( aRegion[i], pPage, rAttrs, sal_False, sal_True );
            }

            // OD 06.08.2002 #99657# - paint border before painting background
            // paint border
            {
                SwRect aTmp( rRect );
                PaintBorder( aTmp, pPage, rAttrs );
            }

            pOut->Pop();
        }
    }

    // OD 19.12.2002 #106318# - fly frame will paint it's subsidiary lines and
    // the subsidiary lines of its lowers on its own, due to overlapping with
    // other fly frames or other objects.
    if( pGlobalShell->GetWin()
        && !bIsChart ) //#i102950# don't paint additional borders for charts
    {
        bool bSubsLineRectsCreated;
        if ( pSubsLines )
        {
            // Lock already existing subsidiary lines
            pSubsLines->LockLines( sal_True );
            bSubsLineRectsCreated = false;
        }
        else
        {
            // create new subsidiardy lines
            pSubsLines = new SwSubsRects;
            bSubsLineRectsCreated = true;
        }

        bool bSpecSubsLineRectsCreated;
        if ( pSpecSubsLines )
        {
            // Lock already existing special subsidiary lines
            pSpecSubsLines->LockLines( sal_True );
            bSpecSubsLineRectsCreated = false;
        }
        else
        {
            // create new special subsidiardy lines
            pSpecSubsLines = new SwSubsRects;
            bSpecSubsLineRectsCreated = true;
        }
        // Add subsidiary lines of fly frame and its lowers
        RefreshLaySubsidiary( pPage, aRect );
        // paint subsidiary lines of fly frame and its lowers
        pSpecSubsLines->PaintSubsidiary( pOut, NULL );
        pSubsLines->PaintSubsidiary( pOut, pLines );
        if ( !bSubsLineRectsCreated )
            // unlock subsidiary lines
            pSubsLines->LockLines( sal_False );
        else
            // delete created subsidiary lines container
            DELETEZ( pSubsLines );

        if ( !bSpecSubsLineRectsCreated )
            // unlock special subsidiary lines
            pSpecSubsLines->LockLines( sal_False );
        else
        {
            // delete created special subsidiary lines container
            DELETEZ( pSpecSubsLines );
        }
    }

    SwLayoutFrm::Paint( aRect );

    Validate();

    // OD 19.12.2002 #106318# - first paint lines added by fly frame paint
    // and then unlock other lines.
    pLines->PaintLines( pOut );
    pLines->LockLines( sal_False );
    // have to paint frame borders added in heaven layer here...
    ProcessPrimitives(g_pBorderLines->GetBorderLines_Clear());

    pOut->Pop();

    if ( pProgress && pNoTxt )
        pProgress->Reschedule();
}
/*************************************************************************
|*
|*    SwTabFrm::Paint()
|*
|*************************************************************************/

void SwTabFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
    if ( pGlobalShell->GetViewOptions()->IsTable() )
    {
        // #i29550#
        if ( IsCollapsingBorders() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // paint shadow
            if ( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                SwRect aRect;
                ::lcl_CalcBorderRect( aRect, this, rAttrs, true );
                PaintShadow( rRect, aRect, rAttrs );
            }

            // paint lines
            SwTabFrmPainter aHelper( *this );
            aHelper.PaintLines( *pGlobalShell->GetOut(), rRect );
        }
        // <-- collapsing

        SwLayoutFrm::Paint( rRect );
    }
    // OD 10.01.2003 #i6467# - no light grey rectangle for page preview
    else if ( pGlobalShell->GetWin() && !pGlobalShell->IsPreView() )
    {
        // OD 10.01.2003 #i6467# - intersect output rectangle with table frame
        SwRect aTabRect( Prt() );
        aTabRect.Pos() += Frm().Pos();
        SwRect aTabOutRect( rRect );
        aTabOutRect.Intersection( aTabRect );
        pGlobalShell->GetViewOptions()->
                DrawRect( pGlobalShell->GetOut(), aTabOutRect, COL_LIGHTGRAY );
    }
    ((SwTabFrm*)this)->ResetComplete();
}

/**
 * Paint border shadow.
 *
 * @param[in]       rRect       aligned rect to clip the result
 * @param[in|out]   rOutRect    full painting area as input
 *                              painting area reduced by shadow space for border and background as output
 * @param[in]       rShadow     includes shadow attributes
 * @param[in]       bDrawFullShadowRectangle    paint full rect of shadow
 * @param[in]       bTop        paint top part of the shadow
 * @param[in]       bBottom     paint bottom part of the shadow
 * @param[in]       bLeft       paint left part of the shadow
 * @param[in]       bRight      paint right part of the shadow
**/
static void lcl_PaintShadow( const SwRect& rRect, SwRect& rOutRect,
    const SvxShadowItem& rShadow, const bool bDrawFullShadowRectangle,
    const bool bTop, const bool bBottom,
    const bool bLeft, const bool bRight )
{
    const long nWidth  = ::lcl_AlignWidth ( rShadow.GetWidth() );
    const long nHeight = ::lcl_AlignHeight( rShadow.GetWidth() );

    SwRects aRegion( 2 );
    SwRect aOut( rOutRect );

    switch ( rShadow.GetLocation() )
    {
        case SVX_SHADOW_BOTTOMRIGHT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Top( rOutRect.Top() + nHeight );
                    aOut.Left( rOutRect.Left() + nWidth );
                    aRegion.push_back( aOut );
                }
                else
                {
                    if( bBottom )
                    {
                        aOut.Top( rOutRect.Bottom() - nHeight );
                        if( bLeft )
                            aOut.Left( rOutRect.Left() + nWidth );
                        aRegion.push_back( aOut );
                    }
                    if( bRight )
                    {
                        aOut.Left( rOutRect.Right() - nWidth );
                        if( bTop )
                            aOut.Top( rOutRect.Top() + nHeight );
                        else
                            aOut.Top( rOutRect.Top() );
                        if( bBottom )
                            aOut.Bottom( rOutRect.Bottom() - nHeight );
                        aRegion.push_back( aOut );
                    }
                }

                if( bRight )
                    rOutRect.Right( rOutRect.Right() - nWidth );
                if( bBottom )
                    rOutRect.Bottom( rOutRect.Bottom()- nHeight );
            }
            break;
        case SVX_SHADOW_TOPLEFT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Bottom( rOutRect.Bottom() - nHeight );
                    aOut.Right( rOutRect.Right() - nWidth );
                    aRegion.push_back( aOut );
                }
                else
                {
                    if( bTop )
                    {
                        aOut.Bottom( rOutRect.Top() + nHeight );
                        if( bRight )
                            aOut.Right( rOutRect.Right() - nWidth );
                        aRegion.push_back( aOut );
                    }
                    if( bLeft )
                    {
                        aOut.Right( rOutRect.Left() + nWidth );
                        if( bBottom )
                            aOut.Bottom( rOutRect.Bottom() - nHeight );
                        else
                            aOut.Bottom( rOutRect.Bottom() );
                        if( bTop )
                            aOut.Top( rOutRect.Top() + nHeight );
                        aRegion.push_back( aOut );
                    }
                }

                if( bLeft )
                    rOutRect.Left( rOutRect.Left() + nWidth );
                if( bTop )
                    rOutRect.Top( rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_TOPRIGHT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Bottom( rOutRect.Bottom() - nHeight);
                    aOut.Left( rOutRect.Left() + nWidth );
                    aRegion.push_back( aOut );
                }
                else
                {
                    if( bTop )
                    {
                        aOut.Bottom( rOutRect.Top() + nHeight );
                        if( bLeft )
                            aOut.Left( rOutRect.Left() + nWidth );
                        aRegion.push_back( aOut );
                    }
                    if( bRight )
                    {
                        aOut.Left( rOutRect.Right() - nWidth );
                        if( bBottom )
                            aOut.Bottom( rOutRect.Bottom() - nHeight );
                        else
                            aOut.Bottom( rOutRect.Bottom() );
                        if( bTop )
                            aOut.Top( rOutRect.Top() + nHeight );
                        aRegion.push_back( aOut );
                    }
                }

                if( bRight )
                    rOutRect.Right( rOutRect.Right() - nWidth );
                if( bTop )
                    rOutRect.Top( rOutRect.Top() + nHeight );
            }
            break;
        case SVX_SHADOW_BOTTOMLEFT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    /// OD 06.08.2002 #99657# - draw full shadow rectangle
                    aOut.Top( rOutRect.Top() + nHeight );
                    aOut.Right( rOutRect.Right() - nWidth );
                    aRegion.push_back( aOut );
                }
                else
                {
                    if( bBottom )
                    {
                        aOut.Top( rOutRect.Bottom()- nHeight );
                        if( bRight )
                            aOut.Right( rOutRect.Right() - nWidth );
                        aRegion.push_back( aOut );
                    }
                    if( bLeft )
                    {
                        aOut.Right( rOutRect.Left() + nWidth );
                        if( bTop )
                            aOut.Top( rOutRect.Top() + nHeight );
                        else
                            aOut.Top( rOutRect.Top() );
                        if( bBottom )
                            aOut.Bottom( rOutRect.Bottom() - nHeight );
                        aRegion.push_back( aOut );
                    }
                }

                if( bLeft )
                    rOutRect.Left( rOutRect.Left() + nWidth );
                if( bBottom )
                    rOutRect.Bottom( rOutRect.Bottom() - nHeight );
            }
            break;
        default:
            assert(false);
            break;
    }

    OutputDevice *pOut = pGlobalShell->GetOut();

    sal_uLong nOldDrawMode = pOut->GetDrawMode();
    Color aShadowColor( rShadow.GetColor().GetRGBColor() );
    if( !aRegion.empty() && pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        // Is heigh contrast mode, the output device has already set the
        // DRAWMODE_SETTINGSFILL flag. This causes the SetFillColor function
        // to ignore the setting of a new color. Therefore we have to reset
        // the drawing mode
        pOut->SetDrawMode( 0 );
        aShadowColor = SwViewOption::GetFontColor();
    }

    if ( pOut->GetFillColor() != aShadowColor )
        pOut->SetFillColor( aShadowColor );

    pOut->SetLineColor();

    pOut->SetDrawMode( nOldDrawMode );

    for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
    {
        SwRect &rOut = aRegion[i];
        aOut = rOut;
        // OD 30.09.2002 #103636# - no SwAlign of shadow rectangle
        // no alignment necessary, because (1) <rRect> is already aligned
        // and because (2) paint of border and background will occur later.
        // Thus, (1) assures that no conflicts with neighbour object will occure
        // and (2) assures that border and background is not affected by the
        // shadow paint.
        /*
        ::SwAlignRect( aOut, pGlobalShell );
        */
        if ( rRect.IsOver( aOut ) && aOut.Height() > 0 && aOut.Width() > 0 )
        {
            aOut._Intersection( rRect );
            pOut->DrawRect( aOut.SVRect() );
        }
    }
}

/*************************************************************************
|*
|*  SwFrm::PaintShadow()
|*
|*  Description         Paints a shadow if the format requests so.
|*      The shadow is always painted on the outer edge of the OutRect.
|*      If needed, the OutRect is shrunk so the painting of the border can be
|*      done on it.
|*
|*************************************************************************/
/// OD 23.08.2002 #99657#
///     draw full shadow rectangle for frames with transparent drawn backgrounds.
void SwFrm::PaintShadow( const SwRect& rRect, SwRect& rOutRect,
                         const SwBorderAttrs &rAttrs ) const
{
    SvxShadowItem rShadow = rAttrs.GetShadow();

    const sal_Bool bCnt    = IsCntntFrm();
    const bool bTop    = !bCnt || rAttrs.GetTopLine  ( *(this) );
    const bool bBottom = !bCnt || rAttrs.GetBottomLine( *(this) );

    if( IsVertical() )
    {
        switch( rShadow.GetLocation() )
        {
            case SVX_SHADOW_BOTTOMRIGHT: rShadow.SetLocation(SVX_SHADOW_BOTTOMLEFT);  break;
            case SVX_SHADOW_TOPLEFT:     rShadow.SetLocation(SVX_SHADOW_TOPRIGHT);    break;
            case SVX_SHADOW_TOPRIGHT:    rShadow.SetLocation(SVX_SHADOW_BOTTOMRIGHT); break;
            case SVX_SHADOW_BOTTOMLEFT:  rShadow.SetLocation(SVX_SHADOW_TOPLEFT);     break;
            default: break;
        }
    }

    /// OD 23.08.2002 #99657# - determine, if full shadow rectangle have to
    ///     be drawn or only two shadow rectangles beside the frame.
    ///     draw full shadow rectangle, if frame background is drawn transparent.
    ///     Status Quo:
    ///         SwLayoutFrm can have transparent drawn backgrounds. Thus,
    ///         "asked" their frame format.
    const bool bDrawFullShadowRectangle =
            ( IsLayoutFrm() &&
              (static_cast<const SwLayoutFrm*>(this))->GetFmt()->IsBackgroundTransparent()
            );

   lcl_PaintShadow(rRect, rOutRect, rShadow, bDrawFullShadowRectangle, bTop, bBottom, true, true);
}

/*************************************************************************
|*
|*  SwFrm::PaintBorderLine()
|*
|*************************************************************************/

void SwFrm::PaintBorderLine( const SwRect& rRect,
                             const SwRect& rOutRect,
                             const SwPageFrm *pPage,
                             const Color *pColor,
                             const SvxBorderStyle nStyle ) const
{
    if ( !rOutRect.IsOver( rRect ) )
        return;

    SwRect aOut( rOutRect );
    aOut._Intersection( rRect );

    const SwTabFrm *pTab = IsCellFrm() ? FindTabFrm() : 0;
    sal_uInt8 nSubCol = ( IsCellFrm() || IsRowFrm() ) ? SUBCOL_TAB :
                   ( IsInSct() ? SUBCOL_SECT :
                   ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );
    if( pColor && pGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pColor = &SwViewOption::GetFontColor();
    }

    if ( pPage->GetSortedObjs() )
    {
        SwRegionRects aRegion( aOut, 4 );
        ::lcl_SubtractFlys( this, pPage, aOut, aRegion );
        for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
            pLines->AddLineRect( aRegion[i], pColor, nStyle, pTab, nSubCol );
    }
    else
        pLines->AddLineRect( aOut, pColor, nStyle, pTab, nSubCol );
}

/*************************************************************************
|*
|*  SwFrm::PaintBorderLines()
|*
|*  Descrtiption        Only all lines once or all lines twice!
|*
|*************************************************************************/

// OD 29.04.2003 #107169# - method called for left and right border rectangles.
// For a printer output device perform adjustment for non-overlapping top and
// bottom border rectangles. Thus, add parameter <_bPrtOutputDev> to indicate
// printer output device.
// NOTE: For printer output device left/right border rectangle <_iorRect>
//       has to be already non-overlapping the outer top/bottom border rectangle.
static void lcl_SubTopBottom( SwRect&              _iorRect,
                                   const SvxBoxItem&    _rBox,
                                   const SwBorderAttrs& _rAttrs,
                                   const SwFrm&         _rFrm,
                                   const SwRectFn&      _rRectFn,
                                   const bool       _bPrtOutputDev )
{
    const sal_Bool bCnt = _rFrm.IsCntntFrm();
    if ( _rBox.GetTop() && _rBox.GetTop()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetTopLine( _rFrm ) )
       )
    {
        // substract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetTop()->GetDistance() );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner top line is a hair line (width = 1)
        bool bIsInnerTopLineHairline = false;
        if ( !_bPrtOutputDev )
        {
            // additionally substract width of top outer line
            // --> left/right inner/outer line doesn't overlap top outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetOutWidth() );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally substract width of top inner line
            // --> left/right inner/outer line doesn't overlap top inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetInWidth() );
            bIsInnerTopLineHairline = _rBox.GetTop()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnSubTop)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border top, if inner top line
        // is a hair line
        if ( bIsInnerTopLineHairline )
        {
            if ( _rFrm.IsVertical() )
            {
                // right of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Right(), 0 );
                Point aRefPt( aCompPt.X() + 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          true, -1 );
                _iorRect.Right( aCompPt.X() );
            }
            else
            {
                // top of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Top() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() - 1 );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          false, +1 );
                _iorRect.Top( aCompPt.Y() );
            }
        }
    }

    if ( _rBox.GetBottom() && _rBox.GetBottom()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetBottomLine( _rFrm ) )
       )
    {
        // substract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetBottom()->GetDistance() );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner bottom line is a hair line (width = 1)
        bool bIsInnerBottomLineHairline = false;
        if ( !_bPrtOutputDev )
        {
            // additionally substract width of bottom outer line
            // --> left/right inner/outer line doesn't overlap bottom outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetOutWidth() );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally substract width of bottom inner line
            // --> left/right inner/outer line doesn't overlap bottom inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetInWidth() );
            bIsInnerBottomLineHairline = _rBox.GetBottom()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnAddBottom)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border bottom, if inner
        // bottom line is a hair line.
        if ( bIsInnerBottomLineHairline )
        {
            if ( _rFrm.IsVertical() )
            {
                // left of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Left(), 0 );
                Point aRefPt( aCompPt.X() - 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          true, +1 );
                _iorRect.Left( aCompPt.X() );
            }
            else
            {
                // bottom of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Bottom() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() + 1 );
                lcl_CompPxPosAndAdjustPos( *(pGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          false, -1 );
                _iorRect.Bottom( aCompPt.Y() );
            }
        }
    }
}

static sal_uInt16 lcl_GetLineWidth( const SvxBorderLine* pLine )
{
    sal_uInt16 result = 0;

    if ( pLine != NULL )
        result = pLine->GetScaledWidth();

    return result;
}

static double lcl_GetExtent( const SvxBorderLine* pSideLine, const SvxBorderLine* pOppositeLine )
{
    double nExtent = 0.0;

    if ( pSideLine && !pSideLine->isEmpty() )
        nExtent = -lcl_GetLineWidth( pSideLine ) / 2.0;
    else if ( pOppositeLine )
        nExtent = lcl_GetLineWidth( pOppositeLine ) / 2.0;

    return nExtent;
}

static void
lcl_MakeBorderLine(SwRect const& rRect,
        bool const isVerticalInModel,
        bool const isLeftOrTopBorderInModel,
        bool const isVertical,
        SvxBorderLine const& rBorder,
        SvxBorderLine const*const pLeftOrTopNeighbour,
        SvxBorderLine const*const pRightOrBottomNeighbour)
{
    bool const isLeftOrTopBorder((isVerticalInModel == isVertical)
            ? isLeftOrTopBorderInModel
            : (isLeftOrTopBorderInModel != isVertical));
    SvxBorderLine const*const pStartNeighbour(
            (!isVertical && isVerticalInModel)
            ? pRightOrBottomNeighbour : pLeftOrTopNeighbour);
    SvxBorderLine const*const pEndNeighbour(
            (pStartNeighbour == pLeftOrTopNeighbour)
            ? pRightOrBottomNeighbour : pLeftOrTopNeighbour);

    basegfx::B2DPoint aStart;
    basegfx::B2DPoint aEnd;
    if (isVertical)
    {   // fdo#38635: always from outer edge
        double const fStartX( (isLeftOrTopBorder)
                ? rRect.Left()  + (rRect.Width() / 2.0)
                : rRect.Right() - (rRect.Width() / 2.0));
        aStart.setX(fStartX);
        aStart.setY(rRect.Top() +
                lcl_AlignHeight(lcl_GetLineWidth(pStartNeighbour))/2.0);
        aEnd.setX(fStartX);
        aEnd.setY(rRect.Bottom() -
                lcl_AlignHeight(lcl_GetLineWidth(pEndNeighbour))/2.0);
    }
    else
    {   // fdo#38635: always from outer edge
        double const fStartY( (isLeftOrTopBorder)
                ? rRect.Top()    + (rRect.Height() / 2.0)
                : rRect.Bottom() - (rRect.Height() / 2.0));
        aStart.setX(rRect.Left() +
                lcl_AlignWidth(lcl_GetLineWidth(pStartNeighbour))/2.0);
        aStart.setY(fStartY);
        aEnd.setX(rRect.Right() -
                lcl_AlignWidth(lcl_GetLineWidth(pEndNeighbour))/2.0);
        aEnd.setY(fStartY);
    }

    // WHen rendering to very small (virtual) devices, like when producing
    // page thumbnails in a mobile device app, the line geometry can end up
    // bogus (negative width or height), so just ignore such border lines.
    // Otherwise we will run into assertions later in lcl_TryMergeBorderLine()
    // at least.
    if (aEnd.getX() < aStart.getX() ||
        aEnd.getY() < aStart.getY())
        return;

    double const nExtentLeftStart = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(pStartNeighbour, 0)
        :   lcl_GetExtent(0, pStartNeighbour);
    double const nExtentLeftEnd = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(pEndNeighbour, 0)
        :   lcl_GetExtent(0, pEndNeighbour);
    double const nExtentRightStart = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(0, pStartNeighbour)
        :   lcl_GetExtent(pStartNeighbour, 0);
    double const nExtentRightEnd = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(0, pEndNeighbour)
        :   lcl_GetExtent(pEndNeighbour, 0);

    double const nLeftWidth = (isLeftOrTopBorder == isVertical)
         ? rBorder.GetInWidth() : rBorder.GetOutWidth();
    double const nRightWidth = (isLeftOrTopBorder == isVertical)
         ? rBorder.GetOutWidth() : rBorder.GetInWidth();
    Color const aLeftColor = (isLeftOrTopBorder == isVertical)
        ? rBorder.GetColorIn(isLeftOrTopBorder)
        : rBorder.GetColorOut(isLeftOrTopBorder);
    Color const aRightColor = (isLeftOrTopBorder == isVertical)
        ? rBorder.GetColorOut(isLeftOrTopBorder)
        : rBorder.GetColorIn(isLeftOrTopBorder);

    ::rtl::Reference<BorderLinePrimitive2D> const xLine =
        new BorderLinePrimitive2D(
            aStart, aEnd, nLeftWidth, rBorder.GetDistance(), nRightWidth,
            nExtentLeftStart, nExtentLeftEnd,
            nExtentRightStart, nExtentRightEnd,
            aLeftColor.getBColor(), aRightColor.getBColor(),
            rBorder.GetColorGap().getBColor(), rBorder.HasGapColor(),
            rBorder.GetBorderLineStyle() );
    g_pBorderLines->AddBorderLine(xLine);
}

// OD 19.05.2003 #109667# - merge <lcl_PaintLeftLine> and <lcl_PaintRightLine>
// into new method <lcl_PaintLeftRightLine(..)>
static void lcl_PaintLeftRightLine( const bool         _bLeft,
                             const SwFrm&           _rFrm,
                             const SwPageFrm&       /*_rPage*/,
                             const SwRect&          _rOutRect,
                             const SwRect&          /*_rRect*/,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn )
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const bool bR2L = _rFrm.IsCellFrm() && _rFrm.IsRightToLeft();
    const SvxBorderLine* pLeftRightBorder = 0;
    const SvxBorderLine* pTopBorder = rBox.GetTop();
    const SvxBorderLine* pBottomBorder = rBox.GetBottom();

    if ( _bLeft )
    {
        pLeftRightBorder = bR2L ? rBox.GetRight() : rBox.GetLeft();
    }
    else
    {
        pLeftRightBorder = bR2L ? rBox.GetLeft() : rBox.GetRight();
    }

    if ( !pLeftRightBorder )
    {
        return;
    }

    SwRect aRect( _rOutRect );
    if ( _bLeft )
    {
        (aRect.*_rRectFn->fnAddRight)( ::lcl_AlignWidth( lcl_GetLineWidth( pLeftRightBorder ) ) -
                                       (aRect.*_rRectFn->fnGetWidth)() );
    }
    else
    {
        (aRect.*_rRectFn->fnSubLeft)( ::lcl_AlignWidth( lcl_GetLineWidth( pLeftRightBorder ) ) -
                                      (aRect.*_rRectFn->fnGetWidth)() );
    }

    if ( _rFrm.IsCntntFrm() )
    {
        // No Top / bottom borders for joint borders
        if ( _rAttrs.JoinedWithPrev( _rFrm ) ) pTopBorder = NULL;
        if ( _rAttrs.JoinedWithNext( _rFrm ) ) pBottomBorder = NULL;
    }

    if ( !pLeftRightBorder->GetInWidth() )
    {
        // OD 06.05.2003 #107169# - init boolean indicating printer output device.
        const bool bPrtOutputDev =
                ( OUTDEV_PRINTER == pGlobalShell->GetOut()->GetOutDevType() );

        // OD 06.05.2003 #107169# - add 6th parameter
        ::lcl_SubTopBottom( aRect, rBox, _rAttrs, _rFrm, _rRectFn, bPrtOutputDev );
    }

    if ( lcl_GetLineWidth( pLeftRightBorder ) > 0 )
    {
        lcl_MakeBorderLine(
            aRect, true, _bLeft, aRect.Height() > aRect.Width(),
            *pLeftRightBorder, pTopBorder, pBottomBorder);
    }
}

// OD 19.05.2003 #109667# - merge <lcl_PaintTopLine> and <lcl_PaintBottomLine>
// into <lcl_PaintTopLine>
static void lcl_PaintTopBottomLine( const bool         _bTop,
                             const SwFrm&           ,
                             const SwPageFrm&       /*_rPage*/,
                             const SwRect&          _rOutRect,
                             const SwRect&          /*_rRect*/,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn )
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const SvxBorderLine* pTopBottomBorder = 0;

    if ( _bTop )
    {
        pTopBottomBorder = rBox.GetTop();
    }
    else
    {
        pTopBottomBorder = rBox.GetBottom();
    }

    if ( !pTopBottomBorder )
    {
        return;
    }

    SwRect aRect( _rOutRect );
    if ( _bTop )
    {
        (aRect.*_rRectFn->fnAddBottom)( ::lcl_AlignHeight( lcl_GetLineWidth( pTopBottomBorder ) ) -
                                        (aRect.*_rRectFn->fnGetHeight)() );
    }
    else
    {
        (aRect.*_rRectFn->fnSubTop)( ::lcl_AlignHeight( lcl_GetLineWidth( pTopBottomBorder ) ) -
                                     (aRect.*_rRectFn->fnGetHeight)() );
    }

    if ( lcl_GetLineWidth( pTopBottomBorder ) > 0 )
    {
        lcl_MakeBorderLine(
            aRect, false, _bTop, aRect.Height() > aRect.Width(),
            *pTopBottomBorder, rBox.GetLeft(), rBox.GetRight());
    }
}

void PaintCharacterBorder(
    const SwFont& rFont,
    const SwRect& rPaintArea,
    const bool bVerticalLayout,
    const bool bJoinWithPrev,
    const bool bJoinWithNext )
{
    SwRect aAlignedRect(rPaintArea);
    SwAlignRect(aAlignedRect, pGlobalShell);

    bool bTop = true;
    bool bBottom = true;
    bool bLeft = true;
    bool bRight = true;

    switch( rFont.GetOrientation(bVerticalLayout) )
    {
        case 0 :
            bLeft = !bJoinWithPrev;
            bRight = !bJoinWithNext;
            break;
        case 900 :
            bBottom = !bJoinWithPrev;
            bTop = !bJoinWithNext;
            break;
        case 1800 :
            bRight = !bJoinWithPrev;
            bLeft = !bJoinWithNext;
            break;
        case 2700 :
            bTop = !bJoinWithPrev;
            bBottom = !bJoinWithNext;
            break;
    }

    // Paint shadow (reduce painting rect)
    {
        const SvxShadowItem aShadow(
            0, &rFont.GetShadowColor(), rFont.GetShadowWidth(),
            rFont.GetAbsShadowLocation(bVerticalLayout));

        if( aShadow.GetLocation() != SVX_SHADOW_NONE )
        {
            lcl_PaintShadow( SwRect(aAlignedRect), aAlignedRect, aShadow,
                             false, bTop, bBottom, bLeft, bRight);
        }
    }

    // Init borders, after this initialization top, bottom, right and left means the
    // absolute position
    const boost::optional<editeng::SvxBorderLine> aTopBorder =
        (bTop ? rFont.GetAbsTopBorder(bVerticalLayout) : boost::none);
    const boost::optional<editeng::SvxBorderLine> aBottomBorder =
        (bBottom ? rFont.GetAbsBottomBorder(bVerticalLayout) : boost::none);
    const boost::optional<editeng::SvxBorderLine> aLeftBorder =
        (bLeft ? rFont.GetAbsLeftBorder(bVerticalLayout) : boost::none);
    const boost::optional<editeng::SvxBorderLine> aRightBorder =
        (bRight ? rFont.GetAbsRightBorder(bVerticalLayout) : boost::none);

    if( aTopBorder )
    {
        Point aLeftTop(
            aAlignedRect.Left(),
            aAlignedRect.Top());
        Point aRightBottom(
            aAlignedRect.Right(),
            aAlignedRect.Top() + aTopBorder.get().GetScaledWidth());

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            false, true, false,
            aTopBorder.get(),
            aLeftBorder.get_ptr(),
            aRightBorder.get_ptr());
    }

    if( aBottomBorder )
    {
        Point aLeftTop(
            aAlignedRect.Left(),
            aAlignedRect.Bottom() - aBottomBorder.get().GetScaledWidth());
        Point aRightBottom(
            aAlignedRect.Right(),
            aAlignedRect.Bottom());

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            false, false, false,
            aBottomBorder.get(),
            aLeftBorder.get_ptr(),
            aRightBorder.get_ptr());
    }

    if( aLeftBorder )
    {
        Point aLeftTop(
            aAlignedRect.Left(),
            aAlignedRect.Top());
        Point aRightBottom(
            aAlignedRect.Left() + aLeftBorder.get().GetScaledWidth(),
            aAlignedRect.Bottom());

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            true, true, true,
            aLeftBorder.get(),
            aTopBorder.get_ptr(),
            aBottomBorder.get_ptr());
    }

    if( aRightBorder )
    {
        Point aLeftTop(
            aAlignedRect.Right() - aRightBorder.get().GetScaledWidth(),
            aAlignedRect.Top());
        Point aRightBottom(
            aAlignedRect.Right(),
            aAlignedRect.Bottom());

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            true, false, true,
            aRightBorder.get(),
            aTopBorder.get_ptr(),
            aBottomBorder.get_ptr());
    }
}

/*************************************************************************
|*
|*  const SwFrm* lcl_HasNextCell( const SwFrm& rFrm )
|*
|* No comment. #i15844#
|*
|*************************************************************************/

static const SwFrm* lcl_HasNextCell( const SwFrm& rFrm )
{
    OSL_ENSURE( rFrm.IsCellFrm(),
            "lcl_HasNextCell( const SwFrm& rFrm ) should be called with SwCellFrm" );

    const SwFrm* pTmpFrm = &rFrm;
    do
    {
        if ( pTmpFrm->GetNext() )
            return pTmpFrm->GetNext();

        pTmpFrm = pTmpFrm->GetUpper()->GetUpper();
    }
    while ( pTmpFrm->IsCellFrm() );

    return 0;
}

/*************************************************************************
|*
|*  SwFrm::PaintBorder()
|*
|*  Description         Paints shadows and borders
|*
|*************************************************************************/

/** local method to determine cell frame, from which the border attributes
    for paint of top/bottom border has to be used.

    OD 21.02.2003 #b4779636#, #107692#

    @author OD

    @param _pCellFrm
    input parameter - constant pointer to cell frame for which the cell frame
    for the border attributes has to be determined.

    @param _rCellBorderAttrs
    input parameter - constant reference to the border attributes of cell frame
    <_pCellFrm>.

    @param _bTop
    input parameter - boolean, that controls, if cell frame for top border or
    for bottom border has to be determined.

    @return constant pointer to cell frame, for which the border attributes has
    to be used
*/
static const SwFrm* lcl_GetCellFrmForBorderAttrs( const SwFrm*         _pCellFrm,
                                           const SwBorderAttrs& _rCellBorderAttrs,
                                           const bool           _bTop )
{
    OSL_ENSURE( _pCellFrm, "No cell frame available, dying soon" );

    // determine, if cell frame is at bottom/top border of a table frame and
    // the table frame has/is a follow.
    const SwFrm* pTmpFrm = _pCellFrm;
    bool bCellAtBorder = true;
    bool bCellAtLeftBorder = !_pCellFrm->GetPrev();
    bool bCellAtRightBorder = !_pCellFrm->GetNext();
    while( !pTmpFrm->IsRowFrm() || !pTmpFrm->GetUpper()->IsTabFrm() )
    {
        pTmpFrm = pTmpFrm->GetUpper();
        if ( pTmpFrm->IsRowFrm() &&
             (_bTop ? pTmpFrm->GetPrev() : pTmpFrm->GetNext())
           )
        {
            bCellAtBorder = false;
        }
        if ( pTmpFrm->IsCellFrm() )
        {
            if ( pTmpFrm->GetPrev() )
            {
                bCellAtLeftBorder = false;
            }
            if ( pTmpFrm->GetNext() )
            {
                bCellAtRightBorder = false;
            }
        }
    }
    OSL_ENSURE( pTmpFrm && pTmpFrm->IsRowFrm(), "No RowFrm available" );

    const SwLayoutFrm* pParentRowFrm = static_cast<const SwLayoutFrm*>(pTmpFrm);
    const SwTabFrm* pParentTabFrm =
            static_cast<const SwTabFrm*>(pParentRowFrm->GetUpper());

    const bool bCellNeedsAttribute = bCellAtBorder &&
                                     ( _bTop ?
                                      // bCellInFirstRowWithMaster
                                       ( !pParentRowFrm->GetPrev() &&
                                         pParentTabFrm->IsFollow() &&
                                         0 == pParentTabFrm->GetTable()->GetRowsToRepeat() ) :
                                      // bCellInLastRowWithFollow
                                       ( !pParentRowFrm->GetNext() &&
                                         pParentTabFrm->GetFollow() )
                                     );

    const SwFrm* pRet = _pCellFrm;
    if ( bCellNeedsAttribute )
    {
        // determine, if cell frame has no borders inside the table.
        const SwFrm* pNextCell = 0;
        bool bNoBordersInside = false;

        if ( bCellAtLeftBorder && ( 0 != ( pNextCell = lcl_HasNextCell( *_pCellFrm ) ) ) )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pNextCell );
            const SwBorderAttrs &rBorderAttrs = *aAccess.Get();
            const SvxBoxItem& rBorderBox = rBorderAttrs.GetBox();
            bCellAtRightBorder = !lcl_HasNextCell( *pNextCell );
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrm->GetPrev() ) &&
                  !rBorderBox.GetLeft() &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrm->GetNext() );
        }
        else
        {
            const SvxBoxItem& rBorderBox = _rCellBorderAttrs.GetBox();
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrm->GetPrev() ) &&
                ( !rBorderBox.GetLeft()   || bCellAtLeftBorder ) &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrm->GetNext() );
        }

        if ( bNoBordersInside )
        {
            if ( _bTop && !_rCellBorderAttrs.GetBox().GetTop() )
            {
                //-hack
                // Cell frame has no top border and no border inside the table, but
                // it is at the top border of a table frame, which is a follow.
                // Thus, use border attributes of cell frame in first row of complete table.
                // First, determine first table frame of complete table.
                SwTabFrm* pMasterTabFrm = pParentTabFrm->FindMaster( true );
                // determine first row of complete table.
                const SwFrm* pFirstRow = pMasterTabFrm->GetLower();
                // return first cell in first row
                SwFrm* pLowerCell = const_cast<SwFrm*>(pFirstRow->GetLower());
                while ( !pLowerCell->IsCellFrm() ||
                        ( pLowerCell->GetLower() && pLowerCell->GetLower()->IsRowFrm() )
                      )
                {
                    pLowerCell = pLowerCell->GetLower();
                }
                OSL_ENSURE( pLowerCell && pLowerCell->IsCellFrm(), "No CellFrm available" );
                pRet = pLowerCell;
            }
            else if ( !_bTop && !_rCellBorderAttrs.GetBox().GetBottom() )
            {
                //-hack
                // Cell frame has no bottom border and no border inside the table,
                // but it is at the bottom border of a table frame, which has a follow.
                // Thus, use border attributes of cell frame in last row of complete table.
                // First, determine last table frame of complete table.
                SwTabFrm* pLastTabFrm = const_cast<SwTabFrm*>(pParentTabFrm->GetFollow());
                while ( pLastTabFrm->GetFollow() )
                {
                    pLastTabFrm = pLastTabFrm->GetFollow();
                }
                // determine last row of complete table.
                SwFrm* pLastRow = pLastTabFrm->GetLastLower();
                // return first bottom border cell in last row
                SwFrm* pLowerCell = const_cast<SwFrm*>(pLastRow->GetLower());
                while ( !pLowerCell->IsCellFrm() ||
                        ( pLowerCell->GetLower() && pLowerCell->GetLower()->IsRowFrm() )
                      )
                {
                    if ( pLowerCell->IsRowFrm() )
                    {
                        while ( pLowerCell->GetNext() )
                        {
                            pLowerCell = pLowerCell->GetNext();
                        }
                    }
                    pLowerCell = pLowerCell->GetLower();
                }
                OSL_ENSURE( pLowerCell && pLowerCell->IsCellFrm(), "No CellFrm available" );
                pRet = pLowerCell;
            }
        }
    }

    return pRet;
}

drawinglayer::processor2d::BaseProcessor2D * SwFrm::CreateProcessor2D( ) const
{
    basegfx::B2DRange aViewRange;

    SdrPage *pDrawPage = getRootFrm()->GetCurrShell()->Imp()->GetPageView()->GetPage();
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos(
            basegfx::B2DHomMatrix(  ),
            getRootFrm()->GetCurrShell()->GetOut()->GetViewTransformation(),
            aViewRange,
            GetXDrawPageForSdrPage( pDrawPage ),
            0.0,
            uno::Sequence< beans::PropertyValue >() );

    return  drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *getRootFrm()->GetCurrShell()->GetOut(),
                    aNewViewInfos );
}

void SwFrm::ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DSequence& rSequence ) const
{
    drawinglayer::processor2d::BaseProcessor2D * pProcessor2D = CreateProcessor2D();

    if ( pProcessor2D )
    {
        pProcessor2D->process( rSequence );
        delete pProcessor2D;
    }
}

void SwFrm::PaintBorder( const SwRect& rRect, const SwPageFrm *pPage,
                         const SwBorderAttrs &rAttrs ) const
{
    //fuer (Row,Body,Ftn,Root,Column,NoTxt) gibt's hier nix zu tun
    if ( (GetType() & 0x90C5) )
        return;

    if ( (GetType() & 0x2000) &&    //Cell
         !pGlobalShell->GetViewOptions()->IsTable() )
        return;

    // #i29550#
    if ( IsTabFrm() || IsCellFrm() || IsRowFrm() )
    {
        const SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
            return;

        if ( pTabFrm->GetTable()->IsNewModel() && ( !IsCellFrm() || IsCoveredCell() ) )
            return;
    }

    const bool bLine = rAttrs.IsLine() ? true : false;
    const bool bShadow = rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE;

    // - flag to control,
    //-hack has to be used.
    const bool bb4779636HackActive = true;
    //
    const SwFrm* pCellFrmForBottomBorderAttrs = 0;
    const SwFrm* pCellFrmForTopBorderAttrs = 0;
    bool         bFoundCellForTopOrBorderAttrs = false;
    if ( bb4779636HackActive && IsCellFrm() )
    {
        pCellFrmForBottomBorderAttrs = lcl_GetCellFrmForBorderAttrs( this, rAttrs, false );
        if ( pCellFrmForBottomBorderAttrs != this )
            bFoundCellForTopOrBorderAttrs = true;
        pCellFrmForTopBorderAttrs = lcl_GetCellFrmForBorderAttrs( this, rAttrs, true );
        if ( pCellFrmForTopBorderAttrs != this )
            bFoundCellForTopOrBorderAttrs = true;
    }

    // - add condition <bFoundCellForTopOrBorderAttrs>
    //-hack
    if ( bLine || bShadow || bFoundCellForTopOrBorderAttrs )
    {
        //If the rectangle is completely inside the PrtArea, no border needs to
        //be painted.
        //For the PrtArea the aligned value needs to be used, otherwise it could
        //happen, that some parts won't be processed.
        SwRect aRect( Prt() );
        aRect += Frm().Pos();
        ::SwAlignRect( aRect, pGlobalShell );
        // OD 27.09.2002 #103636# - new local boolean variable in order to
        // suspend border paint under special cases - see below.
        // NOTE: This is a fix for the implementation of feature #99657#.
        bool bDrawOnlyShadowForTransparentFrame = false;
        if ( aRect.IsInside( rRect ) )
        {
            // OD 27.09.2002 #103636# - paint shadow, if background is transparent.
            // Because of introduced transparent background for fly frame #99657#,
            // the shadow have to be drawn if the background is transparent,
            // in spite the fact that the paint rectangle <rRect> lies fully
            // in the printing area.
            // NOTE to chosen solution:
            //     On transparent background, continue processing, but suspend
            //     drawing of border by setting <bDrawOnlyShadowForTransparentFrame>
            //     to true.
            if ( IsLayoutFrm() &&
                 static_cast<const SwLayoutFrm*>(this)->GetFmt()->IsBackgroundTransparent() )
            {
                 bDrawOnlyShadowForTransparentFrame = true;
            }
            else
            {
                return;
            }
        }

        if ( !pPage )
            pPage = FindPageFrm();

        ::lcl_CalcBorderRect( aRect, this, rAttrs, true );
        rAttrs.SetGetCacheLine( sal_True );
        if ( bShadow )
            PaintShadow( rRect, aRect, rAttrs );
        // OD 27.09.2002 #103636# - suspend drawing of border
        // add condition < NOT bDrawOnlyShadowForTransparentFrame > - see above
        // - add condition <bFoundCellForTopOrBorderAttrs>
        //-hack.
        if ( ( bLine || bFoundCellForTopOrBorderAttrs ) &&
             !bDrawOnlyShadowForTransparentFrame )
        {
            const SwFrm* pDirRefFrm = IsCellFrm() ? FindTabFrm() : this;
            SWRECTFN( pDirRefFrm )
            ::lcl_PaintLeftRightLine ( true, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
            ::lcl_PaintLeftRightLine ( false, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
            if ( !IsCntntFrm() || rAttrs.GetTopLine( *(this) ) )
            {
                // -
                //-hack
                // paint is found, paint its top border.
                if ( IsCellFrm() && pCellFrmForTopBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(),
                                                pCellFrmForTopBorderAttrs );
                    const SwBorderAttrs &rTopAttrs = *aAccess.Get();
                    ::lcl_PaintTopBottomLine( true, *(this), *(pPage), aRect, rRect, rTopAttrs, fnRect );
                }
                else
                {
                    ::lcl_PaintTopBottomLine( true, *(this), *(pPage), aRect, rRect, rAttrs, fnRect );
                }
            }
            if ( !IsCntntFrm() || rAttrs.GetBottomLine( *(this) ) )
            {
                // -
                //-hack
                // paint is found, paint its bottom border.
                if ( IsCellFrm() && pCellFrmForBottomBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(),
                                                pCellFrmForBottomBorderAttrs );
                    const SwBorderAttrs &rBottomAttrs = *aAccess.Get();
                    ::lcl_PaintTopBottomLine(false, *(this), *(pPage), aRect, rRect, rBottomAttrs, fnRect);
                }
                else
                {
                    ::lcl_PaintTopBottomLine(false, *(this), *(pPage), aRect, rRect, rAttrs, fnRect);
                }
            }
        }
        rAttrs.SetGetCacheLine( sal_False );
    }
}
/*************************************************************************
|*
|*  SwFtnContFrm::PaintBorder()
|*
|*  Description         Special implementation because of the footnote line.
|*      Currently only the top frame needs to be taken into account.
|*      Other lines and shadows are set aside.
|*************************************************************************/

void SwFtnContFrm::PaintBorder( const SwRect& rRect, const SwPageFrm *pPage,
                                const SwBorderAttrs & ) const
{
    //If the rectangle is completely inside the PrtArea, no border needs to
    //be painted.
    SwRect aRect( Prt() );
    aRect.Pos() += Frm().Pos();
    if ( !aRect.IsInside( rRect ) )
        PaintLine( rRect, pPage );
}
/*************************************************************************
|*
|*  SwFtnContFrm::PaintLine()
|*
|*  Description         Paint footnote lines.
|*
|*************************************************************************/

void SwFtnContFrm::PaintLine( const SwRect& rRect,
                              const SwPageFrm *pPage ) const
{
    //The length of the line is derived from the percentual indication on the
    //PageDesc. The position is also stated on the PageDesc.
    //The pen can directly be taken from the PageDesc.

    if ( !pPage )
        pPage = FindPageFrm();
    const SwPageFtnInfo &rInf = pPage->GetPageDesc()->GetFtnInfo();

    SWRECTFN( this )
    SwTwips nPrtWidth = (Prt().*fnRect->fnGetWidth)();
    Fraction aFract( nPrtWidth, 1 );
    const SwTwips nWidth = (long)(aFract *= rInf.GetWidth());

    SwTwips nX = (this->*fnRect->fnGetPrtLeft)();
    switch ( rInf.GetAdj() )
    {
        case FTNADJ_CENTER:
            nX += nPrtWidth/2 - nWidth/2; break;
        case FTNADJ_RIGHT:
            nX += nPrtWidth - nWidth; break;
        case FTNADJ_LEFT:
            /* do nothing */; break;
        default:
            OSL_ENSURE( !this, "New adjustment for footnote lines?" );
    }
    SwTwips nLineWidth = rInf.GetLineWidth();
    const SwRect aLineRect = bVert ?
        SwRect( Point(Frm().Left()+Frm().Width()-rInf.GetTopDist()-nLineWidth,
                      nX), Size( nLineWidth, nWidth ) )
            : SwRect( Point( nX, Frm().Pos().Y() + rInf.GetTopDist() ),
                            Size( nWidth, rInf.GetLineWidth()));
    if ( aLineRect.HasArea() )
        PaintBorderLine( rRect, aLineRect , pPage, &rInf.GetLineColor(),
                rInf.GetLineStyle() );
}

/*************************************************************************
|*
|*  SwLayoutFrm::PaintColLines()
|*
|*  Description         Paints the separator line for inside columns
|*
|*************************************************************************/

void SwLayoutFrm::PaintColLines( const SwRect &rRect, const SwFmtCol &rFmtCol,
                                 const SwPageFrm *pPage ) const
{
    const SwFrm *pCol = Lower();
    if ( !pCol || !pCol->IsColumnFrm() )
        return;
    //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
    SwRectFn fnRect = pCol->IsVertical() ? ( pCol->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    SwRect aLineRect = Prt();
    aLineRect += Frm().Pos();

    SwTwips nTop = ((aLineRect.*fnRect->fnGetHeight)()*rFmtCol.GetLineHeight())
                   / 100 - (aLineRect.*fnRect->fnGetHeight)();
    SwTwips nBottom = 0;

    switch ( rFmtCol.GetLineAdj() )
    {
        case COLADJ_CENTER:
            nBottom = nTop / 2; nTop -= nBottom; break;
        case COLADJ_TOP:
            nBottom = nTop; nTop = 0; break;
        case COLADJ_BOTTOM:
            break;
        default:
            OSL_ENSURE( !this, "New adjustment for column lines?" );
    }

    if( nTop )
        (aLineRect.*fnRect->fnSubTop)( nTop );
    if( nBottom )
        (aLineRect.*fnRect->fnAddBottom)( nBottom );

    SwTwips nPenHalf = rFmtCol.GetLineWidth();
    (aLineRect.*fnRect->fnSetWidth)( nPenHalf );
    nPenHalf /= 2;

    //We need to be a bit generous here, to not lose something.
    SwRect aRect( rRect );
    (aRect.*fnRect->fnSubLeft)( nPenHalf + nPixelSzW );
    (aRect.*fnRect->fnAddRight)( nPenHalf + nPixelSzW );
    SwRectGet fnGetX = IsRightToLeft() ? fnRect->fnGetLeft : fnRect->fnGetRight;
    while ( pCol->GetNext() )
    {
        (aLineRect.*fnRect->fnSetPosX)
            ( (pCol->Frm().*fnGetX)() - nPenHalf );
        if ( aRect.IsOver( aLineRect ) )
            PaintBorderLine( aRect, aLineRect , pPage, &rFmtCol.GetLineColor(),
                   rFmtCol.GetLineStyle() );
        pCol = pCol->GetNext();
    }
}

void SwPageFrm::PaintGrid( OutputDevice* pOut, SwRect &rRect ) const
{
    if( !bHasGrid || pRetoucheFly || pRetoucheFly2 )
        return;
    GETGRID( this )
    if( pGrid && ( OUTDEV_PRINTER != pOut->GetOutDevType() ?
        pGrid->GetDisplayGrid() : pGrid->GetPrintGrid() ) )
    {
        const SwLayoutFrm* pBody = FindBodyCont();
        if( pBody )
        {
            SwRect aGrid( pBody->Prt() );
            aGrid += pBody->Frm().Pos();

            SwRect aInter( aGrid );
            aInter.Intersection( rRect );
            if( aInter.HasArea() )
            {
                sal_Bool bGrid = pGrid->GetRubyTextBelow();
                bool bCell = GRID_LINES_CHARS == pGrid->GetGridType();
                long nGrid = pGrid->GetBaseHeight();
                const SwDoc* pDoc = GetFmt()->GetDoc();
                long nGridWidth = GETGRIDWIDTH(pGrid,pDoc); //for textgrid refactor
                long nRuby = pGrid->GetRubyHeight();
                long nSum = nGrid + nRuby;
                const Color *pCol = &pGrid->GetColor();

                SwTwips nRight = aInter.Left() + aInter.Width();
                SwTwips nBottom = aInter.Top() + aInter.Height();
                if( IsVertical() )
                {
                    SwTwips nOrig = aGrid.Left() + aGrid.Width();
                    SwTwips nY = nOrig + nSum *
                                 ( ( nOrig - aInter.Left() ) / nSum );
                    SwRect aTmp( Point( nY, aInter.Top() ),
                                Size( 1, aInter.Height() ) );
                    SwTwips nX = aGrid.Top() + nGrid *
                                ( ( aInter.Top() - aGrid.Top() )/ nGrid );
                    if( nX < aInter.Top() )
                        nX += nGrid;
                    SwTwips nGridBottom = aGrid.Top() + aGrid.Height();
                    bool bLeft = aGrid.Top() >= aInter.Top();
                    bool bRight = nGridBottom <= nBottom;
                    bool bBorder = bLeft || bRight;
                    while( nY > nRight )
                    {
                        aTmp.Pos().X() = nY;
                        if( bGrid )
                        {
                            nY -= nGrid;
                            SwTwips nPosY = std::max( aInter.Left(), nY );
                            SwTwips nHeight = std::min(nRight, aTmp.Pos().X())-nPosY;
                            if( nHeight > 0 )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nPosY, nX ),
                                                Size( nHeight, 1 ) );
                                    while( aVert.Top() <= nBottom )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().Y() += nGrid;
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point( nPosY, aGrid.Top() ),
                                                  Size( nHeight, 1 ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY -= nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = std::max( aInter.Left(), nY );
                                SwTwips nW = std::min(nRight, aTmp.Pos().X()) - nPos;
                                SwRect aVert( Point( nPos, aGrid.Top() ),
                                              Size( nW, 1 ) );
                                if( nW > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                    while( nY >= aInter.Left() )
                    {
                        aTmp.Pos().X() = nY;
                        PaintBorderLine( rRect, aTmp, this, pCol);
                        if( bGrid )
                        {
                            nY -= nGrid;
                            SwTwips nHeight = aTmp.Pos().X()
                                              - std::max(aInter.Left(), nY );
                            if( nHeight > 0 )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point(aTmp.Pos().X()-nHeight,
                                                  nX ), Size( nHeight, 1 ) );
                                    while( aVert.Top() <= nBottom )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().Y() += nGrid;
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point(aTmp.Pos().X()-nHeight,
                                            aGrid.Top() ), Size( nHeight, 1 ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY -= nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = std::max( aInter.Left(), nY );
                                SwTwips nW = std::min(nRight, aTmp.Pos().X()) - nPos;
                                SwRect aVert( Point( nPos, aGrid.Top() ),
                                              Size( nW, 1 ) );
                                if( nW > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().Y() = nGridBottom;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                }
                else
                {
                    SwTwips nOrig = aGrid.Top();
                    SwTwips nY = nOrig + nSum *( (aInter.Top()-nOrig)/nSum );
                    SwRect aTmp( Point( aInter.Left(), nY ),
                                Size( aInter.Width(), 1 ) );
                    //for textgrid refactor
                    SwTwips nX = aGrid.Left() + nGridWidth *
                        ( ( aInter.Left() - aGrid.Left() )/ nGridWidth );
                    if( nX < aInter.Left() )
                        nX += nGridWidth;
                    SwTwips nGridRight = aGrid.Left() + aGrid.Width();
                    bool bLeft = aGrid.Left() >= aInter.Left();
                    bool bRight = nGridRight <= nRight;
                    bool bBorder = bLeft || bRight;
                    while( nY < aInter.Top() )
                    {
                        aTmp.Pos().setY(nY);
                        if( bGrid )
                        {
                            nY += nGrid;
                            SwTwips nPosY = std::max( aInter.Top(), aTmp.Pos().getY() );
                            SwTwips nHeight = std::min(nBottom, nY ) - nPosY;
                            if( nHeight )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nX, nPosY ),
                                                Size( 1, nHeight ) );
                                    while( aVert.Left() <= nRight )
                                    {
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                        aVert.Pos().X() += nGridWidth;  //for textgrid refactor
                                    }
                                }
                                else if ( bBorder )
                                {
                                    SwRect aVert( Point( aGrid.Left(), nPosY ),
                                                Size( 1, nHeight ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().X() = nGridRight;
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY += nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = std::max(aInter.Top(),aTmp.Pos().getY());
                                SwTwips nH = std::min( nBottom, nY ) - nPos;
                                SwRect aVert( Point( aGrid.Left(), nPos ),
                                            Size( 1, nH ) );
                                if( nH > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().setX(nGridRight);
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                    while( nY <= nBottom )
                    {
                        aTmp.Pos().setY(nY);
                        PaintBorderLine( rRect, aTmp, this, pCol);
                        if( bGrid )
                        {
                            nY += nGrid;
                            SwTwips nHeight = std::min(nBottom, nY) - aTmp.Pos().getY();
                            if( nHeight )
                            {
                                if( bCell )
                                {
                                    SwRect aVert( Point( nX, aTmp.Pos().getY() ),
                                                Size( 1, nHeight ) );
                                    while( aVert.Left() <= nRight )
                                    {
                                        PaintBorderLine( rRect, aVert, this, pCol);
                                        aVert.Pos().setX(aVert.Pos().getX() + nGridWidth);  //for textgrid refactor
                                    }
                                }
                                else if( bBorder )
                                {
                                    SwRect aVert( Point( aGrid.Left(),
                                        aTmp.Pos().getY() ), Size( 1, nHeight ) );
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().setX(nGridRight);
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        else
                        {
                            nY += nRuby;
                            if( bBorder )
                            {
                                SwTwips nPos = std::max(aInter.Top(),aTmp.Pos().Y());
                                SwTwips nH = std::min( nBottom, nY ) - nPos;
                                SwRect aVert( Point( aGrid.Left(), nPos ),
                                            Size( 1, nH ) );
                                if( nH > 0 )
                                {
                                    if( bLeft )
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    if( bRight )
                                    {
                                        aVert.Pos().setX(nGridRight);
                                        PaintBorderLine(rRect,aVert,this,pCol);
                                    }
                                }
                            }
                        }
                        bGrid = !bGrid;
                    }
                }
            }
        }
    }
}

/** paint margin area of a page

    OD 20.11.2002 for #104598#:
    implement paint of margin area; margin area will be painted for a
    view shell with a window and if the document is not in online layout.

    @author OD

    @param _rOutputRect
    input parameter - constant instance reference of the rectangle, for
    which an output has to be generated.

    @param _pViewShell
    input parameter - instance of the view shell, on which the output
    has to be generated.
*/
void SwPageFrm::PaintMarginArea( const SwRect& _rOutputRect,
                                 ViewShell* _pViewShell ) const
{
    if (  _pViewShell->GetWin() &&
         !_pViewShell->GetViewOptions()->getBrowseMode() )
    {
        SwRect aPgPrtRect( Prt() );
        aPgPrtRect.Pos() += Frm().Pos();
        if ( !aPgPrtRect.IsInside( _rOutputRect ) )
        {
            SwRect aPgRect = Frm();
            aPgRect._Intersection( _rOutputRect );
            if(aPgRect.Height() < 0 || aPgRect.Width() <= 0)    // No intersection
                return;
            SwRegionRects aPgRegion( aPgRect );
            aPgRegion -= aPgPrtRect;
            const SwPageFrm* pPage = static_cast<const SwPageFrm*>(this);
            if ( pPage->GetSortedObjs() )
                ::lcl_SubtractFlys( this, pPage, aPgRect, aPgRegion );
            if ( !aPgRegion.empty() )
            {
                OutputDevice *pOut = _pViewShell->GetOut();
                if ( pOut->GetFillColor() != aGlobalRetoucheColor )
                    pOut->SetFillColor( aGlobalRetoucheColor );
                for ( sal_uInt16 i = 0; i < aPgRegion.size(); ++i )
                {
                    if ( 1 < aPgRegion.size() )
                    {
                        ::SwAlignRect( aPgRegion[i], pGlobalShell );
                        if( !aPgRegion[i].HasArea() )
                            continue;
                    }
                    pOut->DrawRect(aPgRegion[i].SVRect());
                }
            }
        }
    }
}

const sal_Int8 SwPageFrm::mnShadowPxWidth = 9;

sal_Bool SwPageFrm::IsRightShadowNeeded() const
{
    const ViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bIsLTR = getRootFrm()->IsLeftToRightViewLayout();

    // We paint the right shadow if we're not in book mode
    // or if we've no sibling or are the last page of the "row"
    return !pSh || (!pSh->GetViewOptions()->IsViewLayoutBookMode()) || !GetNext()
        || (this == Lower())  || (bIsLTR && OnRightPage())
        || (!bIsLTR && !OnRightPage());

}

sal_Bool SwPageFrm::IsLeftShadowNeeded() const
{
    const ViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bIsLTR = getRootFrm()->IsLeftToRightViewLayout();

    // We paint the left shadow if we're not in book mode
    // or if we've no sibling or are the last page of the "row"
    return !pSh || (!pSh->GetViewOptions()->IsViewLayoutBookMode()) || !GetPrev()
        || (bIsLTR && !OnRightPage())
        || (!bIsLTR && OnRightPage());
}

/** determine rectangle for bottom page shadow

    OD 12.02.2003 for #i9719# and #105645#

    @author OD
*/
/*static*/ void SwPageFrm::GetHorizontalShadowRect( const SwRect& _rPageRect,
                                                const ViewShell*    _pViewShell,
                                                SwRect&       _orHorizontalShadowRect,
                                                bool bPaintLeftShadow,
                                                bool bPaintRightShadow,
                                                bool bRightSidebar )
{
    const SwPostItMgr *pMgr = _pViewShell ? _pViewShell->GetPostItMgr() : 0;
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell );
    SwRect aPagePxRect =
            _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );

    long lShadowAdjustment = mnShadowPxWidth - 1; // TODO extract this

    _orHorizontalShadowRect.Chg(
                    Point( aPagePxRect.Left() + (bPaintLeftShadow ? lShadowAdjustment : 0), 0 ),
                    Size( aPagePxRect.Width() - ( (bPaintLeftShadow ? lShadowAdjustment : 0) + (bPaintRightShadow ? lShadowAdjustment : 0) ),
                        mnShadowPxWidth ) );

    if(pMgr && pMgr->ShowNotes() && pMgr->HasNotes())
    {
        // Notes are displayed, we've to extend borders
        SwTwips aSidebarTotalWidth = pMgr->GetSidebarWidth(true) + pMgr->GetSidebarBorderWidth(true);
        if(bRightSidebar)
            _orHorizontalShadowRect.Right( _orHorizontalShadowRect.Right() + aSidebarTotalWidth );
        else
            _orHorizontalShadowRect.Left( _orHorizontalShadowRect.Left() - aSidebarTotalWidth );
    }
}

enum PaintArea {LEFT, RIGHT, TOP, BOTTOM};

/// Wrapper around pOut->DrawBitmapEx.
static void lcl_paintBitmapExToRect(OutputDevice *pOut, Point aPoint, BitmapEx& rBitmapEx, PaintArea eArea)
{
    // The problem is that if we get called multiple times and the color is
    // partly transparent, then the result will get darker and darker. To avoid
    // this, always paint the background color before doing the real paint.
    Rectangle aRect(aPoint, rBitmapEx.GetSizePixel());

    switch (eArea)
    {
        case LEFT: aRect.Left() = aRect.Right() - 1; break;
        case RIGHT: aRect.Right() = aRect.Left() + 1; break;
        case TOP: aRect.Top() = aRect.Bottom() - 1; break;
        case BOTTOM: aRect.Bottom() = aRect.Top() + 1; break;
    }

    pOut->SetFillColor( SwViewOption::GetAppBackgroundColor());
    pOut->SetLineColor();
    pOut->DrawRect(pOut->PixelToLogic(aRect));

    pOut->DrawBitmapEx(pOut->PixelToLogic(aPoint), rBitmapEx);
}

/** paint page border and shadow

    OD 12.02.2003 for #i9719# and #105645#
    implement paint of page border and shadow

    @author OD
*/
/*static*/ void SwPageFrm::PaintBorderAndShadow( const SwRect& _rPageRect,
                                                 const ViewShell*    _pViewShell,
                                                 bool bPaintLeftShadow,
                                                 bool bPaintRightShadow,
                                                 bool bRightSidebar )
{
    // No shadow in prefs
    if( !SwViewOption::IsShadow() ) return;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *_pViewShell->GetOut() );

    static vcl::DeleteOnDeinit< drawinglayer::primitive2d::DiscreteShadow > shadowMaskObj
        ( new drawinglayer::primitive2d::DiscreteShadow( SW_RES( BMP_PAGE_SHADOW_MASK ) ));
    static drawinglayer::primitive2d::DiscreteShadow& shadowMask = *shadowMaskObj.get();
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopRightShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomRightShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomLeftShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageRightShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopLeftShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageLeftShadowBaseObj( new BitmapEx );
    static BitmapEx& aPageTopRightShadow = *aPageTopRightShadowObj.get();
    static BitmapEx& aPageBottomRightShadow = *aPageBottomRightShadowObj.get();
    static BitmapEx& aPageBottomLeftShadow = *aPageBottomLeftShadowObj.get();
    static BitmapEx& aPageBottomShadowBase = *aPageBottomShadowBaseObj.get();
    static BitmapEx& aPageRightShadowBase = *aPageRightShadowBaseObj.get();
    static BitmapEx& aPageTopShadowBase = *aPageTopShadowBaseObj.get();
    static BitmapEx& aPageTopLeftShadow = *aPageTopLeftShadowObj.get();
    static BitmapEx& aPageLeftShadowBase = *aPageLeftShadowBaseObj.get();
    static Color aShadowColor( COL_AUTO );

    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell );
    SwRect aPagePxRect =
        _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );


    if(aShadowColor != SwViewOption::GetShadowColor() ) {
        aShadowColor = SwViewOption::GetShadowColor();

        AlphaMask aMask( shadowMask.getBottomRight().GetBitmap() );
        Bitmap aFilledSquare( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageBottomRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getBottomLeft().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageBottomLeftShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getBottom().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageBottomShadowBase = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTop().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopShadowBase = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopRight().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getRight().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageRightShadowBase = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopLeft().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopLeftShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getLeft().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageLeftShadowBase = BitmapEx( aFilledSquare, aMask );
    }

    SwRect aPaintRect;
    OutputDevice *pOut = _pViewShell->GetOut();

    SwPageFrm::GetHorizontalShadowRect( _rPageRect, _pViewShell, aPaintRect, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

    // Right shadow & corners
    if ( bPaintRightShadow )
    {
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right() + 1, aPagePxRect.Bottom() + 1 - (aPageBottomRightShadow.GetSizePixel().Height() - mnShadowPxWidth) ) ),
            aPageBottomRightShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right() + 1, aPagePxRect.Top() - mnShadowPxWidth ) ),
            aPageTopRightShadow );

        if ( aPagePxRect.Height() > 2 * mnShadowPxWidth)
        {
            BitmapEx aPageRightShadow = aPageRightShadowBase;
            aPageRightShadow.Scale( 1, aPagePxRect.Height() - 2 * (mnShadowPxWidth - 1) );
            lcl_paintBitmapExToRect( pOut, Point( aPaintRect.Right() + mnShadowPxWidth,
                                                 aPagePxRect.Top() + mnShadowPxWidth - 1),
                                     aPageRightShadow, RIGHT );
        }
    }

    // Left shadows and corners
    if(bPaintLeftShadow)
    {
        const long lLeft = aPaintRect.Left() - aPageBottomLeftShadow.GetSizePixel().Width();
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft,
            aPagePxRect.Bottom() + 1 + mnShadowPxWidth - aPageBottomLeftShadow.GetSizePixel().Height() ) ), aPageBottomLeftShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft, aPagePxRect.Top() - mnShadowPxWidth ) ), aPageTopLeftShadow );
        if ( aPagePxRect.Height() > 2 * mnShadowPxWidth)
        {
            BitmapEx aPageLeftShadow = aPageLeftShadowBase;
            aPageLeftShadow.Scale( 1, aPagePxRect.Height() - 2 * (mnShadowPxWidth - 1) );
            lcl_paintBitmapExToRect( pOut, Point( lLeft, aPagePxRect.Top() + mnShadowPxWidth - 1),
                                     aPageLeftShadow, LEFT );
        }
    }

    BitmapEx aPageBottomShadow = aPageBottomShadowBase;
    aPageBottomShadow.Scale( aPaintRect.Width(), 1 );
    lcl_paintBitmapExToRect( pOut, Point( aPaintRect.Left(), aPagePxRect.Bottom() + 1 ),
                             aPageBottomShadow, BOTTOM );
    BitmapEx aPageTopShadow = aPageTopShadowBase;
    aPageTopShadow.Scale( aPaintRect.Width(), 1 );
    lcl_paintBitmapExToRect( pOut, Point( aPaintRect.Left(), aPagePxRect.Top() - mnShadowPxWidth ),
                             aPageTopShadow, TOP );
}

//mod #i6193# paint sidebar for notes
//IMPORTANT: if you change the rects here, also change SwPostItMgr::ScrollbarHit
/*static*/void SwPageFrm::PaintNotesSidebar(const SwRect& _rPageRect, ViewShell* _pViewShell, sal_uInt16 nPageNum, bool bRight)
{
    //TODO: cut out scrollbar area and arrows out of sidepane rect, otherwise it could flicker when pressing arrow buttons
    if (!_pViewShell )
        return;

    SwRect aPageRect( _rPageRect );
    SwAlignRect( aPageRect, _pViewShell );

    const SwPostItMgr *pMgr = _pViewShell->GetPostItMgr();
    if (pMgr && pMgr->ShowNotes() && pMgr->HasNotes())  // do not show anything in print preview
    {
        sal_Int32 nScrollerHeight = pMgr->GetSidebarScrollerHeight();
        const Rectangle &aVisRect = _pViewShell->VisArea().SVRect();
        //draw border and sidepane
        _pViewShell->GetOut()->SetLineColor();
        if (!bRight)
        {
            _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE_BORDER);
            _pViewShell->GetOut()->DrawRect(Rectangle(Point(aPageRect.Left()-pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarBorderWidth(),aPageRect.Height())))    ;
            if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                _pViewShell->GetOut()->SetFillColor(COL_BLACK);
            else
                _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE);
            _pViewShell->GetOut()->DrawRect(Rectangle(Point(aPageRect.Left()-pMgr->GetSidebarWidth()-pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarWidth(),aPageRect.Height())))  ;
        }
        else
        {
            _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE_BORDER);
            SwRect aSidebarBorder(aPageRect.TopRight(),Size(pMgr->GetSidebarBorderWidth(),aPageRect.Height()));
            _pViewShell->GetOut()->DrawRect(aSidebarBorder.SVRect());
            if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                _pViewShell->GetOut()->SetFillColor(COL_BLACK);
            else
                _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE);
            SwRect aSidebar(Point(aPageRect.Right()+pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarWidth(),aPageRect.Height()));
            _pViewShell->GetOut()->DrawRect(aSidebar.SVRect());
        }
        if (pMgr->ShowScrollbar(nPageNum))
        {
            // draw scrollbar area and arrows
            Point aPointBottom;
            Point aPointTop;
            aPointBottom = !bRight ? Point(aPageRect.Left() - pMgr->GetSidebarWidth() - pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- _pViewShell->GetOut()->PixelToLogic(Size(0,2+pMgr->GetSidebarScrollerHeight())).Height()) :
                                    Point(aPageRect.Right() + pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- _pViewShell->GetOut()->PixelToLogic(Size(0,2+pMgr->GetSidebarScrollerHeight())).Height());
            aPointTop = !bRight ?    Point(aPageRect.Left() - pMgr->GetSidebarWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + _pViewShell->GetOut()->PixelToLogic(Size(0,2)).Height()) :
                                Point(aPageRect.Right() + pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + _pViewShell->GetOut()->PixelToLogic(Size(0,2)).Height());
            Size aSize(pMgr->GetSidebarWidth() - _pViewShell->GetOut()->PixelToLogic(Size(4,0)).Width(), _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()) ;
            Rectangle aRectBottom(aPointBottom,aSize);
            Rectangle aRectTop(aPointTop,aSize);

            if (aRectBottom.IsOver(aVisRect))
            {

                if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                {
                    _pViewShell->GetOut()->SetLineColor(COL_WHITE);
                    _pViewShell->GetOut()->SetFillColor(COL_BLACK);
                }
                else
                {
                    _pViewShell->GetOut()->SetLineColor(COL_BLACK);
                    _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE_SCROLLAREA);
                }
                _pViewShell->GetOut()->DrawRect(aRectBottom);
                _pViewShell->GetOut()->DrawLine(aPointBottom + Point(pMgr->GetSidebarWidth()/3,0), aPointBottom + Point(pMgr->GetSidebarWidth()/3 , _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()));

                _pViewShell->GetOut()->SetLineColor();
                Point aMiddleFirst(aPointBottom + Point(pMgr->GetSidebarWidth()/6,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
                Point aMiddleSecond(aPointBottom + Point(pMgr->GetSidebarWidth()/3*2,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
                PaintNotesSidebarArrows(aMiddleFirst,aMiddleSecond,_pViewShell,pMgr->GetArrowColor(KEY_PAGEUP,nPageNum), pMgr->GetArrowColor(KEY_PAGEDOWN,nPageNum));
            }
            if (aRectTop.IsOver(aVisRect))
            {
                if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                {
                    _pViewShell->GetOut()->SetLineColor(COL_WHITE);
                    _pViewShell->GetOut()->SetFillColor(COL_BLACK);
                }
                else
                {
                    _pViewShell->GetOut()->SetLineColor(COL_BLACK);
                    _pViewShell->GetOut()->SetFillColor(COL_NOTES_SIDEPANE_SCROLLAREA);
                }
                _pViewShell->GetOut()->DrawRect(aRectTop);
                _pViewShell->GetOut()->DrawLine(aPointTop + Point(pMgr->GetSidebarWidth()/3*2,0), aPointTop + Point(pMgr->GetSidebarWidth()/3*2 , _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()));

                _pViewShell->GetOut()->SetLineColor();
                Point aMiddleFirst(aPointTop + Point(pMgr->GetSidebarWidth()/3,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
                Point aMiddleSecond(aPointTop + Point(pMgr->GetSidebarWidth()/6*5,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
                PaintNotesSidebarArrows(aMiddleFirst,aMiddleSecond,_pViewShell, pMgr->GetArrowColor(KEY_PAGEUP,nPageNum), pMgr->GetArrowColor(KEY_PAGEDOWN,nPageNum));
            }
        }
    }
}

/*static*/ void SwPageFrm::PaintNotesSidebarArrows(const Point &aMiddleFirst, const Point &aMiddleSecond, ViewShell* _pViewShell, const Color aColorUp, const Color aColorDown)
{
    Polygon aTriangleUp(3);
    Polygon aTriangleDown(3);

    aTriangleUp.SetPoint(aMiddleFirst + Point(0,_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),0);
    aTriangleUp.SetPoint(aMiddleFirst + Point(_pViewShell->GetOut()->PixelToLogic(Size(-3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),1);
    aTriangleUp.SetPoint(aMiddleFirst + Point(_pViewShell->GetOut()->PixelToLogic(Size(3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),2);

    aTriangleDown.SetPoint(aMiddleSecond + Point(_pViewShell->GetOut()->PixelToLogic(Size(-3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),0);
    aTriangleDown.SetPoint(aMiddleSecond + Point(_pViewShell->GetOut()->PixelToLogic(Size(+3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),1);
    aTriangleDown.SetPoint(aMiddleSecond + Point(0,_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),2);

    _pViewShell->GetOut()->SetFillColor(aColorUp);
    _pViewShell->GetOut()->DrawPolygon(aTriangleUp);
    _pViewShell->GetOut()->SetFillColor(aColorDown);
    _pViewShell->GetOut()->DrawPolygon(aTriangleDown);
}

/** get bound rectangle of border and shadow for repaints

    OD 12.02.2003 for #i9719# and #105645#

    author OD
*/
/*static*/ void SwPageFrm::GetBorderAndShadowBoundRect( const SwRect& _rPageRect,
                                                        const ViewShell*    _pViewShell,
                                                        SwRect& _orBorderAndShadowBoundRect,
                                                        bool bLeftShadow,
                                                        bool bRightShadow,
                                                        bool bRightSidebar
                                                      )
{
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell );
    SwRect aPagePxRect =
            _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );
    aPagePxRect.Bottom( aPagePxRect.Bottom() + mnShadowPxWidth + 1 );
    aPagePxRect.Top( aPagePxRect.Top() - mnShadowPxWidth - 1 );

    SwRect aTmpRect;

    // Always ask for full shadow since we want a bounding rect
    // including at least the page frame
    SwPageFrm::GetHorizontalShadowRect( _rPageRect, _pViewShell, aTmpRect, false, false, bRightSidebar );

    if(bLeftShadow) aPagePxRect.Left( aTmpRect.Left() - mnShadowPxWidth - 1);
    if(bRightShadow) aPagePxRect.Right( aTmpRect.Right() + mnShadowPxWidth + 1);

    _orBorderAndShadowBoundRect = _pViewShell->GetOut()->PixelToLogic( aPagePxRect.SVRect() );
}

SwRect SwPageFrm::GetBoundRect() const
{
    const ViewShell *pSh = getRootFrm()->GetCurrShell();
    SwRect aPageRect( Frm() );
    SwRect aResult;

    if(!pSh) {
        return SwRect( Point(0, 0), Size(0, 0) );
    }

    SwPageFrm::GetBorderAndShadowBoundRect( aPageRect, pSh, aResult,
        IsLeftShadowNeeded(), IsRightShadowNeeded(), SidebarPosition() ==  sw::sidebarwindows::SIDEBAR_RIGHT );
    return aResult;
}

/*static*/ SwTwips SwPageFrm::GetSidebarBorderWidth( const ViewShell* _pViewShell )
{
    const SwPostItMgr* pPostItMgr = _pViewShell ? _pViewShell->GetPostItMgr() : 0;
    const SwTwips nRet = pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() ? pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() : 0;
    return nRet;
}

/*************************************************************************
|*
|*  SwFrm::PaintBaBo()
|*
|*************************************************************************/

void SwFrm::PaintBaBo( const SwRect& rRect, const SwPageFrm *pPage,
                       const sal_Bool bLowerBorder, const bool bOnlyTxtBackground ) const
{
    if ( !pPage )
        pPage = FindPageFrm();

    OutputDevice *pOut = pGlobalShell->GetOut();

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pOut );

    // OD 2004-04-23 #116347#
    pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
    pOut->SetLineColor();

    SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    // OD 20.11.2002 #104598# - take care of page margin area
    // Note: code move from <SwFrm::PaintBackground(..)> to new method
    // <SwPageFrm::Paintmargin(..)>.
    if ( IsPageFrm() && !bOnlyTxtBackground)
    {
        static_cast<const SwPageFrm*>(this)->PaintMarginArea( rRect, pGlobalShell );
    }

    // paint background
    {
        PaintBackground( rRect, pPage, rAttrs, sal_False, bLowerBorder, bOnlyTxtBackground );
    }

    // OD 06.08.2002 #99657# - paint border before painting background
    // paint grid for page frame and paint border
    if (!bOnlyTxtBackground)
    {
        SwRect aRect( rRect );
        if( IsPageFrm() )
            ((SwPageFrm*)this)->PaintGrid( pOut, aRect );
        PaintBorder( aRect, pPage, rAttrs );
    }

    pOut->Pop();
}

/*************************************************************************
|*
|*  SwFrm::PaintBackground()
|*
|*************************************************************************/
/// OD 05.09.2002 #102912#
/// Do not paint background for fly frames without a background brush by
/// calling <PaintBaBo> at the page or at the fly frame its anchored
void SwFrm::PaintBackground( const SwRect &rRect, const SwPageFrm *pPage,
                              const SwBorderAttrs & rAttrs,
                             const sal_Bool bLowerMode,
                             const sal_Bool bLowerBorder,
                             const bool bOnlyTxtBackground ) const
{
    // OD 20.01.2003 #i1837# - no paint of table background, if corresponding
    // option is *not* set.
    if( IsTabFrm() &&
        !pGlobalShell->GetViewOptions()->IsTable() )
    {
        return;
    }

    // nothing to do for covered table cells:
    if( IsCellFrm() && IsCoveredCell() )
        return;

    ViewShell *pSh = pGlobalShell;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pSh->GetOut() );

    const SvxBrushItem* pItem;
    const XFillStyleItem* pFillStyleItem;
    const XFillGradientItem* pFillGradientItem;
    /// OD 05.09.2002 #102912#
    /// temporary background brush for a fly frame without a background brush
    SvxBrushItem* pTmpBackBrush = 0;
    const Color* pCol;
    SwRect aOrigBackRect;
    const bool bPageFrm = IsPageFrm();
    sal_Bool bLowMode = sal_True;

    sal_Bool bBack = GetBackgroundBrush( pItem, pFillStyleItem, pFillGradientItem, pCol, aOrigBackRect, bLowerMode );
    //- Output if a separate background is used.
    bool bNoFlyBackground = !bFlyMetafile && !bBack && IsFlyFrm();
    if ( bNoFlyBackground )
    {
        // OD 05.09.2002 #102912# - Fly frame has no background.
        // Try to find background brush at parents, if previous call of
        // <GetBackgroundBrush> disabled this option with the parameter <bLowerMode>
        if ( bLowerMode )
        {
            bBack = GetBackgroundBrush( pItem, pFillStyleItem, pFillGradientItem, pCol, aOrigBackRect, false );
        }
        // If still no background found for the fly frame, initialize the
        // background brush <pItem> with global retouche color and set <bBack>
        // to sal_True, that fly frame will paint its background using this color.
        if ( !bBack )
        {
            // OD 10.01.2003 #i6467# - on print output, pdf output and
            // in embedded mode not editing color COL_WHITE is used instead of
            // the global retouche color.
            if ( pSh->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
                 pSh->GetViewOptions()->IsPDFExport() ||
                 ( pSh->GetDoc()->GetDocShell()->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED &&
                   !pSh->GetDoc()->GetDocShell()->IsInPlaceActive()
                 )
               )
            {
                pTmpBackBrush = new SvxBrushItem( Color( COL_WHITE ), RES_BACKGROUND );
            }
            else
            {
                pTmpBackBrush = new SvxBrushItem( aGlobalRetoucheColor, RES_BACKGROUND);
            }
            pItem = pTmpBackBrush;
            bBack = true;
        }
    }

    SwRect aPaintRect( Frm() );
    if( IsTxtFrm() || IsSctFrm() )
        aPaintRect = UnionFrm( sal_True );

    if ( aPaintRect.IsOver( rRect ) )
    {
        if ( bBack || bPageFrm || !bLowerMode )
        {
            const sal_Bool bBrowse = pSh->GetViewOptions()->getBrowseMode();
            SwRect aRect;
            if ( (bPageFrm && bBrowse) ||
                 (IsTxtFrm() && Prt().SSize() == Frm().SSize()) )
            {
                aRect = Frm();
                ::SwAlignRect( aRect, pGlobalShell );
            }
            else
            {
                ::lcl_CalcBorderRect( aRect, this, rAttrs, false );
                if ( (IsTxtFrm() || IsTabFrm()) && GetPrev() )
                {
                    if ( GetPrev()->GetAttrSet()->GetBackground() ==
                         GetAttrSet()->GetBackground() )
                    {
                        aRect.Top( Frm().Top() );
                    }
                }
            }
            aRect.Intersection( rRect );

            OutputDevice *pOut = pSh->GetOut();

            if ( aRect.HasArea() )
            {
                SvxBrushItem* pNewItem = 0;
                SwRegionRects aRegion( aRect );
                if( pCol )
                {
                    pNewItem = new SvxBrushItem( *pCol, RES_BACKGROUND );
                    pItem = pNewItem;
                }
                if ( pPage->GetSortedObjs() )
                    ::lcl_SubtractFlys( this, pPage, aRect, aRegion );

                /// OD 06.08.2002 #99657# - determine, if background transparency
                ///     have to be considered for drawing.
                ///     --> Status Quo: background transparency have to be
                ///        considered for fly frames
                const sal_Bool bConsiderBackgroundTransparency = IsFlyFrm();
                if (!pFillStyleItem || pFillStyleItem->GetValue() != XFILL_GRADIENT || !pFillGradientItem)
                {
                    for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                    {
                        if ( 1 < aRegion.size() )
                        {
                            ::SwAlignRect( aRegion[i], pGlobalShell );
                            if( !aRegion[i].HasArea() )
                                continue;
                        }
                        /// OD 06.08.2002 #99657# - add 6th parameter to indicate, if
                        ///     background transparency have to be considered
                        ///     Set missing 5th parameter to the default value GRFNUM_NO
                        ///         - see declaration in /core/inc/frmtool.hxx.
                        if (IsTxtFrm() || !bOnlyTxtBackground)
                            ::DrawGraphic( pItem, pFillStyleItem, pFillGradientItem, pOut, aOrigBackRect, aRegion[i], GRFNUM_NO,
                                    bConsiderBackgroundTransparency );
                    }
                }
                else
                {
                    ::DrawGraphic( pItem, pFillStyleItem, pFillGradientItem, pOut, aOrigBackRect, aRect, GRFNUM_NO,
                            bConsiderBackgroundTransparency );
                }
                if( pCol )
                    delete pNewItem;
            }
        }
        else
            bLowMode = bLowerMode ? sal_True : sal_False;
    }

    /// OD 05.09.2002 #102912#
    /// delete temporary background brush.
    delete pTmpBackBrush;

    //Now process lower and his neighbour.
    //We end this as soon as a Frm leaves the chain and therefore is not a lower
    //of me anymore
    const SwFrm *pFrm = GetLower();
    if ( pFrm )
    {
        SwRect aFrmRect;
        SwRect aRect( PaintArea() );
        aRect._Intersection( rRect );
        SwRect aBorderRect( aRect );
        SwShortCut aShortCut( *pFrm, aBorderRect );
        do
        {   if ( pProgress )
                pProgress->Reschedule();

            aFrmRect = pFrm->PaintArea();
            if ( aFrmRect.IsOver( aBorderRect ) )
            {
                SwBorderAttrAccess aAccess( SwFrm::GetCache(), (SwFrm*)pFrm );
                const SwBorderAttrs &rTmpAttrs = *aAccess.Get();
                if ( ( pFrm->IsLayoutFrm() && bLowerBorder ) ||
                     aFrmRect.IsOver( aRect ) )
                    pFrm->PaintBackground( aRect, pPage, rTmpAttrs, bLowMode,
                                           bLowerBorder );
                if ( bLowerBorder )
                    pFrm->PaintBorder( aBorderRect, pPage, rTmpAttrs );
            }
            pFrm = pFrm->GetNext();
        } while ( pFrm && pFrm->GetUpper() == this &&
                  !aShortCut.Stop( aFrmRect ) );
    }
}

/*************************************************************************
|*
|*  SwPageFrm::RefreshSubsidiary()
|*
|*  Description         Refreshes all subsidiary lines of a page.
|*
|*************************************************************************/

void SwPageFrm::RefreshSubsidiary( const SwRect &rRect ) const
{
    if ( IS_SUBS || IS_SUBS_TABLE || IS_SUBS_SECTION || IS_SUBS_FLYS )
    {
        SwRect aRect( rRect );
        // OD 18.02.2003 #104989# - Not necessary and incorrect alignment of
        // the output rectangle.
        //::SwAlignRect( aRect, pGlobalShell );
        if ( aRect.HasArea() )
        {
            //During paint using the root, the array is controlled from there.
            //Otherwise we'll handle it for our self.
            bool bDelSubs = false;
            if ( !pSubsLines )
            {
                pSubsLines = new SwSubsRects;
                // OD 20.12.2002 #106318# - create container for special subsidiary lines
                pSpecSubsLines = new SwSubsRects;
                bDelSubs = true;
            }

            RefreshLaySubsidiary( this, aRect );

            if ( bDelSubs )
            {
                // OD 20.12.2002 #106318# - paint special subsidiary lines
                // and delete its container
                pSpecSubsLines->PaintSubsidiary( pGlobalShell->GetOut(), NULL );
                DELETEZ( pSpecSubsLines );

                pSubsLines->PaintSubsidiary( pGlobalShell->GetOut(), pLines );
                DELETEZ( pSubsLines );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::RefreshLaySubsidiary()
|*
|*************************************************************************/
void SwLayoutFrm::RefreshLaySubsidiary( const SwPageFrm *pPage,
                                        const SwRect &rRect ) const
{
    const sal_Bool bSubsOpt   = IS_SUBS;
    if ( bSubsOpt )
        PaintSubsidiaryLines( pPage, rRect );

    const SwFrm *pLow = Lower();
    if( !pLow )
        return;
    SwShortCut aShortCut( *pLow, rRect );
    while( pLow && !aShortCut.Stop( pLow->Frm() ) )
    {
        if ( pLow->Frm().IsOver( rRect ) && pLow->Frm().HasArea() )
        {
            if ( pLow->IsLayoutFrm() )
                ((const SwLayoutFrm*)pLow)->RefreshLaySubsidiary( pPage, rRect);
            else if ( pLow->GetDrawObjs() )
            {
                const SwSortedObjs& rObjs = *(pLow->GetDrawObjs());
                for ( sal_uInt32 i = 0; i < rObjs.Count(); ++i )
                {
                    const SwAnchoredObject* pAnchoredObj = rObjs[i];
                    if ( pPage->GetFmt()->GetDoc()->IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                         pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        const SwFlyFrm *pFly =
                                    static_cast<const SwFlyFrm*>(pAnchoredObj);
                        if ( pFly->IsFlyInCntFrm() && pFly->Frm().IsOver( rRect ) )
                        {
                            if ( !pFly->Lower() || !pFly->Lower()->IsNoTxtFrm() ||
                                 !((SwNoTxtFrm*)pFly->Lower())->HasAnimation())
                                pFly->RefreshLaySubsidiary( pPage, rRect );
                        }
                    }
                }
            }
        }
        pLow = pLow->GetNext();
    }
}

/*************************************************************************
|*
|*  SwLayoutFrm::PaintSubsidiaryLines()
|*
|*  Description         Subsidiary lines to paint the PrtAreas
|*      Only the LayoutFrms which directly contain Cntnt.
|*
|*************************************************************************/

//Paints the desired line and pays attention to not overpaint any flys.
// OD 18.11.2002 #99672# - new parameter <_pSubsLines>
static void lcl_RefreshLine( const SwLayoutFrm *pLay,
                                  const SwPageFrm *pPage,
                                  const Point &rP1,
                                  const Point &rP2,
                                  const sal_uInt8 nSubColor,
                                  SwLineRects* _pSubsLines )
{
    //In which direction do we loop? Can only be horizontal or vertical.
    OSL_ENSURE( ((rP1.X() == rP2.X()) || (rP1.Y() == rP2.Y())),
            "Sloped subsidiary lines are not allowed." );

    const bool bHori = rP1.Y() == rP2.Y();

    // use pointers to member function in order to unify flow
    typedef long& (Point:: *pmfPt)();
    const pmfPt pmfPtX = &Point::X;
    const pmfPt pmfPtY = &Point::Y;
    const pmfPt pDirPt = bHori ? pmfPtX : pmfPtY;

    Point aP1( rP1 );
    Point aP2( rP2 );

    while ( (aP1.*pDirPt)() < (aP2.*pDirPt)() )
    {
        //If the starting point lies in a fly, it is directly set behind the
        //fly.
        //The end point moves to the start if the end point lies in a fly or we
        //have a fly between starting point and end point.
        // In this way, every position is output one by one.

        //If I'm a fly I'll only avoid those flys which are places 'above' me;
        //this means those who are behind me in the array.
        //Even if I'm inside a fly or inside a fly inside a fly a.s.o I won't
        //avoid any of those flys.
        SwOrderIter aIter( pPage );
        const SwFlyFrm *pMyFly = pLay->FindFlyFrm();
        if ( pMyFly )
        {
            aIter.Current( pMyFly->GetVirtDrawObj() );
            while ( 0 != (pMyFly = pMyFly->GetAnchorFrm()->FindFlyFrm()) )
            {
                if ( aIter()->GetOrdNum() > pMyFly->GetVirtDrawObj()->GetOrdNum() )
                    aIter.Current( pMyFly->GetVirtDrawObj() );
            }
        }
        else
            aIter.Bottom();

        while ( aIter() )
        {
            const SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)aIter();
            const SwFlyFrm *pFly = pObj ? pObj->GetFlyFrm() : 0;

            //I certainly won't avoid myself, even if I'm placed _inside_ the
            //fly I won't avoid it.
            if ( !pFly || (pFly == pLay || pFly->IsAnLower( pLay )) )
            {
                aIter.Next();
                continue;
            }

            // OD 19.12.2002 #106318# - do *not* consider fly frames with
            // a transparent background.
            // OD 2004-02-12 #110582#-2 - do *not* consider fly frame, which
            // belongs to a invisible layer
            if ( pFly->IsBackgroundTransparent() ||
                 !pFly->GetFmt()->GetDoc()->IsVisibleLayerId( pObj->GetLayer() ) )
            {
                aIter.Next();
                continue;
            }

            //Is the Obj placed on the line
            const long nP1OthPt = !bHori ? rP1.X() : rP1.Y();
            const Rectangle &rBound = pObj->GetCurrentBoundRect();
            const Point aDrPt( rBound.TopLeft() );
            const long nDrOthPt = !bHori ? aDrPt.X() : aDrPt.Y();
            const Size  aDrSz( rBound.GetSize() );
            const long nDrOthSz = !bHori ? aDrSz.Width() : aDrSz.Height();

            if ( nP1OthPt >= nDrOthPt && nP1OthPt <= nDrOthPt + nDrOthSz )
            {
                const long nDrDirPt = bHori ? aDrPt.X() : aDrPt.Y();
                const long nDrDirSz = bHori ? aDrSz.Width() : aDrSz.Height();

                if ( (aP1.*pDirPt)() >= nDrDirPt && (aP1.*pDirPt)() <= nDrDirPt + nDrDirSz )
                    (aP1.*pDirPt)() = nDrDirPt + nDrDirSz;

                if ( (aP2.*pDirPt)() >= nDrDirPt && (aP1.*pDirPt)() < (nDrDirPt - 1) )
                    (aP2.*pDirPt)() = nDrDirPt - 1;
            }
            aIter.Next();
        }

        if ( (aP1.*pDirPt)() < (aP2.*pDirPt)() )
        {
            SwRect aRect( aP1, aP2 );
            // OD 18.11.2002 #99672# - use parameter <_pSubsLines> instead of
            // global variable <pSubsLines>.
            _pSubsLines->AddLineRect( aRect, 0, table::BorderLineStyle::SOLID,
                    0, nSubColor );
        }
        aP1 = aP2;
        (aP1.*pDirPt)() += 1;
        aP2 = rP2;
    }
}

static drawinglayer::primitive2d::Primitive2DSequence lcl_CreatePageAreaDelimiterPrimitives(
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DSequence aSeq( 4 );

    basegfx::BColor aLineColor = SwViewOption::GetDocBoundariesColor().getBColor();
    double nLineLength = 200.0; // in Twips

    Point aPoints[] = { rRect.TopLeft(), rRect.TopRight(), rRect.BottomRight(), rRect.BottomLeft() };
    double aXOffDirs[] = { -1.0, 1.0, 1.0, -1.0 };
    double aYOffDirs[] = { -1.0, -1.0, 1.0, 1.0 };

    // Actually loop over the corners to create the two lines
    for ( int i = 0; i < 4; i++ )
    {
        basegfx::B2DVector aHorizVector( aXOffDirs[i], 0.0 );
        basegfx::B2DVector aVertVector( 0.0, aYOffDirs[i] );

        basegfx::B2DPoint aBPoint( aPoints[i].getX(), aPoints[i].getY() );

        basegfx::B2DPolygon aPolygon;
        aPolygon.append( aBPoint + aHorizVector * nLineLength );
        aPolygon.append( aBPoint );
        aPolygon.append( aBPoint + aVertVector * nLineLength );

        drawinglayer::primitive2d::PolygonHairlinePrimitive2D* pLine =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aLineColor );
        aSeq[i] = drawinglayer::primitive2d::Primitive2DReference( pLine );
    }

    return aSeq;
}

static drawinglayer::primitive2d::Primitive2DSequence lcl_CreateRectangleDelimiterPrimitives (
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DSequence aSeq( 1 );
    basegfx::BColor aLineColor = SwViewOption::GetDocBoundariesColor().getBColor();

    basegfx::B2DPolygon aPolygon;
    aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Top() ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Top() ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Right(), rRect.Bottom() ) );
    aPolygon.append( basegfx::B2DPoint( rRect.Left(), rRect.Bottom() ) );
    aPolygon.setClosed( true );

    drawinglayer::primitive2d::PolygonHairlinePrimitive2D* pLine =
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aPolygon, aLineColor );
    aSeq[0] = drawinglayer::primitive2d::Primitive2DReference( pLine );

    return aSeq;
}

static drawinglayer::primitive2d::Primitive2DSequence lcl_CreateColumnAreaDelimiterPrimitives(
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DSequence aSeq( 4 );

    basegfx::BColor aLineColor = SwViewOption::GetDocBoundariesColor().getBColor();
    double nLineLength = 100.0; // in Twips

    Point aPoints[] = { rRect.TopLeft(), rRect.TopRight(), rRect.BottomRight(), rRect.BottomLeft() };
    double aXOffDirs[] = { 1.0, -1.0, -1.0, 1.0 };
    double aYOffDirs[] = { 1.0, 1.0, -1.0, -1.0 };

    // Actually loop over the corners to create the two lines
    for ( int i = 0; i < 4; i++ )
    {
        basegfx::B2DVector aHorizVector( aXOffDirs[i], 0.0 );
        basegfx::B2DVector aVertVector( 0.0, aYOffDirs[i] );

        basegfx::B2DPoint aBPoint( aPoints[i].getX(), aPoints[i].getY() );

        basegfx::B2DPolygon aPolygon;
        aPolygon.append( aBPoint + aHorizVector * nLineLength );
        aPolygon.append( aBPoint );
        aPolygon.append( aBPoint + aVertVector * nLineLength );

        drawinglayer::primitive2d::PolygonHairlinePrimitive2D* pLine =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aLineColor );
        aSeq[i] = drawinglayer::primitive2d::Primitive2DReference( pLine );
    }

    return aSeq;
}

void SwPageFrm::PaintSubsidiaryLines( const SwPageFrm *,
                                        const SwRect & ) const
{
    if ( !pGlobalShell->IsHeaderFooterEdit() )
    {
        const SwFrm* pLay = Lower();
        const SwFrm* pFtnCont = NULL;
        const SwFrm* pPageBody = NULL;
        while ( pLay && !( pFtnCont && pPageBody ) )
        {
            if ( pLay->IsFtnContFrm( ) )
                pFtnCont = pLay;
            if ( pLay->IsBodyFrm() )
                pPageBody = pLay;
            pLay = pLay->GetNext();
        }

        SwRect aArea( pPageBody->Frm() );
        if ( pFtnCont )
            aArea.AddBottom( pFtnCont->Frm().Bottom() - aArea.Bottom() );

        if ( !pGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
            ProcessPrimitives( lcl_CreatePageAreaDelimiterPrimitives( aArea ) );
        else
            ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
    }
}

void SwColumnFrm::PaintSubsidiaryLines( const SwPageFrm *,
                                        const SwRect & ) const
{
    const SwFrm* pLay = Lower();
    const SwFrm* pFtnCont = NULL;
    const SwFrm* pColBody = NULL;
    while ( pLay && !( pFtnCont && pColBody ) )
    {
        if ( pLay->IsFtnContFrm( ) )
            pFtnCont = pLay;
        if ( pLay->IsBodyFrm() )
            pColBody = pLay;
        pLay = pLay->GetNext();
    }

    SwRect aArea( pColBody->Frm() );

    // #i3662# - enlarge top of column body frame's printing area
    // in sections to top of section frame.
    const bool bColInSection =  GetUpper()->IsSctFrm();
    if ( bColInSection )
    {
        if ( IsVertical() )
            aArea.Right( GetUpper()->Frm().Right() );
        else
            aArea.Top( GetUpper()->Frm().Top() );
    }

    if ( pFtnCont )
        aArea.AddBottom( pFtnCont->Frm().Bottom() - aArea.Bottom() );

    ::SwAlignRect( aArea, pGlobalShell );

    if ( !pGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
        ProcessPrimitives( lcl_CreateColumnAreaDelimiterPrimitives( aArea ) );
    else
        ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
}

void SwSectionFrm::PaintSubsidiaryLines( const SwPageFrm * pPage,
                                        const SwRect & rRect ) const
{
    const bool bNoLowerColumn = !Lower() || !Lower()->IsColumnFrm();
    if ( bNoLowerColumn )
    {
        SwLayoutFrm::PaintSubsidiaryLines( pPage, rRect );
    }
}

/** The SwBodyFrm doesn't print any subsidiary line: it's bounds are painted
    either by the parent page or the parent column frame.
  */
void SwBodyFrm::PaintSubsidiaryLines( const SwPageFrm *,
                                        const SwRect & ) const
{
}

void SwHeadFootFrm::PaintSubsidiaryLines( const SwPageFrm *, const SwRect & ) const
{
    if ( pGlobalShell->IsHeaderFooterEdit() )
    {
        SwRect aArea( Prt() );
        aArea.Pos() += Frm().Pos();
        if ( !pGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
            ProcessPrimitives( lcl_CreatePageAreaDelimiterPrimitives( aArea ) );
        else
            ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
    }
}

/** This method is overridden in order to have no subsidiary lines
    around the footnotes.
  */
void SwFtnFrm::PaintSubsidiaryLines( const SwPageFrm *,
                                        const SwRect & ) const
{
}

/** This method is overridden in order to have no subsidiary lines
    around the footnotes containers.
  */
void SwFtnContFrm::PaintSubsidiaryLines( const SwPageFrm *,
                                        const SwRect & ) const
{
}

void SwLayoutFrm::PaintSubsidiaryLines( const SwPageFrm *pPage,
                                        const SwRect &rRect ) const
{
    bool bNewTableModel = false;

    // #i29550#
    if ( IsTabFrm() || IsCellFrm() || IsRowFrm() )
    {
        const SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
            return;

        bNewTableModel = pTabFrm->GetTable()->IsNewModel();
        // in the new table model, we have an early return for all cell-related
        // frames, except from non-covered table cells
        if ( bNewTableModel )
            if ( IsTabFrm() ||
                 IsRowFrm() ||
                 ( IsCellFrm() && IsCoveredCell() ) )
                return;
    }
    // <-- collapsing

    const bool bFlys = pPage->GetSortedObjs() ? true : false;

    const bool bCell = IsCellFrm();
    // use frame area for cells
    // OD 13.02.2003 #i3662# - for section use also frame area
    const bool bUseFrmArea = bCell || IsSctFrm();
    SwRect aOriginal( bUseFrmArea ? Frm() : Prt() );
    if ( !bUseFrmArea )
        aOriginal.Pos() += Frm().Pos();

    ::SwAlignRect( aOriginal, pGlobalShell );

    if ( !aOriginal.IsOver( rRect ) )
        return;

    SwRect aOut( aOriginal );
    aOut._Intersection( rRect );

    const SwTwips nRight = aOut.Right();
    const SwTwips nBottom= aOut.Bottom();

    const Point aRT( nRight, aOut.Top() );
    const Point aRB( nRight, nBottom );
    const Point aLB( aOut.Left(), nBottom );

    sal_uInt8 nSubColor = ( bCell || IsRowFrm() ) ? SUBCOL_TAB :
                     ( IsInSct() ? SUBCOL_SECT :
                     ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );

    // OD 18.11.2002 #99672# - collect body, header, footer, footnote and section
    // sub-lines in <pSpecSubsLine> array.
    const bool bSpecialSublines = IsBodyFrm() || IsHeaderFrm() || IsFooterFrm() ||
                                  IsFtnFrm() || IsSctFrm();
    SwLineRects* pUsedSubsLines = bSpecialSublines ? pSpecSubsLines : pSubsLines;

    // NOTE: for cell frames only left and right (horizontal layout) respectively
    //      top and bottom (vertical layout) lines painted.
    // NOTE2: this does not hold for the new table model!!! We paint the top border
    // of each non-covered table cell.
    const bool bVert = IsVertical() ? true : false;
    if ( bFlys )
    {
        // OD 14.11.2002 #104822# - add control for drawing left and right lines
        if ( !bCell || bNewTableModel || !bVert )
        {
            if ( aOriginal.Left() == aOut.Left() )
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aLB, nSubColor, pUsedSubsLines );
            // OD 14.11.2002 #104821# - in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
                ::lcl_RefreshLine( this, pPage, aRT, aRB, nSubColor, pUsedSubsLines );
        }
        // OD 14.11.2002 #104822# - adjust control for drawing top and bottom lines
        if ( !bCell || bNewTableModel || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
                // OD 14.11.2002 #104821# - in horizontal layout set page/column break at top
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aRT, nSubColor, pUsedSubsLines );
            if ( aOriginal.Bottom() == nBottom )
                ::lcl_RefreshLine( this, pPage, aLB, aRB, nSubColor,
                                   pUsedSubsLines );
        }
    }
    else
    {
        // OD 14.11.2002 #104822# - add control for drawing left and right lines
        if ( !bCell || bNewTableModel || !bVert )
        {
            if ( aOriginal.Left() == aOut.Left() )
            {
                const SwRect aRect( aOut.Pos(), aLB );
                pUsedSubsLines->AddLineRect( aRect, 0,
                        table::BorderLineStyle::SOLID, 0, nSubColor );
            }
            // OD 14.11.2002 #104821# - in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
            {
                const SwRect aRect( aRT, aRB );
                pUsedSubsLines->AddLineRect( aRect, 0,
                        table::BorderLineStyle::SOLID, 0, nSubColor );
            }
        }
        // OD 14.11.2002 #104822# - adjust control for drawing top and bottom lines
        if ( !bCell || bNewTableModel || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
            {
                // OD 14.11.2002 #104821# - in horizontal layout set page/column break at top
                const SwRect aRect( aOut.Pos(), aRT );
                pUsedSubsLines->AddLineRect( aRect, 0,
                        table::BorderLineStyle::SOLID, 0, nSubColor );
            }
            if ( aOriginal.Bottom() == nBottom )
            {
                const SwRect aRect( aLB, aRB );
                pUsedSubsLines->AddLineRect( aRect, 0,
                        table::BorderLineStyle::SOLID, 0, nSubColor );
            }
        }
    }
}

/*************************************************************************
|*
|*  SwPageFrm::RefreshExtraData(), SwLayoutFrm::RefreshExtraData()
|*
|* Description          Refreshes all extra data (line breaks a.s.o) of the
|*                      page. Basically only those objects are considered which
|*                      horizontally overlap the Rect.
|*
|*************************************************************************/

void SwPageFrm::RefreshExtraData( const SwRect &rRect ) const
{
    const SwLineNumberInfo &rInfo = GetFmt()->GetDoc()->GetLineNumberInfo();
    bool bLineInFly = (rInfo.IsPaintLineNumbers() && rInfo.IsCountInFlys())
        || (sal_Int16)SW_MOD()->GetRedlineMarkPos() != text::HoriOrientation::NONE;

    SwRect aRect( rRect );
    ::SwAlignRect( aRect, pGlobalShell );
    if ( aRect.HasArea() )
    {
        SwLayoutFrm::RefreshExtraData( aRect );

        if ( bLineInFly && GetSortedObjs() )
            for ( sal_uInt16 i = 0; i < GetSortedObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                    if ( pFly->Frm().Top() <= aRect.Bottom() &&
                         pFly->Frm().Bottom() >= aRect.Top() )
                        pFly->RefreshExtraData( aRect );
                }
            }
    }
}

void SwLayoutFrm::RefreshExtraData( const SwRect &rRect ) const
{

    const SwLineNumberInfo &rInfo = GetFmt()->GetDoc()->GetLineNumberInfo();
    bool bLineInBody = rInfo.IsPaintLineNumbers(),
             bLineInFly  = bLineInBody && rInfo.IsCountInFlys(),
             bRedLine = (sal_Int16)SW_MOD()->GetRedlineMarkPos()!=text::HoriOrientation::NONE;

    const SwCntntFrm *pCnt = ContainsCntnt();
    while ( pCnt && IsAnLower( pCnt ) )
    {
        if ( pCnt->IsTxtFrm() && ( bRedLine ||
             ( !pCnt->IsInTab() &&
               ((bLineInBody && pCnt->IsInDocBody()) ||
               (bLineInFly  && pCnt->IsInFly())) ) ) &&
             pCnt->Frm().Top() <= rRect.Bottom() &&
             pCnt->Frm().Bottom() >= rRect.Top() )
        {
            ((SwTxtFrm*)pCnt)->PaintExtraData( rRect );
        }
        if ( bLineInFly && pCnt->GetDrawObjs() )
            for ( sal_uInt32 i = 0; i < pCnt->GetDrawObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pCnt->GetDrawObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                    if ( pFly->IsFlyInCntFrm() &&
                         pFly->Frm().Top() <= rRect.Bottom() &&
                         pFly->Frm().Bottom() >= rRect.Top() )
                        pFly->RefreshExtraData( rRect );
                }
        }
        pCnt = pCnt->GetNextCntntFrm();
    }
}

/** SwPageFrm::GetDrawBackgrdColor - for #102450#

    determine the color, that is respectively will be drawn as background
    for the page frame.
    Using existing method SwFrm::GetBackgroundBrush to determine the color
    that is set at the page frame respectively is parent. If none is found
    return the global retouche color

    @author OD

    @return Color
*/
const Color& SwPageFrm::GetDrawBackgrdColor() const
{
    const SvxBrushItem* pBrushItem;
    const XFillStyleItem* pFillStyleItem;
    const XFillGradientItem* pFillGradientItem;
    const Color* pDummyColor;
    SwRect aDummyRect;
    if ( GetBackgroundBrush( pBrushItem, pFillStyleItem, pFillGradientItem, pDummyColor, aDummyRect, true) )
        return pBrushItem->GetColor();
    else
        return aGlobalRetoucheColor;
}

/*************************************************************************
|*
|*    SwPageFrm::GetEmptyPageFont()
|*
|*    create/return font used to paint the "empty page" string
|*
|*************************************************************************/

const Font& SwPageFrm::GetEmptyPageFont()
{
    static Font* pEmptyPgFont = 0;
    if ( 0 == pEmptyPgFont )
    {
        pEmptyPgFont = new Font;
        pEmptyPgFont->SetSize( Size( 0, 80 * 20 )); // == 80 pt
        pEmptyPgFont->SetWeight( WEIGHT_BOLD );
        pEmptyPgFont->SetStyleName( aEmptyStr );
        pEmptyPgFont->SetName(OUString("Helvetica"));
        pEmptyPgFont->SetFamily( FAMILY_SWISS );
        pEmptyPgFont->SetTransparent( sal_True );
        pEmptyPgFont->SetColor( COL_GRAY );
    }

    return *pEmptyPgFont;
}

/*************************************************************************
|*
|*    SwFrm::Retouche
|*
|*    Description       Retouch for a section.
|*      Retouch will only be done, if the Frm is the last one in his chain.
|*      The whole area of the upper which is located below the Frm will be
|*      cleared using PaintBackground.
|*************************************************************************/

void SwFrm::Retouche( const SwPageFrm * pPage, const SwRect &rRect ) const
{
    if ( bFlyMetafile )
        return;

    OSL_ENSURE( GetUpper(), "Retouche try without Upper." );
    OSL_ENSURE( getRootFrm()->GetCurrShell() && pGlobalShell->GetWin(), "Retouche on a printer?" );

    SwRect aRetouche( GetUpper()->PaintArea() );
    aRetouche.Top( Frm().Top() + Frm().Height() );
    aRetouche.Intersection( pGlobalShell->VisArea() );

    if ( aRetouche.HasArea() )
    {
        //Omit the passed Rect. To do this, we unfortunately need a region to
        //cut out.
        SwRegionRects aRegion( aRetouche );
        aRegion -= rRect;
        ViewShell *pSh = getRootFrm()->GetCurrShell();

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( 0, 0, 0, *pSh->GetOut() );

        for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
        {
            SwRect &rRetouche = aRegion[i];

            GetUpper()->PaintBaBo( rRetouche, pPage, sal_True );

            //Hell and Heaven need to be refreshed too.
            //To avoid recursion my retouch flag needs to be reset first!
            ResetRetouche();
            SwRect aRetouchePart( rRetouche );
            if ( aRetouchePart.HasArea() )
            {
                const Color aPageBackgrdColor = pPage->GetDrawBackgrdColor();
                const IDocumentDrawModelAccess* pIDDMA = pSh->getIDocumentDrawModelAccess();
                // --> OD #i76669#
                SwViewObjectContactRedirector aSwRedirector( *pSh );
                // <--

                pSh->Imp()->PaintLayer( pIDDMA->GetHellId(), 0,
                                        aRetouchePart, &aPageBackgrdColor,
                                        (pPage->IsRightToLeft() ? true : false),
                                        &aSwRedirector );
                pSh->Imp()->PaintLayer( pIDDMA->GetHeavenId(), 0,
                                        aRetouchePart, &aPageBackgrdColor,
                                        (pPage->IsRightToLeft() ? true : false),
                                        &aSwRedirector );
            }

            SetRetouche();

            //Because we leave all paint areas, we need to refresh the
            //subsidiary lines.
            pPage->RefreshSubsidiary( aRetouchePart );
        }
    }
    if ( ViewShell::IsLstEndAction() )
        ResetRetouche();
}

/** SwFrm::GetBackgroundBrush

    @descr
    determine the background brush for the frame:
    the background brush is taken from it-self or from its parent (anchor/upper).
    Normally, the background brush is taken, which has no transparent color or
    which has a background graphic. But there are some special cases:
    (1) No background brush is taken from a page frame, if view option "IsPageBack"
        isn't set.
    (2) Background brush from a index section is taken under special conditions.
        In this case parameter <rpCol> is set to the index shading color.
    (3) New (OD 20.08.2002) - Background brush is taken, if on background drawing
        of the frame transparency is considered and its color is not "no fill"/"auto fill"
    ---- old description in german:
    Description         Returns the Backgroundbrush for the area of the Frm.
        The Brush is defined by the Frm or by an upper, the first Brush is
        used. If no Brush is defined for a Frm, sal_False is returned.

    @param rpBrush
    output parameter - constant reference pointer the found background brush

    @param rpFillStyle
    output parameter - constant reference pointer the found background fill style

    @param rpFillGradient
    output parameter - constant reference pointer the found background fill gradient

    @param rpCol
    output parameter - constant reference pointer to the color of the index shading
    set under special conditions, if background brush is taken from an index section.

    @param rOrigRect
    in-/output parameter - reference to the retangle the background brush is
    considered for - adjusted to the frame, from which the background brush is
    taken.

    @parem bLowerMode
    input parameter - boolean indicating, if background brush should *not* be
    taken from parent.

    @return true, if a background brush for the frame is found
*/
sal_Bool SwFrm::GetBackgroundBrush( const SvxBrushItem* & rpBrush,
                                    const XFillStyleItem* & rpFillStyle,
                                    const XFillGradientItem* & rpFillGradient,
                                const Color*& rpCol,
                                SwRect &rOrigRect,
                                sal_Bool bLowerMode ) const
{
    const SwFrm *pFrm = this;
    ViewShell *pSh = getRootFrm()->GetCurrShell();
    const SwViewOption *pOpt = pSh->GetViewOptions();
    rpBrush = 0;
    rpFillStyle = 0;
    rpFillGradient = 0;
    rpCol = NULL;
    do
    {   if ( pFrm->IsPageFrm() && !pOpt->IsPageBack() )
            return sal_False;

        const SvxBrushItem &rBack = pFrm->GetAttrSet()->GetBackground();
        const XFillStyleItem &rFillStyle = pFrm->GetAttrSet()->GetFillStyle();
        const XFillGradientItem &rFillGradient = pFrm->GetAttrSet()->GetFillGradient();
        if( pFrm->IsSctFrm() )
        {
            const SwSection* pSection = ((SwSectionFrm*)pFrm)->GetSection();
            /// OD 20.08.2002 #99657# #GetTransChg#
            ///     Note: If frame <pFrm> is a section of the index and
            ///         it its background color is "no fill"/"auto fill" and
            ///         it has no background graphic and
            ///         we are not in the page preview and
            ///         we are not in read-only mode and
            ///         option "index shadings" is set and
            ///         the output is not the printer
            ///         then set <rpCol> to the color of the index shading
            if( pSection && (   TOX_HEADER_SECTION == pSection->GetType() ||
                                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                (rBack.GetColor() == COL_TRANSPARENT) &&
                ///rBack.GetColor().GetTransparency() &&
                rBack.GetGraphicPos() == GPOS_NONE &&
                !pOpt->IsPagePreview() &&
                !pOpt->IsReadonly() &&
                // #114856# Formular view
                !pOpt->IsFormView() &&
                SwViewOption::IsIndexShadings() &&
                !pOpt->IsPDFExport() &&
                pSh->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
            {
                rpCol = &SwViewOption::GetIndexShadingsColor();
            }
        }

        /// OD 20.08.2002 #99657#
        ///     determine, if background draw of frame <pFrm> considers transparency
        ///     --> Status Quo: background transparency have to be
        ///                     considered for fly frames
        const sal_Bool bConsiderBackgroundTransparency = pFrm->IsFlyFrm();
        /// OD 20.08.2002 #99657#
        ///     add condition:
        ///     If <bConsiderBackgroundTransparency> is set - see above -,
        ///     return brush of frame <pFrm>, if its color is *not* "no fill"/"auto fill"
        if ( !rBack.GetColor().GetTransparency() ||
             rBack.GetGraphicPos() != GPOS_NONE ||
             rpCol ||
             (bConsiderBackgroundTransparency && (rBack.GetColor() != COL_TRANSPARENT || rFillStyle.GetValue() == XFILL_GRADIENT))
           )
        {
            rpBrush = &rBack;
            rpFillStyle = &rFillStyle;
            rpFillGradient = &rFillGradient;
            if ( pFrm->IsPageFrm() &&
                 pSh->GetViewOptions()->getBrowseMode() )
                rOrigRect = pFrm->Frm();
            else
            {
                if ( pFrm->Frm().SSize() != pFrm->Prt().SSize() )
                {
                    SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    ::lcl_CalcBorderRect( rOrigRect, pFrm, rAttrs, false );
                }
                else
                {
                    rOrigRect = pFrm->Prt();
                    rOrigRect += pFrm->Frm().Pos();
                }
            }
            return sal_True;
        }

        if ( bLowerMode )
            /// Do not try to get background brush from parent (anchor/upper)
            return sal_False;

        /// get parent frame - anchor or upper - for next loop
        if ( pFrm->IsFlyFrm() )
            /// OD 20.08.2002 - use "static_cast" instead of "old C-cast"
            pFrm = (static_cast<const SwFlyFrm*>(pFrm))->GetAnchorFrm();
            ///pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
        else
            pFrm = pFrm->GetUpper();

    } while ( pFrm );

    return sal_False;
}

/*************************************************************************
|*
|*  SwFrmFmt::GetGraphic()
|*
|*************************************************************************/

void SetOutDevAndWin( ViewShell *pSh, OutputDevice *pO,
                      Window *pW, sal_uInt16 nZoom )
{
    pSh->mpOut = pO;
    pSh->mpWin = pW;
    pSh->mpOpt->SetZoom( nZoom );
}

Graphic SwFrmFmt::MakeGraphic( ImageMap* )
{
    return Graphic();
}

Graphic SwFlyFrmFmt::MakeGraphic( ImageMap* pMap )
{
    Graphic aRet;
    //search any Fly!
    SwIterator<SwFrm,SwFmt> aIter( *this );
    SwFrm *pFirst = aIter.First();
    ViewShell *pSh;
    if ( pFirst && 0 != ( pSh = pFirst->getRootFrm()->GetCurrShell()) )
    {
        ViewShell *pOldGlobal = pGlobalShell;
        pGlobalShell = pSh;

        bool bNoteURL = pMap &&
            SFX_ITEM_SET != GetAttrSet().GetItemState( RES_URL, sal_True );
        if( bNoteURL )
        {
            OSL_ENSURE( !pNoteURL, "MakeGraphic: pNoteURL already used? " );
            pNoteURL = new SwNoteURL;
        }
        SwFlyFrm *pFly = (SwFlyFrm*)pFirst;

        OutputDevice *pOld = pSh->GetOut();
        VirtualDevice aDev( *pOld );
        aDev.EnableOutput( sal_False );

        GDIMetaFile aMet;
        MapMode aMap( pOld->GetMapMode().GetMapUnit() );
        aDev.SetMapMode( aMap );
        aMet.SetPrefMapMode( aMap );

        ::SwCalcPixStatics( pSh->GetOut() );
        aMet.SetPrefSize( pFly->Frm().SSize() );

        aMet.Record( &aDev );
        aDev.SetLineColor();
        aDev.SetFillColor();
        aDev.SetFont( pOld->GetFont() );

        //Enlarge the rectangle if needed, so the border is painted too.
        SwRect aOut( pFly->Frm() );
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFly );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( rAttrs.CalcRightLine() )
            aOut.SSize().Width() += 2*nPixelSzW;
        if ( rAttrs.CalcBottomLine() )
            aOut.SSize().Height()+= 2*nPixelSzH;

        // #i92711# start Pre/PostPaint encapsulation before pOut is changed to the buffering VDev
        const Region aRepaintRegion(aOut.SVRect());
        pSh->DLPrePaint2(aRepaintRegion);

        Window *pWin = pSh->GetWin();
        sal_uInt16 nZoom = pSh->GetViewOptions()->GetZoom();
        ::SetOutDevAndWin( pSh, &aDev, 0, 100 );
        bFlyMetafile = sal_True;
        pFlyMetafileOut = pWin;

        SwViewImp *pImp = pSh->Imp();
        pFlyOnlyDraw = pFly;
        pLines = new SwLineRects;

        // OD 09.12.2002 #103045# - determine page, fly frame is on
        const SwPageFrm* pFlyPage = pFly->FindPageFrm();
        const Color aPageBackgrdColor = pFlyPage->GetDrawBackgrdColor();
        const IDocumentDrawModelAccess* pIDDMA = pSh->getIDocumentDrawModelAccess();
        // --> OD #i76669#
        SwViewObjectContactRedirector aSwRedirector( *pSh );
        // <--
        pImp->PaintLayer( pIDDMA->GetHellId(), 0, aOut, &aPageBackgrdColor,
                          (pFlyPage->IsRightToLeft() ? true : false),
                          &aSwRedirector );
        pLines->PaintLines( &aDev );
        if ( pFly->IsFlyInCntFrm() )
            pFly->Paint( aOut );
        pLines->PaintLines( &aDev );
        /// OD 30.08.2002 #102450# - add 3rd parameter
        pImp->PaintLayer( pIDDMA->GetHeavenId(), 0, aOut, &aPageBackgrdColor,
                          (pFlyPage->IsRightToLeft() ? true : false),
                          &aSwRedirector );
        pLines->PaintLines( &aDev );
        DELETEZ( pLines );
        pFlyOnlyDraw = 0;

        pFlyMetafileOut = 0;
        bFlyMetafile = sal_False;
        ::SetOutDevAndWin( pSh, pOld, pWin, nZoom );

        // #i92711# end Pre/PostPaint encapsulation when pOut is back and content is painted
           pSh->DLPostPaint2(true);

        aMet.Stop();
        aMet.Move( -pFly->Frm().Left(), -pFly->Frm().Top() );
        aRet = Graphic( aMet );

        if( bNoteURL )
        {
            OSL_ENSURE( pNoteURL, "MakeGraphic: Good Bye, NoteURL." );
            pNoteURL->FillImageMap( pMap, pFly->Frm().Pos(), aMap );
            delete pNoteURL;
            pNoteURL = NULL;
        }
        pGlobalShell = pOldGlobal;
    }
    return aRet;
}

Graphic SwDrawFrmFmt::MakeGraphic( ImageMap* )
{
    Graphic aRet;
    SdrModel *pMod = getIDocumentDrawModelAccess()->GetDrawModel();
    if ( pMod )
    {
        SdrObject *pObj = FindSdrObject();
        SdrView *pView = new SdrView( pMod );
        SdrPageView *pPgView = pView->ShowSdrPage(pView->GetModel()->GetPage(0));
        pView->MarkObj( pObj, pPgView );
        aRet = pView->GetMarkedObjBitmapEx();
        pView->HideSdrPage();
        delete pView;
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
