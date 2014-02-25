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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_TXTFLY_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_TXTFLY_HXX

#include "swtypes.hxx"
#include "swrect.hxx"

class OutputDevice;
class SwCntntFrm;
class SwPageFrm;
class SwTxtFly;
class SdrObject;
class SwTxtPaintInfo;
class SwFmt;
class TextRanger;
class SwAnchoredObject;

#include <fmtsrndenum.hxx>

#include <vector>
typedef std::vector< SwAnchoredObject* > SwAnchoredObjList;

enum PAGESIDE { LEFT_SIDE, RIGHT_SIDE, DONTKNOW_SIDE };

class SwDrawTextInfo;
class SwContourCache;
/** Contour-cache global variable, initialized/destroyed in txtinit.cxx
    and needed in txtfly.cxx by text wrapping.
  */
extern SwContourCache *pContourCache;
class SwTxtFrm;

#define POLY_CNT 20
#define POLY_MIN 5
#define POLY_MAX 4000

class SwContourCache
{
    friend void ClrContourCache();
    const SdrObject *pSdrObj[ POLY_CNT ];
    TextRanger *pTextRanger[ POLY_CNT ];
    long nPntCnt;
    MSHORT nObjCnt;
    const SwRect ContourRect( const SwFmt* pFmt, const SdrObject* pObj,
        const SwTxtFrm* pFrm, const SwRect &rLine, const long nXPos,
        const bool bRight );

public:
    SwContourCache();
    ~SwContourCache();
    const SdrObject* GetObject( MSHORT nPos ) const{ return pSdrObj[ nPos ]; }
    MSHORT GetCount() const { return nObjCnt; }
    void ClrObject( MSHORT nPos );

    /**
      Computes the rectangle that will cover the object in the given line.

      For _non_ contour-flow objects, this is simply the overlap area of
      BoundRect (including spacing), and the line, for contour-flow,
      the PolyPolygon of the object gets traversed
     */
    static const SwRect CalcBoundRect( const SwAnchoredObject* pAnchoredObj,
                                       const SwRect &rLine,
                                       const SwTxtFrm* pFrm,
                                       const long nXPos,
                                       const bool bRight );
};

/**
   The purpose of this class is to be the universal interface between
   formatting/text output and the possibly overlapping free-flying frames.
   During formatting the formatter gets the information from SwTxtFly, whether
   a certain area is present by the attributes of an overlapping frame.
   Such areas are represented by dummy portions.

   The whole text output and touch-up is, again, forwarded to a SwTxtFly.
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
class SwTxtFly
{
    const SwPageFrm             * pPage;
    const SwAnchoredObject      * mpCurrAnchoredObj;
    const SwTxtFrm              * pCurrFrm;
    const SwCntntFrm            * pMaster;
    SwAnchoredObjList           * mpAnchoredObjList;

    long nMinBottom;
    long nNextTop;  /// Stores the upper edge of the "next" frame
    sal_uLong nIndex;

    bool bOn : 1;
    bool bTopRule: 1;
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
    SwRect _GetFrm( const SwRect &rPortion, bool bTop ) const;

    SwAnchoredObjList* InitAnchoredObjList();

    SwAnchoredObjList* GetAnchoredObjList() const;

    /**
        Look for the first object which overlaps with the rectangle.
        Iterates over the anchored object list mpAnchoredObjList.
    */
    bool ForEach( const SwRect &rRect, SwRect* pRect, bool bAvoid ) const;

    /**
      \li There is less than 2cm space on both sides for the text:
      no surround (SURROUND_NONE)

      \li There is more than 2cm space on only one side:
      surround on that side (SURROUND_LEFT or SURROUND_RIGHT)

      \li There is more than 2cm space on both sides, the object is
      larger than 1.5cm: surround on the wider side
      (SURROUND_LET or SURROUND_RIGHT)

      \li There is more than 2cm space on both sides and the object
      width is less than 1.5cm: both sides surround (SURROUND_PARALLEL)
     */
    SwSurround _GetSurroundForTextWrap( const SwAnchoredObject* pAnchoredObj ) const;

    /**
       The right margin is the right margin or it is determined by the
       next object standing on the line.
     */
    void CalcRightMargin( SwRect &rFly,
                          SwAnchoredObjList::size_type nPos,
                          const SwRect &rLine ) const;

    /**
       The left margin is the left margin of the current PrintArea or
       it is determined by the last FlyFrm, which stands on the line.
     */
    void CalcLeftMargin( SwRect &rFly,
                         SwAnchoredObjList::size_type nPos,
                         const SwRect &rLine ) const;

    /**
       \return the position in sorted array
     */
    SwAnchoredObjList::size_type GetPos( const SwAnchoredObject* pAnchoredObj ) const;

    bool GetTop( const SwAnchoredObject* _pAnchoredObj,
                 const bool bInFtn,
                 const bool bInFooterOrHeader );

    SwTwips CalcMinBottom() const;

    const SwCntntFrm* _GetMaster();

