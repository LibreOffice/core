#include <sal/types.h>


#include <stdio.h>

extern "C" {

static const sal_Unicode langID[] = {0x65, 0x6e, 0x0};
static const sal_Unicode langDefaultName[] = {0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68, 0x0};
static const sal_Unicode countryID[] = {0x49, 0x4e, 0x0};
static const sal_Unicode countryDefaultName[] = {0x49, 0x6e, 0x64, 0x69, 0x61, 0x0};
static const sal_Unicode Variant[] = {0x0};

static const sal_Unicode* LCInfoArray[] = {
    langID,
    langDefaultName,
    countryID,
    countryDefaultName,
    Variant
};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLCInfo_en_IN(sal_Int16& count)
{
    count = 0;
    return (sal_Unicode**)LCInfoArray;
}
extern sal_Unicode **  SAL_CALL getLocaleItem_en_GB(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLocaleItem_en_IN(sal_Int16& count)
{
    return getLocaleItem_en_GB(count);
}
static const sal_Unicode replaceTo0[] = {0x5b, 0x24, 0x20b9, 0x2d, 0x34, 0x30, 0x30, 0x39, 0x5d, 0x0};
extern sal_Unicode const * const * SAL_CALL getAllFormats0_en_GB(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);
SAL_DLLPUBLIC_EXPORT sal_Unicode const * const * SAL_CALL getAllFormats0_en_IN(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
{
    to = replaceTo0;
    const sal_Unicode* tmp;
    return getAllFormats0_en_GB(count, from, tmp);
}
extern sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_GB(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_IN(sal_Int16& count)
{
    return getDateAcceptancePatterns_en_GB(count);
}
extern sal_Unicode **  SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollatorImplementation_en_IN(sal_Int16& count)
{
    return getCollatorImplementation_en_US(count);
}
extern sal_Unicode **  SAL_CALL getCollationOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollationOptions_en_IN(sal_Int16& count)
{
    return getCollationOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getSearchOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getSearchOptions_en_IN(sal_Int16& count)
{
    return getSearchOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getIndexAlgorithm_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getIndexAlgorithm_en_IN(sal_Int16& count)
{
    return getIndexAlgorithm_en_US(count);
}
extern sal_Unicode **  SAL_CALL getUnicodeScripts_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getUnicodeScripts_en_IN(sal_Int16& count)
{
    return getUnicodeScripts_en_US(count);
}
extern sal_Unicode **  SAL_CALL getFollowPageWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getFollowPageWords_en_IN(sal_Int16& count)
{
    return getFollowPageWords_en_US(count);
}
static const sal_Unicode calendarID0[] = {0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode defaultCalendar0[] = {1};
static const sal_Unicode dayRef0[] = {0x72, 0x65, 0x66, 0x0};
static const sal_Unicode dayRefName0[] = {0x65, 0x6e, 0x5f, 0x55, 0x53, 0x5f, 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode monthRef0[] = {0x72, 0x65, 0x66, 0x0};
static const sal_Unicode monthRefName0[] = {0x65, 0x6e, 0x5f, 0x55, 0x53, 0x5f, 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode genitiveMonthRef0[] = {0x72, 0x65, 0x66, 0x0};
static const sal_Unicode genitiveMonthRefName0[] = {0x65, 0x6e, 0x5f, 0x55, 0x53, 0x5f, 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode partitiveMonthRef0[] = {0x72, 0x65, 0x66, 0x0};
static const sal_Unicode partitiveMonthRefName0[] = {0x65, 0x6e, 0x5f, 0x55, 0x53, 0x5f, 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode eraRef0[] = {0x72, 0x65, 0x66, 0x0};
static const sal_Unicode eraRefName0[] = {0x65, 0x6e, 0x5f, 0x55, 0x53, 0x5f, 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0};
static const sal_Unicode startDayOfWeek0[] = {0x6d, 0x6f, 0x6e, 0x0};
static const sal_Unicode minimalDaysInFirstWeek0[] = {1};
static const sal_Int16 calendarsCount = 1;

static const sal_Unicode nbOfDays[] = {0};
static const sal_Unicode nbOfMonths[] = {0};
static const sal_Unicode nbOfGenitiveMonths[] = {0};
static const sal_Unicode nbOfPartitiveMonths[] = {0};
static const sal_Unicode nbOfEras[] = {0};
static const sal_Unicode* calendars[] = {
    nbOfDays,
    nbOfMonths,
    nbOfGenitiveMonths,
    nbOfPartitiveMonths,
    nbOfEras,
    calendarID0,
    defaultCalendar0,
    dayRef0,
    dayRefName0,
    monthRef0,
    monthRefName0,
    genitiveMonthRef0,
    genitiveMonthRefName0,
    partitiveMonthRef0,
    partitiveMonthRefName0,
    eraRef0,
    eraRefName0,
    startDayOfWeek0,
    minimalDaysInFirstWeek0,
};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCalendars_en_IN(sal_Int16& count)
{
    count = calendarsCount;
    return (sal_Unicode**)calendars;
}
static const sal_Unicode defaultCurrency0[] = {1};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = {1};
static const sal_Unicode defaultCurrencyLegacyOnly0[] = {0};
static const sal_Unicode currencyID0[] = {0x49, 0x4e, 0x52, 0x0};
static const sal_Unicode currencySymbol0[] = {0x20b9, 0x0};
static const sal_Unicode bankSymbol0[] = {0x49, 0x4e, 0x52, 0x0};
static const sal_Unicode currencyName0[] = {0x52, 0x75, 0x70, 0x65, 0x65, 0x0};
static const sal_Unicode currencyDecimalPlaces0[] = {2};

static const sal_Unicode defaultCurrency1[] = {0};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes1[] = {0};
static const sal_Unicode defaultCurrencyLegacyOnly1[] = {1};
static const sal_Unicode currencyID1[] = {0x49, 0x4e, 0x52, 0x0};
static const sal_Unicode currencySymbol1[] = {0x52, 0x73, 0x2e, 0x0};
static const sal_Unicode bankSymbol1[] = {0x49, 0x4e, 0x52, 0x0};
static const sal_Unicode currencyName1[] = {0x49, 0x6e, 0x64, 0x69, 0x61, 0x6e, 0x20, 0x52, 0x75, 0x70, 0x65, 0x65, 0x0};
static const sal_Unicode currencyDecimalPlaces1[] = {2};

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

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCurrencies_en_IN(sal_Int16& count)
{
    count = currencyCount;
    return (sal_Unicode**)currencies;
}
extern sal_Unicode **  SAL_CALL getTransliterations_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getTransliterations_en_IN(sal_Int16& count)
{
    return getTransliterations_en_US(count);
}
extern sal_Unicode **  SAL_CALL getForbiddenCharacters_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getForbiddenCharacters_en_IN(sal_Int16& count)
{
    return getForbiddenCharacters_en_US(count);
}
extern sal_Unicode **  SAL_CALL getBreakIteratorRules_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getBreakIteratorRules_en_IN(sal_Int16& count)
{
    return getBreakIteratorRules_en_US(count);
}
extern sal_Unicode **  SAL_CALL getReservedWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getReservedWords_en_IN(sal_Int16& count)
{
    return getReservedWords_en_US(count);
}
// ---> ContinuousNumbering
extern const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_IN(sal_Int16& nStyles, sal_Int16& nAttributes)
{
    return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
}
// ---> OutlineNumbering
extern const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_IN(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
{
    return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
}
} // extern "C"

