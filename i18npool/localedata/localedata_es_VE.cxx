#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x65, 0x73, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x53, 0x70, 0x61, 0x6e, 0x69, 0x73, 0x68, 0x0 };
    static const sal_Unicode countryID[] = { 0x56, 0x45, 0x0 };
    static const sal_Unicode countryDefaultName[]
        = { 0x56, 0x65, 0x6e, 0x65, 0x7a, 0x75, 0x65, 0x6c, 0x61, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_es_VE(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_es_AR(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_es_VE(sal_Int16& count)
    {
        return getLocaleItem_es_AR(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x42, 0x73, 0x2d, 0x32, 0x30, 0x30, 0x41, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_es_AR(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_es_VE(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_es_AR(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_es_AR(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_es_VE(sal_Int16& count)
    {
        return getDateAcceptancePatterns_es_AR(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_es_VE(sal_Int16& count)
    {
        return getCollatorImplementation_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_es_VE(sal_Int16& count)
    {
        return getCollationOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_es_VE(sal_Int16& count)
    {
        return getSearchOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_es_VE(sal_Int16& count)
    {
        return getIndexAlgorithm_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_es_VE(sal_Int16& count)
    {
        return getUnicodeScripts_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_es_VE(sal_Int16& count)
    {
        return getFollowPageWords_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_es_AR(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_es_VE(sal_Int16& count)
    {
        return getAllCalendars_es_AR(count);
    }
    static const sal_Unicode defaultCurrency0[] = { 0 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
    static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
    static const sal_Unicode currencyID0[] = { 0x56, 0x45, 0x42, 0x0 };
    static const sal_Unicode currencySymbol0[] = { 0x42, 0x73, 0x0 };
    static const sal_Unicode bankSymbol0[] = { 0x56, 0x45, 0x42, 0x0 };
    static const sal_Unicode currencyName0[] = { 0x42, 0x6f, 0x6c, 0x69, 0x76, 0x61, 0x72, 0x0 };
    static const sal_Unicode currencyDecimalPlaces0[] = { 2 };

    static const sal_Unicode defaultCurrency1[] = { 0 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes1[] = { 0 };
    static const sal_Unicode defaultCurrencyLegacyOnly1[] = { 0 };
    static const sal_Unicode currencyID1[] = { 0x56, 0x45, 0x46, 0x0 };
    static const sal_Unicode currencySymbol1[] = { 0x42, 0x73, 0x46, 0x0 };
    static const sal_Unicode bankSymbol1[] = { 0x56, 0x45, 0x46, 0x0 };
    static const sal_Unicode currencyName1[] = { 0x42, 0x6f, 0x6c, 0x69, 0x76, 0x61, 0x72, 0x20,
                                                 0x46, 0x75, 0x65, 0x72, 0x74, 0x65, 0x0 };
    static const sal_Unicode currencyDecimalPlaces1[] = { 2 };

    static const sal_Unicode defaultCurrency2[] = { 1 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes2[] = { 0 };
    static const sal_Unicode defaultCurrencyLegacyOnly2[] = { 0 };
    static const sal_Unicode currencyID2[] = { 0x56, 0x45, 0x53, 0x0 };
    static const sal_Unicode currencySymbol2[] = { 0x42, 0x73, 0x2e, 0x53, 0x2e, 0x0 };
    static const sal_Unicode bankSymbol2[] = { 0x56, 0x45, 0x53, 0x0 };
    static const sal_Unicode currencyName2[]
        = { 0x42, 0x6f, 0x6c, 0x69, 0x76, 0x61, 0x72, 0x20, 0x53,
            0x6f, 0x62, 0x65, 0x72, 0x61, 0x6e, 0x6f, 0x0 };
    static const sal_Unicode currencyDecimalPlaces2[] = { 2 };

    static const sal_Int16 currencyCount = 3;

    static const sal_Unicode* currencies[] = {
        currencyID0,
        currencySymbol0,
        bankSymbol0,
        currencyName0,
        defaultCurrency0,
        defaultCurrencyUsedInCompatibleFormatCodes0,
        currencyDecimalPlaces0,
        defaultCurrencyLegacyOnly0,
        currencyID1,
        currencySymbol1,
        bankSymbol1,
        currencyName1,
        defaultCurrency1,
        defaultCurrencyUsedInCompatibleFormatCodes1,
        currencyDecimalPlaces1,
        defaultCurrencyLegacyOnly1,
        currencyID2,
        currencySymbol2,
        bankSymbol2,
        currencyName2,
        defaultCurrency2,
        defaultCurrencyUsedInCompatibleFormatCodes2,
        currencyDecimalPlaces2,
        defaultCurrencyLegacyOnly2,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_es_VE(sal_Int16& count)
    {
        count = currencyCount;
        return (sal_Unicode**)currencies;
    }
    extern sal_Unicode** SAL_CALL getTransliterations_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_es_VE(sal_Int16& count)
    {
        return getTransliterations_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_es_VE(sal_Int16& count)
    {
        return getForbiddenCharacters_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_es_VE(sal_Int16& count)
    {
        return getBreakIteratorRules_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_es_VE(sal_Int16& count)
    {
        return getReservedWords_es_ES(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_es_VE(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_es_VE(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
