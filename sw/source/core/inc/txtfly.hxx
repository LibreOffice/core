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

#pragma once

#include <editeng/txtrange.hxx>
#include <tools/solar.h>
#include <swtypes.hxx>
#include <swrect.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <nodeoffset.hxx>
#include <memory>
#include <vector>

class OutputDevice;
class SwPageFrame;
class SdrObject;
class SwFormat;
class SwAnchoredObject;
class SwTextFrame;
class SwDrawTextInfo;
class SwContourCache;

typedef std::vector< SwAnchoredObject* > SwAnchoredObjList;

/** Contour-cache global variable, initialized/destroyed in txtinit.cxx
    and needed in txtfly.cxx by text wrapping.
  */
extern SwContourCache *pContourCache;

#define POLY_CNT 20
#define POLY_MIN 5
#define POLY_MAX 4000

void ClrContourCache( const SdrObject *pObj );

class SwContourCache
{
    friend void ClrContourCache();
    struct CacheItem
    {
        const SdrObject *mpSdrObj;
        std::unique_ptr<TextRanger> mxTextRanger;
    };
    std::vector<CacheItem> mvItems;
    tools::Long mnPointCount;
    SwRect ContourRect( const SwFormat* pFormat, const SdrObject* pObj,
        const SwTextFrame* pFrame, const SwRect &rLine, const tools::Long nXPos,
        const bool bRight );

public:
    SwContourCache();
    ~SwContourCache();
    const SdrObject* GetObject( sal_uInt16 nPos ) const{ return mvItems[ nPos ].mpSdrObj; }
    sal_uInt16 GetCount() const { return mvItems.size(); }
    void ClrObject( sal_uInt16 nPos );

    /**
      Computes the rectangle that will cover the object in the given line.

      For _non_ contour-flow objects, this is simply the overlap area of
      BoundRect (including spacing), and the line, for contour-flow,
      the tools::PolyPolygon of the object gets traversed
     */
    static SwRect CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                       const SwRect &rLine,
                                       const SwTextFrame* pFrame,
                                       const tools::Long nXPos,
                                       const bool bRight );
};

/**
   The purpose of this class is to be the universal interface between
   formatting/text output and the possibly overlapping free-flying frames.
   During formatting the formatter gets the information from SwTextFly, whether
   a certain area is present by the attributes of an overlapping frame.
   Such areas are represented by dummy portions.

   The whole text output and touch-up is, again, forwarded to a SwTextFly.
   This one decides, whether parts of the text need to be clipped and splits
   the areas for e.g. a DrawRect.

   Please note that all free-flying frames are located in a PtrArray, sorted
   by TopLeft.

   Internally we always use document-global values. The IN and OUT parameters
   are, however, adjusted to the needs of the LineIter most of the time. That
   is: they are converted to frame- and window-local coordinates.
   If multiple frames with wrap attributes are located on the same line, we get
   the following settings for the text flow:

        L/R    P     L     R     N
         P   -P-P- -P-L  -P R- -P N
         L   -L P- -L L  -L R- -L N
         R    R-P-  R-L   R R-  R N
         N    N P-  N L   N R-  N N

   (P=parallel, L=left, R=right, N=no wrap)

   We can describe the behaviour as follows:
   Every frame can push away text, with the restriction that it only has influence
   until the next frame.
 */
class SwTextFly
{
    const SwPageFrame                * m_pPage;
    const SwAnchoredObject           * mpCurrAnchoredObj;
    const SwTextFrame                * m_pCurrFrame;
    const SwTextFrame                * m_pMaster;
    std::unique_ptr<SwAnchoredObjList> mpAnchoredObjList;

    tools::Long m_nMinBottom;
    tools::Long m_nNextTop;  /// Stores the upper edge of the "next" frame
    SwNodeOffset m_nCurrFrameNodeIndex;

    bool m_bOn : 1;
    bool m_bTopRule: 1;
    bool mbIgnoreCurrentFrame: 1;
    bool mbIgnoreContour: 1;

    /** boolean, indicating if objects in page header|footer are considered for
        text frames not in page header|footer.
     */
    bool mbIgnoreObjsInHeaderFooter: 1;

    /**
        This method will be called during the LineIter formatting
            \li to compute the position of the next \c FlyPortion
            \li remember new overlappings after a change of the line height.

        \param[in] rPortion
            Scope: document global.
     */
    SwRect GetFrame_( const SwRect &rPortion ) const;

    SwAnchoredObjList* InitAnchoredObjList();

    SwAnchoredObjList* GetAnchoredObjList() const;

    /**
        Look for the first object which overlaps with the rectangle.
        Iterates over the anchored object list mpAnchoredObjList.
    */
    bool ForEach( const SwRect &rRect, SwRect* pRect, bool bAvoid ) const;

    /**
      \li There is less than 2cm space on both sides for the text:
      no surround (css::text::WrapTextMode_NONE)

      \li There is more than 2cm space on only one side:
      surround on that side (css::text::WrapTextMode_LEFT or css::text::WrapTextMode_RIGHT)

      \li There is more than 2cm space on both sides, the object is
      larger than 1.5cm: surround on the wider side
      (css::text::WrapTextMode_LEFT or css::text::WrapTextMode_RIGHT)

      \li There is more than 2cm space on both sides and the object
      width is less than 1.5cm: both sides surround (css::text::WrapTextMode_PARALLEL)
     */
    css::text::WrapTextMode GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const;

