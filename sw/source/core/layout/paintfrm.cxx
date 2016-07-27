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
#include <sfx2/docfile.hxx>
#include <sfx2/progress.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <svx/framelink.hxx>
#include <drawdoc.hxx>
#include <tgrditem.hxx>
#include <calbck.hxx>
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
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDeviceAccess.hxx>

#include <ndole.hxx>
#include <PostItMgr.hxx>
#include <FrameControlsManager.hxx>
#include <vcl/settings.hxx>

#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

#include <svtools/borderhelper.hxx>

#include "pagefrm.hrc"
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <svx/unoapi.hxx>
#include <comphelper/sequence.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolortools.hxx>

#include <vector>
#include <algorithm>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <paintfrm.hxx>

#define COL_NOTES_SIDEPANE                  RGB_COLORDATA(230,230,230)
#define COL_NOTES_SIDEPANE_BORDER           RGB_COLORDATA(200,200,200)
#define COL_NOTES_SIDEPANE_SCROLLAREA       RGB_COLORDATA(230,230,220)

using namespace ::editeng;
using namespace ::com::sun::star;
using ::drawinglayer::primitive2d::BorderLinePrimitive2D;
using ::std::pair;
using ::std::make_pair;

struct SwPaintProperties;

//other subsidiary lines enabled?
#define IS_SUBS (!gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() && \
                 !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() && \
                 !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&\
                 !gProp.pSGlobalShell->GetViewOptions()->IsWhitespaceHidden() &&\
                 SwViewOption::IsDocBoundaries())
//subsidiary lines for sections
#define IS_SUBS_SECTION (!gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() && \
                         !gProp.pSGlobalShell->GetViewOptions()->IsReadonly()&&\
                         !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&\
                          SwViewOption::IsSectionBoundaries())
#define IS_SUBS_FLYS (!gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() && \
                      !gProp.pSGlobalShell->GetViewOptions()->IsReadonly()&&\
                      !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&\
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
    const SwTabFrame *pTab;
          sal_uInt8     nSubColor;  //colorize subsidiary lines
          bool          bPainted;   //already painted?
          sal_uInt8     nLock;      //To distinguish the line and the hell layer.
public:
    SwLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderStyle nStyle,
                const SwTabFrame *pT , const sal_uInt8 nSCol );

    const Color&         GetColor() const { return aColor;}
    SvxBorderStyle       GetStyle() const { return nStyle; }
    const SwTabFrame      *GetTab()   const { return pTab;  }
    void  SetPainted()                    { bPainted = true; }
    void  Lock( bool bLock )              { if ( bLock )
                                                ++nLock;
                                            else if ( nLock )
                                                --nLock;
                                          }
    bool  IsPainted()               const { return bPainted; }
    bool  IsLocked()                const { return nLock != 0;  }
    sal_uInt8  GetSubColor()                const { return nSubColor;}

    bool MakeUnion( const SwRect &rRect, SwPaintProperties &properties );
};

#ifdef IOS
static void dummy_function()
{
    pid_t pid = getpid();
    (void) pid;
}
#endif

class SwLineRects
{
public:
    std::vector< SwLineRect > aLineRects;
    typedef std::vector< SwLineRect >::const_iterator const_iterator;
    typedef std::vector< SwLineRect >::iterator iterator;
    typedef std::vector< SwLineRect >::reverse_iterator reverse_iterator;
    typedef std::vector< SwLineRect >::size_type size_type;
    size_t nLastCount;  //avoid unnecessary cycles in PaintLines
    SwLineRects() : nLastCount( 0 )
    {
#ifdef IOS
        // Work around what is either a compiler bug in Xcode 5.1.1,
        // or some unknown problem in this file. If I ifdef out this
        // call, I get a crash in SwSubsRects::PaintSubsidiary: the
        // address of the rLi reference variable is claimed to be
        // 0x4000000!
        dummy_function();
#endif
    }
    void AddLineRect( const SwRect& rRect,  const Color *pColor, const SvxBorderStyle nStyle,
                      const SwTabFrame *pTab, const sal_uInt8 nSCol, SwPaintProperties &properties );
    void ConnectEdges( OutputDevice *pOut, SwPaintProperties &properties );
    void PaintLines  ( OutputDevice *pOut, SwPaintProperties &properties );
    void LockLines( bool bLock );

    //Limit lines to 100
    bool isFull() const { return aLineRects.size()>100; }
};

class SwSubsRects : public SwLineRects
{
    void RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects, SwPaintProperties &properties );
public:
    void PaintSubsidiary( OutputDevice *pOut, const SwLineRects *pRects, SwPaintProperties &properties );
};

class BorderLines
{
    drawinglayer::primitive2d::Primitive2DContainer m_Lines;
public:
    void AddBorderLine(css::uno::Reference<BorderLinePrimitive2D> const& xLine, SwPaintProperties& properties);
    drawinglayer::primitive2d::Primitive2DContainer GetBorderLines_Clear()
    {
        drawinglayer::primitive2d::Primitive2DContainer lines;
        lines.swap(m_Lines);
        return lines;
    }
};

// Default zoom factor
const static double aMinDistScale = 0.73;
const static double aEdgeScale = 0.5;

//To optimize the expensive RetouchColor determination
Color aGlobalRetoucheColor;

namespace sw
{
Color* GetActiveRetoucheColor()
{
    return &aGlobalRetoucheColor;
}
}

/**
 * Container for static properties
 */
struct SwPaintProperties {
    // Only repaint the Fly content as well as the background of the Fly content if
    // a metafile is taken of the Fly.
    bool                bSFlyMetafile;
    VclPtr<OutputDevice> pSFlyMetafileOut;
    SwViewShell        *pSGlobalShell;

    // Retouch for transparent Flys is done by the background of the Flys.
    // The Fly itself should certainly not be spared out. See PaintBackground and
    // lcl_SubtractFlys()
    SwFlyFrame           *pSRetoucheFly;
    SwFlyFrame           *pSRetoucheFly2;
    SwFlyFrame           *pSFlyOnlyDraw;

    // The borders will be collected in pSLines during the Paint and later
    // possibly merge them.
    // The help lines will be collected and merged in gProp.pSSubsLines. These will
    // be compared with pSLines before the work in order to avoid help lines
    // to hide borders.
    BorderLines        *pBLines;
    SwLineRects        *pSLines;
    SwSubsRects        *pSSubsLines;

    // global variable for sub-lines of body, header, footer, section and footnote frames.
    SwSubsRects        *pSSpecSubsLines;
    SfxProgress        *pSProgress;

    // Sizes of a pixel and the corresponding halves. Will be reset when
    // entering SwRootFrame::Paint
    long                nSPixelSzW;
    long                nSPixelSzH;
    long                nSHalfPixelSzW;
    long                nSHalfPixelSzH;
    long                nSMinDistPixelW;
    long                nSMinDistPixelH;

    Color               aSGlobalRetoucheColor;

    // Current zoom factor
    double              aSScaleX;
    double              aSScaleY;

    SwPaintProperties()
      : bSFlyMetafile(false)
      , pSFlyMetafileOut(nullptr)
      , pSGlobalShell(nullptr)
      , pSRetoucheFly(nullptr)
      , pSRetoucheFly2(nullptr)
      , pSFlyOnlyDraw(nullptr)
      , pBLines(nullptr)
      , pSLines(nullptr)
      , pSSubsLines(nullptr)
      , pSSpecSubsLines(nullptr)
      , pSProgress(nullptr)
      , nSPixelSzW(0)
      , nSPixelSzH(0)
      , nSHalfPixelSzW(0)
      , nSHalfPixelSzH(0)
      , nSMinDistPixelW(0)
      , nSMinDistPixelH(0)
      , aSScaleX(1)
      , aSScaleY(1)
    {
    }

};

static SwPaintProperties gProp;

namespace {

bool isTableBoundariesEnabled()
{
    if (!gProp.pSGlobalShell->GetViewOptions()->IsTable())
        return false;

    if (gProp.pSGlobalShell->GetViewOptions()->IsPagePreview())
        return false;

    if (gProp.pSGlobalShell->GetViewOptions()->IsReadonly())
        return false;

    if (gProp.pSGlobalShell->GetViewOptions()->IsFormView())
        return false;

    return SwViewOption::IsTableBoundaries();
}

}

/**
 * Set borders alignment statics
 * Adjustment for 'small' twip-to-pixel relations:
 * For 'small' twip-to-pixel relations (less then 2:1)
 * values of <gProp.nSHalfPixelSzW> and <gProp.nSHalfPixelSzH> are set to ZERO
 */
