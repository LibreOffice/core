#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x61, 0x66, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x41, 0x66, 0x72, 0x69, 0x6b, 0x61, 0x61, 0x6e, 0x73, 0x0 };
    static const sal_Unicode countryID[] = { 0x4e, 0x41, 0x0 };
    static const sal_Unicode countryDefaultName[]
        = { 0x4e, 0x61, 0x6d, 0x69, 0x62, 0x69, 0x61, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_af_NA(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_en_NA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_af_NA(sal_Int16& count)
    {
        return getLocaleItem_en_NA(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x24, 0x2d, 0x38, 0x30, 0x33, 0x36, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_en_NA(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_af_NA(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_en_NA(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_en_NA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_af_NA(sal_Int16& count)
    {
        return getDateAcceptancePatterns_en_NA(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_af_NA(sal_Int16& count)
    {
        return getCollatorImplementation_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_af_NA(sal_Int16& count)
    {
        return getCollationOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_af_NA(sal_Int16& count)
    {
        return getSearchOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_af_NA(sal_Int16& count)
    {
        return getIndexAlgorithm_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_af_NA(sal_Int16& count)
    {
        return getUnicodeScripts_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_af_NA(sal_Int16& count)
    {
        return getFollowPageWords_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_af_NA(sal_Int16& count)
    {
        return getAllCalendars_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_en_NA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_af_NA(sal_Int16& count)
    {
        return getAllCurrencies_en_NA(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_af_NA(sal_Int16& count)
    {
        return getTransliterations_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_af_NA(sal_Int16& count)
    {
        return getForbiddenCharacters_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_af_NA(sal_Int16& count)
    {
        return getBreakIteratorRules_af_ZA(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_af_ZA(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_af_NA(sal_Int16& count)
    {
        return getReservedWords_af_ZA(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_af_NA(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_af_NA(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
