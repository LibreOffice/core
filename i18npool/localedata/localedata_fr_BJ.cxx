#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x66, 0x72, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x46, 0x72, 0x65, 0x6e, 0x63, 0x68, 0x0 };
    static const sal_Unicode countryID[] = { 0x42, 0x4a, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x42, 0xe9, 0x6e, 0x69, 0x6e, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_fr_BJ(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_fr_BJ(sal_Int16& count)
    {
        return getLocaleItem_fr_BF(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x46, 0x2d, 0x38, 0x34, 0x30, 0x43, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_fr_BF(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_fr_BJ(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_fr_BF(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_fr_BJ(sal_Int16& count)
    {
        return getDateAcceptancePatterns_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_fr_BJ(sal_Int16& count)
    {
        return getCollatorImplementation_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_fr_BJ(sal_Int16& count)
    {
        return getCollationOptions_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_fr_BJ(sal_Int16& count)
    {
        return getSearchOptions_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_fr_BJ(sal_Int16& count)
    {
        return getIndexAlgorithm_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_fr_BJ(sal_Int16& count)
    {
        return getUnicodeScripts_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_fr_BJ(sal_Int16& count)
    {
        return getFollowPageWords_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_fr_BJ(sal_Int16& count)
    {
        return getAllCalendars_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_fr_BJ(sal_Int16& count)
    {
        return getAllCurrencies_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_fr_BJ(sal_Int16& count)
    {
        return getTransliterations_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_fr_BJ(sal_Int16& count)
    {
        return getForbiddenCharacters_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_fr_BJ(sal_Int16& count)
    {
        return getBreakIteratorRules_fr_BF(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_fr_BF(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_fr_BJ(sal_Int16& count)
    {
        return getReservedWords_fr_BF(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_fr_BF(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_fr_BJ(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_fr_BF(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_fr_BF(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_fr_BJ(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_fr_BF(nStyles, nLevels, nAttributes);
    }
} // extern "C"
