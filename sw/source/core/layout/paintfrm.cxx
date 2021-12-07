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
#include <sfx2/printer.hxx>
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
#include <strings.hrc>
#include <swmodule.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
#include <viewimp.hxx>
#include <dflyobj.hxx>
#include <flyfrm.hxx>
#include <frmatr.hxx>
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
#include "virtoutp.hxx"
#include <lineinfo.hxx>
#include <dbg_lay.hxx>
#include <docsh.hxx>
#include <svx/svdogrp.hxx>
#include <sortedobjs.hxx>
#include <EnhancedPDFExportHelper.hxx>
#include <bodyfrm.hxx>
#include <hffrm.hxx>
#include <colfrm.hxx>
#include <sw_primitivetypes2d.hxx>
#include <swfont.hxx>

#include <svx/sdr/primitive2d/sdrframeborderprimitive2d.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentRedlineAccess.hxx>

#include <ndole.hxx>
#include <PostItMgr.hxx>
#include <FrameControlsManager.hxx>
#include <vcl/settings.hxx>

#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>

#include <svtools/borderhelper.hxx>

#include <bitmaps.hlst>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <svx/unoapi.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xfillit0.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <basegfx/utils/b2dclipstate.hxx>
#include <sal/log.hxx>

#include <memory>
#include <vector>
#include <algorithm>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <view.hxx>
#include <paintfrm.hxx>
#include <textboxhelper.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vcl/BitmapTools.hxx>
#include <comphelper/lok.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <vcl/GraphicLoader.hxx>

using namespace ::editeng;
using namespace ::com::sun::star;

namespace {

struct SwPaintProperties;

//Class declaration; here because they are only used in this file
enum class SubColFlags {
    Page     = 0x01,    //Helplines of the page
    Tab      = 0x08,   //Helplines inside tables
    Fly      = 0x10,    //Helplines inside fly frames
    Sect     = 0x20,    //Helplines inside sections
};

}

namespace o3tl {
    template<> struct typed_flags<SubColFlags> : is_typed_flags<SubColFlags, 0x39> {};
}

namespace {

// Classes collecting the border lines and help lines
class SwLineRect : public SwRect
{
    Color m_aColor;
    SvxBorderLineStyle m_nStyle;
    const SwTabFrame* m_pTabFrame;
    SubColFlags m_nSubColor; //colorize subsidiary lines
    bool m_bPainted; //already painted?
    sal_uInt8 m_nLock; //To distinguish the line and the hell layer.
public:
    SwLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderLineStyle nStyle,
                const SwTabFrame *pT , const SubColFlags nSCol );

    const Color& GetColor() const { return m_aColor; }
    SvxBorderLineStyle GetStyle() const { return m_nStyle; }
    const SwTabFrame* GetTab() const { return m_pTabFrame; }
    void SetPainted() { m_bPainted = true; }
    void Lock(bool bLock)
    {
        if (bLock)
            ++m_nLock;
        else if (m_nLock)
            --m_nLock;
    }
    bool IsPainted() const { return m_bPainted; }
    bool IsLocked() const { return m_nLock != 0; }
    SubColFlags GetSubColor() const { return m_nSubColor; }

    bool MakeUnion(const SwRect& rRect, SwPaintProperties const& properties);
};

}

#ifdef IOS
static void dummy_function()
{
    pid_t pid = getpid();
    (void) pid;
}
#endif

namespace {

class SwLineRects
{
public:
    std::vector<SwLineRect> m_aLineRects;
    typedef std::vector< SwLineRect >::const_iterator const_iterator;
    typedef std::vector< SwLineRect >::iterator iterator;
    typedef std::vector< SwLineRect >::reverse_iterator reverse_iterator;
    typedef std::vector< SwLineRect >::size_type size_type;
    size_t m_nLastCount; //avoid unnecessary cycles in PaintLines
    SwLineRects()
        : m_nLastCount(0)
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
    void AddLineRect( const SwRect& rRect,  const Color *pColor, const SvxBorderLineStyle nStyle,
                      const SwTabFrame *pTab, const SubColFlags nSCol, SwPaintProperties const &properties );
    void ConnectEdges( OutputDevice const *pOut, SwPaintProperties const &properties );
    void PaintLines  ( OutputDevice *pOut, SwPaintProperties const &properties );
    void LockLines( bool bLock );

    //Limit lines to 100
    bool isFull() const { return m_aLineRects.size() > 100; }
};

class SwSubsRects : public SwLineRects
{
    void RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects, SwPaintProperties const &properties );
public:
    void PaintSubsidiary( OutputDevice *pOut, const SwLineRects *pRects, SwPaintProperties const &properties );
};

class BorderLines
{
    drawinglayer::primitive2d::Primitive2DContainer m_Lines;
public:
    void AddBorderLines(drawinglayer::primitive2d::Primitive2DContainer&& rContainer);
    drawinglayer::primitive2d::Primitive2DContainer GetBorderLines_Clear()
    {
        drawinglayer::primitive2d::Primitive2DContainer lines;
        lines.swap(m_Lines);
        return lines;
    }
};

}

// Default zoom factor
const double aEdgeScale = 0.5;

//To optimize the expensive RetouchColor determination
Color aGlobalRetoucheColor;

namespace sw
{
Color* GetActiveRetoucheColor()
{
    return &aGlobalRetoucheColor;
}
}

namespace {

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
    // The Fly itself should certainly not be spared out. See PaintSwFrameBackground and
    // lcl_SubtractFlys()
    SwFlyFrame           *pSRetoucheFly;
    SwFlyFrame           *pSRetoucheFly2;
    SwFlyFrame           *pSFlyOnlyDraw;

    // The borders will be collected in pSLines during the Paint and later
    // possibly merge them.
    // The help lines will be collected and merged in gProp.pSSubsLines. These will
    // be compared with pSLines before the work in order to avoid help lines
    // to hide borders.
    std::unique_ptr<BorderLines> pBLines;
    std::unique_ptr<SwLineRects> pSLines;
    std::unique_ptr<SwSubsRects> pSSubsLines;

    // global variable for sub-lines of body, header, footer, section and footnote frames.
    std::unique_ptr<SwSubsRects> pSSpecSubsLines;
    SfxProgress        *pSProgress;

    // Sizes of a pixel and the corresponding halves. Will be reset when
    // entering SwRootFrame::PaintSwFrame
    tools::Long                nSPixelSzW;
    tools::Long                nSPixelSzH;
    tools::Long                nSHalfPixelSzW;
    tools::Long                nSHalfPixelSzH;
    tools::Long                nSMinDistPixelW;
    tools::Long                nSMinDistPixelH;

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

}

static SwPaintProperties gProp;

static bool isSubsidiaryLinesFlysEnabled()
{
    return !gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&
           SwViewOption::IsObjectBoundaries();
}
//other subsidiary lines enabled?
static bool isSubsidiaryLinesEnabled()
{
    return !gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsWhitespaceHidden() &&
           SwViewOption::IsDocBoundaries();
}
//subsidiary lines for sections
static bool isSubsidiaryLinesForSectionsEnabled()
{
    return !gProp.pSGlobalShell->GetViewOptions()->IsPagePreview() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
           !gProp.pSGlobalShell->GetViewOptions()->IsFormView() &&
           SwViewOption::IsSectionBoundaries();
}


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
 * For 'small' twip-to-pixel relations (less than 2:1)
 * values of <gProp.nSHalfPixelSzW> and <gProp.nSHalfPixelSzH> are set to ZERO
 */
void SwCalcPixStatics( vcl::RenderContext const *pOut )
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
    gProp.aSScaleX = double(rMap.GetScaleX());
    gProp.aSScaleY = double(rMap.GetScaleY());
}

namespace {

/**
 * To be able to save the statics so the paint is more or less reentrant
 */
class SwSavePaintStatics : public SwPaintProperties
{
public:
    SwSavePaintStatics();
    ~SwSavePaintStatics();
};

}