void SwCalcPixStatics( vcl::RenderContext *pOut )
{
    // determine 'small' twip-to-pixel relation
    bool bSmallTwipToPxRelW = false;
    bool bSmallTwipToPxRelH = false;
    {
        Size aCheckTwipToPxRelSz( pOut->PixelToLogic( Size( 100, 100 )) );
        if ( (aCheckTwipToPxRelSz.Width()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelW = true;
        }
        if ( (aCheckTwipToPxRelSz.Height()/100.0) < 2.0 )
        {
            bSmallTwipToPxRelH = true;
        }
    }

    Size aSz( pOut->PixelToLogic( Size( 1,1 )) );

    gProp.nSPixelSzW = aSz.Width();
    if( !gProp.nSPixelSzW )
        gProp.nSPixelSzW = 1;
    gProp.nSPixelSzH = aSz.Height();
    if( !gProp.nSPixelSzH )
        gProp.nSPixelSzH = 1;

    // consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelW )
    {
        gProp.nSHalfPixelSzW = gProp.nSPixelSzW / 2 + 1;
    }
    else
    {
        gProp.nSHalfPixelSzW = 0;
    }
    // consider 'small' twip-to-pixel relations
    if ( !bSmallTwipToPxRelH )
    {
        gProp.nSHalfPixelSzH = gProp.nSPixelSzH / 2 + 1;
    }
    else
    {
        gProp.nSHalfPixelSzH = 0;
    }

    gProp.nSMinDistPixelW = gProp.nSPixelSzW * 2 + 1;
    gProp.nSMinDistPixelH = gProp.nSPixelSzH * 2 + 1;

    const MapMode &rMap = pOut->GetMapMode();
    gProp.aSScaleX = rMap.GetScaleX();
    gProp.aSScaleY = rMap.GetScaleY();
}

/**
 * To be able to save the statics so the paint is more or less reentrant
 */
class SwSavePaintStatics : public SwPaintProperties
{
public:
    SwSavePaintStatics();
    ~SwSavePaintStatics();
};

SwSavePaintStatics::SwSavePaintStatics()
{
    // Saving globales
    bSFlyMetafile = gProp.bSFlyMetafile;
    pSGlobalShell = gProp.pSGlobalShell;
    pSFlyMetafileOut = gProp.pSFlyMetafileOut;
    pSRetoucheFly = gProp.pSRetoucheFly;
    pSRetoucheFly2 = gProp.pSRetoucheFly2;
    pSFlyOnlyDraw = gProp.pSFlyOnlyDraw;
    pBLines = gProp.pBLines;
    pSLines = gProp.pSLines;
    pSSubsLines = gProp.pSSubsLines;
    pSSpecSubsLines = gProp.pSSpecSubsLines;
    pSProgress = gProp.pSProgress;
    nSPixelSzW = gProp.nSPixelSzW;
    nSPixelSzH = gProp.nSPixelSzH;
    nSHalfPixelSzW = gProp.nSHalfPixelSzW;
    nSHalfPixelSzH = gProp.nSHalfPixelSzH;
    nSMinDistPixelW = gProp.nSMinDistPixelW;
    nSMinDistPixelH = gProp.nSMinDistPixelH ;
    aSGlobalRetoucheColor = aGlobalRetoucheColor;
    aSScaleX = gProp.aSScaleX;
    aSScaleY = gProp.aSScaleY;

    // Restoring globales to default
    gProp.bSFlyMetafile = false;
    gProp.pSFlyMetafileOut = nullptr;
    gProp.pSRetoucheFly  = nullptr;
    gProp.pSRetoucheFly2 = nullptr;
    gProp.nSPixelSzW = gProp.nSPixelSzH =
    gProp.nSHalfPixelSzW = gProp.nSHalfPixelSzH =
    gProp.nSMinDistPixelW = gProp.nSMinDistPixelH = 0;
    gProp.aSScaleX = gProp.aSScaleY = 1.0;
    gProp.pBLines = nullptr;
    gProp.pSLines = nullptr;
    gProp.pSSubsLines = nullptr;
    gProp.pSSpecSubsLines = nullptr;
    gProp.pSProgress = nullptr;
}

SwSavePaintStatics::~SwSavePaintStatics()
{
    // Restoring globales to saved one
    gProp.pSGlobalShell       = pSGlobalShell;
    gProp.bSFlyMetafile       = bSFlyMetafile;
    gProp.pSFlyMetafileOut    = pSFlyMetafileOut;
    gProp.pSRetoucheFly       = pSRetoucheFly;
    gProp.pSRetoucheFly2      = pSRetoucheFly2;
    gProp.pSFlyOnlyDraw       = pSFlyOnlyDraw;
    gProp.pBLines             = pBLines;
    gProp.pSLines             = pSLines;
    gProp.pSSubsLines         = pSSubsLines;
    gProp.pSSpecSubsLines     = pSSpecSubsLines;
    gProp.pSProgress          = pSProgress;
    gProp.nSPixelSzW          = nSPixelSzW;
    gProp.nSPixelSzH          = nSPixelSzH;
    gProp.nSHalfPixelSzW      = nSHalfPixelSzW;
    gProp.nSHalfPixelSzH      = nSHalfPixelSzH;
    gProp.nSMinDistPixelW     = nSMinDistPixelW;
    gProp.nSMinDistPixelH     = nSMinDistPixelH;
    aGlobalRetoucheColor      = aSGlobalRetoucheColor;
    gProp.aSScaleX            = aSScaleX;
    gProp.aSScaleY            = aSScaleY;
}

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
    pair<double, double> const& mergeA,
    pair<double, double> const& mergeB,
    SwPaintProperties& properties)
{
    double const fMergeGap(properties.nSPixelSzW + properties.nSHalfPixelSzW); // NOT static!
    // A is above/before B
    if( mergeA.second <= mergeB.first &&
        mergeA.second + fMergeGap >= mergeB.first )
    {
        return 1;
    }
    // B is above/before A
    else if( mergeB.second <= mergeA.first &&
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
static css::uno::Reference<BorderLinePrimitive2D>
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
static css::uno::Reference<BorderLinePrimitive2D>
lcl_TryMergeBorderLine(BorderLinePrimitive2D const& rThis,
                       BorderLinePrimitive2D const& rOther,
                       SwPaintProperties& properties)
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
        &&  (rtl::math::approxEqual(rThis.getLeftWidth(),  rOther.getLeftWidth()))
        &&  (rtl::math::approxEqual(rThis.getDistance(),   rOther.getDistance()))
        &&  (rtl::math::approxEqual(rThis.getRightWidth(), rOther.getRightWidth()))
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
            if (rtl::math::approxEqual(rThis.getStart().getX(), rOther.getStart().getX()))
            {
                assert(rtl::math::approxEqual(rThis.getEnd().getX(), rOther.getEnd().getX()));
                nRet = lcl_TryMergeLines(
                    make_pair(rThis.getStart().getY(), rThis.getEnd().getY()),
                    make_pair(rOther.getStart().getY(),rOther.getEnd().getY()),
                    properties);
            }
        }
        else // horizontal line
        {
            if (rtl::math::approxEqual(rThis.getStart().getY(), rOther.getStart().getY()))
            {
                assert(rtl::math::approxEqual(rThis.getEnd().getY(), rOther.getEnd().getY()));
                nRet = lcl_TryMergeLines(
                    make_pair(rThis.getStart().getX(), rThis.getEnd().getX()),
                    make_pair(rOther.getStart().getX(),rOther.getEnd().getX()),
                    properties);
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
    return nullptr;
}

void BorderLines::AddBorderLine(
        css::uno::Reference<BorderLinePrimitive2D> const& xLine, SwPaintProperties& properties)
{
    for (drawinglayer::primitive2d::Primitive2DContainer::reverse_iterator it = m_Lines.rbegin(); it != m_Lines.rend();
         ++it)
    {
        css::uno::Reference<BorderLinePrimitive2D> const xMerged(
            lcl_TryMergeBorderLine(*static_cast<BorderLinePrimitive2D*>((*it).get()), *xLine.get(), properties));
        if (xMerged.is())
        {
            *it = xMerged; // replace existing line with merged
            return;
        }
    }
    m_Lines.push_back(xLine);
}

SwLineRect::SwLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderStyle nStyl,
                        const SwTabFrame *pT, const sal_uInt8 nSCol ) :
    SwRect( rRect ),
    nStyle( nStyl ),
    pTab( pT ),
    nSubColor( nSCol ),
    bPainted( false ),
    nLock( 0 )
{
    if ( pCol != nullptr )
        aColor = *pCol;
}

bool SwLineRect::MakeUnion( const SwRect &rRect, SwPaintProperties& properties)
{
    // It has already been tested outside, whether the rectangles have
    // the same orientation (horizontal or vertical), color, etc.
    if ( Height() > Width() ) //Vertical line
    {
        if ( Left()  == rRect.Left() && Width() == rRect.Width() )
        {
            // Merge when there is no gap between the lines
            const long nAdd = properties.nSPixelSzW + properties.nSHalfPixelSzW;
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
            const long nAdd = properties.nSPixelSzW + properties.nSHalfPixelSzW;
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
                               const SwTabFrame *pTab, const sal_uInt8 nSCol, SwPaintProperties& properties )
{
    // Loop backwards because lines which can be combined, can usually be painted
    // in the same context
    for (reverse_iterator it = aLineRects.rbegin(); it != aLineRects.rend();
         ++it)
    {
        SwLineRect &rLRect = (*it);
        // Test for the orientation, color, table
        if ( rLRect.GetTab() == pTab &&
             !rLRect.IsPainted() && rLRect.GetSubColor() == nSCol &&
             (rLRect.Height() > rLRect.Width()) == (rRect.Height() > rRect.Width()) &&
             (pCol && rLRect.GetColor() == *pCol) )
        {
            if ( rLRect.MakeUnion( rRect, properties ) )
                return;
        }
    }
    aLineRects.push_back( SwLineRect( rRect, pCol, nStyle, pTab, nSCol ) );
}

void SwLineRects::ConnectEdges( OutputDevice *pOut, SwPaintProperties& properties )
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        // I'm not doing anything for a too small zoom
        if ( properties.aSScaleX < aEdgeScale || properties.aSScaleY < aEdgeScale )
            return;
    }

    static const long nAdd = 20;

    std::vector<SwLineRect*> aCheck;

    for (size_t i = 0; i < aLineRects.size(); ++i)
    {
        SwLineRect &rL1 = aLineRects[i];
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
        for (iterator it2 = aLineRects.begin(); it2 != aLineRects.end(); ++it2)
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
        for ( size_t k = 0; !bRemove && k < aCheck.size(); ++k )
        {
            SwLineRect &rR1 = *aCheck[k];

            for ( size_t k2 = k+1; !bRemove && k2 < aCheck.size(); ++k2 )
            {
                SwLineRect &rR2 = *aCheck[k2];
                if ( bVert )
                {
                    SwLineRect *pLA = nullptr;
                    SwLineRect *pLB = nullptr;
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
                            aLineRects.push_back( SwLineRect( aIns, &rL1.GetColor(),
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
                            rL1.Top( pLB->Top() ); // extend i1 on the top
                        else
                            bRemove = true; //stopping, remove i1
                    }
                }
                else
                {
                    SwLineRect *pLA = nullptr;
                    SwLineRect *pLB = nullptr;
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
                            aLineRects.push_back( SwLineRect( aIns, &rL1.GetColor(),
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
            aLineRects.erase(aLineRects.begin() + i);
            --i;
        }
    }
}

void SwSubsRects::RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects, SwPaintProperties& properties )
{
    // All help lines that are covered by any border will be removed or split
    for (size_t i = 0; i < aLineRects.size(); ++i)
    {
        // get a copy instead of a reference, because an <insert> may destroy
        // the object due to a necessary array resize.
        const SwLineRect aSubsLineRect = SwLineRect(aLineRects[i]);

        // add condition <aSubsLineRect.IsLocked()> in order to consider only
        // border lines, which are *not* locked.
        if ( aSubsLineRect.IsPainted() ||
             aSubsLineRect.IsLocked() )
            continue;

        const bool bVerticalSubs = aSubsLineRect.Height() > aSubsLineRect.Width();
        SwRect aSubsRect( aSubsLineRect );
        if ( bVerticalSubs )
        {
            aSubsRect.Left  ( aSubsRect.Left()  - (properties.nSPixelSzW+properties.nSHalfPixelSzW) );
            aSubsRect.Right ( aSubsRect.Right() + (properties.nSPixelSzW+properties.nSHalfPixelSzW) );
        }
        else
        {
            aSubsRect.Top   ( aSubsRect.Top()    - (properties.nSPixelSzH+properties.nSHalfPixelSzH) );
            aSubsRect.Bottom( aSubsRect.Bottom() + (properties.nSPixelSzH+properties.nSHalfPixelSzH) );
        }
        for (const_iterator itK = rRects.aLineRects.begin(); itK != rRects.aLineRects.end(); ++itK)
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
                        long nTmp = rLine.Top()-(properties.nSPixelSzH+1);
                        if ( aSubsLineRect.Top() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Bottom( nTmp );
                            aLineRects.push_back( SwLineRect( aNewSubsRect, nullptr, aSubsLineRect.GetStyle(), nullptr,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        nTmp = rLine.Bottom()+properties.nSPixelSzH+1;
                        if ( aSubsLineRect.Bottom() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Top( nTmp );
                            aLineRects.push_back( SwLineRect( aNewSubsRect, nullptr, aSubsLineRect.GetStyle(), nullptr,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        aLineRects.erase(aLineRects.begin() + i);
                        --i;
                        break;
                    }
                }
                else // Horizontal
                {
                    if ( aSubsRect.Top() <= rLine.Bottom() &&
                         aSubsRect.Bottom() >= rLine.Top() )
                    {
                        long nTmp = rLine.Left()-(properties.nSPixelSzW+1);
                        if ( aSubsLineRect.Left() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Right( nTmp );
                            aLineRects.push_back( SwLineRect( aNewSubsRect, nullptr, aSubsLineRect.GetStyle(), nullptr,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        nTmp = rLine.Right()+properties.nSPixelSzW+1;
                        if ( aSubsLineRect.Right() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Left( nTmp );
                            aLineRects.push_back(  SwLineRect( aNewSubsRect, nullptr, aSubsLineRect.GetStyle(), nullptr,
                                                aSubsLineRect.GetSubColor() ) );
                        }
                        aLineRects.erase(aLineRects.begin() + i);
                        --i;
                        break;
                    }
                }
            }
        }
    }
}

void SwLineRects::LockLines( bool bLock )
{
    for (iterator it = aLineRects.begin(); it != aLineRects.end(); ++it)
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

void SwLineRects::PaintLines( OutputDevice *pOut, SwPaintProperties &properties )
{
    // Paint the borders. Sadly two passes are needed.
    // Once for the inside and once for the outside edges of tables
    if ( aLineRects.size() != nLastCount )
    {
        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pOut );

        pOut->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
        pOut->SetFillColor();
        pOut->SetLineColor();
        ConnectEdges( pOut, properties );
        const Color *pLast = nullptr;

        bool bPaint2nd = false;
        size_t nMinCount = aLineRects.size();

        for ( size_t i = 0; i < aLineRects.size(); ++i )
        {
            SwLineRect &rLRect = aLineRects[i];

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
                            nTLeft  = rLRect.GetTab()->Frame().Left() + rLRect.GetTab()->Prt().Left(),
                            nTRight = rLRect.GetTab()->Frame().Left() + rLRect.GetTab()->Prt().Right();
                    if ( (nTLeft >= nLLeft && nTLeft <= nLRight) ||
                         (nTRight>= nLLeft && nTRight<= nLRight) )
                        bPaint = false;
                }
                else
                {
                    // Horizontal edge, overlapping with the table edge?
                    SwTwips nLTop    = rLRect.Top()    - 30,
                            nLBottom = rLRect.Bottom() + 30,
                            nTTop    = rLRect.GetTab()->Frame().Top()  + rLRect.GetTab()->Prt().Top(),
                            nTBottom = rLRect.GetTab()->Frame().Top()  + rLRect.GetTab()->Prt().Bottom();
                    if ( (nTTop    >= nLTop && nTTop      <= nLBottom) ||
                         (nTBottom >= nLTop && nTBottom <= nLBottom) )
                        bPaint = false;
                }
            }
            if ( bPaint )
            {
                if ( !pLast || *pLast != rLRect.GetColor() )
                {
                    pLast = &rLRect.GetColor();

                    DrawModeFlags nOldDrawMode = pOut->GetDrawMode();
                    if( properties.pSGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                        pOut->SetDrawMode( DrawModeFlags::Default );

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
            for ( size_t i = 0; i < aLineRects.size(); ++i )
            {
                SwLineRect &rLRect = aLineRects[i];
                if ( rLRect.IsPainted() )
                    continue;

                if ( rLRect.IsLocked() )
                {
                    nMinCount = std::min( nMinCount, i );
                    continue;
                }

                if ( !pLast || *pLast != rLRect.GetColor() )
                {
                    pLast = &rLRect.GetColor();

                    DrawModeFlags nOldDrawMode = pOut->GetDrawMode();
                    if( properties.pSGlobalShell->GetWin() &&
                        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
                    {
                        pOut->SetDrawMode( DrawModeFlags::Default );
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
                                   const SwLineRects *pRects,
                                   SwPaintProperties& properties )
{
    if ( !aLineRects.empty() )
    {
        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pOut );

        // Remove all help line that are almost covered (tables)
        for (size_type i = 0; i != aLineRects.size(); ++i)
        {
            SwLineRect &rLi = aLineRects[i];
            const bool bVerticalSubs = rLi.Height() > rLi.Width();

            for (size_type k = i + 1; k != aLineRects.size(); ++k)
            {
                SwLineRect &rLk = aLineRects[k];
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
                                aLineRects.erase(aLineRects.begin() + k);
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
                                aLineRects.erase(aLineRects.begin() + k);
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

        if ( pRects && (!pRects->aLineRects.empty()) )
            RemoveSuperfluousSubsidiaryLines( *pRects, properties );

        if ( !aLineRects.empty() )
        {
            pOut->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            pOut->SetLineColor();

            // Reset draw mode in high contrast mode in order to get fill color
            // set at output device. Recover draw mode after draw of lines.
            // Necessary for the subsidiary lines painted by the fly frames.
            DrawModeFlags nOldDrawMode = pOut->GetDrawMode();
            if( gProp.pSGlobalShell->GetWin() &&
                Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            {
                pOut->SetDrawMode( DrawModeFlags::Default );
            }

            for (SwSubsRects::iterator it = aLineRects.begin(); it != aLineRects.end();
                 ++it)
            {
                SwLineRect &rLRect = (*it);
                // Add condition <!rLRect.IsLocked()> to prevent paint of locked subsidiary lines.
                if ( !rLRect.IsPainted() &&
                     !rLRect.IsLocked() )
                {
                    const Color *pCol = nullptr;
                    switch ( rLRect.GetSubColor() )
                    {
                        case SUBCOL_PAGE: pCol = &SwViewOption::GetDocBoundariesColor(); break;
                        case SUBCOL_FLY: pCol = &SwViewOption::GetObjectBoundariesColor(); break;
                        case SUBCOL_TAB: pCol = &SwViewOption::GetTableBoundariesColor(); break;
                        case SUBCOL_SECT: pCol = &SwViewOption::GetSectionBoundColor(); break;
                    }

                    if (pCol && pOut->GetFillColor() != *pCol)
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

// Various functions that are use in this file.

/**
 * Function <SwAlignRect(..)> is also used outside this file
 *
 * Correction: adjust rectangle on pixel level in order to make sure,
 * that the border "leaves its original pixel", if it has to
 * No prior adjustments for odd relation between pixel and twip
 */
void SwAlignRect( SwRect &rRect, const SwViewShell *pSh, const vcl::RenderContext* pRenderContext )
{
    if( !rRect.HasArea() )
        return;

    // Make sure that view shell (parameter <pSh>) exists, if the output device
    // is taken from this view shell --> no output device, no alignment
    // Output device taken from view shell <pSh>, if <gProp.bSFlyMetafile> not set
    if ( !gProp.bSFlyMetafile && !pSh )
    {
        return;
    }

    const vcl::RenderContext *pOut = gProp.bSFlyMetafile ?
                        gProp.pSFlyMetafileOut.get() : pRenderContext;

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

/**
 * Helper for twip adjustments on pixel base
 *
 * This method compares the x- or y-pixel position of two twip-points.
 * If the x-/y-pixel positions are the same, the x-/y-pixel position of
 * the second twip point is adjusted by a given amount of pixels
*/
static void lcl_CompPxPosAndAdjustPos( const vcl::RenderContext&  _rOut,
                                const Point&         _rRefPt,
                                Point&               _rCompPt,
                                const bool          _bChkXPos,
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

/**
 * Method to pixel-align rectangle for drawing graphic object
 *
 * Because we are drawing graphics from the left-top-corner in conjunction
 * with size coordinates, these coordinates have to be calculated at a pixel
 * level.
 * Thus, we convert the rectangle to pixel and then convert to left-top-corner
 * and then get size of pixel rectangle back to logic.
 * This calculation is necessary, because there's a different between
 * the conversion from logic to pixel of a normal rectangle with its left-top-
 * and right-bottom-corner and the same conversion of the same rectangle
 * with left-top-corner and size.
 *
 * NOTE: Call this method before each <GraphicObject.Draw(...)>
*/
void SwAlignGrfRect( SwRect *pGrfRect, const vcl::RenderContext &rOut )
{
    Rectangle aPxRect = rOut.LogicToPixel( pGrfRect->SVRect() );
    pGrfRect->Pos( rOut.PixelToLogic( aPxRect.TopLeft() ) );
    pGrfRect->SSize( rOut.PixelToLogic( aPxRect.GetSize() ) );
}

static long lcl_AlignWidth( const long nWidth, SwPaintProperties& properties )
{
    if ( nWidth )
    {
        const long nW = nWidth % properties.nSPixelSzW;

        if ( !nW || nW > properties.nSHalfPixelSzW )
            return std::max(1L, nWidth - properties.nSHalfPixelSzW);
    }
    return nWidth;
}

static long lcl_AlignHeight( const long nHeight, SwPaintProperties& properties )
{
    if ( nHeight )
    {
        const long nH = nHeight % properties.nSPixelSzH;

        if ( !nH || nH > properties.nSHalfPixelSzH )
            return std::max(1L, nHeight - properties.nSHalfPixelSzH);
    }
    return nHeight;
}

static long lcl_MinHeightDist( const long nDist, SwPaintProperties& properties )
{
    if ( properties.aSScaleX < aMinDistScale || properties.aSScaleY < aMinDistScale )
        return nDist;
    return ::lcl_AlignHeight( std::max( nDist, properties.nSMinDistPixelH ), properties);
}

/**
 * Calculate PrtArea plus surrounding plus shadow
 */
static void lcl_CalcBorderRect( SwRect &rRect, const SwFrame *pFrame,
                                        const SwBorderAttrs &rAttrs,
                                        const bool bShadow,
                                        SwPaintProperties& properties)
{
    // Special handling for cell frames.
    // The printing area of a cell frame is completely enclosed in the frame area
    // and a cell frame has no shadow. Thus, for cell frames the calculated
    // area equals the frame area.
    // Notes: Borders of cell frames in R2L text direction will switch its side
    //        - left border is painted on the right; right border on the left.
    //        See <lcl_PaintLeftLine> and <lcl_PaintRightLine>.
    if( pFrame->IsSctFrame() )
    {
        rRect = pFrame->Prt();
        rRect.Pos() += pFrame->Frame().Pos();
    }
    else if ( pFrame->IsCellFrame() )
        rRect = pFrame->Frame();
    else
    {
        rRect = pFrame->Prt();
        rRect.Pos() += pFrame->Frame().Pos();

        if ( rAttrs.IsLine() || rAttrs.IsBorderDist() ||
             (bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE) )
        {
            SwRectFn fnRect = pFrame->IsVertical() ? ( pFrame->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

            const SvxBoxItem &rBox = rAttrs.GetBox();
            const bool bTop = 0 != (pFrame->*fnRect->fnGetTopMargin)();
            if ( bTop )
            {
                SwTwips nDiff = rBox.GetTop() ?
                    rBox.CalcLineSpace( SvxBoxItemLine::TOP ) :
                    ( rAttrs.IsBorderDist() ?
                      // Increase of distance by one twip is incorrect.
                      rBox.GetDistance( SvxBoxItemLine::TOP ) : 0 );
                if( nDiff )
                    (rRect.*fnRect->fnSubTop)( nDiff );
            }

            const bool bBottom = 0 != (pFrame->*fnRect->fnGetBottomMargin)();
            if ( bBottom )
            {
                SwTwips nDiff = 0;
                // #i29550#
                if ( pFrame->IsTabFrame() &&
                     static_cast<const SwTabFrame*>(pFrame)->IsCollapsingBorders() )
                {
                    // For collapsing borders, we have to add the height of
                    // the height of the last line
                    nDiff = static_cast<const SwTabFrame*>(pFrame)->GetBottomLineSize();
                }
                else
                {
                    nDiff = rBox.GetBottom() ?
                    rBox.CalcLineSpace( SvxBoxItemLine::BOTTOM ) :
                    ( rAttrs.IsBorderDist() ?
                      // Increase of distance by one twip is incorrect.
                      rBox.GetDistance( SvxBoxItemLine::BOTTOM ) : 0 );
                }
                if( nDiff )
                    (rRect.*fnRect->fnAddBottom)( nDiff );
            }

            if ( rBox.GetLeft() )
                (rRect.*fnRect->fnSubLeft)( rBox.CalcLineSpace( SvxBoxItemLine::LEFT ) );
            else if ( rAttrs.IsBorderDist() )
                 // Increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnSubLeft)( rBox.GetDistance( SvxBoxItemLine::LEFT ) );

            if ( rBox.GetRight() )
                (rRect.*fnRect->fnAddRight)( rBox.CalcLineSpace( SvxBoxItemLine::RIGHT ) );
            else if ( rAttrs.IsBorderDist() )
                 // Increase of distance by one twip is incorrect.
                (rRect.*fnRect->fnAddRight)( rBox.GetDistance( SvxBoxItemLine::RIGHT ) );

            if ( bShadow && rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                const SvxShadowItem &rShadow = rAttrs.GetShadow();
                if ( bTop )
                    (rRect.*fnRect->fnSubTop)(rShadow.CalcShadowSpace(SvxShadowItemSide::TOP));
                (rRect.*fnRect->fnSubLeft)(rShadow.CalcShadowSpace(SvxShadowItemSide::LEFT));
                if ( bBottom )
                    (rRect.*fnRect->fnAddBottom)
                                    (rShadow.CalcShadowSpace( SvxShadowItemSide::BOTTOM ));
                (rRect.*fnRect->fnAddRight)(rShadow.CalcShadowSpace(SvxShadowItemSide::RIGHT));
            }
        }
    }

    ::SwAlignRect( rRect, properties.pSGlobalShell, properties.pSGlobalShell ? properties.pSGlobalShell->GetOut() : nullptr );
}

/**
 * Extend left/right border/shadow rectangle to bottom of previous frame/to
 * top of next frame, if border/shadow is joined with previous/next frame
 */
static void lcl_ExtendLeftAndRight( SwRect&                _rRect,
                                         const SwFrame&           _rFrame,
                                         const SwBorderAttrs&   _rAttrs,
                                         const SwRectFn&        _rRectFn )
{
    if ( _rAttrs.JoinedWithPrev( _rFrame ) )
    {
        const SwFrame* pPrevFrame = _rFrame.GetPrev();
        (_rRect.*_rRectFn->fnSetTop)( (pPrevFrame->*_rRectFn->fnGetPrtBottom)() );
    }
    if ( _rAttrs.JoinedWithNext( _rFrame ) )
    {
        const SwFrame* pNextFrame = _rFrame.GetNext();
        (_rRect.*_rRectFn->fnSetBottom)( (pNextFrame->*_rRectFn->fnGetPrtTop)() );
    }
}

static void lcl_SubtractFlys( const SwFrame *pFrame, const SwPageFrame *pPage,
   const SwRect &rRect, SwRegionRects &rRegion, SwPaintProperties & rProperties)
{
    const SwSortedObjs& rObjs = *pPage->GetSortedObjs();
    const SwFlyFrame* pSelfFly = pFrame->IsInFly() ? pFrame->FindFlyFrame() : gProp.pSRetoucheFly2;
    if (!gProp.pSRetoucheFly)
        gProp.pSRetoucheFly = gProp.pSRetoucheFly2;

    for (size_t j = 0; (j < rObjs.size()) && !rRegion.empty(); ++j)
    {
        const SwAnchoredObject* pAnchoredObj = rObjs[j];
        const SdrObject* pSdrObj = pAnchoredObj->GetDrawObj();

        // Do not consider invisible objects
        if (!pPage->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(pSdrObj->GetLayer()))
            continue;

        if (dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) ==  nullptr)
            continue;

        const SwFlyFrame *pFly = static_cast<const SwFlyFrame*>(pAnchoredObj);

        if (pSelfFly == pFly || gProp.pSRetoucheFly == pFly || !rRect.IsOver(pFly->Frame()))
            continue;

        if (!pFly->GetFormat()->GetPrint().GetValue() &&
                (OUTDEV_PRINTER == gProp.pSGlobalShell->GetOut()->GetOutDevType() ||
                gProp.pSGlobalShell->IsPreview()))
            continue;

        const bool bLowerOfSelf = pSelfFly && pFly->IsLowerOf( pSelfFly );

        //For character bound Flys only examine those Flys in which it is not
        //anchored itself.
        //Why only for character bound ones you may ask? It never makes sense to
        //subtract frames in which it is anchored itself right?
        if (pSelfFly && pSelfFly->IsLowerOf(pFly))
            continue;

        //Any why does it not apply for the RetoucheFly too?
        if (gProp.pSRetoucheFly && gProp.pSRetoucheFly->IsLowerOf(pFly))
            continue;

#if OSL_DEBUG_LEVEL > 0
        //Flys who are anchored inside their own one, must have a bigger OrdNum
        //or be character bound.
        if (pSelfFly && bLowerOfSelf)
        {
            OSL_ENSURE( pFly->IsFlyInContentFrame() ||
                    pSdrObj->GetOrdNumDirect() > pSelfFly->GetVirtDrawObj()->GetOrdNumDirect(),
                    "Fly with wrong z-Order" );
        }
#endif

        bool bStopOnHell = true;
        if (pSelfFly)
        {
            const SdrObject *pTmp = pSelfFly->GetVirtDrawObj();
            if (pSdrObj->GetLayer() == pTmp->GetLayer())
            {
                if (pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect())
                    //In the same layer we only observe those that are above.
                    continue;
            }
            else
            {
                if (!bLowerOfSelf && !pFly->GetFormat()->GetOpaque().GetValue())
                    //From other layers we are only interested in non
                    //transparent ones or those that are internal
                    continue;
                bStopOnHell = false;
            }
        }
        if (gProp.pSRetoucheFly)
        {
            const SdrObject *pTmp = gProp.pSRetoucheFly->GetVirtDrawObj();
            if ( pSdrObj->GetLayer() == pTmp->GetLayer() )
            {
                if ( pSdrObj->GetOrdNumDirect() < pTmp->GetOrdNumDirect() )
                    //In the same layer we only observe those that are above.
                    continue;
            }
            else
            {
                if (!pFly->IsLowerOf( gProp.pSRetoucheFly ) && !pFly->GetFormat()->GetOpaque().GetValue())
                    //From other layers we are only interested in non
                    //transparent ones or those that are internal
                    continue;
                bStopOnHell = false;
            }
        }

        //If the content of the Fly is transparent, we subtract it only if it's
        //contained in the hell layer.
        const IDocumentDrawModelAccess& rIDDMA = pFly->GetFormat()->getIDocumentDrawModelAccess();
        bool bHell = pSdrObj->GetLayer() == rIDDMA.GetHellId();
        if ( (bStopOnHell && bHell) ||
             /// Change internal order of condition
             ///    first check "!bHell", then "..->Lower()" and "..->IsNoTextFrame()"
             ///    have not to be performed, if frame is in "Hell"
             ( !bHell && pFly->Lower() && pFly->Lower()->IsNoTextFrame() &&
               (static_cast<SwNoTextFrame const*>(pFly->Lower())->IsTransparent() ||
                static_cast<SwNoTextFrame const*>(pFly->Lower())->HasAnimation() ||
                 pFly->GetFormat()->GetSurround().IsContour()
               )
             )
           )
            continue;

        // Own if-statements for transparent background/shadow of fly frames
        // in order to handle special conditions.
        if (pFly->IsBackgroundTransparent())
        {
            // Background <pFly> is transparent drawn. Thus normally, its region
            // have not to be subtracted from given region.
            // But, if method is called for a fly frame and
            // <pFly> is a direct lower of this fly frame and
            // <pFly> inherites its transparent background brush from its parent,
            // then <pFly> frame area have to be subtracted from given region.
            // NOTE: Because in Status Quo transparent backgrounds can only be
            //     assigned to fly frames, the handle of this special case
            //     avoids drawing of transparent areas more than once, if
            //     a fly frame inherites a transparent background from its
            //     parent fly frame.
            if (pFrame->IsFlyFrame() &&
                (pFly->GetAnchorFrame()->FindFlyFrame() == pFrame) &&
                static_cast<const SwFlyFrameFormat*>(pFly->GetFormat())->IsBackgroundBrushInherited()
               )
            {
                SwRect aRect;
                SwBorderAttrAccess aAccess( SwFrame::GetCache(), static_cast<SwFrame const *>(pFly) );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                ::lcl_CalcBorderRect( aRect, pFly, rAttrs, true, rProperties );
                rRegion -= aRect;
                continue;
            }
            else
            {
                continue;
            }
        }

        if (bHell && pFly->GetAnchorFrame()->IsInFly())
        {
            //So the border won't get dismantled by the background of the other
            //Fly.
            SwRect aRect;
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), static_cast<SwFrame const *>(pFly) );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            ::lcl_CalcBorderRect( aRect, pFly, rAttrs, true, rProperties );
            rRegion -= aRect;
        }
        else
        {
            SwRect aRect( pFly->Prt() );
            aRect += pFly->Frame().Pos();
            rRegion -= aRect;
        }
    }
    if (gProp.pSRetoucheFly == gProp.pSRetoucheFly2)
        gProp.pSRetoucheFly = nullptr;
}

static void lcl_implDrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                 vcl::RenderContext* _pOut,
                                 const SwRect& _rAlignedPaintRect,
                                 const GraphicObject& _rGraphicObj,
                                 SwPaintProperties& properties)
{
    /// determine color of background
    ///     If color of background brush is not "no fill"/"auto fill" or
    ///     <SwPaintProperties.bSFlyMetafile> is set, use color of background brush, otherwise
    ///     use global retouche color.
    const Color aColor( ( (_rBackgrdBrush.GetColor() != COL_TRANSPARENT) || properties.bSFlyMetafile )
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
        tools::PolyPolygon aPoly( _rAlignedPaintRect.SVRect() );
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

/**
 * This is a local help method to draw a background for a graphic
 *
 * Under certain circumstances we have to draw a background for a graphic.
 * This method takes care of the conditions and draws the background with the
 * corresponding color.
 * Method introduced for bug fix #103876# in order to optimize drawing tiled
 * background graphics. Previously, this code was integrated in method
 * <lcl_DrawGraphic>.
 * Method implemented as a inline, checking the conditions and calling method
 * method <lcl_implDrawGraphicBackgrd(..)> for the intrinsic drawing.
 *
 * @param _rBackgrdBrush
 * background brush contain the color the background has to be drawn.
 *
 * @param _pOut
 * output device the background has to be drawn in.
 *
 * @param _rAlignedPaintRect
 * paint rectangle in the output device, which has to be drawn with the background.
 * rectangle have to be aligned by method ::SwAlignRect
 *
 * @param _rGraphicObj
 * graphic object, for which the background has to be drawn. Used for checking
 * the transparency of its bitmap, its type and if the graphic is drawn transparent
 *
 * @param _bNumberingGraphic
 * boolean indicating that graphic is used as a numbering.
 *
 * @param _bBackgrdAlreadyDrawn
 * boolean (optional; default: false) indicating, if the background is already drawn.
*/
static inline void lcl_DrawGraphicBackgrd( const SvxBrushItem& _rBackgrdBrush,
                                    OutputDevice* _pOut,
                                    const SwRect& _rAlignedPaintRect,
                                    const GraphicObject& _rGraphicObj,
                                    bool _bNumberingGraphic,
                                    SwPaintProperties& properties,
                                    bool _bBackgrdAlreadyDrawn = false)
{
    // draw background with background color, if
    //     (1) graphic is not used as a numbering AND
    //     (2) background is not already drawn AND
    //     (3) intrinsic graphic is transparent OR intrinsic graphic doesn't exists
    if ( !_bNumberingGraphic &&
         !_bBackgrdAlreadyDrawn &&
         ( _rGraphicObj.IsTransparent() || _rGraphicObj.GetType() == GRAPHIC_NONE  )
       )
    {
        lcl_implDrawGraphicBackgrd( _rBackgrdBrush, _pOut, _rAlignedPaintRect, _rGraphicObj, properties );
    }
}

/**
 * NNOTE: the transparency of the background graphic is saved in
 * SvxBrushItem.GetGraphicObject(<shell>).GetAttr().Set/GetTransparency()
 * and is considered in the drawing of the graphic
 *
 * Thus, to provide transparent background graphic for text frames nothing
 * has to be coded
 *
 * Use align rectangle for drawing graphic Pixel-align coordinates for
 * drawing graphic
 * Outsource code for drawing background of the graphic
 * with a background color in method <lcl_DrawGraphicBackgrd>
 *
 * Also, change type of <bGrfNum> and <bClip> from <bool> to <bool>
 */
static void lcl_DrawGraphic( const SvxBrushItem& rBrush, vcl::RenderContext *pOut,
                      SwViewShell &rSh, const SwRect &rGrf, const SwRect &rOut,
                      bool bClip, bool bGrfNum,
                      SwPaintProperties& properties,
                      bool bBackgrdAlreadyDrawn = false )
                      // add parameter <bBackgrdAlreadyDrawn> to indicate
                      // that the background is already drawn.
{
    // Calculate align rectangle from parameter <rGrf> and use aligned
    // rectangle <aAlignedGrfRect> in the following code
    SwRect aAlignedGrfRect = rGrf;
    ::SwAlignRect( aAlignedGrfRect, &rSh, pOut );

    // Change type from <bool> to <bool>.
    const bool bNotInside = bClip && !rOut.IsInside( aAlignedGrfRect );
    if ( bNotInside )
    {
        pOut->Push( PushFlags::CLIPREGION );
        pOut->IntersectClipRegion( rOut.SVRect() );
    }

    GraphicObject *pGrf = const_cast<GraphicObject*>(rBrush.GetGraphicObject());

    // Outsource drawing of background with a background color
    ::lcl_DrawGraphicBackgrd( rBrush, pOut, aAlignedGrfRect, *pGrf, bGrfNum, properties, bBackgrdAlreadyDrawn );

    // Because for drawing a graphic left-top-corner and size coordinates are
    // used, these coordinates have to be determined on pixel level.
    ::SwAlignGrfRect( &aAlignedGrfRect, *pOut );

    paintGraphicUsingPrimitivesHelper(*pOut,
        *pGrf, pGrf->GetAttr(), aAlignedGrfRect);

    if ( bNotInside )
        pOut->Pop();
}

bool DrawFillAttributes(
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const SwRect& rOriginalLayoutRect,
    const SwRegionRects& rPaintRegion,
    vcl::RenderContext& rOut)
{
    if(rFillAttributes.get() && rFillAttributes->isUsed())
    {
        basegfx::B2DRange aPaintRange(
            rPaintRegion.GetOrigin().Left(),
            rPaintRegion.GetOrigin().Top(),
            rPaintRegion.GetOrigin().Right(),
            rPaintRegion.GetOrigin().Bottom());

        if (!aPaintRange.isEmpty() &&
            !rPaintRegion.empty() &&
            !basegfx::fTools::equalZero(aPaintRange.getWidth()) &&
            !basegfx::fTools::equalZero(aPaintRange.getHeight()))
        {
            const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;

            //UUUU need to expand for correct AAed and non-AAed visualization as primitive.
            // This must probably be removed again when we will be able to get all Writer visualization
            // as primitives and Writer prepares all it's stuff in high precision coordinates (also
            // needs to avoid moving boundaries around to better show overlapping stuff...)
            if(aSvtOptionsDrawinglayer.IsAntiAliasing())
            {
                // if AAed in principle expand by 0.5 in all directions. Since painting edges of
                // AAed regions does not add to no transparence (0.5 opacity covered by 0.5 opacity
                // is not full opacity but 0.75 opacity) we need some overlap here to avoid paint
                // artifacts. Checked experimentally - a little bit more in Y is needed, probably
                // due to still existing integer alignment and crunching in writer.
                static double fExpandX = 0.55;
                static double fExpandY = 0.70;
                const basegfx::B2DVector aSingleUnit(rOut.GetInverseViewTransformation() * basegfx::B2DVector(fExpandX, fExpandY));

                aPaintRange.expand(aPaintRange.getMinimum() - aSingleUnit);
                aPaintRange.expand(aPaintRange.getMaximum() + aSingleUnit);
            }
            else
            {
                // if not AAed expand by one unit to bottom right due to the missing unit
                // from SwRect/Rectangle integer handling
                const basegfx::B2DVector aSingleUnit(rOut.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));

                aPaintRange.expand(aPaintRange.getMaximum() + aSingleUnit);
            }

            const basegfx::B2DRange aDefineRange(
                rOriginalLayoutRect.Left(),
                rOriginalLayoutRect.Top(),
                rOriginalLayoutRect.Right(),
                rOriginalLayoutRect.Bottom());

            const drawinglayer::primitive2d::Primitive2DContainer& rSequence = rFillAttributes->getPrimitive2DSequence(
                aPaintRange,
                aDefineRange);

            if(rSequence.size())
            {
                drawinglayer::primitive2d::Primitive2DContainer const*
                    pPrimitives(&rSequence);
                drawinglayer::primitive2d::Primitive2DContainer primitives;
                // tdf#86578 the awful lcl_SubtractFlys hack
                if (rPaintRegion.size() > 1 || rPaintRegion[0] != rPaintRegion.GetOrigin())
                {
                    tools::PolyPolygon tempRegion;
                    for (size_t i = 0; i < rPaintRegion.size(); ++i)
                    {
                        tempRegion.Insert( tools::Polygon(rPaintRegion[i].SVRect()));
                    }
                    basegfx::B2DPolyPolygon const maskRegion( tempRegion.getB2DPolyPolygon());

                    primitives.resize(1);
                    primitives[0] = new drawinglayer::primitive2d::MaskPrimitive2D(
                            maskRegion, rSequence);
                    pPrimitives = &primitives;
                }
                assert(pPrimitives && pPrimitives->size());

                const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                    basegfx::B2DHomMatrix(),
                    rOut.GetViewTransformation(),
                    aPaintRange,
                    nullptr,
                    0.0,
                    uno::Sequence< beans::PropertyValue >());
                drawinglayer::processor2d::BaseProcessor2D* pProcessor = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                    rOut,
                    aViewInformation2D);

                if(pProcessor)
                {
                    pProcessor->process(*pPrimitives);

                    delete pProcessor;

                    return true;
                }
            }
        }
    }

    return false;
}

void DrawGraphic(
    const SvxBrushItem *pBrush,
    vcl::RenderContext *pOutDev,
    const SwRect &rOrg,
    const SwRect &rOut,
    const sal_uInt8 nGrfNum,
    const bool bConsiderBackgroundTransparency )
    // Add 6th parameter to indicate that method should
    // consider background transparency, saved in the color of the brush item
{
    SwViewShell &rSh = *gProp.pSGlobalShell;
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
                pBrush->PurgeMedium();
            }
            OUString referer;
            SfxObjectShell * sh = rSh.GetDoc()->GetPersist();
            if (sh != nullptr && sh->HasName()) {
                referer = sh->GetMedium()->GetName();
            }
            const Graphic* pGrf = pBrush->GetGraphic(referer);
            if( pGrf && GRAPHIC_NONE != pGrf->GetType() )
            {
                ePos = pBrush->GetGraphicPos();
                if( pGrf->IsSupportedGraphic() )
                    // don't the use the specific output device! Bug 94802
                    aGrfSize = ::GetGraphicSizeTwip( *pGrf, nullptr );
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
        // Despite the fact that the background graphic has to fill the complete
        // area, we already checked, whether the graphic will completely fill out
        // the region the <rOut> that is to be painted. Thus, nothing has to be
        // touched again.
        // E.g. this is the case for a Fly Frame without a background
        // brush positioned on the border of the page which inherited the background
        // brush from the page.
        bRetouche = !rOut.IsInside( aGrf );
        break;

    case GPOS_TILED:
        {
            // draw background of tiled graphic before drawing tiled graphic in loop
            // determine graphic object
            GraphicObject* pGraphicObj = const_cast< GraphicObject* >(pBrush->GetGraphicObject());
            // calculate aligned paint rectangle
            SwRect aAlignedPaintRect = rOut;
            ::SwAlignRect( aAlignedPaintRect, &rSh, pOutDev );
            // draw background color for aligned paint rectangle
            lcl_DrawGraphicBackgrd( *pBrush, pOutDev, aAlignedPaintRect, *pGraphicObj, bGrfNum, gProp );

            // set left-top-corner of background graphic to left-top-corner of the
            // area, from which the background brush is determined.
            aGrf.Pos() = rOrg.Pos();
            // setup clipping at output device
            pOutDev->Push( PushFlags::CLIPREGION );
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

                //                  A_out
                // n_chars = k1 *  ---------- + k2 * A_bitmap
                //                  A_bitmap

                // minimum n_chars is obtained for (derive for  A_bitmap,
                // set to 0, take positive solution):
                //                   k1
                // A_bitmap = Sqrt( ---- A_out )
                //                   k2

                // where k1 is the number of chars per draw operation, and
                // k2 is the number of chars per bitmap pixel.
                // This is approximately 50 and 7 for current PDF writer, respectively.

                const double    k1( 50 );
                const double    k2( 7 );
                const Size      aSize( aAlignedPaintRect.SSize() );
                const double    Abitmap( k1/k2 * static_cast<double>(aSize.Width())*aSize.Height() );

                pGraphicObj->DrawTiled( pOutDev,
                                        aAlignedPaintRect.SVRect(),
                                        aGrf.SSize(),
                                        Size( aPaintOffset.X(), aPaintOffset.Y() ),
                                        nullptr, GraphicManagerDrawFlags::STANDARD,
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
        pOutDev->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
        pOutDev->SetLineColor();

        // check, if a existing background graphic (not filling the complete
        // background) is transparent drawn and the background color is
        // "no fill" respectively "auto fill", if background transparency
        // has to be considered.
        // If YES, memorize transparency of background graphic.
        // check also, if background graphic bitmap is transparent.
        bool bTransparentGrfWithNoFillBackgrd = false;
        sal_Int32 nGrfTransparency = 0;
        bool bGrfIsTransparent = false;
        if ( (ePos != GPOS_NONE) &&
             (ePos != GPOS_TILED) && (ePos != GPOS_AREA)
           )
        {
            GraphicObject *pGrf = const_cast<GraphicObject*>(pBrush->GetGraphicObject());
            if ( bConsiderBackgroundTransparency )
            {
                GraphicAttr aGrfAttr = pGrf->GetAttr();
                if ( (aGrfAttr.GetTransparency() != 0) &&
                     (pBrush->GetColor() == COL_TRANSPARENT)
                   )
                {
                    bTransparentGrfWithNoFillBackgrd = true;
                    nGrfTransparency = aGrfAttr.GetTransparency();
                }
            }
            if ( pGrf->IsTransparent() )
            {
                bGrfIsTransparent = true;
            }
        }

        // to get color of brush, check background color against COL_TRANSPARENT ("no fill"/"auto fill")
        // instead of checking, if transparency is not set.
        const Color aColor( pBrush &&
                            ( !(pBrush->GetColor() == COL_TRANSPARENT) ||
                              gProp.bSFlyMetafile )
                    ? pBrush->GetColor()
                    : aGlobalRetoucheColor );

        // determine, if background region have to be
        //     drawn transparent.
        //     background region has to be drawn transparent, if
        //         background transparency have to be considered
        //     AND
        //       ( background color is transparent OR
        //         background graphic is transparent and background color is "no fill"
        //       )

        enum DrawStyle {
            Default,
            Transparent,
        } eDrawStyle = Default;

        if (bConsiderBackgroundTransparency &&
                ( ( aColor.GetTransparency() != 0) ||
                bTransparentGrfWithNoFillBackgrd ) )
        {
            eDrawStyle = Transparent;
        }

        // #i75614# reset draw mode in high contrast mode in order to get fill color set
        const DrawModeFlags nOldDrawMode = pOutDev->GetDrawMode();
        if ( gProp.pSGlobalShell->GetWin() &&
             Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            pOutDev->SetDrawMode( DrawModeFlags::Default );
        }

        // OD 06.08.2002 #99657# - if background region has to be drawn
        // transparent, set only the RGB values of the background color as
        // the fill color for the output device.
        switch (eDrawStyle)
        {
            case Transparent:
            {
                if( pOutDev->GetFillColor() != aColor.GetRGBColor() )
                    pOutDev->SetFillColor( aColor.GetRGBColor() );
                break;
            }
            default:
            {
                if( pOutDev->GetFillColor() != aColor )
                    pOutDev->SetFillColor( aColor );
                break;
            }
        }

        // #i75614#
        // restore draw mode
        pOutDev->SetDrawMode( nOldDrawMode );

        // OD 02.09.2002 #99657#
        switch (eDrawStyle)
        {
            case Transparent:
            {
                // background region have to be drawn transparent.
                // Thus, create a poly-polygon from the region and draw it with
                // the corresponding transparency percent.
                tools::PolyPolygon aDrawPoly( rOut.SVRect() );
                if ( aGrf.HasArea() )
                {
                    if ( !bGrfIsTransparent )
                    {
                        // subtract area of background graphic from draw area
                        // OD 08.10.2002 #103898# - consider only that part of the
                        // graphic area that is overlapping with draw area.
                        SwRect aTmpGrf = aGrf;
                        aTmpGrf.Intersection( rOut );
                        if ( aTmpGrf.HasArea() )
                        {
                            tools::Polygon aGrfPoly( aTmpGrf.SVRect() );
                            aDrawPoly.Insert( aGrfPoly );
                        }
                    }
                    else
                        bGrfBackgrdAlreadyDrawn = true;
                }
                // calculate transparency percent:
                // ( <transparency value[0x01..0xFF]>*100 + 0x7F ) / 0xFF
                // If there is a background graphic with a background color "no fill"/"auto fill",
                // the transparency value is taken from the background graphic,
                // otherwise take the transparency value from the color.
                sal_Int8 nTransparencyPercent = static_cast<sal_Int8>(
                  (( bTransparentGrfWithNoFillBackgrd ? nGrfTransparency : aColor.GetTransparency()
                   )*100 + 0x7F)/0xFF);
                // draw poly-polygon transparent
                pOutDev->DrawTransparent( aDrawPoly, nTransparencyPercent );

                break;
            }
            case Default:
            default:
            {
                SwRegionRects aRegion( rOut, 4 );
                if ( !bGrfIsTransparent )
                    aRegion -= aGrf;
                else
                    bGrfBackgrdAlreadyDrawn = true;
                // loop rectangles of background region, which has to be drawn
                for( size_t i = 0; i < aRegion.size(); ++i )
                {
                    pOutDev->DrawRect( aRegion[i].SVRect() );
                }
            }
        }
        pOutDev ->Pop();
    }

    if( bDraw && aGrf.IsOver( rOut ) )
        // OD 02.09.2002 #99657#
        // add parameter <bGrfBackgrdAlreadyDrawn>
        lcl_DrawGraphic( *pBrush, pOutDev, rSh, aGrf, rOut, true, bGrfNum, gProp,
                         bGrfBackgrdAlreadyDrawn );

    if( bReplaceGrfNum )
    {
        const BitmapEx& rBmp = rSh.GetReplacementBitmap(false);
        vcl::Font aTmp( pOutDev->GetFont() );
        Graphic::DrawEx( pOutDev, aEmptyOUStr, aTmp, rBmp, rOrg.Pos(), rOrg.SSize() );
    }
}

/**
 * Local helper for SwRootFrame::Paint(..) - Adjust given rectangle to pixel size
 *
 * By OD at 27.09.2002 for #103636#
 * In order to avoid paint errors caused by multiple alignments (e.g. ::SwAlignRect(..))
 * and other changes to the to be painted rectangle, this method is called for the
 * rectangle to be painted in order to adjust it to the pixel it is overlapping
*/
static void lcl_AdjustRectToPixelSize( SwRect& io_aSwRect, const vcl::RenderContext &aOut )
{
    // local constant object of class <Size> to determine number of Twips
    // representing a pixel.
    const Size aTwipToPxSize( aOut.PixelToLogic( Size( 1,1 )) );

    // local object of class <Rectangle> in Twip coordinates
    // calculated from given rectangle aligned to pixel centers.
    const Rectangle aPxCenterRect = aOut.PixelToLogic(
            aOut.LogicToPixel( io_aSwRect.SVRect() ) );

    // local constant object of class <Rectangle> representing given rectangle
    // in pixel.
    const Rectangle aOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );

    // calculate adjusted rectangle from pixel centered rectangle.
    // Due to rounding differences <aPxCenterRect> doesn't exactly represents
    // the Twip-centers. Thus, adjust borders by half of pixel width/height plus 1.
    // Afterwards, adjust calculated Twip-positions of the all borders.
    Rectangle aSizedRect = aPxCenterRect;
    aSizedRect.Left() -= (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Right() += (aTwipToPxSize.Width()/2 + 1);
    aSizedRect.Top() -= (aTwipToPxSize.Height()/2 + 1);
    aSizedRect.Bottom() += (aTwipToPxSize.Height()/2 + 1);

    // adjust left()
    while ( (aOut.LogicToPixel(aSizedRect)).Left() < aOrgPxRect.Left() )
    {
        ++aSizedRect.Left();
    }
    // adjust right()
    while ( (aOut.LogicToPixel(aSizedRect)).Right() > aOrgPxRect.Right() )
    {
        --aSizedRect.Right();
    }
    // adjust top()
    while ( (aOut.LogicToPixel(aSizedRect)).Top() < aOrgPxRect.Top() )
    {
        ++aSizedRect.Top();
    }
    // adjust bottom()
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
    // check Left()
    --aSizedRect.Left();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Left() >= (aTestNewPxRect.Left()+1),
            "Error in lcl_AlignRectToPixelSize(..): Left() not correct adjusted");
    ++aSizedRect.Left();
    // check Right()
    ++aSizedRect.Right();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Right() <= (aTestNewPxRect.Right()-1),
            "Error in lcl_AlignRectToPixelSize(..): Right() not correct adjusted");
    --aSizedRect.Right();
    // check Top()
    --aSizedRect.Top();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Top() >= (aTestNewPxRect.Top()+1),
            "Error in lcl_AlignRectToPixelSize(..): Top() not correct adjusted");
    ++aSizedRect.Top();
    // check Bottom()
    ++aSizedRect.Bottom();
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Bottom() <= (aTestNewPxRect.Bottom()-1),
            "Error in lcl_AlignRectToPixelSize(..): Bottom() not correct adjusted");
    --aSizedRect.Bottom();
#endif
}

// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES START

struct SwLineEntry
{
    SwTwips mnKey;
    SwTwips mnStartPos;
    SwTwips mnEndPos;
    SwTwips mnOffset;

    bool mbOffsetPerp;
    bool mbOffsetStart;
    bool mbOffsetEnd;

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
        mnOffset( 0 ),
        mbOffsetPerp(false),
        mbOffsetStart(false),
        mbOffsetEnd(false),
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
typedef std::map< SwTwips, SwLineEntrySet > SwLineEntryMap;

class SwTabFramePainter
{
    SwLineEntryMap maVertLines;
    SwLineEntryMap maHoriLines;
    const SwTabFrame& mrTabFrame;

    void Insert( SwLineEntry&, bool bHori );
    void Insert( const SwFrame& rFrame, const SvxBoxItem& rBoxItem );
    void HandleFrame( const SwLayoutFrame& rFrame );
    void FindStylesForLine( const Point&,
                            const Point&,
                            svx::frame::Style*,
                            bool bHori ) const;

    void AdjustTopLeftFrames();

public:
    explicit SwTabFramePainter( const SwTabFrame& rTabFrame );

    void PaintLines( OutputDevice& rDev, const SwRect& rRect ) const;
};

SwTabFramePainter::SwTabFramePainter( const SwTabFrame& rTabFrame )
    : mrTabFrame( rTabFrame )
{
    HandleFrame( rTabFrame );
    AdjustTopLeftFrames();
}

void SwTabFramePainter::HandleFrame( const SwLayoutFrame& rLayoutFrame )
{
    // Add border lines of cell frames. Skip covered cells. Skip cells
    // in special row span row, which do not have a negative row span:
    if ( rLayoutFrame.IsCellFrame() && !rLayoutFrame.IsCoveredCell() )
    {
        const SwCellFrame* pThisCell = static_cast<const SwCellFrame*>(&rLayoutFrame);
        const SwRowFrame* pRowFrame = static_cast<const SwRowFrame*>(pThisCell->GetUpper());
        const long nRowSpan = pThisCell->GetTabBox()->getRowSpan();
        if ( !pRowFrame->IsRowSpanLine() || nRowSpan > 1 || nRowSpan < -1 )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), &rLayoutFrame );
            const SwBorderAttrs& rAttrs = *aAccess.Get();
            const SvxBoxItem& rBox = rAttrs.GetBox();
            Insert( rLayoutFrame, rBox );
        }
    }

    // Recurse into lower layout frames, but do not recurse into lower tabframes.
    const SwFrame* pLower = rLayoutFrame.Lower();
    while ( pLower )
    {
        const SwLayoutFrame* pLowerLayFrame = dynamic_cast<const SwLayoutFrame*>(pLower);
        if ( pLowerLayFrame && !pLowerLayFrame->IsTabFrame() )
            HandleFrame( *pLowerLayFrame );

        pLower = pLower->GetNext();
    }
}

void SwTabFramePainter::PaintLines(OutputDevice& rDev, const SwRect& rRect) const
{
    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, rDev );

    SwLineEntryMap::const_iterator aIter = maHoriLines.begin();
    bool bHori = true;

    // color for subsidiary lines:
    const Color& rCol( SwViewOption::GetTableBoundariesColor() );

    // high contrast mode:
    // overrides the color of non-subsidiary lines.
    const Color* pHCColor = nullptr;
    DrawModeFlags nOldDrawMode = rDev.GetDrawMode();
    if( gProp.pSGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pHCColor = &SwViewOption::GetFontColor();
        rDev.SetDrawMode( DrawModeFlags::Default );
    }

    const SwFrame* pUpper = mrTabFrame.GetUpper();
    SwRect aUpper( pUpper->Prt() );
    aUpper.Pos() += pUpper->Frame().Pos();
    SwRect aUpperAligned( aUpper );
    ::SwAlignRect( aUpperAligned, gProp.pSGlobalShell, &rDev );

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
        for (SwLineEntrySet::const_iterator aSetIter = rEntrySet.begin();
                 aSetIter != rEntrySet.end(); ++aSetIter)
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

            svx::frame::Style aStyles[ 7 ];
            aStyles[ 0 ] = rEntryStyle;
            FindStylesForLine( aStart, aEnd, aStyles, bHori );

            // Account for double line thicknesses for the top- and left-most borders.
            if (rEntry.mnOffset)
            {
                if (bHori)
                {
                    if (rEntry.mbOffsetPerp)
                    {
                        // Apply offset in perpendicular direction.
                        aStart.Y() -= rEntry.mnOffset;
                        aEnd.Y() -= rEntry.mnOffset;
                    }
                    if (rEntry.mbOffsetStart)
                        // Apply offset at the start of a border.
                        aStart.X() -= rEntry.mnOffset;
                    if (rEntry.mbOffsetEnd)
                        // Apply offset at the end of a border.
                        aEnd.X() += rEntry.mnOffset;
                }
                else
                {
                    if (rEntry.mbOffsetPerp)
                    {
                        // Apply offset in perpendicular direction.
                        aStart.X() -= rEntry.mnOffset;
                        aEnd.X() -= rEntry.mnOffset;
                    }
                    if (rEntry.mbOffsetStart)
                        // Apply offset at the start of a border.
                        aStart.Y() -= rEntry.mnOffset;
                    if (rEntry.mbOffsetEnd)
                        // Apply offset at the end of a border.
                        aEnd.Y() += rEntry.mnOffset;
                }
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

            if (!rRect.IsOver(aRepaintRect))
            {
                continue;
            }

            // subsidiary lines
            const Color* pTmpColor = nullptr;
            if (0 == aStyles[ 0 ].GetWidth())
            {
                if (isTableBoundariesEnabled() && gProp.pSGlobalShell->GetWin())
                    aStyles[ 0 ].Set( rCol, rCol, rCol, false, 1, 0, 0 );
                else
                    aStyles[0].SetType(table::BorderLineStyle::NONE);
            }
            else
                pTmpColor = pHCColor;

            // The (twip) positions will be adjusted to meet these requirements:
            // 1. The y coordinates are located in the middle of the pixel grid
            // 2. The x coordinated are located at the beginning of the pixel grid
            // This is done, because the horizontal lines are painted "at
            // beginning", whereas the vertical lines are painted "centered".
            // By making the line sizes a multiple of one pixel size, we can
            // assure that all lines having the same twip size have the same
            // pixel size, independent of their position on the screen.
            Point aPaintStart = rDev.PixelToLogic( rDev.LogicToPixel(aStart) );
            Point aPaintEnd = rDev.PixelToLogic( rDev.LogicToPixel(aEnd) );

            if (gProp.pSGlobalShell->GetWin())
            {
                // The table borders do not use SwAlignRect, but all the other frames do.
                // Therefore we tweak the outer borders a bit to achieve that the outer
                // borders match the subsidiary lines of the upper:
                if (aStart.X() == aUpper.Left())
                    aPaintStart.X() = aUpperAligned.Left();
                else if (aStart.X() == aUpper.Rigth_())
                    aPaintStart.X() = aUpperAligned.Rigth_();
                if (aStart.Y() == aUpper.Top())
                    aPaintStart.Y() = aUpperAligned.Top();
                else if (aStart.Y() == aUpper.Bottom_())
                    aPaintStart.Y() = aUpperAligned.Bottom_();

                if (aEnd.X() == aUpper.Left())
                    aPaintEnd.X() = aUpperAligned.Left();
                else if (aEnd.X() == aUpper.Rigth_())
                    aPaintEnd.X() = aUpperAligned.Rigth_();
                if (aEnd.Y() == aUpper.Top())
                    aPaintEnd.Y() = aUpperAligned.Top();
                else if (aEnd.Y() == aUpper.Bottom_())
                    aPaintEnd.Y() = aUpperAligned.Bottom_();
            }

            // logically vertical lines are painted centered on the line,
            // logically horizontal lines are painted "below" the line
            bool const isBelow((mrTabFrame.IsVertical()) ? !bHori : bHori);
            double const offsetStart = (isBelow)
                ?   aStyles[0].GetWidth() / 2.0
                :   std::max<double>(aStyles[1].GetWidth(),
                        aStyles[3].GetWidth()) / 2.0;
            double const offsetEnd = (isBelow)
                ?   aStyles[0].GetWidth() / 2.0
                :   std::max<double>(aStyles[4].GetWidth(),
                        aStyles[6].GetWidth()) / 2.0;
            if (mrTabFrame.IsVertical())
            {
                aPaintStart.X() -= static_cast<long>(offsetStart + 0.5);
                aPaintEnd.X()   -= static_cast<long>(offsetEnd   + 0.5);
            }
            else
            {
                aPaintStart.Y() += static_cast<long>(offsetStart + 0.5);
                aPaintEnd.Y()   += static_cast<long>(offsetEnd   + 0.5);
            }

            if( rEntryStyle.Type() == table::BorderLineStyle::DOUBLE_THIN )
            {
                long aPixel = rDev.PixelToLogic( Point(1, 1) ).getX();
                SwRect aPaintEx( aPaintStart, aPaintEnd );
                if( bHori )
                {
                    aPaintEx.Pos().Y() -= aPixel;
                    aPaintEx.SSize().Height() += aPixel * 2;
                }
                else
                {
                    aPaintEx.Pos().X() -= aPixel;
                    aPaintEx.SSize().Width() += aPixel * 2;
                }

                gProp.pSGlobalShell->InvalidateWindows( aPaintEx );
            }

            if (bHori)
            {
                mrTabFrame.ProcessPrimitives( svx::frame::CreateBorderPrimitives(
                    aPaintStart,
                    aPaintEnd,
                    aStyles[ 0 ],   // current style
                    aStyles[ 1 ],   // aLFromT
                    aStyles[ 2 ],   // aLFromL
                    aStyles[ 3 ],   // aLFromB
                    aStyles[ 4 ],   // aRFromT
                    aStyles[ 5 ],   // aRFromR
                    aStyles[ 6 ],   // aRFromB
                    pTmpColor)
                );
            }
            else
            {
                mrTabFrame.ProcessPrimitives( svx::frame::CreateBorderPrimitives(
                    aPaintEnd,
                    aPaintStart,
                    aStyles[ 0 ],   // current style
                    aStyles[ 4 ],   // aBFromL
                    aStyles[ 5 ],   // aBFromB
                    aStyles[ 6 ],   // aBFromR
                    aStyles[ 1 ],   // aTFromL
                    aStyles[ 2 ],   // aTFromT
                    aStyles[ 3 ],   // aTFromR
                    pTmpColor)
                );
            }
        }

        ++aIter;
    }

    // restore output device:
    rDev.SetDrawMode( nOldDrawMode );
}

/**
 * Finds the lines that join the line defined by (StartPoint, EndPoint) in either
 * StartPoint or Endpoint. The styles of these lines are required for DR's magic
 * line painting functions
 */
void SwTabFramePainter::FindStylesForLine( const Point& rStartPoint,
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

    SwLineEntryMap::const_iterator aMapIter = maVertLines.find( rStartPoint.X() );
    OSL_ENSURE( aMapIter != maVertLines.end(), "FindStylesForLine: Error" );
    const SwLineEntrySet& rVertSet = (*aMapIter).second;
    SwLineEntrySet::const_iterator aIter = rVertSet.begin();

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

namespace {

void calcOffsetForDoubleLine( SwLineEntryMap& rLines )
{
    SwLineEntryMap aNewLines;
    SwLineEntryMap::iterator it = rLines.begin(), itEnd = rLines.end();
    bool bFirst = true;
    for (; it != itEnd; ++it)
    {
        if (bFirst)
        {
            // First line needs to be offset to account for double line thickness.
            SwLineEntrySet aNewSet;
            const SwLineEntrySet& rSet = it->second;
            SwLineEntrySet::iterator itSet = rSet.begin(), itSetEnd = rSet.end();
            size_t nEntryCount = rSet.size();
            for (size_t i = 0; itSet != itSetEnd; ++itSet, ++i)
            {
                SwLineEntry aLine = *itSet;
                if (aLine.maAttribute.Secn())
                {
                    // Apply offset only for double lines.
                    aLine.mnOffset = static_cast<SwTwips>(aLine.maAttribute.Dist());
                    aLine.mbOffsetPerp = true;

                    if (i == 0)
                        aLine.mbOffsetStart = true;
                    if (i == nEntryCount - 1)
                        aLine.mbOffsetEnd = true;
                }

                aNewSet.insert(aLine);
            }

            aNewLines.insert(SwLineEntryMap::value_type(it->first, aNewSet));
        }
        else
            aNewLines.insert(SwLineEntryMap::value_type(it->first, it->second));

        bFirst = false;
    }
    rLines.swap(aNewLines);
}

}

void SwTabFramePainter::AdjustTopLeftFrames()
{
    calcOffsetForDoubleLine(maHoriLines);
    calcOffsetForDoubleLine(maVertLines);
}

/**
 * Special case: #i9860#
 * first line in follow table without repeated headlines
 */
static bool lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines(
        SwTabFrame const& rTabFrame, SwFrame const& rFrame, SvxBoxItem const& rBoxItem)
{
    SwRowFrame const*const pThisRowFrame =
        dynamic_cast<const SwRowFrame*>(rFrame.GetUpper());
    return (pThisRowFrame
        && (pThisRowFrame->GetUpper() == &rTabFrame)
        && rTabFrame.IsFollow()
        && !rTabFrame.GetTable()->GetRowsToRepeat()
        &&  (  !pThisRowFrame->GetPrev()
            || static_cast<const SwRowFrame*>(pThisRowFrame->GetPrev())
                    ->IsRowSpanLine())
        && !rBoxItem.GetTop()
        && rBoxItem.GetBottom());
}

void SwTabFramePainter::Insert( const SwFrame& rFrame, const SvxBoxItem& rBoxItem )
{
    // build 4 line entries for the 4 borders:
    SwRect aBorderRect = rFrame.Frame();
    if ( rFrame.IsTabFrame() )
    {
        aBorderRect = rFrame.Prt();
        aBorderRect.Pos() += rFrame.Frame().Pos();
    }

    bool const bBottomAsTop(lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines(
                mrTabFrame, rFrame, rBoxItem));
    bool const bVert = mrTabFrame.IsVertical();
    bool const bR2L  = mrTabFrame.IsRightToLeft();

    SwViewShell* pViewShell = mrTabFrame.getRootFrame()->GetCurrShell();
    OutputDevice* pOutDev = pViewShell->GetOut();
    const MapMode& rMapMode = pOutDev->GetMapMode();
    const Fraction& rFracX = rMapMode.GetScaleX();
    const Fraction& rFracY = rMapMode.GetScaleY();

    svx::frame::Style aL(rBoxItem.GetLeft());
    aL.SetPatternScale(rFracY);
    svx::frame::Style aR(rBoxItem.GetRight());
    aR.SetPatternScale(rFracY);
    svx::frame::Style aT(rBoxItem.GetTop());
    aT.SetPatternScale(rFracX);
    svx::frame::Style aB(rBoxItem.GetBottom());
    aB.SetPatternScale(rFracX);

    aR.MirrorSelf();
    aB.MirrorSelf();

    const SwTwips nLeft   = aBorderRect.Left_();
    const SwTwips nRight  = aBorderRect.Rigth_();
    const SwTwips nTop    = aBorderRect.Top_();
    const SwTwips nBottom = aBorderRect.Bottom_();

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

void SwTabFramePainter::Insert( SwLineEntry& rNew, bool bHori )
{
    // get all lines from structure, that have key entry of pLE
    SwLineEntryMap* pLine2 = bHori ? &maHoriLines : &maVertLines;
    const SwTwips nKey = rNew.mnKey;
    SwLineEntryMap::iterator aMapIter = pLine2->find( nKey );

    SwLineEntrySet* pLineSet = aMapIter != pLine2->end() ? &((*aMapIter).second) : nullptr;
    if ( !pLineSet )
    {
        SwLineEntrySet aNewSet;
        (*pLine2)[ nKey ] = aNewSet;
        pLineSet = &(*pLine2)[ nKey ];
    }
    SwLineEntrySet::iterator aIter = pLineSet->begin();

    while ( aIter != pLineSet->end() && rNew.mnStartPos < rNew.mnEndPos )
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

/**
 * FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES END
 * --> OD #i76669#
 */
namespace
{
    class SwViewObjectContactRedirector : public sdr::contact::ViewObjectContactRedirector
    {
        private:
            const SwViewShell& mrViewShell;

        public:
            explicit SwViewObjectContactRedirector( const SwViewShell& rSh )
                : mrViewShell( rSh )
            {};

            virtual ~SwViewObjectContactRedirector()
            {}

            virtual drawinglayer::primitive2d::Primitive2DContainer createRedirectedPrimitive2DSequence(
                                    const sdr::contact::ViewObjectContact& rOriginal,
                                    const sdr::contact::DisplayInfo& rDisplayInfo) override
            {
                bool bPaint( true );

                SdrObject* pObj = rOriginal.GetViewContact().TryToGetSdrObject();
                if ( pObj )
                {
                    bPaint = SwFlyFrame::IsPaint( pObj, &mrViewShell );
                }

                if ( !bPaint )
                {
                    return drawinglayer::primitive2d::Primitive2DContainer();
                }

                return sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
                                                        rOriginal, rDisplayInfo );
            }
    };

} // end of anonymous namespace
// <--

/**
 * Paint once for every visible page which is touched by Rect
 *
 * 1. Paint borders and backgrounds
 * 2. Paint the draw layer (frames and drawing objects) that is
 *    below the document (hell)
 * 3. Paint the document content (text)
 * 4. Paint the draw layer that is above the document
|*/
void SwRootFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const pPrintData) const
{
    OSL_ENSURE( Lower() && Lower()->IsPageFrame(), "Lower of root is no page." );

    PROTOCOL( this, PROT::FileInit, DbgAction::NONE, nullptr)

    bool bResetRootPaint = false;
    SwViewShell *pSh = mpCurrShell;

    if ( pSh->GetWin() )
    {
        if ( pSh->GetOut() == pSh->GetWin() && !pSh->GetWin()->IsVisible() )
        {
            return;
        }
        if (SwRootFrame::s_isInPaint)
        {
            SwPaintQueue::Add( pSh, rRect );
            return;
        }
    }
    else
        SwRootFrame::s_isInPaint = bResetRootPaint = true;

    SwSavePaintStatics *pStatics = nullptr;
    if ( gProp.pSGlobalShell )
        pStatics = new SwSavePaintStatics();
    gProp.pSGlobalShell = pSh;

    if( !pSh->GetWin() )
        gProp.pSProgress = SfxProgress::GetActiveProgress( static_cast<SfxObjectShell*>(pSh->GetDoc()->GetDocShell()) );

    ::SwCalcPixStatics( pSh->GetOut() );
    aGlobalRetoucheColor = pSh->Imp()->GetRetoucheColor();

    //Trigger an action to clear things up if needed.
    //Using this trick we can ensure that all values are valid in all paints -
    //no problems, no special case(s).
    // #i92745#
    // Extend check on certain states of the 'current' <SwViewShell> instance to
    // all existing <SwViewShell> instances.
    bool bPerformLayoutAction( true );
    {
        for(SwViewShell& rTmpViewShell : pSh->GetRingContainer())
        {
            if ( rTmpViewShell.IsInEndAction() ||
                 rTmpViewShell.IsPaintInProgress() ||
                 ( rTmpViewShell.Imp()->IsAction() &&
                   rTmpViewShell.Imp()->GetLayAction().IsActionInProgress() ) )
            {
                bPerformLayoutAction = false;
            }

            if(!bPerformLayoutAction)
                break;
        }
    }
    if ( bPerformLayoutAction )
    {
        const_cast<SwRootFrame*>(this)->ResetTurbo();
        SwLayAction aAction( const_cast<SwRootFrame*>(this), pSh->Imp() );
        aAction.SetPaint( false );
        aAction.SetComplete( false );
        aAction.SetReschedule( gProp.pSProgress != nullptr );
        aAction.Action(&rRenderContext);
        ResetTurboFlag();
        if ( !pSh->ActionPend() )
            pSh->Imp()->DelRegion();
    }

    SwRect aRect( rRect );
    aRect.Intersection( pSh->VisArea() );

    const bool bExtraData = ::IsExtraData( GetFormat()->GetDoc() );

    gProp.pSLines = new SwLineRects;   //Container for borders.

    // #104289#. During painting, something (OLE) can
    // load the linguistic, which in turn can cause a reformat
    // of the document. Dangerous! We better set this flag to
    // avoid the reformat.
    const bool bOldAction = IsCallbackActionEnabled();
    const_cast<SwRootFrame*>(this)->SetCallbackActionEnabled( false );

    const SwPageFrame *pPage = pSh->Imp()->GetFirstVisPage(&rRenderContext);

    const bool bBookMode = gProp.pSGlobalShell->GetViewOptions()->IsViewLayoutBookMode();
    if ( bBookMode && pPage->GetPrev() && static_cast<const SwPageFrame*>(pPage->GetPrev())->IsEmptyPage() )
        pPage = static_cast<const SwPageFrame*>(pPage->GetPrev());

    // #i68597#
    const bool bGridPainting(pSh->GetWin() && pSh->Imp()->HasDrawView() && pSh->Imp()->GetDrawView()->IsGridVisible());

    // Hide all page break controls before showing them again
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( gProp.pSGlobalShell );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
        SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();
        const SwPageFrame* pHiddenPage = pPage;
        while ( pHiddenPage->GetPrev() != nullptr )
        {
            pHiddenPage = static_cast< const SwPageFrame* >( pHiddenPage->GetPrev() );
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
        const bool bRightSidebar = pPage->SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT;

        if ( !pPage->IsEmptyPage() )
        {
            SwRect aPaintRect;
            SwPageFrame::GetBorderAndShadowBoundRect( pPage->Frame(), pSh, &rRenderContext, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

            if ( aRect.IsOver( aPaintRect ) )
            {
                if ( pSh->GetWin() )
                {
                    gProp.pSSubsLines = new SwSubsRects;
                    gProp.pSSpecSubsLines = new SwSubsRects;
                }
                gProp.pBLines = new BorderLines;

                aPaintRect.Intersection_( aRect );

                if ( bExtraData &&
                     pSh->GetWin() && pSh->IsInEndAction() )
                {
                    // enlarge paint rectangle to complete page width, subtract
                    // current paint area and invalidate the resulting region.
                    SWRECTFN( pPage )
                    SwRect aPageRectTemp( aPaintRect );
                    (aPageRectTemp.*fnRect->fnSetLeftAndWidth)(
                         (pPage->Frame().*fnRect->fnGetLeft)(),
                         (pPage->Frame().*fnRect->fnGetWidth)() );
                    aPageRectTemp.Intersection_( pSh->VisArea() );
                    vcl::Region aPageRectRegion( aPageRectTemp.SVRect() );
                    aPageRectRegion.Exclude( aPaintRect.SVRect() );
                    pSh->GetWin()->Invalidate( aPageRectRegion, InvalidateFlags::Children );
                }

                // #i80793#
                // enlarge paint rectangle for objects overlapping the same pixel
                // in all cases and before the DrawingLayer overlay is initialized.
                lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );

                // #i68597#
                // moved paint pre-process for DrawingLayer overlay here since the above
                // code dependent from bExtraData may expand the PaintRect
                {
                    // #i75172# if called from SwViewShell::ImplEndAction it should no longer
                    // really be used but handled by SwViewShell::ImplEndAction already
                    const vcl::Region aDLRegion(aPaintRect.SVRect());
                    pSh->DLPrePaint2(aDLRegion);
                }

                if(OUTDEV_WINDOW == gProp.pSGlobalShell->GetOut()->GetOutDevType())
                {
                    // OD 27.09.2002 #103636# - changed method SwLayVout::Enter(..)
                    // 2nd parameter is no longer <const> and will be set to the
                    // rectangle the virtual output device is calculated from <aPaintRect>,
                    // if the virtual output is used.
                    s_pVout->Enter(pSh, aPaintRect, !s_isNoVirDev);

                    // OD 27.09.2002 #103636# - adjust paint rectangle to pixel size
                    // Thus, all objects overlapping on pixel level with the unadjusted
                    // paint rectangle will be considered in the paint.
                    lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );
                }

                // maybe this can be put in the above scope. Since we are not sure, just leave it ATM
                s_pVout->SetOrgRect( aPaintRect );

                // OD 29.08.2002 #102450#
                // determine background color of page for <PaintLayer> method
                // calls, paint <hell> or <heaven>
                const Color aPageBackgrdColor(pPage->GetDrawBackgrdColor());

                pPage->PaintBaBo( aPaintRect, pPage );

                if ( pSh->Imp()->HasDrawView() )
                {
                    gProp.pSLines->LockLines( true );
                    const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
                    pSh->Imp()->PaintLayer( rIDDMA.GetHellId(),
                                            pPrintData,
                                            pPage->Frame(),
                                            &aPageBackgrdColor,
                                            pPage->IsRightToLeft(),
                                            &aSwRedirector );
                    gProp.pSLines->PaintLines( pSh->GetOut(), gProp );
                    gProp.pSLines->LockLines( false );
                }

                if ( pSh->GetDoc()->GetDocumentSettingManager().get( DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS ) )
                    pPage->PaintBaBo( aPaintRect, pPage, /*bOnlyTextBackground=*/true );

                if( pSh->GetWin() )
                {
                    // collect sub-lines
                    pPage->RefreshSubsidiary( aPaintRect );
                    // paint special sub-lines
                    gProp.pSSpecSubsLines->PaintSubsidiary( pSh->GetOut(), nullptr, gProp );
                }

                pPage->Paint( rRenderContext, aPaintRect );

                // no paint of page border and shadow, if writer is in place mode.
                if( pSh->GetWin() && pSh->GetDoc()->GetDocShell() &&
                    !pSh->GetDoc()->GetDocShell()->IsInPlaceActive() )
                {
                    SwPageFrame::PaintBorderAndShadow( pPage->Frame(), pSh, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
                    SwPageFrame::PaintNotesSidebar( pPage->Frame(), pSh, pPage->GetPhyPageNum(), bRightSidebar);
                }

                gProp.pSLines->PaintLines( pSh->GetOut(), gProp );
                if ( pSh->GetWin() )
                {
                    gProp.pSSubsLines->PaintSubsidiary( pSh->GetOut(), gProp.pSLines, gProp );
                    DELETEZ( gProp.pSSubsLines );
                    DELETEZ( gProp.pSSpecSubsLines );
                }
                // fdo#42750: delay painting these until after subsidiary lines
                // fdo#45562: delay painting these until after hell layer
                // fdo#47717: but do it before heaven layer
                ProcessPrimitives(gProp.pBLines->GetBorderLines_Clear());

                if ( pSh->Imp()->HasDrawView() )
                {
                    // OD 29.08.2002 #102450# - add 3rd parameter
                    // OD 09.12.2002 #103045# - add 4th parameter for horizontal text direction.
                    pSh->Imp()->PaintLayer( pSh->GetDoc()->getIDocumentDrawModelAccess().GetHeavenId(),
                                            pPrintData,
                                            pPage->Frame(),
                                            &aPageBackgrdColor,
                                            pPage->IsRightToLeft(),
                                            &aSwRedirector );
                }

                if ( bExtraData )
                    pPage->RefreshExtraData( aPaintRect );

                DELETEZ(gProp.pBLines);
                s_pVout->Leave();

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
            SwRect aEmptyPageRect( pPage->Frame() );

            // code from vprint.cxx
            const SwPageFrame& rFormatPage = pPage->GetFormatPage();
            aEmptyPageRect.SSize() = rFormatPage.Frame().SSize();

            SwPageFrame::GetBorderAndShadowBoundRect( aEmptyPageRect, pSh, &rRenderContext, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
            aPaintRect.Intersection_( aRect );

            if ( aRect.IsOver( aEmptyPageRect ) )
            {
                // #i75172# if called from SwViewShell::ImplEndAction it should no longer
                // really be used but handled by SwViewShell::ImplEndAction already
                {
                    const vcl::Region aDLRegion(aPaintRect.SVRect());
                    pSh->DLPrePaint2(aDLRegion);
                }

                if( pSh->GetOut()->GetFillColor() != aGlobalRetoucheColor )
                    pSh->GetOut()->SetFillColor( aGlobalRetoucheColor );

                pSh->GetOut()->SetLineColor(); // OD 20.02.2003 #107369# - no line color
                // OD 20.02.2003 #107369# - use aligned page rectangle
                {
                    SwRect aTmpPageRect( aEmptyPageRect );
                    ::SwAlignRect( aTmpPageRect, pSh, &rRenderContext );
                    aEmptyPageRect = aTmpPageRect;
                }

                pSh->GetOut()->DrawRect( aEmptyPageRect.SVRect() );

                // paint empty page text
                const vcl::Font& rEmptyPageFont = SwPageFrame::GetEmptyPageFont();
                const vcl::Font aOldFont( pSh->GetOut()->GetFont() );

                pSh->GetOut()->SetFont( rEmptyPageFont );
                pSh->GetOut()->DrawText( aEmptyPageRect.SVRect(), SW_RESSTR( STR_EMPTYPAGE ),
                                    DrawTextFlags::VCenter |
                                    DrawTextFlags::Center |
                                    DrawTextFlags::Clip );

                pSh->GetOut()->SetFont( aOldFont );
                // paint shadow and border for empty page
                // OD 19.02.2003 #107369# - use new method to paint page border and
                // shadow
                SwPageFrame::PaintBorderAndShadow( aEmptyPageRect, pSh, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
                SwPageFrame::PaintNotesSidebar( aEmptyPageRect, pSh, pPage->GetPhyPageNum(), bRightSidebar);

                {
                    pSh->DLPostPaint2(true);
                }
            }
        }

        OSL_ENSURE( !pPage->GetNext() || pPage->GetNext()->IsPageFrame(),
                "Neighbour of page is not a page." );
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
    }

    DELETEZ( gProp.pSLines );

    if ( bResetRootPaint )
        SwRootFrame::s_isInPaint = false;
    if ( pStatics )
        delete pStatics;
    else
    {
        gProp.pSProgress = nullptr;
        gProp.pSGlobalShell = nullptr;
    }

    const_cast<SwRootFrame*>(this)->SetCallbackActionEnabled( bOldAction );
}

static void lcl_EmergencyFormatFootnoteCont( SwFootnoteContFrame *pCont )
{
    vcl::RenderContext* pRenderContext = pCont->getRootFrame()->GetCurrShell()->GetOut();

    //It's possible that the Cont will get destroyed.
    SwContentFrame *pCnt = pCont->ContainsContent();
    while ( pCnt && pCnt->IsInFootnote() )
    {
        pCnt->Calc(pRenderContext);
        pCnt = pCnt->GetNextContentFrame();
    }
}

class SwShortCut
{
    SwRectDist fnCheck;
    long nLimit;
public:
    SwShortCut( const SwFrame& rFrame, const SwRect& rRect );
    bool Stop( const SwRect& rRect ) const
        { return (rRect.*fnCheck)( nLimit ) > 0; }
};

SwShortCut::SwShortCut( const SwFrame& rFrame, const SwRect& rRect )
{
    bool bVert = rFrame.IsVertical();
    bool bR2L = rFrame.IsRightToLeft();
    if( rFrame.IsNeighbourFrame() && bVert == bR2L )
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
    else if( bVert == rFrame.IsNeighbourFrame() )
    {
        fnCheck = &SwRect::GetTopDistance;
        nLimit = rRect.Top() + rRect.Height();
    }
    else
    {
        if ( rFrame.IsVertLR() )
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

void SwLayoutFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    // #i16816# tagged pdf support
    Frame_Info aFrameInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, &aFrameInfo, nullptr, rRenderContext );

    const SwFrame *pFrame = Lower();
    if ( !pFrame )
        return;

    SwShortCut aShortCut( *pFrame, rRect );
    bool bCnt;
    if ( (bCnt = pFrame->IsContentFrame()) )
        pFrame->Calc(&rRenderContext);

    if ( pFrame->IsFootnoteContFrame() )
    {
        ::lcl_EmergencyFormatFootnoteCont( const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pFrame)) );
        pFrame = Lower();
    }

    const SwPageFrame *pPage = nullptr;
    const bool bWin   = gProp.pSGlobalShell->GetWin() != nullptr;

    while ( IsAnLower( pFrame ) )
    {
        SwRect aPaintRect( pFrame->PaintArea() );
        if( aShortCut.Stop( aPaintRect ) )
            break;
        if ( bCnt && gProp.pSProgress )
            gProp.pSProgress->Reschedule();

        //We need to retouch if a frame explicitly requests it.
        //First do the retouch, because this could flatten the borders.
        if ( pFrame->IsRetouche() )
        {
            if ( pFrame->IsRetoucheFrame() && bWin && !pFrame->GetNext() )
            {   if ( !pPage )
                    pPage = FindPageFrame();
               pFrame->Retouch( pPage, rRect );
            }
            pFrame->ResetRetouche();
        }

        if ( rRect.IsOver( aPaintRect ) )
        {
            if ( bCnt && pFrame->IsCompletePaint() &&
                 !rRect.IsInside( aPaintRect ) && Application::AnyInput( VclInputFlags::KEYBOARD ) )
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
                        gProp.pSGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( rRect.Bottom() + 1 );
                    aPaintRect.Bottom( pFrame->Frame().Bottom() );
                    if ( aPaintRect.Height() > 0 )
                        gProp.pSGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( pFrame->Frame().Top() );
                    aPaintRect.Bottom( pFrame->Frame().Bottom() );
                }
                else
                {
                    gProp.pSGlobalShell->InvalidateWindows( aPaintRect );
                    pFrame = pFrame->GetNext();
                    if ( pFrame && (bCnt = pFrame->IsContentFrame()) )
                        pFrame->Calc(&rRenderContext);
                    continue;
                }
            }
            pFrame->ResetCompletePaint();
            aPaintRect.Intersection_( rRect );

            pFrame->Paint( rRenderContext, aPaintRect );

            if ( Lower() && Lower()->IsColumnFrame() )
            {
                //Paint the column separator line if needed. The page is
                //responsible for the page frame - not the upper.
                const SwFrameFormat *pFormat = GetUpper() && GetUpper()->IsPageFrame()
                                            ? GetUpper()->GetFormat()
                                            : GetFormat();
                const SwFormatCol &rCol = pFormat->GetCol();
                if ( rCol.GetLineAdj() != COLADJ_NONE )
                {
                    if ( !pPage )
                        pPage = pFrame->FindPageFrame();

                    PaintColLines( aPaintRect, rCol, pPage );
                }
            }
        }
        if ( !bCnt && pFrame->GetNext() && pFrame->GetNext()->IsFootnoteContFrame() )
            ::lcl_EmergencyFormatFootnoteCont( const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pFrame->GetNext())) );

        pFrame = pFrame->GetNext();

        if ( pFrame && (bCnt = pFrame->IsContentFrame()) )
            pFrame->Calc(&rRenderContext);
    }
}

static drawinglayer::primitive2d::Primitive2DContainer lcl_CreateDashedIndicatorPrimitive(
        const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd,
        basegfx::BColor aColor )
{
    drawinglayer::primitive2d::Primitive2DContainer aSeq( 1 );

    std::vector< double > aStrokePattern;
    basegfx::B2DPolygon aLinePolygon;
    aLinePolygon.append(rStart);
    aLinePolygon.append(rEnd);

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

        aSeq.resize( 2 );
    }

    // Compute the dashed line primitive
    drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D * pLine =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D (
                basegfx::B2DPolyPolygon( aLinePolygon ),
                drawinglayer::attribute::LineAttribute( aColor ),
                drawinglayer::attribute::StrokeAttribute( aStrokePattern ) );

    aSeq[ aSeq.size( ) - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pLine );

    return aSeq;
}

void SwPageFrame::PaintBreak( ) const
{
    if ( gProp.pSGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER  &&
         !gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() &&
         !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
         !gProp.pSGlobalShell->IsPreview() )
    {
        const SwFrame* pBodyFrame = Lower();
        while ( pBodyFrame && !pBodyFrame->IsBodyFrame() )
            pBodyFrame = pBodyFrame->GetNext();

        if ( pBodyFrame )
        {
            const SwLayoutFrame* pLayBody = static_cast< const SwLayoutFrame* >( pBodyFrame );
            const SwFlowFrame *pFlowFrame = pLayBody->ContainsContent();

            // Test if the first node is a table
            const SwFrame* pFirstFrame = pLayBody->Lower();
            if ( pFirstFrame && pFirstFrame->IsTabFrame() )
                pFlowFrame = static_cast< const SwTabFrame* >( pFirstFrame );

            SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( gProp.pSGlobalShell );
            if ( pWrtSh )
            {
                SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
                SwFrameControlsManager& rMngr = rEditWin.GetFrameControlsManager();

                if ( pFlowFrame && pFlowFrame->IsPageBreak( true ) )
                    rMngr.SetPageBreakControl( this );
                else
                    rMngr.RemoveControlsByType( PageBreak, this );
            }
        }
        SwLayoutFrame::PaintBreak( );
    }
}

void SwColumnFrame::PaintBreak( ) const
{
    if ( gProp.pSGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER  &&
         !gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() &&
         !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
         !gProp.pSGlobalShell->IsPreview() )
    {
        const SwFrame* pBodyFrame = Lower();
        while ( pBodyFrame && !pBodyFrame->IsBodyFrame() )
            pBodyFrame = pBodyFrame->GetNext();

        if ( pBodyFrame )
        {
            const SwContentFrame *pCnt = static_cast< const SwLayoutFrame* >( pBodyFrame )->ContainsContent();
            if ( pCnt && pCnt->IsColBreak( true ) )
            {
                // Paint the break only if:
                //    * Not in header footer edition, to avoid conflicts with the
                //      header/footer marker
                //    * Non-printing characters are shown, as this is more consistent
                //      with other formatting marks
                if ( !gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Header ) &&
                     !gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Footer ) &&
                      gProp.pSGlobalShell->GetViewOptions()->IsLineBreak() )
                {
                    SwRect aRect( pCnt->Prt() );
                    aRect.Pos() += pCnt->Frame().Pos();

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

                    drawinglayer::primitive2d::Primitive2DContainer aSeq =
                        lcl_CreateDashedIndicatorPrimitive( aStart, aEnd, aLineColor );
                    aSeq.resize( aSeq.size( ) + 1 );

                    // Add the text above
                    OUString aBreakText = SW_RESSTR(STR_COLUMN_BREAK);

                    basegfx::B2DVector aFontSize;
                    OutputDevice* pOut = gProp.pSGlobalShell->GetOut();
                    vcl::Font aFont = pOut->GetSettings().GetStyleSettings().GetToolFont();
                    aFont.SetFontHeight( 8 * 20 );
                    pOut->SetFont( aFont );
                    drawinglayer::attribute::FontAttribute aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(
                            aFontSize, aFont, false, false );

                    Rectangle aTextRect;
                    pOut->GetTextBoundRect( aTextRect, aBreakText );
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
                    aSeq[ aSeq.size() - 1 ] = drawinglayer::primitive2d::Primitive2DReference( pText );

                    ProcessPrimitives( aSeq );
                }
            }
        }
    }
}

void SwLayoutFrame::PaintBreak( ) const
{
    const SwFrame* pFrame = Lower();
    while ( pFrame )
    {
        if ( pFrame->IsLayoutFrame() )
            static_cast< const SwLayoutFrame*>( pFrame )->PaintBreak( );
        pFrame = pFrame->GetNext();
    }
}

void SwPageFrame::PaintDecorators( ) const
{
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( gProp.pSGlobalShell );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();

        const SwLayoutFrame* pBody = FindBodyCont();
        if ( pBody )
        {
            SwRect aBodyRect( pBody->Frame() );

            if ( gProp.pSGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER &&
                 !gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() &&
                 !gProp.pSGlobalShell->IsPreview() &&
                 !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
                 !gProp.pSGlobalShell->GetViewOptions()->getBrowseMode() &&
                 ( gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Header ) ||
                   gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Footer ) ) )
            {
                bool bRtl = AllSettings::GetLayoutRTL();
                const SwRect& rVisArea = gProp.pSGlobalShell->VisArea();
                long nXOff = std::min( aBodyRect.Right(), rVisArea.Right() );
                if ( bRtl )
                    nXOff = std::max( aBodyRect.Left(), rVisArea.Left() );

                // Header
                if ( gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Header ) )
                {
                    const SwFrame* pHeaderFrame = Lower();
                    if ( !pHeaderFrame->IsHeaderFrame() )
                        pHeaderFrame = nullptr;

                    long nHeaderYOff = aBodyRect.Top();
                    Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nHeaderYOff ) );
                    rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, Header, nOutputOff );
                }

                // Footer
                if ( gProp.pSGlobalShell->IsShowHeaderFooterSeparator( Footer ) )
                {
                    const SwFrame* pFootnoteContFrame = Lower();
                    while ( pFootnoteContFrame )
                    {
                        if ( pFootnoteContFrame->IsFootnoteContFrame() )
                            aBodyRect.AddBottom( pFootnoteContFrame->Frame().Bottom() - aBodyRect.Bottom() );
                        pFootnoteContFrame = pFootnoteContFrame->GetNext();
                    }

                    long nFooterYOff = aBodyRect.Bottom();
                    Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nFooterYOff ) );
                    rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, Footer, nOutputOff );
                }
            }
        }
    }
}

