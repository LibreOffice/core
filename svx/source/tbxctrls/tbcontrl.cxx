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

#include <utility>

#include <comphelper/configurationlistener.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <tools/color.hxx>
#include <svl/numformat.hxx>
#include <svl/poolitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/customweld.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/weldutils.hxx>
#include <svtools/valueset.hxx>
#include <svtools/ctrlbox.hxx>
#include <svl/style.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/borderhelper.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/sfxstatuslistener.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <helpids.h>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svx/xtable.hxx>
#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/cmapitem.hxx>
#include <svx/colorwindow.hxx>
#include <svx/colorbox.hxx>
#include <svx/tbcontrl.hxx>
#include <svx/dialmgr.hxx>
#include <svx/PaletteManager.hxx>
#include <memory>

#include <tbxcolorupdate.hxx>
#include <editeng/eerdll.hxx>
#include <editeng/editrids.hrc>
#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svl/currencytable.hxx>
#include <svtools/langtab.hxx>
#include <cppu/unotype.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <officecfg/Office/Common.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <bitmaps.hlst>
#include <sal/log.hxx>
#include <unotools/collatorwrapper.hxx>

#include <comphelper/lok.hxx>
#include <tools/json_writer.hxx>

#define MAX_MRU_FONTNAME_ENTRIES    5

#define COMBO_WIDTH_IN_CHARS        18

// namespaces
using namespace ::editeng;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace
{
class SvxStyleBox_Base
{
public:
    SvxStyleBox_Base(std::unique_ptr<weld::ComboBox> xWidget, const OUString& rCommand, SfxStyleFamily eFamily,
                     const Reference<XDispatchProvider>& rDispatchProvider,
                     const Reference<XFrame>& _xFrame,const OUString& rClearFormatKey,
                     const OUString& rMoreKey, bool bInSpecialMode, SvxStyleToolBoxControl& rCtrl);

    virtual ~SvxStyleBox_Base()
    {
    }

    void            SetFamily( SfxStyleFamily eNewFamily );

    void            SetDefaultStyle( const OUString& rDefault ) { sDefaultStyle = rDefault; }

    int get_count() const { return m_xWidget->get_count(); }
    OUString get_text(int nIndex) const { return m_xWidget->get_text(nIndex); }
    OUString get_active_text() const { return m_xWidget->get_active_text(); }

    void append_text(const OUString& rStr)
    {
        OUString sId(OUString::number(m_xWidget->get_count()));
        m_xWidget->append(sId, rStr);
    }

    void insert_separator(int pos, const OUString& rId)
    {
        m_xWidget->insert_separator(pos, rId);
    }

    void set_active_or_entry_text(const OUString& rText)
    {
        const int nFound = m_xWidget->find_text(rText);
        if (nFound != -1)
            m_xWidget->set_active(nFound);
        else
            m_xWidget->set_entry_text(rText);
    }

    void set_active(int nActive)
    {
        m_xWidget->set_active(nActive);
    }

    void freeze()
    {
        m_xWidget->freeze();
    }

    void save_value()
    {
        m_xWidget->save_value();
    }

    void clear()
    {
        m_xWidget->clear();
        m_nMaxUserDrawFontWidth = 0;
    }

    void thaw()
    {
        m_xWidget->thaw();
    }

    virtual bool DoKeyInput(const KeyEvent& rKEvt);

private:
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(DumpAsPropertyTreeHdl, tools::JsonWriter&, void);
    DECL_LINK(CustomRenderHdl, weld::ComboBox::render_args, void);
    DECL_LINK(CustomGetSizeHdl, OutputDevice&, Size);

    /// Calculate the optimal width of the dropdown.  Very expensive operation, triggers lots of font measurement.
    void CalcOptimalExtraUserWidth(vcl::RenderContext& rRenderContext);

    void Select(bool bNonTravelSelect);

protected:
    SvxStyleToolBoxControl& m_rCtrl;

    std::unique_ptr<weld::Builder>  m_xMenuBuilder;
    std::unique_ptr<weld::Menu>     m_xMenu;
    std::unique_ptr<weld::ComboBox> m_xWidget;

    SfxStyleFamily                  eStyleFamily;
    int                             m_nMaxUserDrawFontWidth;
    int                             m_nLastItemWithMenu;
    bool                            bRelease;
    Reference< XDispatchProvider >  m_xDispatchProvider;
    Reference< XFrame >             m_xFrame;
    OUString                        m_aCommand;
    OUString                        aClearFormatKey;
    OUString                        aMoreKey;
    OUString                        sDefaultStyle;
    bool                            bInSpecialMode;

    void            ReleaseFocus();
    static Color    TestColorsVisible(const Color &FontCol, const Color &BackCol);
    static void     UserDrawEntry(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const OUString &rStyleName);
    void            SetupEntry(vcl::RenderContext& rRenderContext, sal_Int32 nItem, const tools::Rectangle& rRect, std::u16string_view rStyleName, bool bIsNotSelected);
    static bool     AdjustFontForItemHeight(OutputDevice& rDevice, tools::Rectangle const & rTextRect, tools::Long nHeight);
    DECL_LINK(MenuSelectHdl, const OString&, void);
    DECL_STATIC_LINK(SvxStyleBox_Base, ShowMoreHdl, void*, void);
};

class SvxStyleBox_Impl final : public InterimItemWindow
                             , public SvxStyleBox_Base
{
public:
    SvxStyleBox_Impl(vcl::Window* pParent, const OUString& rCommand, SfxStyleFamily eFamily, const Reference< XDispatchProvider >& rDispatchProvider,
                     const Reference< XFrame >& _xFrame,const OUString& rClearFormatKey, const OUString& rMoreKey, bool bInSpecialMode, SvxStyleToolBoxControl& rCtrl);

    virtual ~SvxStyleBox_Impl() override
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        m_xMenu.reset();
        m_xMenuBuilder.reset();
        InterimItemWindow::dispose();
    }

    virtual bool DoKeyInput(const KeyEvent& rKEvt) override;

private:

    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;
    void  SetOptimalSize();
};

class SvxFontNameBox_Impl;
class SvxFontNameBox_Base;

class SvxFontNameToolBoxControl final : public cppu::ImplInheritanceHelper<svt::ToolboxController,
                                                                           css::lang::XServiceInfo>
{
public:
    SvxFontNameToolBoxControl();

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XToolbarController
    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    VclPtr<SvxFontNameBox_Impl> m_xVclBox;
    std::unique_ptr<SvxFontNameBox_Base> m_xWeldBox;
    SvxFontNameBox_Base* m_pBox;
};

class FontOptionsListener final : public comphelper::ConfigurationListenerProperty<bool>
{
private:
    SvxFontNameBox_Base& m_rBox;

    virtual void setProperty(const css::uno::Any &rProperty) override;
public:
    FontOptionsListener(const rtl::Reference<comphelper::ConfigurationListener>& rListener, const OUString& rProp, SvxFontNameBox_Base& rBox)
        : comphelper::ConfigurationListenerProperty<bool>(rListener, rProp)
        , m_rBox(rBox)
    {
    }
};

class SvxFontNameBox_Base
{
private:
    rtl::Reference<comphelper::ConfigurationListener> m_xListener;
    FontOptionsListener m_aWYSIWYG;
    FontOptionsListener m_aHistory;

protected:
    SvxFontNameToolBoxControl& m_rCtrl;

    std::unique_ptr<FontNameBox>   m_xWidget;
    const FontList*                pFontList;
    ::std::unique_ptr<FontList>    m_aOwnFontList;
    vcl::Font                      aCurFont;
    sal_uInt16                     nFtCount;
    bool                           bRelease;
    Reference< XDispatchProvider > m_xDispatchProvider;
    Reference< XFrame >            m_xFrame;
    bool            mbCheckingUnknownFont;

    void            ReleaseFocus_Impl();

    void            Select(bool bNonTravelSelect);

    void            EndPreview()
    {
        Sequence< PropertyValue > aArgs;
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharEndPreviewFontName",
                                         aArgs );
    }
    void            CheckAndMarkUnknownFont();

public:
    SvxFontNameBox_Base(std::unique_ptr<weld::ComboBox> xWidget, const Reference<XDispatchProvider>& rDispatchProvider,
                        const Reference<XFrame>& rFrame, SvxFontNameToolBoxControl& rCtrl);
    virtual ~SvxFontNameBox_Base()
    {
        m_xListener->dispose();
    }

    void            FillList();
    void            Update( const css::awt::FontDescriptor* pFontDesc );
    sal_uInt16      GetListCount() const { return nFtCount; }
    void            Clear() { m_xWidget->clear(); nFtCount = 0; }
    void            Fill( const FontList* pList )
    {
        m_xWidget->Fill(pList);
        nFtCount = pList->GetFontNameCount();
    }

    void SetOwnFontList(::std::unique_ptr<FontList> && _aOwnFontList) { m_aOwnFontList = std::move(_aOwnFontList); }

    virtual void set_sensitive(bool bSensitive)
    {
        m_xWidget->set_sensitive(bSensitive);
    }

    void set_active_or_entry_text(const OUString& rText);

    void statusChanged_Impl(const css::frame::FeatureStateEvent& rEvent);

    virtual bool DoKeyInput(const KeyEvent& rKEvt);

    void EnableControls();

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(DumpAsPropertyTreeHdl, tools::JsonWriter&, void);
};

void FontOptionsListener::setProperty(const css::uno::Any &rProperty)
{
    comphelper::ConfigurationListenerProperty<bool>::setProperty(rProperty);
    m_rBox.EnableControls();
}

class SvxFontNameBox_Impl final : public InterimItemWindow
                                , public SvxFontNameBox_Base
{
private:
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    void            SetOptimalSize();

    virtual bool DoKeyInput(const KeyEvent& rKEvt) override;

public:
    SvxFontNameBox_Impl(vcl::Window* pParent, const Reference<XDispatchProvider>& rDispatchProvider,
                        const Reference<XFrame>& rFrame, SvxFontNameToolBoxControl& rCtrl);

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual ~SvxFontNameBox_Impl() override
    {
        disposeOnce();
    }

    virtual Reference< css::accessibility::XAccessible > CreateAccessible() override;

    virtual void set_sensitive(bool bSensitive) override
    {
        m_xWidget->set_sensitive(bSensitive);
        if (bSensitive)
            InterimItemWindow::Enable();
        else
            InterimItemWindow::Disable();
    }
};


// SelectHdl needs the Modifiers, get them in MouseButtonUp
class SvxFrmValueSet_Impl final : public ValueSet
{
private:
    sal_uInt16 nModifier;

    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override
    {
        nModifier = rMEvt.GetModifier();
        return ValueSet::MouseButtonUp(rMEvt);
    }

public:
    SvxFrmValueSet_Impl()
        : ValueSet(nullptr)
        , nModifier(0)
    {
    }
    sal_uInt16 GetModifier() const {return nModifier;}
};

}

namespace {

class SvxFrameToolBoxControl;

class SvxFrameWindow_Impl final : public WeldToolbarPopup
{
private:
    rtl::Reference<SvxFrameToolBoxControl> mxControl;
    std::unique_ptr<SvxFrmValueSet_Impl> mxFrameSet;
    std::unique_ptr<weld::CustomWeld> mxFrameSetWin;
    std::vector<std::pair<BitmapEx, OUString>> aImgVec;
    bool                        bParagraphMode;

    void InitImageList();
    void CalcSizeValueSet();
    DECL_LINK( SelectHdl, ValueSet*, void );

    void SetDiagonalDownBorder(const SvxLineItem& dDownLineItem);
    void SetDiagonalUpBorder(const SvxLineItem& dUpLineItem);

public:
    SvxFrameWindow_Impl(SvxFrameToolBoxControl* pControl, weld::Widget* pParent);
    virtual void GrabFocus() override
    {
        mxFrameSet->GrabFocus();
    }

    virtual void    statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

class SvxFrameToolBoxControl : public svt::PopupWindowController
{
public:
    explicit SvxFrameToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL execute(sal_Int16 nKeyModifier) override;
private:
    virtual std::unique_ptr<WeldToolbarPopup> weldPopupWindow() override;
    virtual VclPtr<vcl::Window> createVclPopupWindow( vcl::Window* pParent ) override;
};

    class LineListBox final : public ValueSet
    {
    public:
        typedef Color (*ColorFunc)(Color);
        typedef Color (*ColorDistFunc)(Color, Color);

        LineListBox();

        /** Set the width in Twips */
        Size SetWidth( tools::Long nWidth )
        {
            tools::Long nOldWidth = m_nWidth;
            m_nWidth = nWidth;
            return UpdateEntries( nOldWidth );
        }

        void SetNone( const OUString& sNone )
        {
            m_sNone = sNone;
        }

        /** Insert a listbox entry with all widths in Twips. */
        void            InsertEntry(const BorderWidthImpl& rWidthImpl,
                            SvxBorderLineStyle nStyle, tools::Long nMinWidth = 0,
                            ColorFunc pColor1Fn = &sameColor,
                            ColorFunc pColor2Fn = &sameColor,
                            ColorDistFunc pColorDistFn = &sameDistColor);

        SvxBorderLineStyle GetEntryStyle( sal_Int32 nPos ) const;

        SvxBorderLineStyle GetSelectEntryStyle() const;

        void            SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }

        const Color&    GetColor() const { return aColor; }

        virtual void    SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    private:

        void         ImpGetLine(tools::Long nLine1, tools::Long nLine2, tools::Long nDistance,
                                Color nColor1, Color nColor2, Color nColorDist,
                                SvxBorderLineStyle nStyle, BitmapEx& rBmp);

        void            UpdatePaintLineColor();       // returns sal_True if maPaintCol has changed

        Size            UpdateEntries( tools::Long nOldWidth );
        sal_Int32       GetStylePos( sal_Int32  nListPos, tools::Long nWidth );

        const Color& GetPaintColor() const
        {
            return maPaintCol;
        }

        Color   GetColorLine1( sal_Int32  nPos );
        Color   GetColorLine2( sal_Int32  nPos );
        Color   GetColorDist( sal_Int32  nPos );

                        LineListBox( const LineListBox& ) = delete;
        LineListBox&    operator =( const LineListBox& ) = delete;

        std::vector<std::unique_ptr<ImpLineListData>> m_vLineList;
        tools::Long            m_nWidth;
        OUString        m_sNone;
        ScopedVclPtr<VirtualDevice>   aVirDev;
        Size            aTxtSize;
        Color const     aColor;
        Color           maPaintCol;
        FieldUnit       eSourceUnit;
    };

    SvxBorderLineStyle LineListBox::GetSelectEntryStyle() const
    {
        SvxBorderLineStyle nStyle = SvxBorderLineStyle::SOLID;
        size_t nPos = GetSelectItemPos();
        if (nPos != VALUESET_ITEM_NOTFOUND)
        {
            if (!m_sNone.isEmpty())
                --nPos;
            nStyle = GetEntryStyle( nPos );
        }

        return nStyle;
    }

    void LineListBox::ImpGetLine( tools::Long nLine1, tools::Long nLine2, tools::Long nDistance,
                                Color aColor1, Color aColor2, Color aColorDist,
                                SvxBorderLineStyle nStyle, BitmapEx& rBmp )
    {
        auto nMinWidth = GetDrawingArea()->get_ref_device().approximate_digit_width() * COMBO_WIDTH_IN_CHARS;
        Size aSize(nMinWidth, aTxtSize.Height());
        aSize.AdjustWidth( -(aTxtSize.Width()) );
        aSize.AdjustWidth( -6 );

        // SourceUnit to Twips
        if ( eSourceUnit == FieldUnit::POINT )
        {
            nLine1      /= 5;
            nLine2      /= 5;
            nDistance   /= 5;
        }

        // Paint the lines
        aSize = aVirDev->PixelToLogic( aSize );
        tools::Long nPix = aVirDev->PixelToLogic( Size( 0, 1 ) ).Height();
        sal_uInt32 n1 = nLine1;
        sal_uInt32 n2 = nLine2;
        tools::Long nDist  = nDistance;
        n1 += nPix-1;
        n1 -= n1%nPix;
        if ( n2 )
        {
            nDist += nPix-1;
            nDist -= nDist%nPix;
            n2    += nPix-1;
            n2    -= n2%nPix;
        }
        tools::Long nVirHeight = n1+nDist+n2;
        if ( nVirHeight > aSize.Height() )
            aSize.setHeight( nVirHeight );
        // negative width should not be drawn
        if ( aSize.Width() <= 0 )
            return;

        Size aVirSize = aVirDev->LogicToPixel( aSize );
        if ( aVirDev->GetOutputSizePixel() != aVirSize )
            aVirDev->SetOutputSizePixel( aVirSize );
        aVirDev->SetFillColor( aColorDist );
        aVirDev->DrawRect( tools::Rectangle( Point(), aSize ) );

        aVirDev->SetFillColor( aColor1 );

        double y1 = double( n1 ) / 2;
        svtools::DrawLine( *aVirDev, basegfx::B2DPoint( 0, y1 ), basegfx::B2DPoint( aSize.Width( ), y1 ), n1, nStyle );

        if ( n2 )
        {
            double y2 =  n1 + nDist + double( n2 ) / 2;
            aVirDev->SetFillColor( aColor2 );
            svtools::DrawLine( *aVirDev, basegfx::B2DPoint( 0, y2 ), basegfx::B2DPoint( aSize.Width(), y2 ), n2, SvxBorderLineStyle::SOLID );
        }
        rBmp = aVirDev->GetBitmapEx( Point(), Size( aSize.Width(), n1+nDist+n2 ) );
    }