SwSavePaintStatics::SwSavePaintStatics()
{
    // Saving globales
    bSFlyMetafile = gProp.bSFlyMetafile;
    pSGlobalShell = gProp.pSGlobalShell;
    pSFlyMetafileOut = gProp.pSFlyMetafileOut;
    pSRetoucheFly = gProp.pSRetoucheFly;
    pSRetoucheFly2 = gProp.pSRetoucheFly2;
    pSFlyOnlyDraw = gProp.pSFlyOnlyDraw;
    pBLines = std::move(gProp.pBLines);
    pSLines = std::move(gProp.pSLines);
    pSSubsLines = std::move(gProp.pSSubsLines);
    pSSpecSubsLines = std::move(gProp.pSSpecSubsLines);
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
    gProp.pBLines             = std::move(pBLines);
    gProp.pSLines             = std::move(pSLines);
    gProp.pSSubsLines         = std::move(pSSubsLines);
    gProp.pSSpecSubsLines     = std::move(pSSpecSubsLines);
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

void BorderLines::AddBorderLines(drawinglayer::primitive2d::Primitive2DContainer&& rContainer)
{
    if(!rContainer.empty())
    {
        m_Lines.append(std::move(rContainer));
    }
}

SwLineRect::SwLineRect(const SwRect& rRect, const Color* pCol, const SvxBorderLineStyle nStyl,
                       const SwTabFrame* pT, const SubColFlags nSCol)
    : SwRect(rRect)
    , m_nStyle(nStyl)
    , m_pTabFrame(pT)
    , m_nSubColor(nSCol)
    , m_bPainted(false)
    , m_nLock(0)
{
    if ( pCol != nullptr )
        m_aColor = *pCol;
}

bool SwLineRect::MakeUnion( const SwRect &rRect, SwPaintProperties const & properties)
{
    // It has already been tested outside, whether the rectangles have
    // the same orientation (horizontal or vertical), color, etc.
    if ( Height() > Width() ) //Vertical line
    {
        if ( Left()  == rRect.Left() && Width() == rRect.Width() )
        {
            // Merge when there is no gap between the lines
            const tools::Long nAdd = properties.nSPixelSzW + properties.nSHalfPixelSzW;
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
            const tools::Long nAdd = properties.nSPixelSzW + properties.nSHalfPixelSzW;
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

void SwLineRects::AddLineRect( const SwRect &rRect, const Color *pCol, const SvxBorderLineStyle nStyle,
                               const SwTabFrame *pTab, const SubColFlags nSCol, SwPaintProperties const & properties )
{
    // Loop backwards because lines which can be combined, can usually be painted
    // in the same context
    for (reverse_iterator it = m_aLineRects.rbegin(); it != m_aLineRects.rend(); ++it)
    {
        SwLineRect &rLRect = *it;
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
    m_aLineRects.emplace_back(rRect, pCol, nStyle, pTab, nSCol);
}

void SwLineRects::ConnectEdges( OutputDevice const *pOut, SwPaintProperties const & properties )
{
    if ( pOut->GetOutDevType() != OUTDEV_PRINTER )
    {
        // I'm not doing anything for a too small zoom
        if ( properties.aSScaleX < aEdgeScale || properties.aSScaleY < aEdgeScale )
            return;
    }

    static const tools::Long nAdd = 20;

    std::vector<SwLineRect*> aCheck;

    for (size_t i = 0; i < m_aLineRects.size(); ++i)
    {
        SwLineRect& rL1 = m_aLineRects[i];
        if ( !rL1.GetTab() || rL1.IsPainted() || rL1.IsLocked() )
            continue;

        aCheck.clear();

        const bool bVert = rL1.Height() > rL1.Width();
        tools::Long nL1a, nL1b, nL1c, nL1d;

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
        for (iterator it2 = m_aLineRects.begin(); it2 != m_aLineRects.end(); ++it2)
        {
            SwLineRect &rL2 = *it2;
            if ( rL2.GetTab() != rL1.GetTab() ||
                 rL2.IsPainted()              ||
                 rL2.IsLocked()               ||
                 (bVert == (rL2.Height() > rL2.Width())) )
                continue;

            tools::Long nL2a, nL2b, nL2c, nL2d;
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
                            if ( !rL1.Contains( aIns ) )
                                continue;
                            m_aLineRects.emplace_back(aIns, &rL1.GetColor(),
                                                      SvxBorderLineStyle::SOLID, rL1.GetTab(),
                                                      SubColFlags::Tab);
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
                            if ( !rL1.Contains( aIns ) )
                                continue;
                            m_aLineRects.emplace_back(aIns, &rL1.GetColor(),
                                                      SvxBorderLineStyle::SOLID, rL1.GetTab(),
                                                      SubColFlags::Tab);
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
            m_aLineRects.erase(m_aLineRects.begin() + i);
            --i;
        }
    }
}

void SwSubsRects::RemoveSuperfluousSubsidiaryLines( const SwLineRects &rRects, SwPaintProperties const & properties )
{
    // All help lines that are covered by any border will be removed or split
    for (size_t i = 0; i < m_aLineRects.size(); ++i)
    {
        // get a copy instead of a reference, because an <insert> may destroy
        // the object due to a necessary array resize.
        const SwLineRect aSubsLineRect(m_aLineRects[i]);

        // add condition <aSubsLineRect.IsLocked()> in order to consider only
        // border lines, which are *not* locked.
        if ( aSubsLineRect.IsPainted() ||
             aSubsLineRect.IsLocked() )
            continue;

        const bool bVerticalSubs = aSubsLineRect.Height() > aSubsLineRect.Width();
        SwRect aSubsRect( aSubsLineRect );
        if ( bVerticalSubs )
        {
            aSubsRect.AddLeft  ( - (properties.nSPixelSzW+properties.nSHalfPixelSzW) );
            aSubsRect.AddRight ( properties.nSPixelSzW+properties.nSHalfPixelSzW );
        }
        else
        {
            aSubsRect.AddTop   ( - (properties.nSPixelSzH+properties.nSHalfPixelSzH) );
            aSubsRect.AddBottom( properties.nSPixelSzH+properties.nSHalfPixelSzH );
        }
        for (const_iterator itK = rRects.m_aLineRects.begin(); itK != rRects.m_aLineRects.end();
             ++itK)
        {
            const SwLineRect &rLine = *itK;

            // do *not* consider painted or locked border lines.
            // #i1837# - locked border lines have to be considered.
            if ( rLine.IsLocked () )
                continue;

            if ( !bVerticalSubs == ( rLine.Height() > rLine.Width() ) ) //same direction?
                continue;

            if ( aSubsRect.Overlaps( rLine ) )
            {
                if ( bVerticalSubs ) // Vertical?
                {
                    if ( aSubsRect.Left()  <= rLine.Right() &&
                         aSubsRect.Right() >= rLine.Left() )
                    {
                        tools::Long nTmp = rLine.Top()-(properties.nSPixelSzH+1);
                        if ( aSubsLineRect.Top() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Bottom( nTmp );
                            m_aLineRects.emplace_back(aNewSubsRect, nullptr,
                                                      aSubsLineRect.GetStyle(), nullptr,
                                                      aSubsLineRect.GetSubColor());
                        }
                        nTmp = rLine.Bottom()+properties.nSPixelSzH+1;
                        if ( aSubsLineRect.Bottom() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Top( nTmp );
                            m_aLineRects.emplace_back(aNewSubsRect, nullptr,
                                                      aSubsLineRect.GetStyle(), nullptr,
                                                      aSubsLineRect.GetSubColor());
                        }
                        m_aLineRects.erase(m_aLineRects.begin() + i);
                        --i;
                        break;
                    }
                }
                else // Horizontal
                {
                    if ( aSubsRect.Top() <= rLine.Bottom() &&
                         aSubsRect.Bottom() >= rLine.Top() )
                    {
                        tools::Long nTmp = rLine.Left()-(properties.nSPixelSzW+1);
                        if ( aSubsLineRect.Left() < nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Right( nTmp );
                            m_aLineRects.emplace_back(aNewSubsRect, nullptr,
                                                      aSubsLineRect.GetStyle(), nullptr,
                                                      aSubsLineRect.GetSubColor());
                        }
                        nTmp = rLine.Right()+properties.nSPixelSzW+1;
                        if ( aSubsLineRect.Right() > nTmp )
                        {
                            SwRect aNewSubsRect( aSubsLineRect );
                            aNewSubsRect.Left( nTmp );
                            m_aLineRects.emplace_back(aNewSubsRect, nullptr,
                                                      aSubsLineRect.GetStyle(), nullptr,
                                                      aSubsLineRect.GetSubColor());
                        }
                        m_aLineRects.erase(m_aLineRects.begin() + i);
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
    for (SwLineRect& rLRect : m_aLineRects)
        rLRect.Lock(bLock);
}

static void lcl_DrawDashedRect( OutputDevice * pOut, SwLineRect const & rLRect )
{
    tools::Long startX = rLRect.Left(  ), endX;
    tools::Long startY = rLRect.Top(  ),  endY;

    // Discriminate vertically stretched rect from horizontally stretched
    // and restrict minimum nHalfLWidth to 1
    tools::Long nHalfLWidth = std::max( std::min( rLRect.Width(  ), rLRect.Height(  ) ) / 2, tools::Long(1) );

    if ( rLRect.Height(  ) > rLRect.Width(  ) )
    {
        startX += nHalfLWidth;
        endX = startX;
        endY = startY + rLRect.Height(  );
    }
    else
    {
        startY += nHalfLWidth;
        endY = startY;
        endX = startX + rLRect.Width(  );
    }

    svtools::DrawLine( *pOut, Point( startX, startY ), Point( endX, endY ),
            sal_uInt32( nHalfLWidth * 2 ), rLRect.GetStyle( ) );
}

void SwLineRects::PaintLines( OutputDevice *pOut, SwPaintProperties const &properties )
{
    // Paint the borders. Sadly two passes are needed.
    // Once for the inside and once for the outside edges of tables
    if (m_aLineRects.size() == m_nLastCount)
        return;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pOut );

    pOut->Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
    pOut->SetFillColor();
    pOut->SetLineColor();
    ConnectEdges( pOut, properties );
    const Color *pLast = nullptr;

    bool bPaint2nd = false;
    size_t nMinCount = m_aLineRects.size();

    for (size_t i = 0; i < m_aLineRects.size(); ++i)
    {
        SwLineRect& rLRect = m_aLineRects[i];

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
                        nTLeft  = rLRect.GetTab()->getFrameArea().Left() + rLRect.GetTab()->getFramePrintArea().Left(),
                        nTRight = rLRect.GetTab()->getFrameArea().Left() + rLRect.GetTab()->getFramePrintArea().Right();
                if ( (nTLeft >= nLLeft && nTLeft <= nLRight) ||
                     (nTRight>= nLLeft && nTRight<= nLRight) )
                    bPaint = false;
            }
            else
            {
                // Horizontal edge, overlapping with the table edge?
                SwTwips nLTop    = rLRect.Top()    - 30,
                        nLBottom = rLRect.Bottom() + 30,
                        nTTop    = rLRect.GetTab()->getFrameArea().Top()  + rLRect.GetTab()->getFramePrintArea().Top(),
                        nTBottom = rLRect.GetTab()->getFrameArea().Top()  + rLRect.GetTab()->getFramePrintArea().Bottom();
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
        for (size_t i = 0; i < m_aLineRects.size(); ++i)
        {
            SwLineRect& rLRect = m_aLineRects[i];
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
    m_nLastCount = nMinCount;
    pOut->Pop();

}

void SwSubsRects::PaintSubsidiary( OutputDevice *pOut,
                                   const SwLineRects *pRects,
                                   SwPaintProperties const & properties )
{
    if (m_aLineRects.empty())
        return;

    // #i16816# tagged pdf support
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, nullptr, nullptr, *pOut );

    // Remove all help line that are almost covered (tables)
    for (size_type i = 0; i != m_aLineRects.size(); ++i)
    {
        SwLineRect& rLi = m_aLineRects[i];
        const bool bVerticalSubs = rLi.Height() > rLi.Width();

        for (size_type k = i + 1; k != m_aLineRects.size(); ++k)
        {
            SwLineRect& rLk = m_aLineRects[k];
            if ( rLi.SSize() == rLk.SSize() )
            {
                if ( bVerticalSubs == ( rLk.Height() > rLk.Width() ) )
                {
                    if ( bVerticalSubs )
                    {
                        tools::Long nLi = rLi.Right();
                        tools::Long nLk = rLk.Right();
                        if ( rLi.Top() == rLk.Top() &&
                             ((nLi < rLk.Left() && nLi+21 > rLk.Left()) ||
                              (nLk < rLi.Left() && nLk+21 > rLi.Left())))
                        {
                            m_aLineRects.erase(m_aLineRects.begin() + i);
                            // don't continue with inner loop any more:
                            // the array may shrink!
                            --i;
                            break;
                        }
                    }
                    else
                    {
                        tools::Long nLi = rLi.Bottom();
                        tools::Long nLk = rLk.Bottom();
                        if ( rLi.Left() == rLk.Left() &&
                             ((nLi < rLk.Top() && nLi+21 > rLk.Top()) ||
                              (nLk < rLi.Top() && nLk+21 > rLi.Top())))
                        {
                            m_aLineRects.erase(m_aLineRects.begin() + i);
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

    if (pRects && (!pRects->m_aLineRects.empty()))
        RemoveSuperfluousSubsidiaryLines( *pRects, properties );

    if (m_aLineRects.empty())
        return;

    pOut->Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
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

    for (SwLineRect& rLRect : m_aLineRects)
    {
        // Add condition <!rLRect.IsLocked()> to prevent paint of locked subsidiary lines.
        if ( !rLRect.IsPainted() &&
             !rLRect.IsLocked() )
        {
            const Color *pCol = nullptr;
            switch ( rLRect.GetSubColor() )
            {
                case SubColFlags::Page: pCol = &SwViewOption::GetDocBoundariesColor(); break;
                case SubColFlags::Fly: pCol = &SwViewOption::GetObjectBoundariesColor(); break;
                case SubColFlags::Tab: pCol = &SwViewOption::GetTableBoundariesColor(); break;
                case SubColFlags::Sect: pCol = &SwViewOption::GetSectionBoundColor(); break;
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
    const tools::Rectangle aOrgPxRect = pOut->LogicToPixel( rRect.SVRect() );
    // Determine pixel-center rectangle in twip
    const SwRect aPxCenterRect( pOut->PixelToLogic( aOrgPxRect ) );

    // Perform adjustments on pixel level.
    SwRect aAlignedPxRect( aOrgPxRect );
    if ( rRect.Top() > aPxCenterRect.Top() )
    {
        // 'leave pixel overlapping on top'
        aAlignedPxRect.AddTop( 1 );
    }

    if ( rRect.Bottom() < aPxCenterRect.Bottom() )
    {
        // 'leave pixel overlapping on bottom'
        aAlignedPxRect.AddBottom( - 1 );
    }

    if ( rRect.Left() > aPxCenterRect.Left() )
    {
        // 'leave pixel overlapping on left'
        aAlignedPxRect.AddLeft( 1 );
    }

    if ( rRect.Right() < aPxCenterRect.Right() )
    {
        // 'leave pixel overlapping on right'
        aAlignedPxRect.AddRight( - 1 );
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

    rRect = SwRect(pOut->PixelToLogic( aAlignedPxRect.SVRect() ));

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
    tools::Rectangle aPxRect = rOut.LogicToPixel( pGrfRect->SVRect() );
    pGrfRect->Pos( rOut.PixelToLogic( aPxRect.TopLeft() ) );
    pGrfRect->SSize( rOut.PixelToLogic( aPxRect.GetSize() ) );
}

static tools::Long lcl_AlignWidth( const tools::Long nWidth, SwPaintProperties const & properties )
{
    if ( nWidth )
    {
        const tools::Long nW = nWidth % properties.nSPixelSzW;

        if ( !nW || nW > properties.nSHalfPixelSzW )
            return std::max(tools::Long(1), nWidth - properties.nSHalfPixelSzW);
    }
    return nWidth;
}

static tools::Long lcl_AlignHeight( const tools::Long nHeight, SwPaintProperties const & properties )
{
    if ( nHeight )
    {
        const tools::Long nH = nHeight % properties.nSPixelSzH;

        if ( !nH || nH > properties.nSHalfPixelSzH )
            return std::max(tools::Long(1), nHeight - properties.nSHalfPixelSzH);
    }
    return nHeight;
}

/**
 * Calculate PrtArea plus surrounding plus shadow
 */
static void lcl_CalcBorderRect( SwRect &rRect, const SwFrame *pFrame,
                                        const SwBorderAttrs &rAttrs,
                                        const bool bShadow,
                                        SwPaintProperties const & properties)
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
        rRect = pFrame->getFramePrintArea();
        rRect.Pos() += pFrame->getFrameArea().Pos();
    }
    else if ( pFrame->IsCellFrame() )
        rRect = pFrame->getFrameArea();
    else
    {
        rRect = pFrame->getFramePrintArea();
        rRect.Pos() += pFrame->getFrameArea().Pos();

        SwRectFn fnRect = pFrame->IsVertical() ? ( pFrame->IsVertLR() ? (pFrame->IsVertLRBT() ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert ) : fnRectHori;

        const SvxBoxItem &rBox = rAttrs.GetBox();
        const bool bTop = 0 != (pFrame->*fnRect->fnGetTopMargin)();
        if ( bTop )
        {
            SwTwips nDiff = rBox.GetTop() ?
                rBox.CalcLineSpace( SvxBoxItemLine::TOP ) :
                rBox.GetDistance( SvxBoxItemLine::TOP );
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
                    rBox.GetDistance( SvxBoxItemLine::BOTTOM );
            }
            if( nDiff )
                (rRect.*fnRect->fnAddBottom)( nDiff );
        }

        if ( rBox.GetLeft() )
            (rRect.*fnRect->fnSubLeft)( rBox.CalcLineSpace( SvxBoxItemLine::LEFT ) );
        else
            (rRect.*fnRect->fnSubLeft)( rBox.GetDistance( SvxBoxItemLine::LEFT ) );

        if ( rBox.GetRight() )
            (rRect.*fnRect->fnAddRight)( rBox.CalcLineSpace( SvxBoxItemLine::RIGHT ) );
        else
            (rRect.*fnRect->fnAddRight)( rBox.GetDistance( SvxBoxItemLine::RIGHT ) );

        if ( bShadow && rAttrs.GetShadow().GetLocation() != SvxShadowLocation::NONE )
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

/// Returns a range suitable for subtraction when lcl_SubtractFlys() is used.
/// Otherwise DrawFillAttributes() expands the clip path itself.
static basegfx::B2DRange lcl_ShrinkFly(const SwRect& rRect)
{
    static MapMode aMapMode(MapUnit::MapTwip);
    static const Size aSingleUnit = Application::GetDefaultDevice()->PixelToLogic(Size(1, 1), aMapMode);

    double x1 = rRect.Left() + aSingleUnit.getWidth();
    double y1 = rRect.Top() + aSingleUnit.getHeight();
    double x2 = rRect.Right() - aSingleUnit.getWidth();
    double y2 = rRect.Bottom() - aSingleUnit.getHeight();

    return basegfx::B2DRange(x1, y1, x2, y2);
}

static void lcl_SubtractFlys( const SwFrame *pFrame, const SwPageFrame *pPage,
   const SwRect &rRect, SwRegionRects &rRegion, basegfx::utils::B2DClipState& rClipState, SwPaintProperties const & rProperties)
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

        const SwFlyFrame *pFly = pAnchoredObj->DynCastFlyFrame();
        if (!pFly)
            continue;

        if (pSelfFly == pFly || gProp.pSRetoucheFly == pFly || !rRect.Overlaps(pFly->getFrameArea()))
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
                pFly->GetFormat()->IsBackgroundBrushInherited()
               )
            {
                SwRect aRect;
                SwBorderAttrAccess aAccess( SwFrame::GetCache(), static_cast<SwFrame const *>(pFly) );
                const SwBorderAttrs &rAttrs = *aAccess.Get();
                ::lcl_CalcBorderRect( aRect, pFly, rAttrs, true, rProperties );
                rRegion -= aRect;
                rClipState.subtractRange(lcl_ShrinkFly(aRect));
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
            rClipState.subtractRange(lcl_ShrinkFly(aRect));
        }
        else
        {
            SwRect aRect( pFly->getFramePrintArea() );
            aRect += pFly->getFrameArea().Pos();
            rRegion -= aRect;
            rClipState.subtractRange(lcl_ShrinkFly(aRect));
        }
    }
    if (gProp.pSRetoucheFly == gProp.pSRetoucheFly2)
        gProp.pSRetoucheFly = nullptr;
}

static void lcl_implDrawGraphicBackground(const SvxBrushItem& _rBackgrdBrush,
                                       vcl::RenderContext& _rOut,
                                       const SwRect& _rAlignedPaintRect,
                                       const GraphicObject& _rGraphicObj,
                                       SwPaintProperties const & properties)
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
    if ( aColor.IsTransparent() )
    ///     background color is transparent --> draw transparent.
    {
        bDrawTransparent = true;
        nTransparencyPercent = ((255 - aColor.GetAlpha())*100 + 0x7F)/0xFF;
    }
    else if ( (_rGraphicObj.GetAttr().IsTransparent()) &&
                (_rBackgrdBrush.GetColor() == COL_TRANSPARENT) )
    ///     graphic is drawn transparent and background color is
    ///     "no fill"/"auto fill" --> draw transparent
    {
        bDrawTransparent = true;
        nTransparencyPercent = 100 - (_rGraphicObj.GetAttr().GetAlpha() * 100 + 127) / 255;
    }

    if ( bDrawTransparent )
    {
        /// draw background transparent
        if( _rOut.GetFillColor() != aColor.GetRGBColor() )
            _rOut.SetFillColor( aColor.GetRGBColor() );
        tools::PolyPolygon aPoly( _rAlignedPaintRect.SVRect() );
        _rOut.DrawTransparent( aPoly, nTransparencyPercent );
    }
    else
    {
        /// draw background opaque
        if ( _rOut.GetFillColor() != aColor )
            _rOut.SetFillColor( aColor );
        _rOut.DrawRect( _rAlignedPaintRect.SVRect() );
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
 * Method implemented as an inline, checking the conditions and calling method
 * method <lcl_implDrawGraphicBackground(..)> for the intrinsic drawing.
 *
 * @param _rBackgrdBrush
 * background brush contain the color the background has to be drawn.
 *
 * @param _rOut
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
static void lcl_DrawGraphicBackground( const SvxBrushItem& _rBackgrdBrush,
                                    OutputDevice& _rOut,
                                    const SwRect& _rAlignedPaintRect,
                                    const GraphicObject& _rGraphicObj,
                                    bool _bNumberingGraphic,
                                    SwPaintProperties const & properties,
                                    bool _bBackgrdAlreadyDrawn = false)
{
    // draw background with background color, if
    //     (1) graphic is not used as a numbering AND
    //     (2) background is not already drawn AND
    //     (3) intrinsic graphic is transparent OR intrinsic graphic doesn't exists
    if ( !_bNumberingGraphic &&
         !_bBackgrdAlreadyDrawn &&
         ( _rGraphicObj.IsTransparent() || _rGraphicObj.GetType() == GraphicType::NONE  )
       )
    {
        lcl_implDrawGraphicBackground( _rBackgrdBrush, _rOut, _rAlignedPaintRect, _rGraphicObj, properties );
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
 * with a background color in method <lcl_DrawGraphicBackground>
 *
 * Also, change type of <bGrfNum> and <bClip> from <bool> to <bool>
 */
static void lcl_DrawGraphic( const SvxBrushItem& rBrush, vcl::RenderContext &rOutDev,
                      const SwViewShell &rSh, const SwRect &rGrf, const SwRect &rOut,
                      bool bGrfNum,
                      SwPaintProperties const & properties,
                      bool bBackgrdAlreadyDrawn )
                      // add parameter <bBackgrdAlreadyDrawn> to indicate
                      // that the background is already drawn.
{
    // Calculate align rectangle from parameter <rGrf> and use aligned
    // rectangle <aAlignedGrfRect> in the following code
    SwRect aAlignedGrfRect = rGrf;
    ::SwAlignRect( aAlignedGrfRect, &rSh, &rOutDev );

    // Change type from <bool> to <bool>.
    const bool bNotInside = !rOut.Contains( aAlignedGrfRect );
    if ( bNotInside )
    {
        rOutDev.Push( vcl::PushFlags::CLIPREGION );
        rOutDev.IntersectClipRegion( rOut.SVRect() );
    }

    GraphicObject *pGrf = const_cast<GraphicObject*>(rBrush.GetGraphicObject());

    OUString aOriginURL = pGrf->GetGraphic().getOriginURL();
    if (pGrf->GetGraphic().GetType() == GraphicType::Default && !aOriginURL.isEmpty())
    {
        Graphic aGraphic = vcl::graphic::loadFromURL(aOriginURL);
        pGrf->SetGraphic(aGraphic);
    }

    // Outsource drawing of background with a background color
    ::lcl_DrawGraphicBackground( rBrush, rOutDev, aAlignedGrfRect, *pGrf, bGrfNum, properties, bBackgrdAlreadyDrawn );

    // Because for drawing a graphic left-top-corner and size coordinates are
    // used, these coordinates have to be determined on pixel level.
    ::SwAlignGrfRect( &aAlignedGrfRect, rOutDev );

    const basegfx::B2DHomMatrix aGraphicTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            aAlignedGrfRect.Width(), aAlignedGrfRect.Height(),
            aAlignedGrfRect.Left(), aAlignedGrfRect.Top()));

    paintGraphicUsingPrimitivesHelper(
        rOutDev,
        *pGrf,
        pGrf->GetAttr(),
        aGraphicTransform,
        OUString(),
        OUString(),
        OUString());

    if ( bNotInside )
        rOutDev.Pop();
}

bool DrawFillAttributes(
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const SwRect& rOriginalLayoutRect,
    const SwRegionRects& rPaintRegion,
    const basegfx::utils::B2DClipState& rClipState,
    vcl::RenderContext& rOut)
{
    if(rFillAttributes && rFillAttributes->isUsed())
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
            // need to expand for correct AAed and non-AAed visualization as primitive.
            // This must probably be removed again when we will be able to get all Writer visualization
            // as primitives and Writer prepares all it's stuff in high precision coordinates (also
            // needs to avoid moving boundaries around to better show overlapping stuff...)
            if(SvtOptionsDrawinglayer::IsAntiAliasing())
            {
                // if AAed in principle expand by 0.5 in all directions. Since painting edges of
                // AAed regions does not add to no transparence (0.5 opacity covered by 0.5 opacity
                // is not full opacity but 0.75 opacity) we need some overlap here to avoid paint
                // artifacts. Checked experimentally - a little bit more in Y is needed, probably
                // due to still existing integer alignment and crunching in writer.
                static const double fExpandX = 0.55;
                static const double fExpandY = 0.70;
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
                    basegfx::B2DPolyPolygon const& maskRegion(rClipState.getClipPoly());
                    primitives.resize(1);
                    primitives[0] = new drawinglayer::primitive2d::MaskPrimitive2D(
                            maskRegion, drawinglayer::primitive2d::Primitive2DContainer(rSequence));
                    pPrimitives = &primitives;
                }
                assert(pPrimitives && pPrimitives->size());

                const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                    basegfx::B2DHomMatrix(),
                    rOut.GetViewTransformation(),
                    aPaintRange,
                    nullptr,
                    0.0);
                std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                    rOut,
                    aViewInformation2D) );
                pProcessor->process(*pPrimitives);
                return true;
            }
        }
    }

    return false;
}

void DrawGraphic(
    const SvxBrushItem *pBrush,
    vcl::RenderContext &rOutDev,
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
            OUString referer;
            SfxObjectShell * sh = rSh.GetDoc()->GetPersist();
            if (sh != nullptr && sh->HasName()) {
                referer = sh->GetMedium()->GetName();
            }
            const Graphic* pGrf = pBrush->GetGraphic(referer);
            if( pGrf && GraphicType::NONE != pGrf->GetType() )
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
        aGrf.Pos().setY( rOrg.Top() );
        aGrf.Pos().setX( rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2 );
        break;

    case GPOS_RT:
        aGrf.Pos().setY( rOrg.Top() );
        aGrf.Pos().setX( rOrg.Right() - aGrfSize.Width() );
        break;

    case GPOS_LM:
        aGrf.Pos().setY( rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2 );
        aGrf.Pos().setX( rOrg.Left() );
        break;

    case GPOS_MM:
        aGrf.Pos().setY( rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2 );
        aGrf.Pos().setX( rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2 );
        break;

    case GPOS_RM:
        aGrf.Pos().setY( rOrg.Top() + rOrg.Height()/2 - aGrfSize.Height()/2 );
        aGrf.Pos().setX( rOrg.Right() - aGrfSize.Width() );
        break;

    case GPOS_LB:
        aGrf.Pos().setY( rOrg.Bottom() - aGrfSize.Height() );
        aGrf.Pos().setX( rOrg.Left() );
        break;

    case GPOS_MB:
        aGrf.Pos().setY( rOrg.Bottom() - aGrfSize.Height() );
        aGrf.Pos().setX( rOrg.Left() + rOrg.Width()/2 - aGrfSize.Width()/2 );
        break;

    case GPOS_RB:
        aGrf.Pos().setY( rOrg.Bottom() - aGrfSize.Height() );
        aGrf.Pos().setX( rOrg.Right() - aGrfSize.Width() );
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
        bRetouche = !rOut.Contains( aGrf );
        break;

    case GPOS_TILED:
        {
            // draw background of tiled graphic before drawing tiled graphic in loop
            // determine graphic object
            GraphicObject* pGraphicObj = const_cast< GraphicObject* >(pBrush->GetGraphicObject());
            // calculate aligned paint rectangle
            SwRect aAlignedPaintRect = rOut;
            ::SwAlignRect( aAlignedPaintRect, &rSh, &rOutDev );
            // draw background color for aligned paint rectangle
            lcl_DrawGraphicBackground( *pBrush, rOutDev, aAlignedPaintRect, *pGraphicObj, bGrfNum, gProp );

            // set left-top-corner of background graphic to left-top-corner of the
            // area, from which the background brush is determined.
            aGrf.Pos() = rOrg.Pos();
            // setup clipping at output device
            rOutDev.Push( vcl::PushFlags::CLIPREGION );
            rOutDev.IntersectClipRegion( rOut.SVRect() );
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

                pGraphicObj->DrawTiled( rOutDev,
                                        aAlignedPaintRect.SVRect(),
                                        aGrf.SSize(),
                                        Size( aPaintOffset.X(), aPaintOffset.Y() ),
                                        std::max( 128, static_cast<int>( sqrt(sqrt( Abitmap)) + .5 ) ) );
            }
            // reset clipping at output device
            rOutDev.Pop();
            // set <bDraw> and <bRetouche> to false, indicating that background
            // graphic and background are already drawn.
            bDraw = bRetouche = false;
        }
        break;

    case GPOS_NONE:
        bDraw = false;
        break;

    default: OSL_ENSURE( false, "new Graphic position?" );
    }

    /// init variable <bGrfBackgrdAlreadDrawn> to indicate, if background of
    /// graphic is already drawn or not.
    bool bGrfBackgrdAlreadyDrawn = false;
    if ( bRetouche )
    {
        rOutDev.Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
        rOutDev.SetLineColor();

        // check, if an existing background graphic (not filling the complete
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
                if ( (aGrfAttr.IsTransparent()) &&
                     (pBrush->GetColor() == COL_TRANSPARENT)
                   )
                {
                    bTransparentGrfWithNoFillBackgrd = true;
                    nGrfTransparency = 255 - aGrfAttr.GetAlpha();
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
                            ( (pBrush->GetColor() != COL_TRANSPARENT) ||
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
                ( ( aColor.IsTransparent()) ||
                bTransparentGrfWithNoFillBackgrd ) )
        {
            eDrawStyle = Transparent;
        }

        // #i75614# reset draw mode in high contrast mode in order to get fill color set
        const DrawModeFlags nOldDrawMode = rOutDev.GetDrawMode();
        if ( gProp.pSGlobalShell->GetWin() &&
             Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            rOutDev.SetDrawMode( DrawModeFlags::Default );
        }

        // If background region has to be drawn transparent, set only the RGB values of the background color as
        // the fill color for the output device.
        switch (eDrawStyle)
        {
            case Transparent:
            {
                if( rOutDev.GetFillColor() != aColor.GetRGBColor() )
                    rOutDev.SetFillColor( aColor.GetRGBColor() );
                break;
            }
            default:
            {
                if( rOutDev.GetFillColor() != aColor )
                    rOutDev.SetFillColor( aColor );
                break;
            }
        }

        // #i75614#
        // restore draw mode
        rOutDev.SetDrawMode( nOldDrawMode );

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
                        // Consider only that part of the graphic area that is overlapping with draw area.
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
                  (( bTransparentGrfWithNoFillBackgrd ? nGrfTransparency : (255 - aColor.GetAlpha())
                   )*100 + 0x7F)/0xFF);
                // draw poly-polygon transparent
                rOutDev.DrawTransparent( aDrawPoly, nTransparencyPercent );

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
                    rOutDev.DrawRect( aRegion[i].SVRect() );
                }
            }
        }
        rOutDev.Pop();
    }

    if( bDraw && aGrf.Overlaps( rOut ) )
        lcl_DrawGraphic( *pBrush, rOutDev, rSh, aGrf, rOut, bGrfNum, gProp,
                         bGrfBackgrdAlreadyDrawn );

    if( bReplaceGrfNum )
    {
        const BitmapEx& rBmp = rSh.GetReplacementBitmap(false);
        vcl::Font aTmp( rOutDev.GetFont() );
        Graphic::DrawEx(rOutDev, OUString(), aTmp, rBmp, rOrg.Pos(), rOrg.SSize());
    }
}

/**
 * Local helper for SwRootFrame::PaintSwFrame(..) - Adjust given rectangle to pixel size
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
    const tools::Rectangle aPxCenterRect = aOut.PixelToLogic(
            aOut.LogicToPixel( io_aSwRect.SVRect() ) );

    // local constant object of class <Rectangle> representing given rectangle
    // in pixel.
    const tools::Rectangle aOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );

    // calculate adjusted rectangle from pixel centered rectangle.
    // Due to rounding differences <aPxCenterRect> doesn't exactly represents
    // the Twip-centers. Thus, adjust borders by half of pixel width/height plus 1.
    // Afterwards, adjust calculated Twip-positions of the all borders.
    tools::Rectangle aSizedRect = aPxCenterRect;
    aSizedRect.AdjustLeft( -(aTwipToPxSize.Width()/2 + 1) );
    aSizedRect.AdjustRight( aTwipToPxSize.Width()/2 + 1 );
    aSizedRect.AdjustTop( -(aTwipToPxSize.Height()/2 + 1) );
    aSizedRect.AdjustBottom(aTwipToPxSize.Height()/2 + 1);

    // adjust left()
    while ( aOut.LogicToPixel(aSizedRect).Left() < aOrgPxRect.Left() )
    {
        aSizedRect.AdjustLeft( 1 );
    }
    // adjust right()
    while ( aOut.LogicToPixel(aSizedRect).Right() > aOrgPxRect.Right() )
    {
        aSizedRect.AdjustRight( -1 );
    }
    // adjust top()
    while ( aOut.LogicToPixel(aSizedRect).Top() < aOrgPxRect.Top() )
    {
        aSizedRect.AdjustTop( 1 );
    }
    // adjust bottom()
    while ( aOut.LogicToPixel(aSizedRect).Bottom() > aOrgPxRect.Bottom() )
    {
        aSizedRect.AdjustBottom( -1 );
    }

    io_aSwRect = SwRect( aSizedRect );

#if OSL_DEBUG_LEVEL > 0
    tools::Rectangle aTestOrgPxRect = aOut.LogicToPixel( io_aSwRect.SVRect() );
    tools::Rectangle aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect == aTestNewPxRect,
            "Error in lcl_AlignRectToPixelSize(..): Adjusted rectangle has incorrect position or size");
    // check Left()
    aSizedRect.AdjustLeft( -1 );
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Left() >= (aTestNewPxRect.Left()+1),
            "Error in lcl_AlignRectToPixelSize(..): Left() not correct adjusted");
    aSizedRect.AdjustLeft( 1 );
    // check Right()
    aSizedRect.AdjustRight( 1 );
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Right() <= (aTestNewPxRect.Right()-1),
            "Error in lcl_AlignRectToPixelSize(..): Right() not correct adjusted");
    aSizedRect.AdjustRight( -1 );
    // check Top()
    aSizedRect.AdjustTop( -1 );
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Top() >= (aTestNewPxRect.Top()+1),
            "Error in lcl_AlignRectToPixelSize(..): Top() not correct adjusted");
    aSizedRect.AdjustTop( 1 );
    // check Bottom()
    aSizedRect.AdjustBottom( 1 );
    aTestNewPxRect = aOut.LogicToPixel( aSizedRect );
    OSL_ENSURE( aTestOrgPxRect.Bottom() <= (aTestNewPxRect.Bottom()-1),
            "Error in lcl_AlignRectToPixelSize(..): Bottom() not correct adjusted");
    aSizedRect.AdjustBottom( -1 );
#endif
}

// FUNCTIONS USED FOR COLLAPSING TABLE BORDER LINES START

namespace {

struct SwLineEntry
{
    SwTwips mnKey;
    SwTwips mnStartPos;
    SwTwips mnEndPos;
    SwTwips mnLimitedEndPos;

    svx::frame::Style maAttribute;

    enum OverlapType { NO_OVERLAP, OVERLAP1, OVERLAP2, OVERLAP3 };

    enum class VerticalType { LEFT, RIGHT };

public:
    SwLineEntry( SwTwips nKey,
                 SwTwips nStartPos,
                 SwTwips nEndPos,
                 const svx::frame::Style& rAttribute );

    OverlapType Overlaps( const SwLineEntry& rComp ) const;

    /**
     * Assuming that this entry is for a Word-style covering cell and the border matching eType is
     * set, limit the end position of this border in case covered cells have no borders set.
     */
    void LimitVerticalEndPos(const SwFrame& rFrame, VerticalType eType);
};

}

SwLineEntry::SwLineEntry( SwTwips nKey,
                          SwTwips nStartPos,
                          SwTwips nEndPos,
                          const svx::frame::Style& rAttribute )
    :   mnKey( nKey ),
        mnStartPos( nStartPos ),
        mnEndPos( nEndPos ),
        mnLimitedEndPos(0),
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
    else if (mnStartPos <= rNew.mnStartPos)
        eRet = OVERLAP2;

    // 8, 9
    return eRet;
}

void SwLineEntry::LimitVerticalEndPos(const SwFrame& rFrame, VerticalType eType)
{
    if (!rFrame.IsCellFrame())
    {
        return;
    }

    const auto& rCellFrame = static_cast<const SwCellFrame&>(rFrame);
    std::vector<const SwCellFrame*> aCoveredCells = rCellFrame.GetCoveredCells();
    // Iterate in reverse order, so we can stop at the first cell that has a border. This can
    // determine what is the minimal end position that is safe to use as a limit.
    for (auto it = aCoveredCells.rbegin(); it != aCoveredCells.rend(); ++it)
    {
        const SwCellFrame* pCoveredCell = *it;
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pCoveredCell );
        const SwBorderAttrs& rAttrs = *aAccess.Get();
        const SvxBoxItem& rBox = rAttrs.GetBox();
        if (eType == VerticalType::LEFT && rBox.GetLeft())
        {
            break;
        }

        if (eType == VerticalType::RIGHT && rBox.GetRight())
        {
            break;
        }

        mnLimitedEndPos = pCoveredCell->getFrameArea().Top();
    }
}

namespace {

struct lt_SwLineEntry
{
    bool operator()( const SwLineEntry& e1, const SwLineEntry& e2 ) const
    {
        return e1.mnStartPos < e2.mnStartPos;
    }
};

}

typedef std::set< SwLineEntry, lt_SwLineEntry > SwLineEntrySet;
typedef std::map< SwTwips, SwLineEntrySet > SwLineEntryMap;

namespace {

class SwTabFramePainter
{
    SwLineEntryMap maVertLines;
    SwLineEntryMap maHoriLines;
    const SwTabFrame& mrTabFrame;

    void Insert( SwLineEntry&, bool bHori );
    void Insert(const SwFrame& rFrame, const SvxBoxItem& rBoxItem, const SwRect &rPaintArea);
    void HandleFrame(const SwLayoutFrame& rFrame, const SwRect& rPaintArea);
    void FindStylesForLine( const Point&,
                            const Point&,
                            svx::frame::Style*,
                            bool bHori ) const;

public:
    explicit SwTabFramePainter( const SwTabFrame& rTabFrame );

    void PaintLines( OutputDevice& rDev, const SwRect& rRect ) const;
};

}

SwTabFramePainter::SwTabFramePainter( const SwTabFrame& rTabFrame )
    : mrTabFrame( rTabFrame )
{
    SwRect aPaintArea = rTabFrame.GetUpper()->GetPaintArea();
    HandleFrame(rTabFrame, aPaintArea);
}

void SwTabFramePainter::HandleFrame(const SwLayoutFrame& rLayoutFrame, const SwRect& rPaintArea)
{
    // Add border lines of cell frames. Skip covered cells. Skip cells
    // in special row span row, which do not have a negative row span:
    if ( rLayoutFrame.IsCellFrame() && !rLayoutFrame.IsCoveredCell() )
    {
        const SwCellFrame* pThisCell = static_cast<const SwCellFrame*>(&rLayoutFrame);
        const SwRowFrame* pRowFrame = static_cast<const SwRowFrame*>(pThisCell->GetUpper());
        const tools::Long nRowSpan = pThisCell->GetTabBox()->getRowSpan();
        if ( !pRowFrame->IsRowSpanLine() || nRowSpan > 1 || nRowSpan < -1 )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), &rLayoutFrame );
            const SwBorderAttrs& rAttrs = *aAccess.Get();
            const SvxBoxItem& rBox = rAttrs.GetBox();
            Insert(rLayoutFrame, rBox, rPaintArea);
        }
    }

    // Recurse into lower layout frames, but do not recurse into lower tabframes.
    const SwFrame* pLower = rLayoutFrame.Lower();
    while ( pLower )
    {
        if (pLower->IsLayoutFrame() && !pLower->IsTabFrame())
        {
            const SwLayoutFrame* pLowerLayFrame = static_cast<const SwLayoutFrame*>(pLower);
            HandleFrame(*pLowerLayFrame, rPaintArea);
        }
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
    SwRect aUpper( pUpper->getFramePrintArea() );
    aUpper.Pos() += pUpper->getFrameArea().Pos();
    SwRect aUpperAligned( aUpper );
    ::SwAlignRect( aUpperAligned, gProp.pSGlobalShell, &rDev );

    // prepare SdrFrameBorderDataVector
    std::shared_ptr<drawinglayer::primitive2d::SdrFrameBorderDataVector> aData(
        std::make_shared<drawinglayer::primitive2d::SdrFrameBorderDataVector>());

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
        for (const SwLineEntry& rEntry : rEntrySet)
        {
            const svx::frame::Style& rEntryStyle( rEntry.maAttribute );

            Point aStart, aEnd;
            if ( bHori )
            {
                aStart.setX( rEntry.mnStartPos );
                aStart.setY( rEntry.mnKey );
                aEnd.setX( rEntry.mnEndPos );
                aEnd.setY( rEntry.mnKey );
            }
            else
            {
                aStart.setX( rEntry.mnKey );
                aStart.setY( rEntry.mnStartPos );
                aEnd.setX( rEntry.mnKey );
                aEnd.setY( rEntry.mnEndPos );
            }

            svx::frame::Style aStyles[ 7 ];
            aStyles[ 0 ] = rEntryStyle;
            FindStylesForLine( aStart, aEnd, aStyles, bHori );

            if (!bHori && rEntry.mnLimitedEndPos)
            {
                aEnd.setY(rEntry.mnLimitedEndPos);
            }

            SwRect aRepaintRect( aStart, aEnd );

            // the repaint rectangle has to be moved a bit for the centered lines:
            SwTwips nRepaintRectSize = !rEntryStyle.GetWidth() ? 1 : rEntryStyle.GetWidth();
            if ( bHori )
            {
                aRepaintRect.Height( 2 * nRepaintRectSize );
                aRepaintRect.Pos().AdjustY( -nRepaintRectSize );

                // To decide on visibility it is also necessary to expand the RepaintRect
                // to left/right according existing BorderLine overlap matchings, else there
                // will be repaint errors when scrolling in e.t TripleLine BorderLines.
                // aStyles[1] == aLFromT, aStyles[3] == aLFromB, aStyles[4] == aRFromT, aStyles[6] == aRFromB
                if(aStyles[1].IsUsed() || aStyles[3].IsUsed() || aStyles[4].IsUsed() || aStyles[6].IsUsed())
                {
                    const double fLineWidthMaxLeft(std::max(aStyles[1].GetWidth(), aStyles[3].GetWidth()));
                    const double fLineWidthMaxRight(std::max(aStyles[4].GetWidth(), aStyles[6].GetWidth()));
                    aRepaintRect.Width(aRepaintRect.Width() + (fLineWidthMaxLeft + fLineWidthMaxRight));
                    aRepaintRect.Pos().AdjustX( -fLineWidthMaxLeft );
                }
            }
            else
            {
                aRepaintRect.Width( 2 * nRepaintRectSize );
                aRepaintRect.Pos().AdjustX( -nRepaintRectSize );

                // Accordingly to horizontal case, but for top/bottom
                // aStyles[3] == aTFromR, aStyles[1] == aTFromL, aStyles[6] == aBFromR, aStyles[4] == aBFromL
                if(aStyles[3].IsUsed() || aStyles[1].IsUsed() || aStyles[6].IsUsed() || aStyles[4].IsUsed())
                {
                    const double fLineWidthMaxTop(std::max(aStyles[3].GetWidth(), aStyles[1].GetWidth()));
                    const double fLineWidthMaxBottom(std::max(aStyles[6].GetWidth(), aStyles[4].GetWidth()));
                    aRepaintRect.Height(aRepaintRect.Height() + (fLineWidthMaxTop + fLineWidthMaxBottom));
                    aRepaintRect.Pos().AdjustY( -fLineWidthMaxTop );
                }
            }

            if (!rRect.Overlaps(aRepaintRect))
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
                    aStyles[0].SetType(SvxBorderLineStyle::NONE);
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
                    aPaintStart.setX( aUpperAligned.Left() );
                else if (aStart.X() == aUpper.Right_())
                    aPaintStart.setX( aUpperAligned.Right_() );
                if (aStart.Y() == aUpper.Top())
                    aPaintStart.setY( aUpperAligned.Top() );
                else if (aStart.Y() == aUpper.Bottom_())
                    aPaintStart.setY( aUpperAligned.Bottom_() );

                if (aEnd.X() == aUpper.Left())
                    aPaintEnd.setX( aUpperAligned.Left() );
                else if (aEnd.X() == aUpper.Right_())
                    aPaintEnd.setX( aUpperAligned.Right_() );
                if (aEnd.Y() == aUpper.Top())
                    aPaintEnd.setY( aUpperAligned.Top() );
                else if (aEnd.Y() == aUpper.Bottom_())
                    aPaintEnd.setY( aUpperAligned.Bottom_() );
            }

            if(aStyles[0].IsUsed())
            {
                if (bHori)
                {
                    const basegfx::B2DPoint aOrigin(aPaintStart.X(), aPaintStart.Y());
                    const basegfx::B2DVector aX(basegfx::B2DPoint(aPaintEnd.X(), aPaintEnd.Y()) - aOrigin);

                    if(!aX.equalZero())
                    {
                        const basegfx::B2DVector aY(basegfx::getNormalizedPerpendicular(aX));
                        aData->emplace_back(
                            aOrigin,
                            aX,
                            aStyles[0],
                            pTmpColor);
                        drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                        rInstance.addSdrConnectStyleData(true, aStyles[1], -aY, true); // aLFromT
                        rInstance.addSdrConnectStyleData(true, aStyles[2], -aX, true); // aLFromL
                        rInstance.addSdrConnectStyleData(true, aStyles[3], aY, false); // aLFromB

                        rInstance.addSdrConnectStyleData(false, aStyles[4], -aY, true); // aRFromT
                        rInstance.addSdrConnectStyleData(false, aStyles[5], aX, false); // aRFromR
                        rInstance.addSdrConnectStyleData(false, aStyles[6], aY, false); // aRFromB
                    }
                }
                else // vertical
                {
                    const basegfx::B2DPoint aOrigin(aPaintStart.X(), aPaintStart.Y());
                    const basegfx::B2DVector aX(basegfx::B2DPoint(aPaintEnd.X(), aPaintEnd.Y()) - aOrigin);

                    if(!aX.equalZero())
                    {
                        const basegfx::B2DVector aY(basegfx::getNormalizedPerpendicular(aX));
                        aData->emplace_back(
                            aOrigin,
                            aX,
                            aStyles[0],
                            pTmpColor);
                        drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                        rInstance.addSdrConnectStyleData(true, aStyles[3], -aY, false); // aTFromR
                        rInstance.addSdrConnectStyleData(true, aStyles[2], -aX, true); // aTFromT
                        rInstance.addSdrConnectStyleData(true, aStyles[1], aY, true); // aTFromL

                        rInstance.addSdrConnectStyleData(false, aStyles[6], -aY, false); // aBFromR
                        rInstance.addSdrConnectStyleData(false, aStyles[5], aX, false); // aBFromB
                        rInstance.addSdrConnectStyleData(false, aStyles[4], aY, true); // aBFromL
                    }
                }
            }
        }
        ++aIter;
    }

    // create instance of SdrFrameBorderPrimitive2D if
    // SdrFrameBorderDataVector is used
    if(!aData->empty())
    {
        drawinglayer::primitive2d::Primitive2DContainer aSequence;
        aSequence.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::SdrFrameBorderPrimitive2D(
                    aData,
                    true)));    // force visualization to minimal one discrete unit (pixel)
        // paint
        mrTabFrame.ProcessPrimitives(aSequence);
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
    // For example, aLFromB means: this vertical line intersects my horizontal line at its left end,
    // from bottom.
    // pStyles[ 1 ] = bHori ? aLFromT : TFromL
    // pStyles[ 2 ] = bHori ? aLFromL : TFromT,
    // pStyles[ 3 ] = bHori ? aLFromB : TFromR,
    // pStyles[ 4 ] = bHori ? aRFromT : BFromL,
    // pStyles[ 5 ] = bHori ? aRFromR : BFromB,
    // pStyles[ 6 ] = bHori ? aRFromB : BFromR,

    SwLineEntryMap::const_iterator aMapIter = maVertLines.find( rStartPoint.X() );
    OSL_ENSURE( aMapIter != maVertLines.end(), "FindStylesForLine: Error" );
    const SwLineEntrySet& rVertSet = (*aMapIter).second;

    for ( const SwLineEntry& rEntry : rVertSet )
    {
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
    }

    aMapIter = maHoriLines.find( rStartPoint.Y() );
    OSL_ENSURE( aMapIter != maHoriLines.end(), "FindStylesForLine: Error" );
    const SwLineEntrySet& rHoriSet = (*aMapIter).second;

    for ( const SwLineEntry& rEntry : rHoriSet )
    {
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
    }

    if ( bHori )
    {
        aMapIter = maVertLines.find( rEndPoint.X() );
        OSL_ENSURE( aMapIter != maVertLines.end(), "FindStylesForLine: Error" );
        const SwLineEntrySet& rVertSet2 = (*aMapIter).second;

        for ( const SwLineEntry& rEntry : rVertSet2 )
        {
            if ( rEndPoint.Y() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
            else if ( rEndPoint.Y() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
        }
    }
    else
    {
        aMapIter = maHoriLines.find( rEndPoint.Y() );
        OSL_ENSURE( aMapIter != maHoriLines.end(), "FindStylesForLine: Error" );
        const SwLineEntrySet& rHoriSet2 = (*aMapIter).second;

        for ( const SwLineEntry& rEntry : rHoriSet2 )
        {
            if ( rEndPoint.X() == rEntry.mnEndPos )
                pStyles[ 4 ] = rEntry.maAttribute;
            else if ( rEndPoint.X() == rEntry.mnStartPos )
                pStyles[ 6 ] = rEntry.maAttribute;
        }
    }
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

void SwTabFramePainter::Insert(const SwFrame& rFrame, const SvxBoxItem& rBoxItem, const SwRect& rPaintArea)
{
    // build 4 line entries for the 4 borders:
    SwRect aBorderRect = rFrame.getFrameArea();

    aBorderRect.Intersection(rPaintArea);

    bool const bBottomAsTop(lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines(
                mrTabFrame, rFrame, rBoxItem));
    bool const bVert = mrTabFrame.IsVertical();
    bool const bR2L  = mrTabFrame.IsRightToLeft();

    bool bWordTableCell = false;
    SwViewShell* pShell = rFrame.getRootFrame()->GetCurrShell();
    if (pShell)
    {
        const IDocumentSettingAccess& rIDSA = pShell->GetDoc()->getIDocumentSettingAccess();
        bWordTableCell = rIDSA.get(DocumentSettingId::TABLE_ROW_KEEP);
    }

    // no scaling needed, it's all in the primitives and the target device
    svx::frame::Style aL(rBoxItem.GetLeft(), 1.0);
    aL.SetWordTableCell(bWordTableCell);
    svx::frame::Style aR(rBoxItem.GetRight(), 1.0);
    aR.SetWordTableCell(bWordTableCell);
    svx::frame::Style aT(rBoxItem.GetTop(), 1.0);
    aT.SetWordTableCell(bWordTableCell);
    svx::frame::Style aB(rBoxItem.GetBottom(), 1.0);
    aB.SetWordTableCell(bWordTableCell);

    aR.MirrorSelf();
    aB.MirrorSelf();

    const SwTwips nLeft   = aBorderRect.Left_();
    const SwTwips nRight  = aBorderRect.Right_();
    const SwTwips nTop    = aBorderRect.Top_();
    const SwTwips nBottom = aBorderRect.Bottom_();

    aL.SetRefMode( svx::frame::RefMode::Centered );
    aR.SetRefMode( svx::frame::RefMode::Centered );
    aT.SetRefMode( !bVert ? svx::frame::RefMode::Begin : svx::frame::RefMode::End );
    aB.SetRefMode( !bVert ? svx::frame::RefMode::Begin : svx::frame::RefMode::End );

    SwLineEntry aLeft  (nLeft,   nTop,  nBottom,
            bVert ? aB                         : (bR2L ? aR : aL));
    if (bWordTableCell && rBoxItem.GetLeft())
    {
        aLeft.LimitVerticalEndPos(rFrame, SwLineEntry::VerticalType::LEFT);
    }

    SwLineEntry aRight (nRight,  nTop,  nBottom,
            bVert ? (bBottomAsTop ? aB : aT) : (bR2L ? aL : aR));
    if (bWordTableCell && rBoxItem.GetRight())
    {
        aRight.LimitVerticalEndPos(rFrame, SwLineEntry::VerticalType::RIGHT);
    }
    SwLineEntry aTop   (nTop,    nLeft, nRight,
            bVert ? aL                         : (bBottomAsTop ? aB : aT));
    SwLineEntry aBottom(nBottom, nLeft, nRight,
            bVert ? aR                         : aB);

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

        if (rOld.mnLimitedEndPos)
        {
            // Don't merge with this line entry as it ends sooner than mnEndPos.
            ++aIter;
            continue;
        }

        const SwLineEntry::OverlapType nOverlapType = rOld.Overlaps( rNew );

        const svx::frame::Style& rOldAttr = rOld.maAttribute;
        const svx::frame::Style& rNewAttr = rNew.maAttribute;
        const svx::frame::Style& rCmpAttr = std::max(rNewAttr, rOldAttr);

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
void SwRootFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const pPrintData) const
{
    OSL_ENSURE( Lower() && Lower()->IsPageFrame(), "Lower of root is no page." );

    PROTOCOL( this, PROT::FileInit, DbgAction::NONE, nullptr)

    bool bResetRootPaint = false;
    SwViewShell *pSh = mpCurrShell;

    if ( pSh->GetWin() )
    {
        if ( pSh->GetOut() == pSh->GetWin()->GetOutDev() && !pSh->GetWin()->IsVisible() )
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

    std::unique_ptr<SwSavePaintStatics> pStatics;
    if ( gProp.pSGlobalShell )
        pStatics.reset(new SwSavePaintStatics());
    gProp.pSGlobalShell = pSh;

    if( !pSh->GetWin() )
        gProp.pSProgress = SfxProgress::GetActiveProgress( static_cast<SfxObjectShell*>(pSh->GetDoc()->GetDocShell()) );

    ::SwCalcPixStatics( pSh->GetOut() );
    aGlobalRetoucheColor = pSh->Imp()->GetRetoucheColor();

    // Copy rRect; for one, rRect could become dangling during the below action, and for another it
    // needs to be copied to aRect anyway as that is modified further down below:
    SwRect aRect( rRect );

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
            pSh->Imp()->DeletePaintRegion();
    }

    aRect.Intersection( pSh->VisArea() );

    const bool bExtraData = ::IsExtraData( GetFormat()->GetDoc() );

    gProp.pSLines.reset(new SwLineRects); // Container for borders.

    // #104289#. During painting, something (OLE) can
    // load the linguistic, which in turn can cause a reformat
    // of the document. Dangerous! We better set this flag to
    // avoid the reformat.
    const bool bOldAction = IsCallbackActionEnabled();
    const_cast<SwRootFrame*>(this)->SetCallbackActionEnabled( false );

    const SwPageFrame *pPage = pSh->Imp()->GetFirstVisPage(&rRenderContext);

    // #126222. The positions of headers and footers of the previous
    // pages have to be updated, else these headers and footers could
    // get visible at a wrong position.
    const SwPageFrame *pPageDeco = static_cast<const SwPageFrame*>(pPage->GetPrev());
    while (pPageDeco)
    {
        pPageDeco->PaintDecorators();
        OSL_ENSURE(!pPageDeco->GetPrev() || pPageDeco->GetPrev()->IsPageFrame(),
            "Neighbour of page is not a page.");
        pPageDeco = static_cast<const SwPageFrame*>(pPageDeco->GetPrev());
    }

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
            SwFrameControlPtr pControl = rMngr.GetControl( FrameControlType::PageBreak, pHiddenPage );
            if ( pControl )
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
            SwPageFrame::GetBorderAndShadowBoundRect( pPage->getFrameArea(), pSh, &rRenderContext, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

            if ( aRect.Overlaps( aPaintRect ) )
            {
                if ( pSh->GetWin() )
                {
                    gProp.pSSubsLines.reset(new SwSubsRects);
                    gProp.pSSpecSubsLines.reset(new SwSubsRects);
                }
                gProp.pBLines.reset(new BorderLines);

                aPaintRect.Intersection_( aRect );

                if ( bExtraData &&
                     pSh->GetWin() && pSh->IsInEndAction() )
                {
                    // enlarge paint rectangle to complete page width, subtract
                    // current paint area and invalidate the resulting region.
                    SwRectFnSet aRectFnSet(pPage);
                    SwRect aPageRectTemp( aPaintRect );
                    aRectFnSet.SetLeftAndWidth( aPageRectTemp,
                         aRectFnSet.GetLeft(pPage->getFrameArea()),
                         aRectFnSet.GetWidth(pPage->getFrameArea()) );
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
                    // changed method SwLayVout::Enter(..)
                    // 2nd parameter is no longer <const> and will be set to the
                    // rectangle the virtual output device is calculated from <aPaintRect>,
                    // if the virtual output is used.
                    s_pVout->Enter(pSh, aPaintRect, !s_isNoVirDev);

                    // Adjust paint rectangle to pixel size
                    // Thus, all objects overlapping on pixel level with the unadjusted
                    // paint rectangle will be considered in the paint.
                    lcl_AdjustRectToPixelSize( aPaintRect, *(pSh->GetOut()) );
                }

                // maybe this can be put in the above scope. Since we are not sure, just leave it ATM
                s_pVout->SetOrgRect( aPaintRect );

                // determine background color of page for <PaintLayer> method
                // calls, paint <hell> or <heaven>
                const Color aPageBackgrdColor(pPage->GetDrawBackgroundColor());

                pPage->PaintBaBo( aPaintRect, pPage );

                if ( pSh->Imp()->HasDrawView() )
                {
                    gProp.pSLines->LockLines( true );
                    const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
                    pSh->Imp()->PaintLayer( rIDDMA.GetHellId(),
                                            pPrintData,
                                            *pPage, pPage->getFrameArea(),
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

                pPage->PaintSwFrame( rRenderContext, aPaintRect );

                // no paint of page border and shadow, if writer is in place mode.
                if( pSh->GetWin() && pSh->GetDoc()->GetDocShell() &&
                    !pSh->GetDoc()->GetDocShell()->IsInPlaceActive() )
                {
                    SwPageFrame::PaintBorderAndShadow( pPage->getFrameArea(), pSh, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
                    SwPageFrame::PaintNotesSidebar( pPage->getFrameArea(), pSh, pPage->GetPhyPageNum(), bRightSidebar);
                }

                gProp.pSLines->PaintLines( pSh->GetOut(), gProp );
                if ( pSh->GetWin() )
                {
                    gProp.pSSubsLines->PaintSubsidiary( pSh->GetOut(), gProp.pSLines.get(), gProp );
                    gProp.pSSubsLines.reset();
                    gProp.pSSpecSubsLines.reset();
                }
                // fdo#42750: delay painting these until after subsidiary lines
                // fdo#45562: delay painting these until after hell layer
                // fdo#47717: but do it before heaven layer
                ProcessPrimitives(gProp.pBLines->GetBorderLines_Clear());

                if ( pSh->Imp()->HasDrawView() )
                {
                    pSh->Imp()->PaintLayer( pSh->GetDoc()->getIDocumentDrawModelAccess().GetHeavenId(),
                                            pPrintData,
                                            *pPage, pPage->getFrameArea(),
                                            &aPageBackgrdColor,
                                            pPage->IsRightToLeft(),
                                            &aSwRedirector );
                }

                if ( bExtraData )
                    pPage->RefreshExtraData( aPaintRect );

                gProp.pBLines.reset();
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
            SwRect aEmptyPageRect( pPage->getFrameArea() );

            // code from vprint.cxx
            const SwPageFrame& rFormatPage = pPage->GetFormatPage();
            aEmptyPageRect.SSize( rFormatPage.getFrameArea().SSize() );

            SwPageFrame::GetBorderAndShadowBoundRect( aEmptyPageRect, pSh, &rRenderContext, aPaintRect,
                bPaintLeftShadow, bPaintRightShadow, bRightSidebar );
            aPaintRect.Intersection_( aRect );

            if ( aRect.Overlaps( aEmptyPageRect ) )
            {
                // #i75172# if called from SwViewShell::ImplEndAction it should no longer
                // really be used but handled by SwViewShell::ImplEndAction already
                {
                    const vcl::Region aDLRegion(aPaintRect.SVRect());
                    pSh->DLPrePaint2(aDLRegion);
                }

                if( pSh->GetOut()->GetFillColor() != aGlobalRetoucheColor )
                    pSh->GetOut()->SetFillColor( aGlobalRetoucheColor );
                // No line color
                pSh->GetOut()->SetLineColor();
                // Use aligned page rectangle
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
                pSh->GetOut()->DrawText( aEmptyPageRect.SVRect(), SwResId( STR_EMPTYPAGE ),
                                    DrawTextFlags::VCenter |
                                    DrawTextFlags::Center |
                                    DrawTextFlags::Clip );

                pSh->GetOut()->SetFont( aOldFont );
                // paint shadow and border for empty page
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

    gProp.pSLines.reset();

    if ( bResetRootPaint )
        SwRootFrame::s_isInPaint = false;
    if ( pStatics )
        pStatics.reset();
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

namespace {

class SwShortCut
{
    SwRectDist m_fnCheck;
    tools::Long m_nLimit;

public:
    SwShortCut( const SwFrame& rFrame, const SwRect& rRect );
    bool Stop(const SwRect& rRect) const { return (rRect.*m_fnCheck)(m_nLimit) > 0; }
};

}

SwShortCut::SwShortCut( const SwFrame& rFrame, const SwRect& rRect )
{
    bool bVert = rFrame.IsVertical();
    bool bR2L = rFrame.IsRightToLeft();
    if( rFrame.IsNeighbourFrame() && bVert == bR2L )
    {
        if( bVert )
        {
            m_fnCheck = &SwRect::GetBottomDistance;
            m_nLimit = rRect.Top();
        }
        else
        {
            m_fnCheck = &SwRect::GetLeftDistance;
            m_nLimit = rRect.Left() + rRect.Width();
        }
    }
    else if( bVert == rFrame.IsNeighbourFrame() )
    {
        m_fnCheck = &SwRect::GetTopDistance;
        m_nLimit = rRect.Top() + rRect.Height();
    }
    else
    {
        if ( rFrame.IsVertLR() )
        {
            m_fnCheck = &SwRect::GetLeftDistance;
            m_nLimit = rRect.Right();
        }
        else
        {
            m_fnCheck = &SwRect::GetRightDistance;
            m_nLimit = rRect.Left();
        }
    }
}

void SwLayoutFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    // #i16816# tagged pdf support
    Frame_Info aFrameInfo( *this );
    SwTaggedPDFHelper aTaggedPDFHelper( nullptr, &aFrameInfo, nullptr, rRenderContext );

    const SwFrame *pFrame = Lower();
    if ( !pFrame )
        return;

    SwFrameDeleteGuard g(const_cast<SwLayoutFrame*>(this)); // lock because Calc() and recursion
    SwShortCut aShortCut( *pFrame, rRect );
    bool bCnt = pFrame->IsContentFrame();
    if ( bCnt )
        pFrame->Calc(&rRenderContext);

    if ( pFrame->IsFootnoteContFrame() )
    {
        ::lcl_EmergencyFormatFootnoteCont( const_cast<SwFootnoteContFrame*>(static_cast<const SwFootnoteContFrame*>(pFrame)) );
        pFrame = Lower();
    }

    const SwPageFrame *pPage = nullptr;
    bool bWin = gProp.pSGlobalShell->GetWin() != nullptr;
    if (comphelper::LibreOfficeKit::isTiledPainting())
        // Tiled rendering is similar to printing in this case: painting transparently multiple
        // times will result in darker colors: avoid that.
        bWin = false;

    while ( IsAnLower( pFrame ) )
    {
        SwRect aPaintRect( pFrame->GetPaintArea() );
        if( aShortCut.Stop( aPaintRect ) )
            break;
        if ( bCnt && gProp.pSProgress )
           SfxProgress::Reschedule();

        //We need to retouch if a frame explicitly requests it.
        //First do the retouch, because this could flatten the borders.
        if ( pFrame->IsRetouche() )
        {
            if ( pFrame->IsRetoucheFrame() && bWin && !pFrame->GetNext() )
            {
                if ( !pPage )
                    pPage = FindPageFrame();
                pFrame->Retouch( pPage, rRect );
            }
            pFrame->ResetRetouche();
        }

        if ( rRect.Overlaps( aPaintRect ) )
        {
            if ( bCnt && pFrame->IsCompletePaint() &&
                 !rRect.Contains( aPaintRect ) && Application::AnyInput( VclInputFlags::KEYBOARD ) )
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
                    aPaintRect.Bottom( pFrame->getFrameArea().Bottom() );
                    if ( aPaintRect.Height() > 0 )
                        gProp.pSGlobalShell->InvalidateWindows(aPaintRect);
                    aPaintRect.Top( pFrame->getFrameArea().Top() );
                    aPaintRect.Bottom( pFrame->getFrameArea().Bottom() );
                }
                else
                {
                    gProp.pSGlobalShell->InvalidateWindows( aPaintRect );
                    pFrame = pFrame->GetNext();
                    if ( pFrame )
                    {
                        bCnt = pFrame->IsContentFrame();
                        if ( bCnt )
                            pFrame->Calc(&rRenderContext);
                    }
                    continue;
                }
            }
            pFrame->ResetCompletePaint();
            aPaintRect.Intersection_( rRect );

            pFrame->PaintSwFrame( rRenderContext, aPaintRect );

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

        if ( pFrame )
        {
            bCnt = pFrame->IsContentFrame();
            if ( bCnt )
                pFrame->Calc(&rRenderContext);
        }
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
        basegfx::BColor aHslLine = basegfx::utils::rgb2hsl( aColor );
        double nLuminance = aHslLine.getZ() * 2.5;
        if ( nLuminance == 0 )
            nLuminance = 0.5;
        else if ( nLuminance >= 1.0 )
            nLuminance = aHslLine.getZ() * 0.4;
        aHslLine.setZ( nLuminance );
        const basegfx::BColor aOtherColor = basegfx::utils::hsl2rgb( aHslLine );

        // Compute the plain line
        aSeq[0] =
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aLinePolygon, aOtherColor );

        // Dashed line in twips
        aStrokePattern.push_back( 40 );
        aStrokePattern.push_back( 40 );

        aSeq.resize( 2 );
    }

    // Compute the dashed line primitive
    aSeq[ aSeq.size( ) - 1 ] =
            new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D (
                basegfx::B2DPolyPolygon( aLinePolygon ),
                drawinglayer::attribute::LineAttribute( aColor ),
                drawinglayer::attribute::StrokeAttribute( std::move(aStrokePattern) ) );


    return aSeq;
}

void SwPageFrame::PaintBreak( ) const
{
    if ( gProp.pSGlobalShell->GetOut()->GetOutDevType() == OUTDEV_PRINTER  ||
         gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() ||
         gProp.pSGlobalShell->GetViewOptions()->IsReadonly() ||
         gProp.pSGlobalShell->IsPreview() )
        return;

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
                rMngr.RemoveControlsByType( FrameControlType::PageBreak, this );
        }
    }
    SwLayoutFrame::PaintBreak( );
}

void SwColumnFrame::PaintBreak( ) const
{
    if ( gProp.pSGlobalShell->GetOut()->GetOutDevType() == OUTDEV_PRINTER  ||
         gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() ||
         gProp.pSGlobalShell->GetViewOptions()->IsReadonly() ||
         gProp.pSGlobalShell->IsPreview() )
        return;

    const SwFrame* pBodyFrame = Lower();
    while ( pBodyFrame && !pBodyFrame->IsBodyFrame() )
        pBodyFrame = pBodyFrame->GetNext();

    if ( !pBodyFrame )
        return;

    const SwContentFrame *pCnt = static_cast< const SwLayoutFrame* >( pBodyFrame )->ContainsContent();
    if ( !(pCnt && pCnt->IsColBreak( true )) )
        return;

    // Paint the break only if:
    //    * Not in header footer edition, to avoid conflicts with the
    //      header/footer marker
    //    * Non-printing characters are shown, as this is more consistent
    //      with other formatting marks
    if ( !(!gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Header ) &&
         !gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Footer ) &&
          gProp.pSGlobalShell->GetViewOptions()->IsLineBreak()) )
        return;

    SwRect aRect( pCnt->getFramePrintArea() );
    aRect.Pos() += pCnt->getFrameArea().Pos();

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

    // Add the text above
    OUString aBreakText = SwResId(STR_COLUMN_BREAK);

    basegfx::B2DVector aFontSize;
    OutputDevice* pOut = gProp.pSGlobalShell->GetOut();
    vcl::Font aFont = pOut->GetSettings().GetStyleSettings().GetToolFont();
    aFont.SetFontHeight( 8 * 20 );
    pOut->SetFont( aFont );
    drawinglayer::attribute::FontAttribute aFontAttr = drawinglayer::primitive2d::getFontAttributeFromVclFont(
            aFontSize, aFont, IsRightToLeft(), false );

    tools::Rectangle aTextRect;
    pOut->GetTextBoundRect( aTextRect, aBreakText );
    tools::Long nTextOff = ( nWidth - aTextRect.GetWidth() ) / 2;

    basegfx::B2DHomMatrix aTextMatrix( basegfx::utils::createScaleTranslateB2DHomMatrix(
                aFontSize.getX(), aFontSize.getY(),
                aRect.Left() + nTextOff, aRect.Top() ) );
    if ( IsVertical() )
    {
        aTextMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix (
                aFontSize.getX(), aFontSize.getY(), 0.0, M_PI_2,
                aRect.Right(), aRect.Top() + nTextOff );
    }

    aSeq.push_back(
            new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
                aTextMatrix,
                aBreakText, 0, aBreakText.getLength(),
                std::vector< double >(),
                aFontAttr,
                lang::Locale(),
                aLineColor ) );

    ProcessPrimitives( aSeq );
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
    if ( !pWrtSh )
        return;

    SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();

    const SwLayoutFrame* pBody = FindBodyCont();
    if ( !pBody )
        return;

    SwRect aBodyRect( pBody->getFrameArea() );

    if ( !(gProp.pSGlobalShell->GetOut()->GetOutDevType() != OUTDEV_PRINTER &&
         !gProp.pSGlobalShell->GetViewOptions()->IsPDFExport() &&
         !gProp.pSGlobalShell->IsPreview() &&
         !gProp.pSGlobalShell->GetViewOptions()->IsReadonly() &&
         !gProp.pSGlobalShell->GetViewOptions()->getBrowseMode() &&
         ( gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Header ) ||
           gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Footer ) )) )
        return;

    bool bRtl = AllSettings::GetLayoutRTL();
    const SwRect& rVisArea = gProp.pSGlobalShell->VisArea();
    tools::Long nXOff = std::min( aBodyRect.Right(), rVisArea.Right() );
    if ( bRtl )
        nXOff = std::max( aBodyRect.Left(), rVisArea.Left() );

    // Header
    if ( gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Header ) )
    {
        const SwFrame* pHeaderFrame = Lower();
        if ( !pHeaderFrame->IsHeaderFrame() )
            pHeaderFrame = nullptr;

        tools::Long nHeaderYOff = aBodyRect.Top();
        Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nHeaderYOff ) );
        rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, FrameControlType::Header, nOutputOff );
    }

    // Footer
    if ( !gProp.pSGlobalShell->IsShowHeaderFooterSeparator( FrameControlType::Footer ) )
        return;

    const SwFrame* pFootnoteContFrame = Lower();
    while ( pFootnoteContFrame )
    {
        if ( pFootnoteContFrame->IsFootnoteContFrame() )
            aBodyRect.AddBottom( pFootnoteContFrame->getFrameArea().Bottom() - aBodyRect.Bottom() );
        pFootnoteContFrame = pFootnoteContFrame->GetNext();
    }

    tools::Long nFooterYOff = aBodyRect.Bottom();
    Point nOutputOff = rEditWin.LogicToPixel( Point( nXOff, nFooterYOff ) );
    rEditWin.GetFrameControlsManager().SetHeaderFooterControl( this, FrameControlType::Footer, nOutputOff );
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
         GetFormat()->IsBackgroundBrushInherited() )
    {
        const SvxBrushItem* pBackgroundBrush = nullptr;
        std::optional<Color> xSectionTOXColor;
        SwRect aDummyRect;
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

        if ( GetBackgroundBrush( aFillAttributes, pBackgroundBrush, xSectionTOXColor, aDummyRect, false, /*bConsiderTextBox=*/false) )
        {
            if ( xSectionTOXColor &&
                 (xSectionTOXColor->IsTransparent()) &&
                 (xSectionTOXColor != COL_TRANSPARENT) )
            {
                bBackgroundTransparent = true;
            }
            else if(aFillAttributes && aFillAttributes->isUsed())
            {
                bBackgroundTransparent = aFillAttributes->isTransparent();
            }
            else if ( pBackgroundBrush )
            {
                if ( (pBackgroundBrush->GetColor().IsTransparent()) &&
                     (pBackgroundBrush->GetColor() != COL_TRANSPARENT) )
                {
                    bBackgroundTransparent = true;
                }
                else
                {
                    const GraphicObject *pTmpGrf =
                            pBackgroundBrush->GetGraphicObject();
                    if ( pTmpGrf &&
                         (pTmpGrf->GetAttr().IsTransparent())
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
    SdrObjUserCall *pUserCall = GetUserCall(pObj);

    if ( nullptr == pUserCall )
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
        if ( auto pFlyDraw = dynamic_cast<SwVirtFlyDrawObj *>( pObj ) )
        {
            SwFlyFrame *pFly = pFlyDraw->GetFlyFrame();
            if ( gProp.pSFlyOnlyDraw && gProp.pSFlyOnlyDraw == pFly )
                return true;

            //Try to avoid displaying the intermediate stage, Flys which don't
            //overlap with the page on which they are anchored won't be
            //painted.
            //HACK: exception: printing of frames in tables, those can overlap
            //a page once in a while when dealing with oversized tables (HTML).
            SwPageFrame *pPage = pFly->FindPageFrame();
            if ( pPage && pPage->getFrameArea().Overlaps( pFly->getFrameArea() ) )
            {
                    pAnch = pFly->AnchorFrame();
            }

        }
        else
        {
            // Consider 'virtual' drawing objects
            SwDrawContact* pDrawContact = dynamic_cast<SwDrawContact*>(pUserCall);
            pAnch = pDrawContact ? pDrawContact->GetAnchorFrame(pObj) : nullptr;
            if ( pAnch )
            {
                if ( !pAnch->isFrameAreaPositionValid() )
                    pAnch = nullptr;
                else if ( pSh->GetOut() == pSh->getIDocumentDeviceAccess().getPrinter( false ))
                {
                    //HACK: we have to omit some of the objects for printing,
                    //otherwise they would be printed twice.
                    //The objects should get printed if the TableHack is active
                    //right now. Afterwards they must not be printed if the
                    //page over which they float position wise gets printed.
                    const SwPageFrame *pPage = pAnch->FindPageFrame();
                    if ( !pPage->getFrameArea().Overlaps( SwRect(pObj->GetCurrentBoundRect()) ) )
                        pAnch = nullptr;
                }
            }
            else
            {
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

void SwCellFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    if ( GetLayoutRowSpan() >= 1 )
        SwLayoutFrame::PaintSwFrame( rRenderContext, rRect );
}

namespace {

struct BorderLinesGuard
{
    explicit BorderLinesGuard() : m_pBorderLines(std::move(gProp.pBLines))
    {
        gProp.pBLines.reset(new BorderLines);
    }
    ~BorderLinesGuard()
    {
        gProp.pBLines = std::move(m_pBorderLines);
    }
private:
    std::unique_ptr<BorderLines> m_pBorderLines;
};

}

// set strikethrough for deleted objects anchored to character
void SwFrame::SetDrawObjsAsDeleted( bool bDeleted )
{
    if ( SwSortedObjs *pObjs = GetDrawObjs() )
    {
        for (SwAnchoredObject* pAnchoredObj : *pObjs)
        {
            if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
            {
                pFly->SetDeleted(bDeleted);
            }
        }
    }
}

void SwFlyFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
{
    //optimize thumbnail generation and store procedure to improve odt saving performance, #i120030#
    SwViewShell *pShell = getRootFrame()->GetCurrShell();
    if (pShell && pShell->GetDoc() && pShell->GetDoc()->GetDocShell())
    {
        bool bInGenerateThumbnail = pShell->GetDoc()->GetDocShell()->IsInGenerateAndStoreThumbnail();
        if (bInGenerateThumbnail)
        {
            const SwRect& aVisRect = pShell->VisArea();
            if (!aVisRect.Overlaps(getFrameArea()))
                return;
        }
    }

    //because of the overlapping of frames and drawing objects the flys have to
    //paint their borders (and those of the internal ones) directly.
    //e.g. #33066#
    gProp.pSLines->LockLines(true);
    BorderLinesGuard blg; // this should not paint borders added from PaintBaBo

    SwRect aRect( rRect );
    aRect.Intersection_( getFrameArea() );

    rRenderContext.Push( vcl::PushFlags::CLIPREGION );
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
            // add 2nd parameter with value <true>
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
            const SwFlyFrameFormat* pSwFrameFormat = GetFormat();

            if (pSwFrameFormat && pSwFrameFormat->supportsFullDrawingLayerFillAttributeSet())
            {
                // check for transparency
                const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(pSwFrameFormat->getSdrAllFillAttributesHelper());

                // check if the new fill attributes are used
                if(aFillAttributes && aFillAttributes->isUsed())
                {
                    bPaintCompleteBack = true;
                }
            }
            else
            {
                std::unique_ptr<SvxBrushItem> aBack = GetFormat()->makeBackgroundBrushItem();
                //     to determine, if background has to be painted, by checking, if
                //     background color is not COL_TRANSPARENT ("no fill"/"auto fill")
                //     or a background graphic exists.
                bPaintCompleteBack = aBack &&
                    ((aBack->GetColor() != COL_TRANSPARENT) ||
                    aBack->GetGraphicPos() != GPOS_NONE);
            }
        }
        // paint of margin needed.
        const bool bPaintMarginOnly( !bPaintCompleteBack &&
                                     getFramePrintArea().SSize() != getFrameArea().SSize() );

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
                aPaintRect.Intersection_( pParentFlyFrame->getFrameArea() );
                pParentFlyFrame->PaintSwFrameBackground( aPaintRect, pPage, rAttrs );

                gProp.pSRetoucheFly2 = pOldRet;
            }
        }

        if ( bPaintCompleteBack || bPaintMarginOnly )
        {
            //#24926# JP 01.02.96, PaintBaBo is here partially so PaintSwFrameShadowAndBorder
            //receives the original Rect but PaintSwFrameBackground only the limited
            //one.

            rRenderContext.Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
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
                    SwRect aTmp( getFramePrintArea() ); aTmp += getFrameArea().Pos();
                    aRegion -= aTmp;
                }
                if ( bContour )
                {
                    rRenderContext.Push();
                    // #i80822#
                    // apply clip region under the same conditions, which are
                    // used in <SwNoTextFrame::PaintSwFrame(..)> to set the clip region
                    // for painting the graphic/OLE. Thus, the clip region is
                    // also applied for the PDF export.
                    SwViewShell *pSh = getRootFrame()->GetCurrShell();

                    if ( !rRenderContext.GetConnectMetaFile() || !pSh || !pSh->GetWin() )
                    {
                        rRenderContext.SetClipRegion(vcl::Region(aPoly));
                    }

                    for ( size_t i = 0; i < aRegion.size(); ++i )
                    {
                        PaintSwFrameBackground( aRegion[i], pPage, rAttrs, false, true );
                    }

                    rRenderContext.Pop();
                }
                else
                {
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                    {
                        PaintSwFrameBackground( aRegion[i], pPage, rAttrs, false, true );
                    }
                }
            }

            // paint border before painting background
            PaintSwFrameShadowAndBorder(rRect, pPage, rAttrs);

            rRenderContext.Pop();
        }
    }

    // fly frame will paint it's subsidiary lines and
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
            gProp.pSSubsLines.reset(new SwSubsRects);
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
            gProp.pSSpecSubsLines.reset(new SwSubsRects);
            bSpecSubsLineRectsCreated = true;
        }
        // Add subsidiary lines of fly frame and its lowers
        RefreshLaySubsidiary( pPage, aRect );
        // paint subsidiary lines of fly frame and its lowers
        gProp.pSSpecSubsLines->PaintSubsidiary( &rRenderContext, nullptr, gProp );
        gProp.pSSubsLines->PaintSubsidiary(&rRenderContext, gProp.pSLines.get(), gProp);
        if ( !bSubsLineRectsCreated )
            // unlock subsidiary lines
            gProp.pSSubsLines->LockLines( false );
        else
        {
            // delete created subsidiary lines container
            gProp.pSSubsLines.reset();
        }

        if ( !bSpecSubsLineRectsCreated )
            // unlock special subsidiary lines
            gProp.pSSpecSubsLines->LockLines( false );
        else
        {
            // delete created special subsidiary lines container
            gProp.pSSpecSubsLines.reset();
        }
    }

    SwLayoutFrame::PaintSwFrame( rRenderContext, aRect );

    Validate();

    // first paint lines added by fly frame paint
    // and then unlock other lines.
    gProp.pSLines->PaintLines( &rRenderContext, gProp );
    gProp.pSLines->LockLines( false );
    // have to paint frame borders added in heaven layer here...
    ProcessPrimitives(gProp.pBLines->GetBorderLines_Clear());

    PaintDecorators();

    // crossing out for tracked deletion
    if ( GetAuthor() != std::string::npos && IsDeleted() )
    {
        tools::Long startX = aRect.Left(  ), endX = aRect.Right();
        tools::Long startY = aRect.Top(  ),  endY = aRect.Bottom();
        rRenderContext.SetLineColor( SwPostItMgr::GetColorAnchor(GetAuthor()) );
        rRenderContext.DrawLine(Point(startX, startY), Point(endX, endY));
        rRenderContext.DrawLine(Point(startX, endY), Point(endX, startY));
    }

    rRenderContext.Pop();

    if ( gProp.pSProgress && pNoText )
        SfxProgress::Reschedule();
}

