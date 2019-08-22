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

#include <memory>
#include <vcl/dllapi.h>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/wall.hxx>
#include <vcl/font.hxx>
#include <tools/poly.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/region.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaactiontypes.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>

class OutputDevice;
class SvStream;
enum class DrawTextFlags;

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
    MetaActionType const mnType;

protected:
    virtual             ~MetaAction() override;

public:
                        MetaAction();
    explicit            MetaAction( MetaActionType nType );
                        MetaAction( MetaAction const & );

    virtual void        Execute( OutputDevice* pOut );

    oslInterlockedCount GetRefCount() const { return m_nCount; }

    virtual rtl::Reference<MetaAction> Clone();

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    MetaActionType      GetType() const { return mnType; }

public:
    static MetaAction*  ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData );
};

class VCL_DLLPUBLIC MetaPixelAction : public MetaAction
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
protected:
    virtual             ~MetaPixelAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaPixelAction( const Point& rPt, const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetPoint() const { return maPt; }
    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaPointAction : public MetaAction
{
private:
    Point               maPt;

public:
                        MetaPointAction();
    MetaPointAction(MetaPointAction const &) = default;
    MetaPointAction(MetaPointAction &&) = default;
    MetaPointAction & operator =(MetaPointAction const &) = delete; // due to MetaAction
    MetaPointAction & operator =(MetaPointAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaPointAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPointAction( const Point& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaLineAction : public MetaAction
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
protected:
    virtual             ~MetaLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaLineAction( const Point& rStart, const Point& rEnd );
                        MetaLineAction( const Point& rStart, const Point& rEnd,
                                        const LineInfo& rLineInfo );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaRectAction : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaRectAction();
    MetaRectAction(MetaRectAction const &) = default;
    MetaRectAction(MetaRectAction &&) = default;
    MetaRectAction & operator =(MetaRectAction const &) = delete; // due to MetaAction
    MetaRectAction & operator =(MetaRectAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaRectAction( const tools::Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaRoundRectAction : public MetaAction
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
protected:
    virtual             ~MetaRoundRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaRoundRectAction( const tools::Rectangle& rRect,
                                             sal_uInt32 nHorzRound, sal_uInt32 nVertRound );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    sal_uInt32          GetHorzRound() const { return mnHorzRound; }
    sal_uInt32          GetVertRound() const { return mnVertRound; }
};

class VCL_DLLPUBLIC MetaEllipseAction : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaEllipseAction();
    MetaEllipseAction(MetaEllipseAction const &) = default;
    MetaEllipseAction(MetaEllipseAction &&) = default;
    MetaEllipseAction & operator =(MetaEllipseAction const &) = delete; // due to MetaAction
    MetaEllipseAction & operator =(MetaEllipseAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaEllipseAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaEllipseAction( const tools::Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaArcAction : public MetaAction
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
protected:
    virtual             ~MetaArcAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaArcAction( const tools::Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPieAction : public MetaAction
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
protected:
    virtual             ~MetaPieAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaPieAction( const tools::Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaChordAction : public MetaAction
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
protected:
    virtual             ~MetaChordAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaChordAction( const tools::Rectangle& rRect,
                                         const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPolyLineAction : public MetaAction
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
protected:
    virtual             ~MetaPolyLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPolyLineAction( const tools::Polygon& );
    explicit            MetaPolyLineAction( const tools::Polygon&, const LineInfo& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Polygon& GetPolygon() const { return maPoly; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaPolygonAction : public MetaAction
{
private:

    tools::Polygon      maPoly;

public:
                        MetaPolygonAction();
    MetaPolygonAction(MetaPolygonAction const &) = default;
    MetaPolygonAction(MetaPolygonAction &&) = default;
    MetaPolygonAction & operator =(MetaPolygonAction const &) = delete; // due to MetaAction
    MetaPolygonAction & operator =(MetaPolygonAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaPolygonAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPolygonAction( const tools::Polygon& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Polygon& GetPolygon() const { return maPoly; }
};

class VCL_DLLPUBLIC MetaPolyPolygonAction : public MetaAction
{
private:

    tools::PolyPolygon  maPolyPoly;

public:
                        MetaPolyPolygonAction();
    MetaPolyPolygonAction(MetaPolyPolygonAction const &) = default;
    MetaPolyPolygonAction(MetaPolyPolygonAction &&) = default;
    MetaPolyPolygonAction & operator =(MetaPolyPolygonAction const &) = delete; // due to MetaAction
    MetaPolyPolygonAction & operator =(MetaPolyPolygonAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaPolyPolygonAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPolyPolygonAction( const tools::PolyPolygon& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
};

class VCL_DLLPUBLIC MetaTextAction : public MetaAction
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
protected:
    virtual             ~MetaTextAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    MetaTextAction( const Point& rPt, const OUString& rStr,
                    sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( long nHorzMove, long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    const Point&    GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
};

class VCL_DLLPUBLIC MetaTextArrayAction : public MetaAction
{
private:

    Point       maStartPt;
    OUString    maStr;
    std::unique_ptr<long[]>
                mpDXAry;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

protected:
    virtual             ~MetaTextArrayAction() override;

public:
                        MetaTextArrayAction();
                        MetaTextArrayAction( const MetaTextArrayAction& rAction );
    MetaTextArrayAction( const Point& rStartPt, const OUString& rStr,
                         const long* pDXAry, sal_Int32 nIndex,
                         sal_Int32 nLen );

    virtual void        Execute( OutputDevice* pOut ) override;

    virtual rtl::Reference<MetaAction> Clone() override;

    virtual void    Move( long nHorzMove, long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    virtual void    Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void    Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    const Point&    GetPoint() const { return maStartPt; }
    const OUString& GetText() const { return maStr; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
    long*           GetDXArray() const { return mpDXAry.get(); }
};

class VCL_DLLPUBLIC MetaStretchTextAction : public MetaAction
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
protected:
    virtual             ~MetaStretchTextAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                           const OUString& rStr,
                           sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( long nHorzMove, long nVertMove ) override;
    virtual void    Scale( double fScaleX, double fScaleY ) override;

    const Point&    GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt32      GetWidth() const { return mnWidth; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
};

class VCL_DLLPUBLIC MetaTextRectAction : public MetaAction
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
protected:
    virtual             ~MetaTextRectAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    MetaTextRectAction( const tools::Rectangle& rRect,
                        const OUString& rStr, DrawTextFlags nStyle );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const OUString&     GetText() const { return maStr; }
    DrawTextFlags       GetStyle() const { return mnStyle; }
};

class VCL_DLLPUBLIC MetaTextLineAction : public MetaAction
{
private:

    Point               maPos;
    long                mnWidth;
    FontStrikeout       meStrikeout;
    FontLineStyle       meUnderline;
    FontLineStyle       meOverline;

public:
                        MetaTextLineAction();
    MetaTextLineAction(MetaTextLineAction const &) = default;
    MetaTextLineAction(MetaTextLineAction &&) = default;
    MetaTextLineAction & operator =(MetaTextLineAction const &) = delete; // due to MetaAction
    MetaTextLineAction & operator =(MetaTextLineAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaTextLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaTextLineAction( const Point& rPos, long nWidth,
                                            FontStrikeout eStrikeout,
                                            FontLineStyle eUnderline,
                                            FontLineStyle eOverline );
    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Point&        GetStartPoint() const { return maPos; }
    long                GetWidth() const { return mnWidth; }
    FontStrikeout       GetStrikeout() const { return meStrikeout; }
    FontLineStyle       GetUnderline() const { return meUnderline; }
    FontLineStyle       GetOverline()  const { return meOverline; }
};

class VCL_DLLPUBLIC MetaBmpAction : public MetaAction
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
protected:
    virtual             ~MetaBmpAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpAction( const Point& rPt, const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpScaleAction : public MetaAction
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
protected:
    virtual             ~MetaBmpScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                            const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaBmpScalePartAction : public MetaAction
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
protected:
    virtual             ~MetaBmpScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaBmpExAction : public MetaAction
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
protected:
    virtual             ~MetaBmpExAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpExScaleAction : public MetaAction
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
protected:
    virtual             ~MetaBmpExScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                              const BitmapEx& rBmpEx ) ;

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaBmpExScalePartAction : public MetaAction
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
protected:
    virtual             ~MetaBmpExScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaMaskAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color const         maColor;
    Point               maPt;

public:
                        MetaMaskAction();
    MetaMaskAction(MetaMaskAction const &) = default;
    MetaMaskAction(MetaMaskAction &&) = default;
    MetaMaskAction & operator =(MetaMaskAction const &) = delete; // due to MetaAction
    MetaMaskAction & operator =(MetaMaskAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaMaskAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaMaskAction( const Point& rPt,
                                        const Bitmap& rBmp,
                                        const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaMaskScaleAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color const         maColor;
    Point               maPt;
    Size                maSz;

public:
                        MetaMaskScaleAction();
    MetaMaskScaleAction(MetaMaskScaleAction const &) = default;
    MetaMaskScaleAction(MetaMaskScaleAction &&) = default;
    MetaMaskScaleAction & operator =(MetaMaskScaleAction const &) = delete; // due to MetaAction
    MetaMaskScaleAction & operator =(MetaMaskScaleAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaMaskScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                             const Bitmap& rBmp,
                                             const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

class VCL_DLLPUBLIC MetaMaskScalePartAction : public MetaAction
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
protected:
    virtual             ~MetaMaskScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                 const Point& rSrcPt, const Size& rSrcSz,
                                                 const Bitmap& rBmp,
                                                 const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

class VCL_DLLPUBLIC MetaGradientAction : public MetaAction
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
protected:
    virtual             ~MetaGradientAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaGradientAction( const tools::Rectangle& rRect, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaGradientExAction : public MetaAction
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
protected:
    virtual             ~MetaGradientExAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaGradientExAction( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaHatchAction : public MetaAction
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
protected:
    virtual             ~MetaHatchAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaHatchAction( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Hatch&        GetHatch() const { return maHatch; }
};

class VCL_DLLPUBLIC MetaWallpaperAction : public MetaAction
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
protected:
    virtual             ~MetaWallpaperAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaWallpaperAction( const tools::Rectangle& rRect,
                                             const Wallpaper& rPaper );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
    const Wallpaper&    GetWallpaper() const { return maWallpaper; }
};

class VCL_DLLPUBLIC MetaClipRegionAction : public MetaAction
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
protected:
    virtual             ~MetaClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaClipRegionAction( const vcl::Region& rRegion, bool bClip );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Region&  GetRegion() const { return maRegion; }
    bool                IsClipping() const { return mbClip; }
};

class VCL_DLLPUBLIC MetaISectRectClipRegionAction : public MetaAction
{
private:

    tools::Rectangle           maRect;

public:
                        MetaISectRectClipRegionAction();
    MetaISectRectClipRegionAction(MetaISectRectClipRegionAction const &) = default;
    MetaISectRectClipRegionAction(MetaISectRectClipRegionAction &&) = default;
    MetaISectRectClipRegionAction & operator =(MetaISectRectClipRegionAction const &) = delete; // due to MetaAction
    MetaISectRectClipRegionAction & operator =(MetaISectRectClipRegionAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaISectRectClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaISectRectClipRegionAction( const tools::Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaISectRegionClipRegionAction : public MetaAction
{
private:

    vcl::Region          maRegion;

public:
                        MetaISectRegionClipRegionAction();
    MetaISectRegionClipRegionAction(MetaISectRegionClipRegionAction const &) = default;
    MetaISectRegionClipRegionAction(MetaISectRegionClipRegionAction &&) = default;
    MetaISectRegionClipRegionAction & operator =(MetaISectRegionClipRegionAction const &) = delete; // due to MetaAction
    MetaISectRegionClipRegionAction & operator =(MetaISectRegionClipRegionAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaISectRegionClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaISectRegionClipRegionAction( const vcl::Region& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Region&  GetRegion() const { return maRegion; }
};

class VCL_DLLPUBLIC MetaMoveClipRegionAction : public MetaAction
{
private:

    long                mnHorzMove;
    long                mnVertMove;

public:
                        MetaMoveClipRegionAction();
    MetaMoveClipRegionAction(MetaMoveClipRegionAction const &) = default;
    MetaMoveClipRegionAction(MetaMoveClipRegionAction &&) = default;
    MetaMoveClipRegionAction & operator =(MetaMoveClipRegionAction const &) = delete; // due to MetaAction
    MetaMoveClipRegionAction & operator =(MetaMoveClipRegionAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaMoveClipRegionAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaMoveClipRegionAction( long nHorzMove, long nVertMove );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    long                GetHorzMove() const { return mnHorzMove; }
    long                GetVertMove() const { return mnVertMove; }
};

class VCL_DLLPUBLIC MetaLineColorAction : public MetaAction
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
protected:
    virtual             ~MetaLineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaFillColorAction : public MetaAction
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
protected:
    virtual             ~MetaFillColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextColorAction : public MetaAction
{
private:

    Color               maColor;

public:
                        MetaTextColorAction();
    MetaTextColorAction(MetaTextColorAction const &) = default;
    MetaTextColorAction(MetaTextColorAction &&) = default;
    MetaTextColorAction & operator =(MetaTextColorAction const &) = delete; // due to MetaAction
    MetaTextColorAction & operator =(MetaTextColorAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaTextColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaTextColorAction( const Color& );

    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaTextFillColorAction : public MetaAction
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
protected:
    virtual             ~MetaTextFillColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaTextFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextLineColorAction : public MetaAction
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
protected:
    virtual             ~MetaTextLineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaTextLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaOverlineColorAction : public MetaAction
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
protected:
    virtual             ~MetaOverlineColorAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaOverlineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextAlignAction : public MetaAction
{
private:

    TextAlign           maAlign;

public:
                        MetaTextAlignAction();
    MetaTextAlignAction(MetaTextAlignAction const &) = default;
    MetaTextAlignAction(MetaTextAlignAction &&) = default;
    MetaTextAlignAction & operator =(MetaTextAlignAction const &) = delete; // due to MetaAction
    MetaTextAlignAction & operator =(MetaTextAlignAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaTextAlignAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaTextAlignAction( TextAlign eAlign );

    TextAlign           GetTextAlign() const { return maAlign; }
};

class VCL_DLLPUBLIC MetaMapModeAction : public MetaAction
{
private:

    MapMode             maMapMode;

public:
                        MetaMapModeAction();
    MetaMapModeAction(MetaMapModeAction const &) = default;
    MetaMapModeAction(MetaMapModeAction &&) = default;
    MetaMapModeAction & operator =(MetaMapModeAction const &) = delete; // due to MetaAction
    MetaMapModeAction & operator =(MetaMapModeAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaMapModeAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaMapModeAction( const MapMode& );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const MapMode&      GetMapMode() const { return maMapMode; }
};

class VCL_DLLPUBLIC MetaFontAction : public MetaAction
{
private:

    vcl::Font           maFont;

public:
                        MetaFontAction();
    MetaFontAction(MetaFontAction const &) = default;
    MetaFontAction(MetaFontAction &&) = default;
    MetaFontAction & operator =(MetaFontAction const &) = delete; // due to MetaAction
    MetaFontAction & operator =(MetaFontAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaFontAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaFontAction( const vcl::Font& );

    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const vcl::Font&    GetFont() const { return maFont; }
};

class VCL_DLLPUBLIC MetaPushAction : public MetaAction
{
private:

    PushFlags           mnFlags;

public:
                        MetaPushAction();
    MetaPushAction(MetaPushAction const &) = default;
    MetaPushAction(MetaPushAction &&) = default;
    MetaPushAction & operator =(MetaPushAction const &) = delete; // due to MetaAction
    MetaPushAction & operator =(MetaPushAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaPushAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPushAction( PushFlags nFlags );

    PushFlags           GetFlags() const { return mnFlags; }
};

class VCL_DLLPUBLIC MetaPopAction : public MetaAction
{
public:

                        MetaPopAction();
    MetaPopAction(MetaPopAction const &) = default;
    MetaPopAction(MetaPopAction &&) = default;
    MetaPopAction & operator =(MetaPopAction const &) = delete; // due to MetaAction
    MetaPopAction & operator =(MetaPopAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaPopAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;
};

class VCL_DLLPUBLIC MetaRasterOpAction : public MetaAction
{
private:

    RasterOp            meRasterOp;

public:
                        MetaRasterOpAction();
    MetaRasterOpAction(MetaRasterOpAction const &) = default;
    MetaRasterOpAction(MetaRasterOpAction &&) = default;
    MetaRasterOpAction & operator =(MetaRasterOpAction const &) = delete; // due to MetaAction
    MetaRasterOpAction & operator =(MetaRasterOpAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaRasterOpAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaRasterOpAction( RasterOp eRasterOp );

    RasterOp            GetRasterOp() const { return meRasterOp; }
};

class VCL_DLLPUBLIC MetaTransparentAction : public MetaAction
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
protected:
    virtual             ~MetaTransparentAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaTransparentAction( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    sal_uInt16              GetTransparence() const { return mnTransPercent; }
};

class VCL_DLLPUBLIC MetaFloatTransparentAction : public MetaAction
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
protected:
    virtual             ~MetaFloatTransparentAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                    const Size& rSize, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const GDIMetaFile&  GetGDIMetaFile() const { return maMtf; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaEPSAction : public MetaAction
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
protected:
    virtual             ~MetaEPSAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaEPSAction( const Point& rPoint, const Size& rSize,
                                       const GfxLink& rGfxLink, const GDIMetaFile& rSubst );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const GfxLink&      GetLink() const { return maGfxLink; }
    const GDIMetaFile&  GetSubstitute() const { return maSubst; }
    const Point&        GetPoint() const { return maPoint; }
    const Size&         GetSize() const { return maSize; }
};

class VCL_DLLPUBLIC MetaRefPointAction : public MetaAction
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
protected:
    virtual             ~MetaRefPointAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaRefPointAction( const Point& rRefPoint, bool bSet );

    const Point&        GetRefPoint() const { return maRefPoint; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaCommentAction : public MetaAction
{
private:

    OString             maComment;
    sal_Int32           mnValue;
    sal_uInt32          mnDataSize;
    std::unique_ptr<sal_uInt8[]>
                        mpData;

    SAL_DLLPRIVATE void ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize );

protected:
    virtual             ~MetaCommentAction() override;

public:
    explicit            MetaCommentAction();
    explicit            MetaCommentAction( const MetaCommentAction& rAct );
    explicit            MetaCommentAction( const OString& rComment, sal_Int32 nValue = 0, const sal_uInt8* pData = nullptr, sal_uInt32 nDataSize = 0 );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    const OString&      GetComment() const { return maComment; }
    sal_Int32           GetValue() const { return mnValue; }
    sal_uInt32          GetDataSize() const { return mnDataSize; }
    const sal_uInt8*    GetData() const { return mpData.get(); }
};

class VCL_DLLPUBLIC MetaLayoutModeAction : public MetaAction
{
private:

    ComplexTextLayoutFlags  mnLayoutMode;

public:
                        MetaLayoutModeAction();
    MetaLayoutModeAction(MetaLayoutModeAction const &) = default;
    MetaLayoutModeAction(MetaLayoutModeAction &&) = default;
    MetaLayoutModeAction & operator =(MetaLayoutModeAction const &) = delete; // due to MetaAction
    MetaLayoutModeAction & operator =(MetaLayoutModeAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaLayoutModeAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaLayoutModeAction( ComplexTextLayoutFlags nLayoutMode );

    ComplexTextLayoutFlags  GetLayoutMode() const { return mnLayoutMode; }
};

class VCL_DLLPUBLIC MetaTextLanguageAction : public MetaAction
{
private:

    LanguageType        meTextLanguage;

public:
                        MetaTextLanguageAction();
    MetaTextLanguageAction(MetaTextLanguageAction const &) = default;
    MetaTextLanguageAction(MetaTextLanguageAction &&) = default;
    MetaTextLanguageAction & operator =(MetaTextLanguageAction const &) = delete; // due to MetaAction
    MetaTextLanguageAction & operator =(MetaTextLanguageAction &&) = delete; // due to MetaAction
protected:
    virtual             ~MetaTextLanguageAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaTextLanguageAction( LanguageType );

    LanguageType        GetTextLanguage() const { return meTextLanguage; }
};

#endif // INCLUDED_VCL_METAACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
