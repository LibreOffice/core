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
#ifndef INCLUDED_SVX_DLGCTRL_HXX
#define INCLUDED_SVX_DLGCTRL_HXX

#include <svtools/ctrlbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/svxdllapi.h>
#include <svx/rectenum.hxx>
#include <vcl/graph.hxx>
#include <svx/xtable.hxx>
#include <rtl/ref.hxx>
#include <o3tl/typed_flags_set.hxx>

class XOBitmap;
class XOutdevItemPool;

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
} } } }

/*************************************************************************
|* Derived from SfxTabPage for being able to get notified through the
|* virtual method from the control.
\************************************************************************/

class SAL_WARN_UNUSED SvxTabPage : public SfxTabPage
{

public:
    SvxTabPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet &rAttrSet)
        : SfxTabPage(pParent, rID, rUIXMLDescription, &rAttrSet)
    {
    }
    virtual void PointChanged( vcl::Window* pWindow, RectPoint eRP ) = 0;
};

/*************************************************************************
|* Control for display and selection of the corner and center points of
|* an object
\************************************************************************/

enum class CTL_STATE
{
    NONE     = 0,
    NOHORZ   = 1,       // no horizontal input information is used
    NOVERT   = 2,       // no vertical input information is used
};
namespace o3tl
{
    template<> struct typed_flags<CTL_STATE> : is_typed_flags<CTL_STATE, 0x03> {};
}

class SvxRectCtlAccessibleContext;
class SvxPixelCtlAccessible;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxRectCtl : public Control
{
private:
    SVX_DLLPRIVATE void             InitSettings(vcl::RenderContext& rRenderContext);
    SVX_DLLPRIVATE void             InitRectBitmap();
    SVX_DLLPRIVATE BitmapEx&        GetRectBitmap();
    SVX_DLLPRIVATE void             Resize_Impl();

protected:
    rtl::Reference<SvxRectCtlAccessibleContext> pAccContext;
    sal_uInt16 nBorderWidth;
    sal_uInt16 nRadius;
    Size aSize;
    Point aPtLT, aPtMT, aPtRT;
    Point aPtLM, aPtMM, aPtRM;
    Point aPtLB, aPtMB, aPtRB;
    Point aPtNew;
    RectPoint eRP, eDefRP;
    BitmapEx* pBitmap;
    CTL_STATE m_nState;

    bool mbCompleteDisable : 1;
    bool mbUpdateForeground : 1;
    bool mbUpdateBackground : 1;

    void MarkToResetSettings(bool bUpdateForeground, bool bUpdateBackground);

    RectPoint          GetRPFromPoint( Point, bool bRTL = false ) const;
    const Point&        GetPointFromRP( RectPoint ) const;
    void                SetFocusRect();
    Point               SetActualRPWithoutInvalidate( RectPoint eNewRP );  // returns the last point

    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;

    Point               GetApproxLogPtFromPixPt( const Point& rRoughPixelPoint ) const;
public:
    SvxRectCtl( vcl::Window* pParent, RectPoint eRpt = RectPoint::MM,
                sal_uInt16 nBorder = 200, sal_uInt16 nCircle = 80 );
    void SetControlSettings(RectPoint eRpt, sal_uInt16 nBorder, sal_uInt16 nCircl);
    virtual ~SvxRectCtl() override;
    virtual void dispose() override;

    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKeyEvt ) override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        Resize() override;
    virtual Size        GetOptimalSize() const override;

    void                Reset();
    RectPoint           GetActualRP() const { return eRP;}
    void                SetActualRP( RectPoint eNewRP );

    void                SetState( CTL_STATE nState );

    static const sal_uInt8 NO_CHILDREN = 9;   // returns number of usable radio buttons

    tools::Rectangle           CalculateFocusRectangle() const;
    tools::Rectangle           CalculateFocusRectangle( RectPoint eRectPoint ) const;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    RectPoint          GetApproxRPFromPixPt( const css::awt::Point& rPixelPoint ) const;

    bool IsCompletelyDisabled() const { return mbCompleteDisable; }
    void DoCompletelyDisable(bool bNew);
};

