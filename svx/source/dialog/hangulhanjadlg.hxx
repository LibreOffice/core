/*************************************************************************
 *
 *  $RCSfile: hangulhanjadlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:42:06 $
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
#define SVX_HANGUL_HANJA_DLG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include "hangulhanja.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <memory>   // for auto_ptr

class SvxCommonLinguisticControl;
//.............................................................................
namespace svx
{
//.............................................................................

    //=========================================================================
    //= HangulHanjaConversionDialog
    //=========================================================================

    class HangulHanjaConversionDialog : public  ModalDialog
    {
    private:
        ::std::auto_ptr< SvxCommonLinguisticControl >
                    m_pPlayground;                  // oder matters: before all other controls!

        PushButton      m_aFind;
        ListBox         m_aSuggestions;
        FixedText       m_aFormat;
        RadioButton     m_aSimpleConversion;
        RadioButton     m_aHangulBracketed;
        RadioButton     m_aHanjaBracketed;
        ::std::auto_ptr< RadioButton >  m_pHanjaAbove;
        ::std::auto_ptr< RadioButton >  m_pHanjaBelow;
        ::std::auto_ptr< RadioButton >  m_pHangulAbove;
        ::std::auto_ptr< RadioButton >  m_pHangulBelow;
        FixedText       m_aConversion;
        CheckBox        m_aHangulOnly;
        CheckBox        m_aHanjaOnly;
        CheckBox        m_aReplaceByChar;

        CheckBox*       m_pIgnoreNonPrimary;
        bool            m_bDocumentMode;
                            // are we working for a document? This is normally true, but in case
                            // the user uses the "find" functionality, we switch to working
                            // with what the user entered, which then does not have any relation to
                            // the document anymore. Some functionality must be disabled then

    public:
        HangulHanjaConversionDialog(
                Window* _pParent,
                HangulHanjaConversion::ConversionDirection _ePrimaryDirection );
        ~HangulHanjaConversionDialog( );

    public:
        void    SetIgnoreHdl( const Link& _rHdl );
        void    SetIgnoreAllHdl( const Link& _rHdl );
        void    SetChangeHdl( const Link& _rHdl );
        void    SetChangeAllHdl( const Link& _rHdl );

        void    SetClickByCharacterHdl( const Link& _rHdl );
        void    SetConversionFormatChangedHdl( const Link& _rHdl );
        void    SetFindHdl( const Link& _rHdl );

        String  GetCurrentString( ) const;
        void    SetCurrentString(
                    const String& _rNewString,
                    const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions,
                    bool _bOriginatesFromDocument = true
                );

        void    FocusSuggestion( );

        // retrieves the current suggestion
        String  GetCurrentSuggestion( ) const;

        void        SetConversionFormat( HangulHanjaConversion::ConversionFormat _eType );
        HangulHanjaConversion::ConversionFormat    GetConversionFormat( ) const;

        void            SetByCharacter( sal_Bool _bByCharacter );
        sal_Bool        GetByCharacter( ) const;

        // should text which does not fit the primary conversion direction be ignored
        void            SetUseBothDirections( sal_Bool _bBoth ) const;
        sal_Bool        GetUseBothDirections( ) const;

    private:
        DECL_LINK( OnClose, void* );
        DECL_LINK( OnSuggestionModified, void* );
        DECL_LINK( OnSuggestionSelected, void* );

        // fill the suggestion list box with suggestions for the actual input
        void FillSuggestions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions );
    };

//.............................................................................
}   // namespace svx
//.............................................................................

#endif // SVX_HANGUL_HANJA_HXX
