#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x6e, 0x6e, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x4e, 0x6f, 0x72, 0x77, 0x65, 0x67, 0x69, 0x61, 0x6e,
            0x20, 0x4e, 0x79, 0x6e, 0x6f, 0x72, 0x73, 0x6b, 0x0 };
    static const sal_Unicode countryID[] = { 0x4e, 0x4f, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x4e, 0x6f, 0x72, 0x77, 0x61, 0x79, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_nn_NO(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_nn_NO(sal_Int16& count)
    {
        return getLocaleItem_no_NO(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x6b, 0x72, 0x2d, 0x38, 0x31, 0x34, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_no_NO(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_nn_NO(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_no_NO(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_nn_NO(sal_Int16& count)
    {
        return getDateAcceptancePatterns_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_nn_NO(sal_Int16& count)
    {
        return getCollatorImplementation_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_nn_NO(sal_Int16& count)
    {
        return getCollationOptions_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_nn_NO(sal_Int16& count)
    {
        return getSearchOptions_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_nn_NO(sal_Int16& count)
    {
        return getIndexAlgorithm_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_nn_NO(sal_Int16& count)
    {
        return getUnicodeScripts_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_nn_NO(sal_Int16& count)
    {
        return getFollowPageWords_no_NO(count);
    }
    static const sal_Unicode calendarID0[]
        = { 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode defaultCalendar0[] = { 1 };
    static const sal_Unicode dayID00[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName00[] = { 0x73, 0x75, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName00[] = { 0x73, 0x75, 0x6e, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName00[] = { 0x73, 0x0 };
    static const sal_Unicode dayID01[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName01[] = { 0x6d, 0xe5, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName01[] = { 0x6d, 0xe5, 0x6e, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName01[] = { 0x6d, 0x0 };
    static const sal_Unicode dayID02[] = { 0x74, 0x75, 0x65, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName02[] = { 0x74, 0x79, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName02[] = { 0x74, 0x79, 0x73, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName02[] = { 0x74, 0x0 };
    static const sal_Unicode dayID03[] = { 0x77, 0x65, 0x64, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName03[] = { 0x6f, 0x6e, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName03[] = { 0x6f, 0x6e, 0x73, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName03[] = { 0x6f, 0x0 };
    static const sal_Unicode dayID04[] = { 0x74, 0x68, 0x75, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName04[] = { 0x74, 0x6f, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName04[]
        = { 0x74, 0x6f, 0x72, 0x73, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName04[] = { 0x74, 0x0 };
    static const sal_Unicode dayID05[] = { 0x66, 0x72, 0x69, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName05[] = { 0x66, 0x72, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName05[] = { 0x66, 0x72, 0x65, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName05[] = { 0x66, 0x0 };
    static const sal_Unicode dayID06[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName06[] = { 0x6c, 0x61, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName06[]
        = { 0x6c, 0x61, 0x75, 0x72, 0x64, 0x61, 0x67, 0x0 };
    static const sal_Unicode dayDefaultNarrowName06[] = { 0x6c, 0x0 };
    static const sal_Unicode monthRef0[] = { 0x72, 0x65, 0x66, 0x0 };
    static const sal_Unicode monthRefName0[] = { 0x6e, 0x6f, 0x5f, 0x4e, 0x4f, 0x5f, 0x67, 0x72,
                                                 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthRef0[] = { 0x72, 0x65, 0x66, 0x0 };
    static const sal_Unicode genitiveMonthRefName0[]
        = { 0x6e, 0x6f, 0x5f, 0x4e, 0x4f, 0x5f, 0x67, 0x72,
            0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthRef0[] = { 0x72, 0x65, 0x66, 0x0 };
    static const sal_Unicode partitiveMonthRefName0[]
        = { 0x6e, 0x6f, 0x5f, 0x4e, 0x4f, 0x5f, 0x67, 0x72,
            0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode eraRef0[] = { 0x72, 0x65, 0x66, 0x0 };
    static const sal_Unicode eraRefName0[] = { 0x6e, 0x6f, 0x5f, 0x4e, 0x4f, 0x5f, 0x67, 0x72,
                                               0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode startDayOfWeek0[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode minimalDaysInFirstWeek0[] = { 4 };
    static const sal_Int16 calendarsCount = 1;

    static const sal_Unicode nbOfDays[] = { 7 };
    static const sal_Unicode nbOfMonths[] = { 0 };
    static const sal_Unicode nbOfGenitiveMonths[] = { 0 };
    static const sal_Unicode nbOfPartitiveMonths[] = { 0 };
    static const sal_Unicode nbOfEras[] = { 0 };
    static const sal_Unicode* calendars[] = {
        nbOfDays,
        nbOfMonths,
        nbOfGenitiveMonths,
        nbOfPartitiveMonths,
        nbOfEras,
        calendarID0,
        defaultCalendar0,
        dayID00,
        dayDefaultAbbrvName00,
        dayDefaultFullName00,
        dayDefaultNarrowName00,
        dayID01,
        dayDefaultAbbrvName01,
        dayDefaultFullName01,
        dayDefaultNarrowName01,
        dayID02,
        dayDefaultAbbrvName02,
        dayDefaultFullName02,
        dayDefaultNarrowName02,
        dayID03,
        dayDefaultAbbrvName03,
        dayDefaultFullName03,
        dayDefaultNarrowName03,
        dayID04,
        dayDefaultAbbrvName04,
        dayDefaultFullName04,
        dayDefaultNarrowName04,
        dayID05,
        dayDefaultAbbrvName05,
        dayDefaultFullName05,
        dayDefaultNarrowName05,
        dayID06,
        dayDefaultAbbrvName06,
        dayDefaultFullName06,
        dayDefaultNarrowName06,
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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_nn_NO(sal_Int16& count)
    {
        count = calendarsCount;
        return (sal_Unicode**)calendars;
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_nn_NO(sal_Int16& count)
    {
        return getAllCurrencies_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_nn_NO(sal_Int16& count)
    {
        return getTransliterations_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_nn_NO(sal_Int16& count)
    {
        return getForbiddenCharacters_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_nn_NO(sal_Int16& count)
    {
        return getBreakIteratorRules_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_nn_NO(sal_Int16& count)
    {
        return getReservedWords_no_NO(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_no_NO(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_nn_NO(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_no_NO(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_no_NO(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_nn_NO(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_no_NO(nStyles, nLevels, nAttributes);
    }
} // extern "C"
