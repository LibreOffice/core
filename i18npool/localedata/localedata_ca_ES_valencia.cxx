#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x71, 0x6c, 0x74, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x43, 0x61, 0x74, 0x61, 0x6c, 0x61, 0x6e, 0x20, 0x28, 0x56,
            0x61, 0x6c, 0x65, 0x6e, 0x63, 0x69, 0x61, 0x6e, 0x29, 0x0 };
    static const sal_Unicode countryID[] = { 0x45, 0x53, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x53, 0x70, 0x61, 0x69, 0x6e, 0x0 };
    static const sal_Unicode Variant[] = { 0x63, 0x61, 0x2d, 0x45, 0x53, 0x2d, 0x76, 0x61,
                                           0x6c, 0x65, 0x6e, 0x63, 0x69, 0x61, 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_ca_ES_valencia(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_ca_ES_valencia(sal_Int16& count)
    {
        return getLocaleItem_ca_ES(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x20a7, 0x2d, 0x38, 0x30, 0x33, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_ca_ES(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL getAllFormats0_ca_ES_valencia(
        sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_ca_ES(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getDateAcceptancePatterns_ca_ES_valencia(sal_Int16& count)
    {
        return getDateAcceptancePatterns_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getCollatorImplementation_ca_ES_valencia(sal_Int16& count)
    {
        return getCollatorImplementation_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_ca_ES_valencia(sal_Int16& count)
    {
        return getCollationOptions_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_ca_ES_valencia(sal_Int16& count)
    {
        return getSearchOptions_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_ca_ES_valencia(sal_Int16& count)
    {
        return getIndexAlgorithm_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_ca_ES_valencia(sal_Int16& count)
    {
        return getUnicodeScripts_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_ca_ES_valencia(sal_Int16& count)
    {
        return getFollowPageWords_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_ca_ES_valencia(sal_Int16& count)
    {
        return getAllCalendars_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_ca_ES_valencia(sal_Int16& count)
    {
        return getAllCurrencies_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_ca_ES_valencia(sal_Int16& count)
    {
        return getTransliterations_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getForbiddenCharacters_ca_ES_valencia(sal_Int16& count)
    {
        return getForbiddenCharacters_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getBreakIteratorRules_ca_ES_valencia(sal_Int16& count)
    {
        return getBreakIteratorRules_ca_ES(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_ca_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_ca_ES_valencia(sal_Int16& count)
    {
        return getReservedWords_ca_ES(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_ca_ES(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_ca_ES_valencia(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_ca_ES(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_ca_ES(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_ca_ES_valencia(
        sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_ca_ES(nStyles, nLevels, nAttributes);
    }
} // extern "C"