void SwFlyFrame::PaintDecorators() const
{
    // Show the un-float button
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( gProp.pSGlobalShell );
    if ( pWrtSh )
    {
        UpdateUnfloatButton(pWrtSh, IsShowUnfloatButton(pWrtSh));
    }
}

void SwTextFrame::PaintOutlineContentVisibilityButton() const
{
    SwWrtShell* pWrtSh = dynamic_cast<SwWrtShell*>(gProp.pSGlobalShell);
    if (pWrtSh && pWrtSh->GetViewOptions()->IsShowOutlineContentVisibilityButton())
        UpdateOutlineContentVisibilityButton(pWrtSh);
}


void SwTabFrame::PaintSwFrame(vcl::RenderContext& rRenderContext, SwRect const& rRect, SwPrintData const*const) const
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
            if ( rAttrs.GetShadow().GetLocation() != SvxShadowLocation::NONE )
            {
                SwRect aRect;
                ::lcl_CalcBorderRect( aRect, this, rAttrs, true, gProp );
                PaintShadow( rRect, aRect, rAttrs );
            }

            SwTabFramePainter aHelper(*this);
            aHelper.PaintLines(rRenderContext, rRect);
        }

        SwLayoutFrame::PaintSwFrame( rRenderContext, rRect );
    }
    // #i6467# - no light grey rectangle for page preview
    else if ( gProp.pSGlobalShell->GetWin() && !gProp.pSGlobalShell->IsPreview() )
    {
        //  #i6467# - intersect output rectangle with table frame
        SwRect aTabRect( getFramePrintArea() );
        aTabRect.Pos() += getFrameArea().Pos();
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
    SwPaintProperties const & properties)
{
    const tools::Long nWidth  = ::lcl_AlignWidth ( rShadow.GetWidth(), properties );
    const tools::Long nHeight = ::lcl_AlignHeight( rShadow.GetWidth(), properties );

    SwRects aRegion;
    SwRect aOut( rOutRect );

    switch ( rShadow.GetLocation() )
    {
        case SvxShadowLocation::BottomRight:
            {
                if ( bDrawFullShadowRectangle )
                {
                    // draw full shadow rectangle
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
                    rOutRect.AddRight(- nWidth );
                if( bBottom )
                    rOutRect.AddBottom(- nHeight );
            }
            break;
        case SvxShadowLocation::TopLeft:
            {
                if ( bDrawFullShadowRectangle )
                {
                    // draw full shadow rectangle
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
                    rOutRect.AddLeft( nWidth );
                if( bTop )
                    rOutRect.AddTop( nHeight );
            }
            break;
        case SvxShadowLocation::TopRight:
            {
                if ( bDrawFullShadowRectangle )
                {
                    // draw full shadow rectangle
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
                    rOutRect.AddRight( - nWidth );
                if( bTop )
                    rOutRect.AddTop( nHeight );
            }
            break;
        case SvxShadowLocation::BottomLeft:
            {
                if ( bDrawFullShadowRectangle )
                {
                    // draw full shadow rectangle
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
                    rOutRect.AddLeft( nWidth );
                if( bBottom )
                    rOutRect.AddBottom( - nHeight );
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

    for (const SwRect & rOut : aRegion)
    {
        aOut = rOut;
        if ( rRect.Overlaps( aOut ) && aOut.Height() > 0 && aOut.Width() > 0 )
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
            case SvxShadowLocation::BottomRight: rShadow.SetLocation(SvxShadowLocation::BottomLeft);  break;
            case SvxShadowLocation::TopLeft:     rShadow.SetLocation(SvxShadowLocation::TopRight);    break;
            case SvxShadowLocation::TopRight:    rShadow.SetLocation(SvxShadowLocation::BottomRight); break;
            case SvxShadowLocation::BottomLeft:  rShadow.SetLocation(SvxShadowLocation::TopLeft);     break;
            default: break;
        }
    }

    //  determine, if full shadow rectangle have to be drawn or only two shadow rectangles beside the frame.
    //     draw full shadow rectangle, if frame background is drawn transparent.
    //     Status Quo:
    //         SwLayoutFrame can have transparent drawn backgrounds. Thus,
    //         "asked" their frame format.
    const bool bDrawFullShadowRectangle =
            ( IsLayoutFrame() &&
              static_cast<const SwLayoutFrame*>(this)->GetFormat()->IsBackgroundTransparent()
            );

    SwRectFnSet aRectFnSet(this);
    ::lcl_ExtendLeftAndRight( rOutRect, *(this), rAttrs, aRectFnSet.FnRect() );

    lcl_PaintShadow(rRect, rOutRect, rShadow, bDrawFullShadowRectangle, bTop, bBottom, true, true, gProp);
}

void SwFrame::PaintBorderLine( const SwRect& rRect,
                             const SwRect& rOutRect,
                             const SwPageFrame * pPage,
                             const Color *pColor,
                             const SvxBorderLineStyle nStyle ) const
{
    if ( !rOutRect.Overlaps( rRect ) )
        return;

    SwRect aOut( rOutRect );
    aOut.Intersection_( rRect );

    const SwTabFrame *pTab = IsCellFrame() ? FindTabFrame() : nullptr;
    SubColFlags nSubCol = ( IsCellFrame() || IsRowFrame() )
                          ? SubColFlags::Tab
                          : ( IsInSct()
                              ? SubColFlags::Sect
                              : ( IsInFly() ? SubColFlags::Fly : SubColFlags::Page ) );
    if( pColor && gProp.pSGlobalShell->GetWin() &&
        Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pColor = &SwViewOption::GetFontColor();
    }

    if (pPage->GetSortedObjs() &&
        pPage->GetFormat()->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS))
    {
        SwRegionRects aRegion( aOut, 4 );
        basegfx::utils::B2DClipState aClipState;
        ::lcl_SubtractFlys( this, pPage, aOut, aRegion, aClipState, gProp );
        for ( size_t i = 0; i < aRegion.size(); ++i )
            gProp.pSLines->AddLineRect( aRegion[i], pColor, nStyle, pTab, nSubCol, gProp );
    }
    else
        gProp.pSLines->AddLineRect( aOut, pColor, nStyle, pTab, nSubCol, gProp );
}

namespace drawinglayer::primitive2d
{
        namespace {

        class SwBorderRectanglePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the transformation defining the geometry of this BorderRectangle
            basegfx::B2DHomMatrix       maB2DHomMatrix;

            /// the four styles to be used
            svx::frame::Style           maStyleTop;
            svx::frame::Style           maStyleRight;
            svx::frame::Style           maStyleBottom;
            svx::frame::Style           maStyleLeft;

        protected:
            /// local decomposition.
            virtual void create2DDecomposition(
                Primitive2DContainer& rContainer,
                const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SwBorderRectanglePrimitive2D(
                const basegfx::B2DHomMatrix& rB2DHomMatrix,
                const svx::frame::Style& rStyleTop,
                const svx::frame::Style& rStyleRight,
                const svx::frame::Style& rStyleBottom,
                const svx::frame::Style& rStyleLeft);

            /// data read access
            const basegfx::B2DHomMatrix& getB2DHomMatrix() const { return maB2DHomMatrix; }
            const svx::frame::Style& getStyleTop() const { return maStyleTop; }
            const svx::frame::Style& getStyleRight() const { return maStyleRight; }
            const svx::frame::Style& getStyleBottom() const { return maStyleBottom; }
            const svx::frame::Style& getStyleLeft() const { return maStyleLeft; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        }

        void SwBorderRectanglePrimitive2D::create2DDecomposition(
            Primitive2DContainer& rContainer,
            const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DPoint aTopLeft(getB2DHomMatrix() * basegfx::B2DPoint(0.0, 0.0));
            basegfx::B2DPoint aTopRight(getB2DHomMatrix() * basegfx::B2DPoint(1.0, 0.0));
            basegfx::B2DPoint aBottomLeft(getB2DHomMatrix() * basegfx::B2DPoint(0.0, 1.0));
            basegfx::B2DPoint aBottomRight(getB2DHomMatrix() * basegfx::B2DPoint(1.0, 1.0));

            // prepare SdrFrameBorderDataVector
            std::shared_ptr<drawinglayer::primitive2d::SdrFrameBorderDataVector> aData(
                std::make_shared<drawinglayer::primitive2d::SdrFrameBorderDataVector>());

            if(getStyleTop().IsUsed())
            {
                // move top left/right inwards half border width
                basegfx::B2DVector aDown(getB2DHomMatrix() * basegfx::B2DVector(0.0, 1.0));
                aDown.setLength(getStyleTop().GetWidth() * 0.5);
                aTopLeft += aDown;
                aTopRight += aDown;
            }

            if(getStyleBottom().IsUsed())
            {
                // move bottom left/right inwards half border width
                basegfx::B2DVector aUp(getB2DHomMatrix() * basegfx::B2DVector(0.0, -1.0));
                aUp.setLength(getStyleBottom().GetWidth() * 0.5);
                aBottomLeft += aUp;
                aBottomRight += aUp;
            }

            if(getStyleLeft().IsUsed())
            {
                // move left top/bottom inwards half border width
                basegfx::B2DVector aRight(getB2DHomMatrix() * basegfx::B2DVector(1.0, 0.0));
                aRight.setLength(getStyleLeft().GetWidth() * 0.5);
                aTopLeft += aRight;
                aBottomLeft += aRight;
            }

            if(getStyleRight().IsUsed())
            {
                // move right top/bottom inwards half border width
                basegfx::B2DVector aLeft(getB2DHomMatrix() * basegfx::B2DVector(-1.0, 0.0));
                aLeft.setLength(getStyleRight().GetWidth() * 0.5);
                aTopRight += aLeft;
                aBottomRight += aLeft;
            }

            // go round-robin, from TopLeft to TopRight, down, left and back up. That
            // way, the borders will not need to be mirrored in any way
            if(getStyleTop().IsUsed())
            {
                // create BorderPrimitive(s) for top border
                const basegfx::B2DVector aVector(aTopRight - aTopLeft);
                aData->emplace_back(
                    aTopLeft,
                    aVector,
                    getStyleTop(),
                    nullptr);
                drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                if(getStyleLeft().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(true, getStyleLeft(), basegfx::B2DVector(aBottomLeft - aTopLeft), false);
                }

                if(getStyleRight().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(false, getStyleRight(), basegfx::B2DVector(aBottomRight - aTopRight), false);
                }
            }

            if(getStyleRight().IsUsed())
            {
                // create BorderPrimitive(s) for right border
                const basegfx::B2DVector aVector(aBottomRight - aTopRight);
                aData->emplace_back(
                    aTopRight,
                    aVector,
                    getStyleRight(),
                    nullptr);
                drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                if(getStyleTop().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(true, getStyleTop(), basegfx::B2DVector(aTopLeft - aTopRight), false);
                }

                if(getStyleBottom().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(false, getStyleBottom(), basegfx::B2DVector(aBottomLeft - aBottomRight), false);
                }
            }

            if(getStyleBottom().IsUsed())
            {
                // create BorderPrimitive(s) for bottom border
                const basegfx::B2DVector aVector(aBottomLeft - aBottomRight);
                aData->emplace_back(
                    aBottomRight,
                    aVector,
                    getStyleBottom(),
                    nullptr);
                drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                if(getStyleRight().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(true, getStyleRight(), basegfx::B2DVector(aTopRight - aBottomRight), false);
                }

                if(getStyleLeft().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(false, getStyleLeft(), basegfx::B2DVector(aTopLeft - aBottomLeft), false);
                }
            }

            if(getStyleLeft().IsUsed())
            {
                // create BorderPrimitive(s) for left border
                const basegfx::B2DVector aVector(aTopLeft - aBottomLeft);
                aData->emplace_back(
                    aBottomLeft,
                    aVector,
                    getStyleLeft(),
                    nullptr);
                drawinglayer::primitive2d::SdrFrameBorderData& rInstance(aData->back());

                if(getStyleBottom().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(true, getStyleBottom(), basegfx::B2DVector(aBottomRight - aBottomLeft), false);
                }

                if(getStyleTop().IsUsed())
                {
                    rInstance.addSdrConnectStyleData(false, getStyleTop(), basegfx::B2DVector(aTopRight - aTopLeft), false);
                }
            }

            // create instance of SdrFrameBorderPrimitive2D if
            // SdrFrameBorderDataVector is used
            if(!aData->empty())
            {
                rContainer.append(
                    drawinglayer::primitive2d::Primitive2DReference(
                        new drawinglayer::primitive2d::SdrFrameBorderPrimitive2D(
                            aData,
                            true)));    // force visualization to minimal one discrete unit (pixel)
            }
        }

        SwBorderRectanglePrimitive2D::SwBorderRectanglePrimitive2D(
            const basegfx::B2DHomMatrix& rB2DHomMatrix,
            const svx::frame::Style& rStyleTop,
            const svx::frame::Style& rStyleRight,
            const svx::frame::Style& rStyleBottom,
            const svx::frame::Style& rStyleLeft)
        :   maB2DHomMatrix(rB2DHomMatrix),
            maStyleTop(rStyleTop),
            maStyleRight(rStyleRight),
            maStyleBottom(rStyleBottom),
            maStyleLeft(rStyleLeft)
        {
        }

        bool SwBorderRectanglePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const SwBorderRectanglePrimitive2D& rCompare = static_cast<const SwBorderRectanglePrimitive2D&>(rPrimitive);

                return (getB2DHomMatrix() == rCompare.getB2DHomMatrix() &&
                    getStyleTop() == rCompare.getStyleTop() &&
                    getStyleRight() == rCompare.getStyleRight() &&
                    getStyleBottom() == rCompare.getStyleBottom() &&
                    getStyleLeft() == rCompare.getStyleLeft());
            }

            return false;
        }

        basegfx::B2DRange SwBorderRectanglePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);

            aRetval.transform(getB2DHomMatrix());
            return aRetval;
        }

        // provide unique ID
        sal_uInt32 SwBorderRectanglePrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SWBORDERRECTANGLERIMITIVE;
        }

} // end of namespace drawinglayer::primitive2d

namespace {

editeng::SvxBorderLine const * get_ptr(std::optional<editeng::SvxBorderLine> const & opt) {
    return opt ? &*opt : nullptr;
}

}

void PaintCharacterBorder(
    const SwFont& rFont,
    const SwRect& rPaintArea,
    const bool bVerticalLayout,
    const bool bVerticalLayoutLRBT,
    const bool bJoinWithPrev,
    const bool bJoinWithNext )
{
    SwRect aAlignedRect(rPaintArea);
    SwAlignRect(aAlignedRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut());

    bool bTop = true;
    bool bBottom = true;
    bool bLeft = true;
    bool bRight = true;

    switch (rFont.GetOrientation(bVerticalLayout, bVerticalLayoutLRBT).get())
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
            rFont.GetAbsShadowLocation(bVerticalLayout, bVerticalLayoutLRBT));

        if( aShadow.GetLocation() != SvxShadowLocation::NONE )
        {
            lcl_PaintShadow( rPaintArea, aAlignedRect, aShadow,
                             false, bTop, bBottom, bLeft, bRight, gProp);
        }
    }

    const basegfx::B2DHomMatrix aBorderTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(
            aAlignedRect.Width(), aAlignedRect.Height(),
            aAlignedRect.Left(), aAlignedRect.Top()));
    const svx::frame::Style aStyleTop(
        bTop ? get_ptr(rFont.GetAbsTopBorder(bVerticalLayout, bVerticalLayoutLRBT)) : nullptr,
        1.0);
    const svx::frame::Style aStyleRight(
        bRight ? get_ptr(rFont.GetAbsRightBorder(bVerticalLayout, bVerticalLayoutLRBT)) : nullptr,
        1.0);
    const svx::frame::Style aStyleBottom(
        bBottom ? get_ptr(rFont.GetAbsBottomBorder(bVerticalLayout, bVerticalLayoutLRBT))
                : nullptr,
        1.0);
    const svx::frame::Style aStyleLeft(
        bLeft ? get_ptr(rFont.GetAbsLeftBorder(bVerticalLayout, bVerticalLayoutLRBT)) : nullptr,
        1.0);
    drawinglayer::primitive2d::Primitive2DContainer aBorderLineTarget;

    aBorderLineTarget.append(
        drawinglayer::primitive2d::Primitive2DReference(
            new drawinglayer::primitive2d::SwBorderRectanglePrimitive2D(
                aBorderTransform,
                aStyleTop,
                aStyleRight,
                aStyleBottom,
                aStyleLeft)));
    gProp.pBLines->AddBorderLines(std::move(aBorderLineTarget));
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

