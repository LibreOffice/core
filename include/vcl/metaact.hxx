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

#ifndef INCLUDED_VCL_METAACT_HXX
#define INCLUDED_VCL_METAACT_HXX

#include <sal/config.h>

#include <config_options.h>

#include <o3tl/span.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <tools/poly.hxx>

#include <vcl/dllapi.h>
#include <vcl/rendercontext/State.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/region.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/wall.hxx>

#include <memory>

class OutputDevice;
class SvStream;
enum class DrawTextFlags;

enum class PolyFillMode
{
    EVEN_ODD_RULE_ALTERNATE = 0x01,
    NON_ZERO_RULE_WINDING = 0x02,
};

struct ImplMetaReadData
{
    rtl_TextEncoding meActualCharSet;
    int mnParseDepth;

    ImplMetaReadData()
        : meActualCharSet(RTL_TEXTENCODING_ASCII_US)
        , mnParseDepth(0)
    {}
};

struct ImplMetaWriteData
{
    rtl_TextEncoding meActualCharSet;

    ImplMetaWriteData() :
        meActualCharSet( RTL_TEXTENCODING_ASCII_US )
    {}
};

class VCL_DLLPUBLIC MetaAction : public salhelper::SimpleReferenceObject
{
private:
    MetaActionType       mnType;

protected:
    virtual             ~MetaAction() override;

public:
                        MetaAction();
    explicit            MetaAction( MetaActionType nType );
                        MetaAction( MetaAction const & );

    virtual void        Execute( OutputDevice* pOut );

    oslInterlockedCount GetRefCount() const { return m_nCount; }

    virtual rtl::Reference<MetaAction> Clone() const;

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    MetaActionType      GetType() const { return mnType; }
    /** \#i10613# Extracted from Printer::GetPreparedMetaFile. Returns true
        if given action requires special transparency handling
    */
    virtual bool        IsTransparent() const { return false; }

protected:
    static void         ReadFillMode(SvStream &rIStm, PolyFillMode &eFillMode);
    static void         WriteFillMode(SvStream &rIStm, PolyFillMode eFillMode);
    static void         ReadColor(SvStream& rIStm, ::Color& rColor);
    static void         WriteColor(SvStream& rIStm, ::Color aColor);
};

