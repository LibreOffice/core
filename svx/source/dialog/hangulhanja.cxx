/*************************************************************************
 *
 *  $RCSfile: hangulhanja.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 17:41:42 $
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

#ifndef SVX_HANGUL_HANJA_CONVERSION_HXX
#include "hangulhanja.hxx"
#endif
#ifndef SVX_HANGUL_HANJA_DLG_HXX
#include "hangulhanjadlg.hxx"
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include <set>
#include <map>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_UNICODESCRIPT_HPP_
#include <com/sun/star/i18n/UnicodeScript.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XTEXTCONVERSION_HPP_
#include <com/sun/star/i18n/XTextConversion.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONTYPE_HPP_
#include <com/sun/star/i18n/TextConversionType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TEXTCONVERSIONOPTION_HPP_
#include <com/sun/star/i18n/TextConversionOption.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#define HHC HangulHanjaConversion

//.............................................................................
namespace svx
{
//.............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::i18n::TextConversionOption;
    using namespace ::com::sun::star::i18n::TextConversionType;
    using namespace ::com::sun::star::lang;
/*
    using HangulHanjaConversion::ReplacementAction;
    using HangulHanjaConversion::eExchange;
    using HangulHanjaConversion::eReplacementBracketed;
    using HangulHanjaConversion::eOriginalBracketed;
    using HangulHanjaConversion::eReplacementAbove;
    using HangulHanjaConversion::eOriginalAbove;
    using HangulHanjaConversion::eReplacementBelow;
    using HangulHanjaConversion::eOriginalBelow;

    using HangulHanjaConversion::eHangulToHanja;
    using HangulHanjaConversion::eHanjaToHangul;

    using HangulHanjaConversion::eSimpleConversion;
    using HangulHanjaConversion::eHangulBracketed;
    using HangulHanjaConversion::eHanjaBracketed;
    using HangulHanjaConversion::eRubyHanjaAbove;
    using HangulHanjaConversion::eRubyHanjaBelow;
    using HangulHanjaConversion::eRubyHangulAbove;
    using HangulHanjaConversion::eRubyHangulBelow;

    using ::com::sun::star::i18n::TextConversionType::TO_HANJA;
    using ::com::sun::star::i18n::TextConversionType::TO_HANGUL;
    using ::com::sun::star::i18n::TextConversionOption::CHARACTER_BY_CHARACTER;
    using ::com::sun::star::i18n::TextConversionOption::NONE;
*/
    //=========================================================================
    //= HangulHanjaConversion_Impl
    //=========================================================================
    //using HangulHanjaConversion::ConversionFormat;

    class HangulHanjaConversion_Impl
    {
    private:
        typedef ::std::set< ::rtl::OUString, ::std::less< ::rtl::OUString > >                   StringBag;
        typedef ::std::map< ::rtl::OUString, ::rtl::OUString, ::std::less< ::rtl::OUString > >  StringMap;

    private:
        static  StringBag                       m_sIgnoreList;
                StringMap                       m_aChangeList;

        // general
        HangulHanjaConversionDialog*
                                m_pConversionDialog;    // the dialog to display for user interaction
        Window*                 m_pUIParent;            // the parent window for any UI we raise
        Reference< XMultiServiceFactory >
                                m_xORB;                 // the service factory to use
        Reference< XTextConversion >
                                m_xConverter;           // the text conversion service
        Locale                  m_aLocale;              // the locale we're working with

        HangulHanjaConversion*  m_pAntiImpl;            // our "anti-impl" instance

        // options
        sal_Bool                m_bByCharacter;                 // are we in "by character" mode currently?
        HHC::ConversionFormat       m_eConversionFormat;            // the current format for the conversion
        HHC::ConversionDirection        m_ePrimaryConversionDirection;  // the primary conversion direction
        HHC::ConversionDirection        m_eCurrentConversionDirection;  // the primary conversion direction

        // state
        ::rtl::OUString         m_sCurrentPortion;      // the text which we are currently working on
        sal_Int32               m_nCurrentStartIndex;   // the start index within m_sCurrentPortion of the current convertible portion
        sal_Int32               m_nCurrentEndIndex;     // the end index (excluding) within m_sCurrentPortion of the current convertible portion
        sal_Int32               m_nReplacementBaseIndex;// index which ReplaceUnit-calls need to be relative to
        Sequence< ::rtl::OUString >
                                m_aCurrentSuggestions;  // the suggestions for the current unit
                                                        // (means for the text [m_nCurrentStartIndex, m_nCurrentEndIndex) in m_sCurrentPortion)

#ifdef FS_HANGUL_HANJA
    public:
        ::rtl::OUString         m_sRememberPos;
#endif

    public:
        HangulHanjaConversion_Impl(
            Window* _pUIParent,
            const Reference< XMultiServiceFactory >& _rxORB,
            const Locale& _rLocale,
            HangulHanjaConversion* _pAntiImpl );

    public:
                void        DoDocumentConversion( );

        inline  sal_Bool    IsByCharacter( ) const { return m_bByCharacter; }

        inline  sal_Bool    IsValid() const { return m_xConverter.is(); }

    protected:
        void    createDialog();

        /** continue with the conversion, return <TRUE/> if and only if the complete conversion is done
            @param _bRepeatCurrentUnit
                if <TRUE/>, an implNextConvertible will be called initially to advance to the next convertible.
                if <FALSE/>, the method will initially work with the current convertible unit
        */
        sal_Bool ContinueConversion( bool _bRepeatCurrentUnit );

    private:
        DECL_LINK( OnIgnore, void* );
        DECL_LINK( OnIgnoreAll, void* );
        DECL_LINK( OnChange, void* );
        DECL_LINK( OnChangeAll, void* );
        DECL_LINK( OnByCharClicked, CheckBox* );
        DECL_LINK( OnConversionTypeChanged, void* );
        DECL_LINK( OnFind, void* );

        /** proceed, after the current convertible has been handled

            <p><b>Attention:</b>
                When returning from this method, the dialog may have been deleted!</p>

            @param _bRepeatCurrentUnit
                will be passed to the <member>ContinueConversion</member> call
        */
        void    implProceed( bool _bRepeatCurrentUnit );

        // change the current convertible, and do _not_ proceed
        void    implChange( const ::rtl::OUString& _rChangeInto );

        /** find the next convertible piece of text, with possibly advancing to the next portion

            @see HangulHanjaConversion::GetNextPortion
        */
        sal_Bool    implNextConvertible( bool _bRepeatUnit );

        /** find the next convertible unit within the current portion
            @param _bRepeatUnit
                if <TRUE/>, the search will start at the beginning of the current unit,
                if <FALSE/>, it will start at the end of the current unit
        */
        bool        implNextConvertibleUnit( const sal_Int32 _nStartAt );

        /** retrieves the next portion, with setting the index members properly
            @return
                <TRUE/> if and only if there is a next portion
        */
        bool        implRetrieveNextPortion( );

        /** fill in m_ePrimaryConversionDirection from m_sCurrentPortion
            @return
                <FALSE/> if and only if something went wrong
        */
        bool        implDeterminePrimaryDirection( );
    };

    //=========================================================================
    //= HangulHanjaConversion_Impl
    //=========================================================================
    HangulHanjaConversion_Impl::StringBag HangulHanjaConversion_Impl::m_sIgnoreList;
    //-------------------------------------------------------------------------
    HangulHanjaConversion_Impl::HangulHanjaConversion_Impl( Window* _pUIParent,
        const Reference< XMultiServiceFactory >& _rxORB, const Locale& _rLocale, HangulHanjaConversion* _pAntiImpl )
        :m_pUIParent( _pUIParent )
        ,m_pAntiImpl( _pAntiImpl )
        ,m_bByCharacter( sal_False )
        ,m_pConversionDialog( NULL )
        ,m_eConversionFormat( HHC::eSimpleConversion )
        ,m_ePrimaryConversionDirection( HHC::eHangulToHanja )
        ,m_eCurrentConversionDirection( HHC::eHangulToHanja )
        ,m_nCurrentStartIndex( 0 )
        ,m_nCurrentEndIndex( 0 )
        ,m_nReplacementBaseIndex( 0 )
        ,m_xORB( _rxORB )
        ,m_aLocale( _rLocale )
    {
        DBG_ASSERT( m_xORB.is(), "HangulHanjaConversion_Impl::HangulHanjaConversion_Impl: no ORB!" );
        // TODO: initialize m_bByCharacter and m_eConversionFormat from the configuration?

        if ( m_xORB.is() )
        {
            ::rtl::OUString sTextConversionService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.TextConversion" ) );
            m_xConverter = m_xConverter.query( m_xORB->createInstance( sTextConversionService ) );
            if ( !m_xConverter.is() )
                ShowServiceNotAvailableError( m_pUIParent, sTextConversionService, sal_True );
        }
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion_Impl::createDialog()
    {
        if ( !m_pConversionDialog )
        {
            m_pConversionDialog = new HangulHanjaConversionDialog( m_pUIParent, m_ePrimaryConversionDirection );

            m_pConversionDialog->SetByCharacter( m_bByCharacter );
            m_pConversionDialog->SetConversionFormat( m_eConversionFormat );

            // the handlers
            m_pConversionDialog->SetIgnoreHdl( LINK( this, HangulHanjaConversion_Impl, OnIgnore ) );
            m_pConversionDialog->SetIgnoreAllHdl( LINK( this, HangulHanjaConversion_Impl, OnIgnoreAll ) );
            m_pConversionDialog->SetChangeHdl( LINK( this, HangulHanjaConversion_Impl, OnChange ) );
            m_pConversionDialog->SetChangeAllHdl( LINK( this, HangulHanjaConversion_Impl, OnChangeAll ) );
            m_pConversionDialog->SetClickByCharacterHdl( LINK( this, HangulHanjaConversion_Impl, OnByCharClicked ) );
            m_pConversionDialog->SetConversionFormatChangedHdl( LINK( this, HangulHanjaConversion_Impl, OnConversionTypeChanged ) );
            m_pConversionDialog->SetFindHdl( LINK( this, HangulHanjaConversion_Impl, OnFind ) );
        }
    }

#ifdef FS_HANGUL_HANJA
    static bool lcl_isConvertible( const sal_Unicode _c )
    {
        return (    (   ( _c >= 'a' )
                    &&  ( _c <= 'z' )
                    )
                ||  (   ( _c >= 'A' )
                    &&  ( _c <= 'Z' )
                    )
                );
    }
#endif

    //-------------------------------------------------------------------------
    bool HangulHanjaConversion_Impl::implNextConvertibleUnit( const sal_Int32 _nStartAt )
    {
        m_aCurrentSuggestions.realloc( 0 );

#ifdef FS_HANGUL_HANJA
        // until we have a text conversion service, do some dummy implementations here
        // use the break iterator to iterate over all words
        ::rtl::OUString sBreakIteratorService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.BreakIterator" ) );
        Reference< XBreakIterator > xBreakIter( m_xORB->createInstance( ::rtl::OUString( sBreakIteratorService ) ), UNO_QUERY );

        sal_Int32 nStartAt( _nStartAt );
        Boundary aWordBoundary = xBreakIter->getWordBoundary( m_sCurrentPortion, nStartAt, m_aLocale, WordType::ANYWORD_IGNOREWHITESPACES, sal_True );
        if ( m_sCurrentPortion.getStr()[ aWordBoundary.startPos ] == ' ' )
        {   // HACK
            aWordBoundary = xBreakIter->nextWord( m_sCurrentPortion, aWordBoundary.startPos, m_aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
            nStartAt = aWordBoundary.startPos;
        }
        if ( IsByCharacter() )
        {
            if ( nStartAt < aWordBoundary.endPos )
                aWordBoundary.startPos = nStartAt;
            else
                aWordBoundary = xBreakIter->nextWord( m_sCurrentPortion, aWordBoundary.endPos, m_aLocale, WordType::ANYWORD_IGNOREWHITESPACES );

            aWordBoundary.endPos = aWordBoundary.startPos + 1;
        }
        else
        {
            while   (   ( aWordBoundary.startPos == aWordBoundary.endPos )          // empty word
                    &&  ( aWordBoundary.endPos < m_sCurrentPortion.getLength() )    // still characters left
                    )
                aWordBoundary = xBreakIter->nextWord( m_sCurrentPortion, aWordBoundary.endPos, m_aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
        }
        m_nCurrentStartIndex = aWordBoundary.startPos;
        m_nCurrentEndIndex = aWordBoundary.endPos;

        // for testing reasons, we add all rotations of the original word as suggestions
        m_aCurrentSuggestions.realloc( 0 );
        if ( m_nCurrentStartIndex < m_sCurrentPortion.getLength() )
        {
            ::rtl::OUString sCurrentUnit = m_sCurrentPortion.copy( m_nCurrentStartIndex, m_nCurrentEndIndex - m_nCurrentStartIndex );
            m_aCurrentSuggestions.realloc( sCurrentUnit.getLength() );
            ::rtl::OUString* pSuggestion = m_aCurrentSuggestions.getArray();
            String sSuggestion( sCurrentUnit );
            for ( sal_Int32 i = 0; i < sCurrentUnit.getLength(); ++i, ++pSuggestion )
            {
                *pSuggestion = sSuggestion = sSuggestion.Copy( 1 ).Append( sSuggestion.GetBuffer()[0] );
            }
        }
#else
        // ask the TextConversion service for the next convertible piece of text
        sal_Int32 nStartLookupAt = _nStartAt;

        // parameters for the converter
        sal_Int32 nLength = m_sCurrentPortion.getLength() - _nStartAt;

        sal_Int16 nConversionType = HHC::eHangulToHanja == m_ePrimaryConversionDirection ? TO_HANJA : TO_HANGUL;
        sal_Int32 nConversionOption = IsByCharacter() ? CHARACTER_BY_CHARACTER : NONE;

        sal_Bool bTryBothDirections = m_pConversionDialog ? m_pConversionDialog->GetUseBothDirections() : sal_True;

        // until we know better, assume that this very conversion attempt will end up with
        // the conversion direction which is our primary direction
        m_eCurrentConversionDirection = m_ePrimaryConversionDirection;

        sal_Bool bFoundAny = sal_True;
        try
        {
            TextConversionResult aResult = m_xConverter->getConversions(
                m_sCurrentPortion,
                _nStartAt,
                nLength,
                m_aLocale,
                nConversionType,
                nConversionOption
            );
            sal_Bool bFoundPrimary = aResult.Boundary.startPos < aResult.Boundary.endPos;
            bFoundAny = bFoundPrimary;

            if ( bTryBothDirections )
            {   // see if we find another convertible when assuming the other direction
                TextConversionResult aSecondResult = m_xConverter->getConversions(
                    m_sCurrentPortion,
                    _nStartAt,
                    nLength,
                    m_aLocale,
                    HHC::eHangulToHanja == m_ePrimaryConversionDirection ? TO_HANGUL : TO_HANJA,    // switched!
                    nConversionOption
                );
                if ( aSecondResult.Boundary.startPos < aSecondResult.Boundary.endPos )
                {   // we indeed found such a convertible

                    // in case the first attempt (with the original conversion direction)
                    // didn't find anything
                    if  (   !bFoundPrimary
                        // or if the second location is _before_ the first one
                        ||  ( aSecondResult.Boundary.startPos < aResult.Boundary.startPos )
                        )
                    {
                        // then use the second finding
                        aResult = aSecondResult;

                        // our current conversion direction changed now
                        m_eCurrentConversionDirection = ( HHC::eHangulToHanja == m_ePrimaryConversionDirection )
                            ? HHC::eHanjaToHangul : HHC::eHangulToHanja;
                        bFoundAny = sal_True;
                    }
                }
            }

            m_aCurrentSuggestions = aResult.Candidates;
            m_nCurrentStartIndex = aResult.Boundary.startPos;
            m_nCurrentEndIndex = aResult.Boundary.endPos;
        }
        catch( const Exception& e )
        {
            e;  // make compiler happy
            DBG_ERROR( "HangulHanjaConversion_Impl::implNextConvertibleUnit: caught an exception!" );
        }
#endif

        return  bFoundAny &&
                (m_nCurrentStartIndex < m_sCurrentPortion.getLength());
    }

    //-------------------------------------------------------------------------
    bool HangulHanjaConversion_Impl::implRetrieveNextPortion( )
    {
        m_sCurrentPortion = ::rtl::OUString();
        m_pAntiImpl->GetNextPortion( m_sCurrentPortion );
        m_nReplacementBaseIndex = 0;
        m_nCurrentStartIndex = m_nCurrentEndIndex = 0;

        return 0 != m_sCurrentPortion.getLength();
    }

    //-------------------------------------------------------------------------
    sal_Bool HangulHanjaConversion_Impl::implNextConvertible( bool _bRepeatUnit )
    {
        if ( _bRepeatUnit || ( m_nCurrentEndIndex < m_sCurrentPortion.getLength() ) )
        {
            if ( implNextConvertibleUnit(
                        _bRepeatUnit
                    ?   ( IsByCharacter() ? m_nCurrentStartIndex : m_nCurrentStartIndex )
                    :   m_nCurrentEndIndex
                ) )
                return sal_True;
        }

        // no convertible text in the current portion anymore
        // -> advance to the next portion
        do
        {
            // next portion
            if ( implRetrieveNextPortion( ) )
            {   // there is a next portion
                // -> find the next convertible unit in the current portion
                if ( implNextConvertibleUnit( 0 ) )
                    return sal_True;
            }
        }
        while ( m_sCurrentPortion.getLength() );

        // no more portions
        return sal_False;
    }

    //-------------------------------------------------------------------------
    sal_Bool HangulHanjaConversion_Impl::ContinueConversion( bool _bRepeatCurrentUnit )
    {
        sal_Bool bNeedUserInteraction = sal_False;  // when we leave here, do we need user interaction?
        sal_Bool bDocumentDone = sal_False;         // did we already check the whole document?

        while ( !bDocumentDone && !bNeedUserInteraction && implNextConvertible( _bRepeatCurrentUnit ) )
        {
            DBG_ASSERT( m_nCurrentStartIndex < m_sCurrentPortion.getLength(),
                "HangulHanjaConversion_Impl::ContinueConversion: invalid index into current portion!" );
            DBG_ASSERT( m_nCurrentStartIndex <= m_nCurrentEndIndex,
                "HangulHanjaConversion_Impl::ContinueConversion: invalid interval!" );

            ::rtl::OUString sCurrentUnit = m_sCurrentPortion.copy( m_nCurrentStartIndex, m_nCurrentEndIndex - m_nCurrentStartIndex );

            // do we need to ignore it?
            sal_Bool bAlwaysIgnoreThis = m_sIgnoreList.end() != m_sIgnoreList.find( sCurrentUnit );

            // do we need to change it?
            StringMap::const_iterator aChangeListPos = m_aChangeList.find( sCurrentUnit );
            sal_Bool bAlwaysChangeThis = m_aChangeList.end() != aChangeListPos;

            if ( bAlwaysChangeThis )
            {
                implChange( aChangeListPos->second );
            }
            else if ( !bAlwaysIgnoreThis )
            {
                // here we need to ask the user for what to do with the text
                // for this, allow derivees to highlight the current text unit in a possible document view
                m_pAntiImpl->HandleNewUnit( m_nCurrentStartIndex - m_nReplacementBaseIndex, m_nCurrentEndIndex - m_nReplacementBaseIndex );

                if ( !m_pConversionDialog )
                {
                    // the dialog does not yet exists (it's our first encounter)
                    createDialog();
                    // initially fill the dialog
                    m_pConversionDialog->SetCurrentString( sCurrentUnit, m_aCurrentSuggestions );
                    // execute it. It will automatically advance
                    m_pConversionDialog->Execute();
                    // we're done
                    DELETEZ( m_pConversionDialog );

                    bDocumentDone = sal_True;
                }
                else
                {
                    m_pConversionDialog->SetCurrentString( sCurrentUnit, m_aCurrentSuggestions );

                    // do not look for the next convertible: We have to wait for the user to interactivly
                    // decide what happens with the current convertible
                    bNeedUserInteraction = sal_True;
                }
            }
        }

        /*
        if ( bDocumentDone )
            return sal_True;            // we explicitly know that the complete document is done
        else if ( bNeedUserInteraction )
            return sal_False;           // the doc is not done, we found a convertible, but need the user to decide
        else
            return sal_True;            // we did not find a next convertible, so the document is implicitly done
        */

        return  bDocumentDone || !bNeedUserInteraction;
    }

    //-------------------------------------------------------------------------
    bool HangulHanjaConversion_Impl::implDeterminePrimaryDirection( )
    {
        m_ePrimaryConversionDirection = HHC::eHangulToHanja;    // default

        bool bSuccess = false;
        try
        {
            // get the break iterator service
            ::rtl::OUString sBreakIteratorService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.BreakIterator" ) );
            Reference< XInterface > xBI( m_xORB->createInstance( ::rtl::OUString( sBreakIteratorService ) ) );
            Reference< XBreakIterator > xBreakIter( xBI, UNO_QUERY );
            if ( !xBreakIter.is() )
            {
                ShowServiceNotAvailableError( m_pUIParent, sBreakIteratorService, sal_True );
            }
            else
            {
                sal_Int32 nNextAsianScript = xBreakIter->beginOfScript( m_sCurrentPortion, m_nCurrentStartIndex, ScriptType::ASIAN );
                if ( -1 == nNextAsianScript )
                    nNextAsianScript = xBreakIter->nextScript( m_sCurrentPortion, m_nCurrentStartIndex, ScriptType::ASIAN );
                if ( ( nNextAsianScript >= m_nCurrentStartIndex ) && ( nNextAsianScript < m_sCurrentPortion.getLength() ) )
                {   // found asian text

                    // determine if it's Hangul
                    CharClass aCharClassificaton( m_xORB, m_aLocale );
                    sal_Int16 nScript = aCharClassificaton.getScript( m_sCurrentPortion, nNextAsianScript );
                    if  (   ( UnicodeScript_kHangulJamo == nScript )
                        ||  ( UnicodeScript_kHangulCompatibilityJamo == nScript )
                        ||  ( UnicodeScript_kHangulSyllable == nScript )
                        )
                    {
                        m_ePrimaryConversionDirection = HHC::eHangulToHanja;
                    }
                    else
                    {
                        m_ePrimaryConversionDirection = HHC::eHanjaToHangul;
                    }

                    bSuccess = true;
                }
            }
        }
        catch( const Exception& e )
        {
            e;  // make compiler happy
            DBG_ERROR( "HangulHanjaConversion_Impl::implDeterminePrimaryDirection: caught an exception!" );
        }
        return bSuccess;
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion_Impl::DoDocumentConversion( )
    {
        // clear the change-all list - it's to be re-initialized for every single document
        {
            StringMap aEmpty;
            m_aChangeList.swap( aEmpty );
        }

        // first of all, we need to guess the direction of our conversion - it is determined by the first
        // hangul or hanja character in the first text
        if ( !implRetrieveNextPortion( ) )
        {
            DBG_ERROR( "HangulHanjaConversion_Impl::DoDocumentConversion: why did you call me if you do have nothing to convert?" );
            // nothing to do
            return;
        }
        if ( !implDeterminePrimaryDirection( ) )
            // something went wrong, has already been asserted
            return;

#ifdef DBG_UTIL
        sal_Bool bCompletelyDone =
#endif
        ContinueConversion( sal_False );
        DBG_ASSERT( bCompletelyDone, "HangulHanjaConversion_Impl::DoDocumentConversion: ContinueConversion should have returned true here!" );
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion_Impl::implProceed( bool _bRepeatCurrentUnit )
    {
        if ( ContinueConversion( _bRepeatCurrentUnit ) )
        {   // we're done with the whole document
            DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::implProceed: we should not reach this here without dialog!" );
            if ( m_pConversionDialog )
                m_pConversionDialog->EndDialog( RET_OK );
        }
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion_Impl::implChange( const ::rtl::OUString& _rChangeInto )
    {
        // translate the conversion format into a replacement action
        // this translation depends on whether we have a Hangul original, or a Hanja original

        // is the original we're about to change in Hangul?
        sal_Bool bOriginalIsHangul = HHC::eHangulToHanja == m_eCurrentConversionDirection;

        HHC::ReplacementAction eAction( HHC::eExchange );
        switch ( m_eConversionFormat )
        {
            case HHC::eSimpleConversion: eAction = HHC::eExchange; break;
            case HHC::eHangulBracketed:  eAction = bOriginalIsHangul ? HHC::eOriginalBracketed : HHC::eReplacementBracketed; break;
            case HHC::eHanjaBracketed:   eAction = bOriginalIsHangul ? HHC::eReplacementBracketed : HHC::eOriginalBracketed; break;
            case HHC::eRubyHanjaAbove:   eAction = bOriginalIsHangul ? HHC::eReplacementAbove : HHC::eOriginalAbove; break;
            case HHC::eRubyHanjaBelow:   eAction = bOriginalIsHangul ? HHC::eReplacementBelow : HHC::eOriginalBelow; break;
            case HHC::eRubyHangulAbove:  eAction = bOriginalIsHangul ? HHC::eOriginalAbove : HHC::eReplacementAbove; break;
            case HHC::eRubyHangulBelow:  eAction = bOriginalIsHangul ? HHC::eOriginalBelow : HHC::eReplacementBelow; break;
            default:
                DBG_ERROR( "HangulHanjaConversion_Impl::implChange: invalid/unexpected conversion format!" );
        }

        // the proper indicies (the wrapper implementation needs indicies relative to the
        // previous replacement)
        DBG_ASSERT( ( m_nReplacementBaseIndex <= m_nCurrentStartIndex ) && ( m_nReplacementBaseIndex <= m_nCurrentEndIndex ),
            "HangulHanjaConversion_Impl::implChange: invalid replacement base!" );

        sal_Int32 nStartIndex = m_nCurrentStartIndex - m_nReplacementBaseIndex;
        sal_Int32 nEndIndex = m_nCurrentEndIndex - m_nReplacementBaseIndex;

        // do the replacement
        m_pAntiImpl->ReplaceUnit( nStartIndex, nEndIndex, _rChangeInto, eAction );

        // adjust the replacement base
        m_nReplacementBaseIndex = m_nCurrentEndIndex;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnIgnore, void*, NOTINTERESTEDIN )
    {
        // simply ignore, and proceed
        implProceed( sal_False );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnIgnoreAll, void*, NOTINTERESTEDIN )
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnIgnoreAll: no dialog! How this?" );

        if ( m_pConversionDialog )
        {
            String sCurrentUnit = m_pConversionDialog->GetCurrentString();
            DBG_ASSERT( m_sIgnoreList.end() == m_sIgnoreList.find( sCurrentUnit ),
                "HangulHanjaConversion_Impl, OnIgnoreAll: shouldn't this have been ignored before" );

            // put into the "ignore all" list
            m_sIgnoreList.insert( sCurrentUnit );

            // and proceed
            implProceed( sal_False );
        }

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnChange, void*, NOTINTERESTEDIN )
    {
        // change
        implChange( m_pConversionDialog->GetCurrentSuggestion( ) );
        // and proceed
        implProceed( sal_False );

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnChangeAll, void*, NOTINTERESTEDIN )
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnChangeAll: no dialog! How this?" );
        if ( m_pConversionDialog )
        {
            ::rtl::OUString sCurrentUnit( m_pConversionDialog->GetCurrentString() );
            ::rtl::OUString sChangeInto( m_pConversionDialog->GetCurrentSuggestion( ) );

            // change the current occurence
            implChange( sChangeInto );

            // put into the "change all" list
            m_aChangeList.insert( StringMap::value_type( sCurrentUnit, sChangeInto ) );

            // and proceed
            implProceed( sal_False );
        }

        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnByCharClicked, CheckBox*, _pBox )
    {
        m_bByCharacter = _pBox->IsChecked();

        // continue conversion, without advancing to the next unit, but instead continuing with the current unit
        implProceed( m_bByCharacter ? sal_True : sal_False );
            // if we just switched to "by character", then we want to start over with the current word
            // if we just switched to "by word", then we don't want to start over with word which contains the current
            //    character, because this may be potentially dagerous in case a character within this word
            //    has already been replaced
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnConversionTypeChanged, void*, NOTINTERESTEDIN )
    {
        m_eConversionFormat = m_pConversionDialog->GetConversionFormat( );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( HangulHanjaConversion_Impl, OnFind, void*, NOTINTERESTEDIN )
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnFind: where did this come from?" );
        if ( m_pConversionDialog )
        {
            try
            {
                ::rtl::OUString sNewOriginal( m_pConversionDialog->GetCurrentSuggestion( ) );
                Sequence< ::rtl::OUString > aSuggestions;
#ifdef FS_HANGUL_HANJA
                // for testing reasons, we add all rotations of the original word as suggestions
                aSuggestions.realloc( sNewOriginal.getLength() );
                ::rtl::OUString* pSuggestion = aSuggestions.getArray();
                String sSuggestion( sNewOriginal );
                for ( sal_Int32 i = 0; i < sNewOriginal.getLength(); ++i, ++pSuggestion )
                {
                    *pSuggestion = sSuggestion = sSuggestion.Copy( 1 ).Append( sSuggestion.GetBuffer()[0] );
                }
#else
                DBG_ASSERT( m_xConverter.is(), "HangulHanjaConversion_Impl::OnFind: no converter!" );
                TextConversionResult aToHanja = m_xConverter->getConversions(
                    sNewOriginal,
                    0, sNewOriginal.getLength(),
                    m_aLocale,
                    TextConversionType::TO_HANJA,
                    TextConversionOption::NONE
                );
                TextConversionResult aToHangul = m_xConverter->getConversions(
                    sNewOriginal,
                    0, sNewOriginal.getLength(),
                    m_aLocale,
                    TextConversionType::TO_HANGUL,
                    TextConversionOption::NONE
                );

                bool bHaveToHanja = ( aToHanja.Boundary.startPos < aToHanja.Boundary.endPos );
                bool bHaveToHangul = ( aToHangul.Boundary.startPos < aToHangul.Boundary.endPos );

                TextConversionResult* pResult = NULL;
                if ( bHaveToHanja && bHaveToHangul )
                {   // it found convertibles in both directions -> use the first
                    if ( aToHangul.Boundary.startPos < aToHanja.Boundary.startPos )
                        pResult = &aToHangul;
                    else
                        pResult = &aToHanja;
                }
                else if ( bHaveToHanja )
                {   // only found toHanja
                    pResult = &aToHanja;
                }
                else
                {   // only found toHangul
                    pResult = &aToHangul;
                }
                if ( pResult )
                    aSuggestions = pResult->Candidates;
#endif

                m_pConversionDialog->SetCurrentString( sNewOriginal, aSuggestions, false );
                m_pConversionDialog->FocusSuggestion();
            }
            catch( const Exception& e )
            {
                e;  // make compiler happy
                DBG_ERROR( "HangulHanjaConversion_Impl::OnFind: caught an exception!" );
            }
        }
        return 0L;
    }

    //=========================================================================
    //= HangulHanjaConversion
    //=========================================================================
    //-------------------------------------------------------------------------
    HangulHanjaConversion::HangulHanjaConversion( Window* _pUIParent, const Reference< XMultiServiceFactory >& _rxORB, const Locale& _rLocale )
        :m_pImpl( new HangulHanjaConversion_Impl( _pUIParent, _rxORB, _rLocale, this ) )
    {
    }

    //-------------------------------------------------------------------------
    HangulHanjaConversion::~HangulHanjaConversion( )
    {
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion::HandleNewUnit( const sal_Int32 _nStartIndex, const sal_Int32 _nEndIndex )
    {
        // nothing to do, only derived classes need this.
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion::GetNextPortion( ::rtl::OUString& /* [out] */ _rNextPortion )
    {
#ifdef FS_HANGUL_HANJA
        #define TEST_TEXT "깔끔한 외형, 아름답게 디자인된 각 부분과 기능의 집약성으로, 디지털 카메라 1300은 쉽게 명함의 이미지를 촬영하여 PDA나 컴퓨터의 주소록으로 데이터를 변환할수 있다."
        static ::rtl::OUString sPortion(
            TEST_TEXT, sizeof( TEST_TEXT ) - 1,
            RTL_TEXTENCODING_UTF8
        );

        if ( m_pImpl->m_sRememberPos == sPortion )
            _rNextPortion = ::rtl::OUString();
        else
            m_pImpl->m_sRememberPos = _rNextPortion = sPortion;
#else
        DBG_ERROR( "HangulHanjaConversion::GetNextPortion: to be overridden!" );
#endif
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion::ReplaceUnit( const sal_Int32 _nUnitStart, const sal_Int32 _nUnitEnd,
            const ::rtl::OUString& _rReplaceWith, ReplacementAction _eAction )
    {
#ifndef FS_HANGUL_HANJA
        DBG_ERROR( "HangulHanjaConversion::ReplaceUnit: to be overridden!" );
#endif
    }

    //-------------------------------------------------------------------------
    void HangulHanjaConversion::ConvertDocument()
    {
#ifndef FS_HANGUL_HANJA
        if ( m_pImpl->IsValid() )
#endif
            m_pImpl->DoDocumentConversion( );
    }

//.............................................................................
}   // namespace svx
//.............................................................................
