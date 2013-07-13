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

#include "hangulhanjadlg.hxx"
#include "hangulhanjadlg.hrc"
#include "commonlingui.hxx"
#include <dialmgr.hxx>

#include <cuires.hrc>
#include "helpid.hrc"

#include <algorithm>
#include <vcl/controllayout.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include "svtools/treelistentry.hxx"

#define HHC editeng::HangulHanjaConversion
#define LINE_CNT        static_cast< sal_uInt16 >(2)

//.............................................................................
namespace svx
{
//.............................................................................
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::linguistic2;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    //-------------------------------------------------------------------------
    namespace
    {
        class FontSwitch
        {
        private:
            OutputDevice& m_rDev;

        public:
            inline FontSwitch( OutputDevice& _rDev, const Font& _rTemporaryFont )
                :m_rDev( _rDev )
            {
                m_rDev.Push( PUSH_FONT );
                m_rDev.SetFont( _rTemporaryFont );
            }
            inline ~FontSwitch( )
            {
                m_rDev.Pop( );
            }
        };
    }

    //=========================================================================
    //= PseudoRubyText
    //=========================================================================
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
        const String        m_sPrimaryText;
        const String        m_sSecondaryText;
        const RubyPosition  m_ePosition;

    public:
        PseudoRubyText( const String& _rPrimary, const String& _rSecondary, const RubyPosition _ePosition );

