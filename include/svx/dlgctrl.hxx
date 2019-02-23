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
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>
#include <svx/xtable.hxx>
#include <rtl/ref.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include <array>

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
    SvxTabPage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet &rAttrSet)
        : SfxTabPage(pParent, rUIXMLDescription, rID, &rAttrSet)
    {
    }
    virtual void PointChanged(weld::DrawingArea* pArea, RectPoint eRP) = 0;
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

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxRectCtl : public weld::CustomWidgetController
{
private:
    VclPtr<SvxTabPage> m_pPage;

    SVX_DLLPRIVATE static void      InitSettings(vcl::RenderContext& rRenderContext);
    SVX_DLLPRIVATE void             InitRectBitmap();
    SVX_DLLPRIVATE BitmapEx&        GetRectBitmap();
    SVX_DLLPRIVATE void             Resize_Impl(const Size& rSize);

    SvxRectCtl(const SvxRectCtl&) = delete;
    SvxRectCtl& operator=(const SvxRectCtl&) = delete;

protected:
    rtl::Reference<SvxRectCtlAccessibleContext> pAccContext;
    sal_uInt16 nBorderWidth;
    Point aPtLT, aPtMT, aPtRT;
    Point aPtLM, aPtMM, aPtRM;
    Point aPtLB, aPtMB, aPtRB;
    Point aPtNew;
    RectPoint eRP, eDefRP;
    std::unique_ptr<BitmapEx> pBitmap;
    CTL_STATE m_nState;

    bool mbCompleteDisable : 1;

    RectPoint           GetRPFromPoint( Point, bool bRTL = false ) const;
    const Point&        GetPointFromRP( RectPoint ) const;
    Point               SetActualRPWithoutInvalidate( RectPoint eNewRP );  // returns the last point

    Point               GetApproxLogPtFromPixPt( const Point& rRoughPixelPoint ) const;
public:
    SvxRectCtl(SvxTabPage* pPage, RectPoint eRpt = RectPoint::MM, sal_uInt16 nBorder = 200);
    void SetControlSettings(RectPoint eRpt, sal_uInt16 nBorder);
    virtual ~SvxRectCtl() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent&) override;
    virtual bool KeyInput(const KeyEvent&) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;
    virtual tools::Rectangle GetFocusRect() override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void StyleUpdated() override;

    void                Reset();
    RectPoint           GetActualRP() const { return eRP;}
    void                SetActualRP( RectPoint eNewRP );

    void                SetState( CTL_STATE nState );

    static const sal_uInt8 NO_CHILDREN = 9;   // returns number of usable radio buttons

    tools::Rectangle           CalculateFocusRectangle() const;
    tools::Rectangle           CalculateFocusRectangle( RectPoint eRectPoint ) const;

    css::uno::Reference<css::accessibility::XAccessible> getAccessibleParent() { return GetDrawingArea()->get_accessible_parent(); }
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
    a11yrelationset get_accessible_relation_set() { return GetDrawingArea()->get_accessible_relation_set(); }

    RectPoint          GetApproxRPFromPixPt( const css::awt::Point& rPixelPoint ) const;

    bool IsCompletelyDisabled() const { return mbCompleteDisable; }
    void DoCompletelyDisable(bool bNew);
};