    LineListBox::LineListBox()
        : ValueSet(nullptr)
        , m_nWidth( 5 )
        , aVirDev(VclPtr<VirtualDevice>::Create())
        , aColor(Application::GetSettings().GetStyleSettings().GetWindowTextColor())
        , maPaintCol(COL_BLACK)
        , eSourceUnit(FieldUnit::POINT)
    {
        aVirDev->SetLineColor();
        aVirDev->SetMapMode( MapMode( MapUnit::MapTwip ) );
    }

    void LineListBox::SetDrawingArea(weld::DrawingArea* pDrawingArea)
    {
        ValueSet::SetDrawingArea(pDrawingArea);

        OutputDevice& rDevice = pDrawingArea->get_ref_device();

        aTxtSize.setWidth( rDevice.approximate_digit_width() );
        aTxtSize.setHeight( rDevice.GetTextHeight() );

        UpdatePaintLineColor();
    }

    sal_Int32 LineListBox::GetStylePos( sal_Int32 nListPos, tools::Long nWidth )
    {
        sal_Int32 nPos = -1;
        if (!m_sNone.isEmpty())
            nListPos--;

        sal_Int32 n = 0;
        size_t i = 0;
        size_t nCount = m_vLineList.size();
        while ( nPos == -1 && i < nCount )
        {
            auto& pData = m_vLineList[ i ];
            if ( pData->GetMinWidth() <= nWidth )
            {
                if ( nListPos == n )
                    nPos = static_cast<sal_Int32>(i);
                n++;
            }
            i++;
        }

        return nPos;
    }

    void LineListBox::InsertEntry(
        const BorderWidthImpl& rWidthImpl, SvxBorderLineStyle nStyle, tools::Long nMinWidth,
        ColorFunc pColor1Fn, ColorFunc pColor2Fn, ColorDistFunc pColorDistFn )
    {
        m_vLineList.emplace_back(new ImpLineListData(
            rWidthImpl, nStyle, nMinWidth, pColor1Fn, pColor2Fn, pColorDistFn));
    }

    SvxBorderLineStyle LineListBox::GetEntryStyle( sal_Int32 nPos ) const
    {
        ImpLineListData* pData = (0 <= nPos && o3tl::make_unsigned(nPos) < m_vLineList.size()) ? m_vLineList[ nPos ].get() : nullptr;
        return pData ? pData->GetStyle() : SvxBorderLineStyle::NONE;
    }

    void LineListBox::UpdatePaintLineColor()
    {
        const StyleSettings&    rSettings = Application::GetSettings().GetStyleSettings();
        Color                   aNewCol( rSettings.GetWindowColor().IsDark()? rSettings.GetLabelTextColor() : aColor );

        bool bRet = aNewCol != maPaintCol;

        if( bRet )
            maPaintCol = aNewCol;
    }

    Size LineListBox::UpdateEntries( tools::Long nOldWidth )
    {
        Size aSize;

        UpdatePaintLineColor( );

        sal_Int32      nSelEntry = GetSelectItemPos();
        sal_Int32       nTypePos = GetStylePos( nSelEntry, nOldWidth );

        // Remove the old entries
        Clear();

        sal_uInt16 nId(1);

        // Add the new entries based on the defined width
        if (!m_sNone.isEmpty())
            InsertItem(nId++, Image(), m_sNone);

        sal_uInt16 n = 0;
        sal_uInt16 nCount = m_vLineList.size( );
        while ( n < nCount )
        {
            auto& pData = m_vLineList[ n ];
            if ( pData->GetMinWidth() <= m_nWidth )
            {
                BitmapEx aBmp;
                ImpGetLine( pData->GetLine1ForWidth( m_nWidth ),
                        pData->GetLine2ForWidth( m_nWidth ),
                        pData->GetDistForWidth( m_nWidth ),
                        GetColorLine1( GetItemCount( ) ),
                        GetColorLine2( GetItemCount( ) ),
                        GetColorDist( GetItemCount( ) ),
                        pData->GetStyle(), aBmp );
                InsertItem(nId, Image(aBmp), SvtLineListBox::GetLineStyleName(pData->GetStyle()));
                Size aBmpSize = aBmp.GetSizePixel();
                if (aBmpSize.Width() > aSize.Width())
                    aSize.setWidth(aBmpSize.getWidth());
                if (aBmpSize.Height() > aSize.Height())
                    aSize.setHeight(aBmpSize.getHeight());
                if ( n == nTypePos )
                    SelectItem(nId);
            }
            else if ( n == nTypePos )
                SetNoSelection();
            n++;
            ++nId;
        }

        Invalidate();

        return aSize;
    }

    Color LineListBox::GetColorLine1( sal_Int32 nPos )
    {
        sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
        if (nStyle == -1)
            return GetPaintColor( );
        auto& pData = m_vLineList[ nStyle ];
        return pData->GetColorLine1( GetColor( ) );
    }

    Color LineListBox::GetColorLine2( sal_Int32 nPos )
    {
        sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
        if (nStyle == -1)
            return GetPaintColor( );
        auto& pData = m_vLineList[ nStyle ];
        return pData->GetColorLine2( GetColor( ) );
    }

    Color LineListBox::GetColorDist( sal_Int32 nPos )
    {
        Color rResult = Application::GetSettings().GetStyleSettings().GetFieldColor();

        sal_Int32 nStyle = GetStylePos( nPos, m_nWidth );
        if (nStyle == -1)
            return rResult;
        auto& pData = m_vLineList[ nStyle ];
        return pData->GetColorDist( GetColor( ), rResult );
    }
}

namespace {

class SvxLineWindow_Impl final : public WeldToolbarPopup
{
private:
    rtl::Reference<SvxFrameToolBoxControl> m_xControl;
    std::unique_ptr<LineListBox> m_xLineStyleLb;
    std::unique_ptr<weld::CustomWeld> m_xLineStyleLbWin;
    bool                m_bIsWriter;

    DECL_LINK( SelectHdl, ValueSet*, void );

public:
    SvxLineWindow_Impl(SvxFrameToolBoxControl* pControl, weld::Widget* pParent);
    virtual void GrabFocus() override
    {
        m_xLineStyleLb->GrabFocus();
    }
};

}

class SvxStyleToolBoxControl;

class SfxStyleControllerItem_Impl : public SfxStatusListener
{
    public:
        SfxStyleControllerItem_Impl( const Reference< XDispatchProvider >& rDispatchProvider,
                                     sal_uInt16 nSlotId,
                                     const OUString& rCommand,
                                     SvxStyleToolBoxControl& rTbxCtl );

    protected:
        virtual void StateChangedAtStatusListener( SfxItemState eState, const SfxPoolItem* pState ) override;

    private:
        SvxStyleToolBoxControl& rControl;
};

#define BUTTON_PADDING 10
#define ITEM_HEIGHT 30

SvxStyleBox_Base::SvxStyleBox_Base(std::unique_ptr<weld::ComboBox> xWidget,
                                   const OUString& rCommand,
                                   SfxStyleFamily eFamily,
                                   const Reference< XDispatchProvider >& rDispatchProvider,
                                   const Reference< XFrame >& _xFrame,
                                   const OUString& rClearFormatKey,
                                   const OUString& rMoreKey,
                                   bool bInSpec, SvxStyleToolBoxControl& rCtrl)
    : m_rCtrl(rCtrl)
    , m_xMenuBuilder(Application::CreateBuilder(nullptr, "svx/ui/stylemenu.ui"))
    , m_xMenu(m_xMenuBuilder->weld_menu("menu"))
    , m_xWidget(std::move(xWidget))
    , eStyleFamily( eFamily )
    , m_nMaxUserDrawFontWidth(0)
    , m_nLastItemWithMenu(-1)
    , bRelease( true )
    , m_xDispatchProvider( rDispatchProvider )
    , m_xFrame(_xFrame)
    , m_aCommand( rCommand )
    , aClearFormatKey( rClearFormatKey )
    , aMoreKey( rMoreKey )
    , bInSpecialMode( bInSpec )
{
    m_xWidget->connect_changed(LINK(this, SvxStyleBox_Base, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, SvxStyleBox_Base, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, SvxStyleBox_Base, ActivateHdl));
    m_xWidget->connect_focus_out(LINK(this, SvxStyleBox_Base, FocusOutHdl));
    m_xWidget->connect_get_property_tree(LINK(this, SvxStyleBox_Base, DumpAsPropertyTreeHdl));
    m_xWidget->set_help_id(HID_STYLE_LISTBOX);
    m_xWidget->set_entry_completion(true);
    m_xMenu->connect_activate(LINK(this, SvxStyleBox_Base, MenuSelectHdl));

    m_xWidget->connect_custom_get_size(LINK(this, SvxStyleBox_Base, CustomGetSizeHdl));
    m_xWidget->connect_custom_render(LINK(this, SvxStyleBox_Base, CustomRenderHdl));
    m_xWidget->set_custom_renderer(true);

    m_xWidget->set_entry_width_chars(COMBO_WIDTH_IN_CHARS + 3);
}

IMPL_LINK(SvxStyleBox_Base, CustomGetSizeHdl, OutputDevice&, rArg, Size)
{
    CalcOptimalExtraUserWidth(rArg);
    return Size(m_nMaxUserDrawFontWidth, ITEM_HEIGHT);
}

SvxStyleBox_Impl::SvxStyleBox_Impl(vcl::Window* pParent,
                                   const OUString& rCommand,
                                   SfxStyleFamily eFamily,
                                   const Reference< XDispatchProvider >& rDispatchProvider,
                                   const Reference< XFrame >& _xFrame,
                                   const OUString& rClearFormatKey,
                                   const OUString& rMoreKey,
                                   bool bInSpec, SvxStyleToolBoxControl& rCtrl)
    : InterimItemWindow(pParent, "svx/ui/applystylebox.ui", "ApplyStyleBox")
    , SvxStyleBox_Base(m_xBuilder->weld_combo_box("applystyle"), rCommand, eFamily,
                       rDispatchProvider, _xFrame, rClearFormatKey, rMoreKey, bInSpec, rCtrl)
{
    InitControlBase(m_xWidget.get());

    set_id("applystyle");
    SetOptimalSize();
}

void SvxStyleBox_Base::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = true;
        return;
    }
    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

IMPL_LINK(SvxStyleBox_Base, MenuSelectHdl, const OString&, rMenuIdent, void)
{
    if (m_nLastItemWithMenu < 0 || m_nLastItemWithMenu >= m_xWidget->get_count())
        return;

    OUString sEntry = m_xWidget->get_text(m_nLastItemWithMenu);

    ReleaseFocus(); // It must be after getting entry pos!
    Sequence<PropertyValue> aArgs{ comphelper::makePropertyValue("Param", sEntry),
                                   comphelper::makePropertyValue("Family",
                                                                 sal_Int16( eStyleFamily )) };

    if (rMenuIdent == "update")
    {
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            ".uno:StyleUpdateByExample", aArgs );
    }
    else if (rMenuIdent == "edit")
    {
        SfxToolBoxControl::Dispatch( m_xDispatchProvider,
            ".uno:EditStyle", aArgs );
    }
}

IMPL_STATIC_LINK_NOARG(SvxStyleBox_Base, ShowMoreHdl, void*, void)
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    DBG_ASSERT( pViewFrm, "SvxStyleBox_Base::Select(): no viewframe" );
    if (!pViewFrm)
        return;
    pViewFrm->ShowChildWindow(SID_SIDEBAR);
    ::sfx2::sidebar::Sidebar::ShowPanel(u"StyleListPanel", pViewFrm->GetFrame().GetFrameInterface(), true);
}

IMPL_LINK(SvxStyleBox_Base, SelectHdl, weld::ComboBox&, rCombo, void)
{
    Select(rCombo.changed_by_direct_pick()); // only when picked from the list
}

IMPL_LINK_NOARG(SvxStyleBox_Base, ActivateHdl, weld::ComboBox&, bool)
{
    Select(true);
    return true;
}

void SvxStyleBox_Base::Select(bool bNonTravelSelect)
{
    if (!bNonTravelSelect)
        return;

    OUString aSearchEntry(m_xWidget->get_active_text());
    bool bDoIt = true, bClear = false;
    if( bInSpecialMode )
    {
        if( aSearchEntry == aClearFormatKey && m_xWidget->get_active() == 0 )
        {
            aSearchEntry = sDefaultStyle;
            bClear = true;
            //not only apply default style but also call 'ClearFormatting'
            Sequence< PropertyValue > aEmptyVals;
            SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:ResetAttributes",
                aEmptyVals);
        }
        else if (aSearchEntry == aMoreKey && m_xWidget->get_active() == (m_xWidget->get_count() - 1))
        {
            Application::PostUserEvent(LINK(nullptr, SvxStyleBox_Base, ShowMoreHdl));
            //tdf#113214 change text back to previous entry
            set_active_or_entry_text(m_xWidget->get_saved_value());
            bDoIt = false;
        }
    }

    //Do we need to create a new style?
    SfxObjectShell *pShell = SfxObjectShell::Current();
    SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
    SfxStyleSheetBase* pStyle = nullptr;

    bool bCreateNew = false;

    if ( pPool )
    {
        pStyle = pPool->First(eStyleFamily);
        while ( pStyle && pStyle->GetName() != aSearchEntry )
            pStyle = pPool->Next();
    }

    if ( !pStyle )
    {
        // cannot find the style for whatever reason
        // therefore create a new style
        bCreateNew = true;
    }

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    ReleaseFocus();

    if( !bDoIt )
        return;

    if ( bClear )
        set_active_or_entry_text(aSearchEntry);
    m_xWidget->save_value();

    Sequence< PropertyValue > aArgs( 2 );
    auto pArgs = aArgs.getArray();
    pArgs[0].Value  <<= aSearchEntry;
    pArgs[1].Name   = "Family";
    pArgs[1].Value  <<= sal_Int16( eStyleFamily );
    if( bCreateNew )
    {
        pArgs[0].Name   = "Param";
        SfxToolBoxControl::Dispatch( m_xDispatchProvider, ".uno:StyleNewByExample", aArgs);
    }
    else
    {
        pArgs[0].Name   = "Template";
        SfxToolBoxControl::Dispatch( m_xDispatchProvider, m_aCommand, aArgs );
    }
}

void SvxStyleBox_Base::SetFamily( SfxStyleFamily eNewFamily )
{
    eStyleFamily = eNewFamily;
}

IMPL_LINK_NOARG(SvxStyleBox_Base, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xWidget->has_focus()) // a combobox can be comprised of different subwidget so double-check if none of those has focus
        set_active_or_entry_text(m_xWidget->get_saved_value());
}

IMPL_LINK(SvxStyleBox_Base, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return DoKeyInput(rKEvt);
}

bool SvxStyleBox_Base::DoKeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_TAB:
            bRelease = false;
            Select(true);
            break;
        case KEY_ESCAPE:
            set_active_or_entry_text(m_xWidget->get_saved_value());
            if (!m_rCtrl.IsInSidebar())
            {
                ReleaseFocus();
                bHandled = true;
            }
            break;
    }

    return bHandled;
}

bool SvxStyleBox_Impl::DoKeyInput(const KeyEvent& rKEvt)
{
    return SvxStyleBox_Base::DoKeyInput(rKEvt) || ChildKeyInput(rKEvt);
}

void SvxStyleBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }

    InterimItemWindow::DataChanged( rDCEvt );
}

bool SvxStyleBox_Base::AdjustFontForItemHeight(OutputDevice& rDevice, tools::Rectangle const & rTextRect, tools::Long nHeight)
{
    if (rTextRect.Bottom() > nHeight)
    {
        // the text does not fit, adjust the font size
        double ratio = static_cast< double >( nHeight ) / rTextRect.Bottom();
        vcl::Font aFont(rDevice.GetFont());
        Size aPixelSize(aFont.GetFontSize());
        aPixelSize.setWidth( aPixelSize.Width() * ratio );
        aPixelSize.setHeight( aPixelSize.Height() * ratio );
        aFont.SetFontSize(aPixelSize);
        rDevice.SetFont(aFont);
        return true;
    }
    return false;
}

void SvxStyleBox_Impl::SetOptimalSize()
{
    // set width in chars low so the size request will not be overridden
    m_xWidget->set_entry_width_chars(1);
    // tdf#132338 purely using this calculation to keep things their traditional width
    Size aSize(LogicToPixel(Size((COMBO_WIDTH_IN_CHARS + 3) * 4, 0), MapMode(MapUnit::MapAppFont)));
    m_xWidget->set_size_request(aSize.Width(), -1);

    SetSizePixel(get_preferred_size());
}

void SvxStyleBox_Base::UserDrawEntry(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, const OUString &rStyleName)
{
    // IMG_TXT_DISTANCE in ilstbox.hxx is 6, then 1 is added as
    // nBorder, and we are adding 1 in order to look better when
    // italics is present
    const int nLeftDistance = 8;

    tools::Rectangle aTextRect;
    rRenderContext.GetTextBoundRect(aTextRect, rStyleName);

    Point aPos(rRect.TopLeft());
    aPos.AdjustX(nLeftDistance );

    if (!AdjustFontForItemHeight(rRenderContext, aTextRect, rRect.GetHeight()))
        aPos.AdjustY((rRect.GetHeight() - aTextRect.Bottom() ) / 2);

    rRenderContext.DrawText(aPos, rStyleName);
}

