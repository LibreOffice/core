#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x6d, 0x72, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x4d, 0x61, 0x72, 0x61, 0x74, 0x68, 0x69, 0x0 };
    static const sal_Unicode countryID[] = { 0x49, 0x4e, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x49, 0x6e, 0x64, 0x69, 0x61, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_mr_IN(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_mr_IN(sal_Int16& count)
    {
        return getLocaleItem_hi_IN(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x20b9, 0x2d, 0x34, 0x34, 0x45, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_hi_IN(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_mr_IN(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_hi_IN(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_mr_IN(sal_Int16& count)
    {
        return getDateAcceptancePatterns_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_mr_IN(sal_Int16& count)
    {
        return getCollatorImplementation_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_mr_IN(sal_Int16& count)
    {
        return getCollationOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_mr_IN(sal_Int16& count)
    {
        return getSearchOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_mr_IN(sal_Int16& count)
    {
        return getIndexAlgorithm_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_mr_IN(sal_Int16& count)
    {
        return getUnicodeScripts_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_mr_IN(sal_Int16& count)
    {
        return getFollowPageWords_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_mr_IN(sal_Int16& count)
    {
        return getAllCalendars_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_mr_IN(sal_Int16& count)
    {
        return getAllCurrencies_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_mr_IN(sal_Int16& count)
    {
        return getTransliterations_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_mr_IN(sal_Int16& count)
    {
        return getForbiddenCharacters_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_mr_IN(sal_Int16& count)
    {
        return getBreakIteratorRules_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_mr_IN(sal_Int16& count)
    {
        return getReservedWords_hi_IN(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_hi_IN(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_mr_IN(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_hi_IN(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_mr_IN(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