/**
 * For feature #99657#
 *
 * OD 12.08.2002
 * determines, if background of fly frame has to be drawn transparent
 * declaration found in /core/inc/flyfrm.cxx
 *
 * OD 08.10.2002 #103898# - If the background of the fly frame itself is not
 * transparent and the background is inherited from its parent/grandparent,
 * the background brush, used for drawing, has to be investigated for transparency.
 *
 * @return true, if background is transparent drawn
*/
bool SwFlyFrame::IsBackgroundTransparent() const
{
    bool bBackgroundTransparent = GetFormat()->IsBackgroundTransparent();
    if ( !bBackgroundTransparent &&
         static_cast<const SwFlyFrameFormat*>(GetFormat())->IsBackgroundBrushInherited() )
    {
        const SvxBrushItem* pBackgrdBrush = nullptr;
        const Color* pSectionTOXColor = nullptr;
        SwRect aDummyRect;
        //UUUU
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

        if ( GetBackgroundBrush( aFillAttributes, pBackgrdBrush, pSectionTOXColor, aDummyRect, false) )
        {
            if ( pSectionTOXColor &&
                 (pSectionTOXColor->GetTransparency() != 0) &&
                 (pSectionTOXColor->GetColor() != COL_TRANSPARENT) )
            {
                bBackgroundTransparent = true;
            }
            else if(aFillAttributes.get() && aFillAttributes->isUsed()) //UUUU
            {
                bBackgroundTransparent = aFillAttributes->isTransparent();
            }
            else if ( pBackgrdBrush )
            {
                if ( (pBackgrdBrush->GetColor().GetTransparency() != 0) &&
                     (pBackgrdBrush->GetColor() != COL_TRANSPARENT) )
                {
                    bBackgroundTransparent = true;
                }
                else
                {
                    const GraphicObject *pTmpGrf =
                            static_cast<const GraphicObject*>(pBackgrdBrush->GetGraphicObject());
                    if ( (pTmpGrf) &&
                         (pTmpGrf->GetAttr().GetTransparency() != 0)
                       )
                    {
                        bBackgroundTransparent = true;
                    }
                }
            }
        }
    }

    return bBackgroundTransparent;
};