std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> SwFrame::CreateProcessor2D( ) const
{
    basegfx::B2DRange aViewRange;

    SdrPage *pDrawPage = getRootFrame()->GetCurrShell()->Imp()->GetPageView()->GetPage();
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos(
            basegfx::B2DHomMatrix(  ),
            getRootFrame()->GetCurrShell()->GetOut()->GetViewTransformation(),
            aViewRange,
            GetXDrawPageForSdrPage( pDrawPage ),
            0.0);

    return  drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *getRootFrame()->GetCurrShell()->GetOut(),
                    aNewViewInfos );
}

void SwFrame::ProcessPrimitives( const drawinglayer::primitive2d::Primitive2DContainer& rSequence ) const
{
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D = CreateProcessor2D();
    if ( pProcessor2D )
    {
        pProcessor2D->process( rSequence );
    }
}

/// Paints shadows and borders
void SwFrame::PaintSwFrameShadowAndBorder(
    const SwRect& rRect,
    const SwPageFrame* /*pPage*/,
    const SwBorderAttrs& rAttrs) const
{
    // There's nothing (Row,Body,Footnote,Root,Column,NoText) need to do here
    if (GetType() & (SwFrameType::NoTxt|SwFrameType::Row|SwFrameType::Body|SwFrameType::Ftn|SwFrameType::Column|SwFrameType::Root))
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
    const bool bShadow = rAttrs.GetShadow().GetLocation() != SvxShadowLocation::NONE;

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
    if ( !(bLine || bShadow || bFoundCellForTopOrBorderAttrs) )
        return;

    //If the rectangle is completely inside the PrtArea, no border needs to
    //be painted.
    //For the PrtArea the aligned value needs to be used, otherwise it could
    //happen, that some parts won't be processed.
    SwRect aRect( getFramePrintArea() );
    aRect += getFrameArea().Pos();
    ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
    // new local boolean variable in order to
    // suspend border paint under special cases - see below.
    // NOTE: This is a fix for the implementation of feature #99657#.
    bool bDrawOnlyShadowForTransparentFrame = false;
    if ( aRect.Contains( rRect ) )
    {
        // paint shadow, if background is transparent.
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

    ::lcl_CalcBorderRect( aRect, this, rAttrs, true, gProp );
    rAttrs.SetGetCacheLine( true );

    if(bShadow)
    {
        PaintShadow(rRect, aRect, rAttrs);
    }

    // suspend drawing of border
    // add condition < NOT bDrawOnlyShadowForTransparentFrame > - see above
    // - add condition <bFoundCellForTopOrBorderAttrs>
    //-hack.
    if((bLine || bFoundCellForTopOrBorderAttrs) && !bDrawOnlyShadowForTransparentFrame)
    {
        // define SvxBorderLine(s) to use
        const SvxBoxItem& rBox(rAttrs.GetBox());
        const SvxBorderLine* pLeftBorder(rBox.GetLeft());
        const SvxBorderLine* pRightBorder(rBox.GetRight());
        const SvxBorderLine* pTopBorder(rBox.GetTop());
        const SvxBorderLine* pBottomBorder(rBox.GetBottom());

        // if R2L, exchange Right/Left
        const bool bR2L(IsCellFrame() && IsRightToLeft());

        if(bR2L)
        {
            std::swap(pLeftBorder, pRightBorder);
        }

        // if ContentFrame and joined Prev/Next, reset top/bottom as needed
        if(IsContentFrame())
        {
            const SwFrame* pDirRefFrame(IsCellFrame() ? FindTabFrame() : this);
            const SwRectFnSet aRectFnSet(pDirRefFrame);
            const SwRectFn& _rRectFn(aRectFnSet.FnRect());

            if(rAttrs.JoinedWithPrev(*this))
            {
                // tdf#115296 re-add adaptation of vert distance to close the evtl.
                // existing gap to previous frame
                const SwFrame* pPrevFrame(GetPrev());
                (aRect.*_rRectFn->fnSetTop)( (pPrevFrame->*_rRectFn->fnGetPrtBottom)() );

                // ...and disable top border paint/creation
                pTopBorder = nullptr;
            }

            if(rAttrs.JoinedWithNext(*this))
            {
                // tdf#115296 re-add adaptation of vert distance to close the evtl.
                // existing gap to next frame
                const SwFrame* pNextFrame(GetNext());
                (aRect.*_rRectFn->fnSetBottom)( (pNextFrame->*_rRectFn->fnGetPrtTop)() );

                // ...and disable bottom border paint/creation
                pBottomBorder = nullptr;
            }
        }

        // necessary to replace TopBorder?
        if((!IsContentFrame() || rAttrs.GetTopLine(*this)) && IsCellFrame() && pCellFrameForTopBorderAttrs != this)
        {
            SwBorderAttrAccess aAccess(SwFrame::GetCache(), pCellFrameForTopBorderAttrs);
            pTopBorder = aAccess.Get()->GetBox().GetTop();
        }

        // necessary to replace BottomBorder?
        if((!IsContentFrame() || rAttrs.GetBottomLine(*this)) && IsCellFrame() && pCellFrameForBottomBorderAttrs != this)
        {
            SwBorderAttrAccess aAccess(SwFrame::GetCache(), pCellFrameForBottomBorderAttrs);
            pBottomBorder = aAccess.Get()->GetBox().GetBottom();
        }

        if(nullptr != pLeftBorder || nullptr != pRightBorder || nullptr != pTopBorder || nullptr != pBottomBorder)
        {
            // now we have all SvxBorderLine(s) sorted out, create geometry
            const basegfx::B2DHomMatrix aBorderTransform(
                basegfx::utils::createScaleTranslateB2DHomMatrix(
                    aRect.Width(), aRect.Height(),
                    aRect.Left(), aRect.Top()));
            const svx::frame::Style aStyleTop(pTopBorder, 1.0);
            const svx::frame::Style aStyleRight(pRightBorder, 1.0);
            const svx::frame::Style aStyleBottom(pBottomBorder, 1.0);
            const svx::frame::Style aStyleLeft(pLeftBorder, 1.0);
            drawinglayer::primitive2d::Primitive2DContainer aBorderLineTarget;

            aBorderLineTarget.append(
                drawinglayer::primitive2d::Primitive2DReference(
                    new drawinglayer::primitive2d::SwBorderRectanglePrimitive2D(
                        aBorderTransform,
                        aStyleTop,
                        aStyleRight,
                        aStyleBottom,
                        aStyleLeft)));
            gProp.pBLines->AddBorderLines(std::move(aBorderLineTarget));
        }
    }

    rAttrs.SetGetCacheLine( false );
}

/**
 * Special implementation because of the footnote line
 *
 * Currently only the top frame needs to be taken into account
 * Other lines and shadows are set aside
 */
void SwFootnoteContFrame::PaintSwFrameShadowAndBorder(
    const SwRect& rRect,
    const SwPageFrame* pPage,
    const SwBorderAttrs&) const
{
    //If the rectangle is completely inside the PrtArea, no border needs to
    //be painted.
    SwRect aRect( getFramePrintArea() );
    aRect.Pos() += getFrameArea().Pos();
    if ( !aRect.Contains( rRect ) )
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

    SwRectFnSet aRectFnSet(this);
    SwTwips nPrtWidth = aRectFnSet.GetWidth(getFramePrintArea());
    Fraction aFract( nPrtWidth, 1 );
    aFract *= rInf.GetWidth();
    const SwTwips nWidth = static_cast<tools::Long>(aFract);

    SwTwips nX = aRectFnSet.GetPrtLeft(*this);
    switch ( rInf.GetAdj() )
    {
        case css::text::HorizontalAdjust_CENTER:
            nX += nPrtWidth/2 - nWidth/2; break;
        case css::text::HorizontalAdjust_RIGHT:
            nX += nPrtWidth - nWidth; break;
        case css::text::HorizontalAdjust_LEFT:
            /* do nothing */; break;
        default:
            SAL_WARN("sw.core", "New adjustment for footnote lines?");
            assert(false);
    }
    SwTwips nLineWidth = rInf.GetLineWidth();
    const SwRect aLineRect = aRectFnSet.IsVert() ?
        SwRect( Point(getFrameArea().Left()+getFrameArea().Width()-rInf.GetTopDist()-nLineWidth,
                      nX), Size( nLineWidth, nWidth ) )
            : SwRect( Point( nX, getFrameArea().Pos().Y() + rInf.GetTopDist() ),
                            Size( nWidth, rInf.GetLineWidth()));
    if ( aLineRect.HasArea() && rInf.GetLineStyle() != SvxBorderLineStyle::NONE)
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

    SwRectFn fnRect = pCol->IsVertical() ? ( pCol->IsVertLR() ? (pCol->IsVertLRBT() ? fnRectVertL2RB2T : fnRectVertL2R) : fnRectVert ) : fnRectHori;

    SwRect aLineRect = getFramePrintArea();
    aLineRect += getFrameArea().Pos();

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
            ( (pCol->getFrameArea().*fnGetX)() - nPenHalf );
        if ( aRect.Overlaps( aLineRect ) )
            PaintBorderLine( aRect, aLineRect , pPage, &rFormatCol.GetLineColor(),
                   rFormatCol.GetLineStyle() );
        pCol = pCol->GetNext();
    }
}

