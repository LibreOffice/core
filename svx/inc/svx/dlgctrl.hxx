/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_DLG_CTRL_HXX
#define _SVX_DLG_CTRL_HXX

#include <svtools/ctrlbox.hxx>
#include <sfx2/tabdlg.hxx>
#include "svx/svxdllapi.h"
#include <svx/rectenum.hxx>
#include <vcl/graph.hxx>
#include <svx/xtable.hxx>

class XOBitmap;
class XOutdevItemPool;

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
} } } }

/*************************************************************************
|* Derived from SfxTabPage for being able to get notified through the
|* virtual method from the control.
\************************************************************************/

class SvxTabPage : public SfxTabPage
{

public:
    SvxTabPage( Window* pParent, ResId Id, const SfxItemSet& rInAttrs  ) :
        SfxTabPage( pParent, Id, rInAttrs ) {}

    virtual void PointChanged( Window* pWindow, RECT_POINT eRP ) = 0;
};

/*************************************************************************
|* Control for display and selection of the corner and center points of
|* an object
\************************************************************************/

typedef sal_uInt16 CTL_STATE;
#define CS_NOHORZ   1       // no horizontal input information is used
#define CS_NOVERT   2       // no vertikal input information is used

class SvxRectCtlAccessibleContext;

class SVX_DLLPUBLIC SvxRectCtl : public Control
{
private:
    SVX_DLLPRIVATE void             InitSettings( sal_Bool bForeground, sal_Bool bBackground );
    SVX_DLLPRIVATE void             InitRectBitmap( void );
    SVX_DLLPRIVATE Bitmap&          GetRectBitmap( void );
    SVX_DLLPRIVATE void             Resize_Impl();

protected:
    SvxRectCtlAccessibleContext*    pAccContext;
    sal_uInt16                          nBorderWidth;
    sal_uInt16                          nRadius;
    Size                            aSize;
    Point                           aPtLT, aPtMT, aPtRT;
    Point                           aPtLM, aPtMM, aPtRM;
    Point                           aPtLB, aPtMB, aPtRB;
    Point                           aPtNew;
    RECT_POINT                      eRP, eDefRP;
    CTL_STYLE                       eCS;
    Bitmap*                         pBitmap;
    CTL_STATE                       m_nState;

    sal_Bool                        mbCompleteDisable;

    RECT_POINT          GetRPFromPoint( Point ) const;
    Point               GetPointFromRP( RECT_POINT ) const;
    void                SetFocusRect( const Rectangle* pRect = NULL );      // pRect == NULL -> calculate rectangle in method
    Point               SetActualRPWithoutInvalidate( RECT_POINT eNewRP );  // returns the last point

    virtual void        GetFocus();
    virtual void        LoseFocus();

    Point               GetApproxLogPtFromPixPt( const Point& rRoughPixelPoint ) const;
public:
    SvxRectCtl( Window* pParent, const ResId& rResId, RECT_POINT eRpt = RP_MM,
                sal_uInt16 nBorder = 200, sal_uInt16 nCircle = 80, CTL_STYLE eStyle = CS_RECT );
    virtual ~SvxRectCtl();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKeyEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        Resize();

    void                Reset();
    RECT_POINT          GetActualRP() const;
    void                SetActualRP( RECT_POINT eNewRP );

    void                SetState( CTL_STATE nState );

    sal_uInt8               GetNumOfChildren( void ) const;   // returns number of usable radio buttons

    Rectangle           CalculateFocusRectangle( void ) const;
    Rectangle           CalculateFocusRectangle( RECT_POINT eRectPoint ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    RECT_POINT          GetApproxRPFromPixPt( const ::com::sun::star::awt::Point& rPixelPoint ) const;

    sal_Bool IsCompletelyDisabled() const { return mbCompleteDisable; }
    void DoCompletelyDisable(sal_Bool bNew);
};

/*************************************************************************
|* Preview control for the display of bitmaps
\************************************************************************/

class SVX_DLLPUBLIC SvxBitmapCtl
{
protected:
    Size            aSize;
    sal_uInt16          nLines;
    Color           aPixelColor, aBackgroundColor;
    const sal_uInt16*   pBmpArray;

public:
            SvxBitmapCtl( Window* pParent, const Size& rSize );
            ~SvxBitmapCtl();

