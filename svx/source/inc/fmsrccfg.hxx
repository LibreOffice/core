/*************************************************************************
 *
 *  $RCSfile: fmsrccfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-21 13:05:14 $
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

#ifndef _FMSRCCF_HXX_
#define _FMSRCCF_HXX_

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif
#ifndef UNOTOOLS_CONFIGVALUECONTAINER_HXX
#include <unotools/configvaluecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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

    struct FmSearchParams
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

        sal_Bool        bAllFields;
        sal_Bool        bUseFormatter;
        sal_Bool        bBackwards;
        sal_Bool        bWildcard;
        sal_Bool        bRegular;
        sal_Bool        bApproxSearch;
            // the last three are mutually exclusive

        // for Levenshtein-search:
        sal_Bool        bLevRelaxed;

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

    class FmSearchConfigItem
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
        void    implTranslateFromConfig( );
        void    implTranslateToConfig( );
    };

//........................................................................
}   // namespace svxform
//........................................................................

// ===================================================================================================

#endif // _FMSRCCF_HXX_