void SwPageFrame::PaintGrid( OutputDevice const * pOut, SwRect const &rRect ) const
{
    if( !m_bHasGrid || gProp.pSRetoucheFly || gProp.pSRetoucheFly2 )
        return;
    SwTextGridItem const*const pGrid(GetGridItem(this));
    if( !(pGrid && ( OUTDEV_PRINTER != pOut->GetOutDevType() ?
        pGrid->GetDisplayGrid() : pGrid->GetPrintGrid() )) )
        return;

    const SwLayoutFrame* pBody = FindBodyCont();
    if( !pBody )
        return;

    SwRect aGrid( pBody->getFramePrintArea() );
    aGrid += pBody->getFrameArea().Pos();

    SwRect aInter( aGrid );
    aInter.Intersection( rRect );
    if( !aInter.HasArea() )
        return;

    bool bGrid = pGrid->GetRubyTextBelow();
    bool bCell = GRID_LINES_CHARS == pGrid->GetGridType();
    tools::Long nGrid = pGrid->GetBaseHeight();
    const SwDoc* pDoc = GetFormat()->GetDoc();
    tools::Long nGridWidth = GetGridWidth(*pGrid, *pDoc);
    tools::Long nRuby = pGrid->GetRubyHeight();
    tools::Long nSum = nGrid + nRuby;
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
            aTmp.Pos().setX( nY );
            if( bGrid )
            {
                nY -= nGrid;
                SwTwips nPosY = std::max( SwTwips(aInter.Left()), nY );
                SwTwips nHeight = std::min(nRight, SwTwips(aTmp.Pos().X()))-nPosY;
                if( nHeight > 0 )
                {
                    if( bCell )
                    {
                        SwRect aVert( Point( nPosY, nX ),
                                    Size( nHeight, 1 ) );
                        while( aVert.Top() <= nBottom )
                        {
                            PaintBorderLine(rRect,aVert,this,pCol);
                            aVert.Pos().AdjustY(nGrid );
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
                            aVert.Pos().setY( nGridBottom );
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
                    SwTwips nPos = std::max( SwTwips(aInter.Left()), nY );
                    SwTwips nW = std::min(nRight, SwTwips(aTmp.Pos().X())) - nPos;
                    SwRect aVert( Point( nPos, aGrid.Top() ),
                                  Size( nW, 1 ) );
                    if( nW > 0 )
                    {
                        if( bLeft )
                            PaintBorderLine(rRect,aVert,this,pCol);
                        if( bRight )
                        {
                            aVert.Pos().setY( nGridBottom );
                            PaintBorderLine(rRect,aVert,this,pCol);
                        }
                    }
                }
            }
            bGrid = !bGrid;
        }
        while( nY >= aInter.Left() )
        {
            aTmp.Pos().setX( nY );
            PaintBorderLine( rRect, aTmp, this, pCol);
            if( bGrid )
            {
                nY -= nGrid;
                SwTwips nHeight = aTmp.Pos().X()
                                  - std::max(SwTwips(aInter.Left()), nY );
                if( nHeight > 0 )
                {
                    if( bCell )
                    {
                        SwRect aVert( Point(aTmp.Pos().X()-nHeight,
                                      nX ), Size( nHeight, 1 ) );
                        while( aVert.Top() <= nBottom )
                        {
                            PaintBorderLine(rRect,aVert,this,pCol);
                            aVert.Pos().AdjustY(nGrid );
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
                            aVert.Pos().setY( nGridBottom );
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
                    SwTwips nPos = std::max( SwTwips(aInter.Left()), nY );
                    SwTwips nW = std::min(nRight, SwTwips(aTmp.Pos().X())) - nPos;
                    SwRect aVert( Point( nPos, aGrid.Top() ),
                                  Size( nW, 1 ) );
                    if( nW > 0 )
                    {
                        if( bLeft )
                            PaintBorderLine(rRect,aVert,this,pCol);
                        if( bRight )
                        {
                            aVert.Pos().setY( nGridBottom );
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
                            aVert.Pos().AdjustX(nGridWidth );  //for textgrid refactor
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
                            aVert.Pos().setX( nGridRight );
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
                                 SwViewShell const * _pViewShell ) const
{
    if (  !_pViewShell->GetWin() || _pViewShell->GetViewOptions()->getBrowseMode() )
        return;

    // Simplified paint with DrawingLayer FillStyle
    SwRect aPgRect = getFrameArea();
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

const sal_Int8 SwPageFrame::snShadowPxWidth = 9;

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
                                                OutputDevice const * pRenderContext,
                                                SwRect&       _orHorizontalShadowRect,
                                                bool bPaintLeftShadow,
                                                bool bPaintRightShadow,
                                                bool bRightSidebar )
{
    const SwPostItMgr *pMgr = _pViewShell->GetPostItMgr();
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell, pRenderContext );
    SwRect aPagePxRect(pRenderContext->LogicToPixel( aAlignedPageRect.SVRect() ));

    tools::Long lShadowAdjustment = snShadowPxWidth - 1; // TODO: extract this

    _orHorizontalShadowRect.Chg(
                    Point( aPagePxRect.Left() + (bPaintLeftShadow ? lShadowAdjustment : 0), 0 ),
                    Size( aPagePxRect.Width() - ( (bPaintLeftShadow ? lShadowAdjustment : 0) + (bPaintRightShadow ? lShadowAdjustment : 0) ),
                        snShadowPxWidth ) );

    if(pMgr && pMgr->ShowNotes() && pMgr->HasNotes())
    {
        // Notes are displayed, we've to extend borders
        SwTwips aSidebarTotalWidth = pMgr->GetSidebarWidth(true) + pMgr->GetSidebarBorderWidth(true);
        if(bRightSidebar)
            _orHorizontalShadowRect.AddRight( aSidebarTotalWidth );
        else
            _orHorizontalShadowRect.AddLeft( - aSidebarTotalWidth );
    }
}

namespace {

enum PaintArea {LEFT, RIGHT, TOP, BOTTOM};

}

#define BORDER_TILE_SIZE 512

/// Wrapper around pOut->DrawBitmapEx.
static void lcl_paintBitmapExToRect(vcl::RenderContext *pOut, const Point& aPoint, const Size& aSize, const BitmapEx& rBitmapEx, PaintArea eArea)
{
    if(!comphelper::LibreOfficeKit::isActive())
    {
        // The problem is that if we get called multiple times and the color is
        // partly transparent, then the result will get darker and darker. To avoid
        // this, always paint the background color before doing the real paint.
        tools::Rectangle aRect(aPoint, aSize);

        if (!aRect.IsEmpty())
        {
            switch (eArea)
            {
            case LEFT: aRect.SetLeft( aRect.Right() - 1 ); break;
            case RIGHT: aRect.SetRight( aRect.Left() + 1 ); break;
            case TOP: aRect.SetTop( aRect.Bottom() - 1 ); break;
            case BOTTOM: aRect.SetBottom( aRect.Top() + 1 ); break;
            }
        }

        pOut->SetFillColor(SwViewOption::GetAppBackgroundColor());
        pOut->SetLineColor();
        pOut->DrawRect(pOut->PixelToLogic(aRect));
    }

    // Tiled render if necessary
    tools::Rectangle aComplete(aPoint, aSize);
    Size aTileSize(BORDER_TILE_SIZE, BORDER_TILE_SIZE);

    tools::Long iterX = eArea != RIGHT && eArea != LEFT ? BORDER_TILE_SIZE : 0;
    tools::Long iterY = eArea == RIGHT || eArea == LEFT ? BORDER_TILE_SIZE : 0;

    for (tools::Rectangle aTile(aPoint, aTileSize); true; aTile.Move(iterX, iterY))
    {
        tools::Rectangle aRender = aComplete.GetIntersection(aTile);
        if (aRender.IsEmpty())
            break;
        pOut->DrawBitmapEx(pOut->PixelToLogic(aRender.TopLeft()),
                           pOut->PixelToLogic(aRender.GetSize()),
                           Point(0, 0), aRender.GetSize(),
                           rBitmapEx);
    }

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

    static vcl::DeleteOnDeinit<drawinglayer::primitive2d::DiscreteShadow> shadowMaskObj(
            vcl::bitmap::loadFromName(BMP_PAGE_SHADOW_MASK,
                                      ImageLoadFlags::IgnoreDarkTheme | ImageLoadFlags::IgnoreScalingFactor));

    drawinglayer::primitive2d::DiscreteShadow& shadowMask = *shadowMaskObj.get();
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopRightShadowObj {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomRightShadowObj {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomLeftShadowObj  {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageBottomShadowBaseObj  {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageRightShadowBaseObj  {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopShadowBaseObj  {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageTopLeftShadowObj  {};
    static vcl::DeleteOnDeinit< BitmapEx > aPageLeftShadowBaseObj  {};
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
    SwRect aPagePxRect(_pViewShell->GetOut()->LogicToPixel( aAlignedPageRect.SVRect() ));

    if (aShadowColor != SwViewOption::GetShadowColor())
    {
        aShadowColor = SwViewOption::GetShadowColor();

        AlphaMask aMask( shadowMask.getBottomRight().GetBitmap() );
        Bitmap aFilledSquare(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageBottomRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getBottomLeft().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageBottomLeftShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getBottom().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageBottomShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTop().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageTopShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopRight().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageTopRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getRight().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageRightShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getTopLeft().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageTopLeftShadow = BitmapEx( aFilledSquare, aMask );

        aMask = AlphaMask( shadowMask.getLeft().GetBitmap() );
        aFilledSquare = Bitmap(aMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
        aFilledSquare.Erase( aShadowColor );
        aPageLeftShadow = BitmapEx( aFilledSquare, aMask );
    }

    SwRect aPaintRect;
    OutputDevice *pOut = _pViewShell->GetOut();

    SwPageFrame::GetHorizontalShadowRect( _rPageRect, _pViewShell, pOut, aPaintRect, bPaintLeftShadow, bPaintRightShadow, bRightSidebar );

    // Right shadow & corners
    if ( bPaintRightShadow )
    {
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right(), aPagePxRect.Bottom() + 1 - (aPageBottomRightShadow.GetSizePixel().Height() - snShadowPxWidth) ) ),
            aPageBottomRightShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( aPaintRect.Right(), aPagePxRect.Top() - snShadowPxWidth ) ),
            aPageTopRightShadow );

        if (aPagePxRect.Height() > 2 * snShadowPxWidth)
        {
            const tools::Long nWidth = aPageRightShadow.GetSizePixel().Width();
            const tools::Long nHeight = aPagePxRect.Height() - 2 * (snShadowPxWidth - 1);
            if (aPageRightShadow.GetSizePixel().Height() < BORDER_TILE_SIZE)
                aPageRightShadow.Scale(Size(nWidth, BORDER_TILE_SIZE), BmpScaleFlag::Fast);

            lcl_paintBitmapExToRect(pOut,
                    Point(aPaintRect.Right() + snShadowPxWidth, aPagePxRect.Top() + snShadowPxWidth - 1),
                    Size(nWidth, nHeight),
                    aPageRightShadow, RIGHT);
        }
    }

    // Left shadows and corners
    if(bPaintLeftShadow)
    {
        const tools::Long lLeft = aPaintRect.Left() - aPageBottomLeftShadow.GetSizePixel().Width();
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft,
            aPagePxRect.Bottom() + 1 + snShadowPxWidth - aPageBottomLeftShadow.GetSizePixel().Height() ) ), aPageBottomLeftShadow );
        pOut->DrawBitmapEx( pOut->PixelToLogic( Point( lLeft, aPagePxRect.Top() - snShadowPxWidth ) ), aPageTopLeftShadow );
        if (aPagePxRect.Height() > 2 * snShadowPxWidth)
        {
            const tools::Long nWidth = aPageLeftShadow.GetSizePixel().Width();
            const tools::Long nHeight = aPagePxRect.Height() - 2 * (snShadowPxWidth - 1);
            if (aPageLeftShadow.GetSizePixel().Height() < BORDER_TILE_SIZE)
                aPageLeftShadow.Scale(Size(nWidth, BORDER_TILE_SIZE), BmpScaleFlag::Fast);

            lcl_paintBitmapExToRect(pOut,
                    Point(lLeft, aPagePxRect.Top() + snShadowPxWidth - 1),
                    Size(nWidth, nHeight),
                    aPageLeftShadow, LEFT);
        }
    }

    // Bottom shadow
    const tools::Long nBottomHeight = aPageBottomShadow.GetSizePixel().Height();
    if (aPageBottomShadow.GetSizePixel().Width() < BORDER_TILE_SIZE)
        aPageBottomShadow.Scale(Size(BORDER_TILE_SIZE, nBottomHeight), BmpScaleFlag::Fast);

    lcl_paintBitmapExToRect(pOut,
            Point(aPaintRect.Left(), aPagePxRect.Bottom() + 2),
            Size(aPaintRect.Width(), nBottomHeight),
            aPageBottomShadow, BOTTOM);

    // Top shadow
    const tools::Long nTopHeight = aPageTopShadow.GetSizePixel().Height();
    if (aPageTopShadow.GetSizePixel().Width() < BORDER_TILE_SIZE)
        aPageTopShadow.Scale(Size(BORDER_TILE_SIZE, nTopHeight), BmpScaleFlag::Fast);

    lcl_paintBitmapExToRect(pOut,
            Point(aPaintRect.Left(), aPagePxRect.Top() - snShadowPxWidth),
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
    if (!(pMgr && pMgr->ShowNotes() && pMgr->HasNotes()))  // do not show anything in print preview
        return;

    sal_Int32 nScrollerHeight = pMgr->GetSidebarScrollerHeight();
    const tools::Rectangle &aVisRect = _pViewShell->VisArea().SVRect();
    //draw border and sidepane
    _pViewShell->GetOut()->SetLineColor();
    if (!bRight)
    {
        _pViewShell->GetOut()->SetFillColor(SwViewOption::GetObjectBoundariesColor());
        _pViewShell->GetOut()->DrawRect(tools::Rectangle(Point(aPageRect.Left()-pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarBorderWidth(),aPageRect.Height())))    ;
        if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            _pViewShell->GetOut()->SetFillColor(COL_BLACK);
        else
            _pViewShell->GetOut()->SetFillColor(SwViewOption::GetSectionBoundColor());
        _pViewShell->GetOut()->DrawRect(tools::Rectangle(Point(aPageRect.Left()-pMgr->GetSidebarWidth()-pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarWidth(),aPageRect.Height())))  ;
    }
    else
    {
        _pViewShell->GetOut()->SetFillColor(SwViewOption::GetObjectBoundariesColor());
        SwRect aSidebarBorder(aPageRect.TopRight(),Size(pMgr->GetSidebarBorderWidth(),aPageRect.Height()));
        _pViewShell->GetOut()->DrawRect(aSidebarBorder.SVRect());
        if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
            _pViewShell->GetOut()->SetFillColor(COL_BLACK);
        else
            _pViewShell->GetOut()->SetFillColor(SwViewOption::GetSectionBoundColor());
        SwRect aSidebar(Point(aPageRect.Right()+pMgr->GetSidebarBorderWidth(),aPageRect.Top()),Size(pMgr->GetSidebarWidth(),aPageRect.Height()));
        _pViewShell->GetOut()->DrawRect(aSidebar.SVRect());
    }
    if (!pMgr->ShowScrollbar(nPageNum))
        return;

    // draw scrollbar area and arrows
    Point aPointBottom;
    Point aPointTop;
    aPointBottom = !bRight ? Point(aPageRect.Left() - pMgr->GetSidebarWidth() - pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- _pViewShell->GetOut()->PixelToLogic(Size(0,2+pMgr->GetSidebarScrollerHeight())).Height()) :
                            Point(aPageRect.Right() + pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- _pViewShell->GetOut()->PixelToLogic(Size(0,2+pMgr->GetSidebarScrollerHeight())).Height());
    aPointTop = !bRight ?    Point(aPageRect.Left() - pMgr->GetSidebarWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + _pViewShell->GetOut()->PixelToLogic(Size(0,2)).Height()) :
                        Point(aPageRect.Right() + pMgr->GetSidebarBorderWidth() + _pViewShell->GetOut()->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + _pViewShell->GetOut()->PixelToLogic(Size(0,2)).Height());
    Size aSize(pMgr->GetSidebarWidth() - _pViewShell->GetOut()->PixelToLogic(Size(4,0)).Width(), _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()) ;
    tools::Rectangle aRectBottom(aPointBottom,aSize);
    tools::Rectangle aRectTop(aPointTop,aSize);

    if (aRectBottom.Overlaps(aVisRect))
    {

        if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
        {
            _pViewShell->GetOut()->SetLineColor(COL_WHITE);
            _pViewShell->GetOut()->SetFillColor(COL_BLACK);
        }
        else
        {
            _pViewShell->GetOut()->SetLineColor(COL_BLACK);
            _pViewShell->GetOut()->SetFillColor(COL_LIGHTGRAY);
        }
        _pViewShell->GetOut()->DrawRect(aRectBottom);
        _pViewShell->GetOut()->DrawLine(aPointBottom + Point(pMgr->GetSidebarWidth()/3,0), aPointBottom + Point(pMgr->GetSidebarWidth()/3 , _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()));

        _pViewShell->GetOut()->SetLineColor();
        Point aMiddleFirst(aPointBottom + Point(pMgr->GetSidebarWidth()/6,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
        Point aMiddleSecond(aPointBottom + Point(pMgr->GetSidebarWidth()/3*2,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
        PaintNotesSidebarArrows(aMiddleFirst,aMiddleSecond,_pViewShell,pMgr->GetArrowColor(KEY_PAGEUP,nPageNum), pMgr->GetArrowColor(KEY_PAGEDOWN,nPageNum));
    }
    if (!aRectTop.Overlaps(aVisRect))
        return;

    if (Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        _pViewShell->GetOut()->SetLineColor(COL_WHITE);
        _pViewShell->GetOut()->SetFillColor(COL_BLACK);
    }
    else
    {
        _pViewShell->GetOut()->SetLineColor(COL_BLACK);
        _pViewShell->GetOut()->SetFillColor(COL_LIGHTGRAY);
    }
    _pViewShell->GetOut()->DrawRect(aRectTop);
    _pViewShell->GetOut()->DrawLine(aPointTop + Point(pMgr->GetSidebarWidth()/3*2,0), aPointTop + Point(pMgr->GetSidebarWidth()/3*2 , _pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()));

    _pViewShell->GetOut()->SetLineColor();
    Point aMiddleFirst(aPointTop + Point(pMgr->GetSidebarWidth()/3,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
    Point aMiddleSecond(aPointTop + Point(pMgr->GetSidebarWidth()/6*5,_pViewShell->GetOut()->PixelToLogic(Size(0,nScrollerHeight)).Height()/2));
    PaintNotesSidebarArrows(aMiddleFirst,aMiddleSecond,_pViewShell, pMgr->GetArrowColor(KEY_PAGEUP,nPageNum), pMgr->GetArrowColor(KEY_PAGEDOWN,nPageNum));
}

/*static*/ void SwPageFrame::PaintNotesSidebarArrows(const Point &aMiddleFirst, const Point &aMiddleSecond, SwViewShell const * _pViewShell, const Color& rColorUp, const Color& rColorDown)
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
                                                        OutputDevice const * pRenderContext,
                                                        SwRect& _orBorderAndShadowBoundRect,
                                                        bool bLeftShadow,
                                                        bool bRightShadow,
                                                        bool bRightSidebar
                                                      )
{
    SwRect aAlignedPageRect( _rPageRect );
    ::SwAlignRect( aAlignedPageRect, _pViewShell, pRenderContext );
    SwRect aPagePxRect(pRenderContext->LogicToPixel( aAlignedPageRect.SVRect() ));
    aPagePxRect.AddBottom( snShadowPxWidth + 1 );
    aPagePxRect.AddTop( - snShadowPxWidth - 1 );

    SwRect aTmpRect;

    // Always ask for full shadow since we want a bounding rect
    // including at least the page frame
    SwPageFrame::GetHorizontalShadowRect( _rPageRect, _pViewShell, pRenderContext, aTmpRect, false, false, bRightSidebar );

    if(bLeftShadow) aPagePxRect.Left( aTmpRect.Left() - snShadowPxWidth - 1);
    if(bRightShadow) aPagePxRect.Right( aTmpRect.Right() + snShadowPxWidth + 1);

    _orBorderAndShadowBoundRect = SwRect(pRenderContext->PixelToLogic( aPagePxRect.SVRect() ));
}

SwRect SwPageFrame::GetBoundRect(OutputDevice const * pOutputDevice) const
{
    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    SwRect aPageRect( getFrameArea() );
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

    pOut->Push( vcl::PushFlags::FILLCOLOR|vcl::PushFlags::LINECOLOR );
    pOut->SetLineColor();

    SwBorderAttrAccess aAccess( SwFrame::GetCache(), this );
    const SwBorderAttrs &rAttrs = *aAccess.Get();

    // take care of page margin area
    // Note: code move from <SwFrame::PaintSwFrameBackground(..)> to new method
    // <SwPageFrame::Paintmargin(..)>.
    if ( IsPageFrame() && !bOnlyTextBackground)
    {
        static_cast<const SwPageFrame*>(this)->PaintMarginArea( rRect, gProp.pSGlobalShell );
    }

    // paint background
    {
        PaintSwFrameBackground( rRect, pPage, rAttrs, false, true/*bLowerBorder*/, bOnlyTextBackground );
    }

    // paint border before painting background
    // paint grid for page frame and paint border
    if (!bOnlyTextBackground)
    {
        SwRect aRect( rRect );

        if( IsPageFrame() )
        {
            static_cast<const SwPageFrame*>(this)->PaintGrid( pOut, aRect );
        }

        PaintSwFrameShadowAndBorder(aRect, pPage, rAttrs);
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

/// Do not paint background for fly frames without a background brush by
/// calling <PaintBaBo> at the page or at the fly frame its anchored
void SwFrame::PaintSwFrameBackground( const SwRect &rRect, const SwPageFrame *pPage,
                             const SwBorderAttrs & rAttrs,
                             const bool bLowerMode,
                             const bool bLowerBorder,
                             const bool bOnlyTextBackground ) const
{
    // #i1837# - no paint of table background, if corresponding option is *not* set.
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
    // temporary background brush for a fly frame without a background brush
    std::unique_ptr<SvxBrushItem> pTmpBackBrush;
    std::optional<Color> pCol;
    SwRect aOrigBackRect;
    const bool bPageFrame = IsPageFrame();
    bool bLowMode = true;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    bool bBack = GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, bLowerMode, /*bConsiderTextBox=*/false );

    // show track changes of table row
    if( IsRowFrame() && !getRootFrame()->IsHideRedlines() )
    {
        RedlineType eType = static_cast<const SwRowFrame*>(this)->GetTabLine()->GetRedlineType();
        if ( RedlineType::Delete == eType || RedlineType::Insert == eType )
        {
            pCol = RedlineType::Delete == eType ? COL_AUTHOR_TABLE_DEL : COL_AUTHOR_TABLE_INS;
            bBack = true;
        }
    }
    else if ( bBack && IsCellFrame() && !getRootFrame()->IsHideRedlines() &&
        // skip cell background to show the row colored according to its tracked change
        RedlineType::None != static_cast<const SwRowFrame*>(GetUpper())->GetTabLine()->GetRedlineType() )
    {
        return;
    }

    //- Output if a separate background is used.
    bool bNoFlyBackground = !gProp.bSFlyMetafile && !bBack && IsFlyFrame();
    if ( bNoFlyBackground )
    {
        // Fly frame has no background.
        // Try to find background brush at parents, if previous call of
        // <GetBackgroundBrush> disabled this option with the parameter <bLowerMode>
        if ( bLowerMode )
        {
            bBack = GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigBackRect, false, /*bConsiderTextBox=*/false );
        }
        // If still no background found for the fly frame, initialize the
        // background brush <pItem> with global retouche color and set <bBack>
        // to true, that fly frame will paint its background using this color.
        if ( !bBack )
        {
            //  #i6467# - on print output, pdf output and in embedded mode not editing color COL_WHITE is used
            // instead of the global retouche color.
            if ( pSh->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
                 pSh->GetViewOptions()->IsPDFExport() ||
                 ( pSh->GetDoc()->GetDocShell()->GetCreateMode() == SfxObjectCreateMode::EMBEDDED &&
                   !pSh->GetDoc()->GetDocShell()->IsInPlaceActive()
                 )
               )
            {
                pTmpBackBrush.reset(new SvxBrushItem( COL_WHITE, RES_BACKGROUND ));

                //UUU
                aFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(COL_WHITE);
            }
            else
            {
                pTmpBackBrush.reset(new SvxBrushItem( aGlobalRetoucheColor, RES_BACKGROUND));

                //UUU
                aFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aGlobalRetoucheColor);
            }

            pItem = pTmpBackBrush.get();
            bBack = true;
        }
    }

    SwRect aPaintRect( getFrameArea() );
    if( IsTextFrame() || IsSctFrame() )
        aPaintRect = UnionFrame( true );

    // bOnlyTextBackground means background that's on top of background shapes,
    // this includes both text and cell frames.
    if ( (!bOnlyTextBackground || IsTextFrame() || IsCellFrame()) && aPaintRect.Overlaps( rRect ) )
    {
        if ( bBack || bPageFrame || !bLowerMode )
        {
            const bool bBrowse = pSh->GetViewOptions()->getBrowseMode();
            SwRect aRect;
            if ( (bPageFrame && bBrowse) ||
                 (IsTextFrame() && getFramePrintArea().SSize() == getFrameArea().SSize()) )
            {
                aRect = getFrameArea();
                ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
            }
            else
            {
                if (bPageFrame && GetAttrSet()->GetItem<SfxBoolItem>(RES_BACKGROUND_FULL_SIZE)->GetValue())
                {
                    aRect = getFrameArea();
                    ::SwAlignRect(aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut());
                }
                else
                {
                   ::lcl_CalcBorderRect( aRect, this, rAttrs, false, gProp);
                }

                if ( (IsTextFrame() || IsTabFrame()) && GetPrev() )
                {
                    if ( GetPrev()->GetAttrSet()->GetBackground() == GetAttrSet()->GetBackground() &&
                         lcl_compareFillAttributes(GetPrev()->getSdrAllFillAttributesHelper(), getSdrAllFillAttributesHelper()))
                    {
                        aRect.Top( getFrameArea().Top() );
                    }
                }
            }
            aRect.Intersection( rRect );

            OutputDevice *pOut = pSh->GetOut();

            if ( aRect.HasArea() )
            {
                std::unique_ptr<SvxBrushItem> pNewItem;

                if( pCol )
                {
                    pNewItem.reset(new SvxBrushItem( *pCol, RES_BACKGROUND ));
                    pItem = pNewItem.get();
                    aFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(*pCol);
                }

                SwRegionRects aRegion( aRect );
                basegfx::B2DPolygon aB2DPolygon{tools::Polygon(aRect.SVRect()).getB2DPolygon()};
                basegfx::utils::B2DClipState aClipState{basegfx::B2DPolyPolygon(aB2DPolygon)};
                if (pPage->GetSortedObjs() &&
                    pSh->GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::SUBTRACT_FLYS))
                {
                    ::lcl_SubtractFlys( this, pPage, aRect, aRegion, aClipState, gProp );
                }

                //  Determine, if background transparency
                //  have to be considered for drawing.
                //  Status Quo: background transparency have to be
                //  considered for fly frames
                const bool bConsiderBackgroundTransparency = IsFlyFrame();
                bool bDone(false);

                // #i125189# We are also done when the new DrawingLayer FillAttributes are used
                // or the FillStyle is set (different from drawing::FillStyle_NONE)
                if(pOut && aFillAttributes)
                {
                    if(aFillAttributes->isUsed())
                    {
                        // check if really something is painted
                        bDone = DrawFillAttributes(aFillAttributes, aOrigBackRect, aRegion, aClipState, *pOut);
                    }

                    if(!bDone)
                    {
                        // if not, still a FillStyle could be set but the transparency is at 100%,
                        // thus need to check the model data itself for FillStyle (do not rely on
                        // SdrAllFillAttributesHelper since it already contains optimized information,
                        // e.g. transparency leads to no fill)
                        const drawing::FillStyle eFillStyle(GetAttrSet()->Get(XATTR_FILLSTYLE).GetValue());

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
                    //  add 6th parameter to indicate, if background transparency have to be considered
                    //     Set missing 5th parameter to the default value GRFNUM_NO
                    //         - see declaration in /core/inc/frmtool.hxx.
                        ::DrawGraphic(
                                pItem,
                                *pOut,
                                aOrigBackRect,
                                aRegion[i],
                                GRFNUM_NO,
                                bConsiderBackgroundTransparency );
                    }
                }
            }
        }
        else
            bLowMode = bLowerMode;
    }

    // delete temporary background brush.
    pTmpBackBrush.reset();

    //Now process lower and his neighbour.
    //We end this as soon as a Frame leaves the chain and therefore is not a lower
    //of me anymore
    const SwFrame *pFrame = GetLower();
    if ( !pFrame )
        return;

    SwRect aFrameRect;
    SwRect aRect( GetPaintArea() );
    aRect.Intersection_( rRect );
    SwRect aBorderRect( aRect );
    SwShortCut aShortCut( *pFrame, aBorderRect );
    do
    {   if ( gProp.pSProgress )
            SfxProgress::Reschedule();

        aFrameRect = pFrame->GetPaintArea();
        if ( aFrameRect.Overlaps( aBorderRect ) )
        {
            SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
            const SwBorderAttrs &rTmpAttrs = *aAccess.Get();
            if ( ( pFrame->IsLayoutFrame() && bLowerBorder ) || aFrameRect.Overlaps( aRect ) )
            {
                pFrame->PaintSwFrameBackground( aRect, pPage, rTmpAttrs, bLowMode,
                                       bLowerBorder, bOnlyTextBackground );
            }

            if ( bLowerBorder )
            {
                pFrame->PaintSwFrameShadowAndBorder( aBorderRect, pPage, rTmpAttrs );
            }
        }
        pFrame = pFrame->GetNext();
    } while ( pFrame && pFrame->GetUpper() == this &&
              !aShortCut.Stop( aFrameRect ) );
}

/// Refreshes all subsidiary lines of a page.
void SwPageFrame::RefreshSubsidiary( const SwRect &rRect ) const
{
    if ( !(isSubsidiaryLinesEnabled() || isTableBoundariesEnabled()
        || isSubsidiaryLinesForSectionsEnabled() || isSubsidiaryLinesFlysEnabled()) )
        return;

    if ( !rRect.HasArea() )
        return;

    //During paint using the root, the array is controlled from there.
    //Otherwise we'll handle it for our self.
    bool bDelSubs = false;
    if ( !gProp.pSSubsLines )
    {
        gProp.pSSubsLines.reset(new SwSubsRects);
        // create container for special subsidiary lines
        gProp.pSSpecSubsLines.reset(new SwSubsRects);
        bDelSubs = true;
    }

    RefreshLaySubsidiary( this, rRect );

    if ( bDelSubs )
    {
        // paint special subsidiary lines and delete its container
        gProp.pSSpecSubsLines->PaintSubsidiary( gProp.pSGlobalShell->GetOut(), nullptr, gProp );
        gProp.pSSpecSubsLines.reset();

        gProp.pSSubsLines->PaintSubsidiary(gProp.pSGlobalShell->GetOut(), gProp.pSLines.get(), gProp);
        gProp.pSSubsLines.reset();
    }
}

void SwLayoutFrame::RefreshLaySubsidiary( const SwPageFrame *pPage,
                                        const SwRect &rRect ) const
{
    const bool bSubsOpt   = isSubsidiaryLinesEnabled();
    if ( bSubsOpt )
        PaintSubsidiaryLines( pPage, rRect );

    const SwFrame *pLow = Lower();
    if( !pLow )
        return;
    SwShortCut aShortCut( *pLow, rRect );
    while( pLow && !aShortCut.Stop( pLow->getFrameArea() ) )
    {
        if ( pLow->getFrameArea().Overlaps( rRect ) && pLow->getFrameArea().HasArea() )
        {
            if ( pLow->IsLayoutFrame() )
                static_cast<const SwLayoutFrame*>(pLow)->RefreshLaySubsidiary( pPage, rRect);
            else if ( pLow->GetDrawObjs() )
            {
                const SwSortedObjs& rObjs = *(pLow->GetDrawObjs());
                for (SwAnchoredObject* pAnchoredObj : rObjs)
                {
                    if ( pPage->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) )
                        if (auto pFly = pAnchoredObj->DynCastFlyFrame() )
                        {
                            if ( pFly->IsFlyInContentFrame() && pFly->getFrameArea().Overlaps( rRect ) )
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
                                  const SubColFlags nSubColor,
                                  SwLineRects* pSubsLines )
{
    //In which direction do we loop? Can only be horizontal or vertical.
    OSL_ENSURE( ((rP1.X() == rP2.X()) || (rP1.Y() == rP2.Y())),
            "Sloped subsidiary lines are not allowed." );

    const bool bHori = rP1.Y() == rP2.Y();

    // use pointers to member function in order to unify flow
    typedef tools::Long (Point::*pmfPtGet)() const;
    typedef void (Point::*pmfPtSet)(tools::Long);
    const pmfPtGet pDirPtX = &Point::X;
    const pmfPtGet pDirPtY = &Point::Y;
    const pmfPtGet pDirPt = bHori ? pDirPtX : pDirPtY;
    const pmfPtSet pDirPtSetX = &Point::setX;
    const pmfPtSet pDirPtSetY = &Point::setY;
    const pmfPtSet pDirPtSet = bHori ? pDirPtSetX : pDirPtSetY;

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

            // do *not* consider fly frames with a transparent background.
            // do *not* consider fly frame, which belongs to an invisible layer
            if ( pFly->IsBackgroundTransparent() ||
                 !pFly->GetFormat()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId( pObj->GetLayer() ) )
            {
                aIter.Next();
                continue;
            }

            //Is the Obj placed on the line
            const tools::Long nP1OthPt = !bHori ? rP1.X() : rP1.Y();
            const tools::Rectangle &rBound = pObj->GetCurrentBoundRect();
            const Point aDrPt( rBound.TopLeft() );
            const tools::Long nDrOthPt = !bHori ? aDrPt.X() : aDrPt.Y();
            const Size  aDrSz( rBound.GetSize() );
            const tools::Long nDrOthSz = !bHori ? aDrSz.Width() : aDrSz.Height();

            if ( nP1OthPt >= nDrOthPt && nP1OthPt <= nDrOthPt + nDrOthSz )
            {
                const tools::Long nDrDirPt = bHori ? aDrPt.X() : aDrPt.Y();
                const tools::Long nDrDirSz = bHori ? aDrSz.Width() : aDrSz.Height();

                if ( (aP1.*pDirPt)() >= nDrDirPt && (aP1.*pDirPt)() <= nDrDirPt + nDrDirSz )
                    (aP1.*pDirPtSet)( nDrDirPt + nDrDirSz );

                if ( (aP2.*pDirPt)() >= nDrDirPt && (aP1.*pDirPt)() < (nDrDirPt - 1) )
                    (aP2.*pDirPtSet)( nDrDirPt - 1 );
            }
            aIter.Next();
        }

        if ( (aP1.*pDirPt)() < (aP2.*pDirPt)() )
        {
            SwRect aRect( aP1, aP2 );
            // use parameter <pSubsLines> instead of global variable <gProp.pSSubsLines>.
            pSubsLines->AddLineRect( aRect, nullptr, SvxBorderLineStyle::SOLID,
                    nullptr, nSubColor, gProp );
        }
        aP1 = aP2;
        (aP1.*pDirPtSet)( (aP1.*pDirPt)() + 1 );
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
    double const aXOffDirs[] = { -1.0, 1.0, 1.0, -1.0 };
    double const aYOffDirs[] = { -1.0, -1.0, 1.0, 1.0 };

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

        aSeq[i] = new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aLineColor );
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

    aSeq[0] = new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aPolygon, aLineColor );

    return aSeq;
}

static drawinglayer::primitive2d::Primitive2DContainer lcl_CreateColumnAreaDelimiterPrimitives(
        const SwRect& rRect )
{
    drawinglayer::primitive2d::Primitive2DContainer aSeq( 4 );

    basegfx::BColor aLineColor = SwViewOption::GetDocBoundariesColor().getBColor();
    double nLineLength = 100.0; // in Twips

    Point aPoints[] = { rRect.TopLeft(), rRect.TopRight(), rRect.BottomRight(), rRect.BottomLeft() };
    double const aXOffDirs[] = { 1.0, -1.0, -1.0, 1.0 };
    double const aYOffDirs[] = { 1.0, 1.0, -1.0, -1.0 };

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

        aSeq[i] = new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    aPolygon, aLineColor );
    }

    return aSeq;
}

void SwPageFrame::PaintSubsidiaryLines( const SwPageFrame *,
                                        const SwRect & ) const
{
    if ( gProp.pSGlobalShell->IsHeaderFooterEdit() )
        return;

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

    SwRect aArea( pPageBody->getFrameArea() );
    if ( pFootnoteCont )
        aArea.AddBottom( pFootnoteCont->getFrameArea().Bottom() - aArea.Bottom() );

    if ( !gProp.pSGlobalShell->GetViewOptions()->IsViewMetaChars( ) )
        ProcessPrimitives( lcl_CreatePageAreaDelimiterPrimitives( aArea ) );
    else
        ProcessPrimitives( lcl_CreateRectangleDelimiterPrimitives( aArea ) );
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

    SwRect aArea( pColBody->getFrameArea() );

    // #i3662# - enlarge top of column body frame's printing area
    // in sections to top of section frame.
    const bool bColInSection =  GetUpper()->IsSctFrame();
    if ( bColInSection )
    {
        if ( IsVertical() )
            aArea.Right( GetUpper()->getFrameArea().Right() );
        else
            aArea.Top( GetUpper()->getFrameArea().Top() );
    }

    if ( pFootnoteCont )
        aArea.AddBottom( pFootnoteCont->getFrameArea().Bottom() - aArea.Bottom() );

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
        SwRect aArea( getFramePrintArea() );
        aArea.Pos() += getFrameArea().Pos();
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
    //  #i3662# - use frame area for cells for section use also frame area
    const bool bUseFrameArea = bCell || IsSctFrame();
    SwRect aOriginal( bUseFrameArea ? getFrameArea() : getFramePrintArea() );
    if ( !bUseFrameArea )
        aOriginal.Pos() += getFrameArea().Pos();

    ::SwAlignRect( aOriginal, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );

    if ( !aOriginal.Overlaps( rRect ) )
        return;

    SwRect aOut( aOriginal );
    aOut.Intersection_( rRect );

    const SwTwips nRight = aOut.Right();
    const SwTwips nBottom= aOut.Bottom();

    const Point aRT( nRight, aOut.Top() );
    const Point aRB( nRight, nBottom );
    const Point aLB( aOut.Left(), nBottom );

    SubColFlags nSubColor = ( bCell || IsRowFrame() )
                            ? SubColFlags::Tab
                            : ( IsInSct()
                                ? SubColFlags::Sect
                                : ( IsInFly() ? SubColFlags::Fly : SubColFlags::Page ) );

    // collect body, header, footer, footnote and section
    // sub-lines in <pSpecSubsLine> array.
    const bool bSpecialSublines = IsBodyFrame() || IsHeaderFrame() || IsFooterFrame() ||
                                  IsFootnoteFrame() || IsSctFrame();
    SwLineRects *const pUsedSubsLines = bSpecialSublines
            ? gProp.pSSpecSubsLines.get() : gProp.pSSubsLines.get();

    // NOTE: for cell frames only left and right (horizontal layout) respectively
    //      top and bottom (vertical layout) lines painted.
    // NOTE2: this does not hold for the new table model!!! We paint the top border
    // of each non-covered table cell.
    const bool bVert = IsVertical();
    if ( bFlys )
    {
        // add control for drawing left and right lines
        if ( !bCell || bNewTableModel || !bVert )
        {
            if ( aOriginal.Left() == aOut.Left() )
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aLB, nSubColor, pUsedSubsLines );
            // in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
                ::lcl_RefreshLine( this, pPage, aRT, aRB, nSubColor, pUsedSubsLines );
        }
        // adjust control for drawing top and bottom lines
        if ( !bCell || bNewTableModel || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
                // in horizontal layout set page/column break at top
                ::lcl_RefreshLine( this, pPage, aOut.Pos(), aRT, nSubColor, pUsedSubsLines );
            if ( aOriginal.Bottom() == nBottom )
                ::lcl_RefreshLine( this, pPage, aLB, aRB, nSubColor,
                                   pUsedSubsLines );
        }
    }
    else
    {
        // add control for drawing left and right lines
        if ( !bCell || bNewTableModel || !bVert )
        {
            if ( aOriginal.Left() == aOut.Left() )
            {
                const SwRect aRect( aOut.Pos(), aLB );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        SvxBorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
            // in vertical layout set page/column break at right
            if ( aOriginal.Right() == nRight )
            {
                const SwRect aRect( aRT, aRB );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        SvxBorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
        }
        // adjust control for drawing top and bottom lines
        if ( !bCell || bNewTableModel || bVert )
        {
            if ( aOriginal.Top() == aOut.Top() )
            {
                // in horizontal layout set page/column break at top
                const SwRect aRect( aOut.Pos(), aRT );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        SvxBorderLineStyle::SOLID, nullptr, nSubColor, gProp );
            }
            if ( aOriginal.Bottom() == nBottom )
            {
                const SwRect aRect( aLB, aRB );
                pUsedSubsLines->AddLineRect( aRect, nullptr,
                        SvxBorderLineStyle::SOLID, nullptr, nSubColor, gProp );
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
        || static_cast<sal_Int16>(SW_MOD()->GetRedlineMarkPos()) != text::HoriOrientation::NONE;

    SwRect aRect( rRect );
    ::SwAlignRect( aRect, gProp.pSGlobalShell, gProp.pSGlobalShell->GetOut() );
    if ( !aRect.HasArea() )
        return;

    SwLayoutFrame::RefreshExtraData( aRect );

    if ( bLineInFly && GetSortedObjs() )
        for (SwAnchoredObject* pAnchoredObj : *GetSortedObjs())
        {
            if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
            {
                if ( pFly->getFrameArea().Top() <= aRect.Bottom() &&
                     pFly->getFrameArea().Bottom() >= aRect.Top() )
                    pFly->RefreshExtraData( aRect );
            }
        }
}

void SwLayoutFrame::RefreshExtraData( const SwRect &rRect ) const
{

    const SwLineNumberInfo &rInfo = GetFormat()->GetDoc()->GetLineNumberInfo();
    bool bLineInBody = rInfo.IsPaintLineNumbers(),
             bLineInFly  = bLineInBody && rInfo.IsCountInFlys(),
             bRedLine = static_cast<sal_Int16>(SW_MOD()->GetRedlineMarkPos())!=text::HoriOrientation::NONE;

    const SwContentFrame *pCnt = ContainsContent();
    while ( pCnt && IsAnLower( pCnt ) )
    {
        if ( pCnt->IsTextFrame() && ( bRedLine ||
             ( !pCnt->IsInTab() &&
               ((bLineInBody && pCnt->IsInDocBody()) ||
               (bLineInFly  && pCnt->IsInFly())) ) ) &&
             pCnt->getFrameArea().Top() <= rRect.Bottom() &&
             pCnt->getFrameArea().Bottom() >= rRect.Top() )
        {
            static_cast<const SwTextFrame*>(pCnt)->PaintExtraData( rRect );
        }
        if ( bLineInFly && pCnt->GetDrawObjs() )
            for (SwAnchoredObject* pAnchoredObj : *pCnt->GetDrawObjs())
            {
                if ( auto pFly = pAnchoredObj->DynCastFlyFrame() )
                {
                    if ( pFly->IsFlyInContentFrame() &&
                         pFly->getFrameArea().Top() <= rRect.Bottom() &&
                         pFly->getFrameArea().Bottom() >= rRect.Top() )
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
Color SwPageFrame::GetDrawBackgroundColor() const
{
    const SvxBrushItem* pBrushItem;
    std::optional<Color> xDummyColor;
    SwRect aDummyRect;
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

    if ( GetBackgroundBrush( aFillAttributes, pBrushItem, xDummyColor, aDummyRect, true, /*bConsiderTextBox=*/false) )
    {
        if(aFillAttributes && aFillAttributes->isUsed())
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
    static vcl::Font aEmptyPgFont = []()
    {
        vcl::Font tmp;
        tmp.SetFontSize( Size( 0, 80 * 20 )); // == 80 pt
        tmp.SetWeight( WEIGHT_BOLD );
        tmp.SetStyleName(OUString());
        tmp.SetFamilyName("Helvetica");
        tmp.SetFamily( FAMILY_SWISS );
        tmp.SetTransparent( true );
        tmp.SetColor( COL_GRAY );
        return tmp;
    }();

    return aEmptyPgFont;
}

/**
 * Retouch for a section
 *
 * Retouch will only be done, if the Frame is the last one in his chain.
 * The whole area of the upper which is located below the Frame will be
 * cleared using PaintSwFrameBackground.
 */
void SwFrame::Retouch( const SwPageFrame * pPage, const SwRect &rRect ) const
{
    if ( gProp.bSFlyMetafile )
        return;

    OSL_ENSURE( GetUpper(), "Retouche try without Upper." );
    OSL_ENSURE( getRootFrame()->GetCurrShell() && gProp.pSGlobalShell->GetWin(), "Retouche on a printer?" );

    SwRect aRetouche( GetUpper()->GetPaintArea() );
    aRetouche.Top( getFrameArea().Top() + getFrameArea().Height() );
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
                const Color aPageBackgrdColor(pPage->GetDrawBackgroundColor());
                const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
                // --> OD #i76669#
                SwViewObjectContactRedirector aSwRedirector( *pSh );
                // <--

                pSh->Imp()->PaintLayer( rIDDMA.GetHellId(), nullptr,
                                        *pPage, rRetouche, &aPageBackgrdColor,
                                        pPage->IsRightToLeft(),
                                        &aSwRedirector );
                pSh->Imp()->PaintLayer( rIDDMA.GetHeavenId(), nullptr,
                                        *pPage, rRetouche, &aPageBackgrdColor,
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
 * (2) Background brush from an index section is taken under special conditions.
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
 * @param bConsiderTextBox
 * consider the TextBox of this fly frame (if there is any) when determining
 * the background color, useful for automatic font color.
 *
 * @return true, if a background brush for the frame is found
 */
bool SwFrame::GetBackgroundBrush(
    drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes,
    const SvxBrushItem* & rpBrush,
    std::optional<Color>& rxCol,
    SwRect &rOrigRect,
    bool bLowerMode,
    bool bConsiderTextBox ) const
{
    const SwFrame *pFrame = this;
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const SwViewOption *pOpt = pSh->GetViewOptions();
    rpBrush = nullptr;
    rxCol.reset();
    do
    {
        if ( pFrame->IsPageFrame() && !pOpt->IsPageBack() )
            return false;

        if (pFrame->supportsFullDrawingLayerFillAttributeSet())
        {
            bool bHandledTextBox = false;
            if (pFrame->IsFlyFrame() && bConsiderTextBox)
            {
                const SwFlyFrame* pFlyFrame = static_cast<const SwFlyFrame*>(pFrame);
                SwFrameFormat* pShape
                    = SwTextBoxHelper::getOtherTextBoxFormat(pFlyFrame->GetFormat(), RES_FLYFRMFMT);
                if (pShape)
                {
                    SdrObject* pObject = pShape->FindRealSdrObject();
                    if (pObject)
                    {
                        // Work with the fill attributes of the shape of the fly frame.
                        rFillAttributes =
                            std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(
                                pObject->GetMergedItemSet());
                        bHandledTextBox = true;
                    }
                }
            }

            if (!bHandledTextBox)
                rFillAttributes = pFrame->getSdrAllFillAttributesHelper();
        }
        const SvxBrushItem &rBack = pFrame->GetAttrSet()->GetBackground();

        if( pFrame->IsSctFrame() )
        {
            const SwSection* pSection = static_cast<const SwSectionFrame*>(pFrame)->GetSection();
            //     Note: If frame <pFrame> is a section of the index and
            //         it its background color is "no fill"/"auto fill" and
            //         it has no background graphic and
            //         we are not in the page preview and
            //         we are not in read-only mode and
            //         option "index shadings" is set and
            //         the output is not the printer
            //         then set <rpCol> to the color of the index shading
            if( pSection && (   SectionType::ToxHeader == pSection->GetType() ||
                                SectionType::ToxContent == pSection->GetType() ) &&
                (rBack.GetColor() == COL_TRANSPARENT) &&
                rBack.GetGraphicPos() == GPOS_NONE &&
                !pOpt->IsPagePreview() &&
                !pOpt->IsReadonly() &&
                // #114856# Form view
                !pOpt->IsFormView() &&
                SwViewOption::IsIndexShadings() &&
                !pOpt->IsPDFExport() &&
                pSh->GetOut()->GetOutDevType() != OUTDEV_PRINTER )
            {
                rxCol = SwViewOption::GetIndexShadingsColor();
            }
        }

        //     determine, if background draw of frame <pFrame> considers transparency
        //     Status Quo: background transparency have to be
        //                     considered for fly frames
        const bool bConsiderBackgroundTransparency = pFrame->IsFlyFrame();

        // #i125189# Do not base the decision for using the parent's fill style for this
        // frame when the new DrawingLayer FillAttributes are used on the SdrAllFillAttributesHelper
        // information. There the data is already optimized to no fill in the case that the
        // transparence is at 100% while no fill is the criteria for derivation
        bool bNewDrawingLayerFillStyleIsUsedAndNotNoFill(false);

        if(rFillAttributes)
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
                const drawing::FillStyle eFillStyle(pFrame->GetAttrSet()->Get(XATTR_FILLSTYLE).GetValue());

                if(drawing::FillStyle_NONE != eFillStyle)
                {
                    bNewDrawingLayerFillStyleIsUsedAndNotNoFill = true;
                }
            }
        }

        //     add condition:
        //     If <bConsiderBackgroundTransparency> is set - see above -,
        //     return brush of frame <pFrame>, if its color is *not* "no fill"/"auto fill"
        if (
            // #i125189# Done when the new DrawingLayer FillAttributes are used and
            // not drawing::FillStyle_NONE (see above)
            bNewDrawingLayerFillStyleIsUsedAndNotNoFill ||

            // done when SvxBrushItem is used
            rBack.GetColor().GetAlpha() == 255 || rBack.GetGraphicPos() != GPOS_NONE ||

            // done when direct color is forced
            rxCol ||

            // done when consider BG transparency and color is not completely transparent
            (bConsiderBackgroundTransparency && (rBack.GetColor() != COL_TRANSPARENT))
           )
        {
            rpBrush = &rBack;
            if ( pFrame->IsPageFrame() && pSh->GetViewOptions()->getBrowseMode() )
            {
                rOrigRect = pFrame->getFrameArea();
                ::SwAlignRect(rOrigRect, pSh, pSh->GetOut());
            }
            else
            {
                if (pFrame->IsPageFrame()
                    && pFrame->GetAttrSet()->GetItem<SfxBoolItem>(RES_BACKGROUND_FULL_SIZE)->GetValue())
                {
                    rOrigRect = pFrame->getFrameArea();
                }
                else if (pFrame->getFrameArea().SSize() != pFrame->getFramePrintArea().SSize())
                {
                    SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFrame );
                    const SwBorderAttrs &rAttrs = *aAccess.Get();
                    ::lcl_CalcBorderRect( rOrigRect, pFrame, rAttrs, false, gProp );
                }
                else
                {
                    rOrigRect = pFrame->getFramePrintArea();
                    rOrigRect += pFrame->getFrameArea().Pos();
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
            pFrame = static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame();
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

Graphic SwFrameFormat::MakeGraphic( ImageMap*, const sal_uInt32 /*nMaximumQuadraticPixels*/, const std::optional<Size>& /*rTargetDPI*/ )
{
    return Graphic();
}

Graphic SwFlyFrameFormat::MakeGraphic( ImageMap* pMap, const sal_uInt32 /*nMaximumQuadraticPixels*/, const std::optional<Size>& /*rTargetDPI*/ )
{
    Graphic aRet;
    //search any Fly!
    SwIterator<SwFrame,SwFormat> aIter( *this );
    SwFrame *pFirst = aIter.First();
    SwViewShell *const pSh =
        pFirst ? pFirst->getRootFrame()->GetCurrShell() : nullptr;
    if (nullptr != pSh)
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
        aMet.SetPrefSize( pFly->getFrameArea().SSize() );

        aMet.Record( pDev.get() );
        pDev->SetLineColor();
        pDev->SetFillColor();
        pDev->SetFont( pOld->GetFont() );

        //Enlarge the rectangle if needed, so the border is painted too.
        SwRect aOut( pFly->getFrameArea() );
        SwBorderAttrAccess aAccess( SwFrame::GetCache(), pFly );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        if ( rAttrs.CalcRightLine() )
            aOut.AddWidth(2*gProp.nSPixelSzW );
        if ( rAttrs.CalcBottomLine() )
            aOut.AddHeight(2*gProp.nSPixelSzH );

        // #i92711# start Pre/PostPaint encapsulation before pOut is changed to the buffering VDev
        const vcl::Region aRepaintRegion(aOut.SVRect());
        pSh->DLPrePaint2(aRepaintRegion);

        vcl::Window *pWin = pSh->GetWin();
        sal_uInt16 nZoom = pSh->GetViewOptions()->GetZoom();
        ::SetOutDevAndWin( pSh, pDev, nullptr, 100 );
        gProp.bSFlyMetafile = true;
        gProp.pSFlyMetafileOut = pWin->GetOutDev();

        SwViewShellImp *pImp = pSh->Imp();
        gProp.pSFlyOnlyDraw = pFly;
        gProp.pSLines.reset(new SwLineRects);

        // determine page, fly frame is on
        const SwPageFrame* pFlyPage = pFly->FindPageFrame();
        const Color aPageBackgrdColor(pFlyPage->GetDrawBackgroundColor());
        const IDocumentDrawModelAccess& rIDDMA = pSh->getIDocumentDrawModelAccess();
        // --> OD #i76669#
        SwViewObjectContactRedirector aSwRedirector( *pSh );
        // <--
        pImp->PaintLayer( rIDDMA.GetHellId(), nullptr,
                          *pFlyPage, aOut, &aPageBackgrdColor,
                          pFlyPage->IsRightToLeft(),
                          &aSwRedirector );
        gProp.pSLines->PaintLines( pDev, gProp );
        if ( pFly->IsFlyInContentFrame() )
            pFly->PaintSwFrame( *pDev, aOut );
        gProp.pSLines->PaintLines( pDev, gProp );
        pImp->PaintLayer( rIDDMA.GetHeavenId(), nullptr,
                          *pFlyPage, aOut, &aPageBackgrdColor,
                          pFlyPage->IsRightToLeft(),
                          &aSwRedirector );
        gProp.pSLines->PaintLines( pDev, gProp );
        gProp.pSLines.reset();
        gProp.pSFlyOnlyDraw = nullptr;

        gProp.pSFlyMetafileOut = nullptr;
        gProp.bSFlyMetafile = false;
        ::SetOutDevAndWin( pSh, pOld, pWin, nZoom );

        // #i92711# end Pre/PostPaint encapsulation when pOut is back and content is painted
        pSh->DLPostPaint2(true);

        aMet.Stop();
        aMet.Move( -pFly->getFrameArea().Left(), -pFly->getFrameArea().Top() );
        aRet = Graphic( aMet );

        if( bNoteURL )
        {
            OSL_ENSURE( pNoteURL, "MakeGraphic: Good Bye, NoteURL." );
            delete pNoteURL;
            pNoteURL = nullptr;
        }
        gProp.pSGlobalShell = pOldGlobal;
    }
    return aRet;
}

Graphic SwDrawFrameFormat::MakeGraphic( ImageMap*, const sal_uInt32 nMaximumQuadraticPixels, const std::optional<Size>& rTargetDPI )
{
    Graphic aRet;
    SwDrawModel* pMod = getIDocumentDrawModelAccess().GetDrawModel();
    if ( pMod )
    {
        SdrObject *pObj = FindSdrObject();
        SdrView aView( *pMod );
        SdrPageView *pPgView = aView.ShowSdrPage(aView.GetModel()->GetPage(0));
        aView.MarkObj( pObj, pPgView );
        aRet = aView.GetMarkedObjBitmapEx(/*bNoVDevIfOneBmpMarked=*/false, nMaximumQuadraticPixels, rTargetDPI);
        aView.HideSdrPage();
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
