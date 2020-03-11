#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x65, 0x73, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x53, 0x70, 0x61, 0x6e, 0x69, 0x73, 0x68, 0x0 };
    static const sal_Unicode countryID[] = { 0x55, 0x59, 0x0 };
    static const sal_Unicode countryDefaultName[]
        = { 0x55, 0x72, 0x75, 0x67, 0x75, 0x61, 0x79, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_es_UY(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }

    static const sal_Unicode LC_CTYPE_Unoid[] = { 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x0 };
    static const sal_Unicode dateSeparator[] = { 0x2f, 0x0 };
    static const sal_Unicode thousandSeparator[] = { 0x2e, 0x0 };
    static const sal_Unicode decimalSeparator[] = { 0x2c, 0x0 };
    static const sal_Unicode decimalSeparatorAlternative[] = { 0x0 };
    static const sal_Unicode timeSeparator[] = { 0x3a, 0x0 };
    static const sal_Unicode time100SecSeparator[] = { 0x2c, 0x0 };
    static const sal_Unicode listSeparator[] = { 0x3b, 0x0 };
    static const sal_Unicode LongDateDayOfWeekSeparator[] = { 0x2c, 0x20, 0x0 };
    static const sal_Unicode LongDateDaySeparator[] = { 0x20, 0x0 };
    static const sal_Unicode LongDateMonthSeparator[] = { 0x20, 0x0 };
    static const sal_Unicode LongDateYearSeparator[] = { 0x20, 0x0 };
    static const sal_Unicode quotationStart[] = { 0x2018, 0x0 };
    static const sal_Unicode quotationEnd[] = { 0x2019, 0x0 };
    static const sal_Unicode doubleQuotationStart[] = { 0x201c, 0x0 };
    static const sal_Unicode doubleQuotationEnd[] = { 0x201d, 0x0 };
    static const sal_Unicode timeAM[] = { 0x61, 0x2e, 0x6d, 0x2e, 0x0 };
    static const sal_Unicode timePM[] = { 0x70, 0x2e, 0x6d, 0x2e, 0x0 };
    static const sal_Unicode measurementSystem[] = { 0x4d, 0x65, 0x74, 0x72, 0x69, 0x63, 0x0 };

    static const sal_Unicode* LCType[] = { LC_CTYPE_Unoid,
                                           dateSeparator,
                                           thousandSeparator,
                                           decimalSeparator,
                                           timeSeparator,
                                           time100SecSeparator,
                                           listSeparator,
                                           quotationStart,
                                           quotationEnd,
                                           doubleQuotationStart,
                                           doubleQuotationEnd,
                                           timeAM,
                                           timePM,
                                           measurementSystem,
                                           LongDateDayOfWeekSeparator,
                                           LongDateDaySeparator,
                                           LongDateMonthSeparator,
                                           LongDateYearSeparator,
                                           decimalSeparatorAlternative };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_es_UY(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCType);
        return (sal_Unicode**)LCType;
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x24, 0x2d, 0x33, 0x38, 0x30, 0x41, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_es_AR(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_es_UY(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_es_AR(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_es_AR(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_es_UY(sal_Int16& count)
    {
        return getDateAcceptancePatterns_es_AR(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_es_UY(sal_Int16& count)
    {
        return getCollatorImplementation_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_es_UY(sal_Int16& count)
    {
        return getCollationOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_es_UY(sal_Int16& count)
    {
        return getSearchOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_es_UY(sal_Int16& count)
    {
        return getIndexAlgorithm_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_es_UY(sal_Int16& count)
    {
        return getUnicodeScripts_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_es_UY(sal_Int16& count)
    {
        return getFollowPageWords_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_es_AR(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_es_UY(sal_Int16& count)
    {
        return getAllCalendars_es_AR(count);
    }
    static const sal_Unicode defaultCurrency0[] = { 1 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
    static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
    static const sal_Unicode currencyID0[] = { 0x55, 0x59, 0x55, 0x0 };
    static const sal_Unicode currencySymbol0[] = { 0x24, 0x0 };
    static const sal_Unicode bankSymbol0[] = { 0x55, 0x59, 0x55, 0x0 };
    static const sal_Unicode currencyName0[]
        = { 0x50, 0x65, 0x73, 0x6f, 0x20, 0x55, 0x72, 0x75, 0x67, 0x75, 0x61, 0x79, 0x6f, 0x0 };
    static const sal_Unicode currencyDecimalPlaces0[] = { 2 };

    static const sal_Int16 currencyCount = 1;

    static const sal_Unicode* currencies[] = {
        currencyID0,
        currencySymbol0,
        bankSymbol0,
        currencyName0,
        defaultCurrency0,
        defaultCurrencyUsedInCompatibleFormatCodes0,
        currencyDecimalPlaces0,
        defaultCurrencyLegacyOnly0,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_es_UY(sal_Int16& count)
    {
        count = currencyCount;
        return (sal_Unicode**)currencies;
    }
    extern sal_Unicode** SAL_CALL getTransliterations_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_es_UY(sal_Int16& count)
    {
        return getTransliterations_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_es_UY(sal_Int16& count)
    {
        return getForbiddenCharacters_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_es_UY(sal_Int16& count)
    {
        return getBreakIteratorRules_es_ES(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_es_ES(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_es_UY(sal_Int16& count)
    {
        return getReservedWords_es_ES(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_es_UY(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_es_UY(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
