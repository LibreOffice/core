/*************************************************************************
 *
 *  $RCSfile: hangulhanjadlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:48:28 $
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
#include "helpid.hrc"

#include <algorithm>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <vcl/controllayout.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#ifndef _SVTOOLS_LINGUPROPS_HXX_
#include <svtools/linguprops.hxx>
#endif
//#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
//#include <com/sun/star/frame/XStorable.hpp>
//#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONDICTIONARYTYPE_HPP_
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#ifndef _COM_SUN_STAR_LINGUISTIC2_CONVERSIONDIRECTION_HDL_
#include <com/sun/star/linguistic2/ConversionDirection.hdl>
#endif
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONOPTION_HDL_
#include <com/sun/star/i18n/TextConversionOption.hdl>
#endif

#include <comphelper/processfactory.hxx>
//#include <ucbhelper/content.hxx>


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
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::linguistic2;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::rtl;

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

        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eOptions,
                                        LINK( this, HangulHanjaConversionDialog, OnOption ) );
        m_pPlayground->GetButton( SvxCommonLinguisticControl::eOptions )->Show();
//      m_pPlayground->EnableButton( SvxCommonLinguisticControl::eOptions, true );

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

        m_pPlayground->GetButton(SvxCommonLinguisticControl::eClose     )->SetHelpId(HID_HANGULDLG_BUTTON_CLOSE    );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eIgnore    )->SetHelpId(HID_HANGULDLG_BUTTON_IGNORE   );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eIgnoreAll )->SetHelpId(HID_HANGULDLG_BUTTON_IGNOREALL);
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eChange    )->SetHelpId(HID_HANGULDLG_BUTTON_CHANGE   );
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eChangeAll )->SetHelpId(HID_HANGULDLG_BUTTON_CHANGEALL);
        m_pPlayground->GetButton(SvxCommonLinguisticControl::eOptions   )->SetHelpId(HID_HANGULDLG_BUTTON_OPTIONS  );
        m_pPlayground->GetWordInputControl().SetHelpId(HID_HANGULDLG_EDIT_NEWWORD);

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
    void HangulHanjaConversionDialog::SetOptionsHdl( const Link& _rHdl )
    {
        m_pPlayground->SetButtonHandler( SvxCommonLinguisticControl::eOptions, _rHdl );
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

    IMPL_LINK( HangulHanjaConversionDialog, OnOption, void*, NOTINTERESTEDIN )
    {
        HangulHanjaOptionsDialog        aOptDlg( this );
        aOptDlg.Execute();
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

    //-------------------------------------------------------------------------
    void HangulHanjaConversionDialog::EnableRubySupport( sal_Bool bVal )
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

    namespace
    {
        static Reference< XConversionDictionaryList >   xConvDictList;

        Reference< XConversionDictionaryList > GetXConversionDictionaryList()
        {
            Reference< XMultiServiceFactory >   xMgr( ::comphelper::getProcessServiceFactory() );
            if( xMgr.is() && !xConvDictList.is() )
            {
                xConvDictList = Reference< XConversionDictionaryList > ( xMgr->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.ConversionDictionaryList")) ),
                    UNO_QUERY );
            }

            return xConvDictList;
        }
    }


    HHDictList::HHDictList( void )
        :m_aList    ( 32 )
        ,m_nNew     ( 0 )
    {
    }

    HHDictList::~HHDictList()
    {
    }

    void HHDictList::Clear( void )
    {
        m_aList = Sequence< Reference< XConversionDictionary > >( 32 );
        m_nNew = 0;
    }

    void HHDictList::Add( Reference< XConversionDictionary > _xDict )
    {
        if( _xDict.is() )
        {
            if( m_nNew >= sal_uInt32( m_aList.getLength() ) )
                m_aList.realloc( m_aList.getLength() * 2 );

            *( m_aList.getArray() + m_nNew ) = _xDict;
            ++m_nNew;
        }
    }

    sal_uInt32 HHDictList::Count( void ) const
    {
        return m_nNew;
    }

    Reference< XConversionDictionary > HHDictList::Get( sal_uInt32 _nInd )
    {
        Reference< XConversionDictionary >  x;

        if( _nInd < m_nNew )
            x = *( m_aList.getArray() + _nInd );
        return x;
    }

    String HHDictList::GetName( sal_uInt32 _nInd )
    {
        String                              s;
        Reference< XConversionDictionary >  x = Get( _nInd );
        if( x.is() )
            s = x->getName();

        return s;
    }

    bool HHDictList::GetIsActive( sal_uInt32 _nInd )
    {
        bool                                b;
        Reference< XConversionDictionary >  x = Get( _nInd );
        if( x.is() )
            b = x->isActive();
        else
            b = false;

        return b;
    }

    void HangulHanjaOptionsDialog::Init( void )
    {
        Reference< XConversionDictionaryList >  xDicList( GetXConversionDictionaryList() );
        m_aDictList.Clear();
        if( xDicList.is() )
        {
            Reference< XNameContainer > xNameCont = xDicList->getDictionaryContainer();
            Reference< XNameAccess >    xNameAccess = Reference< XNameAccess >( xNameCont, UNO_QUERY );
            if( xNameAccess.is() )
            {
                Sequence< ::rtl::OUString >     aDictNames( xNameAccess->getElementNames() );

                const ::rtl::OUString*          pDic = aDictNames.getConstArray();
                sal_Int32                       nCount = aDictNames.getLength();

                sal_Int32                       i;
                for( i = 0 ; i < nCount ; ++i )
                {
                    Any                                 aAny( xNameAccess->getByName( pDic[ i ] ) );
                    Reference< XConversionDictionary >  xDic;
                    if( ( aAny >>= xDic ) && xDic.is() )
                        m_aDictList.Add( xDic );
                }
            }
        }

        m_aDictsLB.Clear();
        sal_uInt32      nNumOfDicts = m_aDictList.Count();
        for( sal_uInt32 n = 0 ; n < nNumOfDicts ; ++n )
            AddDict( m_aDictList.GetName( n ), m_aDictList.GetIsActive( n ) );
    }

    IMPL_LINK( HangulHanjaOptionsDialog, OkHdl, void*, NOTINTERESTEDIN )
    {
        sal_uInt32              nCnt = m_aDictList.Count();
        sal_uInt32              n = 0;
        sal_uInt32              nActiveDics = 0;
        Sequence< OUString >    aActiveDics;

        aActiveDics.realloc( nCnt );
        OUString*               pActActiveDic = aActiveDics.getArray();

        while( nCnt )
        {
            Reference< XConversionDictionary >  xDict = m_aDictList.Get( n );
            SvLBoxEntry*                        pEntry = m_aDictsLB.SvTreeListBox::GetEntry( n );

            DBG_ASSERT( xDict.is(), "-HangulHanjaOptionsDialog::OkHdl(): someone is evaporated..." );
            DBG_ASSERT( pEntry, "-HangulHanjaOptionsDialog::OkHdl(): no one there in list?" );

            bool    bActive = m_aDictsLB.GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED;
            xDict->setActive( bActive );
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

        aTmp <<= bool( m_aAutocloseCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_AUTO_CLOSE_DIALOG, aTmp );

        aTmp <<= bool( m_aShowrecentlyfirstCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST, aTmp );

        aTmp <<= bool( m_aAutoreplaceuniqueCB.IsChecked() );
        aLngCfg.SetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES, aTmp );

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK( HangulHanjaOptionsDialog, DictsLB_SelectHdl, void*, NOTINTERESTEDIN )
    {
        bool    bSel = m_aDictsLB.FirstSelected() != NULL;

        m_aEditPB.Enable( bSel );
        m_aDeletePB.Enable( bSel );

        return 0;
    }

    IMPL_LINK( HangulHanjaOptionsDialog, NewDictHdl, void*, NOTINTERESTEDIN )
    {
        String                      aName;
        HangulHanjaNewDictDialog    aNewDlg( this );
        aNewDlg.Execute();
        if( aNewDlg.GetName( aName ) )
        {
            Reference< XConversionDictionaryList >  xDicList( GetXConversionDictionaryList() );
            if( xDicList.is() )
            {
                try
                {
                    xDicList->addNewDictionary( aName, SvxCreateLocale( LANGUAGE_KOREAN ), ConversionDictionaryType::HANGUL_HANJA );
                    Init();         //build new list
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

    IMPL_LINK( HangulHanjaOptionsDialog, EditDictHdl, void*, NOTINTERESTEDIN )
    {
        SvLBoxEntry*    pEntry = m_aDictsLB.FirstSelected();
        DBG_ASSERT( pEntry, "+HangulHanjaEditDictDialog::EditDictHdl(): call of edit should not be possible with no selection!" );
        if( pEntry )
        {
            HangulHanjaEditDictDialog   aEdDlg( this, m_aDictList, m_aDictsLB.GetSelectEntryPos() );
            aEdDlg.Execute();
        }
        return 0L;
    }

    IMPL_LINK( HangulHanjaOptionsDialog, DeleteDictHdl, void*, NOTINTERESTEDIN )
    {
        SvLBoxEntry*    pEntry = m_aDictsLB.FirstSelected();
        if( pEntry )
        {
            Reference< XConversionDictionaryList >  xDicList( GetXConversionDictionaryList() );
            if( xDicList.is() )
            {
                Reference< XNameContainer >     xNameCont = xDicList->getDictionaryContainer();
                if( xNameCont.is() )
                {
                    try
                    {
                        xNameCont->removeByName( OUString( *((const String*)pEntry->GetUserData()) ) );
                        Init();         //build new list
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
        :ModalDialog            ( _pParent, SVX_RES( RID_SVX_MDLG_HANGULHANJA_OPT ) )
        ,m_aUserdefdictFT       ( this, ResId( FT_USERDEFDICT ) )
        ,m_aDictsLB             ( this, ResId( LB_DICTS ) )
        ,m_aOptionsFL           ( this, ResId( FL_OPTIONS ) )
        ,m_aIgnorepostCB        ( this, ResId( CB_IGNOREPOST ) )
        ,m_aAutocloseCB         ( this, ResId( CB_AUTOCLOSE ) )
        ,m_aShowrecentlyfirstCB ( this, ResId( CB_SHOWRECENTLYFIRST ) )
        ,m_aAutoreplaceuniqueCB ( this, ResId( CB_AUTOREPLACEUNIQUE ) )
        ,m_aNewPB               ( this, ResId( PB_HHO_NEW ) )
        ,m_aEditPB              ( this, ResId( PB_HHO_EDIT ) )
        ,m_aDeletePB            ( this, ResId( PB_HHO_DELETE ) )
        ,m_aOkPB                ( this, ResId( PB_HHO_OK ) )
        ,m_aCancelPB            ( this, ResId( PB_HHO_CANCEL ) )
        ,m_aHelpPB              ( this, ResId( PB_HHO_HELP ) )

        ,m_pCheckButtonData     ( NULL )
    {
        m_aDictsLB.SetWindowBits( WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
        m_aDictsLB.SetSelectionMode( SINGLE_SELECTION );
        m_aDictsLB.SetHighlightRange();
//      m_aDictsLB.SetHelpId( xxx );
        m_aDictsLB.SetSelectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );
        m_aDictsLB.SetDeselectHdl( LINK( this, HangulHanjaOptionsDialog, DictsLB_SelectHdl ) );

        m_aOkPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, OkHdl ) );
        m_aNewPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, NewDictHdl ) );
        m_aEditPB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, EditDictHdl ) );
        m_aDeletePB.SetClickHdl( LINK( this, HangulHanjaOptionsDialog, DeleteDictHdl ) );

        FreeResource();

        SvtLinguConfig  aLngCfg;
        Any             aTmp;
        bool            bVal;
        aTmp = aLngCfg.GetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD );
        if( aTmp >>= bVal )
            m_aIgnorepostCB.Check( bVal );

        aTmp = aLngCfg.GetProperty( UPH_IS_AUTO_CLOSE_DIALOG );
        if( aTmp >>= bVal )
            m_aAutocloseCB.Check( bVal );

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
        SvLBoxEntry*    pEntry = m_aDictsLB.First();
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
        SvLBoxEntry*    pEntry = m_aDictsLB.SvTreeListBox::InsertEntry( _rName );
        m_aDictsLB.SetCheckButtonState( pEntry, _bChecked? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
        pEntry->SetUserData( new String( _rName ) );
    }

    //=========================================================================
    //= HangulHanjaNewDictDialog
    //=========================================================================
    //-------------------------------------------------------------------------

    IMPL_LINK( HangulHanjaNewDictDialog, OKHdl, void*, NOTINTERESTEDIN )
    {
        String  aName( m_aDictNameED.GetText() );

        aName.EraseTrailingChars();
        m_bEntered = aName.Len() > 0;
        if( m_bEntered )
            m_aDictNameED.SetText( aName );     // do this in case of trailing chars have been deleted

        EndDialog( RET_OK );
        return 0;
    }

    IMPL_LINK( HangulHanjaNewDictDialog, ModifyHdl, void*, NOTINTERESTEDIN )
    {
        String  aName( m_aDictNameED.GetText() );

        aName.EraseTrailingChars();
        m_aOkBtn.Enable( aName.Len() > 0 );

        return 0;
    }

    HangulHanjaNewDictDialog::HangulHanjaNewDictDialog( Window* _pParent )
        :ModalDialog    ( _pParent, SVX_RES( RID_SVX_MDLG_HANGULHANJA_NEWDICT ) )
        ,m_aNewDictFL   ( this, ResId( FL_NEWDICT ) )
        ,m_aDictNameFT  ( this, ResId( FT_DICTNAME ) )
        ,m_aDictNameED  ( this, ResId( ED_DICTNAME ) )
        ,m_aOkBtn       ( this, ResId( PB_NEWDICT_OK ) )
        ,m_aCancelBtn   ( this, ResId( PB_NEWDICT_ESC ) )
        ,m_aHelpBtn     ( this, ResId( PB_NEWDICT_HLP ) )

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
        {
            _rRetName = m_aDictNameED.GetText();
            _rRetName.EraseTrailingChars();
        }

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
        while( m_nAct < m_nNumOfEntries && !pRet )
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

        ( static_cast< HangulHanjaEditDictDialog* >( GetParent() ) )->UpdateScrollbar();
    }

    SuggestionEdit::SuggestionEdit( Window* pParent, const ResId& rResId,
        ScrollBar& _rScrollBar, SuggestionEdit* _pPrev, SuggestionEdit* _pNext  )
        :Edit( pParent, rResId )
        ,m_rScrollBar( _rScrollBar )
        ,m_pPrev( _pPrev )
        ,m_pNext( _pNext )
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
            USHORT                      nMod = rKeyCode.GetModifier();
            USHORT                      nCode = rKeyCode.GetCode();
            HangulHanjaEditDictDialog*  pDlg = static_cast< HangulHanjaEditDictDialog* >( GetParent() );
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
            if( _xDict.is() )
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


    IMPL_LINK( HangulHanjaEditDictDialog, ScrollHdl, void*, NOTINTERESTEDIN )
    {
        UpdateScrollbar();

        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, OriginalModifyHdl, void*, NOTINTERESTEDIN )
    {
        String  aOrg( m_aOriginalLB.GetText() );
        aOrg.EraseTrailingChars();
        if( aOrg.Len() == 0 )
        {
            m_aOriginal.Erase();
            CheckNewState();

            m_aDeletePB.Enable( false );
        }
        else
        {
            m_aOriginal = aOrg;
            Sequence< OUString >    aEntries;
            bool                    bFound = GetConversions( m_rDictList.Get( m_nCurrentDict ), aOrg, aEntries );
            m_aDeletePB.Enable( bFound );
            if( bFound )
            {
                UpdateSuggestions( aEntries );
                m_bNew = false;
            }
            else if( !m_bNew )
            {
                m_bNew = m_pSuggestions && m_pSuggestions->GetCount();
                if( m_bNew )
                    m_aNewPB.Enable( m_bNew );
            }
        }

        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, OriginalFocusLostHdl, void*, NOTINTERESTEDIN )
    {
        m_aOriginal = m_aOriginalLB.GetText();
        m_aOriginal.EraseTrailingChars();
        CheckNewState();

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

    IMPL_LINK( HangulHanjaEditDictDialog, EditFocusLostHdl1, Edit*, pEdit )
    {
        EditFocusLost( pEdit, 0 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditFocusLostHdl2, Edit*, pEdit )
    {
        EditFocusLost( pEdit, 1 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditFocusLostHdl3, Edit*, pEdit )
    {
        EditFocusLost( pEdit, 2 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, EditFocusLostHdl4, Edit*, pEdit )
    {
        EditFocusLost( pEdit, 3 );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, BookLBSelectHdl, void*, NOTINTERESTEDIN )
    {
        Init( m_aBookLB.GetSelectEntryPos() );
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, OriginalLBSelectHdl, void*, NOTINTERESTEDIN )
    {
        m_aOriginal = m_aOriginalLB.GetText();
        UpdateSuggestions();
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, NewPBPushHdl, void*, NOTINTERESTEDIN )
    {
        DBG_ASSERT( m_pSuggestions, "-HangulHanjaEditDictDialog::NewPBPushHdl(): no suggestions... search in hell..." );
        Reference< XConversionDictionary >  xDict = m_rDictList.Get( m_nCurrentDict );
        if( xDict.is() )
        {
            OUString                aLeft( m_aOriginal );
            const String*           pRight = m_pSuggestions->First();
            bool                    bAdded = false;
            while( pRight )
            {
                //addEntry( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText )
                //throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException) = 0;
                try
                {
                    xDict->addEntry( aLeft, *pRight );
                    bAdded = true;
                }
                catch( const IllegalArgumentException& )
                {
                }
                catch( const ElementExistException& )
                {
                }

                pRight = m_pSuggestions->Next();
            }

            if( bAdded )
            {
//              m_aOriginalLB.InsertEntry( m_aOriginal );   // append "history"
                m_aOriginal.Erase();
                m_pSuggestions->Clear();
                Init( m_nCurrentDict );
            }
        }
        else
        {
            DBG_WARNING( "+HangulHanjaEditDictDialog::NewPBPushHdl(): dictionary faded away..." );
        }
        return 0;
    }

    IMPL_LINK( HangulHanjaEditDictDialog, DeletePBPushHdl, void*, NOTINTERESTEDIN )
    {
        Reference< XConversionDictionary >  xDict = m_rDictList.Get( m_nCurrentDict );
        if( xDict.is() )
        {
            String                  a_Org( m_aOriginalLB.GetText() );
            a_Org.EraseTrailingChars();
            OUString                aOrg( a_Org );
            Sequence< OUString >    aEntries;

            try
            {
                aEntries = xDict->getConversions(   aOrg,
                                                    0,
                                                    aOrg.getLength(),
                                                    ConversionDirection_FROM_LEFT,
                                                    ::com::sun::star::i18n::TextConversionOption::NONE );
            }
            catch( const IllegalArgumentException& )
            {
            }


            sal_uInt32  n = aEntries.getLength();
            OUString*   pEntry = aEntries.getArray();
            while( n )
            {
                try
                {
                    xDict->removeEntry( aOrg, *pEntry );
//                  m_aOriginalLB.InsertEntry( m_aOriginal );   // append "history"
                    m_aOriginal.Erase();
                }
                catch( const NoSuchElementException& )
                {   // can not be...
                }

                ++pEntry;
                --n;
            }

            Init( m_nCurrentDict );
        }
        return 0;
    }

    void HangulHanjaEditDictDialog::Init( sal_uInt32 _nSelDict )
    {
        if( m_pSuggestions )
            m_pSuggestions->Clear();

        if( m_nCurrentDict != _nSelDict )
        {
            m_nCurrentDict = _nSelDict;
            m_aOriginal.Erase();
        }

        UpdateOriginalLB();

        m_aOriginalLB.SetText( m_aOriginal.Len()? m_aOriginal : m_aEditHintText, Selection( 0, SELECTION_MAX ) );
        m_aOriginalLB.GrabFocus();

        m_aScrollSB.SetThumbPos( 0 );

        UpdateScrollbar();

        CheckNewState();
    }

    void HangulHanjaEditDictDialog::UpdateOriginalLB( void )
    {
        m_aOriginalLB.Clear();
        Reference< XConversionDictionary >  xDict = m_rDictList.Get( m_nCurrentDict );
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

            UpdateSuggestions();
        }
        else
        {
            DBG_WARNING( "+HangulHanjaEditDictDialog::UpdateOriginalLB(): dictionary faded away..." );
        }
    }

    void HangulHanjaEditDictDialog::UpdateSuggestions( void )
    {
        if( m_pSuggestions )
            m_pSuggestions->Clear();

        m_aScrollSB.SetThumbPos( 0 );
        UpdateScrollbar();              // will force edits to be filled new
    }

    void HangulHanjaEditDictDialog::UpdateSuggestions( const Sequence< OUString >& _rSuggestions )
    {
        if( m_pSuggestions )
            m_pSuggestions->Clear();

        sal_uInt32          nCnt = _rSuggestions.getLength();
        sal_uInt32          n = 0;
        if( nCnt )
        {
            if( !m_pSuggestions )
                m_pSuggestions = new SuggestionList( MAXNUM_SUGGESTIONS );

            const OUString*     pSugg = _rSuggestions.getConstArray();
            while( nCnt )
            {
                m_pSuggestions->Set( pSugg[ n ], sal_uInt16( n ) );
                ++n;
                --nCnt;
            }
        }

        m_aScrollSB.SetThumbPos( 0 );
        UpdateScrollbar();              // will force edits to be filled new
    }

    void HangulHanjaEditDictDialog::Leave( void )
    {
    }

    void HangulHanjaEditDictDialog::CheckNewState( void )
    {
        String  aOrg( m_aOriginalLB.GetText() );
        aOrg.EraseTrailingChars();
        m_bNew = aOrg.Len() && aOrg != m_aEditHintText && m_pSuggestions && m_pSuggestions->GetCount() > 0;
        m_aNewPB.Enable( m_bNew );
    }

    void HangulHanjaEditDictDialog::SetSuggestion( const String& _rText, sal_uInt16 _nEntryNum )
    {
        if( !m_pSuggestions )
            m_pSuggestions = new SuggestionList( MAXNUM_SUGGESTIONS );

        m_pSuggestions->Set( _rText, _nEntryNum );
    }

    void HangulHanjaEditDictDialog::ResetSuggestion( sal_uInt16 _nEntryNum )
    {
        if( !m_pSuggestions )
            m_pSuggestions = new SuggestionList( MAXNUM_SUGGESTIONS );

        m_pSuggestions->Reset( _nEntryNum );
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
        if( _pEdit->GetText().Len() == 0 )
        {   // empty string -> clear suggestion and check "new"-button state
            m_pSuggestions->Reset( m_nTopPos + _nEntryOffset );
            CheckNewState();
        }
        else if( !m_bNew )
        {
            String  aOrg( m_aOriginalLB.GetText() );
            m_bNew = aOrg.Len() && aOrg != m_aEditHintText;
            if( m_bNew )
                m_aNewPB.Enable( m_bNew );
        }
    }

    void HangulHanjaEditDictDialog::EditFocusLost( Edit* _pEdit, sal_uInt8 _nEntryOffset )
    {
        String  aTxt( _pEdit->GetText() );
        if( aTxt.Len() == 0 )
            ResetSuggestion( m_nTopPos + _nEntryOffset );
        else
            SetSuggestion( aTxt, m_nTopPos + _nEntryOffset );

        CheckNewState();
    }

    HangulHanjaEditDictDialog::HangulHanjaEditDictDialog( Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict )
        :ModalDialog            ( _pParent, SVX_RES( RID_SVX_MDLG_HANGULHANJA_EDIT ) )
        ,m_eState               ( ES_NIL )
        ,m_rDictList            ( _rDictList )
        ,m_nCurrentDict         ( 0xFFFFFFFF )
        ,m_aEditHintText        ( ResId( STR_EDITHINT ) )
        ,m_pSuggestions         ( NULL )
        ,m_aBookFT              ( this, ResId( FT_BOOK ) )
        ,m_aBookLB              ( this, ResId( LB_BOOK ) )
        ,m_aOriginalFT          ( this, ResId( FT_ORIGINAL ) )
        ,m_aOriginalLB          ( this, ResId( LB_ORIGINAL ) )
        ,m_aReplacebycharCB     ( this, ResId( CB_REPLACEBYCHAR ) )
        ,m_aSuggestionsFT       ( this, ResId( FT_SUGGESTIONS ) )
        ,m_aEdit1               ( this, ResId( ED_1 ), m_aScrollSB, NULL, &m_aEdit2 )
        ,m_aEdit2               ( this, ResId( ED_2 ), m_aScrollSB, &m_aEdit1, &m_aEdit3 )
        ,m_aEdit3               ( this, ResId( ED_3 ), m_aScrollSB, &m_aEdit2, &m_aEdit4 )
        ,m_aEdit4               ( this, ResId( ED_4 ), m_aScrollSB, &m_aEdit3, NULL )
        ,m_aScrollSB            ( this, ResId( SB_SCROLL ) )
        ,m_aNewPB               ( this, ResId( PB_HHE_NEW ) )
        ,m_aDeletePB            ( this, ResId( PB_HHE_DELETE ) )
        ,m_aHelpPB              ( this, ResId( PB_HHE_HELP ) )
        ,m_aClosePB             ( this, ResId( PB_HHE_CLOSE ) )

        ,m_nTopPos              ( 0 )
        ,m_bModified            ( false )
        ,m_bNew                 ( false )
    {
        m_aOriginalLB.SetModifyHdl( LINK( this, HangulHanjaEditDictDialog, OriginalModifyHdl ) );
        m_aOriginalLB.SetLoseFocusHdl( LINK( this, HangulHanjaEditDictDialog, OriginalFocusLostHdl ) );
        m_aOriginalLB.SetSelectHdl( LINK( this, HangulHanjaEditDictDialog, OriginalLBSelectHdl ) );

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
        m_aEdit1.SetLoseFocusHdl( LINK( this, HangulHanjaEditDictDialog, EditFocusLostHdl1 ) );
        m_aEdit2.SetLoseFocusHdl( LINK( this, HangulHanjaEditDictDialog, EditFocusLostHdl2 ) );
        m_aEdit3.SetLoseFocusHdl( LINK( this, HangulHanjaEditDictDialog, EditFocusLostHdl3 ) );
        m_aEdit4.SetLoseFocusHdl( LINK( this, HangulHanjaEditDictDialog, EditFocusLostHdl4 ) );

        m_aBookLB.SetSelectHdl( LINK( this, HangulHanjaEditDictDialog, BookLBSelectHdl ) );
        sal_uInt32  nDictCnt = m_rDictList.Count();
        for( sal_uInt32 n = 0 ; n < nDictCnt ; ++n )
            m_aBookLB.InsertEntry( m_rDictList.GetName( n ) );
        m_aBookLB.SelectEntryPos( USHORT( _nSelDict ) );

        FreeResource();

        Init( _nSelDict );
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
