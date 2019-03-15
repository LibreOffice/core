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

#include <hangulhanjadlg.hxx>
#include <dialmgr.hxx>

#include <helpids.h>
#include <strings.hrc>

#include <algorithm>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#define HHC editeng::HangulHanjaConversion
#define LINE_CNT        static_cast< sal_uInt16 >(2)
#define MAXNUM_SUGGESTIONS 50


namespace svx
{

    using namespace ::com::sun::star;
    using namespace css::uno;
    using namespace css::linguistic2;
    using namespace css::lang;
    using namespace css::container;


    namespace
    {
        class FontSwitch
        {
        private:
            OutputDevice& m_rDev;

        public:
            FontSwitch( OutputDevice& _rDev, const vcl::Font& _rTemporaryFont )
                :m_rDev( _rDev )
            {
                m_rDev.Push( PushFlags::FONT );
                m_rDev.SetFont( _rTemporaryFont );
            }
            ~FontSwitch() COVERITY_NOEXCEPT_FALSE
            {
                m_rDev.Pop();
            }
        };
    }

    /** a class which allows to draw two texts in a pseudo-ruby way (which basically
        means one text above or below the other, and a little bit smaller)
    */
    class PseudoRubyText
    {
    public:
        enum RubyPosition
        {
            eAbove, eBelow
        };

    protected:
        OUString      m_sPrimaryText;
        OUString      m_sSecondaryText;
        RubyPosition  m_ePosition;

    public:
        PseudoRubyText();
        void init( const OUString& rPrimaryText, const OUString& rSecondaryText, const RubyPosition& rPosition );
        const OUString& getPrimaryText() const { return m_sPrimaryText; }
        const OUString& getSecondaryText() const { return m_sSecondaryText; }

    public:
        void Paint( vcl::RenderContext& _rDevice, const ::tools::Rectangle& _rRect, DrawTextFlags _nTextStyle,
            ::tools::Rectangle* _pPrimaryLocation, ::tools::Rectangle* _pSecondaryLocation );
    };

    PseudoRubyText::PseudoRubyText()
        : m_ePosition(eAbove)
    {
    }

    void PseudoRubyText::init( const OUString& rPrimaryText, const OUString& rSecondaryText, const RubyPosition& rPosition )
    {
        m_sPrimaryText = rPrimaryText;
        m_sSecondaryText = rSecondaryText;
        m_ePosition = rPosition;
    }