void SvxStyleBox_Base::SetupEntry(vcl::RenderContext& rRenderContext, sal_Int32 nItem, const tools::Rectangle& rRect, std::u16string_view rStyleName, bool bIsNotSelected)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if (!bIsNotSelected)
        rRenderContext.SetTextColor(rStyleSettings.GetHighlightTextColor());
    else
        rRenderContext.SetTextColor(rStyleSettings.GetDialogTextColor());

    // handle the push-button
    if (!bIsNotSelected)
    {
        if (nItem == 0 || nItem == m_xWidget->get_count() - 1)
            m_xWidget->set_item_menu(OString::number(nItem), nullptr);
        else
        {
            m_nLastItemWithMenu = nItem;
            m_xWidget->set_item_menu(OString::number(nItem), m_xMenu.get());
        }
    }

    if (nItem <= 0 || nItem >= m_xWidget->get_count() - 1)
        return;

    SfxObjectShell *pShell = SfxObjectShell::Current();
    SfxStyleSheetBasePool* pPool = pShell->GetStyleSheetPool();
    SfxStyleSheetBase* pStyle = nullptr;

    if ( pPool )
    {
        pStyle = pPool->First(eStyleFamily);
        while (pStyle && pStyle->GetName() != rStyleName)
            pStyle = pPool->Next();
    }

    if (!pStyle )
        return;

    std::optional<SfxItemSet> const pItemSet(pStyle->GetItemSetForPreview());
    if (!pItemSet) return;

    const SvxFontItem * const pFontItem =
        pItemSet->GetItem<SvxFontItem>(SID_ATTR_CHAR_FONT);
    const SvxFontHeightItem * const pFontHeightItem =
        pItemSet->GetItem<SvxFontHeightItem>(SID_ATTR_CHAR_FONTHEIGHT);

    if ( !(pFontItem && pFontHeightItem) )
        return;

    Size aFontSize( 0, pFontHeightItem->GetHeight() );
    Size aPixelSize(rRenderContext.LogicToPixel(aFontSize, MapMode(pShell->GetMapUnit())));

    // setup the font properties
    SvxFont aFont;
    aFont.SetFamilyName(pFontItem->GetFamilyName());
    aFont.SetStyleName(pFontItem->GetStyleName());
    aFont.SetFontSize(aPixelSize);

    const SfxPoolItem *pItem = pItemSet->GetItem( SID_ATTR_CHAR_WEIGHT );
    if ( pItem )
        aFont.SetWeight( static_cast< const SvxWeightItem* >( pItem )->GetWeight() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_POSTURE );
    if ( pItem )
        aFont.SetItalic( static_cast< const SvxPostureItem* >( pItem )->GetPosture() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_CONTOUR );
    if ( pItem )
        aFont.SetOutline( static_cast< const SvxContourItem* >( pItem )->GetValue() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_SHADOWED );
    if ( pItem )
        aFont.SetShadow( static_cast< const SvxShadowedItem* >( pItem )->GetValue() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_RELIEF );
    if ( pItem )
        aFont.SetRelief( static_cast< const SvxCharReliefItem* >( pItem )->GetValue() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_UNDERLINE );
    if ( pItem )
        aFont.SetUnderline( static_cast< const SvxUnderlineItem* >( pItem )->GetLineStyle() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_OVERLINE );
    if ( pItem )
        aFont.SetOverline( static_cast< const SvxOverlineItem* >( pItem )->GetValue() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_STRIKEOUT );
    if ( pItem )
        aFont.SetStrikeout( static_cast< const SvxCrossedOutItem* >( pItem )->GetStrikeout() );

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_CASEMAP );
    if ( pItem )
        aFont.SetCaseMap(static_cast<const SvxCaseMapItem*>(pItem)->GetCaseMap());

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_EMPHASISMARK );
    if ( pItem )
        aFont.SetEmphasisMark( static_cast< const SvxEmphasisMarkItem* >( pItem )->GetEmphasisMark() );

    // setup the device & draw
    Color aFontCol = COL_AUTO, aBackCol = COL_AUTO;

    rRenderContext.SetFont(aFont);

    pItem = pItemSet->GetItem( SID_ATTR_CHAR_COLOR );
    // text color, when nothing is selected
    if ( (nullptr != pItem) && bIsNotSelected)
        aFontCol = static_cast< const SvxColorItem* >( pItem )->GetValue();

    drawing::FillStyle style = drawing::FillStyle_NONE;
    // which kind of Fill style is selected
    pItem = pItemSet->GetItem( XATTR_FILLSTYLE );
    // only when ok and not selected
    if ( (nullptr != pItem) && bIsNotSelected)
        style = static_cast< const XFillStyleItem* >( pItem )->GetValue();

    switch(style)
    {
        case drawing::FillStyle_SOLID:
        {
            // set background color
            pItem = pItemSet->GetItem( XATTR_FILLCOLOR );
            if ( nullptr != pItem )
                aBackCol = static_cast< const XFillColorItem* >( pItem )->GetColorValue();

            if ( aBackCol != COL_AUTO )
            {
                rRenderContext.SetFillColor(aBackCol);
                rRenderContext.DrawRect(rRect);
            }
        }
        break;

        default: break;
        //TODO Draw the other background styles: gradient, hatching and bitmap
    }

    // when the font and background color are too similar, adjust the Font-Color
    if( (aFontCol != COL_AUTO) || (aBackCol != COL_AUTO) )
        aFontCol = TestColorsVisible(aFontCol, (aBackCol != COL_AUTO) ? aBackCol : rRenderContext.GetBackground().GetColor());

    // set text color
    if ( aFontCol != COL_AUTO )
        rRenderContext.SetTextColor(aFontCol);
}

IMPL_LINK(SvxStyleBox_Base, CustomRenderHdl, weld::ComboBox::render_args, aPayload, void)
{
    vcl::RenderContext& rRenderContext = std::get<0>(aPayload);
    const ::tools::Rectangle& rRect = std::get<1>(aPayload);
    bool bSelected = std::get<2>(aPayload);
    const OUString& rId = std::get<3>(aPayload);

    sal_uInt32 nIndex = rId.toUInt32();

    OUString aStyleName(m_xWidget->get_text(nIndex));

    rRenderContext.Push(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR);

    SetupEntry(rRenderContext, nIndex, rRect, aStyleName, !bSelected);

    UserDrawEntry(rRenderContext, rRect, aStyleName);

    rRenderContext.Pop();
}

void SvxStyleBox_Base::CalcOptimalExtraUserWidth(vcl::RenderContext& rRenderContext)
{
    if (m_nMaxUserDrawFontWidth)
        return;

    tools::Long nMaxNormalFontWidth = 0;
    sal_Int32 nEntryCount = m_xWidget->get_count();
    for (sal_Int32 i = 0; i < nEntryCount; ++i)
    {
        OUString sStyleName(get_text(i));
        tools::Rectangle aTextRectForDefaultFont;
        rRenderContext.GetTextBoundRect(aTextRectForDefaultFont, sStyleName);

        const tools::Long nWidth = aTextRectForDefaultFont.GetWidth();

        nMaxNormalFontWidth = std::max(nWidth, nMaxNormalFontWidth);
    }

    m_nMaxUserDrawFontWidth = nMaxNormalFontWidth;
    for (sal_Int32 i = 1; i < nEntryCount-1; ++i)
    {
        OUString sStyleName(get_text(i));

        rRenderContext.Push(vcl::PushFlags::FILLCOLOR | vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR);
        SetupEntry(rRenderContext, i, tools::Rectangle(0, 0, RECT_MAX, ITEM_HEIGHT), sStyleName, true);
        tools::Rectangle aTextRectForActualFont;
        rRenderContext.GetTextBoundRect(aTextRectForActualFont, sStyleName);
        if (AdjustFontForItemHeight(rRenderContext, aTextRectForActualFont, ITEM_HEIGHT))
        {
            //Font didn't fit, so it was changed, refetch with final font size
            rRenderContext.GetTextBoundRect(aTextRectForActualFont, sStyleName);
        }
        rRenderContext.Pop();

        const int nWidth = aTextRectForActualFont.GetWidth() + m_xWidget->get_menu_button_width() + BUTTON_PADDING;

        m_nMaxUserDrawFontWidth = std::max(nWidth, m_nMaxUserDrawFontWidth);
    }
}

// test is the color between Font- and background-color to be identify
// return is always the Font-Color
//        when both light or dark, change the Contrast
//        in other case do not change the origin color
//        when the color is R=G=B=128 the DecreaseContrast make 128 the need an exception
Color SvxStyleBox_Base::TestColorsVisible(const Color &FontCol, const Color &BackCol)
{
    constexpr sal_uInt8  ChgVal = 60;       // increase/decrease the Contrast

    Color  retCol = FontCol;
    if ((FontCol.IsDark() == BackCol.IsDark()) && (FontCol.IsBright() == BackCol.IsBright()))
    {
        sal_uInt8 lumi = retCol.GetLuminance();

        if((lumi > 120) && (lumi < 140))
            retCol.DecreaseLuminance(ChgVal / 2);
        else
            retCol.DecreaseContrast(ChgVal);
    }

    return retCol;
}

IMPL_LINK(SvxStyleBox_Base, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    {
        auto entriesNode = rJsonWriter.startNode("entries");
        for (int i = 0, nEntryCount = m_xWidget->get_count(); i < nEntryCount; ++i)
        {
            auto entryNode = rJsonWriter.startNode("");
            rJsonWriter.put("", m_xWidget->get_text(i));
        }
    }

    int nActive = m_xWidget->get_active();
    rJsonWriter.put("selectedCount", static_cast<sal_Int32>(nActive == -1 ? 0 : 1));

    {
        auto selectedNode = rJsonWriter.startNode("selectedEntries");
        if (nActive != -1)
        {
            auto node = rJsonWriter.startNode("");
            rJsonWriter.put("", static_cast<sal_Int32>(nActive));
        }
    }

    rJsonWriter.put("command", ".uno:StyleApply");
}

static bool lcl_GetDocFontList(const FontList** ppFontList, SvxFontNameBox_Base* pBox)
{
    bool bChanged = false;
    const SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFontListItem = nullptr;

    if ( pDocSh )
        pFontListItem =
            static_cast<const SvxFontListItem*>(pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ));
    else
    {
        ::std::unique_ptr<FontList> aFontList(new FontList(Application::GetDefaultDevice()));
        *ppFontList = aFontList.get();
        pBox->SetOwnFontList(std::move(aFontList));
        bChanged = true;
    }

    if ( pFontListItem )
    {
        const FontList* pNewFontList = pFontListItem->GetFontList();
        DBG_ASSERT( pNewFontList, "Doc-FontList not available!" );

        // No old list, but a new list
        if ( !*ppFontList && pNewFontList )
        {
            // => take over
            *ppFontList = pNewFontList;
            bChanged = true;
        }
        else
        {
            // Comparing the font lists is not perfect.
            // When you change the font list in the Doc, you can track
            // changes here only on the Listbox, because ppFontList
            // has already been updated.
            bChanged =
                ( ( *ppFontList != pNewFontList ) ||
                  pBox->GetListCount() != pNewFontList->GetFontNameCount() );
            // HACK: Comparing is incomplete

            if ( bChanged )
                *ppFontList = pNewFontList;
        }

        if ( pBox )
            pBox->set_sensitive(true);
    }
    else if ( pBox && ( pDocSh || !ppFontList ))
    {
        // Disable box only when we have a SfxObjectShell and didn't get a font list OR
        // we don't have a SfxObjectShell and no current font list.
        // It's possible that we currently have no SfxObjectShell, but a current font list.
        // See #i58471: When a user set the focus into the font name combo box and opens
        // the help window with F1. After closing the help window, we disable the font name
        // combo box. The SfxObjectShell::Current() method returns in that case zero. But the
        // font list hasn't changed and therefore the combo box shouldn't be disabled!
        pBox->set_sensitive(false);
    }

    // Fill the FontBox, also the new list if necessary
    if ( pBox && bChanged )
    {
        if ( *ppFontList )
            pBox->Fill( *ppFontList );
        else
            pBox->Clear();
    }
    return bChanged;
}

SvxFontNameBox_Base::SvxFontNameBox_Base(std::unique_ptr<weld::ComboBox> xWidget,
                                         const Reference<XDispatchProvider>& rDispatchProvider,
                                         const Reference<XFrame>& rFrame,
                                         SvxFontNameToolBoxControl& rCtrl)
    : m_xListener(new comphelper::ConfigurationListener("/org.openoffice.Office.Common/Font/View"))
    , m_aWYSIWYG(m_xListener, "ShowFontBoxWYSIWYG", *this)
    , m_aHistory(m_xListener, "History", *this)
    , m_rCtrl(rCtrl)
    , m_xWidget(new FontNameBox(std::move(xWidget)))
    , pFontList(nullptr)
    , nFtCount(0)
    , bRelease(true)
    , m_xDispatchProvider(rDispatchProvider)
    , m_xFrame(rFrame)
    , mbCheckingUnknownFont(false)
{
    EnableControls();

    m_xWidget->connect_changed(LINK(this, SvxFontNameBox_Base, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, SvxFontNameBox_Base, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, SvxFontNameBox_Base, ActivateHdl));
    m_xWidget->connect_focus_in(LINK(this, SvxFontNameBox_Base, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, SvxFontNameBox_Base, FocusOutHdl));
    m_xWidget->connect_get_property_tree(LINK(this, SvxFontNameBox_Base, DumpAsPropertyTreeHdl));

    m_xWidget->set_entry_width_chars(COMBO_WIDTH_IN_CHARS + 5);
}

SvxFontNameBox_Impl::SvxFontNameBox_Impl(vcl::Window* pParent, const Reference<XDispatchProvider>& rDispatchProvider,
                                         const Reference<XFrame>& rFrame, SvxFontNameToolBoxControl& rCtrl)
    : InterimItemWindow(pParent, "svx/ui/fontnamebox.ui", "FontNameBox")
    , SvxFontNameBox_Base(m_xBuilder->weld_combo_box("fontnamecombobox"), rDispatchProvider, rFrame, rCtrl)
{
    set_id("fontnamecombobox");
    SetOptimalSize();
}

void SvxFontNameBox_Base::FillList()
{
    if (!m_xWidget) // e.g. disposed
        return;
    // Save old Selection, set back in the end
    int nStartPos, nEndPos;
    m_xWidget->get_entry_selection_bounds(nStartPos, nEndPos);

    // Did Doc-Fontlist change?
    lcl_GetDocFontList(&pFontList, this);

    m_xWidget->select_entry_region(nStartPos, nEndPos);
}

void SvxFontNameBox_Base::CheckAndMarkUnknownFont()
{
    if (mbCheckingUnknownFont) //tdf#117537 block rentry
        return;
    mbCheckingUnknownFont = true;
    OUString fontname = m_xWidget->get_active_text();
    lcl_GetDocFontList( &pFontList, this );
    // If the font is unknown, show it in italic.
    vcl::Font font = m_xWidget->get_entry_font();
    if( pFontList != nullptr && pFontList->IsAvailable( fontname ))
    {
        if( font.GetItalic() != ITALIC_NONE )
        {
            font.SetItalic( ITALIC_NONE );
            m_xWidget->set_entry_font(font);
            m_xWidget->set_tooltip_text(SvxResId(RID_SVXSTR_CHARFONTNAME));
        }
    }
    else
    {
        if( font.GetItalic() != ITALIC_NORMAL )
        {
            font.SetItalic( ITALIC_NORMAL );
            m_xWidget->set_entry_font(font);
            m_xWidget->set_tooltip_text(SvxResId(RID_SVXSTR_CHARFONTNAME_NOTAVAILABLE));
        }
    }
    mbCheckingUnknownFont = false;
}

void SvxFontNameBox_Base::Update( const css::awt::FontDescriptor* pFontDesc )
{
    if ( pFontDesc )
    {
        aCurFont.SetFamilyName  ( pFontDesc->Name );
        aCurFont.SetFamily      ( FontFamily( pFontDesc->Family ) );
        aCurFont.SetStyleName   ( pFontDesc->StyleName );
        aCurFont.SetPitch       ( FontPitch( pFontDesc->Pitch ) );
        aCurFont.SetCharSet     ( rtl_TextEncoding( pFontDesc->CharSet ) );
    }
    OUString aCurName = aCurFont.GetFamilyName();
    OUString aText = m_xWidget->get_active_text();
    if (aText != aCurName)
        set_active_or_entry_text(aCurName);
}

void SvxFontNameBox_Base::set_active_or_entry_text(const OUString& rText)
{
    m_xWidget->set_active_or_entry_text(rText);
    CheckAndMarkUnknownFont();
}

IMPL_LINK_NOARG(SvxFontNameBox_Base, FocusInHdl, weld::Widget&, void)
{
    FillList();
}

IMPL_LINK(SvxFontNameBox_Base, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return DoKeyInput(rKEvt);
}