    public:
        void Paint( OutputDevice& _rDevice, const Rectangle& _rRect, sal_uInt16 _nTextStyle,
            Rectangle* _pPrimaryLocation = NULL, Rectangle* _pSecondaryLocation = NULL,
            ::vcl::ControlLayoutData* _pLayoutData = NULL );
    };

    //-------------------------------------------------------------------------
    PseudoRubyText::PseudoRubyText( const String& _rPrimary, const String& _rSecondary, const RubyPosition _ePosition )
        :m_sPrimaryText( _rPrimary )
        ,m_sSecondaryText( _rSecondary )
        ,m_ePosition( _ePosition )
    {
    }

    //-------------------------------------------------------------------------
    void PseudoRubyText::Paint( OutputDevice& _rDevice, const Rectangle& _rRect, sal_uInt16 _nTextStyle,
        Rectangle* _pPrimaryLocation, Rectangle* _pSecondaryLocation, ::vcl::ControlLayoutData* _pLayoutData )
    {
        bool            bLayoutOnly  = NULL != _pLayoutData;
        MetricVector*   pTextMetrics = bLayoutOnly ? &_pLayoutData->m_aUnicodeBoundRects : NULL;
        OUString*       pDisplayText = bLayoutOnly ? &_pLayoutData->m_aDisplayText       : NULL;

        Size aPlaygroundSize( _rRect.GetSize() );

        // the font for the secondary text:
        Font aSmallerFont( _rDevice.GetFont() );
        // heuristic: 80% of the original size
        aSmallerFont.SetHeight( (long)( 0.8 * aSmallerFont.GetHeight() ) );

        // let's calculate the size of our two texts
        Rectangle aPrimaryRect = _rDevice.GetTextRect( _rRect, m_sPrimaryText, _nTextStyle );
        Rectangle aSecondaryRect;
        {
            FontSwitch aFontRestore( _rDevice, aSmallerFont );
            aSecondaryRect = _rDevice.GetTextRect( _rRect, m_sSecondaryText, _nTextStyle );
        }

        // position these rectangles properly
        // x-axis:
        sal_Int32 nCombinedWidth = ::std::max( aSecondaryRect.GetWidth(), aPrimaryRect.GetWidth() );
            // the rectangle where both texts will reside is as high as possible, and as wide as the
            // widest of both text rects
        aPrimaryRect.Left() = aSecondaryRect.Left() = _rRect.Left();
        aPrimaryRect.Right() = aSecondaryRect.Right() = _rRect.Left() + nCombinedWidth;
        if ( TEXT_DRAW_RIGHT & _nTextStyle )
        {
            // move the rectangles to the right
            aPrimaryRect.Move( aPlaygroundSize.Width() - nCombinedWidth, 0 );
            aSecondaryRect.Move( aPlaygroundSize.Width() - nCombinedWidth, 0 );
        }
        else if ( TEXT_DRAW_CENTER & _nTextStyle )
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
        if ( TEXT_DRAW_BOTTOM & _nTextStyle )
        {
            // move the rects to the bottom
            aPrimaryRect.Move( 0, aPlaygroundSize.Height() - nCombinedHeight );
            aSecondaryRect.Move( 0, aPlaygroundSize.Height() - nCombinedHeight );
        }
        else if ( TEXT_DRAW_VCENTER & _nTextStyle )
        {
            // move the rects to the bottom
            aPrimaryRect.Move( 0, ( aPlaygroundSize.Height() - nCombinedHeight ) / 2 );
            aSecondaryRect.Move( 0, ( aPlaygroundSize.Height() - nCombinedHeight ) / 2 );
        }

        // 'til here, everything we did assumes that the secondary text is painted _below_ the primary
        // text. If this isn't the case, we need to correct the rectangles
        if ( eAbove == m_ePosition )
        {
            sal_Int32 nVertDistance = aSecondaryRect.Top() - aPrimaryRect.Top();
            aSecondaryRect.Move( 0, -nVertDistance );
            aPrimaryRect.Move( 0, nCombinedHeight - nVertDistance );
        }

        // now draw the texts
        // as we already calculated the precise rectangles for the texts, we don't want to
        // use the alignment flags given - within it's rect, every text is centered
        sal_uInt16 nDrawTextStyle( _nTextStyle );
        nDrawTextStyle &= ~( TEXT_DRAW_RIGHT | TEXT_DRAW_LEFT | TEXT_DRAW_BOTTOM | TEXT_DRAW_TOP );
        nDrawTextStyle |= TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER;

        _rDevice.DrawText( aPrimaryRect, m_sPrimaryText, nDrawTextStyle, pTextMetrics, pDisplayText );
        {
            FontSwitch aFontRestore( _rDevice, aSmallerFont );
            _rDevice.DrawText( aSecondaryRect, m_sSecondaryText, nDrawTextStyle, pTextMetrics, pDisplayText );
        }

        // outta here
        if ( _pPrimaryLocation )
            *_pPrimaryLocation = aPrimaryRect;
        if ( _pSecondaryLocation )
            *_pSecondaryLocation = aSecondaryRect;
    }

    //=========================================================================
    //= RubyRadioButton
    //=========================================================================
    class RubyRadioButton   :public RadioButton
                            ,protected PseudoRubyText
    {
        using svx::PseudoRubyText::Paint;

    public:
        RubyRadioButton(
            Window* _pParent,
            const ResId& _rId,          // the text in the resource will be taken as primary text
            const String& _rSecondary,  // this will be the secondary text which will be printed somewhat smaller
            const PseudoRubyText::RubyPosition _ePosition );

    protected:
        virtual void    Paint( const Rectangle& _rRect );
    };

    //-------------------------------------------------------------------------
    RubyRadioButton::RubyRadioButton( Window* _pParent, const ResId& _rId,
        const String& _rSecondary, const PseudoRubyText::RubyPosition _ePosition )
        :RadioButton( _pParent, _rId )
        ,PseudoRubyText( RadioButton::GetText(), _rSecondary, _ePosition )
    {
    }

    //-------------------------------------------------------------------------
    void RubyRadioButton::Paint( const Rectangle& )
    {
        HideFocus();

        // calculate the size of the radio image - we're to paint our text _after_ this image
        DBG_ASSERT( !GetModeRadioImage(), "RubyRadioButton::Paint: images not supported!" );
        Size aImageSize = GetRadioImage( GetSettings(), 0 ).GetSizePixel();
        aImageSize.Width()  = CalcZoom( aImageSize.Width() );
        aImageSize.Height()  = CalcZoom( aImageSize.Height() );

        Rectangle aOverallRect( Point( 0, 0 ), GetOutputSizePixel() );
        aOverallRect.Left() += aImageSize.Width() + 4;  // 4 is the separator between the image and the text
        // inflate the rect a little bit (because the VCL radio button does the same)
        Rectangle aTextRect( aOverallRect );
        ++aTextRect.Left(); --aTextRect.Right();
        ++aTextRect.Top(); --aTextRect.Bottom();

        // calculate the text flags for the painting
        sal_uInt16 nTextStyle = TEXT_DRAW_MNEMONIC;
        WinBits nStyle = GetStyle( );

        // the horizontal alignment
        if ( nStyle & WB_RIGHT )
            nTextStyle |= TEXT_DRAW_RIGHT;
        else if ( nStyle & WB_CENTER )
            nTextStyle |= TEXT_DRAW_CENTER;
        else
            nTextStyle |= TEXT_DRAW_LEFT;
        // the vertical alignment
        if ( nStyle & WB_BOTTOM )
            nTextStyle |= TEXT_DRAW_BOTTOM;
        else if ( nStyle & WB_VCENTER )
            nTextStyle |= TEXT_DRAW_VCENTER;
        else
            nTextStyle |= TEXT_DRAW_TOP;
        // mnemonics
        if ( 0 == ( nStyle & WB_NOLABEL ) )
            nTextStyle |= TEXT_DRAW_MNEMONIC;

        // paint the ruby text
        Rectangle aPrimaryTextLocation, aSecondaryTextLocation;
        PseudoRubyText::Paint( *this, aTextRect, nTextStyle, &aPrimaryTextLocation, &aSecondaryTextLocation );

        // the focus rectangle is to be painted around both texts
        Rectangle aCombinedRect( aPrimaryTextLocation );
        aCombinedRect.Union( aSecondaryTextLocation );
        SetFocusRect( aCombinedRect );

        // let the base class paint the radio button
        // for this, give it the proper location to paint the image (vertically centered, relative to our text)
        Rectangle aImageLocation( Point( 0, 0 ), aImageSize );
        sal_Int32 nTextHeight = aSecondaryTextLocation.Bottom() - aPrimaryTextLocation.Top();
        aImageLocation.Top() = aPrimaryTextLocation.Top() + ( nTextHeight - aImageSize.Height() ) / 2;
        aImageLocation.Bottom() = aImageLocation.Top() + aImageSize.Height();
        SetStateRect( aImageLocation );
        DrawRadioButtonState( );

        // mouse clicks should be recognized in a rect which is one pixel larger in each direction, plus
        // includes the image
        aCombinedRect.Left() = aImageLocation.Left(); ++aCombinedRect.Right();
        --aCombinedRect.Top(); ++aCombinedRect.Bottom();
        SetMouseRect( aCombinedRect );

        // paint the focus rect, if necessary
        if ( HasFocus() )
            ShowFocus( aTextRect );
    }

    //=========================================================================
    //= SuggestionSet
    //=========================================================================
    //-------------------------------------------------------------------------

    SuggestionSet::SuggestionSet( Window* pParent )
                    : ValueSet( pParent, pParent->GetStyle() | WB_BORDER )

    {
    }

    SuggestionSet::~SuggestionSet()
    {
        ClearSet();
    }

    void SuggestionSet::UserDraw( const UserDrawEvent& rUDEvt )
    {
        OutputDevice*  pDev = rUDEvt.GetDevice();
        Rectangle aRect = rUDEvt.GetRect();
        sal_uInt16  nItemId = rUDEvt.GetItemId();

        String sText = *static_cast< String* >( GetItemData( nItemId ) );
        pDev->DrawText( aRect, sText, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER );
    }

    void SuggestionSet::ClearSet()
    {
        sal_uInt16 i, nCount = GetItemCount();
        for ( i = 0; i < nCount; ++i )
            delete static_cast< String* >( GetItemData(i) );
        Clear();
    }

    //=========================================================================
    //= SuggestionDisplay
    //=========================================================================
    //-------------------------------------------------------------------------

    SuggestionDisplay::SuggestionDisplay( Window* pParent, const ResId& rResId )
        : Control( pParent, rResId )
        , m_bDisplayListBox(true)
        , m_aValueSet(this)
        , m_aListBox(this,GetStyle() | WB_BORDER )
        , m_bInSelectionUpdate(false)
    {
        m_aValueSet.SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionHdl ) );
        m_aListBox.SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionHdl ) );

        m_aValueSet.SetLineCount( LINE_CNT );
        m_aValueSet.SetStyle( m_aValueSet.GetStyle() | WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL );
        m_aValueSet.SetBorderStyle( WINDOW_BORDER_MONO );
        String aOneCharacter(RTL_CONSTASCII_USTRINGPARAM("AU"));
        long nItemWidth = 2*GetTextWidth( aOneCharacter );
        m_aValueSet.SetItemWidth( nItemWidth );

        Size aSize(GetSizePixel());
        m_aValueSet.SetSizePixel(aSize);
        m_aListBox.SetSizePixel(aSize);

        implUpdateDisplay();
    }

    SuggestionDisplay::~SuggestionDisplay()
    {
    }

    void SuggestionDisplay::implUpdateDisplay()
    {
        bool bShowBox = IsVisible() && m_bDisplayListBox;
        bool bShowSet = IsVisible() && !m_bDisplayListBox;

        m_aListBox.Show(bShowBox);
        m_aValueSet.Show(bShowSet);
    }

    void SuggestionDisplay::StateChanged( StateChangedType nStateChange )
    {
        if( STATE_CHANGE_VISIBLE == nStateChange )
            implUpdateDisplay();
    }

    Control& SuggestionDisplay::implGetCurrentControl()
    {
        if( m_bDisplayListBox )
            return m_aListBox;
        return m_aValueSet;
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
        implGetCurrentControl().GetFocus();
    }
    void SuggestionDisplay::LoseFocus()
    {
        implGetCurrentControl().LoseFocus();
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
            sal_Bool bHasFocus = rOldControl.HasFocus();

            m_bDisplayListBox = bDisplayListBox;

            if( bHasFocus )
            {
                Control& rNewControl = implGetCurrentControl();
                rNewControl.GrabFocus();
            }

            implUpdateDisplay();
        }
    }

    IMPL_LINK( SuggestionDisplay, SelectSuggestionHdl, Control*, pControl )
    {
        if( m_bInSelectionUpdate )
            return 0L;

        m_bInSelectionUpdate = true;
        if(pControl==&m_aListBox)
        {
            sal_uInt16 nPos = m_aListBox.GetSelectEntryPos();
            m_aValueSet.SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
        }
        else
        {
            sal_uInt16 nPos = m_aValueSet.GetSelectItemId()-1; //itemid == pos+1 (id 0 has special meaning)
            m_aListBox.SelectEntryPos( nPos );
        }
        m_bInSelectionUpdate = false;
        m_aSelectLink.Call(this);
        return 0L;
    }

    void SuggestionDisplay::SetSelectHdl( const Link& rLink )
    {
        m_aSelectLink = rLink;
    }
    void SuggestionDisplay::Clear()
    {
        m_aListBox.Clear();
        m_aValueSet.Clear();
    }
    void SuggestionDisplay::InsertEntry( const XubString& rStr )
    {
        sal_uInt16 nItemId = m_aListBox.InsertEntry( rStr ) + 1; //itemid == pos+1 (id 0 has special meaning)
        m_aValueSet.InsertItem( nItemId );
        String* pItemData = new String(rStr);
        m_aValueSet.SetItemData( nItemId, pItemData );
    }
    void SuggestionDisplay::SelectEntryPos( sal_uInt16 nPos )
    {
        m_aListBox.SelectEntryPos( nPos );
        m_aValueSet.SelectItem( nPos+1 ); //itemid == pos+1 (id 0 has special meaning)
    }
    sal_uInt16 SuggestionDisplay::GetEntryCount() const
    {
        return m_aListBox.GetEntryCount();
    }
    XubString SuggestionDisplay::GetEntry( sal_uInt16 nPos ) const
    {
        return m_aListBox.GetEntry( nPos );
    }
    XubString SuggestionDisplay::GetSelectEntry() const
    {
        return m_aListBox.GetSelectEntry();
    }
    void SuggestionDisplay::SetHelpIds()
    {
        this->SetHelpId( HID_HANGULDLG_SUGGESTIONS );
        m_aValueSet.SetHelpId( HID_HANGULDLG_SUGGESTIONS_GRID );
        m_aListBox.SetHelpId( HID_HANGULDLG_SUGGESTIONS_LIST );
    }

    //=========================================================================
    //= HangulHanjaConversionDialog
    //=========================================================================
    //-------------------------------------------------------------------------
    HangulHanjaConversionDialog::HangulHanjaConversionDialog( Window* _pParent, HHC::ConversionDirection _ePrimaryDirection )
        :ModalDialog( _pParent, CUI_RES( RID_SVX_MDLG_HANGULHANJA ) )
        ,m_pPlayground( new SvxCommonLinguisticControl( this ) )
        ,m_aFind            ( m_pPlayground.get(), CUI_RES( PB_FIND ) )
        ,m_aSuggestions     ( m_pPlayground.get(), CUI_RES( CTL_SUGGESTIONS ) )
        ,m_aFormat          ( m_pPlayground.get(), CUI_RES( FT_FORMAT ) )
        ,m_aSimpleConversion( m_pPlayground.get(), CUI_RES( RB_SIMPLE_CONVERSION ) )
        ,m_aHangulBracketed ( m_pPlayground.get(), CUI_RES( RB_HANJA_HANGUL_BRACKETED ) )
        ,m_aHanjaBracketed  ( m_pPlayground.get(), CUI_RES( RB_HANGUL_HANJA_BRACKETED ) )
        ,m_aConversion      ( m_pPlayground.get(), CUI_RES( FT_CONVERSION ) )
        ,m_aHangulOnly      ( m_pPlayground.get(), CUI_RES( CB_HANGUL_ONLY ) )
        ,m_aHanjaOnly       ( m_pPlayground.get(), CUI_RES( CB_HANJA_ONLY ) )
        ,m_aReplaceByChar   ( m_pPlayground.get(), CUI_RES( CB_REPLACE_BY_CHARACTER ) )
        ,m_pIgnoreNonPrimary( NULL )
        ,m_bDocumentMode( true )
    {
        // special creation of the 4 pseudo-ruby radio buttons
        String sSecondaryHangul( CUI_RES( STR_HANGUL ) );
        String sSecondaryHanja( CUI_RES( STR_HANJA ) );
        m_pHanjaAbove.reset( new RubyRadioButton( m_pPlayground.get(), CUI_RES( RB_HANGUL_HANJA_ABOVE ), sSecondaryHanja, PseudoRubyText::eAbove ) );
        m_pHanjaBelow.reset( new RubyRadioButton( m_pPlayground.get(), CUI_RES( RB_HANGUL_HANJA_BELOW ), sSecondaryHanja, PseudoRubyText::eBelow ) );
        m_pHangulAbove.reset( new RubyRadioButton( m_pPlayground.get(), CUI_RES( RB_HANJA_HANGUL_ABOVE ), sSecondaryHangul, PseudoRubyText::eAbove ) );
        m_pHangulBelow.reset( new RubyRadioButton( m_pPlayground.get(), CUI_RES( RB_HANJA_HANGUL_BELOW ), sSecondaryHangul, PseudoRubyText::eBelow ) );

        // since these 4 buttons are not created within the other members, they have a wrong initial Z-Order
        // correct this
        m_pHanjaAbove->SetZOrder( &m_aHanjaBracketed, WINDOW_ZORDER_BEHIND );
        m_pHanjaBelow->SetZOrder( m_pHanjaAbove.get(), WINDOW_ZORDER_BEHIND );
        m_pHangulAbove->SetZOrder( m_pHanjaBelow.get(), WINDOW_ZORDER_BEHIND );
        m_pHangulBelow->SetZOrder( m_pHangulAbove.get(), WINDOW_ZORDER_BEHIND );

        // VCL automatically sets the WB_GROUP bit, if the previous sibling (at the moment of creation)
        // is no radion button
        m_pHanjaAbove->SetStyle( m_pHanjaAbove->GetStyle() & ~WB_GROUP );

        // the "Find" button and the word input control may not have the proper distance/extensions
        // -> correct this
        Point aDistance = LogicToPixel( Point( 3, 0 ), MAP_APPFONT );
        sal_Int32 nTooLargeByPixels =
            // right margin of the word input control
            (   m_pPlayground->GetWordInputControl().GetPosPixel().X()
            +   m_pPlayground->GetWordInputControl().GetSizePixel().Width()
            )
            // minus left margin of the find button
            -   m_aFind.GetPosPixel().X()
            // plus desired distance between the both
            +   aDistance.X();
        // make the word input control smaller
        Size aSize = m_pPlayground->GetWordInputControl().GetSizePixel();
        aSize.Width() -= nTooLargeByPixels;
        m_pPlayground->GetWordInputControl().SetSizePixel( aSize );

        // additionall, the playground is not wide enough (in it's default size)
        sal_Int32 nEnlargeWidth = 0;
        {
            FixedText aBottomAnchor( m_pPlayground.get(), CUI_RES( FT_RESIZE_ANCHOR ) );
            Point aAnchorPos = aBottomAnchor.GetPosPixel();

            nEnlargeWidth = aAnchorPos.X() - m_pPlayground->GetActionButtonsLocation().X();
        }
        m_pPlayground->Enlarge( nEnlargeWidth, 0 );

        // insert our controls into the z-order of the playground
        m_pPlayground->InsertControlGroup( m_aFind, m_aFind, SvxCommonLinguisticControl::eLeftRightWords );
        m_pPlayground->InsertControlGroup( m_aSuggestions, m_aHanjaOnly, SvxCommonLinguisticControl::eSuggestionLabel );
        m_pPlayground->InsertControlGroup( m_aReplaceByChar, m_aReplaceByChar, SvxCommonLinguisticControl::eActionButtons );

        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eClose, LINK( this, HangulHanjaConversionDialog, OnClose ) );
        m_pPlayground->GetWordInputControl().SetModifyHdl( LINK( this,  HangulHanjaConversionDialog, OnSuggestionModified ) );
        m_aSuggestions.SetSelectHdl( LINK( this,  HangulHanjaConversionDialog, OnSuggestionSelected ) );

        m_aReplaceByChar.SetClickHdl( LINK( this, HangulHanjaConversionDialog, ClickByCharacterHdl ) );

        m_aHangulOnly.SetClickHdl( LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );
        m_aHanjaOnly.SetClickHdl(  LINK( this,  HangulHanjaConversionDialog, OnConversionDirectionClicked ) );

        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eOptions,
                                        LINK( this, HangulHanjaConversionDialog, OnOption ) );
        m_pPlayground->GetButton( SvxCommonLinguisticControl::eOptions )->Show();

        if ( editeng::HangulHanjaConversion::eHangulToHanja == _ePrimaryDirection )
        {
            m_pIgnoreNonPrimary = &m_aHangulOnly;
        }
        else
        {
            m_pIgnoreNonPrimary = &m_aHanjaOnly;
        }

        // initial focus
        FocusSuggestion( );

        // initial control values
        m_aSimpleConversion.Check();

        m_pPlayground->GetButton(SvxCommonLinguisticControl::eClose     )->SetHelpId(HID_HANGULDLG_BUTTON_CLOSE    );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eIgnore    )->SetHelpId(HID_HANGULDLG_BUTTON_IGNORE   );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eIgnoreAll )->SetHelpId(HID_HANGULDLG_BUTTON_IGNOREALL);
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eChange    )->SetHelpId(HID_HANGULDLG_BUTTON_CHANGE   );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eChangeAll )->SetHelpId(HID_HANGULDLG_BUTTON_CHANGEALL);
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eOptions   )->SetHelpId(HID_HANGULDLG_BUTTON_OPTIONS  );
        m_pPlayground->GetWordInputControl().SetHelpId(HID_HANGULDLG_EDIT_NEWWORD);

        FreeResource();

        m_aSuggestions.SetHelpIds();
    }

    //-------------------------------------------------------------------------
    HangulHanjaConversionDialog::~HangulHanjaConversionDialog( )
    {
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::FillSuggestions( const ::com::sun::star::uno::Sequence< OUString >& _rSuggestions )
    {
        m_aSuggestions.Clear();

        const OUString* pSuggestions = _rSuggestions.getConstArray();
        const OUString* pSuggestionsEnd = _rSuggestions.getConstArray() + _rSuggestions.getLength();
        while ( pSuggestions != pSuggestionsEnd )
            m_aSuggestions.InsertEntry( *pSuggestions++ );

        // select the first suggestion, and fill in the suggestion edit field
        String sFirstSuggestion;
        if ( m_aSuggestions.GetEntryCount() )
        {
            sFirstSuggestion = m_aSuggestions.GetEntry( 0 );
            m_aSuggestions.SelectEntryPos( 0 );
        }
        m_pPlayground->GetWordInputControl().SetText( sFirstSuggestion );
        m_pPlayground->GetWordInputControl().SaveValue();
        OnSuggestionModified( &m_pPlayground->GetWordInputControl() );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetOptionsChangedHdl( const Link& _rHdl )
    {
        m_aOptionsChangedLink = _rHdl;
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetIgnoreHdl( const Link& _rHdl )
    {
        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eIgnore, _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetIgnoreAllHdl( const Link& _rHdl )
    {
        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eIgnoreAll, _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetChangeHdl( const Link& _rHdl )
    {
        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eChange, _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetChangeAllHdl( const Link& _rHdl )
    {
        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eChangeAll, _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetFindHdl( const Link& _rHdl )
    {
        m_aFind.SetClickHdl( _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetConversionFormatChangedHdl( const Link& _rHdl )
    {
        m_aSimpleConversion.SetClickHdl( _rHdl );
        m_aHangulBracketed.SetClickHdl( _rHdl );
        m_aHanjaBracketed.SetClickHdl( _rHdl );
        m_pHanjaAbove->SetClickHdl( _rHdl );
        m_pHanjaBelow->SetClickHdl( _rHdl );
        m_pHangulAbove->SetClickHdl( _rHdl );
        m_pHangulBelow->SetClickHdl( _rHdl );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetClickByCharacterHdl( const Link& _rHdl )
    {
        m_aClickByCharacterLink = _rHdl;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK_NOARG(HangulHanjaConversionDialog, OnSuggestionSelected)
    {
        m_pPlayground->GetWordInputControl().SetText( m_aSuggestions.GetSelectEntry() );
        OnSuggestionModified( NULL );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK_NOARG(HangulHanjaConversionDialog, OnSuggestionModified)
    {
        m_aFind.Enable( m_pPlayground->GetWordInputControl().GetSavedValue() != m_pPlayground->GetWordInputControl().GetText() );

        bool bSameLen = m_pPlayground->GetWordInputControl().GetText().getLength() == m_pPlayground->GetCurrentText().Len();
        m_pPlayground->EnableButton( SvxCommonLinguisticControl::eChange, m_bDocumentMode && bSameLen );
        m_pPlayground->EnableButton( SvxCommonLinguisticControl::eChangeAll, m_bDocumentMode && bSameLen );

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversionDialog, ClickByCharacterHdl, CheckBox *, pBox )
    {
        m_aClickByCharacterLink.Call(pBox);

        bool bByCharacter = pBox->IsChecked();
        m_aSuggestions.DisplayListBox( !bByCharacter );

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversionDialog, OnConversionDirectionClicked, CheckBox *, pBox )
    {
        CheckBox *pOtherBox = 0;
        if (pBox == &m_aHangulOnly)
            pOtherBox = &m_aHanjaOnly;
        else if (pBox == &m_aHanjaOnly)
            pOtherBox = &m_aHangulOnly;
        if (pBox && pOtherBox)
        {
            sal_Bool bBoxChecked = pBox->IsChecked();
            if (bBoxChecked)
                pOtherBox->Check( sal_False );
            pOtherBox->Enable( !bBoxChecked );
        }

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK_NOARG(HangulHanjaConversionDialog, OnClose)
    {
        Close();
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversionDialog, OnOption)
    {
        HangulHanjaOptionsDialog        aOptDlg( this );
        aOptDlg.Execute();
        m_aOptionsChangedLink.Call(this);
        return 0L;
    }

    //-------------------------------------------------------------------------
    String HangulHanjaConversionDialog::GetCurrentString( ) const
    {
        return m_pPlayground->GetCurrentText( );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::FocusSuggestion( )
    {
        m_pPlayground->GetWordInputControl().GrabFocus();
    }

    //-------------------------------------------------------------------------
    namespace
    {
        void lcl_modifyWindowStyle( Window* _pWin, WinBits _nSet, WinBits _nReset )
        {
            DBG_ASSERT( 0 == ( _nSet & _nReset ), "lcl_modifyWindowStyle: set _and_ reset the same bit?" );
            if ( _pWin )
                _pWin->SetStyle( ( _pWin->GetStyle() | _nSet ) & ~_nReset );
        }
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetCurrentString( const String& _rNewString,
        const Sequence< OUString >& _rSuggestions, bool _bOriginatesFromDocument )
    {
        m_pPlayground->SetCurrentText( _rNewString );

        bool bOldDocumentMode = m_bDocumentMode;
        m_bDocumentMode = _bOriginatesFromDocument; // before FillSuggestions!
        FillSuggestions( _rSuggestions );

        m_pPlayground->EnableButton( SvxCommonLinguisticControl::eIgnoreAll, m_bDocumentMode );
            // all other buttons have been implicitly enabled or disabled during filling in the suggestions

        // switch the def button depending if we're working for document text
        if ( bOldDocumentMode != m_bDocumentMode )
        {
            Window* pOldDefButton = NULL;
            Window* pNewDefButton = NULL;
            if ( m_bDocumentMode )
            {
                pOldDefButton = &m_aFind;
                pNewDefButton = m_pPlayground->GetButton( SvxCommonLinguisticControl::eChange );
            }
            else
            {
                pOldDefButton = m_pPlayground->GetButton( SvxCommonLinguisticControl::eChange );
                pNewDefButton = &m_aFind;
            }

            DBG_ASSERT( WB_DEFBUTTON == ( pOldDefButton->GetStyle( ) & WB_DEFBUTTON ),
                "HangulHanjaConversionDialog::SetCurrentString: wrong previous default button (1)!" );
            DBG_ASSERT( 0 == ( pNewDefButton->GetStyle( ) & WB_DEFBUTTON ),
                "HangulHanjaConversionDialog::SetCurrentString: wrong previous default button (2)!" );

            lcl_modifyWindowStyle( pOldDefButton, 0, WB_DEFBUTTON );
            lcl_modifyWindowStyle( pNewDefButton, WB_DEFBUTTON, 0 );

            // give the focus to the new def button temporarily - VCL is somewhat peculiar
            // in recognizing a new default button
            sal_uInt32 nSaveFocusId = Window::SaveFocus();
            pNewDefButton->GrabFocus();
            Window::EndSaveFocus( nSaveFocusId );
        }
    }

    //-------------------------------------------------------------------------
    String HangulHanjaConversionDialog::GetCurrentSuggestion( ) const
    {
        return m_pPlayground->GetWordInputControl().GetText();
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetByCharacter( bool _bByCharacter )
    {
        m_aReplaceByChar.Check( static_cast<sal_Bool>(_bByCharacter) );
        m_aSuggestions.DisplayListBox( !_bByCharacter );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetConversionDirectionState(
            bool _bTryBothDirections,
            HHC::ConversionDirection _ePrimaryConversionDirection )
    {
        // default state: try both direction
        m_aHangulOnly.Check( sal_False );
        m_aHangulOnly.Enable( sal_True );
        m_aHanjaOnly.Check( sal_False );
        m_aHanjaOnly.Enable( sal_True );

        if (!_bTryBothDirections)
        {
            CheckBox *pBox = _ePrimaryConversionDirection == HHC::eHangulToHanja?
                                    &m_aHangulOnly : &m_aHanjaOnly;
            pBox->Check( sal_True );
            OnConversionDirectionClicked( pBox );
        }
    }

    //-------------------------------------------------------------------------
    bool HangulHanjaConversionDialog::GetUseBothDirections( ) const
    {
        return !m_aHangulOnly.IsChecked() && !m_aHanjaOnly.IsChecked();
    }

    //-------------------------------------------------------------------------
    HHC::ConversionDirection HangulHanjaConversionDialog::GetDirection(
            HHC::ConversionDirection eDefaultDirection ) const
    {
        HHC::ConversionDirection eDirection = eDefaultDirection;
        if (m_aHangulOnly.IsChecked() && !m_aHanjaOnly.IsChecked())
            eDirection = HHC::eHangulToHanja;
        else if (!m_aHangulOnly.IsChecked() && m_aHanjaOnly.IsChecked())
            eDirection = HHC::eHanjaToHangul;
        return eDirection;
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetConversionFormat( HHC::ConversionFormat _eType )
    {
        switch ( _eType )
        {
            case HHC::eSimpleConversion: m_aSimpleConversion.Check(); break;
            case HHC::eHangulBracketed: m_aHangulBracketed.Check(); break;
            case HHC::eHanjaBracketed:  m_aHanjaBracketed.Check(); break;
            case HHC::eRubyHanjaAbove:  m_pHanjaAbove->Check(); break;
            case HHC::eRubyHanjaBelow:  m_pHanjaBelow->Check(); break;
            case HHC::eRubyHangulAbove: m_pHangulAbove->Check(); break;
            case HHC::eRubyHangulBelow: m_pHangulBelow->Check(); break;
        default:
            OSL_FAIL( "HangulHanjaConversionDialog::SetConversionFormat: unknown type!" );
        }
    }

    //-------------------------------------------------------------------------
    HHC::ConversionFormat HangulHanjaConversionDialog::GetConversionFormat( ) const
    {
        if ( m_aSimpleConversion.IsChecked() )
            return HHC::eSimpleConversion;
        if ( m_aHangulBracketed.IsChecked() )
            return HHC::eHangulBracketed;
        if ( m_aHanjaBracketed.IsChecked() )
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

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::EnableRubySupport( bool bVal )
    {
        m_pHanjaAbove->Enable( bVal );
        m_pHanjaBelow->Enable( bVal );
        m_pHangulAbove->Enable( bVal );
        m_pHangulBelow->Enable( bVal );
    }


    //=========================================================================
    //= HangulHanjaOptionsDialog
    //=========================================================================
    //-------------------------------------------------------------------------

    void HangulHanjaOptionsDialog::Init( void )
    {
        if( !m_xConversionDictionaryList.is() )
        {
            m_xConversionDictionaryList = ConversionDictionaryList::create( ::comphelper::getProcessComponentContext() );
        }

        m_aDictList.clear();
        m_aDictsLB.Clear();

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

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, OkHdl)
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
            SvTreeListEntry*                        pEntry = m_aDictsLB.SvTreeListBox::GetEntry( n );

            DBG_ASSERT( xDict.is(), "-HangulHanjaOptionsDialog::OkHdl(): someone is evaporated..." );
            DBG_ASSERT( pEntry, "-HangulHanjaOptionsDialog::OkHdl(): no one there in list?" );

            bool    bActive = m_aDictsLB.GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED;
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

        aTmp <<= bool( m_aIgnorepostCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD, aTmp );

        aTmp <<= bool( m_aShowrecentlyfirstCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST, aTmp );

        aTmp <<= bool( m_aAutoreplaceuniqueCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES, aTmp );

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DictsLB_SelectHdl)
    {
        bool    bSel = m_aDictsLB.FirstSelected() != NULL;

        m_aEditPB.Enable( bSel );
        m_aDeletePB.Enable( bSel );

        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, NewDictHdl)
    {
        String                      aName;
        HangulHanjaNewDictDialog    aNewDlg( this );
        aNewDlg.Execute();
        if( aNewDlg.GetName( aName ) )
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

        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, EditDictHdl)
    {
        SvTreeListEntry*    pEntry = m_aDictsLB.FirstSelected();
        DBG_ASSERT( pEntry, "+HangulHanjaEditDictDialog::EditDictHdl(): call of edit should not be possible with no selection!" );
        if( pEntry )
        {
            HangulHanjaEditDictDialog   aEdDlg( this, m_aDictList, m_aDictsLB.GetSelectEntryPos() );
            aEdDlg.Execute();
        }
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DeleteDictHdl)
    {
        sal_uInt16 nSelPos = m_aDictsLB.GetSelectEntryPos();
        if( nSelPos != LISTBOX_ENTRY_NOTFOUND )
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
                        HHDictList::iterator aIter(m_aDictList.begin());
                        m_aDictList.erase(aIter+nSelPos );
                        m_aDictsLB.RemoveEntry( nSelPos );
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

        return 0L;
    }

    HangulHanjaOptionsDialog::HangulHanjaOptionsDialog( Window* _pParent )
        :ModalDialog            ( _pParent, CUI_RES( RID_SVX_MDLG_HANGULHANJA_OPT ) )
        ,m_aUserdefdictFT       ( this, CUI_RES( FT_USERDEFDICT ) )
        ,m_aDictsLB             ( this, CUI_RES( LB_DICTS ) )
        ,m_aOptionsFL           ( this, CUI_RES( FL_OPTIONS ) )
        ,m_aIgnorepostCB        ( this, CUI_RES( CB_IGNOREPOST ) )
        ,m_aShowrecentlyfirstCB ( this, CUI_RES( CB_SHOWRECENTLYFIRST ) )
        ,m_aAutoreplaceuniqueCB ( this, CUI_RES( CB_AUTOREPLACEUNIQUE ) )
        ,m_aNewPB               ( this, CUI_RES( PB_HHO_NEW ) )
        ,m_aEditPB              ( this, CUI_RES( PB_HHO_EDIT ) )
        ,m_aDeletePB            ( this, CUI_RES( PB_HHO_DELETE ) )
        ,m_aOkPB                ( this, CUI_RES( PB_HHO_OK ) )
        ,m_aCancelPB            ( this, CUI_RES( PB_HHO_CANCEL ) )
        ,m_aHelpPB              ( this, CUI_RES( PB_HHO_HELP ) )

        ,m_pCheckButtonData     ( NULL )
        ,m_xConversionDictionaryList( NULL )
    {
        m_aDictsLB.SetStyle( m_aDictsLB.GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
        m_aDictsLB.SetSelectionMode( SINGLE_SELECTION );
        m_aDictsLB.SetHighlightRange();
        m_aDictsLB.SetSelectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );
        m_aDictsLB.SetDeselectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );

        m_aOkPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, OkHdl ) );
        m_aNewPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, NewDictHdl ) );
        m_aEditPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, EditDictHdl ) );
        m_aDeletePB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, DeleteDictHdl ) );

        FreeResource();

        SvtLinguConfig  aLngCfg;
        Any             aTmp;
        bool            bVal = bool();
        aTmp = aLngCfg.GetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD );
        if( aTmp >>= bVal )
            m_aIgnorepostCB.Check( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST );
        if( aTmp >>= bVal )
            m_aShowrecentlyfirstCB.Check( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES );
        if( aTmp >>= bVal )
            m_aAutoreplaceuniqueCB.Check( bVal );

        Init();
    }

    HangulHanjaOptionsDialog::~HangulHanjaOptionsDialog()
    {
        SvTreeListEntry*    pEntry = m_aDictsLB.First();
        String*         pDel;
        while( pEntry )
        {
            pDel = ( String* ) pEntry->GetUserData();
            if( pDel )
                delete pDel;
            pEntry = m_aDictsLB.Next( pEntry );
        }

        if( m_pCheckButtonData )
            delete m_pCheckButtonData;
    }

    void HangulHanjaOptionsDialog::AddDict( const String& _rName, bool _bChecked )
    {
        SvTreeListEntry*    pEntry = m_aDictsLB.SvTreeListBox::InsertEntry( _rName );
        m_aDictsLB.SetCheckButtonState( pEntry, _bChecked? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        pEntry->SetUserData( new String( _rName ) );
    }

    //=========================================================================
    //= HangulHanjaNewDictDialog
    //=========================================================================
    //-------------------------------------------------------------------------

    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, OKHdl)
    {
        String  aName(comphelper::string::stripEnd(m_aDictNameED.GetText(), ' '));

        m_bEntered = aName.Len() > 0;
        if( m_bEntered )
            m_aDictNameED.SetText( aName );     // do this in case of trailing chars have been deleted

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, ModifyHdl)
    {
        String aName(comphelper::string::stripEnd(m_aDictNameED.GetText(), ' '));

        m_aOkBtn.Enable( aName.Len() > 0 );

        return 0;
    }

    HangulHanjaNewDictDialog::HangulHanjaNewDictDialog( Window* _pParent )
        :ModalDialog    ( _pParent, CUI_RES( RID_SVX_MDLG_HANGULHANJA_NEWDICT ) )
        ,m_aNewDictFL   ( this, CUI_RES( FL_NEWDICT ) )
        ,m_aDictNameFT  ( this, CUI_RES( FT_DICTNAME ) )
        ,m_aDictNameED  ( this, CUI_RES( ED_DICTNAME ) )
        ,m_aOkBtn       ( this, CUI_RES( PB_NEWDICT_OK ) )
        ,m_aCancelBtn   ( this, CUI_RES( PB_NEWDICT_ESC ) )
        ,m_aHelpBtn     ( this, CUI_RES( PB_NEWDICT_HLP ) )

        ,m_bEntered     ( false )
    {
        m_aOkBtn.SetClickHdl( LINK( this, HangulHanjaNewDictDialog, OKHdl ) );

        m_aDictNameED.SetModifyHdl( LINK( this, HangulHanjaNewDictDialog, ModifyHdl ) );

        FreeResource();
    }

    HangulHanjaNewDictDialog::~HangulHanjaNewDictDialog()
    {
    }

    bool HangulHanjaNewDictDialog::GetName( String& _rRetName ) const
    {
        if( m_bEntered )
            _rRetName = comphelper::string::stripEnd(m_aDictNameED.GetText(), ' ');

        return m_bEntered;
    }

    //=========================================================================
    //= HangulHanjaEditDictDialog
    //=========================================================================
    //-------------------------------------------------------------------------

    class SuggestionList
    {
    private:
    protected:
        sal_uInt16          m_nSize;
        String**            m_ppElements;
        sal_uInt16          m_nNumOfEntries;
        sal_uInt16          m_nAct;

        const String*       _Next( void );
    public:
                            SuggestionList( sal_uInt16 _nNumOfElements );
        virtual             ~SuggestionList();

        bool                Set( const String& _rElement, sal_uInt16 _nNumOfElement );
        bool                Reset( sal_uInt16 _nNumOfElement );
        const String*       Get( sal_uInt16 _nNumOfElement ) const;
        void                Clear( void );

        const String*       First( void );
        const String*       Next( void );

        inline sal_uInt16   GetCount( void ) const;
    };

    inline sal_uInt16 SuggestionList::GetCount( void ) const
    {
        return m_nNumOfEntries;
    }

    SuggestionList::SuggestionList( sal_uInt16 _nNumOfElements )
    {
        if( !_nNumOfElements )
            _nNumOfElements = 1;

        m_nSize = _nNumOfElements;

        m_ppElements = new String*[ m_nSize ];
        m_nAct = m_nNumOfEntries = 0;

        String**    ppNull = m_ppElements;
        sal_uInt16  n = _nNumOfElements;
        while( n )
        {
            *ppNull = NULL;
            ++ppNull;
            --n;
        }
    }

    SuggestionList::~SuggestionList()
    {
        Clear();
        delete[] m_ppElements;
    }

    bool SuggestionList::Set( const String& _rElement, sal_uInt16 _nNumOfElement )
    {
        bool    bRet = _nNumOfElement < m_nSize;
        if( bRet )
        {
            String**    ppElem = m_ppElements + _nNumOfElement;
            if( *ppElem )
                **ppElem = _rElement;
            else
            {
                *ppElem = new String( _rElement );
                ++m_nNumOfEntries;
            }
        }

        return bRet;
    }

    bool SuggestionList::Reset( sal_uInt16 _nNumOfElement )
    {
        bool    bRet = _nNumOfElement < m_nSize;
        if( bRet )
        {
            String**    ppElem = m_ppElements + _nNumOfElement;
            if( *ppElem )
            {
                delete *ppElem;
                *ppElem = NULL;
                --m_nNumOfEntries;
            }
        }

        return bRet;
    }

    const String* SuggestionList::Get( sal_uInt16 _nNumOfElement ) const
    {
        const String*   pRet;

        if( _nNumOfElement < m_nSize )
            pRet = m_ppElements[ _nNumOfElement ];
        else
            pRet = NULL;

        return pRet;
    }

    void SuggestionList::Clear( void )
    {
        if( m_nNumOfEntries )
        {
            String**    ppDel = m_ppElements;
            sal_uInt16  nCnt = m_nSize;
            while( nCnt )
            {
                if( *ppDel )
                {
                    delete *ppDel;
                    *ppDel = NULL;
                }

                ++ppDel;
                --nCnt;
            }

            m_nNumOfEntries = m_nAct = 0;
        }
    }

    const String* SuggestionList::_Next( void )
    {
        const String*   pRet = NULL;
        while( m_nAct < m_nSize && !pRet )
        {
            pRet = m_ppElements[ m_nAct ];
            if( !pRet )
                ++m_nAct;
        }

        return pRet;
    }

    const String* SuggestionList::First( void )
    {
        m_nAct = 0;
        return _Next();
    }

    const String* SuggestionList::Next( void )
    {
        const String*   pRet;

        if( m_nAct < m_nNumOfEntries )
        {
            ++m_nAct;
            pRet = _Next();
        }
        else
            pRet = NULL;

        return pRet;
    }


    bool SuggestionEdit::ShouldScroll( bool _bUp ) const
    {
        bool    bRet = false;

        if( _bUp )
        {
            if( !m_pPrev )
                bRet = m_rScrollBar.GetThumbPos() > m_rScrollBar.GetRangeMin();
        }
        else
        {
            if( !m_pNext )
                bRet = m_rScrollBar.GetThumbPos() < ( m_rScrollBar.GetRangeMax() - 4 );
        }

        return bRet;
    }

    void SuggestionEdit::DoJump( bool _bUp )
    {
        const Link&     rLoseFocusHdl = GetLoseFocusHdl();
        if( rLoseFocusHdl.IsSet() )
            rLoseFocusHdl.Call( this );
        m_rScrollBar.SetThumbPos( m_rScrollBar.GetThumbPos() + ( _bUp? -1 : 1 ) );

        ( static_cast< HangulHanjaEditDictDialog* >( GetParentDialog() ) )->UpdateScrollbar();
    }

    SuggestionEdit::SuggestionEdit( Window* pParent, const ResId& rResId,
        ScrollBar& _rScrollBar, SuggestionEdit* _pPrev, SuggestionEdit* _pNext  )
        :Edit( pParent, rResId )
        ,m_pPrev( _pPrev )
        ,m_pNext( _pNext )
        ,m_rScrollBar( _rScrollBar )
    {
    }

    SuggestionEdit::~SuggestionEdit()
    {
    }

    long SuggestionEdit::PreNotify( NotifyEvent& rNEvt )
    {
        long    nHandled = 0;
        if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent*             pKEvt = rNEvt.GetKeyEvent();
            const KeyCode&              rKeyCode = pKEvt->GetKeyCode();
            sal_uInt16                      nMod = rKeyCode.GetModifier();
            sal_uInt16                      nCode = rKeyCode.GetCode();
            if( nCode == KEY_TAB && ( !nMod || KEY_SHIFT == nMod ) )
            {
                bool        bUp = KEY_SHIFT == nMod;
                if( ShouldScroll( bUp ) )
                {
                    DoJump( bUp );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                        // Tab-travel doesn't really happen, so emulate it by setting a selection manually
                    nHandled = 1;
                }
            }
            else if( KEY_UP == nCode || KEY_DOWN == nCode )
            {
                bool        bUp = KEY_UP == nCode;
                if( ShouldScroll( bUp ) )
                {
                    DoJump( bUp );
                    nHandled = 1;
                }
                else if( bUp )
                {
                    if( m_pPrev )
                    {
                        m_pPrev->GrabFocus();
                        nHandled = 1;
                    }
                }
                else if( m_pNext )
                {
                    m_pNext->GrabFocus();
                    nHandled = 1;
                }
            }
        }

        if( !nHandled )
            nHandled = Edit::PreNotify( rNEvt );
        return nHandled;
    }


    namespace
    {
        bool GetConversions(    Reference< XConversionDictionary >  _xDict,
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
                                                        ::com::sun::star::i18n::TextConversionOption::NONE );
                    bRet = _rEntries.getLength() > 0;
                }
                catch( const IllegalArgumentException& )
                {
                }
            }

            return bRet;
        }
    }


    IMPL_LINK_NOARG(HangulHanjaEditDictDialog, ScrollHdl)
    {
        UpdateScrollbar();

        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaEditDictDialog, OriginalModifyHdl)
    {
        m_bModifiedOriginal = true;
        m_aOriginal = comphelper::string::stripEnd(m_aOriginalLB.GetText(), ' ');

        UpdateSuggestions();
        UpdateButtonStates();

        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl1, Edit*, pEdit )
    {
        EditModify( pEdit, 0 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl2, Edit*, pEdit )
    {
        EditModify( pEdit, 1 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl3, Edit*, pEdit )
    {
        EditModify( pEdit, 2 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditModifyHdl4, Edit*, pEdit )
    {
        EditModify( pEdit, 3 );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaEditDictDialog, BookLBSelectHdl)
    {
        InitEditDictDialog( m_aBookLB.GetSelectEntryPos() );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaEditDictDialog, NewPBPushHdl)
    {
        DBG_ASSERT( m_pSuggestions, "-HangulHanjaEditDictDialog::NewPBPushHdl(): no suggestions... search in hell..." );
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() && m_pSuggestions )
        {
            //delete old entry
            bool bRemovedSomething = DeleteEntryFromDictionary( m_aOriginal, xDict );

            OUString                aLeft( m_aOriginal );
            const String*           pRight = m_pSuggestions->First();
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

            if(bAddedSomething||bRemovedSomething)
                InitEditDictDialog( m_nCurrentDict );
        }
        else
        {
            DBG_WARNING( "+HangulHanjaEditDictDialog::NewPBPushHdl(): dictionary faded away..." );
        }
        return 0;
    }

    bool HangulHanjaEditDictDialog::DeleteEntryFromDictionary( const OUString&, const Reference< XConversionDictionary >& xDict  )
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

    IMPL_LINK_NOARG(HangulHanjaEditDictDialog, DeletePBPushHdl)
    {
        if( DeleteEntryFromDictionary( m_aOriginal, m_rDictList[ m_nCurrentDict ] ) )
        {
            m_aOriginal.Erase();
            m_bModifiedOriginal = true;
            InitEditDictDialog( m_nCurrentDict );
        }
        return 0;
    }

    void HangulHanjaEditDictDialog::InitEditDictDialog( sal_uInt32 _nSelDict )
    {
        if( m_pSuggestions )
            m_pSuggestions->Clear();

        if( m_nCurrentDict != _nSelDict )
        {
            m_nCurrentDict = _nSelDict;
            m_aOriginal.Erase();
            m_bModifiedOriginal = true;
        }

        UpdateOriginalLB();

        m_aOriginalLB.SetText( m_aOriginal.Len()? m_aOriginal : m_aEditHintText, Selection( 0, SELECTION_MAX ) );
        m_aOriginalLB.GrabFocus();

        UpdateSuggestions();
        UpdateButtonStates();
    }

    void HangulHanjaEditDictDialog::UpdateOriginalLB( void )
    {
        m_aOriginalLB.Clear();
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() )
        {
            Sequence< OUString >    aEntries = xDict->getConversionEntries( ConversionDirection_FROM_LEFT );
            sal_uInt32              n = aEntries.getLength();
            OUString*               pEntry = aEntries.getArray();
            while( n )
            {
                m_aOriginalLB.InsertEntry( *pEntry );

                ++pEntry;
                --n;
            }
        }
        else
        {
            DBG_WARNING( "+HangulHanjaEditDictDialog::UpdateOriginalLB(): dictionary faded away..." );
        }
    }

    void HangulHanjaEditDictDialog::UpdateButtonStates()
    {
        bool bHaveValidOriginalString = m_aOriginal.Len() && m_aOriginal != m_aEditHintText;
        bool bNew = bHaveValidOriginalString && m_pSuggestions && m_pSuggestions->GetCount() > 0;
        bNew = bNew && (m_bModifiedSuggestions || m_bModifiedOriginal);

        m_aNewPB.Enable( bNew );
        m_aDeletePB.Enable( !m_bModifiedOriginal && bHaveValidOriginalString );
    }

    void HangulHanjaEditDictDialog::UpdateSuggestions( void )
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
                    m_pSuggestions = new SuggestionList( MAXNUM_SUGGESTIONS );

                const OUString* pSugg = aEntries.getConstArray();
                sal_uInt32 n = 0;
                while( nCnt )
                {
                    m_pSuggestions->Set( pSugg[ n ], sal_uInt16( n ) );
                    ++n;
                    --nCnt;
                }
            }
            m_bModifiedSuggestions=false;
        }

        m_aScrollSB.SetThumbPos( 0 );
        UpdateScrollbar();              // will force edits to be filled new
    }

    void HangulHanjaEditDictDialog::SetEditText( Edit& _rEdit, sal_uInt16 _nEntryNum )
    {
        String  aStr;
        if( m_pSuggestions )
        {
            const String*   p = m_pSuggestions->Get( _nEntryNum );
            if( p )
                aStr = *p;
        }

        _rEdit.SetText( aStr );
    }

    void HangulHanjaEditDictDialog::EditModify( Edit* _pEdit, sal_uInt8 _nEntryOffset )
    {
        m_bModifiedSuggestions = true;

        String  aTxt( _pEdit->GetText() );
        sal_uInt16 nEntryNum = m_nTopPos + _nEntryOffset;
        if( aTxt.Len() == 0 )
        {
            //reset suggestion
            if( m_pSuggestions )
                m_pSuggestions->Reset( nEntryNum );
        }
        else
        {
            //set suggestion
            if( !m_pSuggestions )
                m_pSuggestions = new SuggestionList( MAXNUM_SUGGESTIONS );
            m_pSuggestions->Set( aTxt, nEntryNum );
        }

        UpdateButtonStates();
    }

    HangulHanjaEditDictDialog::HangulHanjaEditDictDialog( Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict )
        :ModalDialog            ( _pParent, CUI_RES( RID_SVX_MDLG_HANGULHANJA_EDIT ) )
        ,m_aEditHintText        ( CUI_RES( STR_EDITHINT ) )
        ,m_rDictList            ( _rDictList )
        ,m_nCurrentDict         ( 0xFFFFFFFF )
        ,m_pSuggestions         ( NULL )
        ,m_aBookFT              ( this, CUI_RES( FT_BOOK ) )
        ,m_aBookLB              ( this, CUI_RES( LB_BOOK ) )
        ,m_aOriginalFT          ( this, CUI_RES( FT_ORIGINAL ) )
        ,m_aOriginalLB          ( this, CUI_RES( LB_ORIGINAL ) )
        ,m_aSuggestionsFT       ( this, CUI_RES( FT_SUGGESTIONS ) )
        ,m_aEdit1               ( this, CUI_RES( ED_1 ), m_aScrollSB, NULL, &m_aEdit2 )
        ,m_aEdit2               ( this, CUI_RES( ED_2 ), m_aScrollSB, &m_aEdit1, &m_aEdit3 )
        ,m_aEdit3               ( this, CUI_RES( ED_3 ), m_aScrollSB, &m_aEdit2, &m_aEdit4 )
        ,m_aEdit4               ( this, CUI_RES( ED_4 ), m_aScrollSB, &m_aEdit3, NULL )
        ,m_aScrollSB            ( this, CUI_RES( SB_SCROLL ) )
        ,m_aNewPB               ( this, CUI_RES( PB_HHE_NEW ) )
        ,m_aDeletePB            ( this, CUI_RES( PB_HHE_DELETE ) )
        ,m_aHelpPB              ( this, CUI_RES( PB_HHE_HELP ) )
        ,m_aClosePB             ( this, CUI_RES( PB_HHE_CLOSE ) )
        ,m_nTopPos              ( 0 )
        ,m_bModifiedSuggestions ( false )
        ,m_bModifiedOriginal    ( false )
    {
        m_aOriginalLB.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, OriginalModifyHdl ) );

        m_aNewPB.SetClickHdl( LINK( this, HangulHanjaEditDictDialog, NewPBPushHdl ) );
        m_aNewPB.Enable( false );

        m_aDeletePB.SetClickHdl( LINK( this, HangulHanjaEditDictDialog, DeletePBPushHdl ) );

        m_aDeletePB.Enable( false );

    #if( MAXNUM_SUGGESTIONS <= 4 )
        #error number of suggestions should not under-run the value of 5
    #endif

        Link    aScrLk( LINK( this, HangulHanjaEditDictDialog, ScrollHdl ) );
        m_aScrollSB.SetScrollHdl( aScrLk );
        m_aScrollSB.SetEndScrollHdl( aScrLk );
        m_aScrollSB.SetRangeMin( 0 );
        m_aScrollSB.SetRangeMax( MAXNUM_SUGGESTIONS );
        m_aScrollSB.SetPageSize( 4 );       // because we have 4 edits / page
        m_aScrollSB.SetVisibleSize( 4 );

        m_aEdit1.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl1 ) );
        m_aEdit2.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl2 ) );
        m_aEdit3.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl3 ) );
        m_aEdit4.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl4 ) );

        m_aBookLB.SetSelectHdl( LINK( this, HangulHanjaEditDictDialog, BookLBSelectHdl ) );
        sal_uInt32  nDictCnt = m_rDictList.size();
        for( sal_uInt32 n = 0 ; n < nDictCnt ; ++n )
        {
            Reference< XConversionDictionary >  xDic( m_rDictList[n] );
            String aName;
            if(xDic.is())
                aName = xDic->getName();
            m_aBookLB.InsertEntry( aName );
        }
        m_aBookLB.SelectEntryPos( sal_uInt16( _nSelDict ) );

        FreeResource();

        InitEditDictDialog( _nSelDict );
    }

    HangulHanjaEditDictDialog::~HangulHanjaEditDictDialog()
    {
        if( m_pSuggestions )
            delete m_pSuggestions;
    }

    void HangulHanjaEditDictDialog::UpdateScrollbar( void )
    {
        sal_uInt16  nPos = sal_uInt16( m_aScrollSB.GetThumbPos() );
        m_nTopPos = nPos;

        SetEditText( m_aEdit1, nPos++ );
        SetEditText( m_aEdit2, nPos++ );
        SetEditText( m_aEdit3, nPos++ );
        SetEditText( m_aEdit4, nPos );
    }

//.............................................................................
}   // namespace svx
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