public:

    SwTxtFly();
    SwTxtFly( const SwTxtFrm *pFrm );
    SwTxtFly( const SwTxtFly& rTxtFly );
    ~SwTxtFly();

    void CtorInitTxtFly( const SwTxtFrm *pFrm );

    void SetTopRule();

    SwRect GetFrm( const SwRect &rPortion, bool bTop = true ) const;
    bool IsOn() const;

    /**
        If there is no flying object frame standing in rRect (usually the current row),
        then we are turning ourself off.

        \param rRect is global to the document!
     */
    bool Relax( const SwRect &rRect );
    bool Relax();

    SwTwips GetMinBottom() const;
    const SwCntntFrm* GetMaster() const;

    // This temporary variable needs to be manipulated in const methods
    long GetNextTop() const;
    void SetNextTop( long nNew ) const;

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
        in the area of FlyFrms that are opaque and above the current frame will
        be output.

        DrawText() takes over the on optimization!
     */
    bool DrawTextOpaque( SwDrawTextInfo &rInf );

    /**
        Two subtleties needs to be mentioned:
            \li DrawRect() is allowed over the ClipRects
            \li FlyToRect() returns bigger values than the frame data

        Ensure that the overlapping frames (except the transparent frames)
        won't be scribbled
     */
    void DrawFlyRect( OutputDevice* pOut, const SwRect &rRect,
                      const SwTxtPaintInfo &rInf, bool bNoGraphic = false );

    /**
        Used to switch off the SwTxtFly when there is no overlapping object (Relax).

        \param[in] the line area
        \return whether the line will be overlapped by a frame
     */
    bool IsAnyFrm( const SwRect &rLine ) const;

    /**
        Same as IsAnyFrm(const SwRect&), but uses the current frame print
        area
     */
    bool IsAnyFrm() const;

    /**
        true when a frame or DrawObj must to be taken in account. The optimizations
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

inline SwAnchoredObjList* SwTxtFly::GetAnchoredObjList() const
{
    return mpAnchoredObjList
           ? mpAnchoredObjList
           : const_cast<SwTxtFly*>(this)->InitAnchoredObjList();
}

inline void SwTxtFly::SetTopRule()
{
    bTopRule = false;
}

inline bool SwTxtFly::IsOn() const
{
    return bOn;
}

inline bool SwTxtFly::Relax( const SwRect &rRect )
{
    if (bOn)
    {
        bOn = IsAnyFrm( rRect );
    }
    return bOn;
}

inline bool SwTxtFly::Relax()
{
    if (bOn)
    {
        bOn = IsAnyFrm();
    }
    return bOn;
}

inline SwTwips SwTxtFly::GetMinBottom() const
{
    return mpAnchoredObjList ? nMinBottom : CalcMinBottom();
}

inline const SwCntntFrm* SwTxtFly::GetMaster() const
{
    return pMaster ? pMaster : ((SwTxtFly*)this)->_GetMaster();
}

inline long SwTxtFly::GetNextTop() const
{
    return nNextTop;
}

inline void SwTxtFly::SetNextTop( long nNew ) const
{
    ((SwTxtFly*)this)->nNextTop = nNew;
}

inline SwRect SwTxtFly::GetFrm( const SwRect &rRect, bool bTop ) const
{
    return bOn ? _GetFrm( rRect, bTop ) : SwRect();
}

inline void SwTxtFly::SetIgnoreCurrentFrame( bool bNew )
{
    mbIgnoreCurrentFrame = bNew;
}

inline void SwTxtFly::SetIgnoreContour( bool bNew )
{
    mbIgnoreContour = bNew;
}

inline void SwTxtFly::SetIgnoreObjsInHeaderFooter( const bool bNew )
{
    mbIgnoreObjsInHeaderFooter = bNew;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
