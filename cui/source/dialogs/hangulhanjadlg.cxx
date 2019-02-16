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
#include <vcl/controllayout.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/decoview.hxx>
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
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>

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
        void Paint( OutputDevice& _rDevice, const ::tools::Rectangle& _rRect, DrawTextFlags _nTextStyle,
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

    class RubyRadioButton : public RadioButton
    {

    public:
        RubyRadioButton( vcl::Window* _pParent, WinBits nBits );
        void init( const OUString& rPrimaryText, const OUString& rSecondaryText, const PseudoRubyText::RubyPosition& rPosition );
        virtual Size    GetOptimalSize() const override;

    protected:
        virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& _rRect ) override;

    private:
        PseudoRubyText m_aRubyText;
    };

    RubyRadioButton::RubyRadioButton( vcl::Window* _pParent, WinBits nBits )
        :RadioButton( _pParent, nBits )
    {
    }

    void RubyRadioButton::init( const OUString& rPrimaryText, const OUString& rSecondaryText, const PseudoRubyText::RubyPosition& rPosition )
    {
        m_aRubyText.init( rPrimaryText, rSecondaryText, rPosition );
    }


    void RubyRadioButton::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
    {
        HideFocus();

        // calculate the size of the radio image - we're to paint our text _after_ this image
        DBG_ASSERT( !GetModeRadioImage(), "RubyRadioButton::Paint: images not supported!" );
        Size aImageSize = GetRadioImage(rRenderContext.GetSettings(), DrawButtonFlags::NONE).GetSizePixel();
        aImageSize.setWidth( CalcZoom( aImageSize.Width() ) + 2 );   // + 2 because otherwise the radiobuttons
        aImageSize.setHeight( CalcZoom( aImageSize.Height() ) + 2 ); // appear a bit cut from right and top.

        ::tools::Rectangle aOverallRect( Point( 0, 0 ), GetOutputSizePixel() );
        aOverallRect.AdjustLeft(aImageSize.Width() + 4 );  // 4 is the separator between the image and the text
        // inflate the rect a little bit (because the VCL radio button does the same)
        ::tools::Rectangle aTextRect( aOverallRect );
        aTextRect.AdjustLeft( 1 ); aTextRect.AdjustRight( -1 );
        aTextRect.AdjustTop( 1 ); aTextRect.AdjustBottom( -1 );

        // calculate the text flags for the painting
        DrawTextFlags nTextStyle = DrawTextFlags::Mnemonic;
        WinBits nStyle = GetStyle( );

        // the horizontal alignment
        if ( nStyle & WB_RIGHT )
            nTextStyle |= DrawTextFlags::Right;
        else if ( nStyle & WB_CENTER )
            nTextStyle |= DrawTextFlags::Center;
        else
            nTextStyle |= DrawTextFlags::Left;
        // the vertical alignment
        if ( nStyle & WB_BOTTOM )
            nTextStyle |= DrawTextFlags::Bottom;
        else if ( nStyle & WB_VCENTER )
            nTextStyle |= DrawTextFlags::VCenter;
        else
            nTextStyle |= DrawTextFlags::Top;
        // mnemonics
        if ( 0 == ( nStyle & WB_NOLABEL ) )
            nTextStyle |= DrawTextFlags::Mnemonic;

        // paint the ruby text
        ::tools::Rectangle aPrimaryTextLocation;
        ::tools::Rectangle aSecondaryTextLocation;

        m_aRubyText.Paint(rRenderContext, aTextRect, nTextStyle, &aPrimaryTextLocation, &aSecondaryTextLocation);

        // the focus rectangle is to be painted around both texts
        ::tools::Rectangle aCombinedRect(aPrimaryTextLocation);
        aCombinedRect.Union(aSecondaryTextLocation);
        SetFocusRect(aCombinedRect);

        // let the base class paint the radio button
        // for this, give it the proper location to paint the image (vertically centered, relative to our text)
        ::tools::Rectangle aImageLocation( Point( 0, 0 ), aImageSize );
        sal_Int32 nTextHeight = aSecondaryTextLocation.Bottom() - aPrimaryTextLocation.Top();
        aImageLocation.SetTop( aPrimaryTextLocation.Top() + ( nTextHeight - aImageSize.Height() ) / 2 );
        aImageLocation.SetBottom( aImageLocation.Top() + aImageSize.Height() );
        SetStateRect( aImageLocation );
        DrawRadioButtonState(rRenderContext);

        // mouse clicks should be recognized in a rect which is one pixel larger in each direction, plus
        // includes the image
        aCombinedRect.SetLeft( aImageLocation.Left() );
        aCombinedRect.AdjustRight( 1 );
        aCombinedRect.AdjustTop( -1 );
        aCombinedRect.AdjustBottom( 1 );

        SetMouseRect(aCombinedRect);

        // paint the focus rect, if necessary
        if (HasFocus())
            ShowFocus(aTextRect);
    }

    Size RubyRadioButton::GetOptimalSize() const
    {
        vcl::Font aSmallerFont( GetFont() );
        aSmallerFont.SetFontHeight( static_cast<long>( 0.8 * aSmallerFont.GetFontHeight() ) );
        ::tools::Rectangle rect( Point(), Size( SAL_MAX_INT32, SAL_MAX_INT32 ) );

        Size aPrimarySize = GetTextRect( rect, m_aRubyText.getPrimaryText() ).GetSize();
        Size aSecondarySize;
        {
            FontSwitch aFontRestore( const_cast<RubyRadioButton&>(*this), aSmallerFont );
            aSecondarySize = GetTextRect( rect, m_aRubyText.getSecondaryText() ).GetSize();
        }

        Size minimumSize =  CalcMinimumSize();
        minimumSize.setHeight( aPrimarySize.Height() + aSecondarySize.Height() + 5 );
        minimumSize.setWidth( aPrimarySize.Width() + aSecondarySize.Width() + 5 );
        return minimumSize;
    }

    VCL_BUILDER_FACTORY_ARGS(RubyRadioButton, WB_LEFT|WB_VCENTER)

    SuggestionSet::SuggestionSet( vcl::Window* pParent )
                    : ValueSet( pParent, pParent->GetStyle() | WB_BORDER )

    {
    }

    SuggestionSet::~SuggestionSet()
    {
        disposeOnce();
    }

    void SuggestionSet::dispose()
    {
        ClearSet();
        ValueSet::dispose();
    }

    void SuggestionSet::UserDraw( const UserDrawEvent& rUDEvt )
    {
        vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
        ::tools::Rectangle aRect = rUDEvt.GetRect();
        sal_uInt16  nItemId = rUDEvt.GetItemId();

        OUString sText = *static_cast< OUString* >( GetItemData( nItemId ) );
        pDev->DrawText( aRect, sText, DrawTextFlags::Center | DrawTextFlags::VCenter );
    }

    void SuggestionSet::ClearSet()
    {
        sal_uInt16 i, nCount = GetItemCount();
        for ( i = 0; i < nCount; ++i )
            delete static_cast< OUString* >( GetItemData(i) );
        Clear();
    }

    SuggestionDisplay::SuggestionDisplay( vcl::Window* pParent, WinBits nBits )
        : Control( pParent, nBits )
        , m_bDisplayListBox( true )
        , m_aValueSet( VclPtr<SuggestionSet>::Create(this) )
        , m_aListBox( VclPtr<ListBox>::Create(this,GetStyle() | WB_BORDER) )
        , m_bInSelectionUpdate( false )
    {
        m_aValueSet->SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionValueSetHdl ) );
        m_aListBox->SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionListBoxHdl ) );

        m_aValueSet->SetLineCount( LINE_CNT );
        m_aValueSet->SetStyle( m_aValueSet->GetStyle() | WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL );
        m_aValueSet->SetBorderStyle( WindowBorderStyle::MONO );
        OUString const aOneCharacter("AU");
        long nItemWidth = 2*GetTextWidth( aOneCharacter );
        m_aValueSet->SetItemWidth( nItemWidth );

        Size aSize( approximate_char_width() * 48, GetTextHeight() * 5 );
        m_aValueSet->SetSizePixel( aSize );
        m_aListBox->SetSizePixel( aSize );

        implUpdateDisplay();
    }

    SuggestionDisplay::~SuggestionDisplay()
    {
        disposeOnce();
    }

    void SuggestionDisplay::dispose()
    {
        m_aValueSet.disposeAndClear();
        m_aListBox.disposeAndClear();
        Control::dispose();
    }

    void SuggestionDisplay::implUpdateDisplay()
    {
        bool bShowBox = IsVisible() && m_bDisplayListBox;
        bool bShowSet = IsVisible() && !m_bDisplayListBox;

        m_aListBox->Show( bShowBox );
        m_aValueSet->Show( bShowSet );
    }

    void SuggestionDisplay::StateChanged( StateChangedType nStateChange )
    {
        if( StateChangedType::Visible == nStateChange )
            implUpdateDisplay();
    }

    bool SuggestionDisplay::hasCurrentControl()
    {
        return m_bDisplayListBox || m_aValueSet;
    }

    Control& SuggestionDisplay::implGetCurrentControl()
    {
        if( m_bDisplayListBox )
            return *m_aListBox.get();
        return *m_aValueSet.get();
    }

    void SuggestionDisplay::KeyInput( const KeyEvent& rKEvt )
    {
        implGetCurrentControl().KeyInput( rKEvt );
    }
    void SuggestionDisplay::KeyUp( const KeyEvent& rKEvt )
    {
        implGetCurrentControl().KeyUp( rKEvt );
    }
    void SuggestionDisplay::Activate()
    {
        implGetCurrentControl().Activate();
    }
    void SuggestionDisplay::Deactivate()
    {
        implGetCurrentControl().Deactivate();
    }
    void SuggestionDisplay::GetFocus()
    {
        if (hasCurrentControl())
            implGetCurrentControl().GetFocus();
        else
            Control::LoseFocus();
    }
    void SuggestionDisplay::LoseFocus()
    {
        if (hasCurrentControl())
            implGetCurrentControl().LoseFocus();
        else
            Control::LoseFocus();
    }
    void SuggestionDisplay::Command( const CommandEvent& rCEvt )
    {
        implGetCurrentControl().Command( rCEvt );
    }

    void SuggestionDisplay::DisplayListBox( bool bDisplayListBox )
    {
        if( m_bDisplayListBox != bDisplayListBox )
        {
            Control& rOldControl = implGetCurrentControl();
            bool bHasFocus = rOldControl.HasFocus();

            m_bDisplayListBox = bDisplayListBox;

            if( bHasFocus )
            {
                Control& rNewControl = implGetCurrentControl();
                rNewControl.GrabFocus();
            }

            implUpdateDisplay();
        }
    }

    IMPL_LINK( SuggestionDisplay, SelectSuggestionValueSetHdl, ValueSet*, pControl, void )
    {
        SelectSuggestionHdl(pControl);
    }
    IMPL_LINK( SuggestionDisplay, SelectSuggestionListBoxHdl, ListBox&, rControl, void )
    {
        SelectSuggestionHdl(&rControl);
    }
    void SuggestionDisplay::SelectSuggestionHdl( Control const * pControl )
    {
        if( m_bInSelectionUpdate )
            return;

        m_bInSelectionUpdate = true;
        if( pControl == m_aListBox.get() )
        {
            sal_uInt16 nPos = m_aListBox->GetSelectedEntryPos();
            m_aValueSet->SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
        }
        else
        {
            sal_uInt16 nPos = m_aValueSet->GetSelectedItemId()-1; //itemid == pos+1 (id 0 has special meaning)
            m_aListBox->SelectEntryPos( nPos );
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
        m_aListBox->Clear();
        m_aValueSet->Clear();
    }
    void SuggestionDisplay::InsertEntry( const OUString& rStr )
    {
        sal_uInt16 nItemId = m_aListBox->InsertEntry( rStr ) + 1; //itemid == pos+1 (id 0 has special meaning)
        m_aValueSet->InsertItem( nItemId );
        OUString* pItemData = new OUString( rStr );
        m_aValueSet->SetItemData( nItemId, pItemData );
    }
    void SuggestionDisplay::SelectEntryPos( sal_uInt16 nPos )
    {
        m_aListBox->SelectEntryPos( nPos );
        m_aValueSet->SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
    }
    sal_uInt16 SuggestionDisplay::GetEntryCount() const
    {
        return m_aListBox->GetEntryCount();
    }
    OUString SuggestionDisplay::GetEntry( sal_uInt16 nPos ) const
    {
        return m_aListBox->GetEntry( nPos );
    }
    OUString SuggestionDisplay::GetSelectedEntry() const
    {
        return m_aListBox->GetSelectedEntry();
    }
    void SuggestionDisplay::SetHelpIds()
    {
        SetHelpId( HID_HANGULDLG_SUGGESTIONS );
        m_aValueSet->SetHelpId( HID_HANGULDLG_SUGGESTIONS_GRID );
        m_aListBox->SetHelpId( HID_HANGULDLG_SUGGESTIONS_LIST );
    }

    VCL_BUILDER_FACTORY_ARGS( SuggestionDisplay, WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL );

    HangulHanjaConversionDialog::HangulHanjaConversionDialog( vcl::Window* _pParent, HHC::ConversionDirection _ePrimaryDirection )
        :ModalDialog( _pParent, "HangulHanjaConversionDialog", "cui/ui/hangulhanjaconversiondialog.ui" )
        ,m_pIgnoreNonPrimary( nullptr )
        ,m_bDocumentMode( true )
    {
        get( m_pFind, "find" );
        get( m_pIgnore, "ignore" );
        get( m_pSuggestions, "suggestions" );
        get( m_pSimpleConversion, "simpleconversion" );
        get( m_pHangulBracketed, "hangulbracket" );
        get( m_pHanjaBracketed, "hanjabracket" );
        get( m_pHangulOnly, "hangulonly" );
        get( m_pHanjaOnly, "hanjaonly" );
        get( m_pReplaceByChar, "replacebychar" );
        get( m_pOptions, "options" );
        get( m_pIgnore, "ignore" );
        get( m_pIgnoreAll, "ignoreall" );
        get( m_pReplace, "replace" );
        get( m_pReplaceAll, "replaceall" );
        get( m_pWordInput, "wordinput" );
        get( m_pOriginalWord, "originalword" );
        get( m_pHanjaAbove, "hanja_above" );
        get( m_pHanjaBelow, "hanja_below" );
        get( m_pHangulAbove, "hangul_above" );
        get( m_pHangulBelow, "hangul_below" );

        m_pSuggestions->set_height_request( m_pSuggestions->GetTextHeight() * 5 );
        m_pSuggestions->set_width_request( m_pSuggestions->approximate_char_width() * 48 );

        const OUString sHangul(CuiResId(RID_SVXSTR_HANGUL));
        const OUString sHanja(CuiResId(RID_SVXSTR_HANJA));
        m_pHanjaAbove->init( sHangul, sHanja, PseudoRubyText::eAbove );
        m_pHanjaBelow->init( sHangul, sHanja, PseudoRubyText::eBelow );
        m_pHangulAbove->init( sHanja, sHangul, PseudoRubyText::eAbove );
        m_pHangulBelow->init( sHanja, sHangul, PseudoRubyText::eBelow );

        m_pWordInput->SetModifyHdl( LINK( this,  HangulHanjaConversionDialog, OnSuggestionModified ) );
        m_pSuggestions->SetSelectHdl( LINK( this,  HangulHanjaConversionDialog, OnSuggestionSelected ) );
        m_pReplaceByChar->SetClickHdl( LINK( this, HangulHanjaConversionDialog, ClickByCharacterHdl ) );
        m_pHangulOnly->SetClickHdl( LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );
        m_pHanjaOnly->SetClickHdl(  LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );
        m_pOptions->SetClickHdl( LINK( this, HangulHanjaConversionDialog, OnOption ) );

        if ( editeng::HangulHanjaConversion::eHangulToHanja == _ePrimaryDirection )
        {
            m_pIgnoreNonPrimary = m_pHangulOnly;
        }
        else
        {
            m_pIgnoreNonPrimary = m_pHanjaOnly;
        }

        // initial focus
        FocusSuggestion( );

        // initial control values
        m_pSimpleConversion->Check();

        m_pSuggestions->SetHelpIds();
    }

    HangulHanjaConversionDialog::~HangulHanjaConversionDialog()
    {
        disposeOnce();
    }

    void HangulHanjaConversionDialog::dispose()
    {
        m_pFind.clear();
        m_pIgnore.clear();
        m_pIgnoreAll.clear();
        m_pReplace.clear();
        m_pReplaceAll.clear();
        m_pOptions.clear();
        m_pSuggestions.clear();
        m_pSimpleConversion.clear();
        m_pHangulBracketed.clear();
        m_pHanjaBracketed.clear();
        m_pWordInput.clear();
        m_pOriginalWord.clear();
        m_pHanjaAbove.clear();
        m_pHanjaBelow.clear();
        m_pHangulAbove.clear();
        m_pHangulBelow.clear();
        m_pHangulOnly.clear();
        m_pHanjaOnly.clear();
        m_pReplaceByChar.clear();
        m_pIgnoreNonPrimary.clear();
        ModalDialog::dispose();
    }

    void HangulHanjaConversionDialog::FillSuggestions( const css::uno::Sequence< OUString >& _rSuggestions )
    {
        m_pSuggestions->Clear();
        for ( auto const & suggestion : _rSuggestions )
            m_pSuggestions->InsertEntry( suggestion );

        // select the first suggestion, and fill in the suggestion edit field
        OUString sFirstSuggestion;
        if ( m_pSuggestions->GetEntryCount() )
        {
            sFirstSuggestion = m_pSuggestions->GetEntry( 0 );
            m_pSuggestions->SelectEntryPos( 0 );
        }
        m_pWordInput->SetText( sFirstSuggestion );
        m_pWordInput->SaveValue();
        OnSuggestionModified( *m_pWordInput );
    }


    void HangulHanjaConversionDialog::SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl )
    {
        m_aOptionsChangedLink = _rHdl;
    }


    void HangulHanjaConversionDialog::SetIgnoreHdl( const Link<Button*,void>& _rHdl )
    {
        m_pIgnore->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetIgnoreAllHdl( const Link<Button*,void>& _rHdl )
    {
        m_pIgnoreAll->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetChangeHdl( const Link<Button*,void>& _rHdl )
    {
        m_pReplace->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetChangeAllHdl( const Link<Button*,void>& _rHdl )
    {
        m_pReplaceAll->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetFindHdl( const Link<Button*,void>& _rHdl )
    {
        m_pFind->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetConversionFormatChangedHdl( const Link<Button*,void>& _rHdl )
    {
        m_pSimpleConversion->SetClickHdl( _rHdl );
        m_pHangulBracketed->SetClickHdl( _rHdl );
        m_pHanjaBracketed->SetClickHdl( _rHdl );
        m_pHanjaAbove->SetClickHdl( _rHdl );
        m_pHanjaBelow->SetClickHdl( _rHdl );
        m_pHangulAbove->SetClickHdl( _rHdl );
        m_pHangulBelow->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetClickByCharacterHdl( const Link<CheckBox*,void>& _rHdl )
    {
        m_aClickByCharacterLink = _rHdl;
    }


    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionSelected, SuggestionDisplay&, void )
    {
        m_pWordInput->SetText( m_pSuggestions->GetSelectedEntry() );
        OnSuggestionModified( *m_pWordInput );
    }


    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionModified, Edit&, void )
    {
        m_pFind->Enable( m_pWordInput->IsValueChangedFromSaved() );

        bool bSameLen = m_pWordInput->GetText().getLength() == m_pOriginalWord->GetText().getLength();
        m_pReplace->Enable( m_bDocumentMode && bSameLen );
        m_pReplaceAll->Enable( m_bDocumentMode && bSameLen );
    }


    IMPL_LINK( HangulHanjaConversionDialog, ClickByCharacterHdl, Button*, pBox, void )
    {
        m_aClickByCharacterLink.Call( static_cast<CheckBox*>(pBox) );

        bool bByCharacter = static_cast<CheckBox*>(pBox)->IsChecked();
        m_pSuggestions->DisplayListBox( !bByCharacter );
    }


    IMPL_LINK( HangulHanjaConversionDialog, OnConversionDirectionClicked, Button *, pBox, void )
    {
        CheckBox *pOtherBox = nullptr;
        if ( pBox == m_pHangulOnly )
            pOtherBox = m_pHanjaOnly;
        else if ( pBox == m_pHanjaOnly )
            pOtherBox = m_pHangulOnly;
        if ( pBox && pOtherBox )
        {
            bool bBoxChecked = static_cast<CheckBox*>(pBox)->IsChecked();
            if ( bBoxChecked )
                pOtherBox->Check( false );
            pOtherBox->Enable( !bBoxChecked );
        }
    }

    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnOption, Button*, void )
    {
        ScopedVclPtrInstance< HangulHanjaOptionsDialog > aOptDlg(this);
        aOptDlg->Execute();
        m_aOptionsChangedLink.Call( nullptr );
    }


    OUString HangulHanjaConversionDialog::GetCurrentString( ) const
    {
        return m_pOriginalWord->GetText( );
    }


    void HangulHanjaConversionDialog::FocusSuggestion( )
    {
        m_pWordInput->GrabFocus();
    }


    namespace
    {
        void lcl_modifyWindowStyle( vcl::Window* _pWin, WinBits _nSet, WinBits _nReset )
        {
            DBG_ASSERT( 0 == ( _nSet & _nReset ), "lcl_modifyWindowStyle: set _and_ reset the same bit?" );
            if ( _pWin )
                _pWin->SetStyle( ( _pWin->GetStyle() | _nSet ) & ~_nReset );
        }
    }


    void HangulHanjaConversionDialog::SetCurrentString( const OUString& _rNewString,
        const Sequence< OUString >& _rSuggestions, bool _bOriginatesFromDocument )
    {
        m_pOriginalWord->SetText( _rNewString );

        bool bOldDocumentMode = m_bDocumentMode;
        m_bDocumentMode = _bOriginatesFromDocument; // before FillSuggestions!
        FillSuggestions( _rSuggestions );

        m_pIgnoreAll->Enable( m_bDocumentMode );

        // switch the def button depending if we're working for document text
        if ( bOldDocumentMode != m_bDocumentMode )
        {
            vcl::Window* pOldDefButton = nullptr;
            vcl::Window* pNewDefButton = nullptr;
            if ( m_bDocumentMode )
            {
                pOldDefButton = m_pFind;
                pNewDefButton = m_pReplace;
            }
            else
            {
                pOldDefButton = m_pReplace;
                pNewDefButton = m_pFind;
            }

            DBG_ASSERT( WB_DEFBUTTON == ( pOldDefButton->GetStyle( ) & WB_DEFBUTTON ),
                "HangulHanjaConversionDialog::SetCurrentString: wrong previous default button (1)!" );
            DBG_ASSERT( 0 == ( pNewDefButton->GetStyle( ) & WB_DEFBUTTON ),
                "HangulHanjaConversionDialog::SetCurrentString: wrong previous default button (2)!" );

            lcl_modifyWindowStyle( pOldDefButton, 0, WB_DEFBUTTON );
            lcl_modifyWindowStyle( pNewDefButton, WB_DEFBUTTON, 0 );

            // give the focus to the new def button temporarily - VCL is somewhat peculiar
            // in recognizing a new default button
            VclPtr<vcl::Window> xSaveFocusId = Window::SaveFocus();
            pNewDefButton->GrabFocus();
            Window::EndSaveFocus( xSaveFocusId );
        }
    }


    OUString HangulHanjaConversionDialog::GetCurrentSuggestion( ) const
    {
        return m_pWordInput->GetText();
    }


    void HangulHanjaConversionDialog::SetByCharacter( bool _bByCharacter )
    {
        m_pReplaceByChar->Check( _bByCharacter );
        m_pSuggestions->DisplayListBox( !_bByCharacter );
    }


    void HangulHanjaConversionDialog::SetConversionDirectionState(
            bool _bTryBothDirections,
            HHC::ConversionDirection _ePrimaryConversionDirection )
    {
        // default state: try both direction
        m_pHangulOnly->Check( false );
        m_pHangulOnly->Enable();
        m_pHanjaOnly->Check( false );
        m_pHanjaOnly->Enable();

        if (!_bTryBothDirections)
        {
            CheckBox *pBox = _ePrimaryConversionDirection == HHC::eHangulToHanja ?
                                    m_pHangulOnly.get() : m_pHanjaOnly.get();
            pBox->Check();
            OnConversionDirectionClicked( pBox );
        }
    }


    bool HangulHanjaConversionDialog::GetUseBothDirections( ) const
    {
        return !m_pHangulOnly->IsChecked() && !m_pHanjaOnly->IsChecked();
    }


    HHC::ConversionDirection HangulHanjaConversionDialog::GetDirection(
            HHC::ConversionDirection eDefaultDirection ) const
    {
        HHC::ConversionDirection eDirection = eDefaultDirection;
        if ( m_pHangulOnly->IsChecked() && !m_pHanjaOnly->IsChecked() )
            eDirection = HHC::eHangulToHanja;
        else if ( !m_pHangulOnly->IsChecked() && m_pHanjaOnly->IsChecked() )
            eDirection = HHC::eHanjaToHangul;
        return eDirection;
    }


    void HangulHanjaConversionDialog::SetConversionFormat( HHC::ConversionFormat _eType )
    {
        switch ( _eType )
        {
            case HHC::eSimpleConversion: m_pSimpleConversion->Check(); break;
            case HHC::eHangulBracketed: m_pHangulBracketed->Check(); break;
            case HHC::eHanjaBracketed:  m_pHanjaBracketed->Check(); break;
            case HHC::eRubyHanjaAbove:  m_pHanjaAbove->Check(); break;
            case HHC::eRubyHanjaBelow:  m_pHanjaBelow->Check(); break;
            case HHC::eRubyHangulAbove: m_pHangulAbove->Check(); break;
            case HHC::eRubyHangulBelow: m_pHangulBelow->Check(); break;
        default:
            OSL_FAIL( "HangulHanjaConversionDialog::SetConversionFormat: unknown type!" );
        }
    }


    HHC::ConversionFormat HangulHanjaConversionDialog::GetConversionFormat( ) const
    {
        if ( m_pSimpleConversion->IsChecked() )
            return HHC::eSimpleConversion;
        if ( m_pHangulBracketed->IsChecked() )
            return HHC::eHangulBracketed;
        if ( m_pHanjaBracketed->IsChecked() )
            return HHC::eHanjaBracketed;
        if ( m_pHanjaAbove->IsChecked() )
            return HHC::eRubyHanjaAbove;
        if ( m_pHanjaBelow->IsChecked() )
            return HHC::eRubyHanjaBelow;
        if ( m_pHangulAbove->IsChecked() )
            return HHC::eRubyHangulAbove;
        if ( m_pHangulBelow->IsChecked() )
            return HHC::eRubyHangulBelow;

        OSL_FAIL( "HangulHanjaConversionDialog::GetConversionFormat: no radio checked?" );
        return HHC::eSimpleConversion;
    }


    void HangulHanjaConversionDialog::EnableRubySupport( bool bVal )
    {
        m_pHanjaAbove->Enable( bVal );
        m_pHanjaBelow->Enable( bVal );
        m_pHangulAbove->Enable( bVal );
        m_pHangulBelow->Enable( bVal );
    }


    void HangulHanjaOptionsDialog::Init()
    {
        if( !m_xConversionDictionaryList.is() )
        {
            m_xConversionDictionaryList = ConversionDictionaryList::create( ::comphelper::getProcessComponentContext() );
        }

        m_aDictList.clear();
        m_pDictsLB->Clear();

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
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, OkHdl, Button*, void)
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
            SvTreeListEntry*                        pEntry = m_pDictsLB->SvTreeListBox::GetEntry( n );

            DBG_ASSERT( xDict.is(), "-HangulHanjaOptionsDialog::OkHdl(): someone is evaporated..." );
            DBG_ASSERT( pEntry, "-HangulHanjaOptionsDialog::OkHdl(): no one there in list?" );

            bool    bActive = m_pDictsLB->GetCheckButtonState( pEntry ) == SvButtonState::Checked;
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

        aTmp <<= m_pIgnorepostCB->IsChecked();
        aLngCfg.SetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD, aTmp );

        aTmp <<= m_pShowrecentlyfirstCB->IsChecked();
        aLngCfg.SetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST, aTmp );

        aTmp <<= m_pAutoreplaceuniqueCB->IsChecked();
        aLngCfg.SetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES, aTmp );

        EndDialog( RET_OK );
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DictsLB_SelectHdl, SvTreeListBox*, void)
    {
        bool bSel = m_pDictsLB->FirstSelected() != nullptr;

        m_pEditPB->Enable(bSel);
        m_pDeletePB->Enable(bSel);
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, NewDictHdl, Button*, void)
    {
        OUString                    aName;
        HangulHanjaNewDictDialog aNewDlg(GetFrameWeld());
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

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, EditDictHdl, Button*, void)
    {
        SvTreeListEntry*    pEntry = m_pDictsLB->FirstSelected();
        DBG_ASSERT( pEntry, "+HangulHanjaEditDictDialog::EditDictHdl(): call of edit should not be possible with no selection!" );
        if( pEntry )
        {
            HangulHanjaEditDictDialog aEdDlg(GetFrameWeld(), m_aDictList, m_pDictsLB->GetSelectedEntryPos());
            aEdDlg.run();
        }
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DeleteDictHdl, Button*, void)
    {
        sal_uLong nSelPos = m_pDictsLB->GetSelectedEntryPos();
        if( nSelPos != TREELIST_ENTRY_NOTFOUND )
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
                        m_pDictsLB->RemoveEntry(nSelPos);
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

    HangulHanjaOptionsDialog::HangulHanjaOptionsDialog(vcl::Window* _pParent)
        : ModalDialog( _pParent, "HangulHanjaOptDialog",
            "cui/ui/hangulhanjaoptdialog.ui" )
    {
        get(m_pDictsLB, "dicts");
        get(m_pIgnorepostCB, "ignorepost");
        get(m_pShowrecentlyfirstCB, "showrecentfirst");
        get(m_pAutoreplaceuniqueCB, "autoreplaceunique");
        get(m_pNewPB, "new");
        get(m_pEditPB, "edit");
        get(m_pDeletePB, "delete");
        get(m_pOkPB, "ok");

        m_pDictsLB->set_height_request(m_pDictsLB->GetTextHeight() * 5);
        m_pDictsLB->set_width_request(m_pDictsLB->approximate_char_width() * 32);
        m_pDictsLB->SetStyle( m_pDictsLB->GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL );
        m_pDictsLB->SetForceMakeVisible(true);
        m_pDictsLB->SetSelectionMode( SelectionMode::Single );
        m_pDictsLB->SetHighlightRange();
        m_pDictsLB->SetSelectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );
        m_pDictsLB->SetDeselectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );

        m_pOkPB->SetClickHdl( LINK( this, HangulHanjaOptionsDialog, OkHdl ) );
        m_pNewPB->SetClickHdl( LINK( this, HangulHanjaOptionsDialog, NewDictHdl ) );
        m_pEditPB->SetClickHdl( LINK( this, HangulHanjaOptionsDialog, EditDictHdl ) );
        m_pDeletePB->SetClickHdl( LINK( this, HangulHanjaOptionsDialog, DeleteDictHdl ) );

        SvtLinguConfig  aLngCfg;
        Any             aTmp;
        bool            bVal = bool();
        aTmp = aLngCfg.GetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD );
        if( aTmp >>= bVal )
            m_pIgnorepostCB->Check( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST );
        if( aTmp >>= bVal )
            m_pShowrecentlyfirstCB->Check( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES );
        if( aTmp >>= bVal )
            m_pAutoreplaceuniqueCB->Check( bVal );

        Init();
    }

    HangulHanjaOptionsDialog::~HangulHanjaOptionsDialog()
    {
        disposeOnce();
    }

    void HangulHanjaOptionsDialog::dispose()
    {
        if (m_pDictsLB)
        {
            SvTreeListEntry* pEntry = m_pDictsLB->First();
            while( pEntry )
            {
                delete static_cast<OUString const *>(pEntry->GetUserData());
                pEntry->SetUserData( nullptr );
                pEntry = m_pDictsLB->Next( pEntry );
            }
        }

        m_pDictsLB.clear();
        m_pIgnorepostCB.clear();
        m_pShowrecentlyfirstCB.clear();
        m_pAutoreplaceuniqueCB.clear();
        m_pNewPB.clear();
        m_pEditPB.clear();
        m_pDeletePB.clear();
        m_pOkPB.clear();
        ModalDialog::dispose();
    }

    void HangulHanjaOptionsDialog::AddDict( const OUString& _rName, bool _bChecked )
    {
        SvTreeListEntry*    pEntry = m_pDictsLB->SvTreeListBox::InsertEntry( _rName );
        m_pDictsLB->SetCheckButtonState( pEntry, _bChecked? SvButtonState::Checked : SvButtonState::Unchecked );
        pEntry->SetUserData( new OUString( _rName ) );
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
        DBG_ASSERT( m_pSuggestions, "-HangulHanjaEditDictDialog::NewPBPushHdl(): no suggestions... search in hell..." );
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() && m_pSuggestions )
        {
            //delete old entry
            bool bRemovedSomething = DeleteEntryFromDictionary( xDict );

            OUString                aLeft( m_aOriginal );
            const OUString*           pRight = m_pSuggestions->First();
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

                pRight = m_pSuggestions->Next();
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
        if( m_pSuggestions )
            m_pSuggestions->Clear();

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
        bool bNew = bHaveValidOriginalString && m_pSuggestions && m_pSuggestions->GetCount() > 0;
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

            if( m_pSuggestions )
                m_pSuggestions->Clear();

            //fill found entries into boxes
            sal_uInt32 nCnt = aEntries.getLength();
            if( nCnt )
            {
                if( !m_pSuggestions )
                    m_pSuggestions.reset(new SuggestionList);

                const OUString* pSugg = aEntries.getConstArray();
                sal_uInt32 n = 0;
                while( nCnt )
                {
                    m_pSuggestions->Set( pSugg[ n ], sal_uInt16( n ) );
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
        if( m_pSuggestions )
        {
            aStr = m_pSuggestions->Get(nEntryNum);
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
            if( m_pSuggestions )
                m_pSuggestions->Reset( nEntryNum );
        }
        else
        {
            //set suggestion
            if( !m_pSuggestions )
                m_pSuggestions.reset(new SuggestionList);
            m_pSuggestions->Set( aTxt, nEntryNum );
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