    void PseudoRubyText::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& _rRect, DrawTextFlags _nTextStyle,
                               ::tools::Rectangle* _pPrimaryLocation, ::tools::Rectangle* _pSecondaryLocation )
    {
        Size aPlaygroundSize(_rRect.GetSize());

        // the font for the secondary text:
        vcl::Font aSmallerFont(rRenderContext.GetFont());
        // heuristic: 80% of the original size
        aSmallerFont.SetFontHeight( static_cast<long>( 0.8 * aSmallerFont.GetFontHeight() ) );

        // let's calculate the size of our two texts
        ::tools::Rectangle aPrimaryRect = rRenderContext.GetTextRect( _rRect, m_sPrimaryText, _nTextStyle );
        ::tools::Rectangle aSecondaryRect;
        {
            FontSwitch aFontRestore(rRenderContext, aSmallerFont);
            aSecondaryRect = rRenderContext.GetTextRect(_rRect, m_sSecondaryText, _nTextStyle);
        }

        // position these rectangles properly
        // x-axis:
        sal_Int32 nCombinedWidth = std::max( aSecondaryRect.GetWidth(), aPrimaryRect.GetWidth() );
            // the rectangle where both texts will reside is as high as possible, and as wide as the
            // widest of both text rects
        aPrimaryRect.SetLeft( _rRect.Left() );
        aSecondaryRect.SetLeft( aPrimaryRect.Left() );
        aPrimaryRect.SetRight( _rRect.Left() + nCombinedWidth );
        aSecondaryRect.SetRight( aPrimaryRect.Right() );
        if (DrawTextFlags::Right & _nTextStyle)
        {
            // move the rectangles to the right
            aPrimaryRect.Move( aPlaygroundSize.Width() - nCombinedWidth, 0 );
            aSecondaryRect.Move( aPlaygroundSize.Width() - nCombinedWidth, 0 );
        }
        else if (DrawTextFlags::Center & _nTextStyle)
        {
            // center the rectangles
            aPrimaryRect.Move( ( aPlaygroundSize.Width() - nCombinedWidth ) / 2, 0 );
            aSecondaryRect.Move( ( aPlaygroundSize.Width() - nCombinedWidth ) / 2, 0 );
        }

        // y-axis:
        sal_Int32 nCombinedHeight = aPrimaryRect.GetHeight() + aSecondaryRect.GetHeight();
        // align to the top, for the moment
        aPrimaryRect.Move( 0, _rRect.Top() - aPrimaryRect.Top() );
        aSecondaryRect.Move( 0, aPrimaryRect.Top() + aPrimaryRect.GetHeight() - aSecondaryRect.Top() );
        if (DrawTextFlags::Bottom & _nTextStyle)
        {
            // move the rects to the bottom
            aPrimaryRect.Move( 0, aPlaygroundSize.Height() - nCombinedHeight );
            aSecondaryRect.Move( 0, aPlaygroundSize.Height() - nCombinedHeight );
        }
        else if (DrawTextFlags::VCenter & _nTextStyle)
        {
            // move the rects to the bottom
            aPrimaryRect.Move( 0, ( aPlaygroundSize.Height() - nCombinedHeight ) / 2 );
            aSecondaryRect.Move( 0, ( aPlaygroundSize.Height() - nCombinedHeight ) / 2 );
        }

        // 'til here, everything we did assumes that the secondary text is painted _below_ the primary
        // text. If this isn't the case, we need to correct the rectangles
        if (eAbove == m_ePosition)
        {
            sal_Int32 nVertDistance = aSecondaryRect.Top() - aPrimaryRect.Top();
            aSecondaryRect.Move( 0, -nVertDistance );
            aPrimaryRect.Move( 0, nCombinedHeight - nVertDistance );
        }

        // now draw the texts
        // as we already calculated the precise rectangles for the texts, we don't want to
        // use the alignment flags given - within it's rect, every text is centered
        DrawTextFlags nDrawTextStyle( _nTextStyle );
        nDrawTextStyle &= ~DrawTextFlags( DrawTextFlags::Right | DrawTextFlags::Left | DrawTextFlags::Bottom | DrawTextFlags::Top );
        nDrawTextStyle |= DrawTextFlags::Center | DrawTextFlags::VCenter;

        rRenderContext.DrawText( aPrimaryRect, m_sPrimaryText, nDrawTextStyle );
        {
            FontSwitch aFontRestore(rRenderContext, aSmallerFont);
            rRenderContext.DrawText( aSecondaryRect, m_sSecondaryText, nDrawTextStyle );
        }

        // outta here
        if (_pPrimaryLocation)
            *_pPrimaryLocation = aPrimaryRect;
        if (_pSecondaryLocation)
            *_pSecondaryLocation = aSecondaryRect;
    }

    class RubyRadioButton
    {
    public:
        RubyRadioButton(std::unique_ptr<weld::RadioButton> xControl);
        void init(const OUString& rPrimaryText, const OUString& rSecondaryText, const PseudoRubyText::RubyPosition& rPosition);

        void set_sensitive(bool sensitive) { m_xControl->set_sensitive(sensitive); }
        void set_active(bool active) { m_xControl->set_active(active); }
        bool get_active() const { return m_xControl->get_active(); }

        void connect_clicked(const Link<weld::Button&, void>& rLink) { m_xControl->connect_clicked(rLink); }

    private:
        Size GetOptimalSize() const;
        void Paint(vcl::RenderContext& rRenderContext);

        ScopedVclPtr<VirtualDevice> m_xVirDev;
        std::unique_ptr<weld::RadioButton> m_xControl;
        PseudoRubyText m_aRubyText;
    };

    RubyRadioButton::RubyRadioButton(std::unique_ptr<weld::RadioButton> xControl)
        : m_xVirDev(xControl->create_virtual_device())
        , m_xControl(std::move(xControl))
    {
        // expand the point size of the desired font to the equivalent pixel size
        if (vcl::Window* pDefaultDevice = dynamic_cast<vcl::Window*>(Application::GetDefaultDevice()))
            pDefaultDevice->SetPointFont(*m_xVirDev, m_xControl->get_font());
    }

    void RubyRadioButton::init( const OUString& rPrimaryText, const OUString& rSecondaryText, const PseudoRubyText::RubyPosition& rPosition )
    {
        m_aRubyText.init(rPrimaryText, rSecondaryText, rPosition);

        m_xVirDev->SetOutputSizePixel(GetOptimalSize());

        Paint(*m_xVirDev);

        m_xControl->set_image(m_xVirDev.get());
    }

    void RubyRadioButton::Paint(vcl::RenderContext& rRenderContext)
    {
        ::tools::Rectangle aOverallRect(Point(0, 0), rRenderContext.GetOutputSizePixel());
        // inflate the rect a little bit (because the VCL radio button does the same)
        ::tools::Rectangle aTextRect( aOverallRect );
        aTextRect.AdjustLeft( 1 ); aTextRect.AdjustRight( -1 );
        aTextRect.AdjustTop( 1 ); aTextRect.AdjustBottom( -1 );

        // calculate the text flags for the painting
        DrawTextFlags nTextStyle = DrawTextFlags::Mnemonic |
                                   DrawTextFlags::Left |
                                   DrawTextFlags::VCenter;

        // paint the ruby text
        ::tools::Rectangle aPrimaryTextLocation;
        ::tools::Rectangle aSecondaryTextLocation;

        m_aRubyText.Paint(rRenderContext, aTextRect, nTextStyle, &aPrimaryTextLocation, &aSecondaryTextLocation);
    }

    Size RubyRadioButton::GetOptimalSize() const
    {
        vcl::Font aSmallerFont(m_xVirDev->GetFont());
        aSmallerFont.SetFontHeight( static_cast<long>( 0.8 * aSmallerFont.GetFontHeight() ) );
        ::tools::Rectangle rect( Point(), Size( SAL_MAX_INT32, SAL_MAX_INT32 ) );

        Size aPrimarySize = m_xVirDev->GetTextRect( rect, m_aRubyText.getPrimaryText() ).GetSize();
        Size aSecondarySize;
        {
            FontSwitch aFontRestore(*m_xVirDev, aSmallerFont);
            aSecondarySize = m_xVirDev->GetTextRect( rect, m_aRubyText.getSecondaryText() ).GetSize();
        }

        Size minimumSize;
        minimumSize.setHeight( aPrimarySize.Height() + aSecondarySize.Height() + 5 );
        minimumSize.setWidth( aPrimarySize.Width() + aSecondarySize.Width() + 5 );
        return minimumSize;
    }

    SuggestionSet::SuggestionSet(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
        : SvtValueSet(std::move(xScrolledWindow))

    {
    }

    void SuggestionSet::UserDraw( const UserDrawEvent& rUDEvt )
    {
        vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
        ::tools::Rectangle aRect = rUDEvt.GetRect();
        sal_uInt16  nItemId = rUDEvt.GetItemId();

        OUString sText = *static_cast< OUString* >( GetItemData( nItemId ) );
        pDev->DrawText( aRect, sText, DrawTextFlags::Center | DrawTextFlags::VCenter );
    }

    SuggestionDisplay::SuggestionDisplay(weld::Builder& rBuilder)
        : m_bDisplayListBox( true )
        , m_bInSelectionUpdate( false )
        , m_xValueSet(new SuggestionSet(rBuilder.weld_scrolled_window("scrollwin")))
        , m_xValueSetWin(new weld::CustomWeld(rBuilder, "valueset", *m_xValueSet))
        , m_xListBox(rBuilder.weld_tree_view("listbox"))
    {
        m_xValueSet->SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionValueSetHdl ) );
        m_xListBox->connect_changed( LINK( this, SuggestionDisplay, SelectSuggestionListBoxHdl ) );

        m_xValueSet->SetLineCount( LINE_CNT );
        m_xValueSet->SetStyle( m_xValueSet->GetStyle() | WB_ITEMBORDER | WB_VSCROLL );

        OUString const aOneCharacter("AU");
        auto nItemWidth = 2 * m_xListBox->get_pixel_size(aOneCharacter).Width();
        m_xValueSet->SetItemWidth( nItemWidth );

        Size aSize(m_xListBox->get_approximate_digit_width() * 42, m_xListBox->get_text_height() * 5);
        m_xValueSet->set_size_request(aSize.Width(), aSize.Height());
        m_xListBox->set_size_request(aSize.Width(), aSize.Height());

        implUpdateDisplay();
    }

    void SuggestionDisplay::implUpdateDisplay()
    {
        m_xListBox->set_visible(m_bDisplayListBox);
        if (!m_bDisplayListBox)
            m_xValueSetWin->show();
        else
            m_xValueSetWin->hide();
    }

    weld::Widget& SuggestionDisplay::implGetCurrentControl()
    {
        if (m_bDisplayListBox)
            return *m_xListBox;
        return *m_xValueSet->GetDrawingArea();
    }

    void SuggestionDisplay::DisplayListBox( bool bDisplayListBox )
    {
        if( m_bDisplayListBox != bDisplayListBox )
        {
            weld::Widget& rOldControl = implGetCurrentControl();
            bool bHasFocus = rOldControl.has_focus();

            m_bDisplayListBox = bDisplayListBox;

            if( bHasFocus )
            {
                weld::Widget& rNewControl = implGetCurrentControl();
                rNewControl.grab_focus();
            }

            implUpdateDisplay();
        }
    }

    IMPL_LINK_NOARG(SuggestionDisplay, SelectSuggestionValueSetHdl, SvtValueSet*, void)
    {
        SelectSuggestionHdl(false);
    }

    IMPL_LINK_NOARG(SuggestionDisplay, SelectSuggestionListBoxHdl, weld::TreeView&, void)
    {
        SelectSuggestionHdl(true);
    }

    void SuggestionDisplay::SelectSuggestionHdl(bool bListBox)
    {
        if( m_bInSelectionUpdate )
            return;

        m_bInSelectionUpdate = true;
        if (bListBox)
        {
            sal_uInt16 nPos = m_xListBox->get_selected_index();
            m_xValueSet->SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
        }
        else
        {
            sal_uInt16 nPos = m_xValueSet->GetSelectedItemId()-1; //itemid == pos+1 (id 0 has special meaning)
            m_xListBox->select(nPos);
        }
        m_bInSelectionUpdate = false;
        m_aSelectLink.Call( *this );
    }

    void SuggestionDisplay::SetSelectHdl( const Link<SuggestionDisplay&,void>& rLink )
    {
        m_aSelectLink = rLink;
    }

    void SuggestionDisplay::Clear()
    {
        m_xListBox->clear();
        m_xValueSet->Clear();
    }

    void SuggestionDisplay::InsertEntry( const OUString& rStr )
    {
        m_xListBox->append_text(rStr);
        sal_uInt16 nItemId = m_xListBox->n_children(); //itemid == pos+1 (id 0 has special meaning)
        m_xValueSet->InsertItem( nItemId );
        OUString* pItemData = new OUString( rStr );
        m_xValueSet->SetItemData( nItemId, pItemData );
    }

    void SuggestionDisplay::SelectEntryPos( sal_uInt16 nPos )
    {
        m_xListBox->select(nPos);
        m_xValueSet->SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
    }

    sal_uInt16 SuggestionDisplay::GetEntryCount() const
    {
        return m_xListBox->n_children();
    }

    OUString SuggestionDisplay::GetEntry( sal_uInt16 nPos ) const
    {
        return m_xListBox->get_text( nPos );
    }

    OUString SuggestionDisplay::GetSelectedEntry() const
    {
        return m_xListBox->get_selected_text();
    }

    void SuggestionDisplay::SetHelpIds()
    {
        m_xValueSet->SetHelpId(HID_HANGULDLG_SUGGESTIONS_GRID);
        m_xListBox->set_help_id(HID_HANGULDLG_SUGGESTIONS_LIST);
    }

    HangulHanjaConversionDialog::HangulHanjaConversionDialog(weld::Window* pParent)
        : GenericDialogController(pParent, "cui/ui/hangulhanjaconversiondialog.ui", "HangulHanjaConversionDialog")
        , m_bDocumentMode( true )
        , m_xFind(m_xBuilder->weld_button("find"))
        , m_xIgnore(m_xBuilder->weld_button("ignore"))
        , m_xIgnoreAll(m_xBuilder->weld_button("ignoreall"))
        , m_xReplace(m_xBuilder->weld_button("replace"))
        , m_xReplaceAll(m_xBuilder->weld_button("replaceall"))
        , m_xOptions(m_xBuilder->weld_button("options"))
        , m_xSuggestions(new SuggestionDisplay(*m_xBuilder))
        , m_xSimpleConversion(m_xBuilder->weld_radio_button("simpleconversion"))
        , m_xHangulBracketed(m_xBuilder->weld_radio_button("hangulbracket"))
        , m_xHanjaBracketed(m_xBuilder->weld_radio_button("hanjabracket"))
        , m_xWordInput(m_xBuilder->weld_entry("wordinput"))
        , m_xOriginalWord(m_xBuilder->weld_label("originalword"))
        , m_xHanjaAbove(new RubyRadioButton(m_xBuilder->weld_radio_button("hanja_above")))
        , m_xHanjaBelow(new RubyRadioButton(m_xBuilder->weld_radio_button("hanja_below")))
        , m_xHangulAbove(new RubyRadioButton(m_xBuilder->weld_radio_button("hangul_above")))
        , m_xHangulBelow(new RubyRadioButton(m_xBuilder->weld_radio_button("hangul_below")))
        , m_xHangulOnly(m_xBuilder->weld_check_button("hangulonly"))
        , m_xHanjaOnly(m_xBuilder->weld_check_button("hanjaonly"))
        , m_xReplaceByChar(m_xBuilder->weld_check_button("replacebychar"))
    {
        m_xSuggestions->set_size_request(m_xOriginalWord->get_approximate_digit_width() * 42,
                                         m_xOriginalWord->get_text_height() * 5);

        const OUString sHangul(CuiResId(RID_SVXSTR_HANGUL));
        const OUString sHanja(CuiResId(RID_SVXSTR_HANJA));
        m_xHanjaAbove->init( sHangul, sHanja, PseudoRubyText::eAbove );
        m_xHanjaBelow->init( sHangul, sHanja, PseudoRubyText::eBelow );
        m_xHangulAbove->init( sHanja, sHangul, PseudoRubyText::eAbove );
        m_xHangulBelow->init( sHanja, sHangul, PseudoRubyText::eBelow );

        m_xWordInput->connect_changed( LINK( this,  HangulHanjaConversionDialog, OnSuggestionModified ) );
        m_xSuggestions->SetSelectHdl( LINK( this,  HangulHanjaConversionDialog, OnSuggestionSelected ) );
        m_xReplaceByChar->connect_toggled( LINK( this, HangulHanjaConversionDialog, ClickByCharacterHdl ) );
        m_xHangulOnly->connect_toggled( LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );
        m_xHanjaOnly->connect_toggled(  LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );
        m_xOptions->connect_clicked(LINK(this, HangulHanjaConversionDialog, OnOption));

        // initial focus
        FocusSuggestion( );

        // initial control values
        m_xSimpleConversion->set_active(true);

        m_xSuggestions->SetHelpIds();
    }

    HangulHanjaConversionDialog::~HangulHanjaConversionDialog()
    {
    }

    void HangulHanjaConversionDialog::FillSuggestions( const css::uno::Sequence< OUString >& _rSuggestions )
    {
        m_xSuggestions->Clear();
        for ( auto const & suggestion : _rSuggestions )
            m_xSuggestions->InsertEntry( suggestion );

        // select the first suggestion, and fill in the suggestion edit field
        OUString sFirstSuggestion;
        if ( m_xSuggestions->GetEntryCount() )
        {
            sFirstSuggestion = m_xSuggestions->GetEntry( 0 );
            m_xSuggestions->SelectEntryPos( 0 );
        }
        m_xWordInput->set_text( sFirstSuggestion );
        m_xWordInput->save_value();
        OnSuggestionModified( *m_xWordInput );
    }

    void HangulHanjaConversionDialog::SetOptionsChangedHdl(const Link<LinkParamNone*,void>& rHdl)
    {
        m_aOptionsChangedLink = rHdl;
    }

    void HangulHanjaConversionDialog::SetIgnoreHdl(const Link<weld::Button&,void>& rHdl)
    {
        m_xIgnore->connect_clicked(rHdl);
    }

    void HangulHanjaConversionDialog::SetIgnoreAllHdl(const Link<weld::Button&,void>& rHdl)
    {
        m_xIgnoreAll->connect_clicked(rHdl);
    }

    void HangulHanjaConversionDialog::SetChangeHdl(const Link<weld::Button&,void>& rHdl )
    {
        m_xReplace->connect_clicked(rHdl);
    }

    void HangulHanjaConversionDialog::SetChangeAllHdl(const Link<weld::Button&,void>& rHdl)
    {
        m_xReplaceAll->connect_clicked(rHdl);
    }

    void HangulHanjaConversionDialog::SetFindHdl(const Link<weld::Button&,void>& rHdl)
    {
        m_xFind->connect_clicked(rHdl);
    }

    void HangulHanjaConversionDialog::SetConversionFormatChangedHdl( const Link<weld::Button&,void>& rHdl )
    {
        m_xSimpleConversion->connect_clicked( rHdl );
        m_xHangulBracketed->connect_clicked( rHdl );
        m_xHanjaBracketed->connect_clicked( rHdl );
        m_xHanjaAbove->connect_clicked( rHdl );
        m_xHanjaBelow->connect_clicked( rHdl );
        m_xHangulAbove->connect_clicked( rHdl );
        m_xHangulBelow->connect_clicked( rHdl );
    }

    void HangulHanjaConversionDialog::SetClickByCharacterHdl( const Link<weld::ToggleButton&,void>& _rHdl )
    {
        m_aClickByCharacterLink = _rHdl;
    }

    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionSelected, SuggestionDisplay&, void )
    {
        m_xWordInput->set_text(m_xSuggestions->GetSelectedEntry());
        OnSuggestionModified( *m_xWordInput );
    }

    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionModified, weld::Entry&, void )
    {
        m_xFind->set_sensitive(m_xWordInput->get_value_changed_from_saved());

        bool bSameLen = m_xWordInput->get_text().getLength() == m_xOriginalWord->get_label().getLength();
        m_xReplace->set_sensitive( m_bDocumentMode && bSameLen );
        m_xReplaceAll->set_sensitive( m_bDocumentMode && bSameLen );
    }

    IMPL_LINK(HangulHanjaConversionDialog, ClickByCharacterHdl, weld::ToggleButton&, rBox, void)
    {
        m_aClickByCharacterLink.Call(rBox);
        bool bByCharacter = rBox.get_active();
        m_xSuggestions->DisplayListBox( !bByCharacter );
    }

    IMPL_LINK(HangulHanjaConversionDialog, OnConversionDirectionClicked, weld::ToggleButton&, rBox, void)
    {
        weld::CheckButton* pOtherBox = nullptr;
        if (&rBox == m_xHangulOnly.get())
            pOtherBox = m_xHanjaOnly.get();
        else
            pOtherBox = m_xHangulOnly.get();
        bool bBoxChecked = rBox.get_active();
        if (bBoxChecked)
            pOtherBox->set_active(false);
        pOtherBox->set_sensitive(!bBoxChecked);
    }

    IMPL_LINK_NOARG(HangulHanjaConversionDialog, OnOption, weld::Button&, void)
    {
        HangulHanjaOptionsDialog aOptDlg(m_xDialog.get());
        aOptDlg.run();
        m_aOptionsChangedLink.Call( nullptr );
    }

    OUString HangulHanjaConversionDialog::GetCurrentString( ) const
    {
        return m_xOriginalWord->get_label();
    }

    void HangulHanjaConversionDialog::FocusSuggestion( )
    {
        m_xWordInput->grab_focus();
    }

    void HangulHanjaConversionDialog::SetCurrentString( const OUString& _rNewString,
        const Sequence< OUString >& _rSuggestions, bool _bOriginatesFromDocument )
    {
        m_xOriginalWord->set_label(_rNewString);

        bool bOldDocumentMode = m_bDocumentMode;
        m_bDocumentMode = _bOriginatesFromDocument; // before FillSuggestions!
        FillSuggestions( _rSuggestions );

        m_xIgnoreAll->set_sensitive( m_bDocumentMode );

        // switch the def button depending if we're working for document text
        if (bOldDocumentMode != m_bDocumentMode)
        {
            weld::Widget* pOldDefButton = nullptr;
            weld::Widget* pNewDefButton = nullptr;
            if (m_bDocumentMode)
            {
                pOldDefButton = m_xFind.get();
                pNewDefButton = m_xReplace.get();
            }
            else
            {
                pOldDefButton = m_xReplace.get();
                pNewDefButton = m_xFind.get();
            }

            pOldDefButton->set_has_default(false);
            pNewDefButton->set_has_default(true);
        }
    }

    OUString HangulHanjaConversionDialog::GetCurrentSuggestion( ) const
    {
        return m_xWordInput->get_text();
    }

    void HangulHanjaConversionDialog::SetByCharacter( bool _bByCharacter )
    {
        m_xReplaceByChar->set_active( _bByCharacter );
        m_xSuggestions->DisplayListBox( !_bByCharacter );
    }

    void HangulHanjaConversionDialog::SetConversionDirectionState(
            bool _bTryBothDirections,
            HHC::ConversionDirection ePrimaryConversionDirection )
    {
        // default state: try both direction
        m_xHangulOnly->set_active( false );
        m_xHangulOnly->set_sensitive(true);
        m_xHanjaOnly->set_active( false );
        m_xHanjaOnly->set_sensitive(true);

        if (!_bTryBothDirections)
        {
            weld::CheckButton* pBox = ePrimaryConversionDirection == HHC::eHangulToHanja ?
                                    m_xHangulOnly.get() : m_xHanjaOnly.get();
            pBox->set_active(true);
            OnConversionDirectionClicked(*pBox);
        }
    }

    bool HangulHanjaConversionDialog::GetUseBothDirections( ) const
    {
        return !m_xHangulOnly->get_active() && !m_xHanjaOnly->get_active();
    }

    HHC::ConversionDirection HangulHanjaConversionDialog::GetDirection(
            HHC::ConversionDirection eDefaultDirection ) const
    {
        HHC::ConversionDirection eDirection = eDefaultDirection;
        if (m_xHangulOnly->get_active() && !m_xHanjaOnly->get_active())
            eDirection = HHC::eHangulToHanja;
        else if (!m_xHangulOnly->get_active() && m_xHanjaOnly->get_active())
            eDirection = HHC::eHanjaToHangul;
        return eDirection;
    }

    void HangulHanjaConversionDialog::SetConversionFormat( HHC::ConversionFormat _eType )
    {
        switch ( _eType )
        {
            case HHC::eSimpleConversion: m_xSimpleConversion->set_active(true); break;
            case HHC::eHangulBracketed: m_xHangulBracketed->set_active(true); break;
            case HHC::eHanjaBracketed:  m_xHanjaBracketed->set_active(true); break;
            case HHC::eRubyHanjaAbove:  m_xHanjaAbove->set_active(true); break;
            case HHC::eRubyHanjaBelow:  m_xHanjaBelow->set_active(true); break;
            case HHC::eRubyHangulAbove: m_xHangulAbove->set_active(true); break;
            case HHC::eRubyHangulBelow: m_xHangulBelow->set_active(true); break;
        default:
            OSL_FAIL( "HangulHanjaConversionDialog::SetConversionFormat: unknown type!" );
        }
    }

    HHC::ConversionFormat HangulHanjaConversionDialog::GetConversionFormat( ) const
    {
        if ( m_xSimpleConversion->get_active() )
            return HHC::eSimpleConversion;
        if ( m_xHangulBracketed->get_active() )
            return HHC::eHangulBracketed;
        if ( m_xHanjaBracketed->get_active() )
            return HHC::eHanjaBracketed;
        if ( m_xHanjaAbove->get_active() )
            return HHC::eRubyHanjaAbove;
        if ( m_xHanjaBelow->get_active() )
            return HHC::eRubyHanjaBelow;
        if ( m_xHangulAbove->get_active() )
            return HHC::eRubyHangulAbove;
        if ( m_xHangulBelow->get_active() )
            return HHC::eRubyHangulBelow;

        OSL_FAIL( "HangulHanjaConversionDialog::GetConversionFormat: no radio checked?" );
        return HHC::eSimpleConversion;
    }

    void HangulHanjaConversionDialog::EnableRubySupport( bool bVal )
    {
        m_xHanjaAbove->set_sensitive( bVal );
        m_xHanjaBelow->set_sensitive( bVal );
        m_xHangulAbove->set_sensitive( bVal );
        m_xHangulBelow->set_sensitive( bVal );
    }

    void HangulHanjaOptionsDialog::Init()
    {
        if( !m_xConversionDictionaryList.is() )
        {
            m_xConversionDictionaryList = ConversionDictionaryList::create( ::comphelper::getProcessComponentContext() );
        }

        m_aDictList.clear();
        m_xDictsLB->clear();

        Reference< XNameContainer > xNameCont = m_xConversionDictionaryList->getDictionaryContainer();
        if( xNameCont.is() )
        {
            Sequence< OUString >     aDictNames( xNameCont->getElementNames() );

            const OUString*          pDic = aDictNames.getConstArray();
            sal_Int32                       nCount = aDictNames.getLength();

            sal_Int32                       i;
            for( i = 0 ; i < nCount ; ++i )
            {
                Any                                 aAny( xNameCont->getByName( pDic[ i ] ) );
                Reference< XConversionDictionary >  xDic;
                if( ( aAny >>= xDic ) && xDic.is() )
                {
                    if( LANGUAGE_KOREAN == LanguageTag( xDic->getLocale() ).getLanguageType() )
                    {
                        m_aDictList.push_back( xDic );
                        AddDict( xDic->getName(), xDic->isActive() );
                    }
                }
            }
        }
        if (m_xDictsLB->n_children())
            m_xDictsLB->select(0);
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, OkHdl, weld::Button&, void)
    {
        sal_uInt32              nCnt = m_aDictList.size();
        sal_uInt32              n = 0;
        sal_uInt32              nActiveDics = 0;
        Sequence< OUString >    aActiveDics;

        aActiveDics.realloc( nCnt );
        OUString*               pActActiveDic = aActiveDics.getArray();

        while( nCnt )
        {
            Reference< XConversionDictionary >  xDict = m_aDictList[ n ];

            DBG_ASSERT( xDict.is(), "-HangulHanjaOptionsDialog::OkHdl(): someone is evaporated..." );

            bool bActive = m_xDictsLB->get_toggle(n, 0);
            xDict->setActive( bActive );
            Reference< util::XFlushable > xFlush( xDict, uno::UNO_QUERY );
            if( xFlush.is() )
                xFlush->flush();

            if( bActive )
            {
                pActActiveDic[ nActiveDics ] = xDict->getName();
                ++nActiveDics;
            }

            ++n;
            --nCnt;
        }

        // save configuration
        aActiveDics.realloc( nActiveDics );
        Any             aTmp;
        SvtLinguConfig  aLngCfg;
        aTmp <<= aActiveDics;
        aLngCfg.SetProperty( UPH_ACTIVE_CONVERSION_DICTIONARIES, aTmp );

        aTmp <<= m_xIgnorepostCB->get_active();
        aLngCfg.SetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD, aTmp );

        aTmp <<= m_xShowrecentlyfirstCB->get_active();
        aLngCfg.SetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST, aTmp );

        aTmp <<= m_xAutoreplaceuniqueCB->get_active();
        aLngCfg.SetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES, aTmp );

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DictsLB_SelectHdl, weld::TreeView&, void)
    {
        bool bSel = m_xDictsLB->get_selected_index() != -1;

        m_xEditPB->set_sensitive(bSel);
        m_xDeletePB->set_sensitive(bSel);
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, NewDictHdl, weld::Button&, void)
    {
        OUString                    aName;
        HangulHanjaNewDictDialog aNewDlg(m_xDialog.get());
        aNewDlg.run();
        if (aNewDlg.GetName(aName))
        {
            if( m_xConversionDictionaryList.is() )
            {
                try
                {
                    Reference< XConversionDictionary >  xDic =
                        m_xConversionDictionaryList->addNewDictionary( aName, LanguageTag::convertToLocale( LANGUAGE_KOREAN ), ConversionDictionaryType::HANGUL_HANJA );

                    if( xDic.is() )
                    {
                        //adapt local caches:
                        m_aDictList.push_back( xDic );
                        AddDict( xDic->getName(), xDic->isActive() );
                    }
                }
                catch( const ElementExistException& )
                {
                }
                catch( const NoSupportException& )
                {
                }
            }
        }
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, EditDictHdl, weld::Button&, void)
    {
        int nEntry = m_xDictsLB->get_selected_index();
        DBG_ASSERT(nEntry != -1, "+HangulHanjaEditDictDialog::EditDictHdl(): call of edit should not be possible with no selection!");
        if (nEntry != -1)
        {
            HangulHanjaEditDictDialog aEdDlg(m_xDialog.get(), m_aDictList, nEntry);
            aEdDlg.run();
        }
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DeleteDictHdl, weld::Button&, void)
    {
        int nSelPos = m_xDictsLB->get_selected_index();
        if (nSelPos != -1)
        {
            Reference< XConversionDictionary >  xDic( m_aDictList[ nSelPos ] );
            if( m_xConversionDictionaryList.is() && xDic.is() )
            {
                Reference< XNameContainer >     xNameCont = m_xConversionDictionaryList->getDictionaryContainer();
                if( xNameCont.is() )
                {
                    try
                    {
                        xNameCont->removeByName( xDic->getName() );

                        //adapt local caches:
                        m_aDictList.erase(m_aDictList.begin()+nSelPos );
                        m_xDictsLB->remove(nSelPos);
                    }
                    catch( const ElementExistException& )
                    {
                    }
                    catch( const NoSupportException& )
                    {
                    }
                }
            }
        }
    }

    HangulHanjaOptionsDialog::HangulHanjaOptionsDialog(weld::Window* pParent)
        : GenericDialogController(pParent, "cui/ui/hangulhanjaoptdialog.ui", "HangulHanjaOptDialog")
        , m_xDictsLB(m_xBuilder->weld_tree_view("dicts"))
        , m_xIgnorepostCB(m_xBuilder->weld_check_button("ignorepost"))
        , m_xShowrecentlyfirstCB(m_xBuilder->weld_check_button("showrecentfirst"))
        , m_xAutoreplaceuniqueCB(m_xBuilder->weld_check_button("autoreplaceunique"))
        , m_xNewPB(m_xBuilder->weld_button("new"))
        , m_xEditPB(m_xBuilder->weld_button("edit"))
        , m_xDeletePB(m_xBuilder->weld_button("delete"))
        , m_xOkPB(m_xBuilder->weld_button("ok"))
    {
        m_xDictsLB->set_size_request(m_xDictsLB->get_approximate_digit_width() * 32,
                                     m_xDictsLB->get_height_rows(5));

        std::vector<int> aWidths;
        aWidths.push_back(m_xDictsLB->get_checkbox_column_width());
        m_xDictsLB->set_column_fixed_widths(aWidths);

        m_xDictsLB->connect_changed( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );

        m_xOkPB->connect_clicked( LINK( this, HangulHanjaOptionsDialog, OkHdl ) );
        m_xNewPB->connect_clicked( LINK( this, HangulHanjaOptionsDialog, NewDictHdl ) );
        m_xEditPB->connect_clicked( LINK( this, HangulHanjaOptionsDialog, EditDictHdl ) );
        m_xDeletePB->connect_clicked( LINK( this, HangulHanjaOptionsDialog, DeleteDictHdl ) );

        SvtLinguConfig  aLngCfg;
        Any             aTmp;
        bool            bVal = bool();
        aTmp = aLngCfg.GetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD );
        if( aTmp >>= bVal )
            m_xIgnorepostCB->set_active( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST );
        if( aTmp >>= bVal )
            m_xShowrecentlyfirstCB->set_active( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES );
        if( aTmp >>= bVal )
            m_xAutoreplaceuniqueCB->set_active( bVal );

        Init();
    }

    HangulHanjaOptionsDialog::~HangulHanjaOptionsDialog()
    {
    }

    void HangulHanjaOptionsDialog::AddDict(const OUString& rName, bool bChecked)
    {
        m_xDictsLB->append();
        int nRow = m_xDictsLB->n_children() - 1;
        m_xDictsLB->set_toggle(nRow, bChecked, 0);
        m_xDictsLB->set_text(nRow, rName, 1);
        m_xDictsLB->set_id(nRow, rName);
    }

    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, OKHdl, weld::Button&, void)
    {
        OUString  aName(comphelper::string::stripEnd(m_xDictNameED->get_text(), ' '));

        m_bEntered = !aName.isEmpty();
        if (m_bEntered)
            m_xDictNameED->set_text(aName);     // do this in case of trailing chars have been deleted

        m_xDialog->response(RET_OK);
    }

    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, ModifyHdl, weld::Entry&, void)
    {
        OUString aName(comphelper::string::stripEnd(m_xDictNameED->get_text(), ' '));

        m_xOkBtn->set_sensitive(!aName.isEmpty());
    }

    HangulHanjaNewDictDialog::HangulHanjaNewDictDialog(weld::Window* pParent)
        : GenericDialogController(pParent, "cui/ui/hangulhanjaadddialog.ui", "HangulHanjaAddDialog")
        , m_bEntered(false)
        , m_xOkBtn(m_xBuilder->weld_button("ok"))
        , m_xDictNameED(m_xBuilder->weld_entry("entry"))
    {
        m_xOkBtn->connect_clicked( LINK( this, HangulHanjaNewDictDialog, OKHdl ) );
        m_xDictNameED->connect_changed( LINK( this, HangulHanjaNewDictDialog, ModifyHdl ) );
    }

    HangulHanjaNewDictDialog::~HangulHanjaNewDictDialog()
    {
    }

    bool HangulHanjaNewDictDialog::GetName( OUString& _rRetName ) const
    {
        if( m_bEntered )
            _rRetName = comphelper::string::stripEnd(m_xDictNameED->get_text(), ' ');

        return m_bEntered;
    }

    class SuggestionList
    {
    private:
    protected:
        std::vector<OUString> m_vElements;
        sal_uInt16          m_nNumOfEntries;
        // index of the internal iterator, used for First() and Next() methods
        sal_uInt16          m_nAct;

        const OUString*       Next_();
    public:
                            SuggestionList();
                            ~SuggestionList();

        void                Set( const OUString& _rElement, sal_uInt16 _nNumOfElement );
        void                Reset( sal_uInt16 _nNumOfElement );
        const OUString &    Get( sal_uInt16 _nNumOfElement ) const;
        void                Clear();

        const OUString*     First();
        const OUString*     Next();

        sal_uInt16   GetCount() const { return m_nNumOfEntries; }
    };

    SuggestionList::SuggestionList() :
        m_vElements(MAXNUM_SUGGESTIONS)
    {
        m_nAct = m_nNumOfEntries = 0;
    }

    SuggestionList::~SuggestionList()
    {
        Clear();
    }

    void SuggestionList::Set( const OUString& _rElement, sal_uInt16 _nNumOfElement )
    {
        m_vElements[_nNumOfElement] = _rElement;
        ++m_nNumOfEntries;
    }

    void SuggestionList::Reset( sal_uInt16 _nNumOfElement )
    {
        m_vElements[_nNumOfElement].clear();
        --m_nNumOfEntries;
    }

    const OUString& SuggestionList::Get( sal_uInt16 _nNumOfElement ) const
    {
        return m_vElements[_nNumOfElement];
    }

    void SuggestionList::Clear()
    {
        if( m_nNumOfEntries )
        {
            for (auto & vElement : m_vElements)
                vElement.clear();
            m_nNumOfEntries = m_nAct = 0;
        }
    }

    const OUString* SuggestionList::Next_()
    {
        while( m_nAct < m_vElements.size() )
        {
            auto & s = m_vElements[ m_nAct ];
            if (!s.isEmpty())
                return &s;
            ++m_nAct;
        }

        return nullptr;
    }

    const OUString* SuggestionList::First()
    {
        m_nAct = 0;
        return Next_();
    }

    const OUString* SuggestionList::Next()
    {
        const OUString*   pRet;

        if( m_nAct < m_nNumOfEntries )
        {
            ++m_nAct;
            pRet = Next_();
        }
        else
            pRet = nullptr;

        return pRet;
    }


    bool SuggestionEdit::ShouldScroll( bool _bUp ) const
    {
        bool    bRet = false;

        if( _bUp )
        {
            if( !m_pPrev )
                bRet = m_pScrollBar->vadjustment_get_value() > m_pScrollBar->vadjustment_get_lower();
        }
        else
        {
            if( !m_pNext )
                bRet = m_pScrollBar->vadjustment_get_value() < ( m_pScrollBar->vadjustment_get_upper() - 4 );
        }

        return bRet;
    }

    void SuggestionEdit::DoJump( bool _bUp )
    {
        m_pScrollBar->vadjustment_set_value( m_pScrollBar->vadjustment_get_value() + ( _bUp? -1 : 1 ) );
        m_pParent->UpdateScrollbar();
    }

    SuggestionEdit::SuggestionEdit(std::unique_ptr<weld::Entry> xEntry, HangulHanjaEditDictDialog* pParent)
        : m_pParent(pParent)
        , m_pPrev(nullptr)
        , m_pNext(nullptr)
        , m_pScrollBar(nullptr)
        , m_xEntry(std::move(xEntry))
    {
        m_xEntry->connect_key_press(LINK(this, SuggestionEdit, KeyInputHdl));
    }

    IMPL_LINK(SuggestionEdit, KeyInputHdl, const KeyEvent&, rKEvt, bool)
    {
        bool bHandled = false;

        const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
        sal_uInt16          nMod = rKeyCode.GetModifier();
        sal_uInt16          nCode = rKeyCode.GetCode();
        if( nCode == KEY_TAB && ( !nMod || KEY_SHIFT == nMod ) )
        {
            bool        bUp = KEY_SHIFT == nMod;
            if( ShouldScroll( bUp ) )
            {
                DoJump( bUp );
                m_xEntry->select_region(0, -1);
                    // Tab-travel doesn't really happen, so emulate it by setting a selection manually
                bHandled = true;
            }
        }
        else if( KEY_UP == nCode || KEY_DOWN == nCode )
        {
            bool        bUp = KEY_UP == nCode;
            if( ShouldScroll( bUp ) )
            {
                DoJump( bUp );
                bHandled = true;
            }
            else if( bUp )
            {
                if( m_pPrev )
                {
                    m_pPrev->grab_focus();
                    bHandled = true;
                }
            }
            else if( m_pNext )
            {
                m_pNext->grab_focus();
                bHandled = true;
            }
        }

        return bHandled;
    }

    void SuggestionEdit::init(weld::ScrolledWindow* pScrollBar, SuggestionEdit* pPrev, SuggestionEdit* pNext)
    {
        m_pScrollBar = pScrollBar;
        m_pPrev = pPrev;
        m_pNext = pNext;
    }

    namespace
    {
        bool GetConversions(    const Reference< XConversionDictionary >&  _xDict,
                                const OUString& _rOrg,
                                Sequence< OUString >& _rEntries )
        {
            bool    bRet = false;
            if( _xDict.is() && !_rOrg.isEmpty() )
            {
                try
                {
                    _rEntries = _xDict->getConversions( _rOrg,
                                                        0,
                                                        _rOrg.getLength(),
                                                        ConversionDirection_FROM_LEFT,
                                                        css::i18n::TextConversionOption::NONE );
                    bRet = _rEntries.getLength() > 0;
                }
                catch( const IllegalArgumentException& )
                {
                }
            }

            return bRet;
        }
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, ScrollHdl, weld::ScrolledWindow&, void )
    {
        UpdateScrollbar();
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, OriginalModifyHdl, weld::ComboBox&, void )
    {
        m_bModifiedOriginal = true;
        m_aOriginal = comphelper::string::stripEnd( m_xOriginalLB->get_active_text(), ' ' );

        UpdateSuggestions();
        UpdateButtonStates();
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl1, weld::Entry&, rEdit, void )
    {
        EditModify( &rEdit, 0 );
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl2, weld::Entry&, rEdit, void )
    {
        EditModify( &rEdit, 1 );
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl3, weld::Entry&, rEdit, void )
    {
        EditModify( &rEdit, 2 );
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl4, weld::Entry&, rEdit, void )
    {
        EditModify( &rEdit, 3 );
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, BookLBSelectHdl, weld::ComboBox&, void )
    {
        InitEditDictDialog( m_xBookLB->get_active() );
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, NewPBPushHdl, weld::Button&, void )
    {
        DBG_ASSERT( m_xSuggestions, "-HangulHanjaEditDictDialog::NewPBPushHdl(): no suggestions... search in hell..." );
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() && m_xSuggestions )
        {
            //delete old entry
            bool bRemovedSomething = DeleteEntryFromDictionary( xDict );

            OUString                aLeft( m_aOriginal );
            const OUString*           pRight = m_xSuggestions->First();
            bool bAddedSomething = false;
            while( pRight )
            {
                try
                {
                    //add new entry
                    xDict->addEntry( aLeft, *pRight );
                    bAddedSomething = true;
                }
                catch( const IllegalArgumentException& )
                {
                }
                catch( const ElementExistException& )
                {
                }

                pRight = m_xSuggestions->Next();
            }

            if( bAddedSomething || bRemovedSomething )
                InitEditDictDialog( m_nCurrentDict );
        }
        else
        {
            SAL_INFO( "cui.dialogs", "dictionary faded away..." );
        }
    }

    bool HangulHanjaEditDictDialog::DeleteEntryFromDictionary( const Reference< XConversionDictionary >& xDict  )
    {
        bool bRemovedSomething = false;
        if( xDict.is() )
        {
            OUString                aOrg( m_aOriginal );
            Sequence< OUString >    aEntries;
            GetConversions( xDict, m_aOriginal, aEntries );

            sal_uInt32  n = aEntries.getLength();
            OUString*   pEntry = aEntries.getArray();
            while( n )
            {
                try
                {
                    xDict->removeEntry( aOrg, *pEntry );
                    bRemovedSomething = true;
                }
                catch( const NoSuchElementException& )
                {   // can not be...
                }

                ++pEntry;
                --n;
            }
        }
        return bRemovedSomething;
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, DeletePBPushHdl, weld::Button&, void )
    {
        if( DeleteEntryFromDictionary( m_rDictList[ m_nCurrentDict ] ) )
        {
            m_aOriginal.clear();
            m_bModifiedOriginal = true;
            InitEditDictDialog( m_nCurrentDict );
        }
    }

    void HangulHanjaEditDictDialog::InitEditDictDialog( sal_uInt32 nSelDict )
    {
        if( m_xSuggestions )
            m_xSuggestions->Clear();

        if( m_nCurrentDict != nSelDict )
        {
            m_nCurrentDict = nSelDict;
            m_aOriginal.clear();
            m_bModifiedOriginal = true;
        }

        UpdateOriginalLB();

        m_xOriginalLB->set_entry_text( !m_aOriginal.isEmpty() ? m_aOriginal : m_aEditHintText);
        m_xOriginalLB->select_entry_region(0, -1);
        m_xOriginalLB->grab_focus();

        UpdateSuggestions();
        UpdateButtonStates();
    }

    void HangulHanjaEditDictDialog::UpdateOriginalLB()
    {
        m_xOriginalLB->clear();
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() )
        {
            Sequence< OUString >    aEntries = xDict->getConversionEntries( ConversionDirection_FROM_LEFT );
            sal_uInt32              n = aEntries.getLength();
            OUString*               pEntry = aEntries.getArray();
            while( n )
            {
                m_xOriginalLB->append_text( *pEntry );

                ++pEntry;
                --n;
            }
        }
        else
        {
            SAL_INFO( "cui.dialogs", "dictionary faded away..." );
        }
    }

    void HangulHanjaEditDictDialog::UpdateButtonStates()
    {
        bool bHaveValidOriginalString = !m_aOriginal.isEmpty() && m_aOriginal != m_aEditHintText;
        bool bNew = bHaveValidOriginalString && m_xSuggestions && m_xSuggestions->GetCount() > 0;
        bNew = bNew && ( m_bModifiedSuggestions || m_bModifiedOriginal );

        m_xNewPB->set_sensitive( bNew );
        m_xDeletePB->set_sensitive(!m_bModifiedOriginal && bHaveValidOriginalString);
    }

    void HangulHanjaEditDictDialog::UpdateSuggestions()
    {
        Sequence< OUString > aEntries;
        bool bFound = GetConversions( m_rDictList[ m_nCurrentDict ], m_aOriginal, aEntries );
        if( bFound )
        {
            m_bModifiedOriginal = false;

            if( m_xSuggestions )
                m_xSuggestions->Clear();

            //fill found entries into boxes
            sal_uInt32 nCnt = aEntries.getLength();
            if( nCnt )
            {
                if( !m_xSuggestions )
                    m_xSuggestions.reset(new SuggestionList);

                const OUString* pSugg = aEntries.getConstArray();
                sal_uInt32 n = 0;
                while( nCnt )
                {
                    m_xSuggestions->Set( pSugg[ n ], sal_uInt16( n ) );
                    ++n;
                    --nCnt;
                }
            }
            m_bModifiedSuggestions = false;
        }

        m_xScrollSB->vadjustment_set_value( 0 );
        UpdateScrollbar();              // will force edits to be filled new
    }

    void HangulHanjaEditDictDialog::SetEditText(SuggestionEdit& rEdit, sal_uInt16 nEntryNum)
    {
        OUString  aStr;
        if( m_xSuggestions )
        {
            aStr = m_xSuggestions->Get(nEntryNum);
        }

        rEdit.set_text(aStr);
    }

    void HangulHanjaEditDictDialog::EditModify(const weld::Entry* pEdit, sal_uInt8 _nEntryOffset)
    {
        m_bModifiedSuggestions = true;

        OUString  aTxt( pEdit->get_text() );
        sal_uInt16 nEntryNum = m_nTopPos + _nEntryOffset;
        if( aTxt.isEmpty() )
        {
            //reset suggestion
            if( m_xSuggestions )
                m_xSuggestions->Reset( nEntryNum );
        }
        else
        {
            //set suggestion
            if( !m_xSuggestions )
                m_xSuggestions.reset(new SuggestionList);
            m_xSuggestions->Set( aTxt, nEntryNum );
        }

        UpdateButtonStates();
    }

    HangulHanjaEditDictDialog::HangulHanjaEditDictDialog(weld::Window* pParent, HHDictList& _rDictList, sal_uInt32 nSelDict)
        : GenericDialogController(pParent, "cui/ui/hangulhanjaeditdictdialog.ui", "HangulHanjaEditDictDialog")
        , m_aEditHintText        ( CuiResId(RID_SVXSTR_EDITHINT) )
        , m_rDictList            ( _rDictList )
        , m_nCurrentDict         ( 0xFFFFFFFF )
        , m_nTopPos              ( 0 )
        , m_bModifiedSuggestions ( false )
        , m_bModifiedOriginal    ( false )
        , m_xBookLB(m_xBuilder->weld_combo_box("book"))
        , m_xOriginalLB(m_xBuilder->weld_combo_box("original"))
        , m_xEdit1(new SuggestionEdit(m_xBuilder->weld_entry("edit1"), this))
        , m_xEdit2(new SuggestionEdit(m_xBuilder->weld_entry("edit2"), this))
        , m_xEdit3(new SuggestionEdit(m_xBuilder->weld_entry("edit3"), this))
        , m_xEdit4(new SuggestionEdit(m_xBuilder->weld_entry("edit4"), this))
        , m_xContents(m_xBuilder->weld_widget("box"))
        , m_xScrollSB(m_xBuilder->weld_scrolled_window("scrollbar"))
        , m_xNewPB(m_xBuilder->weld_button("new"))
        , m_xDeletePB(m_xBuilder->weld_button("delete"))
    {
        m_xScrollSB->set_user_managed_scrolling();

        Size aSize(m_xContents->get_preferred_size());
        m_xScrollSB->set_size_request(-1, aSize.Height());

        m_xEdit1->init( m_xScrollSB.get(), nullptr, m_xEdit2.get() );
        m_xEdit2->init( m_xScrollSB.get(), m_xEdit1.get(), m_xEdit3.get() );
        m_xEdit3->init( m_xScrollSB.get(), m_xEdit2.get(), m_xEdit4.get() );
        m_xEdit4->init( m_xScrollSB.get(), m_xEdit3.get(), nullptr );

        m_xOriginalLB->connect_changed( LINK( this, HangulHanjaEditDictDialog, OriginalModifyHdl ) );

        m_xNewPB->connect_clicked( LINK( this, HangulHanjaEditDictDialog, NewPBPushHdl ) );
        m_xNewPB->set_sensitive( false );

        m_xDeletePB->connect_clicked( LINK( this, HangulHanjaEditDictDialog, DeletePBPushHdl ) );
        m_xDeletePB->set_sensitive( false );

        static_assert(MAXNUM_SUGGESTIONS >= 5, "number of suggestions should not under-run the value of 5");

        // 4 here, because we have 4 edits / page
        m_xScrollSB->vadjustment_configure(0, 0, MAXNUM_SUGGESTIONS, 1, 4, 4);
        m_xScrollSB->connect_vadjustment_changed(LINK(this, HangulHanjaEditDictDialog, ScrollHdl));

        m_xEdit1->connect_changed( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl1 ) );
        m_xEdit2->connect_changed( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl2 ) );
        m_xEdit3->connect_changed( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl3 ) );
        m_xEdit4->connect_changed( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl4 ) );

        m_xBookLB->connect_changed( LINK( this, HangulHanjaEditDictDialog, BookLBSelectHdl ) );
        sal_uInt32  nDictCnt = m_rDictList.size();
        for( sal_uInt32 n = 0 ; n < nDictCnt ; ++n )
        {
            Reference< XConversionDictionary >  xDic( m_rDictList[n] );
            OUString aName;
            if( xDic.is() )
                aName = xDic->getName();
            m_xBookLB->append_text( aName );
        }
        m_xBookLB->set_active(nSelDict);

        InitEditDictDialog(nSelDict);
    }

    HangulHanjaEditDictDialog::~HangulHanjaEditDictDialog()
    {
    }

    void HangulHanjaEditDictDialog::UpdateScrollbar()
    {
        sal_uInt16  nPos = m_xScrollSB->vadjustment_get_value();
        m_nTopPos = nPos;

        SetEditText( *m_xEdit1, nPos++ );
        SetEditText( *m_xEdit2, nPos++ );
        SetEditText( *m_xEdit3, nPos++ );
        SetEditText( *m_xEdit4, nPos );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