bool SvxFontNameBox_Base::DoKeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_TAB:
            bRelease = false;
            Select(true);
            break;

        case KEY_ESCAPE:
            set_active_or_entry_text(m_xWidget->get_saved_value());
            if (!m_rCtrl.IsInSidebar())
            {
                ReleaseFocus_Impl();
                bHandled = true;
            }
            EndPreview();
            break;
    }

    return bHandled;
}

bool SvxFontNameBox_Impl::DoKeyInput(const KeyEvent& rKEvt)
{
    return SvxFontNameBox_Base::DoKeyInput(rKEvt) || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(SvxFontNameBox_Base, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xWidget->has_focus()) // a combobox can be comprised of different subwidget so double-check if none of those has focus
    {
        set_active_or_entry_text(m_xWidget->get_saved_value());
        // send EndPreview
        EndPreview();
    }
}

void SvxFontNameBox_Impl::SetOptimalSize()
{
    // set width in chars low so the size request will not be overridden
    m_xWidget->set_entry_width_chars(1);
    // tdf#132338 purely using this calculation to keep things their traditional width
    Size aSize(LogicToPixel(Size((COMBO_WIDTH_IN_CHARS +5) * 4, 0), MapMode(MapUnit::MapAppFont)));
    m_xWidget->set_size_request(aSize.Width(), -1);

    SetSizePixel(get_preferred_size());
}

void SvxFontNameBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }
    else if ( ( rDCEvt.GetType() == DataChangedEventType::FONTS ) ||
              ( rDCEvt.GetType() == DataChangedEventType::DISPLAY ) )
    {
        // The old font list in shell has likely been destroyed at this point, so we need to get
        // the new one before doing anything further.
        lcl_GetDocFontList( &pFontList, this );
    }
}

void SvxFontNameBox_Base::ReleaseFocus_Impl()
{
    if ( !bRelease )
    {
        bRelease = true;
        return;
    }
    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

void SvxFontNameBox_Base::EnableControls()
{
    bool bEnableMRU = m_aHistory.get();
    sal_uInt16 nEntries = bEnableMRU ? MAX_MRU_FONTNAME_ENTRIES : 0;

    bool bNewWYSIWYG = m_aWYSIWYG.get();
    bool bOldWYSIWYG = m_xWidget->IsWYSIWYGEnabled();

    if (m_xWidget->get_max_mru_count() != nEntries || bNewWYSIWYG != bOldWYSIWYG)
    {
        // refill in the next GetFocus-Handler
        pFontList = nullptr;
        Clear();
        m_xWidget->set_max_mru_count(nEntries);
    }

    if (bNewWYSIWYG != bOldWYSIWYG)
        m_xWidget->EnableWYSIWYG(bNewWYSIWYG);
}

IMPL_LINK(SvxFontNameBox_Base, SelectHdl, weld::ComboBox&, rCombo, void)
{
    Select(rCombo.changed_by_direct_pick()); // only when picked from the list
}

IMPL_LINK_NOARG(SvxFontNameBox_Base, ActivateHdl, weld::ComboBox&, bool)
{
    Select(true);
    return true;
}

void SvxFontNameBox_Base::Select(bool bNonTravelSelect)
{
    Sequence< PropertyValue > aArgs( 1 );
    auto pArgs = aArgs.getArray();
    std::unique_ptr<SvxFontItem> pFontItem;
    if ( pFontList )
    {
        FontMetric aFontMetric( pFontList->Get(m_xWidget->get_active_text(),
            aCurFont.GetWeight(),
            aCurFont.GetItalic() ) );
        aCurFont = aFontMetric;

        pFontItem.reset( new SvxFontItem( aFontMetric.GetFamilyType(),
            aFontMetric.GetFamilyName(),
            aFontMetric.GetStyleName(),
            aFontMetric.GetPitch(),
            aFontMetric.GetCharSet(),
            SID_ATTR_CHAR_FONT ) );

        Any a;
        pFontItem->QueryValue( a );
        pArgs[0].Value  = a;
    }

    if (bNonTravelSelect)
    {
        CheckAndMarkUnknownFont();
        //  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        //  This instance may be deleted in the meantime (i.e. when a dialog is opened
        //  while in Dispatch()), accessing members will crash in this case.
        ReleaseFocus_Impl();
        EndPreview();
        if (pFontItem)
        {
            pArgs[0].Name   = "CharFontName";
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharFontName",
                                         aArgs );
        }
    }
    else
    {
        if (pFontItem)
        {
            pArgs[0].Name   = "CharPreviewFontName";
            SfxToolBoxControl::Dispatch( m_xDispatchProvider,
                                         ".uno:CharPreviewFontName",
                                         aArgs );
        }
    }
}

IMPL_LINK(SvxFontNameBox_Base, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    {
        auto entriesNode = rJsonWriter.startNode("entries");
        for (int i = 0, nEntryCount = m_xWidget->get_count(); i < nEntryCount; ++i)
        {
            auto entryNode = rJsonWriter.startNode("");
            rJsonWriter.put("", m_xWidget->get_text(i));
        }
    }

    int nSelectedEntry = m_xWidget->get_active();
    rJsonWriter.put("selectedCount", static_cast<sal_Int32>(nSelectedEntry == -1 ? 0 : 1));

    {
        auto selectedNode = rJsonWriter.startNode("selectedEntries");
        if (nSelectedEntry != -1)
        {
            auto entryNode = rJsonWriter.startNode("");
            rJsonWriter.put("", m_xWidget->get_text(nSelectedEntry));
        }
    }

    rJsonWriter.put("command", ".uno:CharFontName");
}

ColorWindow::ColorWindow(const OUString& rCommand,
                         std::shared_ptr<PaletteManager> const & rPaletteManager,
                         ColorStatus&               rColorStatus,
                         sal_uInt16                 nSlotId,
                         const Reference< XFrame >& rFrame,
                         const MenuOrToolMenuButton& rMenuButton,
                         TopLevelParentFunction const& rTopLevelParentFunction,
                         ColorSelectFunction const & rColorSelectFunction)
    : WeldToolbarPopup(rFrame, rMenuButton.get_widget(), "svx/ui/colorwindow.ui", "palette_popup_window")
    , theSlotId(nSlotId)
    , maCommand(rCommand)
    , maMenuButton(rMenuButton)
    , mxPaletteManager(rPaletteManager)
    , mrColorStatus(rColorStatus)
    , maTopLevelParentFunction(rTopLevelParentFunction)
    , maColorSelectFunction(rColorSelectFunction)
    , mxColorSet(new SvxColorValueSet(m_xBuilder->weld_scrolled_window("colorsetwin", true)))
    , mxRecentColorSet(new SvxColorValueSet(nullptr))
    , mxPaletteListBox(m_xBuilder->weld_combo_box("palette_listbox"))
    , mxButtonAutoColor(m_xBuilder->weld_button("auto_color_button"))
    , mxButtonNoneColor(m_xBuilder->weld_button("none_color_button"))
    , mxButtonPicker(m_xBuilder->weld_button("color_picker_button"))
    , mxAutomaticSeparator(m_xBuilder->weld_widget("separator4"))
    , mxColorSetWin(new weld::CustomWeld(*m_xBuilder, "colorset", *mxColorSet))
    , mxRecentColorSetWin(new weld::CustomWeld(*m_xBuilder, "recent_colorset", *mxRecentColorSet))
    , mpDefaultButton(nullptr)
{
    mxColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );
    mxRecentColorSet->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );

    switch ( theSlotId )
    {
        case SID_ATTR_CHAR_COLOR_BACKGROUND:
        case SID_BACKGROUND_COLOR:
        case SID_ATTR_CHAR_BACK_COLOR:
        case SID_TABLE_CELL_BACKGROUND_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_NOFILL ) );
            break;
        }
        case SID_AUTHOR_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_BY_AUTHOR ) );
            break;
        }
        case SID_BMPMASK_COLOR:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_TRANSPARENT ) );
            break;
        }
        case SID_ATTR_CHAR_COLOR:
        case SID_ATTR_CHAR_COLOR2:
        case SID_EXTRUSION_3D_COLOR:
        {
            mxButtonAutoColor->set_label(EditResId(RID_SVXSTR_AUTOMATIC));
            break;
        }
        case SID_FM_CTL_PROPERTIES:
        {
            mxButtonAutoColor->set_label( SvxResId( RID_SVXSTR_DEFAULT ) );
            break;
        }
        default:
        {
            mxButtonAutoColor->hide();
            mxAutomaticSeparator->hide();
            break;
        }
    }

    mxPaletteListBox->connect_changed(LINK(this, ColorWindow, SelectPaletteHdl));
    std::vector<OUString> aPaletteList = mxPaletteManager->GetPaletteList();
    mxPaletteListBox->freeze();
    for (const auto& rPalette : aPaletteList)
        mxPaletteListBox->append_text(rPalette);
    mxPaletteListBox->thaw();
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    mxPaletteListBox->set_active_text(aPaletteName);
    const int nSelectedEntry(mxPaletteListBox->get_active());
    if (nSelectedEntry != -1)
        mxPaletteManager->SetPalette(nSelectedEntry);

    mxButtonAutoColor->connect_clicked(LINK(this, ColorWindow, AutoColorClickHdl));
    mxButtonNoneColor->connect_clicked(LINK(this, ColorWindow, AutoColorClickHdl));
    mxButtonPicker->connect_clicked(LINK(this, ColorWindow, OpenPickerClickHdl));

    mxColorSet->SetSelectHdl(LINK( this, ColorWindow, SelectHdl));
    mxRecentColorSet->SetSelectHdl(LINK( this, ColorWindow, SelectHdl));
    m_xTopLevel->set_help_id(HID_POPUP_COLOR);
    mxColorSet->SetHelpId(HID_POPUP_COLOR_CTRL);

    mxPaletteManager->ReloadColorSet(*mxColorSet);
    const sal_uInt32 nMaxItems(SvxColorValueSet::getMaxRowCount() * SvxColorValueSet::getColumnCount());
    Size aSize = mxColorSet->layoutAllVisible(nMaxItems);
    mxColorSet->set_size_request(aSize.Width(), aSize.Height());

    mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
    aSize = mxRecentColorSet->layoutAllVisible(mxPaletteManager->GetRecentColorCount());
    mxRecentColorSet->set_size_request(aSize.Width(), aSize.Height());

    AddStatusListener( ".uno:ColorTableState" );
    AddStatusListener( maCommand );
    if ( maCommand == ".uno:FrameLineColor" )
    {
        AddStatusListener( ".uno:BorderTLBR" );
        AddStatusListener( ".uno:BorderBLTR" );
    }
}

void ColorWindow::GrabFocus()
{
    if (mxColorSet->IsNoSelection() && mpDefaultButton)
        mpDefaultButton->grab_focus();
    else
        mxColorSet->GrabFocus();
}

void ColorWindow::ShowNoneButton()
{
    mxButtonNoneColor->show();
}

ColorWindow::~ColorWindow()
{
}

NamedColor ColorWindow::GetSelectEntryColor(ValueSet const * pColorSet)
{
    Color aColor = pColorSet->GetItemColor(pColorSet->GetSelectedItemId());
    OUString sColorName = pColorSet->GetItemText(pColorSet->GetSelectedItemId());
    return std::make_pair(aColor, sColorName);
}

namespace
{
    NamedColor GetAutoColor(sal_uInt16 nSlotId)
    {
        Color aColor;
        OUString sColorName;
        switch (nSlotId)
        {
            case SID_ATTR_CHAR_COLOR_BACKGROUND:
            case SID_BACKGROUND_COLOR:
            case SID_ATTR_CHAR_BACK_COLOR:
            case SID_TABLE_CELL_BACKGROUND_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_NOFILL);
                break;
            case SID_AUTHOR_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_BY_AUTHOR);
                break;
            case SID_BMPMASK_COLOR:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_TRANSPARENT);
                break;
            case SID_FM_CTL_PROPERTIES:
                aColor = COL_TRANSPARENT;
                sColorName = SvxResId(RID_SVXSTR_DEFAULT);
                break;
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_COLOR2:
            case SID_EXTRUSION_3D_COLOR:
            default:
                aColor = COL_AUTO;
                sColorName = EditResId(RID_SVXSTR_AUTOMATIC);
                break;
        }

        return std::make_pair(aColor, sColorName);
    }

    NamedColor GetNoneColor()
    {
        return std::make_pair(COL_NONE_COLOR, comphelper::LibreOfficeKit::isActive() ? SvxResId(RID_SVXSTR_INVISIBLE)
            : SvxResId(RID_SVXSTR_NONE));
    }
}

NamedColor ColorWindow::GetSelectEntryColor() const
{
    if (!mxColorSet->IsNoSelection())
        return GetSelectEntryColor(mxColorSet.get());
    if (!mxRecentColorSet->IsNoSelection())
        return GetSelectEntryColor(mxRecentColorSet.get());
    if (mxButtonNoneColor.get() == mpDefaultButton)
        return GetNoneColor();
    return GetAutoColor();
}

IMPL_LINK(ColorWindow, SelectHdl, ValueSet*, pColorSet, void)
{
    NamedColor aNamedColor = GetSelectEntryColor(pColorSet);

    if (pColorSet != mxRecentColorSet.get())
    {
         mxPaletteManager->AddRecentColor(aNamedColor.first, aNamedColor.second);
         if (!maMenuButton.get_active())
            mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
    }

    maSelectedLink.Call(aNamedColor);

    // deliberate take a copy here in case maMenuButton.set_inactive
    // triggers a callback that destroys ourself
    ColorSelectFunction aColorSelectFunction(maColorSelectFunction);
    OUString sCommand(maCommand);

    maMenuButton.set_inactive();

    aColorSelectFunction(sCommand, aNamedColor);
}

IMPL_LINK_NOARG(ColorWindow, SelectPaletteHdl, weld::ComboBox&, void)
{
    int nPos = mxPaletteListBox->get_active();
    mxPaletteManager->SetPalette( nPos );
    mxPaletteManager->ReloadColorSet(*mxColorSet);
    mxColorSet->layoutToGivenHeight(mxColorSet->GetOutputSizePixel().Height(), mxPaletteManager->GetColorCount());
}

NamedColor ColorWindow::GetAutoColor() const
{
    return ::GetAutoColor(theSlotId);
}

IMPL_LINK(ColorWindow, AutoColorClickHdl, weld::Button&, rButton, void)
{
    NamedColor aNamedColor = &rButton == mxButtonAutoColor.get() ? GetAutoColor() : GetNoneColor();

    mxColorSet->SetNoSelection();
    mxRecentColorSet->SetNoSelection();
    mpDefaultButton = &rButton;

    maSelectedLink.Call(aNamedColor);

    // deliberate take a copy here in case maMenuButton.set_inactive
    // triggers a callback that destroys ourself
    ColorSelectFunction aColorSelectFunction(maColorSelectFunction);
    OUString sCommand(maCommand);

    maMenuButton.set_inactive();

    aColorSelectFunction(sCommand, aNamedColor);
}

IMPL_LINK_NOARG(ColorWindow, OpenPickerClickHdl, weld::Button&, void)
{
    // copy before set_inactive
    auto nColor = GetSelectEntryColor().first;
    auto pParentWindow = maTopLevelParentFunction();
    OUString sCommand = maCommand;
    std::shared_ptr<PaletteManager> xPaletteManager(mxPaletteManager);

    maMenuButton.set_inactive();

    xPaletteManager->PopupColorPicker(pParentWindow, sCommand, nColor);
}

void ColorWindow::SetNoSelection()
{
    mxColorSet->SetNoSelection();
    mxRecentColorSet->SetNoSelection();
    mpDefaultButton = nullptr;
}

bool ColorWindow::IsNoSelection() const
{
    if (!mxColorSet->IsNoSelection())
        return false;
    if (!mxRecentColorSet->IsNoSelection())
        return false;
    return !mxButtonAutoColor->get_visible() && !mxButtonNoneColor->get_visible();
}

void ColorWindow::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if (rEvent.FeatureURL.Complete == ".uno:ColorTableState")
    {
        if (rEvent.IsEnabled && mxPaletteManager->GetPalette() == 0)
        {
            mxPaletteManager->ReloadColorSet(*mxColorSet);
            mxColorSet->layoutToGivenHeight(mxColorSet->GetOutputSizePixel().Height(), mxPaletteManager->GetColorCount());
        }
    }
    else
    {
        mrColorStatus.statusChanged(rEvent);
        SelectEntry(mrColorStatus.GetColor());
    }
}

bool ColorWindow::SelectValueSetEntry(SvxColorValueSet* pColorSet, const Color& rColor)
{
    for (size_t i = 1; i <= pColorSet->GetItemCount(); ++i)
    {
        if (rColor == pColorSet->GetItemColor(i))
        {
            pColorSet->SelectItem(i);
            return true;
        }
    }
    return false;
}