    XOBitmap GetXBitmap();

    void    SetBmpArray( const sal_uInt16* pPixel ) { pBmpArray = pPixel; }
    void    SetLines( sal_uInt16 nLns ) { nLines = nLns; }
    void    SetPixelColor( Color aColor ) { aPixelColor = aColor; }
    void    SetBackgroundColor( Color aColor ) { aBackgroundColor = aColor; }
};

/*************************************************************************
|* Control for editing bitmaps
\************************************************************************/

class SVX_DLLPUBLIC SvxPixelCtl : public Control
{
private:
    using OutputDevice::SetLineColor;

protected:
    sal_uInt16      nLines, nSquares;
    Color       aPixelColor;
    Color       aBackgroundColor;
    Color       aLineColor;
    Size        aRectSize;
    sal_uInt16*     pPixel;
    sal_Bool        bPaintable;

    void    ChangePixel( sal_uInt16 nPixel );

public:
            SvxPixelCtl( Window* pParent, const ResId& rResId,
                        sal_uInt16 nNumber = 8 );
            ~SvxPixelCtl();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void    SetXBitmap( const XOBitmap& rXOBitmap );

    void    SetPixelColor( const Color& rCol ) { aPixelColor = rCol; }
    void    SetBackgroundColor( const Color& rCol ) { aBackgroundColor = rCol; }
    void    SetLineColor( const Color& rCol ) { aLineColor = rCol; }

    sal_uInt16  GetLineCount() const { return nLines; }
    Color   GetPixelColor() const { return aPixelColor; }
    Color   GetBackgroundColor() const { return aBackgroundColor; }

    sal_uInt16* GetBitmapPixelPtr() { return pPixel; }

    void    SetPaintable( sal_Bool bTmp ) { bPaintable = bTmp; }
    void    Reset();
};

/************************************************************************/

class SVX_DLLPUBLIC ColorLB : public ColorListBox
{

public:
         ColorLB( Window* pParent, ResId Id ) : ColorListBox( pParent, Id ) {}
         ColorLB( Window* pParent, WinBits aWB ) : ColorListBox( pParent, aWB ) {}

    virtual void Fill( const XColorListRef &pTab );

    void Append( XColorEntry* pEntry, Bitmap* pBmp = NULL );
    void Modify( XColorEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
};

/************************************************************************/

class SVX_DLLPUBLIC HatchingLB : public ListBox
{
    XHatchListRef mpList;
    sal_Bool      mbUserDraw;
public:
    HatchingLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );

    virtual void Fill( const XHatchListRef &pList );
    virtual void UserDraw( const UserDrawEvent& rUDEvt );

    void    Append( XHatchEntry* pEntry, Bitmap* pBmp = NULL );
    void    Modify( XHatchEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
};

/************************************************************************/

class SVX_DLLPUBLIC GradientLB : public ListBox
{
    XGradientListRef mpList;
    sal_Bool         mbUserDraw;
public:
    GradientLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );

    virtual void Fill( const XGradientListRef &pList );
    virtual void UserDraw( const UserDrawEvent& rUDEvt );

    void    Append( XGradientEntry* pEntry, Bitmap* pBmp = NULL );
    void    Modify( XGradientEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
    void    SelectEntryByList( const XGradientListRef &pList, const String& rStr,
                               const XGradient& rXGradient, sal_uInt16 nDist = 0 );
};

/************************************************************************/

class SVX_DLLPUBLIC BitmapLB : public ListBox
{
public:
    BitmapLB( Window* pParent, ResId Id, sal_Bool bUserDraw = sal_True );

    virtual void Fill( const XBitmapListRef &pList );
    virtual void UserDraw( const UserDrawEvent& rUDEvt );

    void    Append( XBitmapEntry* pEntry, Bitmap* pBmp = NULL );
    void    Modify( XBitmapEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );

private:
    VirtualDevice   aVD;
    Bitmap          aBitmap;