/*************************************************************************
|* Control for editing bitmaps
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxPixelCtl final : public Control
{
private:
    sal_uInt16      nLines, nSquares;
    Color       aPixelColor;
    Color       aBackgroundColor;
    Size        aRectSize;
    sal_uInt16* pPixel;
    bool        bPaintable;
    //Add member identifying position
    Point       aFocusPosition;
    rtl::Reference<SvxPixelCtlAccessible>  m_xAccess;

    using OutputDevice::SetLineColor;

    tools::Rectangle   implCalFocusRect( const Point& aPosition );
    void    ChangePixel( sal_uInt16 nPixel );

public:
    SvxPixelCtl( vcl::Window* pParent, sal_uInt16 nNumber = 8 );

    virtual ~SvxPixelCtl() override;
    virtual void dispose() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;

    void    SetXBitmap( const BitmapEx& rBitmapEx );

    void    SetPixelColor( const Color& rCol ) { aPixelColor = rCol; }
    void    SetBackgroundColor( const Color& rCol ) { aBackgroundColor = rCol; }

    sal_uInt16  GetLineCount() const { return nLines; }

    sal_uInt16  GetBitmapPixel( const sal_uInt16 nPixelNumber );
    sal_uInt16* GetBitmapPixelPtr() { return pPixel; }

    void    SetPaintable( bool bTmp ) { bPaintable = bTmp; }
    void    Reset();
    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
    long GetSquares() const { return nSquares ; }
    long GetWidth() const { return aRectSize.getWidth() ; }
    long GetHeight() const { return aRectSize.getHeight() ; }

    //Device Pixel .
    long ShowPosition( const Point &pt);

    long PointToIndex(const Point &pt) const;
    Point IndexToPoint(long nIndex) const ;
    long GetFocusPosIndex() const ;
    //Keyboard function for key input and focus handling function
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC HatchingLB : public ListBox
{
public:
    explicit HatchingLB(vcl::Window* pParent, WinBits aWB);
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC GradientLB : public ListBox
{
public:
    explicit GradientLB(vcl::Window* pParent, WinBits aWB);
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC BitmapLB : public ListBox
{
public:
    explicit BitmapLB(vcl::Window* pParent, WinBits aWB);
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC FillTypeLB : public ListBox
{

public:
         FillTypeLB( vcl::Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    void Fill();
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC LineLB : public ListBox
{
private:
    /// defines if standard fields (none, solid) are added, default is true
    bool        mbAddStandardFields : 1;

public:
    LineLB(vcl::Window* pParent, WinBits aWB);

    void Fill(const XDashListRef &pList);
    bool getAddStandardFields() const { return mbAddStandardFields; }
    void setAddStandardFields(bool bNew);

    void Append(const XDashEntry& rEntry, const Bitmap& rBitmap );
    void Modify(const XDashEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap );
};


/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC LineEndLB : public ListBox
{

public:
    LineEndLB( vcl::Window* pParent, WinBits aWB );

    void Fill( const XLineEndListRef &pList, bool bStart = true );

    void    Append( const XLineEndEntry& rEntry, const Bitmap& rBitmap );
    void    Modify( const XLineEndEntry& rEntry, sal_Int32 nPos, const Bitmap& rBitmap );
};


class SdrObject;
class SdrPathObj;
class SdrModel;

class SAL_WARN_UNUSED SAL_DLLPUBLIC_RTTI SvxPreviewBase : public Control
{
private:
    SdrModel*             mpModel;
    VclPtr<VirtualDevice> mpBufferDevice;

protected:
    void InitSettings(bool bForeground, bool bBackground);

    // prepare buffered paint
    void LocalPrePaint(vcl::RenderContext const & rRenderContext);

    // end and output buffered paint
    void LocalPostPaint(vcl::RenderContext& rRenderContext);

public:
    SvxPreviewBase(vcl::Window* pParent);
    virtual ~SvxPreviewBase() override;
    virtual void dispose() override;

    // change support
    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    // dada read access
    SdrModel& getModel() const
    {
        return *mpModel;
    }
    OutputDevice& getBufferDevice() const
    {
        return *mpBufferDevice;
    }
};

/*************************************************************************
|*
|* SvxLinePreview
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxXLinePreview : public SvxPreviewBase
{
private:
    SdrPathObj*                                     mpLineObjA;
    SdrPathObj*                                     mpLineObjB;
    SdrPathObj*                                     mpLineObjC;

    Graphic*                                        mpGraphic;
    bool                                            mbWithSymbol;
    Size                                            maSymbolSize;

public:
    SvxXLinePreview( vcl::Window* pParent );
    virtual ~SvxXLinePreview() override;
    virtual void dispose() override;

    void SetLineAttributes(const SfxItemSet& rItemSet);

    void ShowSymbol( bool b ) { mbWithSymbol = b; };
    void SetSymbol( Graphic* p, const Size& s );
    void ResizeSymbol( const Size& s );

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void Resize() override;
    virtual Size GetOptimalSize() const override;
};

/*************************************************************************
|*
|* SvxXRectPreview
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxXRectPreview : public SvxPreviewBase
{
private:
    SdrObject*                                      mpRectangleObject;

public:
    SvxXRectPreview(vcl::Window* pParent);
    virtual ~SvxXRectPreview() override;
    virtual void dispose() override;

    void SetAttributes(const SfxItemSet& rItemSet);

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void Resize() override;
};

/*************************************************************************
|*
|* SvxXShadowPreview
|*
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxXShadowPreview : public SvxPreviewBase
{
private:
    Point maShadowOffset;

    SdrObject* mpRectangleObject;
    SdrObject* mpRectangleShadow;

public:
    SvxXShadowPreview(vcl::Window *pParent);

    virtual ~SvxXShadowPreview() override;
    virtual void dispose() override;

    void SetRectangleAttributes(const SfxItemSet& rItemSet);
    void SetShadowAttributes(const SfxItemSet& rItemSet);
    void SetShadowPosition(const Point& rPos);

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
};

#endif // INCLUDED_SVX_DLGCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
