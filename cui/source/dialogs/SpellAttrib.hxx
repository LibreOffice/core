/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_SPELL_ATTRIB
#define _SVX_SPELL_ATTRIB

#include <vcl/txtattr.hxx>
#include <i18npool/lang.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <tools/color.hxx>

#define TEXTATTR_SPELL_ERROR            (TEXTATTR_USER_START + 1)
#define TEXTATTR_SPELL_LANGUAGE         (TEXTATTR_USER_START + 2)
#define TEXTATTR_SPELL_BACKGROUND       (TEXTATTR_USER_START + 3)

namespace svx{
struct SpellErrorDescription
{
    bool                                                bIsGrammarError;
    ::rtl::OUString                                     sErrorText;
    ::rtl::OUString                                     sDialogTitle;
    ::rtl::OUString                                     sExplanation;
    ::rtl::OUString                                     sExplanationURL;
    ::com::sun::star::lang::Locale                      aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > xGrammarChecker;
    ::rtl::OUString                                     sServiceName; ///< service name of GrammarChecker/SpellChecker
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aSuggestions;
    ::rtl::OUString                                     sRuleId;

    SpellErrorDescription() :
        bIsGrammarError( false ){}

    SpellErrorDescription( bool bGrammar,
                      const ::rtl::OUString& rText,
                      const ::com::sun::star::lang::Locale& rLocale,
                      const ::com::sun::star::uno::Sequence< ::rtl::OUString >& rSuggestions,
                      ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > rxGrammarChecker,
                      const ::rtl::OUString& rServiceName,
                      const ::rtl::OUString* pDialogTitle = 0,
                      const ::rtl::OUString* pExplanation = 0,
                      const ::rtl::OUString* pRuleId = 0,
                      const ::rtl::OUString* pExplanationURL = 0 ) :
        bIsGrammarError( bGrammar ),
        sErrorText( rText ),
        sDialogTitle( ),
        sExplanation( ),
        sExplanationURL( ),
        aLocale( rLocale ),
        xGrammarChecker( rxGrammarChecker ),
        sServiceName( rServiceName ),
        aSuggestions( rSuggestions )
        {
            if( pDialogTitle )
                sDialogTitle = *pDialogTitle;
            if( pExplanation )
                sExplanation = *pExplanation;
            if( pExplanationURL )
                sExplanationURL = *pExplanationURL;
            if( pRuleId )
                sRuleId = *pRuleId;
        };

    int operator==( const SpellErrorDescription& rDesc ) const
    {
        return bIsGrammarError == rDesc.bIsGrammarError &&
                sErrorText.equals( rDesc.sErrorText ) &&
                aLocale.Language.equals( rDesc.aLocale.Language ) &&
                aLocale.Country.equals( rDesc.aLocale.Country ) &&
                aLocale.Variant.equals( rDesc.aLocale.Variant ) &&
                aSuggestions == rDesc.aSuggestions &&
                xGrammarChecker == rDesc.xGrammarChecker &&
                sDialogTitle.equals( rDesc.sDialogTitle ) &&
                sExplanation.equals( rDesc.sExplanation ) &&
                sExplanationURL.equals( rDesc.sExplanationURL ) &&
                sRuleId == rDesc.sRuleId;
    }
};

//-------------------------------------------------
class SpellErrorAttrib : public TextAttrib
{
public:

private:
    SpellErrorDescription        m_aSpellErrorDescription;

                            //not accessible
                            SpellErrorAttrib();
public:
                            SpellErrorAttrib( const SpellErrorDescription& );
                            SpellErrorAttrib( const SpellErrorAttrib& rAttr );
                            ~SpellErrorAttrib();

    const SpellErrorDescription& GetErrorDescription() const { return m_aSpellErrorDescription; }


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};

//-------------------------------------------------
class SpellLanguageAttrib : public TextAttrib
{
    LanguageType m_eLanguage;

                            //not accessible
                            SpellLanguageAttrib();

public:
                            SpellLanguageAttrib(LanguageType eLanguage);
                            SpellLanguageAttrib( const SpellLanguageAttrib& rAttr );
                            ~SpellLanguageAttrib();

    LanguageType            GetLanguage() const {return m_eLanguage;}
    void                    SetLanguage(LanguageType eLang)
                                        {m_eLanguage = eLang;}


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};

//-------------------------------------------------
class SpellBackgroundAttrib : public TextAttrib
{
    Color   m_aBackgroundColor;

                            //not accessible
                            SpellBackgroundAttrib();

public:
                            SpellBackgroundAttrib(const Color& rCol);
                            SpellBackgroundAttrib( const SpellBackgroundAttrib& rAttr );
                            ~SpellBackgroundAttrib();

    const   Color&          GetColor() const { return m_aBackgroundColor;}
    void                    SetColor( const Color& rNewCol ){m_aBackgroundColor = rNewCol;}


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};
}//namespace svx
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