void ColorWindow::SelectEntry(const NamedColor& rNamedColor)
{
    SetNoSelection();

    const Color &rColor = rNamedColor.first;

    if (mxButtonAutoColor->get_visible() && (rColor == COL_TRANSPARENT || rColor == COL_AUTO))
    {
        mpDefaultButton = mxButtonAutoColor.get();
        return;
    }

    if (mxButtonNoneColor->get_visible() && rColor == COL_NONE_COLOR)
    {
        mpDefaultButton = mxButtonNoneColor.get();
        return;
    }

    // try current palette
    bool bFoundColor = SelectValueSetEntry(mxColorSet.get(), rColor);
    // try recently used
    if (!bFoundColor)
        bFoundColor = SelectValueSetEntry(mxRecentColorSet.get(), rColor);
    // if it's not there, add it there now to the end of the recently used
    // so its available somewhere handy, but not without trashing the
    // whole recently used
    if (!bFoundColor)
    {
        const OUString& rColorName = rNamedColor.second;
        mxPaletteManager->AddRecentColor(rColor, rColorName, false);
        mxPaletteManager->ReloadRecentColorSet(*mxRecentColorSet);
        SelectValueSetEntry(mxRecentColorSet.get(), rColor);
    }
}

void ColorWindow::SelectEntry(const Color& rColor)
{
    OUString sColorName = "#" + rColor.AsRGBHexString().toAsciiUpperCase();
    ColorWindow::SelectEntry(std::make_pair(rColor, sColorName));
}

ColorStatus::ColorStatus() :
    maColor( COL_TRANSPARENT ),
    maTLBRColor( COL_TRANSPARENT ),
    maBLTRColor( COL_TRANSPARENT )
{
}

ColorStatus::~ColorStatus()
{
}

void ColorStatus::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    Color aColor( COL_TRANSPARENT );
    css::table::BorderLine2 aTable;

    if ( rEvent.State >>= aTable )
    {
        SvxBorderLine aLine;
        SvxBoxItem::LineToSvxLine( aTable, aLine, false );
        if ( !aLine.isEmpty() )
            aColor = aLine.GetColor();
    }
    else
        rEvent.State >>= aColor;

    if ( rEvent.FeatureURL.Path == "BorderTLBR" )
        maTLBRColor = aColor;
    else if ( rEvent.FeatureURL.Path == "BorderBLTR" )
        maBLTRColor = aColor;
    else
        maColor = aColor;
}

Color ColorStatus::GetColor()
{
    Color aColor( maColor );

    if ( maTLBRColor != COL_TRANSPARENT )
    {
        if ( aColor != maTLBRColor && aColor != COL_TRANSPARENT )
            return COL_TRANSPARENT;
        aColor = maTLBRColor;
    }

    if ( maBLTRColor != COL_TRANSPARENT )
    {
        if ( aColor != maBLTRColor && aColor != COL_TRANSPARENT )
            return COL_TRANSPARENT;
        return maBLTRColor;
    }

    return aColor;
}


SvxFrameWindow_Impl::SvxFrameWindow_Impl(SvxFrameToolBoxControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/floatingframeborder.ui", "FloatingFrameBorder")
    , mxControl(pControl)
    , mxFrameSet(new SvxFrmValueSet_Impl)
    , mxFrameSetWin(new weld::CustomWeld(*m_xBuilder, "valueset", *mxFrameSet))
    , bParagraphMode(false)
{
    mxFrameSet->SetStyle(WB_ITEMBORDER | WB_DOUBLEBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT);
    AddStatusListener(".uno:BorderReducedMode");
    InitImageList();

    /*
     *  1       2        3         4            5
     *  ------------------------------------------------------
     *  NONE    LEFT     RIGHT     LEFTRIGHT    DIAGONALDOWN
     *  TOP     BOTTOM   TOPBOTTOM OUTER        DIAGONALUP
     *  ------------------------------------------------------
     *  HOR     HORINNER VERINNER   ALL         CRISSCROSS      <- can be switched of via bParagraphMode
     */

    sal_uInt16 i = 0;

    for ( i=1; i<11; i++ )
        mxFrameSet->InsertItem(i, Image(aImgVec[i-1].first), aImgVec[i-1].second);

    //bParagraphMode should have been set in StateChanged
    if ( !bParagraphMode )
        for ( i = 11; i < 16; i++ )
            mxFrameSet->InsertItem(i, Image(aImgVec[i-1].first), aImgVec[i-1].second);

    mxFrameSet->SetColCount( 5 );
    mxFrameSet->SetSelectHdl( LINK( this, SvxFrameWindow_Impl, SelectHdl ) );
    CalcSizeValueSet();

    mxFrameSet->SetHelpId( HID_POPUP_FRAME );
    mxFrameSet->SetAccessibleName( SvxResId(RID_SVXSTR_FRAME) );
}

namespace {

enum class FrmValidFlags {
    NONE      = 0x00,
    Left      = 0x01,
    Right     = 0x02,
    Top       = 0x04,
    Bottom    = 0x08,
    HInner    = 0x10,
    VInner    = 0x20,
    AllMask   = 0x3f,
};

}

namespace o3tl {
    template<> struct typed_flags<FrmValidFlags> : is_typed_flags<FrmValidFlags, 0x3f> {};
}

// By default unset lines remain unchanged.
// Via Shift unset lines are reset

IMPL_LINK_NOARG(SvxFrameWindow_Impl, SelectHdl, ValueSet*, void)
{
    SvxBoxItem          aBorderOuter( SID_ATTR_BORDER_OUTER );
    SvxBoxInfoItem      aBorderInner( SID_ATTR_BORDER_INNER );
    SvxBorderLine       theDefLine;

    // diagonal down border
    SvxBorderLine       dDownBorderLine(nullptr, SvxBorderLineWidth::Hairline);
    SvxLineItem         dDownLineItem(SID_ATTR_BORDER_DIAG_TLBR);

    // diagonal up border
    SvxBorderLine       dUpBorderLine(nullptr, SvxBorderLineWidth::Hairline);
    SvxLineItem         dUpLineItem(SID_ATTR_BORDER_DIAG_BLTR);

    bool                bIsDiagonalBorder = false;

    SvxBorderLine       *pLeft = nullptr,
                        *pRight = nullptr,
                        *pTop = nullptr,
                        *pBottom = nullptr;
    sal_uInt16           nSel = mxFrameSet->GetSelectedItemId();
    sal_uInt16           nModifier = mxFrameSet->GetModifier();
    FrmValidFlags        nValidFlags = FrmValidFlags::NONE;

    // tdf#48622, tdf#145828 use correct default to create intended 0.75pt
    // cell border using the border formatting tool in the standard toolbar
    theDefLine.GuessLinesWidths(theDefLine.GetBorderLineStyle(), SvxBorderLineWidth::Thin);

    switch ( nSel )
    {
        case 1: nValidFlags |= FrmValidFlags::AllMask;
                // set nullptr to remove diagonal lines
                dDownLineItem.SetLine(nullptr);
                dUpLineItem.SetLine(nullptr);
                SetDiagonalDownBorder(dDownLineItem);
                SetDiagonalUpBorder(dUpLineItem);
        break;  // NONE
        case 2: pLeft = &theDefLine;
                nValidFlags |= FrmValidFlags::Left;
        break;  // LEFT
        case 3: pRight = &theDefLine;
                nValidFlags |= FrmValidFlags::Right;
        break;  // RIGHT
        case 4: pLeft = pRight = &theDefLine;
                nValidFlags |=  FrmValidFlags::Right|FrmValidFlags::Left;
        break;  // LEFTRIGHT
        case 5: dDownLineItem.SetLine(&dDownBorderLine);
                SetDiagonalDownBorder(dDownLineItem);
                bIsDiagonalBorder = true;
        break;  // DIAGONAL DOWN
        case 6: pTop = &theDefLine;
                nValidFlags |= FrmValidFlags::Top;
        break;  // TOP
        case 7: pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Bottom;
        break;  // BOTTOM
        case 8: pTop =  pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Bottom|FrmValidFlags::Top;
        break;  // TOPBOTTOM
        case 9: pLeft = pRight = pTop = pBottom = &theDefLine;
                nValidFlags |= FrmValidFlags::Left | FrmValidFlags::Right | FrmValidFlags::Top | FrmValidFlags::Bottom;
        break;  // OUTER
        case 10:
                dUpLineItem.SetLine(&dUpBorderLine);
                SetDiagonalUpBorder(dUpLineItem);
                bIsDiagonalBorder = true;
        break;  // DIAGONAL UP

        // Inner Table:
        case 11: // HOR
            pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::HInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
            break;

        case 12: // HORINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::Right|FrmValidFlags::Left|FrmValidFlags::HInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
            break;

        case 13: // VERINNER
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( nullptr, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::Right|FrmValidFlags::Left|FrmValidFlags::VInner|FrmValidFlags::Top|FrmValidFlags::Bottom;
        break;

        case 14: // ALL
            pLeft = pRight = pTop = pBottom = &theDefLine;
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::HORI );
            aBorderInner.SetLine( &theDefLine, SvxBoxInfoItemLine::VERT );
            nValidFlags |= FrmValidFlags::AllMask;
            break;

        case 15:
            // set both diagonal lines to draw criss-cross line
            dDownLineItem.SetLine(&dDownBorderLine);
            dUpLineItem.SetLine(&dUpBorderLine);

            SetDiagonalDownBorder(dDownLineItem);
            SetDiagonalUpBorder(dUpLineItem);
            bIsDiagonalBorder = true;
            break; // CRISS-CROSS

        default:
        break;
    }

    // if diagonal borders selected,
    // no need to execute this block
    if (!bIsDiagonalBorder)
    {
        aBorderOuter.SetLine( pLeft, SvxBoxItemLine::LEFT );
        aBorderOuter.SetLine( pRight, SvxBoxItemLine::RIGHT );
        aBorderOuter.SetLine( pTop, SvxBoxItemLine::TOP );
        aBorderOuter.SetLine( pBottom, SvxBoxItemLine::BOTTOM );

        if(nModifier == KEY_SHIFT)
            nValidFlags |= FrmValidFlags::AllMask;
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::TOP,       bool(nValidFlags&FrmValidFlags::Top ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::BOTTOM,    bool(nValidFlags&FrmValidFlags::Bottom ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::LEFT,      bool(nValidFlags&FrmValidFlags::Left));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::RIGHT,     bool(nValidFlags&FrmValidFlags::Right ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::HORI,      bool(nValidFlags&FrmValidFlags::HInner ));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::VERT,      bool(nValidFlags&FrmValidFlags::VInner));
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
        aBorderInner.SetValid( SvxBoxInfoItemValidFlags::DISABLE,   false );

        Any a1, a2;
        aBorderOuter.QueryValue( a1 );
        aBorderInner.QueryValue( a2 );
        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue("OuterBorder", a1),
                                         comphelper::makePropertyValue("InnerBorder", a2) };

        mxControl->dispatchCommand( ".uno:SetBorderStyle", aArgs );
    }

    if (mxFrameSet)
    {
        /* #i33380# Moved the following line above the Dispatch() call.
           This instance may be deleted in the meantime (i.e. when a dialog is opened
           while in Dispatch()), accessing members will crash in this case. */
        mxFrameSet->SetNoSelection();
    }

    mxControl->EndPopupMode();
}

void SvxFrameWindow_Impl::SetDiagonalDownBorder(const SvxLineItem& dDownLineItem)
{
    // apply diagonal down border
    Any a;
    dDownLineItem.QueryValue(a);
    Sequence<PropertyValue> aArgs{ comphelper::makePropertyValue("BorderTLBR", a) };

    mxControl->dispatchCommand(".uno:BorderTLBR", aArgs);
}

void SvxFrameWindow_Impl::SetDiagonalUpBorder(const SvxLineItem& dUpLineItem)
{
    // apply diagonal up border
    Any a;
    dUpLineItem.QueryValue(a);
    Sequence<PropertyValue> aArgs{ comphelper::makePropertyValue("BorderBLTR", a) };

    mxControl->dispatchCommand(".uno:BorderBLTR", aArgs);
}

void SvxFrameWindow_Impl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( rEvent.FeatureURL.Complete != ".uno:BorderReducedMode" )
        return;

    bool bValue;
    if ( !(rEvent.State >>= bValue) )
        return;

    bParagraphMode = bValue;
    //initial calls mustn't insert or remove elements
    if(!mxFrameSet->GetItemCount())
        return;

    bool bTableMode = ( mxFrameSet->GetItemCount() == 15 );
    bool bResize    = false;

    if ( bTableMode && bParagraphMode )
    {
        for ( sal_uInt16 i = 11; i < 16; i++ )
            mxFrameSet->RemoveItem(i);
        bResize = true;
    }
    else if ( !bTableMode && !bParagraphMode )
    {
        for ( sal_uInt16 i = 11; i < 16; i++ )
            mxFrameSet->InsertItem(i, Image(aImgVec[i-1].first), aImgVec[i-1].second);
        bResize = true;
    }

    if ( bResize )
    {
        CalcSizeValueSet();
    }
}

void SvxFrameWindow_Impl::CalcSizeValueSet()
{
    weld::DrawingArea* pDrawingArea = mxFrameSet->GetDrawingArea();
    const OutputDevice& rDevice = pDrawingArea->get_ref_device();
    Size aItemSize( 20 * rDevice.GetDPIScaleFactor(), 20 * rDevice.GetDPIScaleFactor() );
    Size aSize = mxFrameSet->CalcWindowSizePixel( aItemSize );
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    mxFrameSet->SetOutputSizePixel(aSize);
}

void SvxFrameWindow_Impl::InitImageList()
{
    aImgVec = {
        {BitmapEx(RID_SVXBMP_FRAME1), SvxResId(RID_SVXSTR_TABLE_PRESET_NONE)},
        {BitmapEx(RID_SVXBMP_FRAME2), SvxResId(RID_SVXSTR_PARA_PRESET_ONLYLEFT)},
        {BitmapEx(RID_SVXBMP_FRAME3), SvxResId(RID_SVXSTR_PARA_PRESET_ONLYRIGHT)},
        {BitmapEx(RID_SVXBMP_FRAME4), SvxResId(RID_SVXSTR_PARA_PRESET_LEFTRIGHT)},
        {BitmapEx(RID_SVXBMP_FRAME14), SvxResId(RID_SVXSTR_PARA_PRESET_DIAGONALDOWN)}, // diagonal down border

        {BitmapEx(RID_SVXBMP_FRAME5), SvxResId(RID_SVXSTR_PARA_PRESET_ONLYTOP)},
        {BitmapEx(RID_SVXBMP_FRAME6), SvxResId(RID_SVXSTR_PARA_PRESET_ONLYTBOTTOM)},
        {BitmapEx(RID_SVXBMP_FRAME7), SvxResId(RID_SVXSTR_PARA_PRESET_TOPBOTTOM)},
        {BitmapEx(RID_SVXBMP_FRAME8), SvxResId(RID_SVXSTR_TABLE_PRESET_ONLYOUTER)},
        {BitmapEx(RID_SVXBMP_FRAME13), SvxResId(RID_SVXSTR_PARA_PRESET_DIAGONALUP)}, // diagonal up border

        {BitmapEx(RID_SVXBMP_FRAME9), SvxResId(RID_SVXSTR_PARA_PRESET_TOPBOTTOMHORI)},
        {BitmapEx(RID_SVXBMP_FRAME10), SvxResId(RID_SVXSTR_TABLE_PRESET_OUTERHORI)},
        {BitmapEx(RID_SVXBMP_FRAME11), SvxResId(RID_SVXSTR_TABLE_PRESET_OUTERVERI)},
        {BitmapEx(RID_SVXBMP_FRAME12), SvxResId(RID_SVXSTR_TABLE_PRESET_OUTERALL)},
        {BitmapEx(RID_SVXBMP_FRAME15), SvxResId(RID_SVXSTR_PARA_PRESET_CRISSCROSS)} // criss-cross border
    };
}

static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

SvxLineWindow_Impl::SvxLineWindow_Impl(SvxFrameToolBoxControl* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/floatingframeborder.ui", "FloatingFrameBorder")
    , m_xControl(pControl)
    , m_xLineStyleLb(new LineListBox)
    , m_xLineStyleLbWin(new weld::CustomWeld(*m_xBuilder, "valueset", *m_xLineStyleLb))
    , m_bIsWriter(false)
{
    try
    {
        Reference< lang::XServiceInfo > xServices(m_xFrame->getController()->getModel(), UNO_QUERY_THROW);
        m_bIsWriter = xServices->supportsService("com.sun.star.text.TextDocument");
    }
    catch(const uno::Exception& )
    {
    }

    m_xLineStyleLb->SetStyle( WinBits(WB_FLATVALUESET | WB_ITEMBORDER | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_TABSTOP) );

    m_xLineStyleLb->SetSourceUnit( FieldUnit::TWIP );
    m_xLineStyleLb->SetNone( comphelper::LibreOfficeKit::isActive() ? SvxResId(RID_SVXSTR_INVISIBLE)
        :SvxResId(RID_SVXSTR_NONE) );

    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::SOLID ), SvxBorderLineStyle::SOLID );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DOTTED ), SvxBorderLineStyle::DOTTED );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DASHED ), SvxBorderLineStyle::DASHED );

    // Double lines
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::DOUBLE ), SvxBorderLineStyle::DOUBLE );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_SMALLGAP ), SvxBorderLineStyle::THINTHICK_SMALLGAP, 20 );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_MEDIUMGAP ), SvxBorderLineStyle::THINTHICK_MEDIUMGAP );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THINTHICK_LARGEGAP ), SvxBorderLineStyle::THINTHICK_LARGEGAP );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_SMALLGAP ), SvxBorderLineStyle::THICKTHIN_SMALLGAP, 20 );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_MEDIUMGAP ), SvxBorderLineStyle::THICKTHIN_MEDIUMGAP );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::THICKTHIN_LARGEGAP ), SvxBorderLineStyle::THICKTHIN_LARGEGAP );

    // Engraved / Embossed
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::EMBOSSED ), SvxBorderLineStyle::EMBOSSED, 15,
            &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor,
            &lcl_mediumColor );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::ENGRAVED ), SvxBorderLineStyle::ENGRAVED, 15,
            &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor,
            &lcl_mediumColor );

    // Inset / Outset
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::OUTSET ), SvxBorderLineStyle::OUTSET, 10,
           &SvxBorderLine::lightColor, &SvxBorderLine::darkColor );
    m_xLineStyleLb->InsertEntry( SvxBorderLine::getWidthImpl( SvxBorderLineStyle::INSET ), SvxBorderLineStyle::INSET, 10,
           &SvxBorderLine::darkColor, &SvxBorderLine::lightColor );
    Size aSize = m_xLineStyleLb->SetWidth( 20 ); // 1pt by default

    m_xLineStyleLb->SetSelectHdl( LINK( this, SvxLineWindow_Impl, SelectHdl ) );

    m_xContainer->set_help_id(HID_POPUP_LINE);

    aSize.AdjustWidth(6);
    aSize.AdjustHeight(6);
    aSize = m_xLineStyleLb->CalcWindowSizePixel(aSize);
    m_xLineStyleLb->GetDrawingArea()->set_size_request(aSize.Width(), aSize.Height());
    m_xLineStyleLb->SetOutputSizePixel(aSize);
}

