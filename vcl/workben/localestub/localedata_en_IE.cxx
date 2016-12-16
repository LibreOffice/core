#include <sal/types.h>


#include <stdio.h>

extern "C" {

static const sal_Unicode langID[] = {0x65, 0x6e, 0x0};
static const sal_Unicode langDefaultName[] = {0x45, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68, 0x0};
static const sal_Unicode countryID[] = {0x49, 0x45, 0x0};
static const sal_Unicode countryDefaultName[] = {0x49, 0x72, 0x65, 0x6c, 0x61, 0x6e, 0x64, 0x0};
static const sal_Unicode Variant[] = {0x0};

static const sal_Unicode* LCInfoArray[] = {
    langID,
    langDefaultName,
    countryID,
    countryDefaultName,
    Variant
};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLCInfo_en_IE(sal_Int16& count)
{
    count = 0;
    return (sal_Unicode**)LCInfoArray;
}


static const sal_Unicode LC_CTYPE_Unoid[] = {0x0};
static const sal_Unicode dateSeparator[] = {0x2f, 0x0};
static const sal_Unicode thousandSeparator[] = {0x2c, 0x0};
static const sal_Unicode decimalSeparator[] = {0x2e, 0x0};
static const sal_Unicode timeSeparator[] = {0x3a, 0x0};
static const sal_Unicode time100SecSeparator[] = {0x2e, 0x0};
static const sal_Unicode listSeparator[] = {0x3b, 0x0};
static const sal_Unicode LongDateDayOfWeekSeparator[] = {0x20, 0x0};
static const sal_Unicode LongDateDaySeparator[] = {0x20, 0x0};
static const sal_Unicode LongDateMonthSeparator[] = {0x20, 0x0};
static const sal_Unicode LongDateYearSeparator[] = {0x20, 0x0};
static const sal_Unicode quotationStart[] = {0x2018, 0x0};
static const sal_Unicode quotationEnd[] = {0x2019, 0x0};
static const sal_Unicode doubleQuotationStart[] = {0x201c, 0x0};
static const sal_Unicode doubleQuotationEnd[] = {0x201d, 0x0};
static const sal_Unicode timeAM[] = {0x61, 0x2e, 0x6d, 0x2e, 0x0};
static const sal_Unicode timePM[] = {0x70, 0x2e, 0x6d, 0x2e, 0x0};
static const sal_Unicode measurementSystem[] = {0x6d, 0x65, 0x74, 0x72, 0x69, 0x63, 0x0};

static const sal_Unicode* LCType[] = {
    LC_CTYPE_Unoid,
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
    LongDateYearSeparator
};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLocaleItem_en_IE(sal_Int16& count)
{
    count = 0;
    return (sal_Unicode**)LCType;
}
static const sal_Unicode replaceTo0[] = {0x5b, 0x24, 0x49, 0x52, 0xa3, 0x2d, 0x31, 0x38, 0x30, 0x39, 0x5d, 0x0};
extern sal_Unicode const * const * SAL_CALL getAllFormats0_en_GB(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to);
SAL_DLLPUBLIC_EXPORT sal_Unicode const * const * SAL_CALL getAllFormats0_en_IE(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
{
    to = replaceTo0;
    const sal_Unicode* tmp;
    return getAllFormats0_en_GB(count, from, tmp);
}
extern sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_GB(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getDateAcceptancePatterns_en_IE(sal_Int16& count)
{
    return getDateAcceptancePatterns_en_GB(count);
}
extern sal_Unicode **  SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollatorImplementation_en_IE(sal_Int16& count)
{
    return getCollatorImplementation_en_US(count);
}
extern sal_Unicode **  SAL_CALL getCollationOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollationOptions_en_IE(sal_Int16& count)
{
    return getCollationOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getSearchOptions_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getSearchOptions_en_IE(sal_Int16& count)
{
    return getSearchOptions_en_US(count);
}
extern sal_Unicode **  SAL_CALL getIndexAlgorithm_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getIndexAlgorithm_en_IE(sal_Int16& count)
{
    return getIndexAlgorithm_en_US(count);
}
extern sal_Unicode **  SAL_CALL getUnicodeScripts_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getUnicodeScripts_en_IE(sal_Int16& count)
{
    return getUnicodeScripts_en_US(count);
}
extern sal_Unicode **  SAL_CALL getFollowPageWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getFollowPageWords_en_IE(sal_Int16& count)
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
static const sal_Unicode startDayOfWeek0[] = {0x73, 0x75, 0x6e, 0x0};
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

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCalendars_en_IE(sal_Int16& count)
{
    count = calendarsCount;
    return (sal_Unicode**)calendars;
}
static const sal_Unicode defaultCurrency0[] = {1};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = {0};
static const sal_Unicode defaultCurrencyLegacyOnly0[] = {0};
static const sal_Unicode currencyID0[] = {0x45, 0x55, 0x52, 0x0};
static const sal_Unicode currencySymbol0[] = {0x20ac, 0x0};
static const sal_Unicode bankSymbol0[] = {0x45, 0x55, 0x52, 0x0};
static const sal_Unicode currencyName0[] = {0x45, 0x75, 0x72, 0x6f, 0x0};
static const sal_Unicode currencyDecimalPlaces0[] = {2};

static const sal_Unicode defaultCurrency1[] = {0};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes1[] = {1};
static const sal_Unicode defaultCurrencyLegacyOnly1[] = {0};
static const sal_Unicode currencyID1[] = {0x49, 0x45, 0x50, 0x0};
static const sal_Unicode currencySymbol1[] = {0x49, 0x52, 0xa3, 0x0};
static const sal_Unicode bankSymbol1[] = {0x49, 0x45, 0x50, 0x0};
static const sal_Unicode currencyName1[] = {0x49, 0x72, 0x69, 0x73, 0x68, 0x20, 0x50, 0x6f, 0x75, 0x6e, 0x64, 0x0};
static const sal_Unicode currencyDecimalPlaces1[] = {2};

static const sal_Unicode defaultCurrency2[] = {0};
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes2[] = {0};
static const sal_Unicode defaultCurrencyLegacyOnly2[] = {0};
static const sal_Unicode currencyID2[] = {0x47, 0x42, 0x50, 0x0};
static const sal_Unicode currencySymbol2[] = {0xa3, 0x0};
static const sal_Unicode bankSymbol2[] = {0x47, 0x42, 0x50, 0x0};
static const sal_Unicode currencyName2[] = {0x42, 0x72, 0x69, 0x74, 0x69, 0x73, 0x68, 0x20, 0x50, 0x6f, 0x75, 0x6e, 0x64, 0x20, 0x53, 0x74, 0x65, 0x72, 0x6c, 0x69, 0x6e, 0x67, 0x0};
static const sal_Unicode currencyDecimalPlaces2[] = {2};

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

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCurrencies_en_IE(sal_Int16& count)
{
    count = currencyCount;
    return (sal_Unicode**)currencies;
}
extern sal_Unicode **  SAL_CALL getTransliterations_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getTransliterations_en_IE(sal_Int16& count)
{
    return getTransliterations_en_US(count);
}
extern sal_Unicode **  SAL_CALL getForbiddenCharacters_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getForbiddenCharacters_en_IE(sal_Int16& count)
{
    return getForbiddenCharacters_en_US(count);
}
extern sal_Unicode **  SAL_CALL getBreakIteratorRules_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getBreakIteratorRules_en_IE(sal_Int16& count)
{
    return getBreakIteratorRules_en_US(count);
}
extern sal_Unicode **  SAL_CALL getReservedWords_en_US(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getReservedWords_en_IE(sal_Int16& count)
{
    return getReservedWords_en_US(count);
}
// ---> ContinuousNumbering
extern const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_en_IE(sal_Int16& nStyles, sal_Int16& nAttributes)
{
    return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
}
// ---> OutlineNumbering
extern const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes);
SAL_DLLPUBLIC_EXPORT const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_en_IE(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
{
    return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
}
} // extern "C"

