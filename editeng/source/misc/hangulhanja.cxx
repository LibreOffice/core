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

#include <editeng/hangulhanja.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>

#include <set>
#include <map>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <com/sun/star/i18n/TextConversion.hpp>
#include <com/sun/star/i18n/XExtendedTextConversion.hpp>
#include <com/sun/star/i18n/TextConversionType.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <vcl/stdtext.hxx>
#include <unotools/charclass.hxx>

#include <editeng/edtdlg.hxx>
#include <editeng/editrids.hrc>
#include <editeng/unolingu.hxx>

#define HHC HangulHanjaConversion


namespace editeng
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::i18n::TextConversionOption;
    using namespace ::com::sun::star::i18n::TextConversionType;

    class HangulHanjaConversion_Impl
    {
    private:
        typedef ::std::set< OUString, ::std::less< OUString > >                   StringBag;
        typedef ::std::map< OUString, OUString, ::std::less< OUString > >  StringMap;

    private:
        StringBag               m_sIgnoreList;
        StringMap               m_aChangeList;
        static StringMap        m_aRecentlyUsedList;

        // general
        AbstractHangulHanjaConversionDialog*
                                m_pConversionDialog;    // the dialog to display for user interaction
        VclPtr<vcl::Window>     m_pUIParent;            // the parent window for any UI we raise
        Reference< XComponentContext >
                                m_xContext;             // the service factory to use
        Reference< XExtendedTextConversion >
                                m_xConverter;           // the text conversion service
        lang::Locale            m_aSourceLocale;        // the locale we're working with

        // additions for Chinese simplified / traditional conversion
        HHC::ConversionType     m_eConvType;        // conversion type (Hangul/Hanja, simplified/traditional Chinese,...)
        LanguageType            m_nSourceLang;      // just a 'copy' of m_aSourceLocale in order in order to
                                                    // save the applications from always converting to this
                                                    // type in their implementations
        LanguageType            m_nTargetLang;      // target language of new replacement text
        const vcl::Font*        m_pTargetFont;      // target font of new replacement text
        sal_Int32               m_nConvOptions;     // text conversion options (as used by 'getConversions')
        bool                    m_bIsInteractive;   // specifies if the conversion requires user interaction
                                                    // (and likely a specialised dialog) or if it is to run
                                                    // automatically without any user interaction.
                                                    // True for Hangul / Hanja conversion
                                                    // False for Chinese simlified / traditional conversion

        HangulHanjaConversion*  m_pAntiImpl;            // our "anti-impl" instance

        // options
        bool                        m_bByCharacter;                 // are we in "by character" mode currently?
        HHC::ConversionFormat       m_eConversionFormat;            // the current format for the conversion
        HHC::ConversionDirection    m_ePrimaryConversionDirection;  // the primary conversion direction
        HHC::ConversionDirection    m_eCurrentConversionDirection;  // the primary conversion direction

        //options from Hangul/Hanja Options dialog (also saved to configuration)
        bool                    m_bIgnorePostPositionalWord;
        bool                    m_bShowRecentlyUsedFirst;
        bool                    m_bAutoReplaceUnique;

        // state
        OUString                m_sCurrentPortion;      // the text which we are currently working on
        LanguageType            m_nCurrentPortionLang;  // language of m_sCurrentPortion found
        sal_Int32               m_nCurrentStartIndex;   // the start index within m_sCurrentPortion of the current convertible portion
        sal_Int32               m_nCurrentEndIndex;     // the end index (excluding) within m_sCurrentPortion of the current convertible portion
        sal_Int32               m_nReplacementBaseIndex;// index which ReplaceUnit-calls need to be relative to
        sal_Int32               m_nCurrentConversionOption;
        sal_Int16               m_nCurrentConversionType;
        Sequence< OUString >
                                m_aCurrentSuggestions;  // the suggestions for the current unit
                                                        // (means for the text [m_nCurrentStartIndex, m_nCurrentEndIndex) in m_sCurrentPortion)
        bool                    m_bTryBothDirections;   // specifies if other conversion directions should be tried when looking for convertible characters


    public:
        HangulHanjaConversion_Impl(
            vcl::Window* _pUIParent,
            const Reference< XComponentContext >& rxContext,
            const lang::Locale& _rSourceLocale,
            const lang::Locale& _rTargetLocale,
            const vcl::Font* _pTargetFont,
            sal_Int32 _nConvOptions,
            bool _bIsInteractive,
            HangulHanjaConversion* _pAntiImpl );

    public:
                void        DoDocumentConversion( );

        inline  bool        IsByCharacter( ) const { return m_bByCharacter; }

        inline  bool        IsValid() const { return m_xConverter.is(); }

        inline LanguageType GetSourceLang() const   { return m_nSourceLang; }
        inline LanguageType GetTargetLang() const   { return m_nTargetLang; }
        inline const vcl::Font * GetTargetFont() const   { return m_pTargetFont; }
        inline sal_Int32    GetConvOptions() const  { return m_nConvOptions; }
        inline bool         IsInteractive() const   { return m_bIsInteractive; }

    protected:
        void    createDialog();

        /** continue with the conversion, return <TRUE/> if and only if the complete conversion is done
            @param _bRepeatCurrentUnit
                if <TRUE/>, an implNextConvertible will be called initially to advance to the next convertible.
                if <FALSE/>, the method will initially work with the current convertible unit
        */
        bool ContinueConversion( bool _bRepeatCurrentUnit );

    private:
        DECL_LINK_TYPED( OnOptionsChanged, LinkParamNone*, void );
        DECL_LINK_TYPED( OnIgnore, Button*, void );
        DECL_LINK_TYPED( OnIgnoreAll, Button*, void );
        DECL_LINK_TYPED( OnChange, Button*, void );
        DECL_LINK_TYPED( OnChangeAll, Button*, void );
        DECL_LINK_TYPED( OnByCharClicked, CheckBox*, void );
        DECL_LINK_TYPED( OnConversionTypeChanged, Button*, void );
        DECL_LINK_TYPED( OnFind, Button*, void );

        /** proceed, after the current convertible has been handled

            <p><b>Attention:</b>
                When returning from this method, the dialog may have been deleted!</p>

            @param _bRepeatCurrentUnit
                will be passed to the <member>ContinueConversion</member> call
        */
        void    implProceed( bool _bRepeatCurrentUnit );

        // change the current convertible, and do _not_ proceed
        void    implChange( const OUString& _rChangeInto );

        /** find the next convertible piece of text, with possibly advancing to the next portion

            @see HangulHanjaConversion::GetNextPortion
        */
        bool    implNextConvertible( bool _bRepeatUnit );

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

        /** determine the ConversionDirection for m_sCurrentPortion
            @return
                <FALSE/> if and only if something went wrong
        */
        bool        implGetConversionDirectionForCurrentPortion( HHC::ConversionDirection& rDirection );

        /** member m_aCurrentSuggestions and m_nCurrentEndIndex are updated according to the other settings and current dictionaries

            if _bAllowSearchNextConvertibleText is true _nStartAt is used as starting point to search the next
            convertible text portion. This may result in changing of the member m_nCurrentStartIndex additionally.

            @return
                <TRUE/> if Suggestions were found
        */
        bool        implUpdateSuggestions( const bool _bAllowSearchNextConvertibleText=false, const sal_Int32 _nStartAt=-1 );

        /** reads the options from Hangul/Hanja Options dialog that are saved to configuration
        */
        void implReadOptionsFromConfiguration();

        /** get the string currently considered to be replaced or ignored
        */
        OUString GetCurrentUnit() const;

        /** read options from configuration, update suggestion list and dialog content
        */
        void implUpdateData();

        /** get the conversion direction dependent from m_eConvType and m_eCurrentConversionDirection
            in case of switching the direction is allowed this can be triggered with parameter bSwitchDirection
        */
        sal_Int16 implGetConversionType( bool bSwitchDirection=false ) const;
    };

    HangulHanjaConversion_Impl::StringMap HangulHanjaConversion_Impl::m_aRecentlyUsedList = HangulHanjaConversion_Impl::StringMap();

    HangulHanjaConversion_Impl::HangulHanjaConversion_Impl( vcl::Window* _pUIParent,
                                                            const Reference< XComponentContext >& rxContext,
                                                            const lang::Locale& _rSourceLocale,
                                                            const lang::Locale& _rTargetLocale,
                                                            const vcl::Font* _pTargetFont,
                                                            sal_Int32 _nOptions,
                                                            bool _bIsInteractive,
                                                            HangulHanjaConversion* _pAntiImpl )
        : m_pConversionDialog( NULL )
        , m_pUIParent( _pUIParent )
        , m_xContext( rxContext )
        , m_aSourceLocale( _rSourceLocale )
        , m_nSourceLang( LanguageTag::convertToLanguageType( _rSourceLocale ) )
        , m_nTargetLang( LanguageTag::convertToLanguageType( _rTargetLocale ) )
        , m_pTargetFont( _pTargetFont )
        , m_nConvOptions(_nOptions)
        , m_bIsInteractive( _bIsInteractive )
        , m_pAntiImpl( _pAntiImpl )
        , m_bByCharacter((_nOptions & CHARACTER_BY_CHARACTER) != 0)
        , m_eConversionFormat( HHC::eSimpleConversion)
        , m_ePrimaryConversionDirection( HHC::eHangulToHanja)    // used for eConvHangulHanja
        , m_eCurrentConversionDirection( HHC::eHangulToHanja)    // used for eConvHangulHanja
        , m_nCurrentPortionLang( LANGUAGE_NONE )
        , m_nCurrentStartIndex( 0 )
        , m_nCurrentEndIndex( 0 )
        , m_nReplacementBaseIndex( 0 )
        , m_nCurrentConversionOption( TextConversionOption::NONE )
        , m_nCurrentConversionType( -1 ) // not yet known
        , m_bTryBothDirections( true )
    {
        implReadOptionsFromConfiguration();

        DBG_ASSERT( m_xContext.is(), "HangulHanjaConversion_Impl::HangulHanjaConversion_Impl: no ORB!" );

        // determine conversion type
        if (m_nSourceLang == LANGUAGE_KOREAN && m_nTargetLang == LANGUAGE_KOREAN)
            m_eConvType = HHC::eConvHangulHanja;
        else if ( (m_nSourceLang == LANGUAGE_CHINESE_TRADITIONAL && m_nTargetLang == LANGUAGE_CHINESE_SIMPLIFIED)  ||
                 (m_nSourceLang == LANGUAGE_CHINESE_SIMPLIFIED  && m_nTargetLang == LANGUAGE_CHINESE_TRADITIONAL) )
            m_eConvType = HHC::eConvSimplifiedTraditional;
        else
        {
            m_eConvType = HHC::eConvHangulHanja;
            OSL_FAIL( "failed to determine conversion type from languages" );
        }

        m_xConverter = TextConversion::create( m_xContext );
    }

    void HangulHanjaConversion_Impl::createDialog()
    {
        DBG_ASSERT( m_bIsInteractive, "createDialog when the conversion should not be interactive?" );
        if ( m_bIsInteractive && !m_pConversionDialog )
        {
            EditAbstractDialogFactory* pFact = EditAbstractDialogFactory::Create();
            if(pFact)
            {
                m_pConversionDialog = pFact->CreateHangulHanjaConversionDialog(m_pUIParent, m_ePrimaryConversionDirection );
                DBG_ASSERT(m_pConversionDialog, "Dialog creation failed!");

                m_pConversionDialog->EnableRubySupport( m_pAntiImpl->HasRubySupport() );

                m_pConversionDialog->SetByCharacter( m_bByCharacter );
                m_pConversionDialog->SetConversionFormat( m_eConversionFormat );
                m_pConversionDialog->SetConversionDirectionState( m_bTryBothDirections, m_ePrimaryConversionDirection );

                // the handlers
                m_pConversionDialog->SetOptionsChangedHdl( LINK( this, HangulHanjaConversion_Impl, OnOptionsChanged ) );
                m_pConversionDialog->SetIgnoreHdl( LINK( this, HangulHanjaConversion_Impl, OnIgnore ) );
                m_pConversionDialog->SetIgnoreAllHdl( LINK( this, HangulHanjaConversion_Impl, OnIgnoreAll ) );
                m_pConversionDialog->SetChangeHdl( LINK( this, HangulHanjaConversion_Impl, OnChange ) );
                m_pConversionDialog->SetChangeAllHdl( LINK( this, HangulHanjaConversion_Impl, OnChangeAll ) );
                m_pConversionDialog->SetClickByCharacterHdl( LINK( this, HangulHanjaConversion_Impl, OnByCharClicked ) );
                m_pConversionDialog->SetConversionFormatChangedHdl( LINK( this, HangulHanjaConversion_Impl, OnConversionTypeChanged ) );
                m_pConversionDialog->SetFindHdl( LINK( this, HangulHanjaConversion_Impl, OnFind ) );
            }
        }
    }

    sal_Int16 HangulHanjaConversion_Impl::implGetConversionType( bool bSwitchDirection ) const
    {
        sal_Int16 nConversionType = -1;
        if (m_eConvType == HHC::eConvHangulHanja)
            nConversionType = ( HHC::eHangulToHanja == m_eCurrentConversionDirection && !bSwitchDirection ) ? TO_HANJA : TO_HANGUL;
        else if (m_eConvType == HHC::eConvSimplifiedTraditional)
            nConversionType = LANGUAGE_CHINESE_SIMPLIFIED == m_nTargetLang ? TO_SCHINESE : TO_TCHINESE;
        DBG_ASSERT( nConversionType != -1, "unexpected conversion type" );
        return nConversionType;
    }

    bool HangulHanjaConversion_Impl::implUpdateSuggestions( bool _bAllowSearchNextConvertibleText, const sal_Int32 _nStartAt )
    {
        // parameters for the converter
        sal_Int32 nStartSearch = m_nCurrentStartIndex;
        if( _bAllowSearchNextConvertibleText )
            nStartSearch = _nStartAt;

        sal_Int32 nLength = m_sCurrentPortion.getLength() - nStartSearch;
        m_nCurrentConversionType = implGetConversionType();
        m_nCurrentConversionOption = IsByCharacter() ? CHARACTER_BY_CHARACTER : css::i18n::TextConversionOption::NONE;
        if( m_bIgnorePostPositionalWord )
            m_nCurrentConversionOption = m_nCurrentConversionOption | IGNORE_POST_POSITIONAL_WORD;

        // no need to check both directions for chinese conversion (saves time)
        if (m_eConvType == HHC::eConvSimplifiedTraditional)
            m_bTryBothDirections = false;

        bool bFoundAny = true;
        try
        {
            TextConversionResult aResult = m_xConverter->getConversions(
                m_sCurrentPortion,
                nStartSearch,
                nLength,
                m_aSourceLocale,
                m_nCurrentConversionType,
                m_nCurrentConversionOption
            );
            const bool bFoundPrimary = aResult.Boundary.startPos < aResult.Boundary.endPos;
            bFoundAny = bFoundPrimary;

            if ( m_bTryBothDirections )
            {   // see if we find another convertible when assuming the other direction
                TextConversionResult aSecondResult = m_xConverter->getConversions(
                    m_sCurrentPortion,
                    nStartSearch,
                    nLength,
                    m_aSourceLocale,
                    implGetConversionType( true ), // switched!
                    m_nCurrentConversionOption
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
                        m_eCurrentConversionDirection = ( HHC::eHangulToHanja == m_eCurrentConversionDirection )
                            ? HHC::eHanjaToHangul : HHC::eHangulToHanja;
                        bFoundAny = true;
                    }
                }
            }

            if( _bAllowSearchNextConvertibleText )
            {
                //this might change the current position
                m_aCurrentSuggestions = aResult.Candidates;
                m_nCurrentStartIndex = aResult.Boundary.startPos;
                m_nCurrentEndIndex = aResult.Boundary.endPos;
            }
            else
            {
                //the change of starting position is not allowed
                if( m_nCurrentStartIndex == aResult.Boundary.startPos
                    && aResult.Boundary.endPos != aResult.Boundary.startPos )
                {
                    m_aCurrentSuggestions = aResult.Candidates;
                    m_nCurrentEndIndex = aResult.Boundary.endPos;
                }
                else
                {
                    m_aCurrentSuggestions.realloc( 0 );
                    if( m_sCurrentPortion.getLength() >= m_nCurrentStartIndex+1 )
                        m_nCurrentEndIndex = m_nCurrentStartIndex+1;
                }
            }

            //put recently used string to front:
            if( m_bShowRecentlyUsedFirst && m_aCurrentSuggestions.getLength()>1 )
            {
                OUString sCurrentUnit( GetCurrentUnit() );
                StringMap::const_iterator aRecentlyUsed = m_aRecentlyUsedList.find( sCurrentUnit );
                bool bUsedBefore = aRecentlyUsed != m_aRecentlyUsedList.end();
                if( bUsedBefore && m_aCurrentSuggestions[0] != aRecentlyUsed->second )
                {
                    sal_Int32 nCount = m_aCurrentSuggestions.getLength();
                    Sequence< OUString > aTmp(nCount);
                    aTmp[0]=aRecentlyUsed->second;
                    sal_Int32 nDiff = 1;
                    for( sal_Int32 n=1; n<nCount; n++)//we had 0 already
                    {
                        if( nDiff && m_aCurrentSuggestions[n-nDiff]==aRecentlyUsed->second )
                            nDiff=0;
                        aTmp[n]=m_aCurrentSuggestions[n-nDiff];
                    }
                    m_aCurrentSuggestions = aTmp;
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "HangulHanjaConversion_Impl::implNextConvertibleUnit: caught an exception!" );

            //!!! at least we want to move on in the text in order
            //!!! to avoid an endless loop...
            return false;
        }
        return bFoundAny;
    }

    bool HangulHanjaConversion_Impl::implNextConvertibleUnit( const sal_Int32 _nStartAt )
    {
        m_aCurrentSuggestions.realloc( 0 );

        // ask the TextConversion service for the next convertible piece of text

        // get current values from dialog
        if( m_eConvType == HHC::eConvHangulHanja && m_pConversionDialog )
        {
            m_bTryBothDirections = m_pConversionDialog->GetUseBothDirections();
            HHC::ConversionDirection eDialogDirection = HHC::eHangulToHanja;
            eDialogDirection = m_pConversionDialog->GetDirection( eDialogDirection );

            if( !m_bTryBothDirections && eDialogDirection != m_eCurrentConversionDirection )
            {
                m_eCurrentConversionDirection = eDialogDirection;
            }

            // save currently used value for possible later use
            m_pAntiImpl->m_bTryBothDirectionsSave = m_bTryBothDirections;
            m_pAntiImpl->m_ePrimaryConversionDirectionSave = m_eCurrentConversionDirection;
        }

        bool bFoundAny = implUpdateSuggestions( true, _nStartAt );

        return  bFoundAny &&
                (m_nCurrentStartIndex < m_sCurrentPortion.getLength());
    }

    bool HangulHanjaConversion_Impl::implRetrieveNextPortion( )
    {
        const bool bAllowImplicitChanges = m_eConvType == HHC::eConvSimplifiedTraditional;

        m_sCurrentPortion.clear();
        m_nCurrentPortionLang = LANGUAGE_NONE;
        m_pAntiImpl->GetNextPortion( m_sCurrentPortion, m_nCurrentPortionLang, bAllowImplicitChanges );
        m_nReplacementBaseIndex = 0;
        m_nCurrentStartIndex = m_nCurrentEndIndex = 0;

        bool bRet = !m_sCurrentPortion.isEmpty();

        if (m_eConvType == HHC::eConvHangulHanja && m_bTryBothDirections)
            implGetConversionDirectionForCurrentPortion( m_eCurrentConversionDirection );

        return bRet;
    }

    bool HangulHanjaConversion_Impl::implNextConvertible( bool _bRepeatUnit )
    {
        if ( _bRepeatUnit || ( m_nCurrentEndIndex < m_sCurrentPortion.getLength() ) )
        {
            if ( implNextConvertibleUnit(
                        _bRepeatUnit
                    ?   m_nCurrentStartIndex
                    :   m_nCurrentEndIndex
                ) )
                return true;
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
                    return true;
            }
        }
        while ( !m_sCurrentPortion.isEmpty() );

        // no more portions
        return false;
    }

    OUString HangulHanjaConversion_Impl::GetCurrentUnit() const
    {
        DBG_ASSERT( m_nCurrentStartIndex < m_sCurrentPortion.getLength(),
            "HangulHanjaConversion_Impl::GetCurrentUnit: invalid index into current portion!" );
        DBG_ASSERT( m_nCurrentEndIndex <= m_sCurrentPortion.getLength(),
            "HangulHanjaConversion_Impl::GetCurrentUnit: invalid index into current portion!" );
        DBG_ASSERT( m_nCurrentStartIndex <= m_nCurrentEndIndex,
            "HangulHanjaConversion_Impl::GetCurrentUnit: invalid interval!" );

        OUString sCurrentUnit = m_sCurrentPortion.copy( m_nCurrentStartIndex, m_nCurrentEndIndex - m_nCurrentStartIndex );
        return sCurrentUnit;
    }

    bool HangulHanjaConversion_Impl::ContinueConversion( bool _bRepeatCurrentUnit )
    {
        while ( implNextConvertible( _bRepeatCurrentUnit ) )
        {
            OUString sCurrentUnit( GetCurrentUnit() );

            // do we need to ignore it?
            const bool bAlwaysIgnoreThis = m_sIgnoreList.end() != m_sIgnoreList.find( sCurrentUnit );

            // do we need to change it?
            StringMap::const_iterator aChangeListPos = m_aChangeList.find( sCurrentUnit );
            const bool bAlwaysChangeThis = m_aChangeList.end() != aChangeListPos;

            // do we automatically change this?
            const bool bAutoChange = m_bAutoReplaceUnique && m_aCurrentSuggestions.getLength() == 1;

            if (!m_bIsInteractive)
            {
                // silent conversion (e.g. for simplified/traditional Chinese)...
                if(m_aCurrentSuggestions.getLength()>0)
                    implChange( m_aCurrentSuggestions.getConstArray()[0] );
            }
            else if (bAutoChange)
            {
                implChange( m_aCurrentSuggestions.getConstArray()[0] );
            }
            else if ( bAlwaysChangeThis )
            {
                implChange( aChangeListPos->second );
            }
            else if ( !bAlwaysIgnoreThis )
            {
                // here we need to ask the user for what to do with the text
                // for this, allow derivees to highlight the current text unit in a possible document view
                m_pAntiImpl->HandleNewUnit( m_nCurrentStartIndex - m_nReplacementBaseIndex, m_nCurrentEndIndex - m_nReplacementBaseIndex );

                DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
                if( m_pConversionDialog )
                    m_pConversionDialog->SetCurrentString( sCurrentUnit, m_aCurrentSuggestions );

                // do not look for the next convertible: We have to wait for the user to interactivly
                // decide what happens with the current convertible
                return false;
            }
        }

        return true;
    }

    bool HangulHanjaConversion_Impl::implGetConversionDirectionForCurrentPortion( HHC::ConversionDirection& rDirection )
    {
        // - For eConvHangulHanja the direction is determined by
        // the first encountered Korean character.
        // - For eConvSimplifiedTraditional the conversion direction
        // is already specified by the source language.

        bool bSuccess = true;

        if (m_eConvType == HHC::eConvHangulHanja)
        {
            bSuccess = false;
            try
            {
                // get the break iterator service
                Reference< XBreakIterator > xBreakIter = i18n::BreakIterator::create( m_xContext );
                sal_Int32 nNextAsianScript = xBreakIter->beginOfScript( m_sCurrentPortion, m_nCurrentStartIndex, css::i18n::ScriptType::ASIAN );
                if ( -1 == nNextAsianScript )
                    nNextAsianScript = xBreakIter->nextScript( m_sCurrentPortion, m_nCurrentStartIndex, css::i18n::ScriptType::ASIAN );
                if ( ( nNextAsianScript >= m_nCurrentStartIndex ) && ( nNextAsianScript < m_sCurrentPortion.getLength() ) )
                {   // found asian text

                    // determine if it's Hangul
                    CharClass aCharClassificaton( m_xContext, LanguageTag( m_aSourceLocale) );
                    sal_Int16 nScript = aCharClassificaton.getScript( m_sCurrentPortion, sal::static_int_cast< sal_uInt16 >(nNextAsianScript) );
                    if  (   ( UnicodeScript_kHangulJamo == nScript )
                        ||  ( UnicodeScript_kHangulCompatibilityJamo == nScript )
                        ||  ( UnicodeScript_kHangulSyllable == nScript )
                        )
                    {
                        rDirection = HHC::eHangulToHanja;
                    }
                    else
                    {
                        rDirection = HHC::eHanjaToHangul;
                    }

                    bSuccess = true;
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "HangulHanjaConversion_Impl::implGetConversionDirectionForCurrentPortion: caught an exception!" );
            }
        }

        return bSuccess;
    }

    void HangulHanjaConversion_Impl::DoDocumentConversion( )
    {
        // clear the change-all list - it's to be re-initialized for every single document
        {
            StringMap aEmpty;
            m_aChangeList.swap( aEmpty );
        }

        // first of all, we need to guess the direction of our conversion - it is determined by the first
        // hangul or hanja character in the first text
        if ( !implRetrieveNextPortion() )
        {
            SAL_INFO( "editeng", "HangulHanjaConversion_Impl::DoDocumentConversion: why did you call me if you do have nothing to convert?" );
            // nothing to do
            return;
        }
        if( m_eConvType == HHC::eConvHangulHanja )
        {
            //init conversion direction from saved value
            HHC::ConversionDirection eDirection = HHC::eHangulToHanja;
            if(!implGetConversionDirectionForCurrentPortion( eDirection ))
                // something went wrong, has already been asserted
                return;

            if (HangulHanjaConversion::IsUseSavedConversionDirectionState())
            {
                m_ePrimaryConversionDirection = m_pAntiImpl->m_ePrimaryConversionDirectionSave;
                m_bTryBothDirections = m_pAntiImpl->m_bTryBothDirectionsSave;
                if( m_bTryBothDirections )
                    m_eCurrentConversionDirection = eDirection;
                else
                    m_eCurrentConversionDirection = m_ePrimaryConversionDirection;
            }
            else
            {
                m_ePrimaryConversionDirection = eDirection;
                m_eCurrentConversionDirection = eDirection;
            }
        }

        if (m_bIsInteractive  &&  m_eConvType == HHC::eConvHangulHanja)
        {
            //always open dialog if at least having a hangul or hanja text portion
            createDialog();
            if(HangulHanjaConversion::IsUseSavedConversionDirectionState())
                ContinueConversion( false );
            else
                implUpdateData();
            m_pConversionDialog->Execute();
            DELETEZ( m_pConversionDialog );
        }
        else
        {
            const bool bCompletelyDone = ContinueConversion( false );
            DBG_ASSERT( bCompletelyDone, "HangulHanjaConversion_Impl::DoDocumentConversion: ContinueConversion should have returned true here!" );
        }
    }

    void HangulHanjaConversion_Impl::implProceed( bool _bRepeatCurrentUnit )
    {
        if ( ContinueConversion( _bRepeatCurrentUnit ) )
        {   // we're done with the whole document
            DBG_ASSERT( !m_bIsInteractive || m_pConversionDialog, "HangulHanjaConversion_Impl::implProceed: we should not reach this here without dialog!" );
            if ( m_pConversionDialog )
                m_pConversionDialog->EndDialog( RET_OK );
        }
    }

    void HangulHanjaConversion_Impl::implChange( const OUString& _rChangeInto )
    {
        if( _rChangeInto.isEmpty() )
            return;

        // translate the conversion format into a replacement action
        // this translation depends on whether we have a Hangul original, or a Hanja original

        HHC::ReplacementAction eAction( HHC::eExchange );

        if (m_eConvType == HHC::eConvHangulHanja)
        {
            // is the original we're about to change in Hangul?
            const bool bOriginalIsHangul = HHC::eHangulToHanja == m_eCurrentConversionDirection;

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
                    OSL_FAIL( "HangulHanjaConversion_Impl::implChange: invalid/unexpected conversion format!" );
            }
        }

        // the proper indices (the wrapper implementation needs indices relative to the
        // previous replacement)
        DBG_ASSERT( ( m_nReplacementBaseIndex <= m_nCurrentStartIndex ) && ( m_nReplacementBaseIndex <= m_nCurrentEndIndex ),
            "HangulHanjaConversion_Impl::implChange: invalid replacement base!" );

        sal_Int32 nStartIndex = m_nCurrentStartIndex - m_nReplacementBaseIndex;
        sal_Int32 nEndIndex = m_nCurrentEndIndex - m_nReplacementBaseIndex;

        //remind this decision
        m_aRecentlyUsedList[ GetCurrentUnit() ] = _rChangeInto;

        LanguageType *pNewUnitLang = 0;
        LanguageType  nNewUnitLang = LANGUAGE_NONE;
        if (m_eConvType == HHC::eConvSimplifiedTraditional)
        {
            // check if language needs to be changed
            if ( m_pAntiImpl->GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL &&
                !HangulHanjaConversion::IsTraditional( m_nCurrentPortionLang ))
                nNewUnitLang = LANGUAGE_CHINESE_TRADITIONAL;
            else if ( m_pAntiImpl->GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED &&
                     !HangulHanjaConversion::IsSimplified( m_nCurrentPortionLang ))
                nNewUnitLang = LANGUAGE_CHINESE_SIMPLIFIED;
            if (nNewUnitLang != LANGUAGE_NONE)
                pNewUnitLang = &nNewUnitLang;
        }

        // according to FT we should not (yet) bother about Hangul/Hanja conversion here
        //
        // aOffsets is needed in ReplaceUnit below in order to find out
        // exactly which characters are really changed in order to keep as much
        // from attributation for the text as possible.
        Sequence< sal_Int32 > aOffsets;
        Reference< XExtendedTextConversion > xExtConverter( m_xConverter, UNO_QUERY );
        if (m_eConvType == HHC::eConvSimplifiedTraditional && xExtConverter.is())
        {
            try
            {
                xExtConverter->getConversionWithOffset(
                    m_sCurrentPortion,
                    m_nCurrentStartIndex,
                    m_nCurrentEndIndex - m_nCurrentStartIndex,
                    m_aSourceLocale,
                    m_nCurrentConversionType,
                    m_nCurrentConversionOption,
                    aOffsets
                );
            }
            catch( const Exception& )
            {
                OSL_FAIL( "HangulHanjaConversion_Impl::implChange: caught unexpected exception!" );
                aOffsets.realloc(0);
            }
        }

        // do the replacement
        m_pAntiImpl->ReplaceUnit( nStartIndex, nEndIndex, m_sCurrentPortion,
                _rChangeInto, aOffsets, eAction, pNewUnitLang );


        // adjust the replacement base
        m_nReplacementBaseIndex = m_nCurrentEndIndex;
    }

    void HangulHanjaConversion_Impl::implReadOptionsFromConfiguration()
    {
        SvtLinguConfig  aLngCfg;
        aLngCfg.GetProperty( UPH_IS_IGNORE_POST_POSITIONAL_WORD ) >>= m_bIgnorePostPositionalWord;
        aLngCfg.GetProperty( UPH_IS_SHOW_ENTRIES_RECENTLY_USED_FIRST ) >>= m_bShowRecentlyUsedFirst;
        aLngCfg.GetProperty( UPH_IS_AUTO_REPLACE_UNIQUE_ENTRIES ) >>= m_bAutoReplaceUnique;
    }

    void HangulHanjaConversion_Impl::implUpdateData()
    {
        implReadOptionsFromConfiguration();
        implUpdateSuggestions();

        if(m_pConversionDialog)
        {
            OUString sCurrentUnit( GetCurrentUnit() );

            m_pConversionDialog->SetCurrentString( sCurrentUnit, m_aCurrentSuggestions );
            m_pConversionDialog->FocusSuggestion();
        }

        m_pAntiImpl->HandleNewUnit( m_nCurrentStartIndex - m_nReplacementBaseIndex, m_nCurrentEndIndex - m_nReplacementBaseIndex );
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnOptionsChanged, LinkParamNone*, void)
    {
        //options and dictionaries might have been changed
        //-> update our internal settings and the dialog
        implUpdateData();
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnIgnore, Button*, void)
    {
        // simply ignore, and proceed
        implProceed( false );
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnIgnoreAll, Button*, void)
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnIgnoreAll: no dialog! How this?" );

        if ( m_pConversionDialog )
        {
            OUString sCurrentUnit = m_pConversionDialog->GetCurrentString();
            DBG_ASSERT( m_sIgnoreList.end() == m_sIgnoreList.find( sCurrentUnit ),
                "HangulHanjaConversion_Impl, OnIgnoreAll: shouldn't this have been ignored before" );

            // put into the "ignore all" list
            m_sIgnoreList.insert( sCurrentUnit );

            // and proceed
            implProceed( false );
        }
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnChange, Button*, void)
    {
        // change
        DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
        if( m_pConversionDialog )
            implChange( m_pConversionDialog->GetCurrentSuggestion( ) );
        // and proceed
        implProceed( false );
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnChangeAll, Button*, void)
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnChangeAll: no dialog! How this?" );
        if ( m_pConversionDialog )
        {
            OUString sCurrentUnit( m_pConversionDialog->GetCurrentString() );
            OUString sChangeInto( m_pConversionDialog->GetCurrentSuggestion( ) );

            if( !sChangeInto.isEmpty() )
            {
                // change the current occurrence
                implChange( sChangeInto );

                // put into the "change all" list
                m_aChangeList.insert( StringMap::value_type( sCurrentUnit, sChangeInto ) );
            }

            // and proceed
            implProceed( false );
        }
    }

    IMPL_LINK_TYPED( HangulHanjaConversion_Impl, OnByCharClicked, CheckBox*, _pBox, void )
    {
        m_bByCharacter = _pBox->IsChecked();

        // continue conversion, without advancing to the next unit, but instead continuing with the current unit
        implProceed( true );
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnConversionTypeChanged, Button*, void)
    {
        DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
        if( m_pConversionDialog )
            m_eConversionFormat = m_pConversionDialog->GetConversionFormat( );
    }

    IMPL_LINK_NOARG_TYPED(HangulHanjaConversion_Impl, OnFind, Button*, void)
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnFind: where did this come from?" );
        if ( m_pConversionDialog )
        {
            try
            {
                OUString sNewOriginal( m_pConversionDialog->GetCurrentSuggestion( ) );
                Sequence< OUString > aSuggestions;

                DBG_ASSERT( m_xConverter.is(), "HangulHanjaConversion_Impl::OnFind: no converter!" );
                TextConversionResult aToHanja = m_xConverter->getConversions(
                    sNewOriginal,
                    0, sNewOriginal.getLength(),
                    m_aSourceLocale,
                    TextConversionType::TO_HANJA,
                    TextConversionOption::NONE
                );
                TextConversionResult aToHangul = m_xConverter->getConversions(
                    sNewOriginal,
                    0, sNewOriginal.getLength(),
                    m_aSourceLocale,
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

                m_pConversionDialog->SetCurrentString( sNewOriginal, aSuggestions, false );
                m_pConversionDialog->FocusSuggestion();
            }
            catch( const Exception& )
            {
                OSL_FAIL( "HangulHanjaConversion_Impl::OnFind: caught an exception!" );
            }
        }
    }

    bool    HangulHanjaConversion::m_bUseSavedValues        = false;
    bool    HangulHanjaConversion::m_bTryBothDirectionsSave = false;
    HHC::ConversionDirection HangulHanjaConversion::m_ePrimaryConversionDirectionSave   = HHC::eHangulToHanja;

    HangulHanjaConversion::HangulHanjaConversion( vcl::Window* _pUIParent,
        const Reference< XComponentContext >& rxContext,
        const lang::Locale& _rSourceLocale, const lang::Locale& _rTargetLocale,
        const vcl::Font* _pTargetFont,
        sal_Int32 _nOptions, bool _bIsInteractive)
        :m_pImpl( new HangulHanjaConversion_Impl( _pUIParent, rxContext, _rSourceLocale, _rTargetLocale, _pTargetFont, _nOptions, _bIsInteractive, this ) )
    {
    }

    HangulHanjaConversion::~HangulHanjaConversion( )
    {
    }

    void HangulHanjaConversion::SetUseSavedConversionDirectionState( bool bVal )
    {
        m_bUseSavedValues = bVal;
    }

    bool HangulHanjaConversion::IsUseSavedConversionDirectionState()
    {
        return m_bUseSavedValues;
    }

    LanguageType HangulHanjaConversion::GetSourceLanguage( ) const
    {
        return m_pImpl->GetSourceLang();
    }

    LanguageType HangulHanjaConversion::GetTargetLanguage( ) const
    {
        return m_pImpl->GetTargetLang();
    }

    const vcl::Font * HangulHanjaConversion::GetTargetFont( ) const
    {
        return m_pImpl->GetTargetFont();
    }

    sal_Int32 HangulHanjaConversion::GetConversionOptions( ) const
    {
        return m_pImpl->GetConvOptions();
    }

    bool HangulHanjaConversion::IsInteractive( ) const
    {
        return m_pImpl->IsInteractive();
    }

    void HangulHanjaConversion::ConvertDocument()
    {
        if ( m_pImpl->IsValid() )
            m_pImpl->DoDocumentConversion( );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
