/*************************************************************************
 *
 *  $RCSfile: hangulhanjadlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 15:46:58 $
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
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX
#include "checklbx.hxx"
#endif
#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include "hangulhanja.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include "unolingu.hxx"
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XCONVERSIONDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
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
                    m_pPlayground;                  // order matters: before all other controls!

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
        void    SetOptionsHdl( const Link& _rHdl );

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

        // enables or disbales the checkboxes for ruby formatted replacements
        void            EnableRubySupport( sal_Bool bVal );

    private:
        DECL_LINK( OnClose, void* );
        DECL_LINK( OnOption, void* );
        DECL_LINK( OnSuggestionModified, void* );
        DECL_LINK( OnSuggestionSelected, void* );

        // fill the suggestion list box with suggestions for the actual input
        void FillSuggestions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions );
    };


    class HHDictList
    {
    private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > >
                            m_aList;
        sal_uInt32          m_nNew;     // position where the next dictionary will be inserted
    protected:
    public:
                            HHDictList( void );
        virtual             ~HHDictList();

        void                Clear( void );
        void                Add( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > _xDict );
        sal_uInt32          Count( void ) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary >
                            Get( sal_uInt32 _nInd );
        String              GetName( sal_uInt32 _nInd );
        bool                GetIsActive( sal_uInt32 _nInd );
    };


    class HangulHanjaOptionsDialog : public ModalDialog
    {
    private:
        FixedText           m_aUserdefdictFT;
        SvxCheckListBox     m_aDictsLB;
        FixedLine           m_aOptionsFL;
        CheckBox            m_aIgnorepostCB;
        CheckBox            m_aAutocloseCB;
        CheckBox            m_aShowrecentlyfirstCB;
        CheckBox            m_aAutoreplaceuniqueCB;
        PushButton          m_aNewPB;
        PushButton          m_aEditPB;
        PushButton          m_aDeletePB;
        OKButton            m_aOkPB;
        CancelButton        m_aCancelPB;
        HelpButton          m_aHelpPB;

        SvLBoxButtonData*   m_pCheckButtonData;

        HHDictList          m_aDictList;

        DECL_LINK( OkHdl, void* );
        DECL_LINK( OnNew, void* );
        DECL_LINK( OnEdit, void* );
        DECL_LINK( OnDelete, void* );
        DECL_LINK( DictsLB_SelectHdl, void* );
        DECL_LINK( NewDictHdl, void* );
        DECL_LINK( EditDictHdl, void* );
        DECL_LINK( DeleteDictHdl, void* );

        void                Init( void );       // reads settings from core and init controls
    public:
                            HangulHanjaOptionsDialog( Window* _pParent );
        virtual             ~HangulHanjaOptionsDialog();

        void                AddDict( const String& _rName, bool _bChecked );
    };


    class HangulHanjaNewDictDialog : public ModalDialog
    {
    private:
        FixedLine           m_aNewDictFL;
        FixedText           m_aDictNameFT;
        Edit                m_aDictNameED;
        OKButton            m_aOkBtn;
        CancelButton        m_aCancelBtn;
        HelpButton          m_aHelpBtn;

        bool                m_bEntered;

        DECL_LINK( OKHdl, void* );
        DECL_LINK( ModifyHdl, void* );
    protected:
    public:
                            HangulHanjaNewDictDialog( Window* _pParent );
        virtual             ~HangulHanjaNewDictDialog();

        bool                GetName( String& _rRetName ) const;
    };


    class SuggestionList;

    class SuggestionEdit : public Edit
    {
    private:
        SuggestionEdit*     m_pPrev;
        SuggestionEdit*     m_pNext;
        ScrollBar&          m_rScrollBar;

        bool                ShouldScroll( bool _bUp ) const;
        void                DoJump( bool _bUp );
    public:
                            SuggestionEdit( Window* pParent, const ResId& rResId,
                                    ScrollBar& _rScrollBar,
                                    SuggestionEdit* _pPrev, SuggestionEdit* _pNext );
        virtual             ~SuggestionEdit();
        virtual long        PreNotify( NotifyEvent& rNEvt );
    };


    class HangulHanjaEditDictDialog : public ModalDialog
    {
    private:
        enum EditState
        {
            ES_NIL,                 // neither original nor suggestion(s) entered
            ES_NEW_ORG_EMPTY,       // a new original was entered, no suggestion(s)
            ES_NEW_SUGG,            // no original was entered but at least one suggestion
            ES_NEW_ORG_SUGG,        // a new original and at least one suggestion was entered
            ES_EXIST_ORG,           // while typing, an existing entry matched
            ES_EXIST_ORG_MOD        // an existing entry has been modified
        };
        EditState       m_eState;
        const String    m_aEditHintText;
        HHDictList&     m_rDictList;
        sal_uInt32      m_nCurrentDict;

        String          m_aOriginal;
        SuggestionList* m_pSuggestions;

        FixedText       m_aBookFT;
        ListBox         m_aBookLB;
        FixedText       m_aOriginalFT;
        ComboBox        m_aOriginalLB;
        CheckBox        m_aReplacebycharCB;
        FixedText       m_aSuggestionsFT;
        SuggestionEdit  m_aEdit1;
        SuggestionEdit  m_aEdit2;
        SuggestionEdit  m_aEdit3;
        SuggestionEdit  m_aEdit4;
        ScrollBar       m_aScrollSB;
        PushButton      m_aNewPB;
        PushButton      m_aDeletePB;
        HelpButton      m_aHelpPB;
        CancelButton    m_aClosePB;

        bool            m_bModified;
        bool            m_bNew;
        sal_uInt16      m_nTopPos;

        DECL_LINK( OriginalModifyHdl, void* );
        DECL_LINK( OriginalFocusLostHdl, void* );
        DECL_LINK( ScrollHdl, void* );
        DECL_LINK( EditModifyHdl1, Edit* );
        DECL_LINK( EditModifyHdl2, Edit* );
        DECL_LINK( EditModifyHdl3, Edit* );
        DECL_LINK( EditModifyHdl4, Edit* );
        DECL_LINK( EditFocusLostHdl1, Edit* );
        DECL_LINK( EditFocusLostHdl2, Edit* );
        DECL_LINK( EditFocusLostHdl3, Edit* );
        DECL_LINK( EditFocusLostHdl4, Edit* );

        DECL_LINK( BookLBSelectHdl, void* );
        DECL_LINK( OriginalLBSelectHdl, void* );
        DECL_LINK( NewPBPushHdl, void* );
        DECL_LINK( DeletePBPushHdl, void* );

        void            Init( sal_uInt32 _nSelDict );
        void            UpdateOriginalLB( void );
        void            UpdateSuggestions( void );
        void            UpdateSuggestions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rSuggestions );
        void            Leave( void );
        void            CheckNewState( void );
                        // checks if enough data is entered to enable "New" Button and set Button state accordingly
        void            SetSuggestion( const String& _rText, sal_uInt16 _nEntryNum );
        void            ResetSuggestion( sal_uInt16 _nEntryNum );
        void            SetEditText( Edit& _rEdit, sal_uInt16 _nEntryNum );
        void            EditModify( Edit* _pEdit, sal_uInt8 _nEntryOffset );
        void            EditFocusLost( Edit* _pEdit, sal_uInt8 _nEntryOffset );
    public:
                        HangulHanjaEditDictDialog( Window* _pParent, HHDictList& _rDictList, sal_uInt32 _nSelDict );
                        ~HangulHanjaEditDictDialog();

        void            UpdateScrollbar( void );
    };

//.............................................................................
}   // namespace svx
//.............................................................................

#endif // SVX_HANGUL_HANJA_HXX
