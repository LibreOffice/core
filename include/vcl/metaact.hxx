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

#include <vcl/dllapi.h>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/wall.hxx>
#include <vcl/font.hxx>
#include <tools/poly.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/region.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/metaactiontypes.hxx>
#include <vcl/outdevstate.hxx>

class SvStream;

struct ImplMetaReadData
{
    rtl_TextEncoding meActualCharSet;

    ImplMetaReadData() :
        meActualCharSet( RTL_TEXTENCODING_ASCII_US )
    {}
};

struct ImplMetaWriteData
{
    rtl_TextEncoding meActualCharSet;

    ImplMetaWriteData() :
        meActualCharSet( RTL_TEXTENCODING_ASCII_US )
    {}
};

class VCL_DLLPUBLIC MetaAction
{
private:
    sal_uLong               mnRefCount;
    sal_uInt16              mnType;

    virtual bool    Compare( const MetaAction& ) const;

protected:
    virtual             ~MetaAction();

public:
                        MetaAction();
    explicit            MetaAction( sal_uInt16 nType );

    virtual void        Execute( OutputDevice* pOut );

    virtual MetaAction* Clone();

    virtual void        Move( long nHorzMove, long nVertMove );
    virtual void        Scale( double fScaleX, double fScaleY );

    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData );
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData );

    sal_uInt16              GetType() const { return mnType; }
    sal_uLong               GetRefCount() const { return mnRefCount; }
    void                ResetRefCount() { mnRefCount = 1; }
    void                Duplicate()  { mnRefCount++; }
    void                Delete() { if ( 0 == --mnRefCount ) delete this; }

public:
    OUString            GetTypeName() { return TypeName( mnType ); }

    static MetaAction*  ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData );
    static OUString     TypeName( sal_uInt16 nType );
};

