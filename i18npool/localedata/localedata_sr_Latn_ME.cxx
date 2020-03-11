#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x71, 0x6c, 0x74, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x53, 0x65, 0x72, 0x62, 0x69, 0x61, 0x6e, 0x20, 0x4c, 0x61, 0x74, 0x69, 0x6e, 0x0 };
    static const sal_Unicode countryID[] = { 0x4d, 0x45, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x53, 0x65, 0x72, 0x62, 0x69, 0x61, 0x0 };
    static const sal_Unicode Variant[]
        = { 0x73, 0x72, 0x2d, 0x4c, 0x61, 0x74, 0x6e, 0x2d, 0x4d, 0x45, 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_sr_Latn_ME(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_sr_Latn_ME(sal_Int16& count)
    {
        return getLocaleItem_sr_Latn_RS(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x20ac, 0x2d, 0x43, 0x38, 0x31, 0x41, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_sr_Latn_RS(sal_Int16& count,
                                                                        const sal_Unicode*& from,
                                                                        const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_sr_Latn_ME(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_sr_Latn_RS(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getDateAcceptancePatterns_sr_Latn_ME(sal_Int16& count)
    {
        return getDateAcceptancePatterns_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL
    getCollatorImplementation_sr_Latn_ME(sal_Int16& count)
    {
        return getCollatorImplementation_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_sr_Latn_ME(sal_Int16& count)
    {
        return getCollationOptions_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_sr_Latn_ME(sal_Int16& count)
    {
        return getSearchOptions_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_sr_Latn_ME(sal_Int16& count)
    {
        return getIndexAlgorithm_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_sr_Latn_ME(sal_Int16& count)
    {
        return getUnicodeScripts_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_sr_Latn_ME(sal_Int16& count)
    {
        return getFollowPageWords_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_sr_Latn_ME(sal_Int16& count)
    {
        return getAllCalendars_sr_Latn_RS(count);
    }
    static const sal_Unicode defaultCurrency0[] = { 1 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
    static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
    static const sal_Unicode currencyID0[] = { 0x45, 0x55, 0x52, 0x0 };
    static const sal_Unicode currencySymbol0[] = { 0x20ac, 0x0 };
    static const sal_Unicode bankSymbol0[] = { 0x45, 0x55, 0x52, 0x0 };
    static const sal_Unicode currencyName0[] = { 0x45, 0x75, 0x72, 0x6f, 0x0 };
    static const sal_Unicode currencyDecimalPlaces0[] = { 2 };

    static const sal_Unicode defaultCurrency1[] = { 0 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes1[] = { 0 };
    static const sal_Unicode defaultCurrencyLegacyOnly1[] = { 0 };
    static const sal_Unicode currencyID1[] = { 0x44, 0x45, 0x4d, 0x0 };
    static const sal_Unicode currencySymbol1[] = { 0x44, 0x4d, 0x0 };
    static const sal_Unicode bankSymbol1[] = { 0x44, 0x45, 0x4d, 0x0 };
    static const sal_Unicode currencyName1[]
        = { 0x44, 0x65, 0x75, 0x74, 0x73, 0x63, 0x68, 0x65, 0x20, 0x4d, 0x61, 0x72, 0x6b, 0x0 };
    static const sal_Unicode currencyDecimalPlaces1[] = { 2 };

    static const sal_Int16 currencyCount = 2;

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
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_sr_Latn_ME(sal_Int16& count)
    {
        count = currencyCount;
        return (sal_Unicode**)currencies;
    }
    extern sal_Unicode** SAL_CALL getTransliterations_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_sr_Latn_ME(sal_Int16& count)
    {
        return getTransliterations_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_sr_Latn_ME(sal_Int16& count)
    {
        return getForbiddenCharacters_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_sr_Latn_ME(sal_Int16& count)
    {
        return getBreakIteratorRules_sr_Latn_RS(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_sr_Latn_RS(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_sr_Latn_ME(sal_Int16& count)
    {
        return getReservedWords_sr_Latn_RS(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_sr_Latn_RS(sal_Int16& nStyles, sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_sr_Latn_ME(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_sr_Latn_RS(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_sr_Latn_RS(
        sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_sr_Latn_ME(
        sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_sr_Latn_RS(nStyles, nLevels, nAttributes);
    }
} // extern "C"