    /**
       The right margin is the right margin or it is determined by the
       next object standing on the line.
     */
    void CalcRightMargin( SwRect &rFly,
                          SwAnchoredObjList::size_type nPos,
                          const SwRect &rLine ) const;

    /**
       The left margin is the left margin of the current PrintArea or
       it is determined by the last FlyFrame, which stands on the line.
     */
    void CalcLeftMargin( SwRect &rFly,
                         SwAnchoredObjList::size_type nPos,
                         const SwRect &rLine ) const;

    /**
       \return the position in sorted array
     */
    SwAnchoredObjList::size_type GetPos( const SwAnchoredObject* pAnchoredObj ) const;

    bool GetTop( const SwAnchoredObject* _pAnchoredObj,
                 const bool bInFootnote,
                 const bool bInFooterOrHeader );

    SwTwips CalcMinBottom() const;

    const SwTextFrame* GetMaster_();

public:

    SwTextFly();
    SwTextFly( const SwTextFrame *pFrame );
    SwTextFly( const SwTextFly& rTextFly );
    ~SwTextFly();

    void CtorInitTextFly( const SwTextFrame *pFrame );

    void SetTopRule();

    SwRect GetFrame( const SwRect &rPortion ) const;
    bool IsOn() const;

    /**
        If there is no flying object frame standing in rRect (usually the current row),
        then we are turning ourself off.

        \param rRect is global to the document!
     */
    bool Relax( const SwRect &rRect );
    bool Relax();

    SwTwips GetMinBottom() const;
    const SwTextFrame* GetMaster() const;

    // This temporary variable needs to be manipulated in const methods
    tools::Long GetNextTop() const;
    void SetNextTop( tools::Long nNew ) const;

    /**
      Determines the demanded rectangle for an anchored object,
      considering its surround for text wrapping.

      \param pAnchoredObj the object for which to get the bounds
      \param rLine the bounds of the line to format

      \return the flying object bounds
      */
    SwRect AnchoredObjToRect( const SwAnchoredObject* pAnchoredObj,
                              const SwRect& rRect ) const;

    /**
        This method is called by DrawText().

        Ensures that the overlapping frames (except the transparent frames) won't
        be scribbled by setting clip regions so that only the portions that are not
        in the area of FlyFrames that are opaque and above the current frame will
        be output.

        DrawText() takes over the on optimization!
     */
    void DrawTextOpaque( SwDrawTextInfo &rInf );

    /**
        Two subtleties needs to be mentioned:
            \li DrawRect() is allowed over the ClipRects
            \li FlyToRect() returns bigger values than the frame data

        Ensure that the overlapping frames (except the transparent frames)
        won't be scribbled
     */
    void DrawFlyRect( OutputDevice* pOut, const SwRect &rRect );

    /**
        Used to switch off the SwTextFly when there is no overlapping object (Relax).

        \param[in] the line area
        \return whether the line will be overlapped by a frame
     */
    bool IsAnyFrame( const SwRect &rLine ) const;

    /**
        Same as IsAnyFrame(const SwRect&), but uses the current frame print
        area
     */
    bool IsAnyFrame() const;

    /**
        true when a frame or DrawObj must be taken in account. The optimizations
        like Paint/FormatEmpty for empty sentences or the virtual OutputDevice can
        be used only when false is returned.

        \param rRect
            The rectangle can be empty, the current frame is then used. The value is
            global to the document.
      */
    bool IsAnyObj( const SwRect& rRect ) const;

    void SetIgnoreCurrentFrame( bool bNew );
    void SetIgnoreContour( bool bNew );

    void SetIgnoreObjsInHeaderFooter( const bool bNew );
};

inline SwAnchoredObjList* SwTextFly::GetAnchoredObjList() const
{
    return mpAnchoredObjList
           ? mpAnchoredObjList.get()
           : const_cast<SwTextFly*>(this)->InitAnchoredObjList();
}

inline void SwTextFly::SetTopRule()
{
    m_bTopRule = false;
}

inline bool SwTextFly::IsOn() const
{
    return m_bOn;
}

inline bool SwTextFly::Relax( const SwRect &rRect )
{
    if (m_bOn)
    {
        m_bOn = IsAnyFrame( rRect );
    }
    return m_bOn;
}

inline bool SwTextFly::Relax()
{
    if (m_bOn)
    {
        m_bOn = IsAnyFrame();
    }
    return m_bOn;
}

inline SwTwips SwTextFly::GetMinBottom() const
{
    return mpAnchoredObjList ? m_nMinBottom : CalcMinBottom();
}

inline const SwTextFrame* SwTextFly::GetMaster() const
{
    return m_pMaster ? m_pMaster : const_cast<SwTextFly*>(this)->GetMaster_();
}

inline tools::Long SwTextFly::GetNextTop() const
{
    return m_nNextTop;
}

inline void SwTextFly::SetNextTop( tools::Long nNew ) const
{
    const_cast<SwTextFly*>(this)->m_nNextTop = nNew;
}

inline SwRect SwTextFly::GetFrame( const SwRect &rRect ) const
{
    return m_bOn ? GetFrame_( rRect ) : SwRect();
}

inline void SwTextFly::SetIgnoreCurrentFrame( bool bNew )
{
    mbIgnoreCurrentFrame = bNew;
}

inline void SwTextFly::SetIgnoreContour( bool bNew )
{
    mbIgnoreContour = bNew;
}

inline void SwTextFly::SetIgnoreObjsInHeaderFooter( const bool bNew )
{
    mbIgnoreObjsInHeaderFooter = bNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