bool SwFlyFrame::IsPaint( SdrObject *pObj, const SwViewShell *pSh )
{
    SdrObjUserCall *pUserCall;

    if ( nullptr == ( pUserCall = GetUserCall(pObj) ) )
        return true;

    //Attribute dependent, don't paint for printer or Preview
    bool bPaint =  gProp.pSFlyOnlyDraw ||
                       static_cast<SwContact*>(pUserCall)->GetFormat()->GetPrint().GetValue();
    if ( !bPaint )
        bPaint = pSh->GetWin() && !pSh->IsPreview();

    if ( bPaint )
    {
        //The paint may be prevented by the superior Flys.
        SwFrame *pAnch = nullptr;
        if ( dynamic_cast< const SwFlyDrawObj *>( pObj ) !=  nullptr ) // i#117962#
        {
            bPaint = false;
        }
        if ( dynamic_cast< const SwVirtFlyDrawObj *>( pObj ) !=  nullptr )
        {
            SwFlyFrame *pFly = static_cast<SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();
            if ( gProp.pSFlyOnlyDraw && gProp.pSFlyOnlyDraw == pFly )
                return true;

            //Try to avoid displaying the intermediate stage, Flys which don't
            //overlap with the page on which they are anchored won't be
            //painted.
            //HACK: exception: printing of frames in tables, those can overlap
            //a page once in a while when dealing with oversized tables (HTML).
            SwPageFrame *pPage = pFly->FindPageFrame();
            if ( pPage && pPage->Frame().IsOver( pFly->Frame() ) )
            {
                    pAnch = pFly->AnchorFrame();
            }

        }
        else
        {
            // OD 13.10.2003 #i19919# - consider 'virtual' drawing objects
            // OD 2004-03-29 #i26791#
            SwDrawContact* pDrawContact = dynamic_cast<SwDrawContact*>(pUserCall);
            pAnch = pDrawContact ? pDrawContact->GetAnchorFrame(pObj) : nullptr;
            if ( pAnch )
            {
                if ( !pAnch->GetValidPosFlag() )
                    pAnch = nullptr;
                else if ( sal_IntPtr(pSh->GetOut()) == sal_IntPtr(pSh->getIDocumentDeviceAccess().getPrinter( false )))
                {
                    //HACK: we have to omit some of the objects for printing,
                    //otherwise they would be printed twice.
                    //The objects should get printed if the TableHack is active
                    //right now. Afterwards they must not be printed if the
                    //page over which they float position wise gets printed.
                    const SwPageFrame *pPage = pAnch->FindPageFrame();
                    if ( !pPage->Frame().IsOver( pObj->GetCurrentBoundRect() ) )
                        pAnch = nullptr;
                }
            }
            else
            {
                // OD 02.07.2003 #108784# - debug assert
                if ( dynamic_cast< const SdrObjGroup *>( pObj ) ==  nullptr )
                {
                    OSL_FAIL( "<SwFlyFrame::IsPaint(..)> - paint of drawing object without anchor frame!?" );
                }
            }
        }
        if ( pAnch )
        {
            if ( pAnch->IsInFly() )
                bPaint = SwFlyFrame::IsPaint( pAnch->FindFlyFrame()->GetVirtDrawObj(),
                                            pSh );
            else if ( gProp.pSFlyOnlyDraw )
                bPaint = false;
        }
        else
            bPaint = false;
    }
    return bPaint;
}

void SwCellFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    if ( GetLayoutRowSpan() >= 1 )
        SwLayoutFrame::Paint( rRenderContext, rRect );
}

struct BorderLinesGuard
{
    explicit BorderLinesGuard() : m_pBorderLines(gProp.pBLines)
    {
        gProp.pBLines = new BorderLines;
    }
    ~BorderLinesGuard()
    {
        delete gProp.pBLines;
        gProp.pBLines = m_pBorderLines;
    }
private:
    BorderLines *const m_pBorderLines;
};

void SwFlyFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    //optimize thumbnail generation and store procedure to improve odt saving performance, #i120030#
    SwViewShell *pShell = getRootFrame()->GetCurrShell();
    if (pShell && pShell->GetDoc() && pShell->GetDoc()->GetDocShell())
    {
        bool bInGenerateThumbnail = pShell->GetDoc()->GetDocShell()->IsInGenerateAndStoreThumbnail();
        if (bInGenerateThumbnail)
        {
            SwRect aVisRect = pShell->VisArea();
            if (!aVisRect.IsOver(Frame()))
                return;
        }
    }

    //because of the overlapping of frames and drawing objects the flys have to
    //paint their borders (and those of the internal ones) directly.
    //e.g. #33066#
    gProp.pSLines->LockLines(true);
    BorderLinesGuard blg; // this should not paint borders added from PaintBaBo

    SwRect aRect( rRect );
    aRect.Intersection_( Frame() );

    rRenderContext.Push( PushFlags::CLIPREGION );
    rRenderContext.SetClipRegion();
    const SwPageFrame* pPage = FindPageFrame();

    const SwNoTextFrame *pNoText = Lower() && Lower()->IsNoTextFrame()
                                                ? static_cast<const SwNoTextFrame*>(Lower()) : nullptr;

    bool bIsChart = false; //#i102950# don't paint additional borders for charts
    //check whether we have a chart
    if(pNoText)
    {
        const SwNoTextNode* pNoTNd = dynamic_cast<const SwNoTextNode*>(pNoText->GetNode());
        if( pNoTNd )
        {
            SwOLENode* pOLENd = const_cast<SwOLENode*>(pNoTNd->GetOLENode());
            if( pOLENd && pOLENd->GetOLEObj().GetObject().IsChart() )
                bIsChart = true;
        }
    }

    {
        bool bContour = GetFormat()->GetSurround().IsContour();
        tools::PolyPolygon aPoly;
        if ( bContour )
        {
            // OD 16.04.2003 #i13147# - add 2nd parameter with value <true>
            // to indicate that method is called for paint in order to avoid
            // load of the intrinsic graphic.
            bContour = GetContour( aPoly, true );
        }

        // #i47804# - distinguish complete background paint
        // and margin paint.
        // paint complete background for Writer text fly frames
        bool bPaintCompleteBack( !pNoText );
        // paint complete background for transparent graphic and contour,
        // if own background color exists.
        const bool bIsGraphicTransparent = pNoText && pNoText->IsTransparent();
        if ( !bPaintCompleteBack &&
             ( bIsGraphicTransparent|| bContour ) )
        {
            const SwFrameFormat* pSwFrameFormat = dynamic_cast< const SwFrameFormat* >(GetFormat());

            if (pSwFrameFormat && pSwFrameFormat->supportsFullDrawingLayerFillAttributeSet())
            {
                //UUUU check for transparency
                const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(pSwFrameFormat->getSdrAllFillAttributesHelper());

                // check if the new fill attributes are used
                if(aFillAttributes.get() && aFillAttributes->isUsed())
                {
                    bPaintCompleteBack = true;
                }
            }
            else
            {
                SvxBrushItem aBack = GetFormat()->makeBackgroundBrushItem();
                // OD 07.08.2002 #99657# #GetTransChg#
                //     to determine, if background has to be painted, by checking, if
                //     background color is not COL_TRANSPARENT ("no fill"/"auto fill")
                //     or a background graphic exists.
                bPaintCompleteBack = !(aBack.GetColor() == COL_TRANSPARENT) ||
                                     aBack.GetGraphicPos() != GPOS_NONE;
            }
        }
        // paint of margin needed.
        const bool bPaintMarginOnly( !bPaintCompleteBack &&
                                     Prt().SSize() != Frame().SSize() );

        // #i47804# - paint background of parent fly frame
        // for transparent graphics in layer Hell, if parent fly frame isn't
        // in layer Hell. It's only painted the intersection between the
        // parent fly frame area and the paint area <aRect>
        const IDocumentDrawModelAccess& rIDDMA = GetFormat()->getIDocumentDrawModelAccess();

        if (bIsGraphicTransparent &&
            GetFormat()->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS) &&
            GetVirtDrawObj()->GetLayer() == rIDDMA.GetHellId() &&
            GetAnchorFrame()->FindFlyFrame() )
        {
            const SwFlyFrame* pParentFlyFrame = GetAnchorFrame()->FindFlyFrame();
            if ( pParentFlyFrame->GetDrawObj()->GetLayer() !=
                                            rIDDMA.GetHellId() )
            {
                SwFlyFrame* pOldRet = gProp.pSRetoucheFly2;
                gProp.pSRetoucheFly2 = const_cast<SwFlyFrame*>(this);

                SwBorderAttrAccess aAccess( SwFrame::GetCache(), pParentFlyFrame );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                SwRect aPaintRect( aRect );
                aPaintRect.Intersection_( pParentFlyFrame->Frame() );
                pParentFlyFrame->PaintBackground( aPaintRect, pPage, rAttrs );

                gProp.pSRetoucheFly2 = pOldRet;
            }
        }

        if ( bPaintCompleteBack || bPaintMarginOnly )
        {
            //#24926# JP 01.02.96, PaintBaBo is here partially so PaintBorder
            //receives the original Rect but PaintBackground only the limited
            //one.

            // OD 2004-04-23 #116347#
            rRenderContext.Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            rRenderContext.SetLineColor();

            pPage = FindPageFrame();

            SwBorderAttrAccess aAccess( SwFrame::GetCache(), static_cast<SwFrame const *>(this) );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // paint background
            {
                SwRegionRects aRegion( aRect );
                // #i80822#
                // suppress painting of background in printing area for
                // non-transparent graphics.
                if ( bPaintMarginOnly ||
                     ( pNoText && !bIsGraphicTransparent ) )
                {
                    //What we actually want to paint is the small stripe between
                    //PrtArea and outer border.
                    SwRect aTmp( Prt() ); aTmp += Frame().Pos();
                    aRegion -= aTmp;
                }
                if ( bContour )
                {
                    rRenderContext.Push();
                    // #i80822#
                    // apply clip region under the same conditions, which are
                    // used in <SwNoTextFrame::Paint(..)> to set the clip region
                    // for painting the graphic/OLE. Thus, the clip region is
                    // also applied for the PDF export.
                    SwViewShell *pSh = getRootFrame()->GetCurrShell();
                    if ( !rRenderContext.GetConnectMetaFile() || !pSh || !pSh->GetWin() )
                    {
                        rRenderContext.SetClipRegion(vcl::Region(aPoly));
                    }
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        PaintBackground( aRegion[i], pPage, rAttrs, false, true );
                    rRenderContext.Pop();
                }
                else
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        PaintBackground( aRegion[i], pPage, rAttrs, false, true );
            }

            // OD 06.08.2002 #99657# - paint border before painting background
            // paint border
            PaintBorder( rRect, pPage, rAttrs );

            rRenderContext.Pop();
        }
    }

    // OD 19.12.2002 #106318# - fly frame will paint it's subsidiary lines and
    // the subsidiary lines of its lowers on its own, due to overlapping with
    // other fly frames or other objects.
    if( gProp.pSGlobalShell->GetWin()
        && !bIsChart ) //#i102950# don't paint additional borders for charts
    {
        bool bSubsLineRectsCreated;
        if ( gProp.pSSubsLines )
        {
            // Lock already existing subsidiary lines
            gProp.pSSubsLines->LockLines( true );
            bSubsLineRectsCreated = false;
        }
        else
        {
            // create new subsidiary lines
            gProp.pSSubsLines = new SwSubsRects;
            bSubsLineRectsCreated = true;
        }

        bool bSpecSubsLineRectsCreated;
        if ( gProp.pSSpecSubsLines )
        {
            // Lock already existing special subsidiary lines
            gProp.pSSpecSubsLines->LockLines( true );
            bSpecSubsLineRectsCreated = false;
        }
        else
        {
            // create new special subsidiary lines
            gProp.pSSpecSubsLines = new SwSubsRects;
            bSpecSubsLineRectsCreated = true;
        }
        // Add subsidiary lines of fly frame and its lowers
        RefreshLaySubsidiary( pPage, aRect );
        // paint subsidiary lines of fly frame and its lowers
        gProp.pSSpecSubsLines->PaintSubsidiary( &rRenderContext, nullptr, gProp );
        gProp.pSSubsLines->PaintSubsidiary( &rRenderContext, gProp.pSLines, gProp );
        if ( !bSubsLineRectsCreated )
            // unlock subsidiary lines
            gProp.pSSubsLines->LockLines( false );
        else
            // delete created subsidiary lines container
            DELETEZ( gProp.pSSubsLines );

        if ( !bSpecSubsLineRectsCreated )
            // unlock special subsidiary lines
            gProp.pSSpecSubsLines->LockLines( false );
        else
        {
            // delete created special subsidiary lines container
            DELETEZ( gProp.pSSpecSubsLines );
        }
    }

    SwLayoutFrame::Paint( rRenderContext, aRect );

    Validate();

    // OD 19.12.2002 #106318# - first paint lines added by fly frame paint
    // and then unlock other lines.
    gProp.pSLines->PaintLines( &rRenderContext, gProp );
    gProp.pSLines->LockLines( false );
    // have to paint frame borders added in heaven layer here...
    ProcessPrimitives(gProp.pBLines->GetBorderLines_Clear());

    rRenderContext.Pop();

    if ( gProp.pSProgress && pNoText )
        gProp.pSProgress->Reschedule();
}

void SwTabFrame::Paint(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    const SwViewOption* pViewOption = gProp.pSGlobalShell->GetViewOptions();
    if (pViewOption->IsTable())
    {
        // #i29550#
        if ( IsCollapsingBorders() )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), static_cast<SwFrame const *>(this) );
            const SwBorderAttrs &rAttrs = *aAccess.Get();

            // paint shadow
            if ( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
            {
                SwRect aRect;
                ::lcl_CalcBorderRect( aRect, this, rAttrs, true, gProp );
                PaintShadow( rRect, aRect, rAttrs );
            }

            SwTabFramePainter aHelper(*this);
            aHelper.PaintLines(rRenderContext, rRect);
        }

        SwLayoutFrame::Paint( rRenderContext, rRect );
    }
    // OD 10.01.2003 #i6467# - no light grey rectangle for page preview
    else if ( gProp.pSGlobalShell->GetWin() && !gProp.pSGlobalShell->IsPreview() )
    {
        // OD 10.01.2003 #i6467# - intersect output rectangle with table frame
        SwRect aTabRect( Prt() );
        aTabRect.Pos() += Frame().Pos();
        SwRect aTabOutRect( rRect );
        aTabOutRect.Intersection( aTabRect );
        SwViewOption::DrawRect( &rRenderContext, aTabOutRect, COL_LIGHTGRAY );
    }
    const_cast<SwTabFrame*>(this)->ResetComplete();
}

/**
 * Paint border shadow
 *
 * @param[in]       rRect       aligned rect to clip the result
 * @param[in,out]   rOutRect    full painting area as input
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
    const bool bLeft, const bool bRight,
    SwPaintProperties& properties)
{
    const long nWidth  = ::lcl_AlignWidth ( rShadow.GetWidth(), properties );
    const long nHeight = ::lcl_AlignHeight( rShadow.GetWidth(), properties );

    SwRects aRegion;
    SwRect aOut( rOutRect );

    switch ( rShadow.GetLocation() )
    {
        case SVX_SHADOW_BOTTOMRIGHT:
            {
                if ( bDrawFullShadowRectangle )
                {
                    // OD 06.08.2002 #99657# - draw full shadow rectangle
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
                    // OD 06.08.2002 #99657# - draw full shadow rectangle
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
                    // OD 06.08.2002 #99657# - draw full shadow rectangle
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
                    // OD 06.08.2002 #99657# - draw full shadow rectangle
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

    vcl::RenderContext *pOut = properties.pSGlobalShell->GetOut();

    DrawModeFlags nOldDrawMode = pOut->GetDrawMode();
    Color aShadowColor( rShadow.GetColor().GetRGBColor() );
    if( !aRegion.empty() && properties.pSGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        // In high contrast mode, the output device has already set the
        // DrawModeFlags::SettingsFill flag. This causes the SetFillColor function
        // to ignore the setting of a new color. Therefore we have to reset
        // the drawing mode
        pOut->SetDrawMode( DrawModeFlags::Default );
        aShadowColor = SwViewOption::GetFontColor();
    }

    if ( pOut->GetFillColor() != aShadowColor )
        pOut->SetFillColor( aShadowColor );

    pOut->SetLineColor();

    pOut->SetDrawMode( nOldDrawMode );

    for (SwRect & rOut : aRegion)
    {
        aOut = rOut;
        if ( rRect.IsOver( aOut ) && aOut.Height() > 0 && aOut.Width() > 0 )
        {
            aOut.Intersection_( rRect );
            pOut->DrawRect( aOut.SVRect() );
        }
    }
}

/**
 * Paints a shadow if the format requests so.
 *
 * The shadow is always painted on the outer edge of the OutRect.
 * If needed, the OutRect is shrunk so the painting of the border can be
 * done on it.
 *
 * @note: draw full shadow rectangle for frames with transparent drawn backgrounds (OD 23.08.2002 #99657#)
 */
void SwFrame::PaintShadow( const SwRect& rRect, SwRect& rOutRect,
                         const SwBorderAttrs &rAttrs ) const
{
    SvxShadowItem rShadow = rAttrs.GetShadow();

    const bool bCnt    = IsContentFrame();
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

    // OD 23.08.2002 #99657# - determine, if full shadow rectangle have to
    //     be drawn or only two shadow rectangles beside the frame.
    //     draw full shadow rectangle, if frame background is drawn transparent.
    //     Status Quo:
    //         SwLayoutFrame can have transparent drawn backgrounds. Thus,
    //         "asked" their frame format.
    const bool bDrawFullShadowRectangle =
            ( IsLayoutFrame() &&
              (static_cast<const SwLayoutFrame*>(this))->GetFormat()->IsBackgroundTransparent()
            );

    SWRECTFN( this );
    ::lcl_ExtendLeftAndRight( rOutRect, *(this), rAttrs, fnRect );

    lcl_PaintShadow(rRect, rOutRect, rShadow, bDrawFullShadowRectangle, bTop, bBottom, true, true, gProp);
}

void SwFrame::PaintBorderLine( const SwRect& rRect,
                             const SwRect& rOutRect,
                             const SwPageFrame * pPage,
                             const Color *pColor,
                             const SvxBorderStyle nStyle ) const
{
    if ( !rOutRect.IsOver( rRect ) )
        return;

    SwRect aOut( rOutRect );
    aOut.Intersection_( rRect );

    const SwTabFrame *pTab = IsCellFrame() ? FindTabFrame() : nullptr;
    sal_uInt8 nSubCol = ( IsCellFrame() || IsRowFrame() ) ? SUBCOL_TAB :
                   ( IsInSct() ? SUBCOL_SECT :
                   ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );
    if( pColor && gProp.pSGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pColor = &SwViewOption::GetFontColor();
    }

    if (pPage->GetSortedObjs() &&
        pPage->GetFormat()->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS))
    {
        SwRegionRects aRegion( aOut, 4 );
        ::lcl_SubtractFlys( this, pPage, aOut, aRegion, gProp );
        for ( size_t i = 0; i < aRegion.size(); ++i )
            gProp.pSLines->AddLineRect( aRegion[i], pColor, nStyle, pTab, nSubCol, gProp );
    }
    else
        gProp.pSLines->AddLineRect( aOut, pColor, nStyle, pTab, nSubCol, gProp );
}

/**
 * @note Only all lines once or all lines twice!
 *
 * OD 29.04.2003 #107169# - method called for left and right border rectangles.
 * For a printer output device perform adjustment for non-overlapping top and
 * bottom border rectangles. Thus, add parameter <_bPrtOutputDev> to indicate
 * printer output device.
 * NOTE: For printer output device left/right border rectangle <_iorRect>
 *        has to be already non-overlapping the outer top/bottom border rectangle.
 */
static void lcl_SubTopBottom( SwRect&              _iorRect,
                                   const SvxBoxItem&    _rBox,
                                   const SwBorderAttrs& _rAttrs,
                                   const SwFrame&         _rFrame,
                                   const SwRectFn&      _rRectFn,
                                   const bool       _bPrtOutputDev,
                                   SwPaintProperties& properties )
{
    const bool bCnt = _rFrame.IsContentFrame();
    if ( _rBox.GetTop() && _rBox.GetTop()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetTopLine( _rFrame ) )
       )
    {
        // subtract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetTop()->GetDistance(), properties );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner top line is a hair line (width = 1)
        bool bIsInnerTopLineHairline = false;
        if ( !_bPrtOutputDev )
        {
            // additionally subtract width of top outer line
            // --> left/right inner/outer line doesn't overlap top outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetOutWidth(), properties );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally subtract width of top inner line
            // --> left/right inner/outer line doesn't overlap top inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetTop()->GetInWidth(), properties );
            bIsInnerTopLineHairline = _rBox.GetTop()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnSubTop)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border top, if inner top line
        // is a hair line
        if ( bIsInnerTopLineHairline )
        {
            if ( _rFrame.IsVertical() )
            {
                // right of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Right(), 0 );
                Point aRefPt( aCompPt.X() + 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(properties.pSGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          true, -1 );
                _iorRect.Right( aCompPt.X() );
            }
            else
            {
                // top of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Top() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() - 1 );
                lcl_CompPxPosAndAdjustPos( *(properties.pSGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          false, +1 );
                _iorRect.Top( aCompPt.Y() );
            }
        }
    }

    if ( _rBox.GetBottom() && _rBox.GetBottom()->GetInWidth() &&
         ( !bCnt || _rAttrs.GetBottomLine( _rFrame ) )
       )
    {
        // subtract distance between outer and inner line.
        SwTwips nDist = ::lcl_MinHeightDist( _rBox.GetBottom()->GetDistance(), properties );
        // OD 19.05.2003 #109667# - non-overlapping border rectangles:
        // adjust x-/y-position, if inner bottom line is a hair line (width = 1)
        bool bIsInnerBottomLineHairline = false;
        if ( !_bPrtOutputDev )
        {
            // additionally subtract width of bottom outer line
            // --> left/right inner/outer line doesn't overlap bottom outer line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetOutWidth(), properties );
        }
        else
        {
            // OD 29.04.2003 #107169# - additionally subtract width of bottom inner line
            // --> left/right inner/outer line doesn't overlap bottom inner line.
            nDist += ::lcl_AlignHeight( _rBox.GetBottom()->GetInWidth(), properties );
            bIsInnerBottomLineHairline = _rBox.GetBottom()->GetInWidth() == 1;
        }
        (_iorRect.*_rRectFn->fnAddBottom)( -nDist );
        // OD 19.05.2003 #109667# - adjust calculated border bottom, if inner
        // bottom line is a hair line.
        if ( bIsInnerBottomLineHairline )
        {
            if ( _rFrame.IsVertical() )
            {
                // left of border rectangle has to be checked and adjusted
                Point aCompPt( _iorRect.Left(), 0 );
                Point aRefPt( aCompPt.X() - 1, aCompPt.Y() );
                lcl_CompPxPosAndAdjustPos( *(properties.pSGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          true, +1 );
                _iorRect.Left( aCompPt.X() );
            }
            else
            {
                // bottom of border rectangle has to be checked and adjusted
                Point aCompPt( 0, _iorRect.Bottom() );
                Point aRefPt( aCompPt.X(), aCompPt.Y() + 1 );
                lcl_CompPxPosAndAdjustPos( *(properties.pSGlobalShell->GetOut()),
                                          aRefPt, aCompPt,
                                          false, -1 );
                _iorRect.Bottom( aCompPt.Y() );
            }
        }
    }
}

static sal_uInt16 lcl_GetLineWidth( const SvxBorderLine* pLine )
{
    if ( pLine != nullptr )
        return pLine->GetScaledWidth();

    return 0;
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

static void lcl_MakeBorderLine(SwRect const& rRect,
        bool const isVerticalInModel,
        bool const isLeftOrTopBorderInModel,
        bool const isVertical,
        SvxBorderLine const& rBorder,
        SvxBorderLine const*const pLeftOrTopNeighbour,
        SvxBorderLine const*const pRightOrBottomNeighbour,
        SwPaintProperties& properties)
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
                lcl_AlignHeight(lcl_GetLineWidth(pStartNeighbour), properties)/2.0);
        aEnd.setX(fStartX);
        aEnd.setY(rRect.Bottom() -
                lcl_AlignHeight(lcl_GetLineWidth(pEndNeighbour), properties)/2.0);
    }
    else
    {   // fdo#38635: always from outer edge
        double const fStartY( (isLeftOrTopBorder)
                ? rRect.Top()    + (rRect.Height() / 2.0)
                : rRect.Bottom() - (rRect.Height() / 2.0));
        aStart.setX(rRect.Left() +
                lcl_AlignWidth(lcl_GetLineWidth(pStartNeighbour), properties)/2.0);
        aStart.setY(fStartY);
        aEnd.setX(rRect.Right() -
                lcl_AlignWidth(lcl_GetLineWidth(pEndNeighbour), properties)/2.0);
        aEnd.setY(fStartY);
    }

    // When rendering to very small (virtual) devices, like when producing
    // page thumbnails in a mobile device app, the line geometry can end up
    // bogus (negative width or height), so just ignore such border lines.
    // Otherwise we will run into assertions later in lcl_TryMergeBorderLine()
    // at least.
    if (aEnd.getX() < aStart.getX() ||
        aEnd.getY() < aStart.getY())
        return;

    double const nExtentLeftStart = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(pStartNeighbour, nullptr)
        :   lcl_GetExtent(nullptr, pStartNeighbour);
    double const nExtentLeftEnd = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(pEndNeighbour, nullptr)
        :   lcl_GetExtent(nullptr, pEndNeighbour);
    double const nExtentRightStart = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(nullptr, pStartNeighbour)
        :   lcl_GetExtent(pStartNeighbour, nullptr);
    double const nExtentRightEnd = (isLeftOrTopBorder == isVertical)
        ?   lcl_GetExtent(nullptr, pEndNeighbour)
        :   lcl_GetExtent(pEndNeighbour, nullptr);

    double const nLeftWidth = rBorder.GetOutWidth();
    double const nRightWidth = rBorder.GetInWidth();
    Color const aLeftColor = rBorder.GetColorOut(isLeftOrTopBorder);
    Color const aRightColor = rBorder.GetColorIn(isLeftOrTopBorder);

    css::uno::Reference<BorderLinePrimitive2D> const xLine =
        new BorderLinePrimitive2D(
            aStart, aEnd, nLeftWidth, rBorder.GetDistance(), nRightWidth,
            nExtentLeftStart, nExtentLeftEnd,
            nExtentRightStart, nExtentRightEnd,
            aLeftColor.getBColor(), aRightColor.getBColor(),
            rBorder.GetColorGap().getBColor(), rBorder.HasGapColor(),
            rBorder.GetBorderLineStyle() );
    properties.pBLines->AddBorderLine(xLine, properties);
}

