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
#define MAXNUM_SUGGESTIONS 50


namespace svx
{

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::linguistic2;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;


    namespace
    {
        class FontSwitch
        {
        private:
            OutputDevice& m_rDev;

        public:
            inline FontSwitch( OutputDevice& _rDev, const vcl::Font& _rTemporaryFont )
                :m_rDev( _rDev )
            {
                m_rDev.Push( PushFlags::FONT );
                m_rDev.SetFont( _rTemporaryFont );
            }
            inline ~FontSwitch( )
            {
                m_rDev.Pop( );
            }
        };
    }

    //= PseudoRubyText

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
        void Paint( OutputDevice& _rDevice, const Rectangle& _rRect, sal_uInt16 _nTextStyle,
            Rectangle* _pPrimaryLocation = NULL, Rectangle* _pSecondaryLocation = NULL,
            ::vcl::ControlLayoutData* _pLayoutData = NULL );
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


    void PseudoRubyText::Paint( OutputDevice& _rDevice, const Rectangle& _rRect, sal_uInt16 _nTextStyle,
        Rectangle* _pPrimaryLocation, Rectangle* _pSecondaryLocation, ::vcl::ControlLayoutData* _pLayoutData )
    {
        bool            bLayoutOnly  = NULL != _pLayoutData;
        MetricVector*   pTextMetrics = bLayoutOnly ? &_pLayoutData->m_aUnicodeBoundRects : NULL;
        OUString*       pDisplayText = bLayoutOnly ? &_pLayoutData->m_aDisplayText       : NULL;

        Size aPlaygroundSize( _rRect.GetSize() );

        // the font for the secondary text:
        vcl::Font aSmallerFont( _rDevice.GetFont() );
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

    //= RubyRadioButton

    class RubyRadioButton   :public RadioButton
    {

    public:
        RubyRadioButton( vcl::Window* _pParent, WinBits nBits );
        void init( const OUString& rPrimaryText, const OUString& rSecondaryText, const PseudoRubyText::RubyPosition& rPosition );
        virtual Size    GetOptimalSize() const SAL_OVERRIDE;

    protected:
        virtual void    Paint( const Rectangle& _rRect ) SAL_OVERRIDE;

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


    void RubyRadioButton::Paint( const Rectangle& )
    {
        HideFocus();

        // calculate the size of the radio image - we're to paint our text _after_ this image
        DBG_ASSERT( !GetModeRadioImage(), "RubyRadioButton::Paint: images not supported!" );
        Size aImageSize = GetRadioImage( GetSettings(), 0 ).GetSizePixel();
        aImageSize.Width()  = CalcZoom( aImageSize.Width() ) + 2;   // + 2 because otherwise the radiobuttons
        aImageSize.Height()  = CalcZoom( aImageSize.Height() ) + 2; // appear a bit cut from right and top.

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
        m_aRubyText.Paint( *this, aTextRect, nTextStyle, &aPrimaryTextLocation, &aSecondaryTextLocation );

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

    Size RubyRadioButton::GetOptimalSize() const
    {
        vcl::Font aSmallerFont( GetFont() );
        aSmallerFont.SetHeight( static_cast<long>( 0.8 * aSmallerFont.GetHeight() ) );
        Rectangle rect( Point(), Size( SAL_MAX_INT32, SAL_MAX_INT32 ) );
        sal_uInt16 style = GetStyle();

        Size aPrimarySize = GetTextRect( rect, m_aRubyText.getPrimaryText(), style ).GetSize();
        Size aSecondarySize;
        {
            FontSwitch aFontRestore( const_cast<RubyRadioButton&>(*this), aSmallerFont );
            aSecondarySize = GetTextRect( rect, m_aRubyText.getSecondaryText(), style ).GetSize();
        }

        Size minimumSize =  CalcMinimumSize();
        minimumSize.Height() = aPrimarySize.Height() + aSecondarySize.Height() + 5;
        minimumSize.Width() = aPrimarySize.Width() + aSecondarySize.Width() + 5;
        return minimumSize;
    }

    extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeRubyRadioButton( vcl::Window *pParent, VclBuilder::stringmap & )
    {
        return new RubyRadioButton( pParent, WB_LEFT|WB_VCENTER );
    }


    //= SuggestionSet

    SuggestionSet::SuggestionSet( vcl::Window* pParent )
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

        OUString sText = *static_cast< OUString* >( GetItemData( nItemId ) );
        pDev->DrawText( aRect, sText, TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER );
    }

    void SuggestionSet::ClearSet()
    {
        sal_uInt16 i, nCount = GetItemCount();
        for ( i = 0; i < nCount; ++i )
            delete static_cast< OUString* >( GetItemData(i) );
        Clear();
    }


    //= SuggestionDisplay


    SuggestionDisplay::SuggestionDisplay( vcl::Window* pParent, WinBits nBits )
        : Control( pParent, nBits )
        , m_bDisplayListBox( true )
        , m_aValueSet( this )
        , m_aListBox( this,GetStyle() | WB_BORDER )
        , m_bInSelectionUpdate( false )
    {
        m_aValueSet.SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionHdl ) );
        m_aListBox.SetSelectHdl( LINK( this, SuggestionDisplay, SelectSuggestionHdl ) );

        m_aValueSet.SetLineCount( LINE_CNT );
        m_aValueSet.SetStyle( m_aValueSet.GetStyle() | WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL );
        m_aValueSet.SetBorderStyle( WindowBorderStyle::MONO );
        OUString aOneCharacter("AU");
        long nItemWidth = 2*GetTextWidth( aOneCharacter );
        m_aValueSet.SetItemWidth( nItemWidth );

        Size aSize( approximate_char_width() * 48, GetTextHeight() * 5 );
        m_aValueSet.SetSizePixel( aSize );
        m_aListBox.SetSizePixel( aSize );

        implUpdateDisplay();
    }

