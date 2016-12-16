#include <sal/types.h>


#include <stdio.h>

extern "C" {

static const sal_Unicode langID[] = {0x65, 0x6e, 0x0};
static const sal_Unicode langDefaultName[] = {0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68, 0x0};
static const sal_Unicode countryID[] = {0x50, 0x48, 0x0};
static const sal_Unicode countryDefaultName[] = {0x50, 0x68, 0x69, 0x6c, 0x69, 0x70, 0x70, 0x69, 0x6e, 0x65, 0x73, 0x0};
static const sal_Unicode Variant[] = {0x0};

static const sal_Unicode* LCInfoArray[] = {
    langID,
    langDefaultName,
    countryID,
    countryDefaultName,
    Variant
};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLCInfo_en_PH(sal_Int16& count)
{
    count = 0;
    return (sal_Unicode**)LCInfoArray;
}
extern sal_Unicode **  SAL_CALL getLocaleItem_en_CA(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLocaleItem_en_PH(sal_Int16& count)
{
    return getLocaleItem_en_CA(count);
}
static const sal_Unicode replaceTo0[] = {0x5b, 0x24, 0x50, 0x68, 0x70, 0x2d, 0x33, 0x34, 0x30, 0x39, 0x5d, 0x0};
extern sal_Unicode const * const * SAL_CALL getAllFormats0_en_US(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);
SAL_DLLPUBLIC_EXPORT sal_Unicode const * const * SAL_CALL getAllFormats0_en_PH(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
{
    to = replaceTo0;
    const sal_Unicode* tmp;
    return getAllFormats0_en_US(count, from, tmp);
}
extern sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_PH(sal_Int16& count)
{
    return getDateAcceptancePatterns_en_US(count);
}
extern sal_Unicode **  SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollatorImplementation_en_PH(sal_Int16& count)
{
    return getCollatorImplementation_en_US(count);
}
extern sal_Unicode **  SAL_CALL getCollationOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollationOptions_en_PH(sal_Int16& count)
{
    return getCollationOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getSearchOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getSearchOptions_en_PH(sal_Int16& count)
{
    return getSearchOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getIndexAlgorithm_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getIndexAlgorithm_en_PH(sal_Int16& count)
{
    return getIndexAlgorithm_en_US(count);
}
extern sal_Unicode **  SAL_CALL getUnicodeScripts_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getUnicodeScripts_en_PH(sal_Int16& count)
{
    return getUnicodeScripts_en_US(count);
}
extern sal_Unicode **  SAL_CALL getFollowPageWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getFollowPageWords_en_PH(sal_Int16& count)
{
    return getFollowPageWords_en_US(count);
}
extern sal_Unicode **  SAL_CALL getAllCalendars_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCalendars_en_PH(sal_Int16& count)
{
    return getAllCalendars_en_US(count);
}
static const sal_Unicode defaultCurrency0[] = {1};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = {1};
static const sal_Unicode defaultCurrencyLegacyOnly0[] = {0};
static const sal_Unicode currencyID0[] = {0x50, 0x48, 0x50, 0x0};
static const sal_Unicode currencySymbol0[] = {0x50, 0x68, 0x70, 0x0};
static const sal_Unicode bankSymbol0[] = {0x50, 0x48, 0x50, 0x0};
static const sal_Unicode currencyName0[] = {0x50, 0x65, 0x73, 0x6f, 0x0};
static const sal_Unicode currencyDecimalPlaces0[] = {2};

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

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCurrencies_en_PH(sal_Int16& count)
{
    count = currencyCount;
    return (sal_Unicode**)currencies;
}
extern sal_Unicode **  SAL_CALL getTransliterations_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getTransliterations_en_PH(sal_Int16& count)
{
    return getTransliterations_en_US(count);
}
extern sal_Unicode **  SAL_CALL getForbiddenCharacters_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getForbiddenCharacters_en_PH(sal_Int16& count)
{
    return getForbiddenCharacters_en_US(count);
}
extern sal_Unicode **  SAL_CALL getBreakIteratorRules_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getBreakIteratorRules_en_PH(sal_Int16& count)
{
    return getBreakIteratorRules_en_US(count);
}
extern sal_Unicode **  SAL_CALL getReservedWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getReservedWords_en_PH(sal_Int16& count)
{
    return getReservedWords_en_US(count);
}
// ---> ContinuousNumbering
extern const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_PH(sal_Int16& nStyles, sal_Int16& nAttributes)
{
    return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
}
// ---> OutlineNumbering
extern const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_PH(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
{
    return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
}
} // extern "C"