/**
 * OD 19.05.2003 #109667# - merge <lcl_PaintLeftLine> and <lcl_PaintRightLine>
 * into new method <lcl_PaintLeftRightLine(..)>
 */
static void lcl_PaintLeftRightLine( const bool         _bLeft,
                             const SwFrame&           _rFrame,
                             const SwPageFrame&       /*_rPage*/,
                             const SwRect&          _rOutRect,
                             const SwRect&          /*_rRect*/,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn,
                             SwPaintProperties& properties)
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const bool bR2L = _rFrame.IsCellFrame() && _rFrame.IsRightToLeft();
    const SvxBorderLine* pLeftRightBorder = nullptr;
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
        (aRect.*_rRectFn->fnAddRight)( ::lcl_AlignWidth( lcl_GetLineWidth( pLeftRightBorder ), properties ) -
                                       (aRect.*_rRectFn->fnGetWidth)() );

        // Shift the left border to the left.
        Point aCurPos = aRect.Pos();
        const sal_uInt16 nOffset = pLeftRightBorder->GetDistance();
        aCurPos.X() -= nOffset;
        aCurPos.Y() -= nOffset;
        aRect.Pos(aCurPos);
        Size aCurSize = aRect.SSize();
        aCurSize.Height() += nOffset * 2;
        aRect.SSize(aCurSize);
    }
    else
    {
        (aRect.*_rRectFn->fnSubLeft)( ::lcl_AlignWidth( lcl_GetLineWidth( pLeftRightBorder ), properties ) -
                                      (aRect.*_rRectFn->fnGetWidth)() );
    }

    if ( _rFrame.IsContentFrame() )
    {
        ::lcl_ExtendLeftAndRight( aRect, _rFrame, _rAttrs, _rRectFn );

        // No Top / bottom borders for joint borders
        if ( _rAttrs.JoinedWithPrev( _rFrame ) ) pTopBorder = nullptr;
        if ( _rAttrs.JoinedWithNext( _rFrame ) ) pBottomBorder = nullptr;
    }

    if ( !pLeftRightBorder->GetInWidth() )
    {
        // OD 06.05.2003 #107169# - init boolean indicating printer output device.
        const bool bPrtOutputDev =
                ( OUTDEV_PRINTER == properties.pSGlobalShell->GetOut()->GetOutDevType() );

        // OD 06.05.2003 #107169# - add 6th parameter
        ::lcl_SubTopBottom( aRect, rBox, _rAttrs, _rFrame, _rRectFn, bPrtOutputDev, properties);
    }

    if ( lcl_GetLineWidth( pLeftRightBorder ) > 0 )
    {
        lcl_MakeBorderLine(
            aRect, true, _bLeft, aRect.Height() > aRect.Width(),
            *pLeftRightBorder, pTopBorder, pBottomBorder, properties);
    }
}

/**
 * OD 19.05.2003 #109667# - merge <lcl_PaintTopLine> and <lcl_PaintBottomLine>
 * into <lcl_PaintTopLine>
 */
static void lcl_PaintTopBottomLine( const bool         _bTop,
                             const SwFrame&           ,
                             const SwPageFrame&       /*_rPage*/,
                             const SwRect&          _rOutRect,
                             const SwRect&          /*_rRect*/,
                             const SwBorderAttrs&   _rAttrs,
                             const SwRectFn&        _rRectFn,
                             SwPaintProperties& properties)
{
    const SvxBoxItem& rBox = _rAttrs.GetBox();
    const SvxBorderLine* pTopBottomBorder = nullptr;

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
        (aRect.*_rRectFn->fnAddBottom)( ::lcl_AlignHeight( lcl_GetLineWidth( pTopBottomBorder ), properties ) -
                                        (aRect.*_rRectFn->fnGetHeight)() );

        // Push the top border up a bit.
        const sal_uInt16 nOffset = pTopBottomBorder->GetDistance();
        Point aCurPos = aRect.Pos();
        aCurPos.X() -= nOffset;
        aCurPos.Y() -= nOffset;
        aRect.Pos(aCurPos);
        Size aCurSize = aRect.SSize();
        aCurSize.Width() += nOffset * 2;
        aRect.SSize(aCurSize);
    }
    else
    {
        (aRect.*_rRectFn->fnSubTop)( ::lcl_AlignHeight( lcl_GetLineWidth( pTopBottomBorder ), properties ) -
                                     (aRect.*_rRectFn->fnGetHeight)() );
    }

    if ( lcl_GetLineWidth( pTopBottomBorder ) > 0 )
    {
        lcl_MakeBorderLine(
            aRect, false, _bTop, aRect.Height() > aRect.Width(),
            *pTopBottomBorder, rBox.GetLeft(), rBox.GetRight(), properties);
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
    SwAlignRect(aAlignedRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut());

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
                             false, bTop, bBottom, bLeft, bRight, gProp);
        }
    }

    // Init borders, after this initialization top, bottom, right and left means the
    // absolute position
    boost::optional<editeng::SvxBorderLine> aTopBorder =
        (bTop ? rFont.GetAbsTopBorder(bVerticalLayout) : boost::none);
    boost::optional<editeng::SvxBorderLine> aBottomBorder =
        (bBottom ? rFont.GetAbsBottomBorder(bVerticalLayout) : boost::none);
    boost::optional<editeng::SvxBorderLine> aLeftBorder =
        (bLeft ? rFont.GetAbsLeftBorder(bVerticalLayout) : boost::none);
    boost::optional<editeng::SvxBorderLine> aRightBorder =
        (bRight ? rFont.GetAbsRightBorder(bVerticalLayout) : boost::none);

    if( aTopBorder )
    {
        const sal_uInt16 nOffset = aTopBorder->GetDistance();

        Point aLeftTop(
            aAlignedRect.Left() - nOffset,
            aAlignedRect.Top() - nOffset);
        Point aRightBottom(
            aAlignedRect.Right() + nOffset,
            aAlignedRect.Top() - nOffset + aTopBorder->GetScaledWidth());

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            false, true, false,
            aTopBorder.get(),
            aLeftBorder.get_ptr(),
            aRightBorder.get_ptr(),
            gProp);
    }

    if( aBottomBorder )
    {
        if( aBottomBorder->isDouble() )
            aBottomBorder->SetMirrorWidths();

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
            aRightBorder.get_ptr(),
            gProp );
    }

    if( aLeftBorder )
    {
        const sal_uInt16 nOffset = aLeftBorder->GetDistance();

        Point aLeftTop(
            aAlignedRect.Left() - nOffset,
            aAlignedRect.Top() - nOffset);
        Point aRightBottom(
            aAlignedRect.Left() - nOffset + aLeftBorder->GetScaledWidth(),
            aAlignedRect.Bottom() + nOffset);

        lcl_MakeBorderLine(
            SwRect(aLeftTop, aRightBottom),
            true, true, true,
            aLeftBorder.get(),
            aTopBorder.get_ptr(),
            aBottomBorder.get_ptr(),
            gProp );
    }

    if( aRightBorder )
    {
        if( aRightBorder->isDouble() )
            aRightBorder->SetMirrorWidths();

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
            aBottomBorder.get_ptr(),
            gProp );
    }
}

/// #i15844#
static const SwFrame* lcl_HasNextCell( const SwFrame& rFrame )
{
    OSL_ENSURE( rFrame.IsCellFrame(),
            "lcl_HasNextCell( const SwFrame& rFrame ) should be called with SwCellFrame" );

    const SwFrame* pTmpFrame = &rFrame;
    do
    {
        if ( pTmpFrame->GetNext() )
            return pTmpFrame->GetNext();

        pTmpFrame = pTmpFrame->GetUpper()->GetUpper();
    }
    while ( pTmpFrame->IsCellFrame() );

    return nullptr;
}

/**
 * Determine cell frame, from which the border attributes
 * for paint of top/bottom border has to be used.
 *
 * OD 21.02.2003 #b4779636#, #107692#
 *
 * @param _pCellFrame
 * input parameter - constant pointer to cell frame for which the cell frame
 * for the border attributes has to be determined.
 *
 * @param _rCellBorderAttrs
 * input parameter - constant reference to the border attributes of cell frame
 * <_pCellFrame>.
 *
 * @param _bTop
 * input parameter - boolean, that controls, if cell frame for top border or
 * for bottom border has to be determined.
 *
 * @return constant pointer to cell frame, for which the border attributes has
 * to be used
 */
static const SwFrame* lcl_GetCellFrameForBorderAttrs( const SwFrame*         _pCellFrame,
                                           const SwBorderAttrs& _rCellBorderAttrs,
                                           const bool           _bTop )
{
    OSL_ENSURE( _pCellFrame, "No cell frame available, dying soon" );

    // determine, if cell frame is at bottom/top border of a table frame and
    // the table frame has/is a follow.
    const SwFrame* pTmpFrame = _pCellFrame;
    bool bCellAtBorder = true;
    bool bCellAtLeftBorder = !_pCellFrame->GetPrev();
    bool bCellAtRightBorder = !_pCellFrame->GetNext();
    while( !pTmpFrame->IsRowFrame() || !pTmpFrame->GetUpper()->IsTabFrame() )
    {
        pTmpFrame = pTmpFrame->GetUpper();
        if ( pTmpFrame->IsRowFrame() &&
             (_bTop ? pTmpFrame->GetPrev() : pTmpFrame->GetNext())
           )
        {
            bCellAtBorder = false;
        }
        if ( pTmpFrame->IsCellFrame() )
        {
            if ( pTmpFrame->GetPrev() )
            {
                bCellAtLeftBorder = false;
            }
            if ( pTmpFrame->GetNext() )
            {
                bCellAtRightBorder = false;
            }
        }
    }
    OSL_ENSURE( pTmpFrame && pTmpFrame->IsRowFrame(), "No RowFrame available" );

    const SwLayoutFrame* pParentRowFrame = static_cast<const SwLayoutFrame*>(pTmpFrame);
    const SwTabFrame* pParentTabFrame =
            static_cast<const SwTabFrame*>(pParentRowFrame->GetUpper());

    const bool bCellNeedsAttribute = bCellAtBorder &&
                                     ( _bTop ?
                                      // bCellInFirstRowWithMaster
                                       ( !pParentRowFrame->GetPrev() &&
                                         pParentTabFrame->IsFollow() &&
                                         0 == pParentTabFrame->GetTable()->GetRowsToRepeat() ) :
                                      // bCellInLastRowWithFollow
                                       ( !pParentRowFrame->GetNext() &&
                                         pParentTabFrame->GetFollow() )
                                     );

    const SwFrame* pRet = _pCellFrame;
    if ( bCellNeedsAttribute )
    {
        // determine, if cell frame has no borders inside the table.
        const SwFrame* pNextCell = nullptr;
        bool bNoBordersInside = false;

        if ( bCellAtLeftBorder && ( nullptr != ( pNextCell = lcl_HasNextCell( *_pCellFrame ) ) ) )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), pNextCell );
            const SwBorderAttrs &rBorderAttrs = *aAccess.Get();
            const SvxBoxItem& rBorderBox = rBorderAttrs.GetBox();
            bCellAtRightBorder = !lcl_HasNextCell( *pNextCell );
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrame->GetPrev() ) &&
                  !rBorderBox.GetLeft() &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrame->GetNext() );
        }
        else
        {
            const SvxBoxItem& rBorderBox = _rCellBorderAttrs.GetBox();
            bNoBordersInside =
                ( !rBorderBox.GetTop()    || !pParentRowFrame->GetPrev() ) &&
                ( !rBorderBox.GetLeft()   || bCellAtLeftBorder ) &&
                ( !rBorderBox.GetRight()  || bCellAtRightBorder ) &&
                ( !rBorderBox.GetBottom() || !pParentRowFrame->GetNext() );
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
                SwTabFrame* pMasterTabFrame = pParentTabFrame->FindMaster( true );
                // determine first row of complete table.
                const SwFrame* pFirstRow = pMasterTabFrame->GetLower();
                // return first cell in first row
                SwFrame* pLowerCell = const_cast<SwFrame*>(pFirstRow->GetLower());
                while ( !pLowerCell->IsCellFrame() ||
                        ( pLowerCell->GetLower() && pLowerCell->GetLower()->IsRowFrame() )
                      )
                {
                    pLowerCell = pLowerCell->GetLower();
                }
                OSL_ENSURE( pLowerCell && pLowerCell->IsCellFrame(), "No CellFrame available" );
                pRet = pLowerCell;
            }
            else if ( !_bTop && !_rCellBorderAttrs.GetBox().GetBottom() )
            {
                //-hack
                // Cell frame has no bottom border and no border inside the table,
                // but it is at the bottom border of a table frame, which has a follow.
                // Thus, use border attributes of cell frame in last row of complete table.
                // First, determine last table frame of complete table.
                SwTabFrame* pLastTabFrame = const_cast<SwTabFrame*>(pParentTabFrame->GetFollow());
                while ( pLastTabFrame->GetFollow() )
                {
                    pLastTabFrame = pLastTabFrame->GetFollow();
                }
                // determine last row of complete table.
                SwFrame* pLastRow = pLastTabFrame->GetLastLower();
                // return first bottom border cell in last row
                SwFrame* pLowerCell = pLastRow->GetLower();
                while ( !pLowerCell->IsCellFrame() ||
                        ( pLowerCell->GetLower() && pLowerCell->GetLower()->IsRowFrame() )
                      )
                {
                    if ( pLowerCell->IsRowFrame() )
                    {
                        while ( pLowerCell->GetNext() )
                        {
                            pLowerCell = pLowerCell->GetNext();
                        }
                    }
                    pLowerCell = pLowerCell->GetLower();
                }
                OSL_ENSURE( pLowerCell && pLowerCell->IsCellFrame(), "No CellFrame available" );
                pRet = pLowerCell;
            }
        }
    }

    return pRet;
}

drawinglayer::processor2d::BaseProcessor2D * SwFrame::CreateProcessor2D( ) const
{
    basegfx::B2DRange aViewRange;

    SdrPage *pDrawPage = getRootFrame()->GetCurrShell()->Imp()->GetPageView()->GetPage();
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos(
            basegfx::B2DHomMatrix(  ),
            getRootFrame()->GetCurrShell()->GetOut()->GetViewTransformation(),
            aViewRange,
            GetXDrawPageForSdrPage( pDrawPage ),
            0.0,
            uno::Sequence< beans::PropertyValue >() );

    return  drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *getRootFrame()->GetCurrShell()->GetOut(),
                    aNewViewInfos );
}

void SwFrame::ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DContainer& rSequence ) const
{
    drawinglayer::processor2d::BaseProcessor2D * pProcessor2D = CreateProcessor2D();

    if ( pProcessor2D )
    {
        pProcessor2D->process( rSequence );
        delete pProcessor2D;
    }
}

/// Paints shadows and borders
void SwFrame::PaintBorder( const SwRect& rRect, const SwPageFrame *pPage,
                         const SwBorderAttrs &rAttrs ) const
{
    // There's nothing (Row,Body,Footnote,Root,Column,NoText) need to do here
    if ((GetType() & (SwFrameType::NoTxt|SwFrameType::Row|SwFrameType::Body|SwFrameType::Ftn|SwFrameType::Column|SwFrameType::Root)))
        return;

    if (IsCellFrame() && !gProp.pSGlobalShell->GetViewOptions()->IsTable())
        return;

    // #i29550#
    if ( IsTabFrame() || IsCellFrame() || IsRowFrame() )
    {
        const SwTabFrame* pTabFrame = FindTabFrame();
        if ( pTabFrame->IsCollapsingBorders() )
            return;

        if ( pTabFrame->GetTable()->IsNewModel() && ( !IsCellFrame() || IsCoveredCell() ) )
            return;
    }

    const bool bLine = rAttrs.IsLine();
    const bool bShadow = rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE;

    // - flag to control,
    //-hack has to be used.
    const bool bb4779636HackActive = true;

    const SwFrame* pCellFrameForBottomBorderAttrs = nullptr;
    const SwFrame* pCellFrameForTopBorderAttrs = nullptr;
    bool         bFoundCellForTopOrBorderAttrs = false;
    if ( bb4779636HackActive && IsCellFrame() )
    {
        pCellFrameForBottomBorderAttrs = lcl_GetCellFrameForBorderAttrs( this, rAttrs, false );
        if ( pCellFrameForBottomBorderAttrs != this )
            bFoundCellForTopOrBorderAttrs = true;
        pCellFrameForTopBorderAttrs = lcl_GetCellFrameForBorderAttrs( this, rAttrs, true );
        if ( pCellFrameForTopBorderAttrs != this )
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
        aRect += Frame().Pos();
        ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
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
            if ( IsLayoutFrame() &&
                 static_cast<const SwLayoutFrame*>(this)->GetFormat()->IsBackgroundTransparent() )
            {
                 bDrawOnlyShadowForTransparentFrame = true;
            }
            else
            {
                return;
            }
        }

        if ( !pPage )
            pPage = FindPageFrame();

        ::lcl_CalcBorderRect( aRect, this, rAttrs, true, gProp );
        rAttrs.SetGetCacheLine( true );
        if ( bShadow )
            PaintShadow( rRect, aRect, rAttrs );
        // OD 27.09.2002 #103636# - suspend drawing of border
        // add condition < NOT bDrawOnlyShadowForTransparentFrame > - see above
        // - add condition <bFoundCellForTopOrBorderAttrs>
        //-hack.
        if ( ( bLine || bFoundCellForTopOrBorderAttrs ) &&
             !bDrawOnlyShadowForTransparentFrame )
        {
            const SwFrame* pDirRefFrame = IsCellFrame() ? FindTabFrame() : this;
            SWRECTFN( pDirRefFrame )
            ::lcl_PaintLeftRightLine ( true, *(this), *(pPage), aRect, rRect, rAttrs, fnRect, gProp);
            ::lcl_PaintLeftRightLine ( false, *(this), *(pPage), aRect, rRect, rAttrs, fnRect, gProp);
            if ( !IsContentFrame() || rAttrs.GetTopLine( *(this) ) )
            {
                // -
                //-hack
                // paint is found, paint its top border.
                if ( IsCellFrame() && pCellFrameForTopBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrame::GetCache(),
                                                pCellFrameForTopBorderAttrs );
                    const SwBorderAttrs &rTopAttrs = *aAccess.Get();
                    ::lcl_PaintTopBottomLine( true, *(this), *(pPage), aRect, rRect, rTopAttrs, fnRect, gProp);
                }
                else
                {
                    ::lcl_PaintTopBottomLine( true, *(this), *(pPage), aRect, rRect, rAttrs, fnRect, gProp );
                }
            }
            if ( !IsContentFrame() || rAttrs.GetBottomLine( *(this) ) )
            {
                // -
                //-hack
                // paint is found, paint its bottom border.
                if ( IsCellFrame() && pCellFrameForBottomBorderAttrs != this )
                {
                    SwBorderAttrAccess aAccess( SwFrame::GetCache(),
                                                pCellFrameForBottomBorderAttrs );
                    const SwBorderAttrs &rBottomAttrs = *aAccess.Get();
                    ::lcl_PaintTopBottomLine(false, *(this), *(pPage), aRect, rRect, rBottomAttrs, fnRect, gProp);
                }
                else
                {
                    ::lcl_PaintTopBottomLine(false, *(this), *(pPage), aRect, rRect, rAttrs, fnRect, gProp);
                }
            }
        }
        rAttrs.SetGetCacheLine( false );
    }
}

/**
 * Special implementation because of the footnote line
 *
 * Currently only the top frame needs to be taken into account
 * Other lines and shadows are set aside
 */
void SwFootnoteContFrame::PaintBorder( const SwRect& rRect, const SwPageFrame *pPage,
                                const SwBorderAttrs & ) const
{
    //If the rectangle is completely inside the PrtArea, no border needs to
    //be painted.
    SwRect aRect( Prt() );
    aRect.Pos() += Frame().Pos();
    if ( !aRect.IsInside( rRect ) )
        PaintLine( rRect, pPage );
}

/// Paint footnote lines.
void SwFootnoteContFrame::PaintLine( const SwRect& rRect,
                              const SwPageFrame *pPage ) const
{
    //The length of the line is derived from the percentual indication on the
    //PageDesc. The position is also stated on the PageDesc.
    //The pen can directly be taken from the PageDesc.

    if ( !pPage )
        pPage = FindPageFrame();
    const SwPageFootnoteInfo &rInf = pPage->GetPageDesc()->GetFootnoteInfo();

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
            OSL_ENSURE( false, "New adjustment for footnote lines?" );
    }
    SwTwips nLineWidth = rInf.GetLineWidth();
    const SwRect aLineRect = bVert ?
        SwRect( Point(Frame().Left()+Frame().Width()-rInf.GetTopDist()-nLineWidth,
                      nX), Size( nLineWidth, nWidth ) )
            : SwRect( Point( nX, Frame().Pos().Y() + rInf.GetTopDist() ),
                            Size( nWidth, rInf.GetLineWidth()));
    if ( aLineRect.HasArea() && rInf.GetLineStyle() != css::table::BorderLineStyle::NONE)
        PaintBorderLine( rRect, aLineRect , pPage, &rInf.GetLineColor(),
                rInf.GetLineStyle() );
}