/*************************************************************************
|* Control for editing bitmaps
\************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxPixelCtl final : public weld::CustomWidgetController
{
private:
    static sal_uInt16 constexpr nLines = 8;
    static sal_uInt16 constexpr nSquares = nLines * nLines;

    VclPtr<SvxTabPage> m_pPage;

    Color       aPixelColor;
    Color       aBackgroundColor;
    Size        aRectSize;
    std::array<sal_uInt8,nSquares> maPixelData;
    bool        bPaintable;
    //Add member identifying position
    Point       aFocusPosition;
    rtl::Reference<SvxPixelCtlAccessible>  m_xAccess;

    tools::Rectangle   implCalFocusRect( const Point& aPosition );
    void    ChangePixel( sal_uInt16 nPixel );

    SvxPixelCtl(SvxPixelCtl const&) = delete;
    SvxPixelCtl(SvxPixelCtl&&) = delete;
    SvxPixelCtl& operator=(SvxPixelCtl const&) = delete;
    SvxPixelCtl& operator=(SvxPixelCtl&&) = delete;

public:
    SvxPixelCtl(SvxTabPage* pPage);

    virtual ~SvxPixelCtl() override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void Resize() override;
    virtual tools::Rectangle GetFocusRect() override;

    void    SetXBitmap( const BitmapEx& rBitmapEx );

    void    SetPixelColor( const Color& rCol ) { aPixelColor = rCol; }
    void    SetBackgroundColor( const Color& rCol ) { aBackgroundColor = rCol; }

    static sal_uInt16 GetLineCount() { return nLines; }

    sal_uInt8  GetBitmapPixel( const sal_uInt16 nPixelNumber ) const;
    std::array<sal_uInt8,64> const & GetBitmapPixelPtr() const { return maPixelData; }

    void    SetPaintable( bool bTmp ) { bPaintable = bTmp; }
    void    Reset();

    css::uno::Reference<css::accessibility::XAccessible> getAccessibleParent() { return GetDrawingArea()->get_accessible_parent(); }
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;
    a11yrelationset get_accessible_relation_set() { return GetDrawingArea()->get_accessible_relation_set(); }

    static long GetSquares() { return nSquares ; }
    long GetWidth() const { return aRectSize.getWidth() ; }
    long GetHeight() const { return aRectSize.getHeight() ; }

    //Device Pixel .
    long ShowPosition( const Point &pt);

    long PointToIndex(const Point &pt) const;
    Point IndexToPoint(long nIndex) const ;
    long GetFocusPosIndex() const ;
    //Keyboard function for key input and focus handling function
    virtual bool        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC FillTypeLB : public ListBox
{

public:
         FillTypeLB( vcl::Window* pParent, WinBits aWB ) : ListBox( pParent, aWB ) {}

    void Fill();
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxLineLB
{
private:
    std::unique_ptr<weld::ComboBox> m_xControl;

    /// defines if standard fields (none, solid) are added, default is true
    bool        mbAddStandardFields : 1;

public:
    SvxLineLB(std::unique_ptr<weld::ComboBox> pControl);

    void Fill(const XDashListRef &pList);
    bool getAddStandardFields() const { return mbAddStandardFields; }
    void setAddStandardFields(bool bNew);

    void Append(const XDashEntry& rEntry, const BitmapEx& rBitmap );
    void Modify(const XDashEntry& rEntry, sal_Int32 nPos, const BitmapEx& rBitmap );

    void clear() { m_xControl->clear(); }
    void remove(int nPos) { m_xControl->remove(nPos); }
    int get_active() const { return m_xControl->get_active(); }
    void set_active(int nPos) { m_xControl->set_active(nPos); }
    void set_active_text(const OUString& rStr) { m_xControl->set_active_text(rStr); }
    OUString get_active_text() const { return m_xControl->get_active_text(); }
    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_xControl->connect_changed(rLink); }
    int get_count() const { return m_xControl->get_count(); }
    void append_text(const OUString& rStr) { m_xControl->append_text(rStr); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    void save_value() { m_xControl->save_value(); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    bool get_sensitive() const { return m_xControl->get_sensitive(); }
};

/************************************************************************/

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxLineEndLB
{
private:
    std::unique_ptr<weld::ComboBox> m_xControl;

public:
    SvxLineEndLB(std::unique_ptr<weld::ComboBox> pControl);

    void Fill( const XLineEndListRef &pList, bool bStart = true );

    void    Append( const XLineEndEntry& rEntry, const BitmapEx& rBitmap );
    void    Modify( const XLineEndEntry& rEntry, sal_Int32 nPos, const BitmapEx& rBitmap );

    void clear() { m_xControl->clear(); }
    void remove(int nPos) { m_xControl->remove(nPos); }
    int get_active() const { return m_xControl->get_active(); }
    void set_active(int nPos) { m_xControl->set_active(nPos); }
    void set_active_text(const OUString& rStr) { m_xControl->set_active_text(rStr); }
    OUString get_active_text() const { return m_xControl->get_active_text(); }
    void connect_changed(const Link<weld::ComboBox&, void>& rLink) { m_xControl->connect_changed(rLink); }
    int get_count() const { return m_xControl->get_count(); }
    void append_text(const OUString& rStr) { m_xControl->append_text(rStr); }
    bool get_value_changed_from_saved() const { return m_xControl->get_value_changed_from_saved(); }
    void save_value() { m_xControl->save_value(); }
    void set_sensitive(bool bSensitive) { m_xControl->set_sensitive(bSensitive); }
    bool get_sensitive() const { return m_xControl->get_sensitive(); }
};

class SdrObject;
class SdrPathObj;
class SdrModel;

class SAL_WARN_UNUSED SAL_DLLPUBLIC_RTTI SvxPreviewBase : public weld::CustomWidgetController
{
private:
    std::unique_ptr<SdrModel> mpModel;
    VclPtr<VirtualDevice> mpBufferDevice;

protected:
    void InitSettings();

    tools::Rectangle GetPreviewSize() const;

    // prepare buffered paint
    void LocalPrePaint(vcl::RenderContext const & rRenderContext);

    // end and output buffered paint
    void LocalPostPaint(vcl::RenderContext& rRenderContext);

public:
    SvxPreviewBase();
    virtual void SetDrawingArea(weld::DrawingArea*) override;
    virtual ~SvxPreviewBase() override;

    // change support
    virtual void StyleUpdated() override;

    void SetDrawMode(DrawModeFlags nDrawMode)
    {
        mpBufferDevice->SetDrawMode(nDrawMode);
    }

    Size GetOutputSize() const
    {
        return mpBufferDevice->PixelToLogic(GetOutputSizePixel());
    }

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
    SvxXLinePreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SvxXLinePreview() override;

    void SetLineAttributes(const SfxItemSet& rItemSet);

    void ShowSymbol( bool b ) { mbWithSymbol = b; };
    void SetSymbol( Graphic* p, const Size& s );
    void ResizeSymbol( const Size& s );

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void Resize() override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxXRectPreview : public SvxPreviewBase
{
private:
    SdrObject* mpRectangleObject;

public:
    SvxXRectPreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SvxXRectPreview() override;

    void SetAttributes(const SfxItemSet& rItemSet);

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
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
    SvxXShadowPreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~SvxXShadowPreview() override;

    void SetRectangleAttributes(const SfxItemSet& rItemSet);
    void SetShadowAttributes(const SfxItemSet& rItemSet);
    void SetShadowPosition(const Point& rPos);

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
};

#endif // INCLUDED_SVX_DLGCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