    XBitmapListRef  mpList;
    sal_Bool        mbUserDraw;

    SVX_DLLPRIVATE void SetVirtualDevice();
};

/************************************************************************/

class FillAttrLB : public ColorListBox
{
private:
    VirtualDevice   aVD;
    Bitmap          aBitmap;

    void SetVirtualDevice();

public:
         FillAttrLB( Window* pParent, WinBits aWB );

    virtual void Fill( const XColorListRef    &pList );
    virtual void Fill( const XHatchListRef    &pList );
    virtual void Fill( const XGradientListRef &pList );
    virtual void Fill( const XBitmapListRef   &pList );
};

/************************************************************************/

class FillTypeLB : public ListBox
{

public:
         FillTypeLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
         FillTypeLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    virtual void Fill();
};

/************************************************************************/

class SVX_DLLPUBLIC LineLB : public ListBox
{

public:
         LineLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
         LineLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    virtual void Fill( const XDashListRef &pList );

    void Append( XDashEntry* pEntry, Bitmap* pBmp = NULL );
    void Modify( XDashEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL );
    void FillStyles();
};

/************************************************************************/

class SVX_DLLPUBLIC LineEndLB : public ListBox
{

public:
         LineEndLB( Window* pParent, ResId Id ) : ListBox( pParent, Id ) {}
         LineEndLB( Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    virtual void Fill( const XLineEndListRef &pList, sal_Bool bStart = sal_True );

    void    Append( XLineEndEntry* pEntry, Bitmap* pBmp = NULL,
                    sal_Bool bStart = sal_True );
    void    Modify( XLineEndEntry* pEntry, sal_uInt16 nPos, Bitmap* pBmp = NULL,
                    sal_Bool bStart = sal_True );
};

//////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrModel;

class SvxPreviewBase : public Control
{
private:
    SdrModel*                                       mpModel;
    VirtualDevice*                                  mpBufferDevice;

protected:
    void InitSettings(bool bForeground, bool bBackground);

    // prepare buffered paint
    void LocalPrePaint();

    // end and output buffered paint
    void LocalPostPaint();

public:
    SvxPreviewBase( Window* pParent, const ResId& rResId );
    virtual ~SvxPreviewBase();

    // change support
    virtual void StateChanged(StateChangedType nStateChange);
    virtual void DataChanged(const DataChangedEvent& rDCEvt);

    // dada read access
    SdrModel& getModel() const { return *mpModel; }
    OutputDevice& getBufferDevice() const { return *mpBufferDevice; }
};

/*************************************************************************
|*
|* SvxLinePreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXLinePreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpLineObjA;
    SdrObject*                                      mpLineObjB;
    SdrObject*                                      mpLineObjC;

    Graphic*                                        mpGraphic;
    sal_Bool                                        mbWithSymbol;
    Size                                            maSymbolSize;

public:
    SvxXLinePreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXLinePreview();

    void SetLineAttributes(const SfxItemSet& rItemSet);

    void ShowSymbol( sal_Bool b ) { mbWithSymbol = b; };
    void SetSymbol( Graphic* p, const Size& s );
    void ResizeSymbol( const Size& s );

    virtual void Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXRectPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXRectPreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpRectangleObject;

public:
    SvxXRectPreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXRectPreview();

    void SetAttributes(const SfxItemSet& rItemSet);

    virtual void    Paint( const Rectangle& rRect );
};

/*************************************************************************
|*
|* SvxXShadowPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXShadowPreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpRectangleObject;
    SdrObject*                                      mpRectangleShadow;

public:
    SvxXShadowPreview( Window* pParent, const ResId& rResId );
    virtual ~SvxXShadowPreview();

    void SetRectangleAttributes(const SfxItemSet& rItemSet);
    void SetShadowAttributes(const SfxItemSet& rItemSet);
    void SetShadowPosition(const Point& rPos);

    virtual void    Paint( const Rectangle& rRect );
};

#endif // _SVX_DLG_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
