/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_SPELL_ATTRIB
#define _SVX_SPELL_ATTRIB

#include <svtools/txtattr.hxx>
#include <i18npool/lang.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <tools/color.hxx>
//namespace com{ namespace sun{ namespace star{ namespace linguistic2{
//    class XSpellAlternatives;
//}}}}

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
    ::com::sun::star::lang::Locale                      aLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XProofreader > xGrammarChecker;
    ::rtl::OUString                                     sServiceName; //service name of GrammarChecker/SpellChecker
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
                      const ::rtl::OUString* pRuleId = 0 ) :
        bIsGrammarError( bGrammar ),
        sErrorText( rText ),
        aLocale( rLocale ),
        xGrammarChecker( rxGrammarChecker ),
        sServiceName( rServiceName ),
        aSuggestions( rSuggestions )
        {
            if( pDialogTitle )
                sDialogTitle = *pDialogTitle;
            if( pExplanation )
                sExplanation = *pExplanation;
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
                sRuleId == rDesc.sRuleId;
    }
};
/* -----------------10.09.2003 14:23-----------------

 --------------------------------------------------*/
class SpellErrorAttrib : public TextAttrib
{
public:

private:
    //com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> m_xAlternatives;
    SpellErrorDescription        m_aSpellErrorDescription;

                            //not accessible
                            SpellErrorAttrib();
public:
//                            SpellErrorAttrib(com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> xAlternatives);
                            SpellErrorAttrib( const SpellErrorDescription& );
                            SpellErrorAttrib( const SpellErrorAttrib& rAttr );
                            ~SpellErrorAttrib();

    const SpellErrorDescription& GetErrorDescription() const { return m_aSpellErrorDescription; }


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};
/* -----------------10.09.2003 14:23-----------------

 --------------------------------------------------*/
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
/* -----------------31.10.2003 16:01-----------------

 --------------------------------------------------*/
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