class VCL_DLLPUBLIC MetaPixelAction final : public MetaAction
{
private:
    Point               maPt;
    Color               maColor;

public:
                        MetaPixelAction();
    MetaPixelAction(MetaPixelAction const &) = default;
    MetaPixelAction(MetaPixelAction &&) = default;
    MetaPixelAction & operator =(MetaPixelAction const &) = delete; // due to MetaAction
    MetaPixelAction & operator =(MetaPixelAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPixelAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaPixelAction( const Point& rPt, const Color& rColor );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetPoint() const { return maPt; }
    const Color&        GetColor() const { return maColor; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
    void                SetColor(Color rColor) { maColor = rColor; }
};

class VCL_DLLPUBLIC MetaPointAction final : public MetaAction
{
private:
    Point               maPt;

public:
                        MetaPointAction();
    MetaPointAction(MetaPointAction const &) = default;
    MetaPointAction(MetaPointAction &&) = default;
    MetaPointAction & operator =(MetaPointAction const &) = delete; // due to MetaAction
    MetaPointAction & operator =(MetaPointAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPointAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaPointAction( const Point& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetPoint() const { return maPt; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
};

class VCL_DLLPUBLIC MetaLineAction final : public MetaAction
{
private:

    LineInfo            maLineInfo;
    Point               maStartPt;
    Point               maEndPt;

public:
                        MetaLineAction();
    MetaLineAction(MetaLineAction const &) = default;
    MetaLineAction(MetaLineAction &&) = default;
    MetaLineAction & operator =(MetaLineAction const &) = delete; // due to MetaAction
    MetaLineAction & operator =(MetaLineAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaLineAction( const Point& rStart, const Point& rEnd );
                        MetaLineAction( const Point& rStart, const Point& rEnd,
                                        const LineInfo& rLineInfo );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
    void                SetStartPoint(const Point& rPoint) { maStartPt = rPoint; }
    void                SetEndPoint(const Point& rPoint) { maEndPt = rPoint; }
    void                SetLineInfo(const LineInfo& rLineInfo) { maLineInfo = rLineInfo; }
};

class VCL_DLLPUBLIC MetaRectAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaRectAction();
    MetaRectAction(MetaRectAction const &) = default;
    MetaRectAction(MetaRectAction &&) = default;
    MetaRectAction & operator =(MetaRectAction const &) = delete; // due to MetaAction
    MetaRectAction & operator =(MetaRectAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaRectAction( const tools::Rectangle& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
};

class VCL_DLLPUBLIC MetaRoundRectAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    sal_uInt32          mnHorzRound;
    sal_uInt32          mnVertRound;

public:
                        MetaRoundRectAction();
    MetaRoundRectAction(MetaRoundRectAction const &) = default;
    MetaRoundRectAction(MetaRoundRectAction &&) = default;
    MetaRoundRectAction & operator =(MetaRoundRectAction const &) = delete; // due to MetaAction
    MetaRoundRectAction & operator =(MetaRoundRectAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaRoundRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaRoundRectAction( const tools::Rectangle& rRect,
                                             sal_uInt32 nHorzRound, sal_uInt32 nVertRound );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    sal_uInt32          GetHorzRound() const { return mnHorzRound; }
    sal_uInt32          GetVertRound() const { return mnVertRound; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
    void                SetHorzRound(sal_uInt32 rHorzRound) { mnHorzRound = rHorzRound; }
    void                SetVertRound(sal_uInt32 rVertRound) { mnVertRound = rVertRound; }
};

class VCL_DLLPUBLIC MetaEllipseAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaEllipseAction();
    MetaEllipseAction(MetaEllipseAction const &) = default;
    MetaEllipseAction(MetaEllipseAction &&) = default;
    MetaEllipseAction & operator =(MetaEllipseAction const &) = delete; // due to MetaAction
    MetaEllipseAction & operator =(MetaEllipseAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaEllipseAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaEllipseAction( const tools::Rectangle& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
};

class VCL_DLLPUBLIC MetaArcAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

public:
                        MetaArcAction();
    MetaArcAction(MetaArcAction const &) = default;
    MetaArcAction(MetaArcAction &&) = default;
    MetaArcAction & operator =(MetaArcAction const &) = delete; // due to MetaAction
    MetaArcAction & operator =(MetaArcAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaArcAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaArcAction( const tools::Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
    void                SetStartPoint(const Point& rPoint) { maStartPt = rPoint; }
    void                SetEndPoint(const Point& rPoint) { maEndPt = rPoint; }
};

class VCL_DLLPUBLIC MetaPieAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

public:
                        MetaPieAction();
    MetaPieAction(MetaPieAction const &) = default;
    MetaPieAction(MetaPieAction &&) = default;
    MetaPieAction & operator =(MetaPieAction const &) = delete; // due to MetaAction
    MetaPieAction & operator =(MetaPieAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPieAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaPieAction( const tools::Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
    void                SetStartPoint(const Point& rPoint) { maStartPt = rPoint; }
    void                SetEndPoint(const Point& rPoint) { maEndPt = rPoint; }
};

class VCL_DLLPUBLIC MetaChordAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

public:
                        MetaChordAction();
    MetaChordAction(MetaChordAction const &) = default;
    MetaChordAction(MetaChordAction &&) = default;
    MetaChordAction & operator =(MetaChordAction const &) = delete; // due to MetaAction
    MetaChordAction & operator =(MetaChordAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaChordAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaChordAction( const tools::Rectangle& rRect,
                                         const Point& rStart, const Point& rEnd );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
    void                SetStartPoint(const Point& rPoint) { maStartPt = rPoint; }
    void                SetEndPoint(const Point& rPoint) { maEndPt = rPoint; }
};

class VCL_DLLPUBLIC MetaPolyLineAction final : public MetaAction
{
private:

    LineInfo            maLineInfo;
    tools::Polygon      maPoly;

public:
                        MetaPolyLineAction();
    MetaPolyLineAction(MetaPolyLineAction const &) = default;
    MetaPolyLineAction(MetaPolyLineAction &&) = default;
    MetaPolyLineAction & operator =(MetaPolyLineAction const &) = delete; // due to MetaAction
    MetaPolyLineAction & operator =(MetaPolyLineAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPolyLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaPolyLineAction( const tools::Polygon& );
    explicit            MetaPolyLineAction( const tools::Polygon&, const LineInfo& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Polygon& GetPolygon() const { return maPoly; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
    void                SetLineInfo(const LineInfo& rLineInfo) { maLineInfo = rLineInfo; }
    void                SetPolygon(const tools::Polygon& rPoly) { maPoly = rPoly; }
};

class VCL_DLLPUBLIC MetaPolygonAction final : public MetaAction
{
private:

    tools::Polygon      maPoly;

public:
                        MetaPolygonAction();
    MetaPolygonAction(MetaPolygonAction const &) = default;
    MetaPolygonAction(MetaPolygonAction &&) = default;
    MetaPolygonAction & operator =(MetaPolygonAction const &) = delete; // due to MetaAction
    MetaPolygonAction & operator =(MetaPolygonAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPolygonAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaPolygonAction( const tools::Polygon& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Polygon& GetPolygon() const { return maPoly; }
    void                SetPolygon(const tools::Polygon& rPoly) { maPoly = rPoly; }
};

class VCL_DLLPUBLIC MetaPolyPolygonAction final : public MetaAction
{
private:

    tools::PolyPolygon  maPolyPoly;

public:
                        MetaPolyPolygonAction();
    MetaPolyPolygonAction(MetaPolyPolygonAction const &) = default;
    MetaPolyPolygonAction(MetaPolyPolygonAction &&) = default;
    MetaPolyPolygonAction & operator =(MetaPolyPolygonAction const &) = delete; // due to MetaAction
    MetaPolyPolygonAction & operator =(MetaPolyPolygonAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPolyPolygonAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaPolyPolygonAction( const tools::PolyPolygon& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    void                SetPolyPolygon(const tools::PolyPolygon& rPolyPoly) { maPolyPoly = rPolyPoly; }
};

class SAL_DLLPUBLIC_RTTI MetaTextAction final : public MetaAction
{
private:

    Point           maPt;
    OUString        maStr;
    sal_Int32       mnIndex;
    sal_Int32       mnLen;

public:
                        MetaTextAction();
    MetaTextAction(MetaTextAction const &) = default;
    MetaTextAction(MetaTextAction &&) = default;
    MetaTextAction & operator =(MetaTextAction const &) = delete; // due to MetaAction
    MetaTextAction & operator =(MetaTextAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    MetaTextAction( const Point& rPt, const OUString& rStr,
                    sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    const Point&    GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
    void            SetPoint(const Point& rPt) { maPt = rPt; }
    void            SetText(const OUString& rStr) { maStr = rStr; }
    void            SetIndex(sal_Int32 rIndex) { mnIndex = rIndex; }
    void            SetLen(sal_Int32 rLen) { mnLen = rLen; }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) MetaTextArrayAction final : public MetaAction
{
private:

    Point       maStartPt;
    OUString    maStr;
    std::vector<sal_Int32> maDXAry;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

    virtual             ~MetaTextArrayAction() override;

public:
                        MetaTextArrayAction();
                        MetaTextArrayAction( const MetaTextArrayAction& rAction );
    MetaTextArrayAction( const Point& rStartPt, const OUString& rStr,
                         const std::vector<sal_Int32>& rDXAry, sal_Int32 nIndex,
                         sal_Int32 nLen );
    MetaTextArrayAction( const Point& rStartPt, const OUString& rStr,
                         o3tl::span<const sal_Int32> pDXAry, sal_Int32 nIndex,
                         sal_Int32 nLen );

    virtual void        Execute( OutputDevice* pOut ) override;

    virtual rtl::Reference<MetaAction> Clone() const override;

    virtual void    Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    const Point&    GetPoint() const { return maStartPt; }
    const OUString& GetText() const { return maStr; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
    const std::vector<sal_Int32> & GetDXArray() const { return maDXAry; }
    void            SetPoint(const Point& rPt) { maStartPt = rPt; }
    void            SetText(const OUString& rStr) { maStr = rStr; }
    void            SetIndex(sal_Int32 rIndex) { mnIndex = rIndex; }
    void            SetLen(sal_Int32 rLen) { mnLen = rLen; }
    void            SetDXArray(std::vector<sal_Int32> aArray);
};

class SAL_DLLPUBLIC_RTTI MetaStretchTextAction final : public MetaAction
{
private:

    Point       maPt;
    OUString    maStr;
    sal_uInt32  mnWidth;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

public:
                        MetaStretchTextAction();
    MetaStretchTextAction(MetaStretchTextAction const &) = default;
    MetaStretchTextAction(MetaStretchTextAction &&) = default;
    MetaStretchTextAction & operator =(MetaStretchTextAction const &) = delete; // due to MetaAction
    MetaStretchTextAction & operator =(MetaStretchTextAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaStretchTextAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                           const OUString& rStr,
                           sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    const Point&    GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt32      GetWidth() const { return mnWidth; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
    void            SetPoint(const Point& rPt) { maPt = rPt; }
    void            SetText(const OUString& rStr) { maStr = rStr; }
    void            SetWidth(sal_uInt32 rWidth) { mnWidth = rWidth; }
    void            SetIndex(sal_uInt32 rIndex) { mnIndex = rIndex; }
    void            SetLen(sal_uInt32 rLen) { mnLen = rLen; }
};

class SAL_DLLPUBLIC_RTTI MetaTextRectAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    OUString            maStr;
    DrawTextFlags       mnStyle;

public:
                        MetaTextRectAction();
    MetaTextRectAction(MetaTextRectAction const &) = default;
    MetaTextRectAction(MetaTextRectAction &&) = default;
    MetaTextRectAction & operator =(MetaTextRectAction const &) = delete; // due to MetaAction
    MetaTextRectAction & operator =(MetaTextRectAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    MetaTextRectAction( const tools::Rectangle& rRect,
                        const OUString& rStr, DrawTextFlags nStyle );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const OUString&     GetText() const { return maStr; }
    DrawTextFlags       GetStyle() const { return mnStyle; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
    void                SetText(const OUString& rStr) { maStr = rStr; }
    void                SetStyle(DrawTextFlags rStyle) { mnStyle = rStyle; }
};

class SAL_DLLPUBLIC_RTTI MetaTextLineAction final : public MetaAction
{
private:

    Point               maPos;
    tools::Long                mnWidth;
    FontStrikeout       meStrikeout;
    FontLineStyle       meUnderline;
    FontLineStyle       meOverline;

public:
                        MetaTextLineAction();
    MetaTextLineAction(MetaTextLineAction const &) = default;
    MetaTextLineAction(MetaTextLineAction &&) = default;
    MetaTextLineAction & operator =(MetaTextLineAction const &) = delete; // due to MetaAction
    MetaTextLineAction & operator =(MetaTextLineAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaTextLineAction( const Point& rPos, tools::Long nWidth,
                                            FontStrikeout eStrikeout,
                                            FontLineStyle eUnderline,
                                            FontLineStyle eOverline );
    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetStartPoint() const { return maPos; }
    tools::Long                GetWidth() const { return mnWidth; }
    FontStrikeout       GetStrikeout() const { return meStrikeout; }
    FontLineStyle       GetUnderline() const { return meUnderline; }
    FontLineStyle       GetOverline()  const { return meOverline; }
    void                SetStartPoint(const Point& rPos) { maPos = rPos; }
    void                SetWidth(tools::Long rWidth) { mnWidth = rWidth; }
    void                SetStrikeout(FontStrikeout eStrikeout) { meStrikeout = eStrikeout; }
    void                SetUnderline(FontLineStyle eUnderline) { meUnderline = eUnderline; }
    void                SetOverline(FontLineStyle eOverline) { meOverline = eOverline; }
};

class VCL_DLLPUBLIC MetaBmpAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;

public:
                        MetaBmpAction();
    MetaBmpAction(MetaBmpAction const &) = default;
    MetaBmpAction(MetaBmpAction &&) = default;
    MetaBmpAction & operator =(MetaBmpAction const &) = delete; // due to MetaAction
    MetaBmpAction & operator =(MetaBmpAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpAction( const Point& rPt, const Bitmap& rBmp );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
};

class VCL_DLLPUBLIC MetaBmpScaleAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;
    Size                maSz;

public:
                        MetaBmpScaleAction();
    MetaBmpScaleAction(MetaBmpScaleAction const &) = default;
    MetaBmpScaleAction(MetaBmpScaleAction &&) = default;
    MetaBmpScaleAction & operator =(MetaBmpScaleAction const &) = delete; // due to MetaAction
    MetaBmpScaleAction & operator =(MetaBmpScaleAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                            const Bitmap& rBmp );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
    void                SetSize(const Size& rSz) { maSz = rSz; }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) MetaBmpScalePartAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

public:
                        MetaBmpScalePartAction();
    MetaBmpScalePartAction(MetaBmpScalePartAction const &) = default;
    MetaBmpScalePartAction(MetaBmpScalePartAction &&) = default;
    MetaBmpScalePartAction & operator =(MetaBmpScalePartAction const &) = delete; // due to MetaAction
    MetaBmpScalePartAction & operator =(MetaBmpScalePartAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetDestPoint(const Point& rPt) { maDstPt = rPt; }
    void                SetDestSize(const Size& rSz) { maDstSz = rSz; }
    void                SetSrcPoint(const Point& rPt) { maSrcPt = rPt; }
    void                SetSrcSize(const Size& rSz) { maSrcSz = rSz; }
};

class VCL_DLLPUBLIC MetaBmpExAction final : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maPt;

public:
                        MetaBmpExAction();
    MetaBmpExAction(MetaBmpExAction const &) = default;
    MetaBmpExAction(MetaBmpExAction &&) = default;
    MetaBmpExAction & operator =(MetaBmpExAction const &) = delete; // due to MetaAction
    MetaBmpExAction & operator =(MetaBmpExAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpExAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
    void                SetBitmapEx(BitmapEx rBmpEx) { maBmpEx = rBmpEx; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
    bool                IsTransparent() const override { return GetBitmapEx().IsAlpha(); }
};

class VCL_DLLPUBLIC MetaBmpExScaleAction final : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maPt;
    Size                maSz;

public:
                        MetaBmpExScaleAction();
    MetaBmpExScaleAction(MetaBmpExScaleAction const &) = default;
    MetaBmpExScaleAction(MetaBmpExScaleAction &&) = default;
    MetaBmpExScaleAction & operator =(MetaBmpExScaleAction const &) = delete; // due to MetaAction
    MetaBmpExScaleAction & operator =(MetaBmpExScaleAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpExScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                              const BitmapEx& rBmpEx ) ;

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
    void                SetBitmapEx(const BitmapEx& rBmpEx) { maBmpEx = rBmpEx; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
    void                SetSize(const Size& rSz) { maSz = rSz; }
    bool                IsTransparent() const override { return GetBitmapEx().IsAlpha(); }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) MetaBmpExScalePartAction final : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

public:
                        MetaBmpExScalePartAction();
    MetaBmpExScalePartAction(MetaBmpExScalePartAction const &) = default;
    MetaBmpExScalePartAction(MetaBmpExScalePartAction &&) = default;
    MetaBmpExScalePartAction & operator =(MetaBmpExScalePartAction const &) = delete; // due to MetaAction
    MetaBmpExScalePartAction & operator =(MetaBmpExScalePartAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpExScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const BitmapEx& rBmpEx );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
    void                SetBitmapEx(const BitmapEx& rBmpEx) { maBmpEx = rBmpEx; }
    void                SetDestPoint(const Point& rDstPt) { maDstPt = rDstPt; }
    void                SetDestSize(const Size& rDstSz) { maDstSz = rDstSz; }
    void                SetSrcPoint(const Point& rSrcPt) { maSrcPt = rSrcPt; }
    void                SetSrcSize(const Size& rSrcSz) { maSrcSz = rSrcSz; }
    bool                IsTransparent() const override { return GetBitmapEx().IsAlpha(); }
};

class SAL_DLLPUBLIC_RTTI MetaMaskAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maPt;

public:
                        MetaMaskAction();
    MetaMaskAction(MetaMaskAction const &) = default;
    MetaMaskAction(MetaMaskAction &&) = default;
    MetaMaskAction & operator =(MetaMaskAction const &) = delete; // due to MetaAction
    MetaMaskAction & operator =(MetaMaskAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMaskAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaMaskAction( const Point& rPt,
                                        const Bitmap& rBmp,
                                        const Color& rColor );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
};

class SAL_DLLPUBLIC_RTTI MetaMaskScaleAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maPt;
    Size                maSz;

public:
                        MetaMaskScaleAction();
    MetaMaskScaleAction(MetaMaskScaleAction const &) = default;
    MetaMaskScaleAction(MetaMaskScaleAction &&) = default;
    MetaMaskScaleAction & operator =(MetaMaskScaleAction const &) = delete; // due to MetaAction
    MetaMaskScaleAction & operator =(MetaMaskScaleAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMaskScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                             const Bitmap& rBmp,
                                             const Color& rColor );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetPoint(const Point& rPt) { maPt = rPt; }
    void                SetSize(const Size& rSz) { maSz = rSz; }
};

class SAL_DLLPUBLIC_RTTI MetaMaskScalePartAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

public:
                        MetaMaskScalePartAction();
    MetaMaskScalePartAction(MetaMaskScalePartAction const &) = default;
    MetaMaskScalePartAction(MetaMaskScalePartAction &&) = default;
    MetaMaskScalePartAction & operator =(MetaMaskScalePartAction const &) = delete; // due to MetaAction
    MetaMaskScalePartAction & operator =(MetaMaskScalePartAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMaskScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                 const Point& rSrcPt, const Size& rSrcSz,
                                                 const Bitmap& rBmp,
                                                 const Color& rColor );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
    void                SetBitmap(const Bitmap& rBmp) { maBmp = rBmp; }
    void                SetColor(Color rColor) { maColor = rColor; }
    void                SetDestPoint(const Point& rPt) { maDstPt = rPt; }
    void                SetDestSize(const Size& rSz) { maDstSz = rSz; }
    void                SetSrcPoint(const Point& rPt) { maSrcPt = rPt; }
    void                SetSrcSize(const Size& rSz) { maSrcSz = rSz; }
};

class SAL_DLLPUBLIC_RTTI MetaGradientAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    Gradient            maGradient;

public:
                        MetaGradientAction();
    MetaGradientAction(MetaGradientAction const &) = default;
    MetaGradientAction(MetaGradientAction &&) = default;
    MetaGradientAction & operator =(MetaGradientAction const &) = delete; // due to MetaAction
    MetaGradientAction & operator =(MetaGradientAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaGradientAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaGradientAction( const tools::Rectangle& rRect, const Gradient& rGradient );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Gradient&     GetGradient() const { return maGradient; }
    void                SetGradient(const Gradient& rGradient) { maGradient = rGradient; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
};

class VCL_DLLPUBLIC MetaGradientExAction final : public MetaAction
{
private:

    tools::PolyPolygon  maPolyPoly;
    Gradient            maGradient;

public:
                        MetaGradientExAction();
    MetaGradientExAction(MetaGradientExAction const &) = default;
    MetaGradientExAction(MetaGradientExAction &&) = default;
    MetaGradientExAction & operator =(MetaGradientExAction const &) = delete; // due to MetaAction
    MetaGradientExAction & operator =(MetaGradientExAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaGradientExAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaGradientExAction( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Gradient&     GetGradient() const { return maGradient; }
    void                SetPolyPolygon(const tools::PolyPolygon& rPolyPoly) { maPolyPoly = rPolyPoly; }
    void                SetGradient(const Gradient& rGradient) { maGradient = rGradient; }
};

class SAL_DLLPUBLIC_RTTI MetaHatchAction final : public MetaAction
{
private:

    tools::PolyPolygon  maPolyPoly;
    Hatch               maHatch;

public:
                        MetaHatchAction();
    MetaHatchAction(MetaHatchAction const &) = default;
    MetaHatchAction(MetaHatchAction &&) = default;
    MetaHatchAction & operator =(MetaHatchAction const &) = delete; // due to MetaAction
    MetaHatchAction & operator =(MetaHatchAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaHatchAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaHatchAction( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Hatch&        GetHatch() const { return maHatch; }
    void                SetPolyPolygon(const tools::PolyPolygon& rPolyPoly) { maPolyPoly = rPolyPoly; }
    void                SetHatch(const Hatch& rHatch) { maHatch = rHatch; }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) MetaWallpaperAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;
    Wallpaper           maWallpaper;

public:
                        MetaWallpaperAction();
    MetaWallpaperAction(MetaWallpaperAction const &) = default;
    MetaWallpaperAction(MetaWallpaperAction &&) = default;
    MetaWallpaperAction & operator =(MetaWallpaperAction const &) = delete; // due to MetaAction
    MetaWallpaperAction & operator =(MetaWallpaperAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaWallpaperAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaWallpaperAction( const tools::Rectangle& rRect,
                                             const Wallpaper& rPaper );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Wallpaper&    GetWallpaper() const { return maWallpaper; }
    void                SetWallpaper(const Wallpaper& rWallpaper) { maWallpaper = rWallpaper; }
};

class VCL_DLLPUBLIC MetaClipRegionAction final : public MetaAction
{
private:

    vcl::Region         maRegion;
    bool                mbClip;

public:
                        MetaClipRegionAction();
    MetaClipRegionAction(MetaClipRegionAction const &) = default;
    MetaClipRegionAction(MetaClipRegionAction &&) = default;
    MetaClipRegionAction & operator =(MetaClipRegionAction const &) = delete; // due to MetaAction
    MetaClipRegionAction & operator =(MetaClipRegionAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaClipRegionAction( const vcl::Region& rRegion, bool bClip );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Region&  GetRegion() const { return maRegion; }
    bool                IsClipping() const { return mbClip; }
    void                SetRegion(const vcl::Region& rRegion) { maRegion = rRegion; }
    void                SetClipping(bool bClip) { mbClip = bClip; }
};

class VCL_DLLPUBLIC MetaISectRectClipRegionAction final : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaISectRectClipRegionAction();
    MetaISectRectClipRegionAction(MetaISectRectClipRegionAction const &) = default;
    MetaISectRectClipRegionAction(MetaISectRectClipRegionAction &&) = default;
    MetaISectRectClipRegionAction & operator =(MetaISectRectClipRegionAction const &) = delete; // due to MetaAction
    MetaISectRectClipRegionAction & operator =(MetaISectRectClipRegionAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaISectRectClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaISectRectClipRegionAction( const tools::Rectangle& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    void                SetRect(const tools::Rectangle& rRect) { maRect = rRect; }
};

class VCL_DLLPUBLIC MetaISectRegionClipRegionAction final : public MetaAction
{
private:

    vcl::Region          maRegion;

public:
                        MetaISectRegionClipRegionAction();
    MetaISectRegionClipRegionAction(MetaISectRegionClipRegionAction const &) = default;
    MetaISectRegionClipRegionAction(MetaISectRegionClipRegionAction &&) = default;
    MetaISectRegionClipRegionAction & operator =(MetaISectRegionClipRegionAction const &) = delete; // due to MetaAction
    MetaISectRegionClipRegionAction & operator =(MetaISectRegionClipRegionAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaISectRegionClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaISectRegionClipRegionAction( const vcl::Region& );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Region&  GetRegion() const { return maRegion; }
    void                SetRegion(const vcl::Region& rRegion) { maRegion = rRegion; }
};

class VCL_DLLPUBLIC MetaMoveClipRegionAction final : public MetaAction
{
private:

    tools::Long                mnHorzMove;
    tools::Long                mnVertMove;

public:
                        MetaMoveClipRegionAction();
    MetaMoveClipRegionAction(MetaMoveClipRegionAction const &) = default;
    MetaMoveClipRegionAction(MetaMoveClipRegionAction &&) = default;
    MetaMoveClipRegionAction & operator =(MetaMoveClipRegionAction const &) = delete; // due to MetaAction
    MetaMoveClipRegionAction & operator =(MetaMoveClipRegionAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMoveClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaMoveClipRegionAction( tools::Long nHorzMove, tools::Long nVertMove );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    tools::Long                GetHorzMove() const { return mnHorzMove; }
    tools::Long                GetVertMove() const { return mnVertMove; }
    void                SetHorzMove(tools::Long nHorzMove) { mnHorzMove = nHorzMove; }
    void                SetVertMove(tools::Long nVertMove) { mnVertMove = nVertMove; }
};

class VCL_DLLPUBLIC MetaLineColorAction final : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

public:
                        MetaLineColorAction();
    MetaLineColorAction(MetaLineColorAction const &) = default;
    MetaLineColorAction(MetaLineColorAction &&) = default;
    MetaLineColorAction & operator =(MetaLineColorAction const &) = delete; // due to MetaAction
    MetaLineColorAction & operator =(MetaLineColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaLineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
    void                SetColor(const Color& rColor) { maColor = rColor; }
    void                SetSetting(bool rSet) { mbSet = rSet; }
};

class VCL_DLLPUBLIC MetaFillColorAction final : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

public:
                        MetaFillColorAction();
    MetaFillColorAction(MetaFillColorAction const &) = default;
    MetaFillColorAction(MetaFillColorAction &&) = default;
    MetaFillColorAction & operator =(MetaFillColorAction const &) = delete; // due to MetaAction
    MetaFillColorAction & operator =(MetaFillColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaFillColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
    void                SetSetting(bool rSet) { mbSet = rSet; }
    void                SetColor(Color rColor) { maColor = rColor; }

};

class VCL_DLLPUBLIC MetaFillModeAction final : public MetaAction
{
private:

    PolyFillMode mePolyFillMode;
    bool                mbSet;

public:
                        MetaFillModeAction();
    MetaFillModeAction(MetaFillModeAction const &) = default;
    MetaFillModeAction(MetaFillModeAction &&) = default;
    MetaFillModeAction & operator =(MetaFillModeAction const &) = delete; // due to MetaAction
    MetaFillModeAction & operator =(MetaFillModeAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaFillModeAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaFillModeAction( const PolyFillMode& rPolyFillMode,
                                            bool bSet );

    const PolyFillMode&     GetFillMode() const { return mePolyFillMode; }
    bool                IsSetting() const { return mbSet; }
    void                SetSetting(bool rSet) { mbSet = rSet; }
    void                SetFillMode(PolyFillMode& rFillRule) { mePolyFillMode = rFillRule; }

};

class VCL_DLLPUBLIC MetaTextColorAction final : public MetaAction
{
private:

    Color               maColor;

public:
                        MetaTextColorAction();
    MetaTextColorAction(MetaTextColorAction const &) = default;
    MetaTextColorAction(MetaTextColorAction &&) = default;
    MetaTextColorAction & operator =(MetaTextColorAction const &) = delete; // due to MetaAction
    MetaTextColorAction & operator =(MetaTextColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaTextColorAction( const Color& );

    const Color&        GetColor() const { return maColor; }
    void                SetColor(Color rColor) { maColor = rColor; }
};

class VCL_DLLPUBLIC MetaTextFillColorAction final : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

public:
                        MetaTextFillColorAction();
    MetaTextFillColorAction(MetaTextFillColorAction const &) = default;
    MetaTextFillColorAction(MetaTextFillColorAction &&) = default;
    MetaTextFillColorAction & operator =(MetaTextFillColorAction const &) = delete; // due to MetaAction
    MetaTextFillColorAction & operator =(MetaTextFillColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextFillColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaTextFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    void                SetColor(Color rColor) { maColor = rColor; }
    void                SetSetting(bool bSet) { mbSet = bSet; }
    bool                IsSetting() const { return mbSet; }

};

class VCL_DLLPUBLIC MetaTextLineColorAction final : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

public:
                        MetaTextLineColorAction();
    MetaTextLineColorAction(MetaTextLineColorAction const &) = default;
    MetaTextLineColorAction(MetaTextLineColorAction &&) = default;
    MetaTextLineColorAction & operator =(MetaTextLineColorAction const &) = delete; // due to MetaAction
    MetaTextLineColorAction & operator =(MetaTextLineColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextLineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaTextLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    void                SetColor(const Color& rColor) { maColor = rColor; }
    void                SetSetting(bool bSet) { mbSet = bSet; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaOverlineColorAction final : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

public:
                        MetaOverlineColorAction();
    MetaOverlineColorAction(MetaOverlineColorAction const &) = default;
    MetaOverlineColorAction(MetaOverlineColorAction &&) = default;
    MetaOverlineColorAction & operator =(MetaOverlineColorAction const &) = delete; // due to MetaAction
    MetaOverlineColorAction & operator =(MetaOverlineColorAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaOverlineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaOverlineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    void                SetColor(const Color& rColor) { maColor = rColor; }
    void                SetSetting(bool bSet) { mbSet = bSet; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextAlignAction final : public MetaAction
{
private:

    TextAlign           maAlign;

public:
                        MetaTextAlignAction();
    MetaTextAlignAction(MetaTextAlignAction const &) = default;
    MetaTextAlignAction(MetaTextAlignAction &&) = default;
    MetaTextAlignAction & operator =(MetaTextAlignAction const &) = delete; // due to MetaAction
    MetaTextAlignAction & operator =(MetaTextAlignAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextAlignAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaTextAlignAction( TextAlign eAlign );

    TextAlign           GetTextAlign() const { return maAlign; }
    void                SetTextAlign(TextAlign eAlign) { maAlign = eAlign; }
};

class VCL_DLLPUBLIC MetaMapModeAction final : public MetaAction
{
private:

    MapMode             maMapMode;

public:
                        MetaMapModeAction();
    MetaMapModeAction(MetaMapModeAction const &) = default;
    MetaMapModeAction(MetaMapModeAction &&) = default;
    MetaMapModeAction & operator =(MetaMapModeAction const &) = delete; // due to MetaAction
    MetaMapModeAction & operator =(MetaMapModeAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMapModeAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaMapModeAction( const MapMode& );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const MapMode&      GetMapMode() const { return maMapMode; }
    void                SetMapMode(const MapMode& rMapMode) { maMapMode = rMapMode; }
};

// tdf#127471 decl for friend below
namespace emfio { class ScaledFontDetectCorrectHelper; }

class VCL_DLLPUBLIC MetaFontAction final : public MetaAction
{
private:

    vcl::Font           maFont;

    // tdf#127471 for import correction of own wrong written EMF/WMF files allow correction
    // of FontScale after import. Only from there, so use a friend here and keep the method private
    friend class emfio::ScaledFontDetectCorrectHelper;
    void correctFontScale(tools::Long nNewFontScale) { maFont.SetAverageFontWidth(nNewFontScale); }

public:
                        MetaFontAction();
    MetaFontAction(MetaFontAction const &) = default;
    MetaFontAction(MetaFontAction &&) = default;
    MetaFontAction & operator =(MetaFontAction const &) = delete; // due to MetaAction
    MetaFontAction & operator =(MetaFontAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaFontAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaFontAction( const vcl::Font& );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Font&    GetFont() const { return maFont; }
    void                SetFont(const vcl::Font& rFont) { maFont = rFont; }

};

class VCL_DLLPUBLIC MetaPushAction final : public MetaAction
{
private:

    vcl::PushFlags           mnFlags;

public:
                        MetaPushAction();
    MetaPushAction(MetaPushAction const &) = default;
    MetaPushAction(MetaPushAction &&) = default;
    MetaPushAction & operator =(MetaPushAction const &) = delete; // due to MetaAction
    MetaPushAction & operator =(MetaPushAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPushAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaPushAction( vcl::PushFlags nFlags );

    vcl::PushFlags           GetFlags() const { return mnFlags; }
    void                SetPushFlags(const vcl::PushFlags nFlags) { mnFlags = nFlags; }
};

class VCL_DLLPUBLIC MetaPopAction final : public MetaAction
{
public:

                        MetaPopAction();
    MetaPopAction(MetaPopAction const &) = default;
    MetaPopAction(MetaPopAction &&) = default;
    MetaPopAction & operator =(MetaPopAction const &) = delete; // due to MetaAction
    MetaPopAction & operator =(MetaPopAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPopAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;
};

class VCL_DLLPUBLIC MetaRasterOpAction final : public MetaAction
{
private:

    RasterOp            meRasterOp;

public:
                        MetaRasterOpAction();
    MetaRasterOpAction(MetaRasterOpAction const &) = default;
    MetaRasterOpAction(MetaRasterOpAction &&) = default;
    MetaRasterOpAction & operator =(MetaRasterOpAction const &) = delete; // due to MetaAction
    MetaRasterOpAction & operator =(MetaRasterOpAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaRasterOpAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaRasterOpAction( RasterOp eRasterOp );

    RasterOp            GetRasterOp() const { return meRasterOp; }
    void                SetRasterOp(const RasterOp eRasterOp) { meRasterOp = eRasterOp; }
};

class SAL_DLLPUBLIC_RTTI MetaTransparentAction final : public MetaAction
{
private:

    tools::PolyPolygon  maPolyPoly;
    sal_uInt16          mnTransPercent;

public:
                        MetaTransparentAction();
    MetaTransparentAction(MetaTransparentAction const &) = default;
    MetaTransparentAction(MetaTransparentAction &&) = default;
    MetaTransparentAction & operator =(MetaTransparentAction const &) = delete; // due to MetaAction
    MetaTransparentAction & operator =(MetaTransparentAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTransparentAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaTransparentAction( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    sal_uInt16              GetTransparence() const { return mnTransPercent; }
    void                SetPolyPolygon(const tools::PolyPolygon& rPolyPoly) { maPolyPoly = rPolyPoly; }
    void                SetTransparence(const sal_uInt16 nTransPercent) { mnTransPercent = nTransPercent; }

    bool                IsTransparent() const override { return true; }
};

class SAL_DLLPUBLIC_RTTI MetaFloatTransparentAction final : public MetaAction
{
private:

    GDIMetaFile         maMtf;
    Point               maPoint;
    Size                maSize;
    Gradient            maGradient;

public:
                        MetaFloatTransparentAction();
    MetaFloatTransparentAction(MetaFloatTransparentAction const &) = default;
    MetaFloatTransparentAction(MetaFloatTransparentAction &&) = default;
    MetaFloatTransparentAction & operator =(MetaFloatTransparentAction const &) = delete; // due to MetaAction
    MetaFloatTransparentAction & operator =(MetaFloatTransparentAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaFloatTransparentAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                    const Size& rSize, const Gradient& rGradient );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const GDIMetaFile&  GetGDIMetaFile() const { return maMtf; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
    const Gradient&     GetGradient() const { return maGradient; }
    void                SetGDIMetaFile(const GDIMetaFile &rMtf) { maMtf  = rMtf; }
    void                SetPoint(const Point& rPoint) { maPoint = rPoint; }
    void                SetSize(const Size& rSize) { maSize = rSize; }
    void                SetGradient(const Gradient& rGradient) { maGradient = rGradient; }
    bool                IsTransparent() const override { return true; }
};

class VCL_DLLPUBLIC MetaEPSAction final : public MetaAction
{
private:

    GfxLink             maGfxLink;
    GDIMetaFile         maSubst;
    Point               maPoint;
    Size                maSize;

public:
                        MetaEPSAction();
    MetaEPSAction(MetaEPSAction const &) = default;
    MetaEPSAction(MetaEPSAction &&) = default;
    MetaEPSAction & operator =(MetaEPSAction const &) = delete; // due to MetaAction
    MetaEPSAction & operator =(MetaEPSAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaEPSAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaEPSAction( const Point& rPoint, const Size& rSize,
                                       const GfxLink& rGfxLink, const GDIMetaFile& rSubst );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const GfxLink&      GetLink() const { return maGfxLink; }
    const GDIMetaFile&  GetSubstitute() const { return maSubst; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
    void                SetLink(const GfxLink& rGfxLink) { maGfxLink = rGfxLink; }
    void                SetSubstitute(const GDIMetaFile& rSubst) { maSubst = rSubst; }
    void                SetPoint(const Point& rPoint) { maPoint = rPoint; }
    void                SetSize(const Size& rSize) { maSize = rSize; }
};

class VCL_DLLPUBLIC MetaRefPointAction final : public MetaAction
{
private:

    Point               maRefPoint;
    bool                mbSet;

public:
                        MetaRefPointAction();
    MetaRefPointAction(MetaRefPointAction const &) = default;
    MetaRefPointAction(MetaRefPointAction &&) = default;
    MetaRefPointAction & operator =(MetaRefPointAction const &) = delete; // due to MetaAction
    MetaRefPointAction & operator =(MetaRefPointAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaRefPointAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

                        MetaRefPointAction( const Point& rRefPoint, bool bSet );

    const Point&        GetRefPoint() const { return maRefPoint; }
    void                SetRefPoint(const Point& rRefPoint) { maRefPoint = rRefPoint; }
    void                SetSetting(const bool bSet) { mbSet = bSet; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaCommentAction final : public MetaAction
{
private:

    OString             maComment;
    sal_Int32           mnValue;
    sal_uInt32          mnDataSize;
    std::unique_ptr<sal_uInt8[]>
                        mpData;

    SAL_DLLPRIVATE void ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize );

private:
    virtual             ~MetaCommentAction() override;

public:
    explicit            MetaCommentAction();
    explicit            MetaCommentAction( const MetaCommentAction& rAct );
    explicit            MetaCommentAction( const OString& rComment, sal_Int32 nValue = 0, const sal_uInt8* pData = nullptr, sal_uInt32 nDataSize = 0 );

    virtual void        Move( tools::Long nHorzMove, tools::Long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    const OString&      GetComment() const { return maComment; }
    sal_Int32           GetValue() const { return mnValue; }
    sal_uInt32          GetDataSize() const { return mnDataSize; }
    const sal_uInt8*    GetData() const { return mpData.get(); }
    void                SetComment(const OString& rComment) { maComment = rComment; }
    void                SetValue(const sal_Int32 nValue) { mnValue = nValue; }
    void                SetDataSize(const sal_Int32 nDataSize) { mnDataSize = nDataSize; }
    void                SetData(const sal_uInt8* pData, const sal_uInt32 nDataSize) { ImplInitDynamicData(pData, nDataSize); }
};

class VCL_DLLPUBLIC MetaLayoutModeAction final : public MetaAction
{
private:

    vcl::text::ComplexTextLayoutFlags  mnLayoutMode;

public:
                        MetaLayoutModeAction();
    MetaLayoutModeAction(MetaLayoutModeAction const &) = default;
    MetaLayoutModeAction(MetaLayoutModeAction &&) = default;
    MetaLayoutModeAction & operator =(MetaLayoutModeAction const &) = delete; // due to MetaAction
    MetaLayoutModeAction & operator =(MetaLayoutModeAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaLayoutModeAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaLayoutModeAction( vcl::text::ComplexTextLayoutFlags nLayoutMode );

    vcl::text::ComplexTextLayoutFlags  GetLayoutMode() const { return mnLayoutMode; }
    void                SetLayoutMode(const vcl::text::ComplexTextLayoutFlags nLayoutMode) { mnLayoutMode = nLayoutMode; }
};

class VCL_DLLPUBLIC MetaTextLanguageAction final : public MetaAction
{
private:

    LanguageType        meTextLanguage;

public:
                        MetaTextLanguageAction();
    MetaTextLanguageAction(MetaTextLanguageAction const &) = default;
    MetaTextLanguageAction(MetaTextLanguageAction &&) = default;
    MetaTextLanguageAction & operator =(MetaTextLanguageAction const &) = delete; // due to MetaAction
    MetaTextLanguageAction & operator =(MetaTextLanguageAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaTextLanguageAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() const override;

    explicit            MetaTextLanguageAction( LanguageType );

    LanguageType        GetTextLanguage() const { return meTextLanguage; }
    void                SetTextLanguage(const LanguageType eTextLanguage) { meTextLanguage = eTextLanguage; }
};

#endif // INCLUDED_VCL_METAACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
