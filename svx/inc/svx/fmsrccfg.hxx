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



#ifndef _FMSRCCF_HXX_
#define _FMSRCCF_HXX_

#include <tools/string.hxx>
#include <unotools/configvaluecontainer.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "svx/svxdllapi.h"

// ===================================================================================================

#define MATCHING_ANYWHERE       0
#define MATCHING_BEGINNING      1
#define MATCHING_END            2
#define MATCHING_WHOLETEXT      3

// diese Konstanten duerfen NICHT einfach geaendert werden, da der SearchDialog aus den Einstellungen seiner ListBox
// keine Umrechnungen vornimmt, sondern die einfach in die entsprechenden Methoden der SearchEngine steckt
// (wenn man also die Konstanten oder die Reihenfolge der Listbox-Eintraege im Dialog aendert, muss das jeweils andere
// angepasst werden)


//........................................................................
namespace svxform
{
//........................................................................

    // ===================================================================================================
    // = struct FmSearchParams - Parameter einer Suche
    // ===================================================================================================

    struct SVX_DLLPUBLIC FmSearchParams
    {
    protected:
        sal_Int32       nTransliterationFlags;
            // they're way too sensitive for direct access ....

    public:
        // no bit fields at all (want to pass the addresses to the OConfigurationValueContainer)
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
                        aHistory;
        ::rtl::OUString sSingleSearchField;

        sal_Int16       nSearchForType;
        sal_Int16       nPosition;
        sal_Int16       nLevOther;
        sal_Int16       nLevShorter;
        sal_Int16       nLevLonger;
        sal_Bool        bLevRelaxed;

        sal_Bool        bAllFields;
        sal_Bool        bUseFormatter;
        sal_Bool        bBackwards;
        sal_Bool        bWildcard;
        sal_Bool        bRegular;
        sal_Bool        bApproxSearch;
            // the last three are mutually exclusive

        sal_Bool        bSoundsLikeCJK;

        // ......................................
        FmSearchParams();

        sal_Bool    isIgnoreWidthCJK( ) const;
        void        setIgnoreWidthCJK( sal_Bool _bIgnore );

        sal_Int32   getTransliterationFlags( ) const { return nTransliterationFlags; }
        void        setTransliterationFlags( sal_Int32 _nFlags ) { nTransliterationFlags = _nFlags; }

        sal_Bool    isCaseSensitive( ) const;
        void        setCaseSensitive( sal_Bool _bCase );
    };

    // ===================================================================================================
    // = class FmSearchConfigItem - ein ConfigItem, dass sich Suchparameter merkt
    // ===================================================================================================

    class SVX_DLLPUBLIC FmSearchConfigItem
                :protected  FmSearchParams
                ,public     ::utl::OConfigurationValueContainer
                // order matters!
    {
    private:
        ::osl::Mutex    m_aMutex;

        // wrapper properties:
        // some of the members of FmSearchParams are must be translated to be stored in the configuration
        ::rtl::OUString     m_sSearchForType;
        ::rtl::OUString     m_sSearchPosition;

        sal_Bool            m_bIsMatchCase;
        sal_Bool            m_bIsMatchFullHalfWidthForms;
        sal_Bool            m_bIsMatchHiraganaKatakana;
        sal_Bool            m_bIsMatchContractions;
        sal_Bool            m_bIsMatchMinusDashCho_on;
        sal_Bool            m_bIsMatchRepeatCharMarks;
        sal_Bool            m_bIsMatchVariantFormKanji;
        sal_Bool            m_bIsMatchOldKanaForms;
        sal_Bool            m_bIsMatch_DiZi_DuZu;
        sal_Bool            m_bIsMatch_BaVa_HaFa;
        sal_Bool            m_bIsMatch_TsiThiChi_DhiZi;
        sal_Bool            m_bIsMatch_HyuIyu_ByuVyu;
        sal_Bool            m_bIsMatch_SeShe_ZeJe;
        sal_Bool            m_bIsMatch_IaIya;
        sal_Bool            m_bIsMatch_KiKu;
        sal_Bool            m_bIsIgnorePunctuation;
        sal_Bool            m_bIsIgnoreWhitespace;
        sal_Bool            m_bIsIgnoreProlongedSoundMark;
        sal_Bool            m_bIsIgnoreMiddleDot;

    public:
        FmSearchConfigItem();
        ~FmSearchConfigItem();

        const FmSearchParams&   getParams( ) const;
        void                    setParams( const FmSearchParams& _rParams );

    private:
        SVX_DLLPRIVATE void implTranslateFromConfig( );
        SVX_DLLPRIVATE void implTranslateToConfig( );
    };

//........................................................................
}   // namespace svxform
//........................................................................

// ===================================================================================================

#endif // _FMSRCCF_HXX_