/// Paints the separator line for inside columns
void SwLayoutFrame::PaintColLines( const SwRect &rRect, const SwFormatCol &rFormatCol,
                                 const SwPageFrame *pPage ) const
{
    const SwFrame *pCol = Lower();
    if ( !pCol || !pCol->IsColumnFrame() )
        return;

    SwRectFn fnRect = pCol->IsVertical() ? ( pCol->IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;

    SwRect aLineRect = Prt();
    aLineRect += Frame().Pos();

    SwTwips nTop = ((aLineRect.*fnRect->fnGetHeight)()*rFormatCol.GetLineHeight())
                   / 100 - (aLineRect.*fnRect->fnGetHeight)();
    SwTwips nBottom = 0;

    switch ( rFormatCol.GetLineAdj() )
    {
        case COLADJ_CENTER:
            nBottom = nTop / 2; nTop -= nBottom; break;
        case COLADJ_TOP:
            nBottom = nTop; nTop = 0; break;
        case COLADJ_BOTTOM:
            break;
        default:
            OSL_ENSURE( false, "New adjustment for column lines?" );
    }

    if( nTop )
        (aLineRect.*fnRect->fnSubTop)( nTop );
    if( nBottom )
        (aLineRect.*fnRect->fnAddBottom)( nBottom );

    SwTwips nPenHalf = rFormatCol.GetLineWidth();
    (aLineRect.*fnRect->fnSetWidth)( nPenHalf );
    nPenHalf /= 2;

    //We need to be a bit generous here, to not lose something.
    SwRect aRect( rRect );
    (aRect.*fnRect->fnSubLeft)( nPenHalf + gProp.nSPixelSzW );
    (aRect.*fnRect->fnAddRight)( nPenHalf + gProp.nSPixelSzW );
    SwRectGet fnGetX = IsRightToLeft() ? fnRect->fnGetLeft : fnRect->fnGetRight;
    while ( pCol->GetNext() )
    {
        (aLineRect.*fnRect->fnSetPosX)
            ( (pCol->Frame().*fnGetX)() - nPenHalf );
        if ( aRect.IsOver( aLineRect ) )
            PaintBorderLine( aRect, aLineRect , pPage, &rFormatCol.GetLineColor(),
                   rFormatCol.GetLineStyle() );
        pCol = pCol->GetNext();
    }
}

void SwPageFrame::PaintGrid( OutputDevice* pOut, SwRect &rRect ) const
{
    if( !m_bHasGrid || gProp.pSRetoucheFly || gProp.pSRetoucheFly2 )
        return;
    SwTextGridItem const*const pGrid(GetGridItem(this));
    if( pGrid && ( OUTDEV_PRINTER != pOut->GetOutDevType() ?
        pGrid->GetDisplayGrid() : pGrid->GetPrintGrid() ) )
    {
        const SwLayoutFrame* pBody = FindBodyCont();
        if( pBody )
        {
            SwRect aGrid( pBody->Prt() );
            aGrid += pBody->Frame().Pos();

            SwRect aInter( aGrid );
            aInter.Intersection( rRect );
            if( aInter.HasArea() )
            {
                bool bGrid = pGrid->GetRubyTextBelow();
                bool bCell = GRID_LINES_CHARS == pGrid->GetGridType();
                long nGrid = pGrid->GetBaseHeight();
                const SwDoc* pDoc = GetFormat()->GetDoc();
                long nGridWidth = GetGridWidth(*pGrid, *pDoc);
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

/**
 * Paint margin area of a page
 *
 * OD 20.11.2002 for #104598#:
 * implement paint of margin area; margin area will be painted for a
 * view shell with a window and if the document is not in online layout.
 *
 * @param _rOutputRect
 * input parameter - constant instance reference of the rectangle, for
 * which an output has to be generated.
 *
 * @param _pViewShell
 * input parameter - instance of the view shell, on which the output
 * has to be generated.
 */
void SwPageFrame::PaintMarginArea( const SwRect& _rOutputRect,
                                 SwViewShell* _pViewShell ) const
{
    if (  _pViewShell->GetWin() && !_pViewShell->GetViewOptions()->getBrowseMode() )
    {
        //UUUU Simplified paint with DrawingLayer FillStyle
        SwRect aPgRect = Frame();
        aPgRect.Intersection_( _rOutputRect );

        if(!aPgRect.IsEmpty())
        {
            OutputDevice *pOut = _pViewShell->GetOut();

            if(pOut->GetFillColor() != aGlobalRetoucheColor)
            {
                pOut->SetFillColor(aGlobalRetoucheColor);
            }

            pOut->DrawRect(aPgRect.SVRect());
        }
    }
}

const sal_Int8 SwPageFrame::mnShadowPxWidth = 9;

bool SwPageFrame::IsRightShadowNeeded() const
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bIsLTR = getRootFrame()->IsLeftToRightViewLayout();

    // We paint the right shadow if we're not in book mode
    // or if we've no sibling or are the last page of the "row"
    return !pSh || (!pSh->GetViewOptions()->IsViewLayoutBookMode()) || !GetNext()
        || (this == Lower())  || (bIsLTR && OnRightPage())
        || (!bIsLTR && !OnRightPage());

}

bool SwPageFrame::IsLeftShadowNeeded() const
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bIsLTR = getRootFrame()->IsLeftToRightViewLayout();

    // We paint the left shadow if we're not in book mode
    // or if we've no sibling or are the last page of the "row"
    return !pSh || (!pSh->GetViewOptions()->IsViewLayoutBookMode()) || !GetPrev()
        || (bIsLTR && !OnRightPage())
        || (!bIsLTR && OnRightPage());
}

/**
 * Determine rectangle for bottom page shadow
 * for #i9719#
 */
/*static*/ void SwPageFrame::GetHorizontalShadowRect( const SwRect& _rPageRect,
                                                const SwViewShell*    _pViewShell,
                                                OutputDevice* pRenderContext,
                                                SwRect&       _orHorizontalShadowRect,
                                                bool bPaintLeftShadow,
                                                bool bPaintRightShadow,
                                                bool bRightSidebar )
{
    const SwPostItMgr *pMgr = _pViewShell->GetPostItMgr();
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell, pRenderContext );
    SwRect aPagePxRect = pRenderContext->LogicToPixel( aAlignedPageRect.SVRect() );

    long lShadowAdjustment = mnShadowPxWidth - 1; // TODO: extract this

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
static void lcl_paintBitmapExToRect(vcl::RenderContext *pOut, const Point& aPoint, const Size& aSize, const BitmapEx& rBitmapEx, PaintArea eArea)
{
    // The problem is that if we get called multiple times and the color is
    // partly transparent, then the result will get darker and darker. To avoid
    // this, always paint the background color before doing the real paint.
    Rectangle aRect(aPoint, aSize);

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

    pOut->DrawBitmapEx(pOut->PixelToLogic(aPoint), pOut->PixelToLogic(aSize),
            Point(0, 0), aSize,
            rBitmapEx);
}

/**
 * Paint page border and shadow
 *
 * for #i9719#
 * implement paint of page border and shadow
*/
/*static*/ void SwPageFrame::PaintBorderAndShadow( const SwRect& _rPageRect,
                                                 const SwViewShell*    _pViewShell,
                                                 bool bPaintLeftShadow,
                                                 bool bPaintRightShadow,
                                                 bool bRightSidebar )
{
    // No shadow in prefs
    if (!SwViewOption::IsShadow())
        return;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *_pViewShell->GetOut() );

    static vcl::DeleteOnDeinit< drawinglayer::primitive2d::DiscreteShadow > shadowMaskObj
        ( new drawinglayer::primitive2d::DiscreteShadow( SW_RES( BMP_PAGE_SHADOW_MASK ) ));
    drawinglayer::primitive2d::DiscreteShadow& shadowMask = *shadowMaskObj.get();
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopRightShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomRightShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomLeftShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageRightShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopShadowBaseObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopLeftShadowObj( new BitmapEx );
    static vcl::DeleteOnDeinit< BitmapEx > aPageLeftShadowBaseObj( new BitmapEx );
    BitmapEx& aPageTopRightShadow = *aPageTopRightShadowObj.get();
    BitmapEx& aPageBottomRightShadow = *aPageBottomRightShadowObj.get();
    BitmapEx& aPageBottomLeftShadow = *aPageBottomLeftShadowObj.get();
    BitmapEx& aPageBottomShadow = *aPageBottomShadowBaseObj.get();
    BitmapEx& aPageRightShadow = *aPageRightShadowBaseObj.get();
    BitmapEx& aPageTopShadow = *aPageTopShadowBaseObj.get();
    BitmapEx& aPageTopLeftShadow = *aPageTopLeftShadowObj.get();
    BitmapEx& aPageLeftShadow = *aPageLeftShadowBaseObj.get();
    static Color aShadowColor( COL_AUTO );

    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell, _pViewShell->GetOut() );
    SwRect aPagePxRect = _pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() );

    if (aShadowColor != SwViewOption::GetShadowColor())
    {
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
        aPageBottomShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTop().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopRight().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getRight().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopLeft().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageTopLeftShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getLeft().GetBitmap() );
        aFilledSquare = Bitmap( aMask.GetSizePixel(), 24 );
        aFilledSquare.Erase( aShadowColor );
        aPageLeftShadow = BitmapEx( aFilledSquare, aMask );
    }

    SwRect aPaintRect;
    OutputDevice *pOut = _pViewShell->GetOut();

    SwPageFrame::GetHorizontalShadowRect( _rPageRect, _pViewShell, pOut, aPaintRect, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

    // Right shadow & corners
    if ( bPaintRightShadow )
    {
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right(), aPagePxRect.Bottom() + 1 - (aPageBottomRightShadow.GetSizePixel().Height() - mnShadowPxWidth) ) ),
            aPageBottomRightShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right(), aPagePxRect.Top() - mnShadowPxWidth ) ),
            aPageTopRightShadow );

        if (aPagePxRect.Height() > 2 * mnShadowPxWidth)
        {
            const long nWidth = aPageRightShadow.GetSizePixel().Width();
            const long nHeight = aPagePxRect.Height() - 2 * (mnShadowPxWidth - 1);
            if (aPageRightShadow.GetSizePixel().Height() < nHeight)
                aPageRightShadow.Scale(Size(nWidth, nHeight), BmpScaleFlag::Fast);

            lcl_paintBitmapExToRect(pOut,
                    Point(aPaintRect.Right() + mnShadowPxWidth, aPagePxRect.Top() + mnShadowPxWidth - 1),
                    Size(nWidth, nHeight),
                    aPageRightShadow, RIGHT);
        }
    }

    // Left shadows and corners
    if(bPaintLeftShadow)
    {
        const long lLeft = aPaintRect.Left() - aPageBottomLeftShadow.GetSizePixel().Width();
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft,
            aPagePxRect.Bottom() + 1 + mnShadowPxWidth - aPageBottomLeftShadow.GetSizePixel().Height() ) ), aPageBottomLeftShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft, aPagePxRect.Top() - mnShadowPxWidth ) ), aPageTopLeftShadow );
        if (aPagePxRect.Height() > 2 * mnShadowPxWidth)
        {
            const long nWidth = aPageLeftShadow.GetSizePixel().Width();
            const long nHeight = aPagePxRect.Height() - 2 * (mnShadowPxWidth - 1);
            if (aPageLeftShadow.GetSizePixel().Height() < nHeight)
                aPageLeftShadow.Scale(Size(nWidth, nHeight), BmpScaleFlag::Fast);

            lcl_paintBitmapExToRect(pOut,
                    Point(lLeft, aPagePxRect.Top() + mnShadowPxWidth - 1),
                    Size(nWidth, nHeight),
                    aPageLeftShadow, LEFT);
        }
    }

    // Bottom shadow
    const long nBottomHeight = aPageBottomShadow.GetSizePixel().Height();
    if (aPageBottomShadow.GetSizePixel().Width() < aPaintRect.Width())
        aPageBottomShadow.Scale(Size(aPaintRect.Width(), nBottomHeight), BmpScaleFlag::Fast);

    lcl_paintBitmapExToRect(pOut,
            Point(aPaintRect.Left(), aPagePxRect.Bottom() + 2),
            Size(aPaintRect.Width(), nBottomHeight),
            aPageBottomShadow, BOTTOM);

    // Top shadow
    const long nTopHeight = aPageTopShadow.GetSizePixel().Height();
    if (aPageTopShadow.GetSizePixel().Width() < aPaintRect.Width())
        aPageTopShadow.Scale(Size(aPaintRect.Width(), nTopHeight), BmpScaleFlag::Fast);

    lcl_paintBitmapExToRect(pOut,
            Point(aPaintRect.Left(), aPagePxRect.Top() - mnShadowPxWidth),
            Size(aPaintRect.Width(), nTopHeight),
            aPageTopShadow, TOP);
}

/**
 * mod #i6193# paint sidebar for notes
 * IMPORTANT: if you change the rects here, also change SwPostItMgr::ScrollbarHit
 */
/*static*/void SwPageFrame::PaintNotesSidebar(const SwRect& _rPageRect, SwViewShell* _pViewShell, sal_uInt16 nPageNum, bool bRight)
{
    //TODO: cut out scrollbar area and arrows out of sidepane rect, otherwise it could flicker when pressing arrow buttons
    if (!_pViewShell )
        return;

    SwRect aPageRect( _rPageRect );
    SwAlignRect( aPageRect, _pViewShell, _pViewShell->GetOut() );

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

/*static*/ void SwPageFrame::PaintNotesSidebarArrows(const Point &aMiddleFirst, const Point &aMiddleSecond, SwViewShell* _pViewShell, const Color& rColorUp, const Color& rColorDown)
{
    tools::Polygon aTriangleUp(3);
    tools::Polygon aTriangleDown(3);

    aTriangleUp.SetPoint(aMiddleFirst + Point(0,_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),0);
    aTriangleUp.SetPoint(aMiddleFirst + Point(_pViewShell->GetOut()->PixelToLogic(Size(-3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),1);
    aTriangleUp.SetPoint(aMiddleFirst + Point(_pViewShell->GetOut()->PixelToLogic(Size(3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),2);

    aTriangleDown.SetPoint(aMiddleSecond + Point(_pViewShell->GetOut()->PixelToLogic(Size(-3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),0);
    aTriangleDown.SetPoint(aMiddleSecond + Point(_pViewShell->GetOut()->PixelToLogic(Size(+3,0)).Width(),_pViewShell->GetOut()->PixelToLogic(Size(0,-3)).Height()),1);
    aTriangleDown.SetPoint(aMiddleSecond + Point(0,_pViewShell->GetOut()->PixelToLogic(Size(0,3)).Height()),2);

    _pViewShell->GetOut()->SetFillColor(rColorUp);
    _pViewShell->GetOut()->DrawPolygon(aTriangleUp);
    _pViewShell->GetOut()->SetFillColor(rColorDown);
    _pViewShell->GetOut()->DrawPolygon(aTriangleDown);
}

/**
 * Get bound rectangle of border and shadow for repaints
 *
 * for #i9719#
 */
/*static*/ void SwPageFrame::GetBorderAndShadowBoundRect( const SwRect& _rPageRect,
                                                        const SwViewShell*    _pViewShell,
                                                        OutputDevice* pRenderContext,
                                                        SwRect& _orBorderAndShadowBoundRect,
                                                        bool bLeftShadow,
                                                        bool bRightShadow,
                                                        bool bRightSidebar
                                                      )
{
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell, pRenderContext );
    SwRect aPagePxRect = pRenderContext->LogicToPixel( aAlignedPageRect.SVRect() );
    aPagePxRect.Bottom( aPagePxRect.Bottom() + mnShadowPxWidth + 1 );
    aPagePxRect.Top( aPagePxRect.Top() - mnShadowPxWidth - 1 );

    SwRect aTmpRect;

    // Always ask for full shadow since we want a bounding rect
    // including at least the page frame
    SwPageFrame::GetHorizontalShadowRect( _rPageRect, _pViewShell, pRenderContext, aTmpRect, false, false, bRightSidebar );

    if(bLeftShadow) aPagePxRect.Left( aTmpRect.Left() - mnShadowPxWidth - 1);
    if(bRightShadow) aPagePxRect.Right( aTmpRect.Right() + mnShadowPxWidth + 1);

    _orBorderAndShadowBoundRect = pRenderContext->PixelToLogic( aPagePxRect.SVRect() );
}

SwRect SwPageFrame::GetBoundRect(OutputDevice* pOutputDevice) const
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    SwRect aPageRect( Frame() );
    SwRect aResult;

    if(!pSh) {
        return SwRect( Point(0, 0), Size(0, 0) );
    }

    SwPageFrame::GetBorderAndShadowBoundRect( aPageRect, pSh, pOutputDevice, aResult,
        IsLeftShadowNeeded(), IsRightShadowNeeded(), SidebarPosition() ==  sw::sidebarwindows::SidebarPosition::RIGHT );
    return aResult;
}

/*static*/ SwTwips SwPageFrame::GetSidebarBorderWidth( const SwViewShell* _pViewShell )
{
    const SwPostItMgr* pPostItMgr = _pViewShell ? _pViewShell->GetPostItMgr() : nullptr;
    const SwTwips nRet = pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() ? pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() : 0;
    return nRet;
}

void SwFrame::PaintBaBo( const SwRect& rRect, const SwPageFrame *pPage,
                         const bool bOnlyTextBackground ) const
{
    if ( !pPage )
        pPage = FindPageFrame();

    OutputDevice *pOut = gProp.pSGlobalShell->GetOut();

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pOut );

    // OD 2004-04-23 #116347#
    pOut->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
    pOut->SetLineColor();

    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    // OD 20.11.2002 #104598# - take care of page margin area
    // Note: code move from <SwFrame::PaintBackground(..)> to new method
    // <SwPageFrame::Paintmargin(..)>.
    if ( IsPageFrame() && !bOnlyTextBackground)
    {
        static_cast<const SwPageFrame*>(this)->PaintMarginArea( rRect, gProp.pSGlobalShell );
    }

    // paint background
    {
        PaintBackground( rRect, pPage, rAttrs, false, true/*bLowerBorder*/, bOnlyTextBackground );
    }

    // OD 06.08.2002 #99657# - paint border before painting background
    // paint grid for page frame and paint border
    if (!bOnlyTextBackground)
    {
        SwRect aRect( rRect );
        if( IsPageFrame() )
            static_cast<const SwPageFrame*>(this)->PaintGrid( pOut, aRect );
        PaintBorder( aRect, pPage, rAttrs );
    }

    pOut->Pop();
}

static bool lcl_compareFillAttributes(const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& pA, const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& pB)
{
    if (pA == pB)
        return true;
    if (!pA || !pB)
        return false;
    return pA->getFillAttribute() == pB->getFillAttribute();
}

/// OD 05.09.2002 #102912#
/// Do not paint background for fly frames without a background brush by
/// calling <PaintBaBo> at the page or at the fly frame its anchored
void SwFrame::PaintBackground( const SwRect &rRect, const SwPageFrame *pPage,
                             const SwBorderAttrs & rAttrs,
                             const bool bLowerMode,
                             const bool bLowerBorder,
                             const bool bOnlyTextBackground ) const
{
    // OD 20.01.2003 #i1837# - no paint of table background, if corresponding
    // option is *not* set.
    if( IsTabFrame() &&
        !gProp.pSGlobalShell->GetViewOptions()->IsTable() )
    {
        return;
    }

    // nothing to do for covered table cells:
    if( IsCellFrame() && IsCoveredCell() )
        return;

    SwViewShell *pSh = gProp.pSGlobalShell;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pSh->GetOut() );

    const SvxBrushItem* pItem;
    // OD 05.09.2002 #102912#
    // temporary background brush for a fly frame without a background brush
    SvxBrushItem* pTmpBackBrush = nullptr;
    const Color* pCol;
    SwRect aOrigBackRect;
    const bool bPageFrame = IsPageFrame();
    bool bLowMode = true;

    //UUUU
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    bool bBack = GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, bLowerMode );
    //- Output if a separate background is used.
    bool bNoFlyBackground = !gProp.bSFlyMetafile && !bBack && IsFlyFrame();
    if ( bNoFlyBackground )
    {
        // OD 05.09.2002 #102912# - Fly frame has no background.
        // Try to find background brush at parents, if previous call of
        // <GetBackgroundBrush> disabled this option with the parameter <bLowerMode>
        if ( bLowerMode )
        {
            bBack = GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, false );
        }
        // If still no background found for the fly frame, initialize the
        // background brush <pItem> with global retouche color and set <bBack>
        // to true, that fly frame will paint its background using this color.
        if ( !bBack )
        {
            // OD 10.01.2003 #i6467# - on print output, pdf output and
            // in embedded mode not editing color COL_WHITE is used instead of
            // the global retouche color.
            if ( pSh->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
                 pSh->GetViewOptions()->IsPDFExport() ||
                 ( pSh->GetDoc()->GetDocShell()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED &&
                   !pSh->GetDoc()->GetDocShell()->IsInPlaceActive()
                 )
               )
            {
                pTmpBackBrush = new SvxBrushItem( Color( COL_WHITE ), RES_BACKGROUND );

                //UUU
                aFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(Color( COL_WHITE )));
            }
            else
            {
                pTmpBackBrush = new SvxBrushItem( aGlobalRetoucheColor, RES_BACKGROUND);

                //UUU
                aFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aGlobalRetoucheColor));
            }

            pItem = pTmpBackBrush;
            bBack = true;
        }
    }

    SwRect aPaintRect( Frame() );
    if( IsTextFrame() || IsSctFrame() )
        aPaintRect = UnionFrame( true );

    if ( aPaintRect.IsOver( rRect ) )
    {
        if ( bBack || bPageFrame || !bLowerMode )
        {
            const bool bBrowse = pSh->GetViewOptions()->getBrowseMode();
            SwRect aRect;
            if ( (bPageFrame && bBrowse) ||
                 (IsTextFrame() && Prt().SSize() == Frame().SSize()) )
            {
                aRect = Frame();
                ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
            }
            else
            {
                ::lcl_CalcBorderRect( aRect, this, rAttrs, false, gProp);
                if ( (IsTextFrame() || IsTabFrame()) && GetPrev() )
                {
                    if ( GetPrev()->GetAttrSet()->GetBackground() == GetAttrSet()->GetBackground() &&
                         lcl_compareFillAttributes(GetPrev()->getSdrAllFillAttributesHelper(), getSdrAllFillAttributesHelper()))
                    {
                        aRect.Top( Frame().Top() );
                    }
                }
            }
            aRect.Intersection( rRect );

            OutputDevice *pOut = pSh->GetOut();

            if ( aRect.HasArea() )
            {
                SvxBrushItem* pNewItem = nullptr;

                if( pCol )
                {
                    pNewItem = new SvxBrushItem( *pCol, RES_BACKGROUND );
                    pItem = pNewItem;

                    //UUUU
                    aFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(*pCol));
                }

                SwRegionRects aRegion( aRect );
                if (pPage->GetSortedObjs() &&
                    pSh->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS))
                {
                    ::lcl_SubtractFlys( this, pPage, aRect, aRegion, gProp );
                }

                // OD 06.08.2002 #99657# - determine, if background transparency
                //     have to be considered for drawing.
                //     --> Status Quo: background transparency have to be
                //        considered for fly frames
                const bool bConsiderBackgroundTransparency = IsFlyFrame();
                bool bDone(false);

                // #i125189# We are also done when the new DrawingLayer FillAttributes are used
                // or the FillStyle is set (different from drawing::FillStyle_NONE)
                if(pOut && aFillAttributes.get())
                {
                    if(aFillAttributes->isUsed())
                    {
                        // check if really something is painted
                        bDone = DrawFillAttributes(aFillAttributes, aOrigBackRect, aRegion, *pOut);
                    }

                    if(!bDone)
                    {
                        // if not, still a FillStyle could be set but the transparency is at 100%,
                        // thus need to check the model data itself for FillStyle (do not rely on
                        // SdrAllFillAttributesHelper since it already contains optimized information,
                        // e.g. transparency leads to no fill)
                        const drawing::FillStyle eFillStyle(static_cast< const XFillStyleItem& >(GetAttrSet()->Get(XATTR_FILLSTYLE)).GetValue());

                        if(drawing::FillStyle_NONE != eFillStyle)
                        {
                            bDone = true;
                        }
                    }
                }

                if(!bDone)
                {
                    for (size_t i = 0; i < aRegion.size(); ++i)
                    {
                        if (1 < aRegion.size())
                        {
                            ::SwAlignRect( aRegion[i], gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
                            if( !aRegion[i].HasArea() )
                              continue;
                        }
                    // OD 06.08.2002 #99657# - add 6th parameter to indicate, if
                    //     background transparency have to be considered
                    //     Set missing 5th parameter to the default value GRFNUM_NO
                    //         - see declaration in /core/inc/frmtool.hxx.
                        if (IsTextFrame() || !bOnlyTextBackground)
                            ::DrawGraphic(
                                pItem,
                                pOut,
                                aOrigBackRect,
                                aRegion[i],
                                GRFNUM_NO,
                                bConsiderBackgroundTransparency );
                    }
                }
                if( pCol )
                    delete pNewItem;
            }
        }
        else
            bLowMode = bLowerMode;
    }

    // OD 05.09.2002 #102912#
    // delete temporary background brush.
    delete pTmpBackBrush;

    //Now process lower and his neighbour.
    //We end this as soon as a Frame leaves the chain and therefore is not a lower
    //of me anymore
    const SwFrame *pFrame = GetLower();
    if ( pFrame )
    {
        SwRect aFrameRect;
        SwRect aRect( PaintArea() );
        aRect.Intersection_( rRect );
        SwRect aBorderRect( aRect );
        SwShortCut aShortCut( *pFrame, aBorderRect );
        do
        {   if ( gProp.pSProgress )
                gProp.pSProgress->Reschedule();

            aFrameRect = pFrame->PaintArea();
            if ( aFrameRect.IsOver( aBorderRect ) )
            {
                SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
                const SwBorderAttrs &rTmpAttrs = *aAccess.Get();
                if ( ( pFrame->IsLayoutFrame() && bLowerBorder ) ||
                     aFrameRect.IsOver( aRect ) )
                    pFrame->PaintBackground( aRect, pPage, rTmpAttrs, bLowMode,
                                           bLowerBorder );
                if ( bLowerBorder )
                    pFrame->PaintBorder( aBorderRect, pPage, rTmpAttrs );
            }
            pFrame = pFrame->GetNext();
        } while ( pFrame && pFrame->GetUpper() == this &&
                  !aShortCut.Stop( aFrameRect ) );
    }
}

/// Refreshes all subsidiary lines of a page.
void SwPageFrame::RefreshSubsidiary( const SwRect &rRect ) const
{
    if ( IS_SUBS || isTableBoundariesEnabled() || IS_SUBS_SECTION || IS_SUBS_FLYS )
    {
        if ( rRect.HasArea() )
        {
            //During paint using the root, the array is controlled from there.
            //Otherwise we'll handle it for our self.
            bool bDelSubs = false;
            if ( !gProp.pSSubsLines )
            {
                gProp.pSSubsLines = new SwSubsRects;
                // OD 20.12.2002 #106318# - create container for special subsidiary lines
                gProp.pSSpecSubsLines = new SwSubsRects;
                bDelSubs = true;
            }

            RefreshLaySubsidiary( this, rRect );

            if ( bDelSubs )
            {
                // OD 20.12.2002 #106318# - paint special subsidiary lines
                // and delete its container
                gProp.pSSpecSubsLines->PaintSubsidiary( gProp.pSGlobalShell->GetOut(), nullptr, gProp );
                DELETEZ( gProp.pSSpecSubsLines );

                gProp.pSSubsLines->PaintSubsidiary( gProp.pSGlobalShell->GetOut(), gProp.pSLines, gProp );
                DELETEZ( gProp.pSSubsLines );
            }
        }
    }
}

void SwLayoutFrame::RefreshLaySubsidiary( const SwPageFrame *pPage,
                                        const SwRect &rRect ) const
{
    const bool bSubsOpt   = IS_SUBS;
    if ( bSubsOpt )
        PaintSubsidiaryLines( pPage, rRect );

    const SwFrame *pLow = Lower();
    if( !pLow )
        return;
    SwShortCut aShortCut( *pLow, rRect );
    while( pLow && !aShortCut.Stop( pLow->Frame() ) )
    {
        if ( pLow->Frame().IsOver( rRect ) && pLow->Frame().HasArea() )
        {
            if ( pLow->IsLayoutFrame() )
                static_cast<const SwLayoutFrame*>(pLow)->RefreshLaySubsidiary( pPage, rRect);
            else if ( pLow->GetDrawObjs() )
            {
                const SwSortedObjs& rObjs = *(pLow->GetDrawObjs());
                for (SwAnchoredObject* pAnchoredObj : rObjs)
                {
                    if ( pPage->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                         dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                    {
                        const SwFlyFrame *pFly =
                                    static_cast<const SwFlyFrame*>(pAnchoredObj);
                        if ( pFly->IsFlyInContentFrame() && pFly->Frame().IsOver( rRect ) )
                        {
                            if ( !pFly->Lower() || !pFly->Lower()->IsNoTextFrame() ||
                                 !static_cast<const SwNoTextFrame*>(pFly->Lower())->HasAnimation())
                                pFly->RefreshLaySubsidiary( pPage, rRect );
                        }
                    }
                }
            }
        }
        pLow = pLow->GetNext();
    }
}

/**
 * Subsidiary lines to paint the PrtAreas
 * Only the LayoutFrames which directly contain Content
 * Paints the desired line and pays attention to not overpaint any flys
 */
static void lcl_RefreshLine( const SwLayoutFrame *pLay,
                                  const SwPageFrame *pPage,
                                  const Point &rP1,
                                  const Point &rP2,
                                  const sal_uInt8 nSubColor,
                                  SwLineRects* pSubsLines )
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
        const SwFlyFrame *pMyFly = pLay->FindFlyFrame();
        if ( pMyFly )
        {
            aIter.Current( pMyFly->GetVirtDrawObj() );
            while ( nullptr != (pMyFly = pMyFly->GetAnchorFrame()->FindFlyFrame()) )
            {
                if ( aIter()->GetOrdNum() > pMyFly->GetVirtDrawObj()->GetOrdNum() )
                    aIter.Current( pMyFly->GetVirtDrawObj() );
            }
        }
        else
            aIter.Bottom();

        while ( aIter() )
        {
            const SwVirtFlyDrawObj *pObj = static_cast<const SwVirtFlyDrawObj*>(aIter());
            const SwFlyFrame *pFly = pObj ? pObj->GetFlyFrame() : nullptr;

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
                 !pFly->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( pObj->GetLayer() ) )
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
            // OD 18.11.2002 #99672# - use parameter <pSubsLines> instead of
            // global variable <gProp.pSSubsLines>.
            pSubsLines->AddLineRect( aRect, nullptr, table::BorderLineStyle::SOLID,
                    nullptr, nSubColor, gProp );
        }
        aP1 = aP2;
        (aP1.*pDirPt)() += 1;
        aP2 = rP2;
    }
}