    SuggestionDisplay::~SuggestionDisplay()
    {
    }

    void SuggestionDisplay::implUpdateDisplay()
    {
        bool bShowBox = IsVisible() && m_bDisplayListBox;
        bool bShowSet = IsVisible() && !m_bDisplayListBox;

        m_aListBox.Show( bShowBox );
        m_aValueSet.Show( bShowSet );
    }

    void SuggestionDisplay::StateChanged( StateChangedType nStateChange )
    {
        if( StateChangedType::VISIBLE == nStateChange )
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

    IMPL_LINK( SuggestionDisplay, SelectSuggestionHdl, Control*, pControl )
    {
        if( m_bInSelectionUpdate )
            return 0L;

        m_bInSelectionUpdate = true;
        if( pControl == &m_aListBox )
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
        m_aSelectLink.Call( this );
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
    void SuggestionDisplay::InsertEntry( const OUString& rStr )
    {
        sal_uInt16 nItemId = m_aListBox.InsertEntry( rStr ) + 1; //itemid == pos+1 (id 0 has special meaning)
        m_aValueSet.InsertItem( nItemId );
        OUString* pItemData = new OUString( rStr );
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
    OUString SuggestionDisplay::GetEntry( sal_uInt16 nPos ) const
    {
        return m_aListBox.GetEntry( nPos );
    }
    OUString SuggestionDisplay::GetSelectEntry() const
    {
        return m_aListBox.GetSelectEntry();
    }
    void SuggestionDisplay::SetHelpIds()
    {
        this->SetHelpId( HID_HANGULDLG_SUGGESTIONS );
        m_aValueSet.SetHelpId( HID_HANGULDLG_SUGGESTIONS_GRID );
        m_aListBox.SetHelpId( HID_HANGULDLG_SUGGESTIONS_LIST );
    }

    extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSuggestionDisplay( vcl::Window *pParent, VclBuilder::stringmap & )
    {
        return new SuggestionDisplay( pParent, WB_ITEMBORDER | WB_FLATVALUESET | WB_VSCROLL );
    }


    //= HangulHanjaConversionDialog

    HangulHanjaConversionDialog::HangulHanjaConversionDialog( vcl::Window* _pParent, HHC::ConversionDirection _ePrimaryDirection )
        :ModalDialog( _pParent, "HangulHanjaConversionDialog", "cui/ui/hangulhanjaconversiondialog.ui" )
        ,m_pIgnoreNonPrimary( NULL )
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

        const OUString sHangul = "Hangul";
        const OUString sHanja = "Hanja";
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


    HangulHanjaConversionDialog::~HangulHanjaConversionDialog( )
    {
    }


    void HangulHanjaConversionDialog::FillSuggestions( const ::com::sun::star::uno::Sequence< OUString >& _rSuggestions )
    {
        m_pSuggestions->Clear();
        const OUString* pSuggestions = _rSuggestions.getConstArray();
        const OUString* pSuggestionsEnd = _rSuggestions.getConstArray() + _rSuggestions.getLength();
        while ( pSuggestions != pSuggestionsEnd )
            m_pSuggestions->InsertEntry( *pSuggestions++ );

        // select the first suggestion, and fill in the suggestion edit field
        OUString sFirstSuggestion;
        if ( m_pSuggestions->GetEntryCount() )
        {
            sFirstSuggestion = m_pSuggestions->GetEntry( 0 );
            m_pSuggestions->SelectEntryPos( 0 );
        }
        m_pWordInput->SetText( sFirstSuggestion );
        m_pWordInput->SaveValue();
        OnSuggestionModified( m_pWordInput );
    }


    void HangulHanjaConversionDialog::SetOptionsChangedHdl( const Link& _rHdl )
    {
        m_aOptionsChangedLink = _rHdl;
    }


    void HangulHanjaConversionDialog::SetIgnoreHdl( const Link& _rHdl )
    {
        m_pIgnore->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetIgnoreAllHdl( const Link& _rHdl )
    {
        m_pIgnoreAll->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetChangeHdl( const Link& _rHdl )
    {
        m_pReplace->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetChangeAllHdl( const Link& _rHdl )
    {
        m_pReplaceAll->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetFindHdl( const Link& _rHdl )
    {
        m_pFind->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetConversionFormatChangedHdl( const Link& _rHdl )
    {
        m_pSimpleConversion->SetClickHdl( _rHdl );
        m_pHangulBracketed->SetClickHdl( _rHdl );
        m_pHanjaBracketed->SetClickHdl( _rHdl );
        m_pHanjaAbove->SetClickHdl( _rHdl );
        m_pHanjaBelow->SetClickHdl( _rHdl );
        m_pHangulAbove->SetClickHdl( _rHdl );
        m_pHangulBelow->SetClickHdl( _rHdl );
    }


    void HangulHanjaConversionDialog::SetClickByCharacterHdl( const Link& _rHdl )
    {
        m_aClickByCharacterLink = _rHdl;
    }


    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionSelected )
    {
        m_pWordInput->SetText( m_pSuggestions->GetSelectEntry() );
        OnSuggestionModified( NULL );
        return 0L;
    }


    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnSuggestionModified )
    {
        m_pFind->Enable( m_pWordInput->IsValueChangedFromSaved() );

        bool bSameLen = m_pWordInput->GetText().getLength() == m_pOriginalWord->GetText().getLength();
        m_pReplace->Enable( m_bDocumentMode && bSameLen );
        m_pReplaceAll->Enable( m_bDocumentMode && bSameLen );

        return 0L;
    }


    IMPL_LINK( HangulHanjaConversionDialog, ClickByCharacterHdl, CheckBox *, pBox )
    {
        m_aClickByCharacterLink.Call( pBox );

        bool bByCharacter = pBox->IsChecked();
        m_pSuggestions->DisplayListBox( !bByCharacter );

        return 0L;
    }


    IMPL_LINK( HangulHanjaConversionDialog, OnConversionDirectionClicked, CheckBox *, pBox )
    {
        CheckBox *pOtherBox = 0;
        if ( pBox == m_pHangulOnly )
            pOtherBox = m_pHanjaOnly;
        else if ( pBox == m_pHanjaOnly )
            pOtherBox = m_pHangulOnly;
        if ( pBox && pOtherBox )
        {
            bool bBoxChecked = pBox->IsChecked();
            if ( bBoxChecked )
                pOtherBox->Check( false );
            pOtherBox->Enable( !bBoxChecked );
        }

        return 0L;
    }

    IMPL_LINK_NOARG( HangulHanjaConversionDialog, OnOption )
    {
        HangulHanjaOptionsDialog        aOptDlg( this );
        aOptDlg.Execute();
        m_aOptionsChangedLink.Call( this );
        return 0L;
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
            vcl::Window* pOldDefButton = NULL;
            vcl::Window* pNewDefButton = NULL;
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
            sal_uInt32 nSaveFocusId = Window::SaveFocus();
            pNewDefButton->GrabFocus();
            Window::EndSaveFocus( nSaveFocusId );
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
        m_pHangulOnly->Enable( true );
        m_pHanjaOnly->Check( false );
        m_pHanjaOnly->Enable( true );

        if (!_bTryBothDirections)
        {
            CheckBox *pBox = _ePrimaryConversionDirection == HHC::eHangulToHanja?
                                    m_pHangulOnly : m_pHanjaOnly;
            pBox->Check( true );
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



    //= HangulHanjaOptionsDialog



    void HangulHanjaOptionsDialog::Init( void )
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
            SvTreeListEntry*                        pEntry = m_pDictsLB->SvTreeListBox::GetEntry( n );

            DBG_ASSERT( xDict.is(), "-HangulHanjaOptionsDialog::OkHdl(): someone is evaporated..." );
            DBG_ASSERT( pEntry, "-HangulHanjaOptionsDialog::OkHdl(): no one there in list?" );

            bool    bActive = m_pDictsLB->GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED;
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

        aTmp <<= bool( m_pIgnorepostCB->IsChecked() );
        aLngCfg.SetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD, aTmp );

        aTmp <<= bool( m_pShowrecentlyfirstCB->IsChecked() );
        aLngCfg.SetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST, aTmp );

        aTmp <<= bool( m_pAutoreplaceuniqueCB->IsChecked() );
        aLngCfg.SetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES, aTmp );

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DictsLB_SelectHdl)
    {
        bool bSel = m_pDictsLB->FirstSelected() != NULL;

        m_pEditPB->Enable(bSel);
        m_pDeletePB->Enable(bSel);

        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, NewDictHdl)
    {
        OUString                    aName;
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
        SvTreeListEntry*    pEntry = m_pDictsLB->FirstSelected();
        DBG_ASSERT( pEntry, "+HangulHanjaEditDictDialog::EditDictHdl(): call of edit should not be possible with no selection!" );
        if( pEntry )
        {
            HangulHanjaEditDictDialog   aEdDlg( this, m_aDictList, m_pDictsLB->GetSelectEntryPos() );
            aEdDlg.Execute();
        }
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaOptionsDialog, DeleteDictHdl)
    {
        sal_uLong nSelPos = m_pDictsLB->GetSelectEntryPos();
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

        return 0L;
    }

    HangulHanjaOptionsDialog::HangulHanjaOptionsDialog(vcl::Window* _pParent)
        : ModalDialog( _pParent, "HangulHanjaOptDialog",
            "cui/ui/hangulhanjaoptdialog.ui" )
        , m_pCheckButtonData(NULL)
        , m_xConversionDictionaryList(NULL)
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
        m_pDictsLB->SetStyle( m_pDictsLB->GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
        m_pDictsLB->SetSelectionMode( SINGLE_SELECTION );
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
        SvTreeListEntry*    pEntry = m_pDictsLB->First();
        OUString*         pDel;
        while( pEntry )
        {
            pDel = ( OUString* ) pEntry->GetUserData();
            if( pDel )
                delete pDel;
            pEntry = m_pDictsLB->Next( pEntry );
        }

        if( m_pCheckButtonData )
            delete m_pCheckButtonData;
    }

    void HangulHanjaOptionsDialog::AddDict( const OUString& _rName, bool _bChecked )
    {
        SvTreeListEntry*    pEntry = m_pDictsLB->SvTreeListBox::InsertEntry( _rName );
        m_pDictsLB->SetCheckButtonState( pEntry, _bChecked? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        pEntry->SetUserData( new OUString( _rName ) );
    }


    //= HangulHanjaNewDictDialog



    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, OKHdl)
    {
        OUString  aName(comphelper::string::stripEnd(m_pDictNameED->GetText(), ' '));

        m_bEntered = !aName.isEmpty();
        if( m_bEntered )
            m_pDictNameED->SetText( aName );     // do this in case of trailing chars have been deleted

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK_NOARG(HangulHanjaNewDictDialog, ModifyHdl)
    {
        OUString aName(comphelper::string::stripEnd(m_pDictNameED->GetText(), ' '));

        m_pOkBtn->Enable( !aName.isEmpty() );

        return 0;
    }

    HangulHanjaNewDictDialog::HangulHanjaNewDictDialog(vcl::Window* pParent)
        : ModalDialog(pParent, "HangulHanjaAddDialog", "cui/ui/hangulhanjaadddialog.ui")
        , m_bEntered(false)
    {
        get(m_pOkBtn, "ok");
        get(m_pDictNameED, "entry");

        m_pOkBtn->SetClickHdl( LINK( this, HangulHanjaNewDictDialog, OKHdl ) );
        m_pDictNameED->SetModifyHdl( LINK( this, HangulHanjaNewDictDialog, ModifyHdl ) );
    }

    bool HangulHanjaNewDictDialog::GetName( OUString& _rRetName ) const
    {
        if( m_bEntered )
            _rRetName = comphelper::string::stripEnd(m_pDictNameED->GetText(), ' ');

        return m_bEntered;
    }


    //= HangulHanjaEditDictDialog



    class SuggestionList
    {
    private:
    protected:
        std::vector<OUString*> m_vElements;
        sal_uInt16          m_nNumOfEntries;
        // index of the internal iterator, used for First() and Next() methods
        sal_uInt16          m_nAct;

        const OUString*       _Next( void );
    public:
                            SuggestionList();
                            ~SuggestionList();

        bool                Set( const OUString& _rElement, sal_uInt16 _nNumOfElement );
        bool                Reset( sal_uInt16 _nNumOfElement );
        const OUString*     Get( sal_uInt16 _nNumOfElement ) const;
        void                Clear( void );

        const OUString*     First( void );
        const OUString*     Next( void );

        inline sal_uInt16   GetCount( void ) const { return m_nNumOfEntries; }
    };

    SuggestionList::SuggestionList() :
        m_vElements(MAXNUM_SUGGESTIONS, static_cast<OUString*>(NULL))
    {
        m_nAct = m_nNumOfEntries = 0;
    }

    SuggestionList::~SuggestionList()
    {
        Clear();
    }

    bool SuggestionList::Set( const OUString& _rElement, sal_uInt16 _nNumOfElement )
    {
        bool    bRet = _nNumOfElement < m_vElements.size();
        if( bRet )
        {
            if( m_vElements[_nNumOfElement] != NULL )
                *(m_vElements[_nNumOfElement]) = _rElement;
            else
            {
                m_vElements[_nNumOfElement] = new OUString( _rElement );
                ++m_nNumOfEntries;
            }
        }

        return bRet;
    }

    bool SuggestionList::Reset( sal_uInt16 _nNumOfElement )
    {
        bool    bRet = _nNumOfElement < m_vElements.size();
        if( bRet )
        {
            if( m_vElements[_nNumOfElement] != NULL )
            {
                delete m_vElements[_nNumOfElement];
                m_vElements[_nNumOfElement] = NULL;
                --m_nNumOfEntries;
            }
        }

        return bRet;
    }

    const OUString* SuggestionList::Get( sal_uInt16 _nNumOfElement ) const
    {
        if( _nNumOfElement < m_vElements.size())
            return m_vElements[_nNumOfElement];
        return NULL;
    }

    void SuggestionList::Clear( void )
    {
        if( m_nNumOfEntries )
        {
            for( std::vector<OUString*>::iterator it = m_vElements.begin(); it != m_vElements.end(); ++it )
                if( *it != NULL )
                {
                    delete *it;
                    *it = NULL;
                 }

            m_nNumOfEntries = m_nAct = 0;
        }
    }

    const OUString* SuggestionList::_Next( void )
    {
        const OUString*   pRet = NULL;
        while( m_nAct < m_vElements.size() && !pRet )
        {
            pRet = m_vElements[ m_nAct ];
            if( !pRet )
                ++m_nAct;
        }

        return pRet;
    }

    const OUString* SuggestionList::First( void )
    {
        m_nAct = 0;
        return _Next();
    }

    const OUString* SuggestionList::Next( void )
    {
        const OUString*   pRet;

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
                bRet = m_pScrollBar->GetThumbPos() > m_pScrollBar->GetRangeMin();
        }
        else
        {
            if( !m_pNext )
                bRet = m_pScrollBar->GetThumbPos() < ( m_pScrollBar->GetRangeMax() - 4 );
        }

        return bRet;
    }

    void SuggestionEdit::DoJump( bool _bUp )
    {
        const Link&     rLoseFocusHdl = GetLoseFocusHdl();
        if( rLoseFocusHdl.IsSet() )
            rLoseFocusHdl.Call( this );
        m_pScrollBar->SetThumbPos( m_pScrollBar->GetThumbPos() + ( _bUp? -1 : 1 ) );

        ( static_cast< HangulHanjaEditDictDialog* >( GetParentDialog() ) )->UpdateScrollbar();
    }

    SuggestionEdit::SuggestionEdit( vcl::Window* pParent, WinBits nBits )
        : Edit(pParent, nBits)
        , m_pPrev(NULL)
        , m_pNext(NULL)
        , m_pScrollBar(NULL)
    {
    }

    SuggestionEdit::~SuggestionEdit()
    {
    }

    bool SuggestionEdit::PreNotify( NotifyEvent& rNEvt )
    {
        bool nHandled = false;
        if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent*     pKEvt = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rKeyCode = pKEvt->GetKeyCode();
            sal_uInt16          nMod = rKeyCode.GetModifier();
            sal_uInt16          nCode = rKeyCode.GetCode();
            if( nCode == KEY_TAB && ( !nMod || KEY_SHIFT == nMod ) )
            {
                bool        bUp = KEY_SHIFT == nMod;
                if( ShouldScroll( bUp ) )
                {
                    DoJump( bUp );
                    SetSelection( Selection( 0, SELECTION_MAX ) );
                        // Tab-travel doesn't really happen, so emulate it by setting a selection manually
                    nHandled = true;
                }
            }
            else if( KEY_UP == nCode || KEY_DOWN == nCode )
            {
                bool        bUp = KEY_UP == nCode;
                if( ShouldScroll( bUp ) )
                {
                    DoJump( bUp );
                    nHandled = true;
                }
                else if( bUp )
                {
                    if( m_pPrev )
                    {
                        m_pPrev->GrabFocus();
                        nHandled = true;
                    }
                }
                else if( m_pNext )
                {
                    m_pNext->GrabFocus();
                    nHandled = true;
                }
            }
        }

        if( !nHandled )
            nHandled = Edit::PreNotify( rNEvt );
        return nHandled;
    }

    void SuggestionEdit::init( ScrollBar* pScrollBar, SuggestionEdit* pPrev, SuggestionEdit* pNext)
    {
        m_pScrollBar = pScrollBar;
        m_pPrev = pPrev;
        m_pNext = pNext;
    }

    extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeSuggestionEdit( vcl::Window *pParent, VclBuilder::stringmap & )
    {
        return new SuggestionEdit( pParent, WB_LEFT|WB_VCENTER|WB_BORDER );
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


    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, ScrollHdl )
    {
        UpdateScrollbar();

        return 0;
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, OriginalModifyHdl )
    {
        m_bModifiedOriginal = true;
        m_aOriginal = comphelper::string::stripEnd( m_aOriginalLB->GetText(), ' ' );

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

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, BookLBSelectHdl )
    {
        InitEditDictDialog( m_aBookLB->GetSelectEntryPos() );
        return 0;
    }

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, NewPBPushHdl )
    {
        DBG_ASSERT( m_pSuggestions, "-HangulHanjaEditDictDialog::NewPBPushHdl(): no suggestions... search in hell..." );
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() && m_pSuggestions )
        {
            //delete old entry
            bool bRemovedSomething = DeleteEntryFromDictionary( m_aOriginal, xDict );

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

    IMPL_LINK_NOARG( HangulHanjaEditDictDialog, DeletePBPushHdl )
    {
        if( DeleteEntryFromDictionary( m_aOriginal, m_rDictList[ m_nCurrentDict ] ) )
        {
            m_aOriginal = "";
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
            m_aOriginal = "";
            m_bModifiedOriginal = true;
        }

        UpdateOriginalLB();

        m_aOriginalLB->SetText( !m_aOriginal.isEmpty() ? m_aOriginal : m_aEditHintText, Selection( 0, SELECTION_MAX ) );
        m_aOriginalLB->GrabFocus();

        UpdateSuggestions();
        UpdateButtonStates();
    }

    void HangulHanjaEditDictDialog::UpdateOriginalLB( void )
    {
        m_aOriginalLB->Clear();
        Reference< XConversionDictionary >  xDict = m_rDictList[ m_nCurrentDict ];
        if( xDict.is() )
        {
            Sequence< OUString >    aEntries = xDict->getConversionEntries( ConversionDirection_FROM_LEFT );
            sal_uInt32              n = aEntries.getLength();
            OUString*               pEntry = aEntries.getArray();
            while( n )
            {
                m_aOriginalLB->InsertEntry( *pEntry );

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
        bool bHaveValidOriginalString = !m_aOriginal.isEmpty() && m_aOriginal != m_aEditHintText;
        bool bNew = bHaveValidOriginalString && m_pSuggestions && m_pSuggestions->GetCount() > 0;
        bNew = bNew && ( m_bModifiedSuggestions || m_bModifiedOriginal );

        m_aNewPB->Enable( bNew );
        m_aDeletePB->Enable(!m_bModifiedOriginal && bHaveValidOriginalString);
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
                    m_pSuggestions = new SuggestionList;

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

        m_aScrollSB->SetThumbPos( 0 );
        UpdateScrollbar();              // will force edits to be filled new
    }

    void HangulHanjaEditDictDialog::SetEditText( Edit& _rEdit, sal_uInt16 _nEntryNum )
    {
        OUString  aStr;
        if( m_pSuggestions )
        {
            const OUString*   p = m_pSuggestions->Get( _nEntryNum );
            if( p )
                aStr = *p;
        }

        _rEdit.SetText( aStr );
    }

    void HangulHanjaEditDictDialog::EditModify( Edit* _pEdit, sal_uInt8 _nEntryOffset )
    {
        m_bModifiedSuggestions = true;

        OUString  aTxt( _pEdit->GetText() );
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
                m_pSuggestions = new SuggestionList;
            m_pSuggestions->Set( aTxt, nEntryNum );
        }

        UpdateButtonStates();
    }

    HangulHanjaEditDictDialog::HangulHanjaEditDictDialog( vcl::Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict )
        :ModalDialog            ( _pParent, "HangulHanjaEditDictDialog", "cui/ui/hangulhanjaeditdictdialog.ui" )
        ,m_aEditHintText        ( CUI_RESSTR(RID_SVXSTR_EDITHINT) )
        ,m_rDictList            ( _rDictList )
        ,m_nCurrentDict         ( 0xFFFFFFFF )
        ,m_pSuggestions         ( NULL )
        ,m_nTopPos              ( 0 )
        ,m_bModifiedSuggestions ( false )
        ,m_bModifiedOriginal    ( false )
    {
        get( m_aBookLB, "book" );
        get( m_aOriginalLB, "original" );
        get( m_aNewPB, "new" );
        get( m_aDeletePB, "delete" );
        get( m_aScrollSB, "scrollbar" );
        get( m_aEdit1, "edit1" );
        get( m_aEdit2, "edit2" );
        get( m_aEdit3, "edit3" );
        get( m_aEdit4, "edit4" );

        m_aEdit1->init( m_aScrollSB, NULL, m_aEdit2 );
        m_aEdit2->init( m_aScrollSB, m_aEdit1, m_aEdit3 );
        m_aEdit3->init( m_aScrollSB, m_aEdit2, m_aEdit4 );
        m_aEdit4->init( m_aScrollSB, m_aEdit3, NULL );

        m_aOriginalLB->SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, OriginalModifyHdl ) );

        m_aNewPB->SetClickHdl( LINK( this, HangulHanjaEditDictDialog, NewPBPushHdl ) );
        m_aNewPB->Enable( false );

        m_aDeletePB->SetClickHdl( LINK( this, HangulHanjaEditDictDialog, DeletePBPushHdl ) );
        m_aDeletePB->Enable( false );

    #if( MAXNUM_SUGGESTIONS <= 4 )
        #error number of suggestions should not under-run the value of 5
    #endif

        Link    aScrLk( LINK( this, HangulHanjaEditDictDialog, ScrollHdl ) );
        m_aScrollSB->SetScrollHdl( aScrLk );
        m_aScrollSB->SetEndScrollHdl( aScrLk );
        m_aScrollSB->SetRangeMin( 0 );
        m_aScrollSB->SetRangeMax( MAXNUM_SUGGESTIONS );
        m_aScrollSB->SetPageSize( 4 );       // because we have 4 edits / page
        m_aScrollSB->SetVisibleSize( 4 );

        m_aEdit1->SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl1 ) );
        m_aEdit2->SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl2 ) );
        m_aEdit3->SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl3 ) );
        m_aEdit4->SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, EditModifyHdl4 ) );

        m_aBookLB->SetSelectHdl( LINK( this, HangulHanjaEditDictDialog, BookLBSelectHdl ) );
        sal_uInt32  nDictCnt = m_rDictList.size();
        for( sal_uInt32 n = 0 ; n < nDictCnt ; ++n )
        {
            Reference< XConversionDictionary >  xDic( m_rDictList[n] );
            OUString aName;
            if( xDic.is() )
                aName = xDic->getName();
            m_aBookLB->InsertEntry( aName );
        }
        m_aBookLB->SelectEntryPos( sal_uInt16( _nSelDict ) );

        InitEditDictDialog( _nSelDict );
    }

    HangulHanjaEditDictDialog::~HangulHanjaEditDictDialog()
    {
        if( m_pSuggestions )
            delete m_pSuggestions;
    }

    void HangulHanjaEditDictDialog::UpdateScrollbar( void )
    {
        sal_uInt16  nPos = sal_uInt16( m_aScrollSB->GetThumbPos() );
        m_nTopPos = nPos;

        SetEditText( *m_aEdit1, nPos++ );
        SetEditText( *m_aEdit2, nPos++ );
        SetEditText( *m_aEdit3, nPos++ );
        SetEditText( *m_aEdit4, nPos );
    }


}   // namespace svx


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