IMPL_LINK_NOARG(SvxLineWindow_Impl, SelectHdl, ValueSet*, void)
{
    SvxLineItem     aLineItem( SID_FRAME_LINESTYLE );
    SvxBorderLineStyle  nStyle = m_xLineStyleLb->GetSelectEntryStyle();

    if ( m_xLineStyleLb->GetSelectItemPos( ) > 0 )
    {
        SvxBorderLine aTmp;
        aTmp.SetBorderLineStyle( nStyle );
        aTmp.SetWidth( SvxBorderLineWidth::Thin ); // TODO Make it depend on a width field
        aLineItem.SetLine( &aTmp );
    }
    else
        aLineItem.SetLine( nullptr );

    Any a;
    aLineItem.QueryValue( a, m_bIsWriter ? CONVERT_TWIPS : 0 );
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue("LineStyle", a) };

    m_xControl->dispatchCommand( ".uno:LineStyle", aArgs );

    m_xControl->EndPopupMode();
}

SfxStyleControllerItem_Impl::SfxStyleControllerItem_Impl(
    const Reference< XDispatchProvider >& rDispatchProvider,
    sal_uInt16                                nSlotId,      // Family-ID
    const OUString&                  rCommand,     // .uno: command bound to this item
    SvxStyleToolBoxControl&               rTbxCtl )     // controller instance, which the item is assigned to.
    :   SfxStatusListener( rDispatchProvider, nSlotId, rCommand ),
        rControl( rTbxCtl )
{
}

void SfxStyleControllerItem_Impl::StateChangedAtStatusListener(
    SfxItemState eState, const SfxPoolItem* pState )
{
    switch ( GetId() )
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        case SID_STYLE_FAMILY3:
        case SID_STYLE_FAMILY4:
        case SID_STYLE_FAMILY5:
        {
            const sal_uInt16 nIdx = GetId() - SID_STYLE_FAMILY_START;

            if ( SfxItemState::DEFAULT == eState )
            {
                const SfxTemplateItem* pStateItem =
                    dynamic_cast<const SfxTemplateItem*>( pState  );
                DBG_ASSERT( pStateItem != nullptr, "SfxTemplateItem expected" );
                rControl.SetFamilyState( nIdx, pStateItem );
            }
            else
                rControl.SetFamilyState( nIdx, nullptr );
            break;
        }
    }
}

struct SvxStyleToolBoxControl::Impl
{
    OUString                     aClearForm;
    OUString                     aMore;
    ::std::vector< std::pair< OUString, OUString > >    aDefaultStyles;
    bool                     bSpecModeWriter;
    bool                     bSpecModeCalc;

    VclPtr<SvxStyleBox_Impl> m_xVclBox;
    std::unique_ptr<SvxStyleBox_Base> m_xWeldBox;
    SvxStyleBox_Base* m_pBox;

    Impl()
        :aClearForm         ( SvxResId( RID_SVXSTR_CLEARFORM ) )
        ,aMore              ( SvxResId( RID_SVXSTR_MORE_STYLES ) )
        ,bSpecModeWriter    ( false )
        ,bSpecModeCalc      ( false )
        ,m_pBox             ( nullptr )
    {


    }
    void InitializeStyles(const Reference < frame::XModel >& xModel)
    {
        aDefaultStyles.clear();

        //now convert the default style names to the localized names
        try
        {
            Reference< style::XStyleFamiliesSupplier > xStylesSupplier( xModel, UNO_QUERY_THROW );
            Reference< lang::XServiceInfo > xServices( xModel, UNO_QUERY_THROW );
            bSpecModeWriter = xServices->supportsService("com.sun.star.text.TextDocument");
            if(bSpecModeWriter)
            {
                Reference<container::XNameAccess> xParaStyles;
                xStylesSupplier->getStyleFamilies()->getByName("ParagraphStyles") >>=
                    xParaStyles;
                static const std::vector<OUString> aWriterStyles =
                {
                    "Standard",
                    "Text body",
                    "Title",
                    "Subtitle",
                    "Heading 1",
                    "Heading 2",
                    "Heading 3",
                    "Heading 4",
                    "Quotations",
                    "Preformatted Text"
                };
                for( const OUString& aStyle: aWriterStyles )
                {
                    try
                    {
                        Reference< beans::XPropertySet > xStyle;
                        xParaStyles->getByName( aStyle ) >>= xStyle;
                        OUString sName;
                        xStyle->getPropertyValue("DisplayName") >>= sName;
                        if( !sName.isEmpty() )
                            aDefaultStyles.push_back(
                                std::pair<OUString, OUString>(aStyle, sName) );
                    }
                    catch( const uno::Exception& )
                    {}
                }

            }
            else if( (
                bSpecModeCalc = xServices->supportsService(
                    "com.sun.star.sheet.SpreadsheetDocument")))
            {
                static const char* aCalcStyles[] =
                {
                    "Default",
                    "Accent 1",
                    "Accent 2",
                    "Accent 3",
                    "Heading 1",
                    "Heading 2",
                    "Result"
                };
                Reference<container::XNameAccess> xCellStyles;
                xStylesSupplier->getStyleFamilies()->getByName("CellStyles") >>= xCellStyles;
                for(const char* pCalcStyle : aCalcStyles)
                {
                    try
                    {
                        const OUString sStyleName( OUString::createFromAscii( pCalcStyle ) );
                        if( xCellStyles->hasByName( sStyleName ) )
                        {
                            Reference< beans::XPropertySet > xStyle( xCellStyles->getByName( sStyleName), UNO_QUERY_THROW );
                            OUString sName;
                            xStyle->getPropertyValue("DisplayName") >>= sName;
                            if( !sName.isEmpty() )
                                aDefaultStyles.push_back(
                                    std::pair<OUString, OUString>(sStyleName, sName) );
                        }
                    }
                    catch( const uno::Exception& )
                    {}
                }
            }
        }
        catch(const uno::Exception& )
        {
            OSL_FAIL("error while initializing style names");
        }
    }
};

// mapping table from bound items. BE CAREFUL this table must be in the
// same order as the uno commands bound to the slots SID_STYLE_FAMILY1..n
// MAX_FAMILIES must also be correctly set!
static const char* StyleSlotToStyleCommand[MAX_FAMILIES] =
{
    ".uno:CharStyle",
    ".uno:ParaStyle",
    ".uno:FrameStyle",
    ".uno:PageStyle",
    ".uno:TemplateFamily5"
};

SvxStyleToolBoxControl::SvxStyleToolBoxControl()
    : pImpl(new Impl)
    , pStyleSheetPool(nullptr)
    , nActFamily(0xffff)
{
    for (sal_uInt16 i = 0; i < MAX_FAMILIES; ++i)
    {
        m_xBoundItems[i].clear();
        pFamilyState[i]  = nullptr;
    }
}

SvxStyleToolBoxControl::~SvxStyleToolBoxControl()
{
}

void SAL_CALL SvxStyleToolBoxControl::initialize(const Sequence<Any>& rArguments)
{
    svt::ToolboxController::initialize(rArguments);

    // After initialize we should have a valid frame member where we can retrieve our
    // dispatch provider.
    if ( !m_xFrame.is() )
        return;

    pImpl->InitializeStyles(m_xFrame->getController()->getModel());
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame->getController(), UNO_QUERY );
    for ( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
    {
        m_xBoundItems[i] = new SfxStyleControllerItem_Impl( xDispatchProvider,
                                                            SID_STYLE_FAMILY_START + i,
                                                            OUString::createFromAscii( StyleSlotToStyleCommand[i] ),
                                                            *this );
        pFamilyState[i]  = nullptr;
    }
}

// XComponent
void SAL_CALL SvxStyleToolBoxControl::dispose()
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    pImpl->m_xVclBox.disposeAndClear();
    pImpl->m_xWeldBox.reset();
    pImpl->m_pBox = nullptr;

    for (rtl::Reference<SfxStyleControllerItem_Impl>& pBoundItem : m_xBoundItems)
    {
        if (!pBoundItem)
            continue;
        pBoundItem->UnBind();
    }
    unbindListener();

    for( sal_uInt16 i=0; i<MAX_FAMILIES; i++ )
    {
        if ( m_xBoundItems[i].is() )
        {
            try
            {
                m_xBoundItems[i]->dispose();
            }
            catch ( Exception& )
            {
            }

            m_xBoundItems[i].clear();
        }
        pFamilyState[i].reset();
    }
    pStyleSheetPool = nullptr;
    pImpl.reset();
}

OUString SvxStyleToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.StyleToolBoxControl";
}

sal_Bool SvxStyleToolBoxControl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SvxStyleToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_StyleToolBoxControl_get_implementation(
    css::uno::XComponentContext*,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxStyleToolBoxControl() );
}

void SAL_CALL SvxStyleToolBoxControl::update()
{
    for (rtl::Reference<SfxStyleControllerItem_Impl>& pBoundItem : m_xBoundItems)
        pBoundItem->ReBind();
    bindListener();
}

SfxStyleFamily SvxStyleToolBoxControl::GetActFamily() const
{
    switch ( nActFamily-1 + SID_STYLE_FAMILY_START )
    {
        case SID_STYLE_FAMILY1: return SfxStyleFamily::Char;
        case SID_STYLE_FAMILY2: return SfxStyleFamily::Para;
        case SID_STYLE_FAMILY3: return SfxStyleFamily::Frame;
        case SID_STYLE_FAMILY4: return SfxStyleFamily::Page;
        case SID_STYLE_FAMILY5: return SfxStyleFamily::Pseudo;
        default:
            OSL_FAIL( "unknown style family" );
            break;
    }
    return SfxStyleFamily::Para;
}

void SvxStyleToolBoxControl::FillStyleBox()
{
    SvxStyleBox_Base* pBox = pImpl->m_pBox;

    DBG_ASSERT( pStyleSheetPool, "StyleSheetPool not found!" );
    DBG_ASSERT( pBox,            "Control not found!" );

    if ( !(pStyleSheetPool && pBox && nActFamily!=0xffff) )
        return;

    const SfxStyleFamily    eFamily     = GetActFamily();
    SfxStyleSheetBase*      pStyle      = nullptr;
    bool                    bDoFill     = false;

    auto xIter = pStyleSheetPool->CreateIterator(eFamily, SfxStyleSearchBits::Used);
    sal_uInt16 nCount = xIter->Count();

    // Check whether fill is necessary
    pStyle = xIter->First();
    //!!! TODO: This condition isn't right any longer, because we always show some default entries
    //!!! so the list doesn't show the count
    if ( nCount != pBox->get_count() )
    {
        bDoFill = true;
    }
    else
    {
        sal_uInt16 i= 0;
        while ( pStyle && !bDoFill )
        {
            bDoFill = ( pBox->get_text(i) != pStyle->GetName() );
            pStyle = xIter->Next();
            i++;
        }
    }

    if ( !bDoFill )
        return;

    OUString aStrSel(pBox->get_active_text());
    pBox->freeze();
    pBox->clear();

    std::vector<OUString> aStyles;

    {
        pStyle = xIter->Next();

        if( pImpl->bSpecModeWriter || pImpl->bSpecModeCalc )
        {
            while ( pStyle )
            {
                // sort out default styles
                bool bInsert = true;
                OUString aName( pStyle->GetName() );
                for( auto const & _i: pImpl->aDefaultStyles )
                {
                    if( _i.first == aName || _i.second == aName )
                    {
                        bInsert = false;
                        break;
                    }
                }

                if( bInsert )
                    aStyles.push_back(aName);
                pStyle = xIter->Next();
            }
        }
        else
        {
            while ( pStyle )
            {
                aStyles.push_back(pStyle->GetName());
                pStyle = xIter->Next();
            }
        }
    }

    if (pImpl->bSpecModeWriter || pImpl->bSpecModeCalc)
    {
        pBox->append_text(pImpl->aClearForm);
        pBox->insert_separator(1, "separator");

        // insert default styles
        for (const auto &rStyle : pImpl->aDefaultStyles)
            pBox->append_text(rStyle.second);
    }

    std::sort(aStyles.begin(), aStyles.end());

    for (const auto& rStyle : aStyles)
        pBox->append_text(rStyle);

    if ((pImpl->bSpecModeWriter || pImpl->bSpecModeCalc) && !comphelper::LibreOfficeKit::isActive())
        pBox->append_text(pImpl->aMore);

    pBox->thaw();
    pBox->set_active_or_entry_text(aStrSel);
    pBox->SetFamily( eFamily );
}

void SvxStyleToolBoxControl::SelectStyle( const OUString& rStyleName )
{
    SvxStyleBox_Base* pBox = pImpl->m_pBox;
    DBG_ASSERT( pBox, "Control not found!" );

    if ( !pBox )
        return;

    OUString aStrSel(pBox->get_active_text());

    if ( !rStyleName.isEmpty() )
    {
        OUString aNewStyle = rStyleName;

        auto aFound = std::find_if(pImpl->aDefaultStyles.begin(), pImpl->aDefaultStyles.end(),
            [rStyleName] (auto it) { return it.first == rStyleName || it.second == rStyleName; }
        );

        if (aFound != pImpl->aDefaultStyles.end())
            aNewStyle = aFound->second;

        if ( aNewStyle != aStrSel )
            pBox->set_active_or_entry_text( aNewStyle );
    }
    else
        pBox->set_active(-1);
    pBox->save_value();
}

void SvxStyleToolBoxControl::Update()
{
    SfxStyleSheetBasePool*  pPool     = nullptr;
    SfxObjectShell*         pDocShell = SfxObjectShell::Current();

    if ( pDocShell )
        pPool = pDocShell->GetStyleSheetPool();

    sal_uInt16 i;
    for ( i=0; i<MAX_FAMILIES; i++ )
        if( pFamilyState[i] )
            break;

    if ( i==MAX_FAMILIES || !pPool )
    {
        pStyleSheetPool = pPool;
        return;
    }


    const SfxTemplateItem* pItem = nullptr;

    if ( nActFamily == 0xffff || nullptr == (pItem = pFamilyState[nActFamily-1].get()) )
    // Current range not within allowed ranges or default
    {
        pStyleSheetPool = pPool;
        nActFamily      = 2;

        pItem = pFamilyState[nActFamily-1].get();
        if ( !pItem )
        {
            nActFamily++;
            pItem = pFamilyState[nActFamily-1].get();
        }
    }
    else if ( pPool != pStyleSheetPool )
        pStyleSheetPool = pPool;

    FillStyleBox(); // Decides by itself whether Fill is needed

    if ( pItem )
        SelectStyle( pItem->GetStyleName() );
}

void SvxStyleToolBoxControl::SetFamilyState( sal_uInt16 nIdx,
                                             const SfxTemplateItem* pItem )
{
    pFamilyState[nIdx].reset( pItem == nullptr ? nullptr : new SfxTemplateItem( *pItem ) );
    Update();
}

void SvxStyleToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aGuard;

    if (m_pToolbar)
        m_pToolbar->set_item_sensitive(m_aCommandURL.toUtf8(), rEvent.IsEnabled);
    else
    {
        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (!getToolboxId( nId, &pToolBox ) )
            return;
        pToolBox->EnableItem( nId, rEvent.IsEnabled );
    }

    if (rEvent.IsEnabled)
        Update();
}