class VCL_DLLPUBLIC MetaPixelAction : public MetaAction
{
private:
    Point               maPt;
    Color               maColor;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPixelAction();
protected:
    virtual             ~MetaPixelAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaPixelAction( const Point& rPt, const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Point&        GetPoint() const { return maPt; }
    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaPointAction : public MetaAction
{
private:
    Point               maPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPointAction();
protected:
    virtual             ~MetaPointAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaPointAction( const Point& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaLineAction : public MetaAction
{
private:

    LineInfo            maLineInfo;
    Point               maStartPt;
    Point               maEndPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaLineAction();
protected:
    virtual             ~MetaLineAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaLineAction( const Point& rStart, const Point& rEnd );
                        MetaLineAction( const Point& rStart, const Point& rEnd,
                                        const LineInfo& rLineInfo );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaRectAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;
public:
                        MetaRectAction();
protected:
    virtual             ~MetaRectAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaRectAction( const Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaRoundRectAction : public MetaAction
{
private:

    Rectangle           maRect;
    sal_uInt32          mnHorzRound;
    sal_uInt32          mnVertRound;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaRoundRectAction();
protected:
    virtual             ~MetaRoundRectAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaRoundRectAction( const Rectangle& rRect,
                                             sal_uInt32 nHorzRound, sal_uInt32 nVertRound );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    sal_uInt32          GetHorzRound() const { return mnHorzRound; }
    sal_uInt32          GetVertRound() const { return mnVertRound; }
};

class VCL_DLLPUBLIC MetaEllipseAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaEllipseAction();
protected:
    virtual             ~MetaEllipseAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaEllipseAction( const Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaArcAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaArcAction();
protected:
    virtual             ~MetaArcAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaArcAction( const Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPieAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPieAction();
protected:
    virtual             ~MetaPieAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaPieAction( const Rectangle& rRect,
                                       const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaChordAction : public MetaAction
{
private:

    Rectangle           maRect;
    Point               maStartPt;
    Point               maEndPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaChordAction();
protected:
    virtual             ~MetaChordAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaChordAction( const Rectangle& rRect,
                                         const Point& rStart, const Point& rEnd );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const Point&        GetStartPoint() const { return maStartPt; }
    const Point&        GetEndPoint() const { return maEndPt; }
};

class VCL_DLLPUBLIC MetaPolyLineAction : public MetaAction
{
private:

    LineInfo            maLineInfo;
    Polygon             maPoly;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPolyLineAction();
protected:
    virtual             ~MetaPolyLineAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaPolyLineAction( const Polygon& );
    explicit            MetaPolyLineAction( const Polygon&, const LineInfo& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Polygon&      GetPolygon() const { return maPoly; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

class VCL_DLLPUBLIC MetaPolygonAction : public MetaAction
{
private:

    Polygon             maPoly;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPolygonAction();
protected:
    virtual             ~MetaPolygonAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaPolygonAction( const Polygon& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Polygon&      GetPolygon() const { return maPoly; }
};

class VCL_DLLPUBLIC MetaPolyPolygonAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPolyPolygonAction();
protected:
    virtual             ~MetaPolyPolygonAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaPolyPolygonAction( const PolyPolygon& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
};

class VCL_DLLPUBLIC MetaTextAction : public MetaAction
{
private:

    Point           maPt;
    OUString        maStr;
    sal_Int32       mnIndex;
    sal_Int32       mnLen;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextAction();
protected:
    virtual             ~MetaTextAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    MetaTextAction( const Point& rPt, const OUString& rStr,
                    sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void    Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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
    long*       mpDXAry;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

protected:
    virtual             ~MetaTextArrayAction();

public:
                        MetaTextArrayAction();
                        MetaTextArrayAction( const MetaTextArrayAction& rAction );
    MetaTextArrayAction( const Point& rStartPt, const OUString& rStr,
                         const long* pDXAry, sal_Int32 nIndex,
                         sal_Int32 nLen );

    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;

    virtual MetaAction* Clone() SAL_OVERRIDE;

    virtual void    Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void    Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    virtual void    Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void    Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    const Point&    GetPoint() const { return maStartPt; }
    const OUString& GetText() const { return maStr; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
    long*           GetDXArray() const { return mpDXAry; }
};

class VCL_DLLPUBLIC MetaStretchTextAction : public MetaAction
{
private:

    Point       maPt;
    OUString    maStr;
    sal_uInt32  mnWidth;
    sal_Int32   mnIndex;
    sal_Int32   mnLen;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaStretchTextAction();
protected:
    virtual             ~MetaStretchTextAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                           const OUString& rStr,
                           sal_Int32 nIndex, sal_Int32 nLen );

    virtual void    Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void    Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Point&    GetPoint() const { return maPt; }
    const OUString& GetText() const { return maStr; }
    sal_uInt32      GetWidth() const { return mnWidth; }
    sal_Int32       GetIndex() const { return mnIndex; }
    sal_Int32       GetLen() const { return mnLen; }
};

class VCL_DLLPUBLIC MetaTextRectAction : public MetaAction
{
private:

    Rectangle           maRect;
    OUString       maStr;
    sal_uInt16          mnStyle;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextRectAction();
protected:
    virtual             ~MetaTextRectAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    MetaTextRectAction( const Rectangle& rRect,
                        const OUString& rStr, sal_uInt16 nStyle );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const OUString& GetText() const { return maStr; }
    sal_uInt16              GetStyle() const { return mnStyle; }
};

class VCL_DLLPUBLIC MetaTextLineAction : public MetaAction
{
private:

    Point               maPos;
    long                mnWidth;
    FontStrikeout       meStrikeout;
    FontUnderline       meUnderline;
    FontUnderline       meOverline;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextLineAction();
protected:
    virtual             ~MetaTextLineAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaTextLineAction( const Point& rPos, long nWidth,
                                            FontStrikeout eStrikeout,
                                            FontUnderline eUnderline,
                                            FontUnderline eOverline );
    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Point&        GetStartPoint() const { return maPos; }
    long                GetWidth() const { return mnWidth; }
    FontStrikeout       GetStrikeout() const { return meStrikeout; }
    FontUnderline       GetUnderline() const { return meUnderline; }
    FontUnderline       GetOverline()  const { return meOverline; }
};

class VCL_DLLPUBLIC MetaBmpAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpAction();
protected:
    virtual             ~MetaBmpAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpAction( const Point& rPt, const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpScaleAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maPt;
    Size                maSz;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpScaleAction();
protected:
    virtual             ~MetaBmpScaleAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                            const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpScalePartAction();
protected:
    virtual             ~MetaBmpScalePartAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpExAction();
protected:
    virtual             ~MetaBmpExAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const BitmapEx&     GetBitmapEx() const { return maBmpEx; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaBmpExScaleAction : public MetaAction
{
private:

    BitmapEx            maBmpEx;
    Point               maPt;
    Size                maSz;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpExScaleAction();
protected:
    virtual             ~MetaBmpExScaleAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                              const BitmapEx& rBmpEx ) ;

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaBmpExScalePartAction();
protected:
    virtual             ~MetaBmpExScalePartAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const BitmapEx& rBmpEx );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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
    Color               maColor;
    Point               maPt;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaMaskAction();
protected:
    virtual             ~MetaMaskAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaMaskAction( const Point& rPt,
                                        const Bitmap& rBmp,
                                        const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
};

class VCL_DLLPUBLIC MetaMaskScaleAction : public MetaAction
{
private:

    Bitmap              maBmp;
    Color               maColor;
    Point               maPt;
    Size                maSz;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaMaskScaleAction();
protected:
    virtual             ~MetaMaskScaleAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                             const Bitmap& rBmp,
                                             const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaMaskScalePartAction();
protected:
    virtual             ~MetaMaskScalePartAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                 const Point& rSrcPt, const Size& rSrcSz,
                                                 const Bitmap& rBmp,
                                                 const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    Rectangle           maRect;
    Gradient            maGradient;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaGradientAction();
protected:
    virtual             ~MetaGradientAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaGradientAction( const Rectangle& rRect, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaGradientExAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    Gradient            maGradient;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaGradientExAction();
protected:
    virtual             ~MetaGradientExAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaGradientExAction( const PolyPolygon& rPolyPoly, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Gradient&     GetGradient() const { return maGradient; }
};

class VCL_DLLPUBLIC MetaHatchAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    Hatch               maHatch;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaHatchAction();
protected:
    virtual             ~MetaHatchAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaHatchAction( const PolyPolygon& rPolyPoly, const Hatch& rHatch );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    const Hatch&        GetHatch() const { return maHatch; }
};

class VCL_DLLPUBLIC MetaWallpaperAction : public MetaAction
{
private:

    Rectangle           maRect;
    Wallpaper           maWallpaper;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaWallpaperAction();
protected:
    virtual             ~MetaWallpaperAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaWallpaperAction( const Rectangle& rRect,
                                             const Wallpaper& rPaper );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
    const Wallpaper&    GetWallpaper() const { return maWallpaper; }
};

class VCL_DLLPUBLIC MetaClipRegionAction : public MetaAction
{
private:

    vcl::Region              maRegion;
    bool                mbClip;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaClipRegionAction();
protected:
    virtual             ~MetaClipRegionAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaClipRegionAction( const vcl::Region& rRegion, bool bClip );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const vcl::Region&       GetRegion() const { return maRegion; }
    bool                IsClipping() const { return mbClip; }
};

class VCL_DLLPUBLIC MetaISectRectClipRegionAction : public MetaAction
{
private:

    Rectangle           maRect;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaISectRectClipRegionAction();
protected:
    virtual             ~MetaISectRectClipRegionAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaISectRectClipRegionAction( const Rectangle& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const Rectangle&    GetRect() const { return maRect; }
};

class VCL_DLLPUBLIC MetaISectRegionClipRegionAction : public MetaAction
{
private:

    vcl::Region              maRegion;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaISectRegionClipRegionAction();
protected:
    virtual             ~MetaISectRegionClipRegionAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaISectRegionClipRegionAction( const vcl::Region& );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const vcl::Region&       GetRegion() const { return maRegion; }
};

class VCL_DLLPUBLIC MetaMoveClipRegionAction : public MetaAction
{
private:

    long                mnHorzMove;
    long                mnVertMove;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaMoveClipRegionAction();
protected:
    virtual             ~MetaMoveClipRegionAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaMoveClipRegionAction( long nHorzMove, long nVertMove );

    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    long                GetHorzMove() const { return mnHorzMove; }
    long                GetVertMove() const { return mnVertMove; }
};

class VCL_DLLPUBLIC MetaLineColorAction : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaLineColorAction();
protected:
    virtual             ~MetaLineColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaFillColorAction : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaFillColorAction();
protected:
    virtual             ~MetaFillColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextColorAction : public MetaAction
{
private:

    Color               maColor;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextColorAction();
protected:
    virtual             ~MetaTextColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaTextColorAction( const Color& );

    const Color&        GetColor() const { return maColor; }
};

class VCL_DLLPUBLIC MetaTextFillColorAction : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextFillColorAction();
protected:
    virtual             ~MetaTextFillColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaTextFillColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextLineColorAction : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextLineColorAction();
protected:
    virtual             ~MetaTextLineColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaTextLineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaOverlineColorAction : public MetaAction
{
private:

    Color               maColor;
    bool                mbSet;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaOverlineColorAction();
protected:
    virtual             ~MetaOverlineColorAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaOverlineColorAction( const Color& rColor, bool bSet );

    const Color&        GetColor() const { return maColor; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaTextAlignAction : public MetaAction
{
private:

    TextAlign           maAlign;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextAlignAction();
protected:
    virtual             ~MetaTextAlignAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaTextAlignAction( TextAlign eAlign );

    TextAlign           GetTextAlign() const { return maAlign; }
};

class VCL_DLLPUBLIC MetaMapModeAction : public MetaAction
{
private:

    MapMode             maMapMode;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaMapModeAction();
protected:
    virtual             ~MetaMapModeAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaMapModeAction( const MapMode& );

    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const MapMode&      GetMapMode() const { return maMapMode; }
};

class VCL_DLLPUBLIC MetaFontAction : public MetaAction
{
private:

    vcl::Font           maFont;

    virtual bool        Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaFontAction();
protected:
    virtual             ~MetaFontAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaFontAction( const vcl::Font& );

    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const vcl::Font&    GetFont() const { return maFont; }
};

class VCL_DLLPUBLIC MetaPushAction : public MetaAction
{
private:

    PushFlags              mnFlags;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaPushAction();
protected:
    virtual             ~MetaPushAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaPushAction( PushFlags nFlags );

    PushFlags           GetFlags() const { return mnFlags; }
};

class VCL_DLLPUBLIC MetaPopAction : public MetaAction
{
public:

                        MetaPopAction();
protected:
    virtual             ~MetaPopAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;
};

class VCL_DLLPUBLIC MetaRasterOpAction : public MetaAction
{
private:

    RasterOp            meRasterOp;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaRasterOpAction();
protected:
    virtual             ~MetaRasterOpAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaRasterOpAction( RasterOp eRasterOp );

    RasterOp            GetRasterOp() const { return meRasterOp; }
};

class VCL_DLLPUBLIC MetaTransparentAction : public MetaAction
{
private:

    PolyPolygon         maPolyPoly;
    sal_uInt16              mnTransPercent;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTransparentAction();
protected:
    virtual             ~MetaTransparentAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaTransparentAction( const PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    const PolyPolygon&  GetPolyPolygon() const { return maPolyPoly; }
    sal_uInt16              GetTransparence() const { return mnTransPercent; }
};

class VCL_DLLPUBLIC MetaFloatTransparentAction : public MetaAction
{
private:

    GDIMetaFile         maMtf;
    Point               maPoint;
    Size                maSize;
    Gradient            maGradient;

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaFloatTransparentAction();
protected:
    virtual             ~MetaFloatTransparentAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                    const Size& rSize, const Gradient& rGradient );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaEPSAction();
protected:
    virtual             ~MetaEPSAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaEPSAction( const Point& rPoint, const Size& rSize,
                                       const GfxLink& rGfxLink, const GDIMetaFile& rSubst );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

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

    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaRefPointAction();
protected:
    virtual             ~MetaRefPointAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

                        MetaRefPointAction( const Point& rRefPoint, bool bSet );

    const Point&        GetRefPoint() const { return maRefPoint; }
    bool                IsSetting() const { return mbSet; }
};

class VCL_DLLPUBLIC MetaCommentAction : public MetaAction
{
private:

    OString        maComment;
    sal_Int32           mnValue;
    sal_uInt32          mnDataSize;
    sal_uInt8*          mpData;

    SAL_DLLPRIVATE void ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize );
    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

protected:
    virtual             ~MetaCommentAction();

public:
    explicit            MetaCommentAction( sal_Int32 nValue = 0L );
    explicit            MetaCommentAction( const MetaCommentAction& rAct );
    explicit            MetaCommentAction( const OString& rComment, sal_Int32 nValue = 0L, const sal_uInt8* pData = NULL, sal_uInt32 nDataSize = 0UL );

    virtual void        Move( long nHorzMove, long nVertMove ) SAL_OVERRIDE;
    virtual void        Scale( double fScaleX, double fScaleY ) SAL_OVERRIDE;

    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    const OString& GetComment() const { return maComment; }
    sal_Int32           GetValue() const { return mnValue; }
    sal_uInt32          GetDataSize() const { return mnDataSize; }
    const sal_uInt8*        GetData() const { return mpData; }
};

class VCL_DLLPUBLIC MetaLayoutModeAction : public MetaAction
{
private:

    ComplexTextLayoutMode  mnLayoutMode;
    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaLayoutModeAction();
protected:
    virtual             ~MetaLayoutModeAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaLayoutModeAction( ComplexTextLayoutMode nLayoutMode );

    ComplexTextLayoutMode  GetLayoutMode() const { return mnLayoutMode; }
};

class VCL_DLLPUBLIC MetaTextLanguageAction : public MetaAction
{
private:

    LanguageType        meTextLanguage;
    virtual bool    Compare( const MetaAction& ) const SAL_OVERRIDE;

public:
                        MetaTextLanguageAction();
protected:
    virtual             ~MetaTextLanguageAction();
public:
    virtual void        Execute( OutputDevice* pOut ) SAL_OVERRIDE;
    virtual MetaAction* Clone() SAL_OVERRIDE;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) SAL_OVERRIDE;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) SAL_OVERRIDE;

    explicit            MetaTextLanguageAction( LanguageType );

    LanguageType        GetTextLanguage() const { return meTextLanguage; }
};

#endif // INCLUDED_VCL_METAACT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