static drawinglayer::primitive2d::Primitive2DContainer lcl_CreatePageAreaDelimiterPrimitives(
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DContainer aSeq( 4 );

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

static drawinglayer::primitive2d::Primitive2DContainer lcl_CreateRectangleDelimiterPrimitives (
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DContainer aSeq( 1 );
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

static drawinglayer::primitive2d::Primitive2DContainer lcl_CreateColumnAreaDelimiterPrimitives(
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DContainer aSeq( 4 );

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

void SwPageFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
    if ( !gProp.pSGlobalShell->IsHeaderFooterEdit() )
    {
        const SwFrame* pLay = Lower();
        const SwFrame* pFootnoteCont = nullptr;
        const SwFrame* pPageBody = nullptr;
        while ( pLay && !( pFootnoteCont && pPageBody ) )
        {
            if ( pLay->IsFootnoteContFrame( ) )
                pFootnoteCont = pLay;
            if ( pLay->IsBodyFrame() )
                pPageBody = pLay;
            pLay = pLay->GetNext();
        }

        SwRect aArea( pPageBody->Frame() );
        if ( pFootnoteCont )
            aArea.AddBottom( pFootnoteCont->Frame().Bottom() - aArea.Bottom() );

        if ( !gProp.pSGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
            ProcessPrimitives( lcl_CreatePageAreaDelimiterPrimitives( aArea ) );
        else
            ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
    }
}

void SwColumnFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
    const SwFrame* pLay = Lower();
    const SwFrame* pFootnoteCont = nullptr;
    const SwFrame* pColBody = nullptr;
    while ( pLay && !( pFootnoteCont && pColBody ) )
    {
        if ( pLay->IsFootnoteContFrame( ) )
            pFootnoteCont = pLay;
        if ( pLay->IsBodyFrame() )
            pColBody = pLay;
        pLay = pLay->GetNext();
    }

    SwRect aArea( pColBody->Frame() );

    // #i3662# - enlarge top of column body frame's printing area
    // in sections to top of section frame.
    const bool bColInSection =  GetUpper()->IsSctFrame();
    if ( bColInSection )
    {
        if ( IsVertical() )
            aArea.Right( GetUpper()->Frame().Right() );
        else
            aArea.Top( GetUpper()->Frame().Top() );
    }

    if ( pFootnoteCont )
        aArea.AddBottom( pFootnoteCont->Frame().Bottom() - aArea.Bottom() );

    ::SwAlignRect( aArea, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );

    if ( !gProp.pSGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
        ProcessPrimitives( lcl_CreateColumnAreaDelimiterPrimitives( aArea ) );
    else
        ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
}

void SwSectionFrame::PaintSubsidiaryLines( const SwPageFrame * pPage,
                                        const SwRect & rRect ) const
{
    const bool bNoLowerColumn = !Lower() || !Lower()->IsColumnFrame();
    if ( bNoLowerColumn )
    {
        SwLayoutFrame::PaintSubsidiaryLines( pPage, rRect );
    }
}

/**
 * The SwBodyFrame doesn't print any subsidiary line: it's bounds are painted
 * either by the parent page or the parent column frame.
 */
void SwBodyFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
}

void SwHeadFootFrame::PaintSubsidiaryLines( const SwPageFrame *, const SwRect & ) const
{
    if ( gProp.pSGlobalShell->IsHeaderFooterEdit() )
    {
        SwRect aArea( Prt() );
        aArea.Pos() += Frame().Pos();
        if ( !gProp.pSGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
            ProcessPrimitives( lcl_CreatePageAreaDelimiterPrimitives( aArea ) );
        else
            ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
    }
}

/**
 * This method is overridden in order to have no subsidiary lines
 * around the footnotes.
 */
void SwFootnoteFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
}

/**
 * This method is overridden in order to have no subsidiary lines
 * around the footnotes containers.
 */
void SwFootnoteContFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
}

void SwLayoutFrame::PaintSubsidiaryLines( const SwPageFrame *pPage,
                                        const SwRect &rRect ) const
{
    bool bNewTableModel = false;

    // #i29550#
    if ( IsTabFrame() || IsCellFrame() || IsRowFrame() )
    {
        const SwTabFrame* pTabFrame = FindTabFrame();
        if ( pTabFrame->IsCollapsingBorders() )
            return;

        bNewTableModel = pTabFrame->GetTable()->IsNewModel();
        // in the new table model, we have an early return for all cell-related
        // frames, except from non-covered table cells
        if ( bNewTableModel )
            if ( IsTabFrame() ||
                 IsRowFrame() ||
                 ( IsCellFrame() && IsCoveredCell() ) )
                return;
    }

    const bool bFlys = pPage->GetSortedObjs() != nullptr;

    const bool bCell = IsCellFrame();
    // use frame area for cells
    // OD 13.02.2003 #i3662# - for section use also frame area
    const bool bUseFrameArea = bCell || IsSctFrame();
    SwRect aOriginal( bUseFrameArea ? Frame() : Prt() );
    if ( !bUseFrameArea )
        aOriginal.Pos() += Frame().Pos();

    ::SwAlignRect( aOriginal, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );

    if ( !aOriginal.IsOver( rRect ) )
        return;

    SwRect aOut( aOriginal );
    aOut.Intersection_( rRect );

    const SwTwips nRight = aOut.Right();
    const SwTwips nBottom= aOut.Bottom();

    const Point aRT( nRight, aOut.Top() );
    const Point aRB( nRight, nBottom );
    const Point aLB( aOut.Left(), nBottom );

    sal_uInt8 nSubColor = ( bCell || IsRowFrame() ) ? SUBCOL_TAB :
                     ( IsInSct() ? SUBCOL_SECT :
                     ( IsInFly() ? SUBCOL_FLY : SUBCOL_PAGE ) );

    // OD 18.11.2002 #99672# - collect body, header, footer, footnote and section
    // sub-lines in <pSpecSubsLine> array.
    const bool bSpecialSublines = IsBodyFrame() || IsHeaderFrame() || IsFooterFrame() ||
                                  IsFootnoteFrame() || IsSctFrame();
    SwLineRects* pUsedSubsLines = bSpecialSublines ? gProp.pSSpecSubsLines : gProp.pSSubsLines;

    // NOTE: for cell frames only left and right (horizontal layout) respectively
    //      top and bottom (vertical layout) lines painted.
    // NOTE2: this does not hold for the new table model!!! We paint the top border
    // of each non-covered table cell.
    const bool bVert = IsVertical();
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
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        table::BorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
            // OD 14.11.2002 #104821# - in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
            {
                const SwRect aRect( aRT, aRB );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        table::BorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
        }
        // OD 14.11.2002 #104822# - adjust control for drawing top and bottom lines
        if ( !bCell || bNewTableModel || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
            {
                // OD 14.11.2002 #104821# - in horizontal layout set page/column break at top
                const SwRect aRect( aOut.Pos(), aRT );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        table::BorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
            if ( aOriginal.Bottom() == nBottom )
            {
                const SwRect aRect( aLB, aRB );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        table::BorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
        }
    }
}

/**
 * Refreshes all extra data (line breaks a.s.o) of the page. Basically only those objects
 * are considered which horizontally overlap the Rect.
 */
void SwPageFrame::RefreshExtraData( const SwRect &rRect ) const
{
    const SwLineNumberInfo &rInfo = GetFormat()->GetDoc()->GetLineNumberInfo();
    bool bLineInFly = (rInfo.IsPaintLineNumbers() && rInfo.IsCountInFlys())
        || (sal_Int16)SW_MOD()->GetRedlineMarkPos() != text::HoriOrientation::NONE;

    SwRect aRect( rRect );
    ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
    if ( aRect.HasArea() )
    {
        SwLayoutFrame::RefreshExtraData( aRect );

        if ( bLineInFly && GetSortedObjs() )
            for (SwAnchoredObject* pAnchoredObj : *GetSortedObjs())
            {
                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    const SwFlyFrame *pFly = static_cast<const SwFlyFrame*>(pAnchoredObj);
                    if ( pFly->Frame().Top() <= aRect.Bottom() &&
                         pFly->Frame().Bottom() >= aRect.Top() )
                        pFly->RefreshExtraData( aRect );
                }
            }
    }
}

void SwLayoutFrame::RefreshExtraData( const SwRect &rRect ) const
{

    const SwLineNumberInfo &rInfo = GetFormat()->GetDoc()->GetLineNumberInfo();
    bool bLineInBody = rInfo.IsPaintLineNumbers(),
             bLineInFly  = bLineInBody && rInfo.IsCountInFlys(),
             bRedLine = (sal_Int16)SW_MOD()->GetRedlineMarkPos()!=text::HoriOrientation::NONE;

    const SwContentFrame *pCnt = ContainsContent();
    while ( pCnt && IsAnLower( pCnt ) )
    {
        if ( pCnt->IsTextFrame() && ( bRedLine ||
             ( !pCnt->IsInTab() &&
               ((bLineInBody && pCnt->IsInDocBody()) ||
               (bLineInFly  && pCnt->IsInFly())) ) ) &&
             pCnt->Frame().Top() <= rRect.Bottom() &&
             pCnt->Frame().Bottom() >= rRect.Top() )
        {
            static_cast<const SwTextFrame*>(pCnt)->PaintExtraData( rRect );
        }
        if ( bLineInFly && pCnt->GetDrawObjs() )
            for (SwAnchoredObject* pAnchoredObj : *pCnt->GetDrawObjs())
            {
                if ( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) !=  nullptr )
                {
                    const SwFlyFrame *pFly = static_cast<const SwFlyFrame*>(pAnchoredObj);
                    if ( pFly->IsFlyInContentFrame() &&
                         pFly->Frame().Top() <= rRect.Bottom() &&
                         pFly->Frame().Bottom() >= rRect.Top() )
                        pFly->RefreshExtraData( rRect );
                }
        }
        pCnt = pCnt->GetNextContentFrame();
    }
}

/**
 * For #102450#
 * Determine the color, that is respectively will be drawn as background
 * for the page frame.
 * Using existing method SwFrame::GetBackgroundBrush to determine the color
 * that is set at the page frame respectively is parent. If none is found
 * return the global retouche color
 *
 * @return Color
 */
const Color SwPageFrame::GetDrawBackgrdColor() const
{
    const SvxBrushItem* pBrushItem;
    const Color* pDummyColor;
    SwRect aDummyRect;

    //UUUU
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    if ( GetBackgroundBrush( aFillAttributes, pBrushItem, pDummyColor, aDummyRect, true) )
    {
        if(aFillAttributes.get() && aFillAttributes->isUsed()) //UUUU
        {
            // let SdrAllFillAttributesHelper do the average color calculation
            return Color(aFillAttributes->getAverageColor(aGlobalRetoucheColor.getBColor()));
        }
        else if(pBrushItem)
        {
            OUString referer;
            SwViewShell * sh1 = getRootFrame()->GetCurrShell();
            if (sh1 != nullptr) {
                SfxObjectShell * sh2 = sh1->GetDoc()->GetPersist();
                if (sh2 != nullptr && sh2->HasName()) {
                    referer = sh2->GetMedium()->GetName();
                }
            }
            const Graphic* pGraphic = pBrushItem->GetGraphic(referer);

            if(pGraphic)
            {
                // #29105# when a graphic is set, it may be possible to calculate a single
                // color which looks good in all places of the graphic. Since it is
                // planned to have text edit on the overlay one day and the fallback
                // to aGlobalRetoucheColor returns something useful, just use that
                // for now.
            }
            else
            {
                // not a graphic, use (hopefully) initialized color
                return pBrushItem->GetColor();
            }
        }
    }

    return aGlobalRetoucheColor;
}

/// create/return font used to paint the "empty page" string
const vcl::Font& SwPageFrame::GetEmptyPageFont()
{
    static vcl::Font* pEmptyPgFont = nullptr;
    if ( nullptr == pEmptyPgFont )
    {
        pEmptyPgFont = new vcl::Font;
        pEmptyPgFont->SetFontSize( Size( 0, 80 * 20 )); // == 80 pt
        pEmptyPgFont->SetWeight( WEIGHT_BOLD );
        pEmptyPgFont->SetStyleName( aEmptyOUStr );
        pEmptyPgFont->SetFamilyName("Helvetica");
        pEmptyPgFont->SetFamily( FAMILY_SWISS );
        pEmptyPgFont->SetTransparent( true );
        pEmptyPgFont->SetColor( COL_GRAY );
    }

    return *pEmptyPgFont;
}

/**
 * Retouch for a section
 *
 * Retouch will only be done, if the Frame is the last one in his chain.
 * The whole area of the upper which is located below the Frame will be
 * cleared using PaintBackground.
 */
void SwFrame::Retouch( const SwPageFrame * pPage, const SwRect &rRect ) const
{
    if ( gProp.bSFlyMetafile )
        return;

    OSL_ENSURE( GetUpper(), "Retouche try without Upper." );
    OSL_ENSURE( getRootFrame()->GetCurrShell() && gProp.pSGlobalShell->GetWin(), "Retouche on a printer?" );

    SwRect aRetouche( GetUpper()->PaintArea() );
    aRetouche.Top( Frame().Top() + Frame().Height() );
    aRetouche.Intersection( gProp.pSGlobalShell->VisArea() );

    if ( aRetouche.HasArea() )
    {
        //Omit the passed Rect. To do this, we unfortunately need a region to
        //cut out.
        SwRegionRects aRegion( aRetouche );
        aRegion -= rRect;
        SwViewShell *pSh = getRootFrame()->GetCurrShell();

        // #i16816# tagged pdf support
        SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pSh->GetOut() );

        for ( size_t i = 0; i < aRegion.size(); ++i )
        {
            const SwRect &rRetouche = aRegion[i];

            GetUpper()->PaintBaBo( rRetouche, pPage );

            //Hell and Heaven need to be refreshed too.
            //To avoid recursion my retouch flag needs to be reset first!
            ResetRetouche();
            if ( rRetouche.HasArea() )
            {
                const Color aPageBackgrdColor(pPage->GetDrawBackgrdColor());
                const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
                // --> OD #i76669#
                SwViewObjectContactRedirector aSwRedirector( *pSh );
                // <--

                pSh->Imp()->PaintLayer( rIDDMA.GetHellId(), nullptr,
                                        rRetouche, &aPageBackgrdColor,
                                        pPage->IsRightToLeft(),
                                        &aSwRedirector );
                pSh->Imp()->PaintLayer( rIDDMA.GetHeavenId(), nullptr,
                                        rRetouche, &aPageBackgrdColor,
                                        pPage->IsRightToLeft(),
                                        &aSwRedirector );
            }

            SetRetouche();

            //Because we leave all paint areas, we need to refresh the
            //subsidiary lines.
            pPage->RefreshSubsidiary( rRetouche );
        }
    }
    if ( SwViewShell::IsLstEndAction() )
        ResetRetouche();
}

/**
 * Determine the background brush for the frame:
 * the background brush is taken from it-self or from its parent (anchor/upper).
 * Normally, the background brush is taken, which has no transparent color or
 * which has a background graphic. But there are some special cases:
 * (1) No background brush is taken from a page frame, if view option "IsPageBack"
 *     isn't set.
 * (2) Background brush from a index section is taken under special conditions.
 *     In this case parameter <rpCol> is set to the index shading color.
 * (3) New (OD 20.08.2002) - Background brush is taken, if on background drawing
 *     of the frame transparency is considered and its color is not "no fill"/"auto fill"
 *
 * Old description in German:
 * Returns the Backgroundbrush for the area of the Frame.
 * The Brush is defined by the Frame or by an upper, the first Brush is
 * used. If no Brush is defined for a Frame, false is returned.
 *
 * @param rpBrush
 * output parameter - constant reference pointer the found background brush
 *
 * @param rpFillStyle
 * output parameter - constant reference pointer the found background fill style
 *
 * @param rpFillGradient
 * output parameter - constant reference pointer the found background fill gradient
 *
 * @param rpCol
 * output parameter - constant reference pointer to the color of the index shading
 * set under special conditions, if background brush is taken from an index section.
 *
 * @param rOrigRect
 * in-/output parameter - reference to the rectangle the background brush is
 * considered for - adjusted to the frame, from which the background brush is
 * taken.
 *
 * @parem bLowerMode
 * input parameter - boolean indicating, if background brush should *not* be
 * taken from parent.
 *
 * @return true, if a background brush for the frame is found
 */
bool SwFrame::GetBackgroundBrush(
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const SvxBrushItem* & rpBrush,
    const Color*& rpCol,
    SwRect &rOrigRect,
    bool bLowerMode ) const
{
    const SwFrame *pFrame = this;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const SwViewOption *pOpt = pSh->GetViewOptions();
    rpBrush = nullptr;
    rpCol = nullptr;
    do
    {   if ( pFrame->IsPageFrame() && !pOpt->IsPageBack() )
            return false;

        //UUUU
        if (pFrame->supportsFullDrawingLayerFillAttributeSet())
            rFillAttributes = pFrame->getSdrAllFillAttributesHelper();
        const SvxBrushItem &rBack = pFrame->GetAttrSet()->GetBackground();

        if( pFrame->IsSctFrame() )
        {
            const SwSection* pSection = static_cast<const SwSectionFrame*>(pFrame)->GetSection();
            // OD 20.08.2002 #99657# #GetTransChg#
            //     Note: If frame <pFrame> is a section of the index and
            //         it its background color is "no fill"/"auto fill" and
            //         it has no background graphic and
            //         we are not in the page preview and
            //         we are not in read-only mode and
            //         option "index shadings" is set and
            //         the output is not the printer
            //         then set <rpCol> to the color of the index shading
            if( pSection && (   TOX_HEADER_SECTION == pSection->GetType() ||
                                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                (rBack.GetColor() == COL_TRANSPARENT) &&
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

        // OD 20.08.2002 #99657#
        //     determine, if background draw of frame <pFrame> considers transparency
        //     --> Status Quo: background transparency have to be
        //                     considered for fly frames
        const bool bConsiderBackgroundTransparency = pFrame->IsFlyFrame();

        // #i125189# Do not base the decision for using the parent's fill style for this
        // frame when the new DrawingLayer FillAttributes are used on the SdrAllFillAttributesHelper
        // information. There the data is already optimized to no fill in the case that the
        // transparence is at 100% while no fill is the criteria for derivation
        bool bNewDrawingLayerFillStyleIsUsedAndNotNoFill(false);

        if(rFillAttributes.get())
        {
            // the new DrawingLayer FillStyle is used
            if(rFillAttributes->isUsed())
            {
                // it's not drawing::FillStyle_NONE
                bNewDrawingLayerFillStyleIsUsedAndNotNoFill = true;
            }
            else
            {
                // maybe optimized already when 100% transparency is used somewhere, need to test
                // XFillStyleItem directly from the model data
                const drawing::FillStyle eFillStyle(static_cast< const XFillStyleItem& >(pFrame->GetAttrSet()->Get(XATTR_FILLSTYLE)).GetValue());

                if(drawing::FillStyle_NONE != eFillStyle)
                {
                    bNewDrawingLayerFillStyleIsUsedAndNotNoFill = true;
                }
            }
        }

        // OD 20.08.2002 #99657#
        //     add condition:
        //     If <bConsiderBackgroundTransparency> is set - see above -,
        //     return brush of frame <pFrame>, if its color is *not* "no fill"/"auto fill"
        if (
            // #i125189# Done when the new DrawingLayer FillAttributes are used and
            // not drawing::FillStyle_NONE (see above)
            bNewDrawingLayerFillStyleIsUsedAndNotNoFill ||

            // done when SvxBrushItem is used
            !rBack.GetColor().GetTransparency() || rBack.GetGraphicPos() != GPOS_NONE ||

            // done when direct color is forced
            rpCol ||

            // done when consider BG transparency and color is not completely transparent
            (bConsiderBackgroundTransparency && (rBack.GetColor() != COL_TRANSPARENT))
           )
        {
            rpBrush = &rBack;
            if ( pFrame->IsPageFrame() && pSh->GetViewOptions()->getBrowseMode() )
            {
                rOrigRect = pFrame->Frame();
            }
            else
            {
                if ( pFrame->Frame().SSize() != pFrame->Prt().SSize() )
                {
                    SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    ::lcl_CalcBorderRect( rOrigRect, pFrame, rAttrs, false, gProp );
                }
                else
                {
                    rOrigRect = pFrame->Prt();
                    rOrigRect += pFrame->Frame().Pos();
                }
            }

            return true;
        }

        if ( bLowerMode )
        {
            // Do not try to get background brush from parent (anchor/upper)
            return false;
        }

        // get parent frame - anchor or upper - for next loop
        if ( pFrame->IsFlyFrame() )
        {
            // OD 20.08.2002 - use "static_cast" instead of "old C-cast"
            pFrame = (static_cast<const SwFlyFrame*>(pFrame))->GetAnchorFrame();
        }
        else
        {
            pFrame = pFrame->GetUpper();
        }
    } while ( pFrame );

    return false;
}

void SetOutDevAndWin( SwViewShell *pSh, OutputDevice *pO,
                      vcl::Window *pW, sal_uInt16 nZoom )
{
    pSh->mpOut = pO;
    pSh->mpWin = pW;
    pSh->mpOpt->SetZoom( nZoom );
}

Graphic SwFrameFormat::MakeGraphic( ImageMap* )
{
    return Graphic();
}

Graphic SwFlyFrameFormat::MakeGraphic( ImageMap* pMap )
{
    Graphic aRet;
    //search any Fly!
    SwIterator<SwFrame,SwFormat> aIter( *this );
    SwFrame *pFirst = aIter.First();
    SwViewShell *pSh;
    if ( pFirst && nullptr != ( pSh = pFirst->getRootFrame()->GetCurrShell()) )
    {
        SwViewShell *pOldGlobal = gProp.pSGlobalShell;
        gProp.pSGlobalShell = pSh;

        bool bNoteURL = pMap &&
            SfxItemState::SET != GetAttrSet().GetItemState( RES_URL );
        if( bNoteURL )
        {
            OSL_ENSURE( !pNoteURL, "MakeGraphic: pNoteURL already used? " );
            pNoteURL = new SwNoteURL;
        }
        SwFlyFrame *pFly = static_cast<SwFlyFrame*>(pFirst);

        OutputDevice *pOld = pSh->GetOut();
        ScopedVclPtrInstance< VirtualDevice > pDev( *pOld );
        pDev->EnableOutput( false );

        GDIMetaFile aMet;
        MapMode aMap( pOld->GetMapMode().GetMapUnit() );
        pDev->SetMapMode( aMap );
        aMet.SetPrefMapMode( aMap );

        ::SwCalcPixStatics( pSh->GetOut() );
        aMet.SetPrefSize( pFly->Frame().SSize() );

        aMet.Record( pDev.get() );
        pDev->SetLineColor();
        pDev->SetFillColor();
        pDev->SetFont( pOld->GetFont() );

        //Enlarge the rectangle if needed, so the border is painted too.
        SwRect aOut( pFly->Frame() );
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFly );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( rAttrs.CalcRightLine() )
            aOut.SSize().Width() += 2*gProp.nSPixelSzW;
        if ( rAttrs.CalcBottomLine() )
            aOut.SSize().Height()+= 2*gProp.nSPixelSzH;

        // #i92711# start Pre/PostPaint encapsulation before pOut is changed to the buffering VDev
        const vcl::Region aRepaintRegion(aOut.SVRect());
        pSh->DLPrePaint2(aRepaintRegion);

        vcl::Window *pWin = pSh->GetWin();
        sal_uInt16 nZoom = pSh->GetViewOptions()->GetZoom();
        ::SetOutDevAndWin( pSh, pDev, nullptr, 100 );
        gProp.bSFlyMetafile = true;
        gProp.pSFlyMetafileOut = pWin;

        SwViewShellImp *pImp = pSh->Imp();
        gProp.pSFlyOnlyDraw = pFly;
        gProp.pSLines = new SwLineRects;

        // OD 09.12.2002 #103045# - determine page, fly frame is on
        const SwPageFrame* pFlyPage = pFly->FindPageFrame();
        const Color aPageBackgrdColor(pFlyPage->GetDrawBackgrdColor());
        const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
        // --> OD #i76669#
        SwViewObjectContactRedirector aSwRedirector( *pSh );
        // <--
        pImp->PaintLayer( rIDDMA.GetHellId(), nullptr, aOut, &aPageBackgrdColor,
                          pFlyPage->IsRightToLeft(),
                          &aSwRedirector );
        gProp.pSLines->PaintLines( pDev, gProp );
        if ( pFly->IsFlyInContentFrame() )
            pFly->Paint( *pDev, aOut );
        gProp.pSLines->PaintLines( pDev, gProp );
        // OD 30.08.2002 #102450# - add 3rd parameter
        pImp->PaintLayer( rIDDMA.GetHeavenId(), nullptr, aOut, &aPageBackgrdColor,
                          pFlyPage->IsRightToLeft(),
                          &aSwRedirector );
        gProp.pSLines->PaintLines( pDev, gProp );
        DELETEZ( gProp.pSLines );
        gProp.pSFlyOnlyDraw = nullptr;

        gProp.pSFlyMetafileOut = nullptr;
        gProp.bSFlyMetafile = false;
        ::SetOutDevAndWin( pSh, pOld, pWin, nZoom );

        // #i92711# end Pre/PostPaint encapsulation when pOut is back and content is painted
           pSh->DLPostPaint2(true);

        aMet.Stop();
        aMet.Move( -pFly->Frame().Left(), -pFly->Frame().Top() );
        aRet = Graphic( aMet );

        if( bNoteURL )
        {
            OSL_ENSURE( pNoteURL, "MakeGraphic: Good Bye, NoteURL." );
            pNoteURL->FillImageMap( pMap, pFly->Frame().Pos(), aMap );
            delete pNoteURL;
            pNoteURL = nullptr;
        }
        gProp.pSGlobalShell = pOldGlobal;
    }
    return aRet;
}

Graphic SwDrawFrameFormat::MakeGraphic( ImageMap* )
{
    Graphic aRet;
    SwDrawModel* pMod = getIDocumentDrawModelAccess().GetDrawModel();
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
