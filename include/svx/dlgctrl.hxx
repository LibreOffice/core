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
    SvxTabPage(Window* pParent, ResId Id, const SfxItemSet& rInAttrs)
        : SfxTabPage(pParent, Id, rInAttrs)
    {
    }
    SvxTabPage(Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet &rAttrSet)
        : SfxTabPage(pParent, rID, rUIXMLDescription, rAttrSet)
    {
    }
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
    SvxRectCtl( Window* pParent, RECT_POINT eRpt = RP_MM,
                sal_uInt16 nBorder = 200, sal_uInt16 nCircle = 80, CTL_STYLE eStyle = CS_RECT );
    void SetControlSettings(RECT_POINT eRpt = RP_MM, sal_uInt16 nBorder = 200,
        sal_uInt16 nCircle = 80, CTL_STYLE eStyle = CS_RECT);
    virtual ~SvxRectCtl();

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKeyEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        Resize();
    virtual Size        GetOptimalSize() const;

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
    void SetCS(CTL_STYLE eNew);
};

/*************************************************************************
|* Preview control for the display of bitmaps
\************************************************************************/

class SVX_DLLPUBLIC SvxBitmapCtl
{
protected:
    Size            aSize;
    sal_uInt16      nLines;
    Color           aPixelColor, aBackgroundColor;
    const sal_uInt16*   pBmpArray;

public:
            SvxBitmapCtl( Window* pParent, const Size& rSize );
            ~SvxBitmapCtl();

    BitmapEx GetBitmapEx();

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
    SvxPixelCtl( Window* pParent, sal_uInt16 nNumber = 8 );

    ~SvxPixelCtl();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );
    virtual void Resize();
    virtual Size GetOptimalSize() const;

    void    SetXBitmap( const BitmapEx& rBitmapEx );

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

    void Append( const XColorEntry& rEntry );
    void Modify( const XColorEntry& rEntry, sal_uInt16 nPos );
};

/************************************************************************/

class SVX_DLLPUBLIC HatchingLB : public ListBox
{
    XHatchListRef mpList;
public:
    explicit HatchingLB(Window* pParent, WinBits aWB);

    virtual void Fill( const XHatchListRef &pList );

    void    Append( const XHatchEntry& rEntry, const Bitmap& rBitmap );
    void    Modify( const XHatchEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
};

/************************************************************************/

class SVX_DLLPUBLIC GradientLB : public ListBox
{
    XGradientListRef mpList;
public:
    explicit GradientLB(Window* pParent, WinBits aWB);

    virtual void Fill( const XGradientListRef &pList );

    void    Append( const XGradientEntry& rEntry, const Bitmap& rBitmap );
    void    Modify( const XGradientEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
    void    SelectEntryByList( const XGradientListRef &pList, const String& rStr,
                               const XGradient& rXGradient, sal_uInt16 nDist = 0 );
};

/************************************************************************/

class SVX_DLLPUBLIC BitmapLB : public ListBox
{
public:
    explicit BitmapLB(Window* pParent, WinBits aWB);

    virtual void Fill(const XBitmapListRef &pList);

    void Append(const Size& rSize, const XBitmapEntry& rEntry);
    void Modify(const Size& rSize, const XBitmapEntry& rEntry, sal_uInt16 nPos);
    void SelectEntryByList(const XBitmapList* pList, const String& rStr);

private:
    BitmapEx        maBitmapEx;

    XBitmapListRef  mpList;
};

/************************************************************************/

class FillAttrLB : public ColorListBox
{
private:
    BitmapEx        maBitmapEx;

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
private:
    /// bitfield
    /// defines if standard fields (none, solid) are added, default is true
    bool        mbAddStandardFields : 1;

public:
    LineLB(Window* pParent, WinBits aWB);
    virtual ~LineLB();

    virtual void Fill(const XDashListRef &pList);
    bool getAddStandardFields() const { return mbAddStandardFields; }
    void setAddStandardFields(bool bNew);

    void Append(const XDashEntry& rEntry, const Bitmap& rBitmap );
    void Modify(const XDashEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap );
    void SelectEntryByList(const XDashList* pList, const String& rStr, const XDash& rDash, sal_uInt16 nDist = 0);
};


/************************************************************************/

class SVX_DLLPUBLIC LineEndLB : public ListBox
{

public:
    LineEndLB( Window* pParent, WinBits aWB );
    virtual ~LineEndLB (void);

    virtual void Fill( const XLineEndListRef &pList, bool bStart = true );

    void    Append( const XLineEndEntry& rEntry, const Bitmap& rBitmap, bool bStart = true );
    void    Modify( const XLineEndEntry& rEntry, sal_uInt16 nPos, const Bitmap& rBitmap, bool bStart = true );
};

//////////////////////////////////////////////////////////////////////////////

class SdrObject;
class SdrPathObj;
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
    SvxPreviewBase(Window* pParent);
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
    SdrPathObj*                                     mpLineObjA;
    SdrPathObj*                                     mpLineObjB;
    SdrPathObj*                                     mpLineObjC;

    Graphic*                                        mpGraphic;
    sal_Bool                                        mbWithSymbol;
    Size                                            maSymbolSize;

public:
    SvxXLinePreview( Window* pParent );
    virtual ~SvxXLinePreview();

    void SetLineAttributes(const SfxItemSet& rItemSet);

    void ShowSymbol( sal_Bool b ) { mbWithSymbol = b; };
    void SetSymbol( Graphic* p, const Size& s );
    void ResizeSymbol( const Size& s );

    virtual void Paint( const Rectangle& rRect );
    virtual void Resize();
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
    SvxXRectPreview(Window* pParent);
    virtual ~SvxXRectPreview();

    void SetAttributes(const SfxItemSet& rItemSet);

    virtual void    Paint( const Rectangle& rRect );
    virtual void Resize();
};

/*************************************************************************
|*
|* SvxXShadowPreview
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxXShadowPreview : public SvxPreviewBase
{
private:
    Point maShadowOffset;

    SdrObject* mpRectangleObject;
    SdrObject* mpRectangleShadow;

public:
    SvxXShadowPreview(Window *pParent);

    virtual ~SvxXShadowPreview();

    void SetRectangleAttributes(const SfxItemSet& rItemSet);
    void SetShadowAttributes(const SfxItemSet& rItemSet);
    void SetShadowPosition(const Point& rPos);

    virtual void    Paint( const Rectangle& rRect );
};

#endif // _SVX_DLG_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