css::uno::Reference<css::awt::XWindow> SvxStyleToolBoxControl::createItemWindow(const css::uno::Reference< css::awt::XWindow>& rParent)
{
    uno::Reference< awt::XWindow > xItemWindow;

    if (m_pBuilder)
    {
        SolarMutexGuard aSolarMutexGuard;

        std::unique_ptr<weld::ComboBox> xWidget(m_pBuilder->weld_combo_box("applystyle"));

        xItemWindow = css::uno::Reference<css::awt::XWindow>(new weld::TransportAsXWindow(xWidget.get()));

        pImpl->m_xWeldBox.reset(new SvxStyleBox_Base(std::move(xWidget),
                                                     ".uno:StyleApply",
                                                     SfxStyleFamily::Para,
                                                     Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                                     m_xFrame,
                                                     pImpl->aClearForm,
                                                     pImpl->aMore,
                                                     pImpl->bSpecModeWriter || pImpl->bSpecModeCalc, *this));
        pImpl->m_pBox = pImpl->m_xWeldBox.get();
    }
    else
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
        if ( pParent )
        {
            SolarMutexGuard aSolarMutexGuard;

            pImpl->m_xVclBox = VclPtr<SvxStyleBox_Impl>::Create(pParent,
                                                                ".uno:StyleApply",
                                                                SfxStyleFamily::Para,
                                                                Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ),
                                                                m_xFrame,
                                                                pImpl->aClearForm,
                                                                pImpl->aMore,
                                                                pImpl->bSpecModeWriter || pImpl->bSpecModeCalc, *this);
            pImpl->m_pBox = pImpl->m_xVclBox.get();
            xItemWindow = VCLUnoHelper::GetInterface(pImpl->m_xVclBox);
        }
    }

    if (pImpl->m_pBox && !pImpl->aDefaultStyles.empty())
        pImpl->m_pBox->SetDefaultStyle(pImpl->aDefaultStyles[0].second);

    return xItemWindow;
}

SvxFontNameToolBoxControl::SvxFontNameToolBoxControl()
    : m_pBox(nullptr)
{
}

void SvxFontNameBox_Base::statusChanged_Impl( const css::frame::FeatureStateEvent& rEvent )
{
    if ( !rEvent.IsEnabled )
    {
        set_sensitive(false);
        Update( nullptr );
    }
    else
    {
        set_sensitive(true);

        css::awt::FontDescriptor aFontDesc;
        if ( rEvent.State >>= aFontDesc )
            Update(&aFontDesc);
        else
            set_active_or_entry_text("");
        m_xWidget->save_value();
    }
}

void SvxFontNameToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aGuard;
    m_pBox->statusChanged_Impl(rEvent);

    if (m_pToolbar)
        m_pToolbar->set_item_sensitive(m_aCommandURL.toUtf8(), rEvent.IsEnabled);
    else
    {
        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if (!getToolboxId( nId, &pToolBox ) )
            return;
        pToolBox->EnableItem( nId, rEvent.IsEnabled );
    }
}

css::uno::Reference<css::awt::XWindow> SvxFontNameToolBoxControl::createItemWindow(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    uno::Reference< awt::XWindow > xItemWindow;

    if (m_pBuilder)
    {
        SolarMutexGuard aSolarMutexGuard;

        std::unique_ptr<weld::ComboBox> xWidget(m_pBuilder->weld_combo_box("fontnamecombobox"));

        xItemWindow = css::uno::Reference<css::awt::XWindow>(new weld::TransportAsXWindow(xWidget.get()));

        m_xWeldBox.reset(new SvxFontNameBox_Base(std::move(xWidget),
                                                 Reference<XDispatchProvider>(m_xFrame->getController(), UNO_QUERY),
                                                 m_xFrame, *this));
        m_pBox = m_xWeldBox.get();
    }
    else
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow(rParent);
        if ( pParent )
        {
            SolarMutexGuard aSolarMutexGuard;
            m_xVclBox = VclPtr<SvxFontNameBox_Impl>::Create(pParent,
                                                            Reference<XDispatchProvider>(m_xFrame->getController(), UNO_QUERY),
                                                            m_xFrame, *this);
            m_pBox = m_xVclBox.get();
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox);
        }
    }

    return xItemWindow;
}

void SvxFontNameToolBoxControl::dispose()
{
    ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    m_xWeldBox.reset();
    m_pBox = nullptr;
}

OUString SvxFontNameToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.FontNameToolBoxControl";
}

sal_Bool SvxFontNameToolBoxControl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SvxFontNameToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_FontNameToolBoxControl_get_implementation(
    css::uno::XComponentContext*,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxFontNameToolBoxControl() );
}

SvxColorToolBoxControl::SvxColorToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext ) :
    ImplInheritanceHelper( rContext, nullptr, OUString() ),
    m_bSplitButton(true),
    m_nSlotId(0),
    m_aColorSelectFunction(PaletteManager::DispatchColorCommand)
{
}

namespace {

sal_uInt16 MapCommandToSlotId(const OUString& rCommand)
{
    if (rCommand == ".uno:Color")
        return SID_ATTR_CHAR_COLOR;
    else if (rCommand == ".uno:FontColor")
        return SID_ATTR_CHAR_COLOR2;
    else if (rCommand == ".uno:BackColor")
        return SID_ATTR_CHAR_COLOR_BACKGROUND;
    else if (rCommand == ".uno:CharBackColor")
        return SID_ATTR_CHAR_BACK_COLOR;
    else if (rCommand == ".uno:BackgroundColor")
        return SID_BACKGROUND_COLOR;
    else if (rCommand == ".uno:TableCellBackgroundColor")
        return SID_TABLE_CELL_BACKGROUND_COLOR;
    else if (rCommand == ".uno:Extrusion3DColor")
        return SID_EXTRUSION_3D_COLOR;
    else if (rCommand == ".uno:XLineColor")
        return SID_ATTR_LINE_COLOR;
    else if (rCommand == ".uno:FillColor")
        return SID_ATTR_FILL_COLOR;
    else if (rCommand == ".uno:FrameLineColor")
        return SID_FRAME_LINECOLOR;

    SAL_WARN("svx.tbxcrtls", "Unknown color command: " << rCommand);
    return 0;
}

}

void SvxColorToolBoxControl::initialize( const css::uno::Sequence<css::uno::Any>& rArguments )
{
    PopupWindowController::initialize( rArguments );

    m_nSlotId = MapCommandToSlotId( m_aCommandURL );

    if ( m_nSlotId == SID_ATTR_LINE_COLOR || m_nSlotId == SID_ATTR_FILL_COLOR ||
         m_nSlotId == SID_FRAME_LINECOLOR || m_nSlotId == SID_BACKGROUND_COLOR )
    {
        // Sidebar uses wide buttons for those.
        m_bSplitButton = !m_bSidebar;
    }

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(getCommandURL(), getModuleName());
    OUString aCommandLabel = vcl::CommandInfoProvider::GetLabelForCommand(aProperties);

    OString aId(m_aCommandURL.toUtf8());

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(aId, mxPopoverContainer->getTopLevel());
        m_xBtnUpdater.reset(new svx::ToolboxButtonColorUpdater(m_nSlotId, aId, m_pToolbar, !m_bSplitButton, aCommandLabel, m_xFrame));
        return;
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
    {
        m_xBtnUpdater.reset( new svx::VclToolboxButtonColorUpdater( m_nSlotId, nId, pToolBox, !m_bSplitButton,  aCommandLabel, m_aCommandURL, m_xFrame ) );
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ( m_bSplitButton ? ToolBoxItemBits::DROPDOWN : ToolBoxItemBits::DROPDOWNONLY ) );
    }
}

void SvxColorToolBoxControl::update()
{
    PopupWindowController::update();

    switch( m_nSlotId )
    {
        case SID_ATTR_CHAR_COLOR2:
            addStatusListener( ".uno:CharColorExt");
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND:
            addStatusListener( ".uno:CharBackgroundExt");
            break;

        case SID_FRAME_LINECOLOR:
            addStatusListener( ".uno:BorderTLBR");
            addStatusListener( ".uno:BorderBLTR");
            break;
    }
}

void SvxColorToolBoxControl::EnsurePaletteManager()
{
    if (!m_xPaletteManager)
    {
        m_xPaletteManager = std::make_shared<PaletteManager>();
        m_xPaletteManager->SetBtnUpdater(m_xBtnUpdater.get());
    }
}

SvxColorToolBoxControl::~SvxColorToolBoxControl()
{
    if (m_xPaletteManager)
        m_xPaletteManager->SetBtnUpdater(nullptr);
}

void SvxColorToolBoxControl::setColorSelectFunction(const ColorSelectFunction& aColorSelectFunction)
{
    m_aColorSelectFunction = aColorSelectFunction;
    if (m_xPaletteManager)
        m_xPaletteManager->SetColorSelectFunction(aColorSelectFunction);
}

weld::Window* SvxColorToolBoxControl::GetParentFrame() const
{
    const css::uno::Reference<css::awt::XWindow> xParent = m_xFrame->getContainerWindow();
    return Application::GetFrameWeld(xParent);
}

std::unique_ptr<WeldToolbarPopup> SvxColorToolBoxControl::weldPopupWindow()
{
    EnsurePaletteManager();

    const OString aId(m_aCommandURL.toUtf8());

    auto xPopover = std::make_unique<ColorWindow>(
                        m_aCommandURL,
                        m_xPaletteManager,
                        m_aColorStatus,
                        m_nSlotId,
                        m_xFrame,
                        MenuOrToolMenuButton(m_pToolbar, aId),
                        [this] { return GetParentFrame(); },
                        m_aColorSelectFunction);

    if ( m_bSplitButton )
        xPopover->SetSelectedHdl( LINK( this, SvxColorToolBoxControl, SelectedHdl ) );

    return xPopover;
}

VclPtr<vcl::Window> SvxColorToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox))
        return nullptr;

    EnsurePaletteManager();

    auto xPopover = std::make_unique<ColorWindow>(
                        m_aCommandURL,
                        m_xPaletteManager,
                        m_aColorStatus,
                        m_nSlotId,
                        m_xFrame,
                        MenuOrToolMenuButton(this, pToolBox, nId),
                        [this] { return GetParentFrame(); },
                        m_aColorSelectFunction);

    if ( m_bSplitButton )
        xPopover->SetSelectedHdl( LINK( this, SvxColorToolBoxControl, SelectedHdl ) );

    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::move(xPopover), true);

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(m_aCommandURL, m_sModuleName);
    OUString aWindowTitle = vcl::CommandInfoProvider::GetLabelForCommand(aProperties);
    mxInterimPopover->SetText(aWindowTitle);

    mxInterimPopover->Show();

    return mxInterimPopover;
}

IMPL_LINK(SvxColorToolBoxControl, SelectedHdl, const NamedColor&, rColor, void)
{
    m_xBtnUpdater->Update(rColor);
}

void SvxColorToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if ( rEvent.FeatureURL.Complete == m_aCommandURL )
    {
        if (m_pToolbar)
            m_pToolbar->set_item_sensitive(m_aCommandURL.toUtf8(), rEvent.IsEnabled);
        else
            pToolBox->EnableItem( nId, rEvent.IsEnabled );
    }

    bool bValue;
    if ( !m_bSplitButton )
    {
        m_aColorStatus.statusChanged( rEvent );
        m_xBtnUpdater->Update( m_aColorStatus.GetColor() );
    }
    else if ( rEvent.State >>= bValue )
    {
        if (m_pToolbar)
            m_pToolbar->set_item_active(m_aCommandURL.toUtf8(), bValue);
        else if (pToolBox)
            pToolBox->CheckItem( nId, bValue );
    }
}

void SvxColorToolBoxControl::execute(sal_Int16 /*nSelectModifier*/)
{
    if ( !m_bSplitButton )
    {
        if (m_pToolbar)
        {
            // Toggle the popup also when toolbutton is activated
            const OString aId(m_aCommandURL.toUtf8());
            m_pToolbar->set_menu_item_active(aId, !m_pToolbar->get_menu_item_active(aId));
        }
        else
        {
            // Open the popup also when Enter key is pressed.
            createPopupWindow();
        }
        return;
    }

    OUString aCommand = m_aCommandURL;
    Color aColor = m_xBtnUpdater->GetCurrentColor();

    switch( m_nSlotId )
    {
        case SID_ATTR_CHAR_COLOR2 :
            aCommand    = ".uno:CharColorExt";
            break;

        case SID_ATTR_CHAR_COLOR_BACKGROUND :
            aCommand    = ".uno:CharBackgroundExt";
            break;
    }

    auto aArgs( comphelper::InitPropertySequence( {
        { m_aCommandURL.copy(5), css::uno::makeAny(aColor) }
    } ) );
    dispatchCommand( aCommand, aArgs );

    EnsurePaletteManager();
    OUString sColorName = m_xBtnUpdater->GetCurrentColorName();
    m_xPaletteManager->AddRecentColor(aColor, sColorName);
}

sal_Bool SvxColorToolBoxControl::opensSubToolbar()
{
    // We mark this controller as a sub-toolbar controller, so we get notified
    // (through updateImage method) on button image changes, and could redraw
    // the last used color on top of it.
    return true;
}

void SvxColorToolBoxControl::updateImage()
{
    m_xBtnUpdater->Update(m_xBtnUpdater->GetCurrentColor(), true);
}

OUString SvxColorToolBoxControl::getSubToolbarName()
{
    return OUString();
}

void SvxColorToolBoxControl::functionSelected( const OUString& /*rCommand*/ )
{
}

OUString SvxColorToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.ColorToolBoxControl";
}

css::uno::Sequence<OUString> SvxColorToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_ColorToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxColorToolBoxControl( rContext ) );
}

SvxFrameToolBoxControl::SvxFrameToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rContext )
    : svt::PopupWindowController( rContext, nullptr, OUString() )
{
}

void SAL_CALL SvxFrameToolBoxControl::execute(sal_Int16 /*KeyModifier*/)
{
    if (m_pToolbar)
    {
        // Toggle the popup also when toolbutton is activated
        const OString aId(m_aCommandURL.toUtf8());
        m_pToolbar->set_menu_item_active(aId, !m_pToolbar->get_menu_item_active(aId));
    }
    else
    {
        // Open the popup also when Enter key is pressed.
        createPopupWindow();
    }
}

void SvxFrameToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    svt::PopupWindowController::initialize( rArguments );

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL.toUtf8(), mxPopoverContainer->getTopLevel());
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
}

std::unique_ptr<WeldToolbarPopup> SvxFrameToolBoxControl::weldPopupWindow()
{
    if ( m_aCommandURL == ".uno:LineStyle" )
        return std::make_unique<SvxLineWindow_Impl>(this, m_pToolbar);
    return std::make_unique<SvxFrameWindow_Impl>(this, m_pToolbar);
}

VclPtr<vcl::Window> SvxFrameToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    if ( m_aCommandURL == ".uno:LineStyle" )
    {
        mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
            std::make_unique<SvxLineWindow_Impl>(this, pParent->GetFrameWeld()));

        mxInterimPopover->Show();

        mxInterimPopover->SetText(SvxResId(RID_SVXSTR_FRAME_STYLE));

        return mxInterimPopover;
    }

    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<SvxFrameWindow_Impl>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    mxInterimPopover->SetText(SvxResId(RID_SVXSTR_FRAME));

    return mxInterimPopover;
}

OUString SvxFrameToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.FrameToolBoxControl";
}

css::uno::Sequence< OUString > SvxFrameToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_FrameToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxFrameToolBoxControl( rContext ) );
}

SvxCurrencyToolBoxControl::SvxCurrencyToolBoxControl( const css::uno::Reference<css::uno::XComponentContext>& rContext ) :
    PopupWindowController( rContext, nullptr, OUString() ),
    m_eLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() ),
    m_nFormatKey( NUMBERFORMAT_ENTRY_NOT_FOUND )
{
}

SvxCurrencyToolBoxControl::~SvxCurrencyToolBoxControl() {}

namespace
{
    class SvxCurrencyList_Impl : public WeldToolbarPopup
    {
    private:
        rtl::Reference<SvxCurrencyToolBoxControl> m_xControl;
        std::unique_ptr<weld::Label> m_xLabel;
        std::unique_ptr<weld::TreeView> m_xCurrencyLb;
        std::unique_ptr<weld::Button> m_xOkBtn;
        OUString&       m_rSelectedFormat;
        LanguageType&   m_eSelectedLanguage;

        std::vector<OUString> m_aFormatEntries;
        LanguageType          m_eFormatLanguage;
        DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);
        DECL_LINK(OKHdl, weld::Button&, void);

        virtual void GrabFocus() override;

