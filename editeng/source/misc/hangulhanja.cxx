/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::i18n::TextConversionOption;
    using namespace ::com::sun::star::i18n::TextConversionType;
    using namespace ::com::sun::star::lang;

    class HangulHanjaConversion_Impl
    {
    private:
        typedef ::std::set< OUString, ::std::less< OUString > >                   StringBag;
        typedef ::std::map< OUString, OUString, ::std::less< OUString > >  StringMap;

    private:
        StringBag               m_sIgnoreList;
        StringMap               m_aChangeList;
        static StringMap        m_aRecentlyUsedList;

        
        AbstractHangulHanjaConversionDialog*
                                m_pConversionDialog;    
        Window*                 m_pUIParent;            
        Reference< XComponentContext >
                                m_xContext;             
        Reference< XExtendedTextConversion >
                                m_xConverter;           
        Locale                  m_aSourceLocale;        

        
        HHC::ConversionType     m_eConvType;        
        LanguageType            m_nSourceLang;      
                                                    
                                                    
        LanguageType            m_nTargetLang;      
        const Font*             m_pTargetFont;      
        sal_Int32               m_nConvOptions;     
        bool                    m_bIsInteractive;   
                                                    
                                                    
                                                    
                                                    

        HangulHanjaConversion*  m_pAntiImpl;            

        
        bool                        m_bByCharacter;                 
        HHC::ConversionFormat       m_eConversionFormat;            
        HHC::ConversionDirection    m_ePrimaryConversionDirection;  
        HHC::ConversionDirection    m_eCurrentConversionDirection;  

        
        bool                    m_bIgnorePostPositionalWord;
        bool                    m_bShowRecentlyUsedFirst;
        bool                    m_bAutoReplaceUnique;

        
        OUString                m_sCurrentPortion;      
        LanguageType            m_nCurrentPortionLang;  
        sal_Int32               m_nCurrentStartIndex;   
        sal_Int32               m_nCurrentEndIndex;     
        sal_Int32               m_nReplacementBaseIndex;
        sal_Int32               m_nCurrentConversionOption;
        sal_Int16               m_nCurrentConversionType;
        Sequence< OUString >
                                m_aCurrentSuggestions;  
                                                        
        bool                    m_bTryBothDirections;   


    public:
        HangulHanjaConversion_Impl(
            Window* _pUIParent,
            const Reference< XComponentContext >& rxContext,
            const Locale& _rSourceLocale,
            const Locale& _rTargetLocale,
            const Font* _pTargetFont,
            sal_Int32 _nConvOptions,
            bool _bIsInteractive,
            HangulHanjaConversion* _pAntiImpl );

    public:
                void        DoDocumentConversion( );

        inline  bool        IsByCharacter( ) const { return m_bByCharacter; }

        inline  bool        IsValid() const { return m_xConverter.is(); }

        inline LanguageType GetSourceLang() const   { return m_nSourceLang; }
        inline LanguageType GetTargetLang() const   { return m_nTargetLang; }
        inline const Font * GetTargetFont() const   { return m_pTargetFont; }
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
        DECL_LINK( OnOptionsChanged, void* );
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

    HangulHanjaConversion_Impl::HangulHanjaConversion_Impl( Window* _pUIParent,
                                                            const Reference< XComponentContext >& rxContext,
                                                            const Locale& _rSourceLocale,
                                                            const Locale& _rTargetLocale,
                                                            const Font* _pTargetFont,
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
        , m_bByCharacter((_nOptions & CHARACTER_BY_CHARACTER) ? true : false)
        , m_eConversionFormat( HHC::eSimpleConversion)
        , m_ePrimaryConversionDirection( HHC::eHangulToHanja)    
        , m_eCurrentConversionDirection( HHC::eHangulToHanja)    
        , m_nCurrentPortionLang( LANGUAGE_NONE )
        , m_nCurrentStartIndex( 0 )
        , m_nCurrentEndIndex( 0 )
        , m_nReplacementBaseIndex( 0 )
        , m_nCurrentConversionOption( TextConversionOption::NONE )
        , m_nCurrentConversionType( -1 ) 
        , m_bTryBothDirections( true )
    {
        implReadOptionsFromConfiguration();

        DBG_ASSERT( m_xContext.is(), "HangulHanjaConversion_Impl::HangulHanjaConversion_Impl: no ORB!" );

        
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
                DBG_ASSERT(m_pConversionDialog, "Dialogdiet fail!");

                m_pConversionDialog->EnableRubySupport( m_pAntiImpl->HasRubySupport() );

                m_pConversionDialog->SetByCharacter( m_bByCharacter );
                m_pConversionDialog->SetConversionFormat( m_eConversionFormat );
                m_pConversionDialog->SetConversionDirectionState( m_bTryBothDirections, m_ePrimaryConversionDirection );

                
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
        
        sal_Int32 nStartSearch = m_nCurrentStartIndex;
        if( _bAllowSearchNextConvertibleText )
            nStartSearch = _nStartAt;

        sal_Int32 nLength = m_sCurrentPortion.getLength() - nStartSearch;
        m_nCurrentConversionType = implGetConversionType();
        m_nCurrentConversionOption = IsByCharacter() ? CHARACTER_BY_CHARACTER : NONE;
        if( m_bIgnorePostPositionalWord )
            m_nCurrentConversionOption = m_nCurrentConversionOption | IGNORE_POST_POSITIONAL_WORD;

        
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
            {   
                TextConversionResult aSecondResult = m_xConverter->getConversions(
                    m_sCurrentPortion,
                    nStartSearch,
                    nLength,
                    m_aSourceLocale,
                    implGetConversionType( true ), 
                    m_nCurrentConversionOption
                );
                if ( aSecondResult.Boundary.startPos < aSecondResult.Boundary.endPos )
                {   

                    
                    
                    if  (   !bFoundPrimary
                        
                        ||  ( aSecondResult.Boundary.startPos < aResult.Boundary.startPos )
                        )
                    {
                        
                        aResult = aSecondResult;

                        
                        m_eCurrentConversionDirection = ( HHC::eHangulToHanja == m_eCurrentConversionDirection )
                            ? HHC::eHanjaToHangul : HHC::eHangulToHanja;
                        bFoundAny = true;
                    }
                }
            }

            if( _bAllowSearchNextConvertibleText )
            {
                
                m_aCurrentSuggestions = aResult.Candidates;
                m_nCurrentStartIndex = aResult.Boundary.startPos;
                m_nCurrentEndIndex = aResult.Boundary.endPos;
            }
            else
            {
                
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
                    for( sal_Int32 n=1; n<nCount; n++)
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

            
            
            return false;
        }
        return bFoundAny;
    }

    bool HangulHanjaConversion_Impl::implNextConvertibleUnit( const sal_Int32 _nStartAt )
    {
        m_aCurrentSuggestions.realloc( 0 );

        

        
        if( m_eConvType == HHC::eConvHangulHanja && m_pConversionDialog )
        {
            m_bTryBothDirections = m_pConversionDialog->GetUseBothDirections();
            HHC::ConversionDirection eDialogDirection = HHC::eHangulToHanja;
            eDialogDirection = m_pConversionDialog->GetDirection( eDialogDirection );

            if( !m_bTryBothDirections && eDialogDirection != m_eCurrentConversionDirection )
            {
                m_eCurrentConversionDirection = eDialogDirection;
            }

            
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

        m_sCurrentPortion = OUString();
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
                    ?   ( IsByCharacter() ? m_nCurrentStartIndex : m_nCurrentStartIndex )
                    :   m_nCurrentEndIndex
                ) )
                return true;
        }

        
        
        do
        {
            
            if ( implRetrieveNextPortion( ) )
            {   
                
                if ( implNextConvertibleUnit( 0 ) )
                    return true;
            }
        }
        while ( !m_sCurrentPortion.isEmpty() );

        
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

            
            const bool bAlwaysIgnoreThis = m_sIgnoreList.end() != m_sIgnoreList.find( sCurrentUnit );

            
            StringMap::const_iterator aChangeListPos = m_aChangeList.find( sCurrentUnit );
            const bool bAlwaysChangeThis = m_aChangeList.end() != aChangeListPos;

            
            const bool bAutoChange = m_bAutoReplaceUnique && m_aCurrentSuggestions.getLength() == 1;

            if (!m_bIsInteractive)
            {
                
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
                
                
                m_pAntiImpl->HandleNewUnit( m_nCurrentStartIndex - m_nReplacementBaseIndex, m_nCurrentEndIndex - m_nReplacementBaseIndex );

                DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
                if( m_pConversionDialog )
                    m_pConversionDialog->SetCurrentString( sCurrentUnit, m_aCurrentSuggestions );

                
                
                return false;
            }
        }

        return true;
    }

    bool HangulHanjaConversion_Impl::implGetConversionDirectionForCurrentPortion( HHC::ConversionDirection& rDirection )
    {
        
        
        
        

        bool bSuccess = true;

        if (m_eConvType == HHC::eConvHangulHanja)
        {
            bSuccess = false;
            try
            {
                
                Reference< XBreakIterator > xBreakIter = BreakIterator::create( m_xContext );
                sal_Int32 nNextAsianScript = xBreakIter->beginOfScript( m_sCurrentPortion, m_nCurrentStartIndex, com::sun::star::i18n::ScriptType::ASIAN );
                if ( -1 == nNextAsianScript )
                    nNextAsianScript = xBreakIter->nextScript( m_sCurrentPortion, m_nCurrentStartIndex, com::sun::star::i18n::ScriptType::ASIAN );
                if ( ( nNextAsianScript >= m_nCurrentStartIndex ) && ( nNextAsianScript < m_sCurrentPortion.getLength() ) )
                {   

                    
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
        
        {
            StringMap aEmpty;
            m_aChangeList.swap( aEmpty );
        }

        
        
        if ( !implRetrieveNextPortion() )
        {
            DBG_WARNING( "HangulHanjaConversion_Impl::DoDocumentConversion: why did you call me if you do have nothing to convert?" );
            
            return;
        }
        if( m_eConvType == HHC::eConvHangulHanja )
        {
            
            HHC::ConversionDirection eDirection = HHC::eHangulToHanja;
            if(!implGetConversionDirectionForCurrentPortion( eDirection ))
                
                return;

            if (m_pAntiImpl->IsUseSavedConversionDirectionState())
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
            
            createDialog();
            if(m_pAntiImpl->IsUseSavedConversionDirectionState())
                ContinueConversion( false );
            else
                implUpdateData();
            m_pConversionDialog->Execute();
            DELETEZ( m_pConversionDialog );
        }
        else
        {
#ifdef DBG_UTIL
            const bool bCompletelyDone =
#endif
            ContinueConversion( false );
            DBG_ASSERT( bCompletelyDone, "HangulHanjaConversion_Impl::DoDocumentConversion: ContinueConversion should have returned true here!" );
        }
    }

    void HangulHanjaConversion_Impl::implProceed( bool _bRepeatCurrentUnit )
    {
        if ( ContinueConversion( _bRepeatCurrentUnit ) )
        {   
            DBG_ASSERT( !m_bIsInteractive || m_pConversionDialog, "HangulHanjaConversion_Impl::implProceed: we should not reach this here without dialog!" );
            if ( m_pConversionDialog )
                m_pConversionDialog->EndDialog( RET_OK );
        }
    }

    void HangulHanjaConversion_Impl::implChange( const OUString& _rChangeInto )
    {
        if( _rChangeInto.isEmpty() )
            return;

        
        

        HHC::ReplacementAction eAction( HHC::eExchange );

        if (m_eConvType == HHC::eConvHangulHanja)
        {
            
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

        
        
        DBG_ASSERT( ( m_nReplacementBaseIndex <= m_nCurrentStartIndex ) && ( m_nReplacementBaseIndex <= m_nCurrentEndIndex ),
            "HangulHanjaConversion_Impl::implChange: invalid replacement base!" );

        sal_Int32 nStartIndex = m_nCurrentStartIndex - m_nReplacementBaseIndex;
        sal_Int32 nEndIndex = m_nCurrentEndIndex - m_nReplacementBaseIndex;

        
        m_aRecentlyUsedList[ GetCurrentUnit() ] = _rChangeInto;

        LanguageType *pNewUnitLang = 0;
        LanguageType  nNewUnitLang = LANGUAGE_NONE;
        if (m_eConvType == HHC::eConvSimplifiedTraditional)
        {
            
            if ( m_pAntiImpl->GetTargetLanguage() == LANGUAGE_CHINESE_TRADITIONAL &&
                !m_pAntiImpl->IsTraditional( m_nCurrentPortionLang ))
                nNewUnitLang = LANGUAGE_CHINESE_TRADITIONAL;
            else if ( m_pAntiImpl->GetTargetLanguage() == LANGUAGE_CHINESE_SIMPLIFIED &&
                     !m_pAntiImpl->IsSimplified( m_nCurrentPortionLang ))
                nNewUnitLang = LANGUAGE_CHINESE_SIMPLIFIED;
            if (nNewUnitLang != LANGUAGE_NONE)
                pNewUnitLang = &nNewUnitLang;
        }

        
        //
        
        
        
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

        
        m_pAntiImpl->ReplaceUnit( nStartIndex, nEndIndex, m_sCurrentPortion,
                _rChangeInto, aOffsets, eAction, pNewUnitLang );


        
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

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnOptionsChanged)
    {
        
        
        implUpdateData();

        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnIgnore)
    {
        
        implProceed( false );
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnIgnoreAll)
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnIgnoreAll: no dialog! How this?" );

        if ( m_pConversionDialog )
        {
            OUString sCurrentUnit = m_pConversionDialog->GetCurrentString();
            DBG_ASSERT( m_sIgnoreList.end() == m_sIgnoreList.find( sCurrentUnit ),
                "HangulHanjaConversion_Impl, OnIgnoreAll: shouldn't this have been ignored before" );

            
            m_sIgnoreList.insert( sCurrentUnit );

            
            implProceed( false );
        }

        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnChange)
    {
        
        DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
        if( m_pConversionDialog )
            implChange( m_pConversionDialog->GetCurrentSuggestion( ) );
        
        implProceed( false );

        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnChangeAll)
    {
        DBG_ASSERT( m_pConversionDialog, "HangulHanjaConversion_Impl::OnChangeAll: no dialog! How this?" );
        if ( m_pConversionDialog )
        {
            OUString sCurrentUnit( m_pConversionDialog->GetCurrentString() );
            OUString sChangeInto( m_pConversionDialog->GetCurrentSuggestion( ) );

            if( !sChangeInto.isEmpty() )
            {
                
                implChange( sChangeInto );

                
                m_aChangeList.insert( StringMap::value_type( sCurrentUnit, sChangeInto ) );
            }

            
            implProceed( false );
        }

        return 0L;
    }

    IMPL_LINK( HangulHanjaConversion_Impl, OnByCharClicked, CheckBox*, _pBox )
    {
        m_bByCharacter = _pBox->IsChecked();

        
        implProceed( true );
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnConversionTypeChanged)
    {
        DBG_ASSERT( m_pConversionDialog, "we should always have a dialog here!" );
        if( m_pConversionDialog )
            m_eConversionFormat = m_pConversionDialog->GetConversionFormat( );
        return 0L;
    }

    IMPL_LINK_NOARG(HangulHanjaConversion_Impl, OnFind)
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
                {   
                    if ( aToHangul.Boundary.startPos < aToHanja.Boundary.startPos )
                        pResult = &aToHangul;
                    else
                        pResult = &aToHanja;
                }
                else if ( bHaveToHanja )
                {   
                    pResult = &aToHanja;
                }
                else
                {   
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
        return 0L;
    }

    bool    HangulHanjaConversion::m_bUseSavedValues        = false;
    bool    HangulHanjaConversion::m_bTryBothDirectionsSave = false;
    HHC::ConversionDirection HangulHanjaConversion::m_ePrimaryConversionDirectionSave   = HHC::eHangulToHanja;

    HangulHanjaConversion::HangulHanjaConversion( Window* _pUIParent,
        const Reference< XComponentContext >& rxContext,
        const Locale& _rSourceLocale, const Locale& _rTargetLocale,
        const Font* _pTargetFont,
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

    const Font * HangulHanjaConversion::GetTargetFont( ) const
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
