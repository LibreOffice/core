/*************************************************************************
 *
 *  $RCSfile: hangulhanjadlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:41:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_HANGUL_HANJA_DLG_HXX
#include "hangulhanjadlg.hxx"
#endif
#ifndef SVX_HANGUL_HANJA_DLG_HRC
#include "hangulhanjadlg.hrc"
#endif

#ifndef SVX_COMMON_LINGUI_HXX
#include "commonlingui.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#include "dialogs.hrc"

#include <algorithm>

#ifndef _VCL_CONTROLLAYOUT_HXX
#include <vcl/controllayout.hxx>
#endif

#define HHC HangulHanjaConversion

//.............................................................................
namespace svx
{
//.............................................................................
/*
    using HangulHanjaConversion::eSimpleConversion;
    using HangulHanjaConversion::eHangulBracketed;
    using HangulHanjaConversion::eHanjaBracketed;
    using HangulHanjaConversion::eRubyHanjaAbove;
    using HangulHanjaConversion::eRubyHanjaBelow;
    using HangulHanjaConversion::eRubyHangulAbove;
    using HangulHanjaConversion::eRubyHangulBelow;
*/
    using namespace ::com::sun::star::uno;

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
        void Paint( OutputDevice& _rDevice, const Rectangle& _rRect, USHORT _nTextStyle,
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
    void PseudoRubyText::Paint( OutputDevice& _rDevice, const Rectangle& _rRect, USHORT _nTextStyle,
        Rectangle* _pPrimaryLocation, Rectangle* _pSecondaryLocation, ::vcl::ControlLayoutData* _pLayoutData )
    {
        bool            bLayoutOnly  = NULL != _pLayoutData;
        MetricVector*   pTextMetrics = bLayoutOnly ? &_pLayoutData->m_aUnicodeBoundRects : NULL;
        String*         pDisplayText = bLayoutOnly ? &_pLayoutData->m_aDisplayText       : NULL;

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
        USHORT nDrawTextStyle( _nTextStyle );
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
    void RubyRadioButton::Paint( const Rectangle& _rRect )
    {
        HideFocus();

        // calculate the size of the radio image - we're to paint our text _after_ this image
        DBG_ASSERT( !GetImage(), "RubyRadioButton::Paint: images not supported!" );
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
        USHORT nTextStyle = TEXT_DRAW_MNEMONIC;
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
    //= HangulHanjaConversionDialog
    //=========================================================================
    //-------------------------------------------------------------------------
    HangulHanjaConversionDialog::HangulHanjaConversionDialog( Window* _pParent, HHC::ConversionDirection _ePrimaryDirection )
        :ModalDialog( _pParent, SVX_RES( RID_SVX_MDLG_HANGULHANJA ) )
        ,m_pPlayground( new SvxCommonLinguisticControl( this ) )
        ,m_aFind            ( m_pPlayground.get(), ResId( PB_FIND ) )
        ,m_aSuggestions     ( m_pPlayground.get(), ResId( LB_SUGGESTIONS ) )
        ,m_aFormat          ( m_pPlayground.get(), ResId( FT_FORMAT ) )
        ,m_aSimpleConversion( m_pPlayground.get(), ResId( RB_SIMPLE_CONVERSION ) )
        ,m_aHangulBracketed ( m_pPlayground.get(), ResId( RB_HANJA_HANGUL_BRACKETED ) )
        ,m_aHanjaBracketed  ( m_pPlayground.get(), ResId( RB_HANGUL_HANJA_BRACKETED ) )
        ,m_aConversion      ( m_pPlayground.get(), ResId( FT_CONVERSION ) )
        ,m_aHangulOnly      ( m_pPlayground.get(), ResId( CB_HANGUL_ONLY ) )
        ,m_aHanjaOnly       ( m_pPlayground.get(), ResId( CB_HANJA_ONLY ) )
        ,m_aReplaceByChar   ( m_pPlayground.get(), ResId( CB_REPLACE_BY_CHARACTER ) )
        ,m_pIgnoreNonPrimary( NULL )
        ,m_bDocumentMode( true )
    {
        // special creation of the 4 pseudo-ruby radio buttons
        String sSecondaryHangul( ResId( STR_HANGUL ) );
        String sSecondaryHanja( ResId( STR_HANJA ) );
        m_pHanjaAbove.reset( new RubyRadioButton( m_pPlayground.get(), ResId( RB_HANGUL_HANJA_ABOVE ), sSecondaryHanja, PseudoRubyText::eAbove ) );
        m_pHanjaBelow.reset( new RubyRadioButton( m_pPlayground.get(), ResId( RB_HANGUL_HANJA_BELOW ), sSecondaryHanja, PseudoRubyText::eBelow ) );
        m_pHangulAbove.reset( new RubyRadioButton( m_pPlayground.get(), ResId( RB_HANJA_HANGUL_ABOVE ), sSecondaryHangul, PseudoRubyText::eAbove ) );
        m_pHangulBelow.reset( new RubyRadioButton( m_pPlayground.get(), ResId( RB_HANJA_HANGUL_BELOW ), sSecondaryHangul, PseudoRubyText::eBelow ) );

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
            FixedText aBottomAnchor( m_pPlayground.get(), ResId( FT_RESIZE_ANCHOR ) );
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

        if ( HangulHanjaConversion::eHangulToHanja == _ePrimaryDirection )
        {
            m_aHanjaOnly.Enable( sal_False );
            m_pIgnoreNonPrimary = &m_aHangulOnly;
        }
        else
        {
            m_aHangulOnly.Enable( sal_False );
            m_pIgnoreNonPrimary = &m_aHanjaOnly;
        }
        m_pIgnoreNonPrimary->Check();

        // initial focus
        FocusSuggestion( );

        // initial control values
        m_aSimpleConversion.Check();

        FreeResource();
    }

    //-------------------------------------------------------------------------
    HangulHanjaConversionDialog::~HangulHanjaConversionDialog( )
    {
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::FillSuggestions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions )
    {
        m_aSuggestions.Clear();

        const ::rtl::OUString* pSuggestions = _rSuggestions.getConstArray();
        const ::rtl::OUString* pSuggestionsEnd = _rSuggestions.getConstArray() + _rSuggestions.getLength();
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
        m_aReplaceByChar.SetClickHdl( _rHdl );
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversionDialog, OnSuggestionSelected, void*, NOTINTERESTEDIN )
    {
        m_pPlayground->GetWordInputControl().SetText( m_aSuggestions.GetSelectEntry() );
        OnSuggestionModified( NULL );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversionDialog, OnSuggestionModified, void*, NOTINTERESTEDIN )
    {
        m_aFind.Enable( m_pPlayground->GetWordInputControl().GetSavedValue() != m_pPlayground->GetWordInputControl().GetText() );

        bool bSameLen = m_pPlayground->GetWordInputControl().GetText().Len() == m_pPlayground->GetCurrentText().Len();
        m_pPlayground->EnableButton( SvxCommonLinguisticControl::eChange, m_bDocumentMode && bSameLen );
        m_pPlayground->EnableButton( SvxCommonLinguisticControl::eChangeAll, m_bDocumentMode && bSameLen );

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversionDialog, OnClose, void*, NOTINTERESTEDIN )
    {
        Close();
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
        const Sequence< ::rtl::OUString >& _rSuggestions, bool _bOriginatesFromDocument )
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
    void HangulHanjaConversionDialog::SetByCharacter( sal_Bool _bByCharacter )
    {
        m_aReplaceByChar.Check( _bByCharacter );
    }

    //-------------------------------------------------------------------------
    sal_Bool HangulHanjaConversionDialog::GetByCharacter( ) const
    {
        return m_aReplaceByChar.IsChecked();
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::SetUseBothDirections( sal_Bool _bBoth ) const
    {
        DBG_ASSERT( m_pIgnoreNonPrimary, "HangulHanjaConversionDialog::SetUseBothDirections: where's the check box pointer?" );
        m_pIgnoreNonPrimary->Check( !_bBoth );
    }

    //-------------------------------------------------------------------------
    sal_Bool HangulHanjaConversionDialog::GetUseBothDirections( ) const
    {
        DBG_ASSERT( m_pIgnoreNonPrimary, "HangulHanjaConversionDialog::GetUseBothDirections: where's the check box pointer?" );
        return m_pIgnoreNonPrimary ? !m_pIgnoreNonPrimary->IsChecked( ) : sal_True;
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
            DBG_ERROR( "HangulHanjaConversionDialog::SetConversionFormat: unknown type!" );
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

        DBG_ERROR( "HangulHanjaConversionDialog::GetConversionFormat: no radio checked?" )
        return HHC::eSimpleConversion;
    }

//.............................................................................
}   // namespace svx
//.............................................................................