    public:
        SvxCurrencyList_Impl(SvxCurrencyToolBoxControl* pControl, weld::Widget* pParent, OUString& rSelectedFormat, LanguageType& eSelectedLanguage)
            : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/currencywindow.ui", "CurrencyWindow")
            , m_xControl(pControl)
            , m_xLabel(m_xBuilder->weld_label("label"))
            , m_xCurrencyLb(m_xBuilder->weld_tree_view("currency"))
            , m_xOkBtn(m_xBuilder->weld_button("ok"))
            , m_rSelectedFormat(rSelectedFormat)
            , m_eSelectedLanguage(eSelectedLanguage)
        {
            std::vector< OUString > aList;
            std::vector< sal_uInt16 > aCurrencyList;
            const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
            sal_uInt16 nLen = rCurrencyTable.size();

            SvNumberFormatter aFormatter( m_xControl->getContext(), LANGUAGE_SYSTEM );
            m_eFormatLanguage = aFormatter.GetLanguage();

            SvxCurrencyToolBoxControl::GetCurrencySymbols( aList, true, aCurrencyList );

            sal_uInt16 nPos = 0, nCount = 0;
            sal_Int32 nSelectedPos = -1;
            bool bIsSymbol;
            NfWSStringsDtor aStringsDtor;

            OUString sLongestString;

            m_xCurrencyLb->freeze();
            for( const auto& rItem : aList )
            {
                sal_uInt16& rCurrencyIndex = aCurrencyList[ nCount ];
                if ( rCurrencyIndex < nLen )
                {
                    m_xCurrencyLb->append_text(rItem);

                    if (rItem.getLength() > sLongestString.getLength())
                        sLongestString = rItem;

                    bIsSymbol = nPos >= nLen;

                    sal_uInt16 nDefaultFormat;
                    const NfCurrencyEntry& rCurrencyEntry = rCurrencyTable[ rCurrencyIndex ];
                    if (rCurrencyIndex == 0)
                    {
                        // Stored with system locale, but we want the resolved
                        // full LCID format string. For example
                        // "[$$-409]#,##0.00" instead of "[$$]#,##0.00".
                        NfCurrencyEntry aCurrencyEntry( rCurrencyEntry);
                        aCurrencyEntry.SetLanguage( LanguageTag( aCurrencyEntry.GetLanguage()).getLanguageType());
                        nDefaultFormat = aFormatter.GetCurrencyFormatStrings( aStringsDtor, aCurrencyEntry, bIsSymbol);
                    }
                    else
                    {
                        nDefaultFormat = aFormatter.GetCurrencyFormatStrings( aStringsDtor, rCurrencyEntry, bIsSymbol);
                    }
                    const OUString& rFormatStr = aStringsDtor[ nDefaultFormat ];
                    m_aFormatEntries.push_back( rFormatStr );
                    if( rFormatStr == m_rSelectedFormat )
                        nSelectedPos = nPos;
                    ++nPos;
                }
                ++nCount;
            }
            m_xCurrencyLb->thaw();
            // enable multiple selection enabled so we can start with nothing selected
            m_xCurrencyLb->set_selection_mode(SelectionMode::Multiple);
            m_xCurrencyLb->connect_row_activated( LINK( this, SvxCurrencyList_Impl, RowActivatedHdl ) );
            m_xLabel->set_label(SvxResId(RID_SVXSTR_TBLAFMT_CURRENCY));
            m_xCurrencyLb->select( nSelectedPos );
            m_xOkBtn->connect_clicked(LINK(this, SvxCurrencyList_Impl, OKHdl));

            // gtk will initially make a best guess depending on the first few entries, so copy the probable
            // longest entry to the start temporarily and force in the width at this point
            m_xCurrencyLb->insert_text(0, sLongestString);
            m_xCurrencyLb->set_size_request(m_xCurrencyLb->get_preferred_size().Width(), m_xCurrencyLb->get_height_rows(12));
            m_xCurrencyLb->remove(0);
        }
    };

    void SvxCurrencyList_Impl::GrabFocus()
    {
        m_xCurrencyLb->grab_focus();
    }

    IMPL_LINK_NOARG(SvxCurrencyList_Impl, OKHdl, weld::Button&, void)
    {
        RowActivatedHdl(*m_xCurrencyLb);
    }

    IMPL_LINK_NOARG(SvxCurrencyList_Impl, RowActivatedHdl, weld::TreeView&, bool)
    {
        if (!m_xControl.is())
            return true;

        // multiple selection enabled so we can start with nothing selected,
        // so force single selection after something is picked
        int nSelected = m_xCurrencyLb->get_selected_index();
        if (nSelected == -1)
            return true;

        m_xCurrencyLb->set_selection_mode(SelectionMode::Single);

        m_rSelectedFormat = m_aFormatEntries[nSelected];
        m_eSelectedLanguage = m_eFormatLanguage;

        m_xControl->execute(nSelected + 1);

        m_xControl->EndPopupMode();

        return true;
    }
}

void SvxCurrencyToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    if (m_pToolbar)
    {
        mxPopoverContainer.reset(new ToolbarPopupContainer(m_pToolbar));
        m_pToolbar->set_item_popover(m_aCommandURL.toUtf8(), mxPopoverContainer->getTopLevel());
        return;
    }

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox) && pToolBox->GetItemCommand(nId) == m_aCommandURL)
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWN | pToolBox->GetItemBits(nId));
}

std::unique_ptr<WeldToolbarPopup> SvxCurrencyToolBoxControl::weldPopupWindow()
{
    return std::make_unique<SvxCurrencyList_Impl>(this, m_pToolbar, m_aFormatString, m_eLanguage);
}

VclPtr<vcl::Window> SvxCurrencyToolBoxControl::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<SvxCurrencyList_Impl>(this, pParent->GetFrameWeld(), m_aFormatString, m_eLanguage));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

void SvxCurrencyToolBoxControl::execute( sal_Int16 nSelectModifier )
{
    sal_uInt32 nFormatKey;
    if (m_aFormatString.isEmpty())
        nFormatKey = NUMBERFORMAT_ENTRY_NOT_FOUND;
    else
    {
        if ( nSelectModifier > 0 )
        {
            try
            {
                uno::Reference< util::XNumberFormatsSupplier > xRef( m_xFrame->getController()->getModel(), uno::UNO_QUERY );
                uno::Reference< util::XNumberFormats > rxNumberFormats( xRef->getNumberFormats(), uno::UNO_SET_THROW );
                css::lang::Locale aLocale = LanguageTag::convertToLocale( m_eLanguage );
                nFormatKey = rxNumberFormats->queryKey( m_aFormatString, aLocale, false );
                if ( nFormatKey == NUMBERFORMAT_ENTRY_NOT_FOUND )
                    nFormatKey = rxNumberFormats->addNew( m_aFormatString, aLocale );
                }
                catch( const uno::Exception& )
                {
                    nFormatKey = m_nFormatKey;
                }
        }
        else
            nFormatKey = m_nFormatKey;
    }

    if( nFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue("NumberFormatCurrency",
                                                                       nFormatKey) };
        dispatchCommand( m_aCommandURL, aArgs );
        m_nFormatKey = nFormatKey;
    }
    else
        PopupWindowController::execute( nSelectModifier );
}

OUString SvxCurrencyToolBoxControl::getImplementationName()
{
    return "com.sun.star.comp.svx.CurrencyToolBoxControl";
}

css::uno::Sequence<OUString> SvxCurrencyToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_CurrencyToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new SvxCurrencyToolBoxControl( rContext ) );
}

Reference< css::accessibility::XAccessible > SvxFontNameBox_Impl::CreateAccessible()
{
    FillList();
    return InterimItemWindow::CreateAccessible();
}

//static
void SvxCurrencyToolBoxControl::GetCurrencySymbols( std::vector<OUString>& rList, bool bFlag,
                                                    std::vector<sal_uInt16>& rCurrencyList )
{
    rCurrencyList.clear();

    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    sal_uInt16 nStart = 1;

    OUString aString( ApplyLreOrRleEmbedding( rCurrencyTable[0].GetSymbol() ) + " " );
    aString += ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString(
                                       rCurrencyTable[0].GetLanguage() ) );

    rList.push_back( aString );
    rCurrencyList.push_back( sal_uInt16(-1) ); // nAuto

    if( bFlag )
    {
        rList.push_back( aString );
        rCurrencyList.push_back( 0 );
        ++nStart;
    }

    CollatorWrapper aCollator( ::comphelper::getProcessComponentContext() );
    aCollator.loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );

    static const OUStringLiteral aTwoSpace(u"  ");

    for( sal_uInt16 i = 1; i < nCount; ++i )
    {
        OUString aStr( ApplyLreOrRleEmbedding( rCurrencyTable[i].GetBankSymbol() ) );
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( rCurrencyTable[i].GetSymbol() );
        aStr += aTwoSpace;
        aStr += ApplyLreOrRleEmbedding( SvtLanguageTable::GetLanguageString(
                                        rCurrencyTable[i].GetLanguage() ) );

        std::vector<OUString>::size_type j = nStart;
        for( ; j < rList.size(); ++j )
            if ( aCollator.compareString( aStr, rList[j] ) < 0 )
                break;  // insert before first greater than

        rList.insert( rList.begin() + j, aStr );
        rCurrencyList.insert( rCurrencyList.begin() + j, i );
    }

    // Append ISO codes to symbol list.
    // XXX If this is to be changed, various other places would had to be
    // adapted that assume this order!
    std::vector<OUString>::size_type nCont = rList.size();

    for ( sal_uInt16 i = 1; i < nCount; ++i )
    {
        bool bInsert = true;
        OUString aStr( ApplyLreOrRleEmbedding( rCurrencyTable[i].GetBankSymbol() ) );

        std::vector<OUString>::size_type j = nCont;
        for ( ; j < rList.size() && bInsert; ++j )
        {
            if( rList[j] == aStr )
                bInsert = false;
            else if ( aCollator.compareString( aStr, rList[j] ) < 0 )
                break;  // insert before first greater than
        }
        if ( bInsert )
        {
            rList.insert( rList.begin() + j, aStr );
            rCurrencyList.insert( rCurrencyList.begin() + j, i );
        }
    }
}

ListBoxColorWrapper::ListBoxColorWrapper(ColorListBox* pControl)
    : mpControl(pControl)
{
}

void ListBoxColorWrapper::operator()(
    [[maybe_unused]] const OUString& /*rCommand*/, const NamedColor& rColor)
{
    mpControl->Selected(rColor);
}

void ColorListBox::EnsurePaletteManager()
{
    if (!m_xPaletteManager)
    {
        m_xPaletteManager = std::make_shared<PaletteManager>();
        m_xPaletteManager->SetColorSelectFunction(std::ref(m_aColorWrapper));
    }
}

void ColorListBox::SetSlotId(sal_uInt16 nSlotId, bool bShowNoneButton)
{
    m_nSlotId = nSlotId;
    m_bShowNoneButton = bShowNoneButton;
    m_xButton->set_popover(nullptr);
    m_xColorWindow.reset();
    m_aSelectedColor = bShowNoneButton ? GetNoneColor() : GetAutoColor(m_nSlotId);
    ShowPreview(m_aSelectedColor);
    createColorWindow();
}

ColorListBox::ColorListBox(std::unique_ptr<weld::MenuButton> pControl, TopLevelParentFunction const& rTopLevelParentFunction)
    : m_xButton(std::move(pControl))
    , m_aColorWrapper(this)
    , m_aAutoDisplayColor(Application::GetSettings().GetStyleSettings().GetDialogColor())
    , m_nSlotId(0)
    , m_bShowNoneButton(false)
    , m_aTopLevelParentFunction(rTopLevelParentFunction)
{
    m_xButton->connect_toggled(LINK(this, ColorListBox, ToggleHdl));
    m_aSelectedColor = GetAutoColor(m_nSlotId);
    LockWidthRequest();
    ShowPreview(m_aSelectedColor);
}

IMPL_LINK(ColorListBox, ToggleHdl, weld::Toggleable&, rButton, void)
{
    if (rButton.get_active())
    {
        ColorWindow* pColorWindow = getColorWindow();
        if (pColorWindow && !comphelper::LibreOfficeKit::isActive())
            pColorWindow->GrabFocus();
    }
}

ColorListBox::~ColorListBox()
{
}

ColorWindow* ColorListBox::getColorWindow() const
{
    if (!m_xColorWindow)
        const_cast<ColorListBox*>(this)->createColorWindow();
    return m_xColorWindow.get();
}

void ColorListBox::createColorWindow()
{
    const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    css::uno::Reference<css::frame::XFrame> xFrame(pFrame ? pFrame->GetFrameInterface() : uno::Reference<css::frame::XFrame>());

    EnsurePaletteManager();

    m_xColorWindow.reset(new ColorWindow(
                            OUString() /*m_aCommandURL*/,
                            m_xPaletteManager,
                            m_aColorStatus,
                            m_nSlotId,
                            xFrame,
                            m_xButton.get(),
                            m_aTopLevelParentFunction,
                            m_aColorWrapper));

    SetNoSelection();
    m_xButton->set_popover(m_xColorWindow->getTopLevel());
    if (m_bShowNoneButton)
        m_xColorWindow->ShowNoneButton();
    m_xColorWindow->SelectEntry(m_aSelectedColor);
}

void ColorListBox::SelectEntry(const NamedColor& rColor)
{
    if (rColor.second.trim().isEmpty())
    {
        SelectEntry(rColor.first);
        return;
    }
    ColorWindow* pColorWindow = getColorWindow();
    pColorWindow->SelectEntry(rColor);
    m_aSelectedColor = pColorWindow->GetSelectEntryColor();
    ShowPreview(m_aSelectedColor);
}

void ColorListBox::SelectEntry(const Color& rColor)
{
    ColorWindow* pColorWindow = getColorWindow();
    pColorWindow->SelectEntry(rColor);
    m_aSelectedColor = pColorWindow->GetSelectEntryColor();
    ShowPreview(m_aSelectedColor);
}

void ColorListBox::Selected(const NamedColor& rColor)
{
    ShowPreview(rColor);
    m_aSelectedColor = rColor;
    if (m_aSelectedLink.IsSet())
        m_aSelectedLink.Call(*this);
}

//to avoid the box resizing every time the color is changed to
//the optimal size of the individual color, get the longest
//standard color and stick with that as the size for all
void ColorListBox::LockWidthRequest()
{
    NamedColor aLongestColor;
    tools::Long nMaxStandardColorTextWidth = 0;
    XColorListRef const xColorTable = XColorList::CreateStdColorList();
    for (tools::Long i = 0; i != xColorTable->Count(); ++i)
    {
        XColorEntry& rEntry = *xColorTable->GetColor(i);
        auto nColorTextWidth = m_xButton->get_pixel_size(rEntry.GetName()).Width();
        if (nColorTextWidth > nMaxStandardColorTextWidth)
        {
            nMaxStandardColorTextWidth = nColorTextWidth;
            aLongestColor.second = rEntry.GetName();
        }
    }
    ShowPreview(aLongestColor);
    m_xButton->set_size_request(m_xButton->get_preferred_size().Width(), -1);
}

void ColorListBox::ShowPreview(const NamedColor &rColor)
{
    // ScGridWindow::UpdateAutoFilterFromMenu is similar
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Size aImageSize(rStyleSettings.GetListBoxPreviewDefaultPixelSize());

    ScopedVclPtrInstance<VirtualDevice> xDevice;
    xDevice->SetOutputSize(aImageSize);
    const tools::Rectangle aRect(Point(0, 0), aImageSize);
    if (m_bShowNoneButton && rColor.first == COL_NONE_COLOR)
    {
        const Color aW(COL_WHITE);
        const Color aG(0xef, 0xef, 0xef);
        int nMinDim = std::min(aImageSize.Width(), aImageSize.Height()) + 1;
        int nCheckSize = nMinDim / 3;
        xDevice->DrawCheckered(aRect.TopLeft(), aRect.GetSize(), std::min(nCheckSize, 8), aW, aG);
        xDevice->SetFillColor();
    }
    else
    {
        if (rColor.first == COL_AUTO)
            xDevice->SetFillColor(m_aAutoDisplayColor);
        else
            xDevice->SetFillColor(rColor.first);
    }

    xDevice->SetLineColor(rStyleSettings.GetDisableColor());
    xDevice->DrawRect(aRect);

    m_xButton->set_image(xDevice.get());
    m_xButton->set_label(rColor.second);
}

MenuOrToolMenuButton::MenuOrToolMenuButton(weld::MenuButton* pMenuButton)
    : m_pMenuButton(pMenuButton)
    , m_pToolbar(nullptr)
    , m_pControl(nullptr)
    , m_nId(0)
{
}

MenuOrToolMenuButton::MenuOrToolMenuButton(weld::Toolbar* pToolbar, const OString& rIdent)
    : m_pMenuButton(nullptr)
    , m_pToolbar(pToolbar)
    , m_aIdent(rIdent)
    , m_pControl(nullptr)
    , m_nId(0)
{
}

MenuOrToolMenuButton::MenuOrToolMenuButton(SvxColorToolBoxControl* pControl, ToolBox* pToolbar, ToolBoxItemId nId)
    : m_pMenuButton(nullptr)
    , m_pToolbar(nullptr)
    , m_pControl(pControl)
    , m_xToolBox(pToolbar)
    , m_nId(nId)
{
}

MenuOrToolMenuButton::~MenuOrToolMenuButton()
{
}

bool MenuOrToolMenuButton::get_active() const
{
    if (m_pMenuButton)
        return m_pMenuButton->get_active();
    if (m_pToolbar)
        return m_pToolbar->get_menu_item_active(m_aIdent);
    return m_xToolBox->GetDownItemId() == m_nId;
}

void MenuOrToolMenuButton::set_inactive() const
{
    if (m_pMenuButton)
    {
        if (m_pMenuButton->get_active())
            m_pMenuButton->set_active(false);
        return;
    }
    if (m_pToolbar)
    {
        if (m_pToolbar->get_menu_item_active(m_aIdent))
            m_pToolbar->set_menu_item_active(m_aIdent, false);
        return;
    }
    m_pControl->EndPopupMode();
}

weld::Widget* MenuOrToolMenuButton::get_widget() const
{
    if (m_pMenuButton)
        return m_pMenuButton;
    if (m_pToolbar)
        return m_pToolbar;
    return m_xToolBox->GetFrameWeld();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
