#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x61, 0x72, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x41, 0x72, 0x61, 0x62, 0x69, 0x63, 0x0 };
    static const sal_Unicode countryID[] = { 0x54, 0x4e, 0x0 };
    static const sal_Unicode countryDefaultName[]
        = { 0x54, 0x75, 0x6e, 0x69, 0x73, 0x69, 0x61, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_ar_TN(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }

    static const sal_Unicode LC_CTYPE_Unoid[] = { 0x0 };
    static const sal_Unicode dateSeparator[] = { 0x2f, 0x0 };
    static const sal_Unicode thousandSeparator[] = { 0x66c, 0x0 };
    static const sal_Unicode decimalSeparator[] = { 0x66b, 0x0 };
    static const sal_Unicode decimalSeparatorAlternative[] = { 0x2e, 0x0 };
    static const sal_Unicode timeSeparator[] = { 0x3a, 0x0 };
    static const sal_Unicode time100SecSeparator[] = { 0x2e, 0x0 };
    static const sal_Unicode listSeparator[] = { 0x3b, 0x0 };
    static const sal_Unicode LongDateDayOfWeekSeparator[] = { 0x60c, 0x20, 0x0 };
    static const sal_Unicode LongDateDaySeparator[] = { 0x60c, 0x20, 0x0 };
    static const sal_Unicode LongDateMonthSeparator[] = { 0x60c, 0x20, 0x0 };
    static const sal_Unicode LongDateYearSeparator[] = { 0x20, 0x0 };
    static const sal_Unicode quotationStart[] = { 0x2018, 0x0 };
    static const sal_Unicode quotationEnd[] = { 0x2019, 0x0 };
    static const sal_Unicode doubleQuotationStart[] = { 0x22, 0x0 };
    static const sal_Unicode doubleQuotationEnd[] = { 0x22, 0x0 };
    static const sal_Unicode timeAM[] = { 0x635, 0x0 };
    static const sal_Unicode timePM[] = { 0x645, 0x0 };
    static const sal_Unicode measurementSystem[] = { 0x6d, 0x65, 0x74, 0x72, 0x69, 0x63, 0x0 };

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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_ar_TN(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCType);
        return (sal_Unicode**)LCType;
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x62f, 0x2e, 0x62a, 0x2e, 0x200f, 0x2d, 0x31, 0x43, 0x30, 0x31, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_ar_EG(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_ar_TN(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_ar_EG(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_ar_TN(sal_Int16& count)
    {
        return getDateAcceptancePatterns_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_ar_TN(sal_Int16& count)
    {
        return getCollatorImplementation_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_ar_TN(sal_Int16& count)
    {
        return getCollationOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_ar_TN(sal_Int16& count)
    {
        return getSearchOptions_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_ar_TN(sal_Int16& count)
    {
        return getIndexAlgorithm_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_ar_TN(sal_Int16& count)
    {
        return getUnicodeScripts_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_ar_TN(sal_Int16& count)
    {
        return getFollowPageWords_ar_EG(count);
    }
    static const sal_Unicode calendarID0[]
        = { 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode defaultCalendar0[] = { 1 };
    static const sal_Unicode dayID00[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName00[] = { 0x627, 0x644, 0x623, 0x62d, 0x62f, 0x0 };
    static const sal_Unicode dayDefaultFullName00[] = { 0x627, 0x644, 0x623, 0x62d, 0x62f, 0x0 };
    static const sal_Unicode dayDefaultNarrowName00[] = { 0x627, 0x0 };
    static const sal_Unicode dayID01[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName01[]
        = { 0x627, 0x644, 0x627, 0x62b, 0x646, 0x64a, 0x646, 0x0 };
    static const sal_Unicode dayDefaultFullName01[]
        = { 0x627, 0x644, 0x627, 0x62b, 0x646, 0x64a, 0x646, 0x0 };
    static const sal_Unicode dayDefaultNarrowName01[] = { 0x627, 0x0 };
    static const sal_Unicode dayID02[] = { 0x74, 0x75, 0x65, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName02[]
        = { 0x627, 0x644, 0x62b, 0x644, 0x627, 0x62b, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultFullName02[]
        = { 0x627, 0x644, 0x62b, 0x644, 0x627, 0x62b, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultNarrowName02[] = { 0x627, 0x0 };
    static const sal_Unicode dayID03[] = { 0x77, 0x65, 0x64, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName03[]
        = { 0x627, 0x644, 0x623, 0x631, 0x628, 0x639, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultFullName03[]
        = { 0x627, 0x644, 0x623, 0x631, 0x628, 0x639, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultNarrowName03[] = { 0x627, 0x0 };
    static const sal_Unicode dayID04[] = { 0x74, 0x68, 0x75, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName04[]
        = { 0x627, 0x644, 0x62e, 0x645, 0x64a, 0x633, 0x0 };
    static const sal_Unicode dayDefaultFullName04[]
        = { 0x627, 0x644, 0x62e, 0x645, 0x64a, 0x633, 0x0 };
    static const sal_Unicode dayDefaultNarrowName04[] = { 0x627, 0x0 };
    static const sal_Unicode dayID05[] = { 0x66, 0x72, 0x69, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName05[]
        = { 0x627, 0x644, 0x62c, 0x645, 0x639, 0x629, 0x0 };
    static const sal_Unicode dayDefaultFullName05[]
        = { 0x627, 0x644, 0x62c, 0x645, 0x639, 0x629, 0x0 };
    static const sal_Unicode dayDefaultNarrowName05[] = { 0x627, 0x0 };
    static const sal_Unicode dayID06[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName06[] = { 0x627, 0x644, 0x633, 0x628, 0x62a, 0x0 };
    static const sal_Unicode dayDefaultFullName06[] = { 0x627, 0x644, 0x633, 0x628, 0x62a, 0x0 };
    static const sal_Unicode dayDefaultNarrowName06[] = { 0x627, 0x0 };
    static const sal_Unicode monthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName00[] = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName00[] = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName00[] = { 0x64a, 0x0 };
    static const sal_Unicode monthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName01[] = { 0x641, 0x0 };
    static const sal_Unicode monthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName02[] = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode monthDefaultFullName02[] = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode monthDefaultNarrowName02[] = { 0x645, 0x0 };
    static const sal_Unicode monthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName03[] = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode monthDefaultFullName03[] = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode monthDefaultNarrowName03[] = { 0x623, 0x0 };
    static const sal_Unicode monthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName04[] = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultFullName04[] = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultNarrowName04[] = { 0x645, 0x0 };
    static const sal_Unicode monthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName05[] = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultFullName05[] = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultNarrowName05[] = { 0x64a, 0x0 };
    static const sal_Unicode monthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName06[] = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultFullName06[] = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode monthDefaultNarrowName06[] = { 0x64a, 0x0 };
    static const sal_Unicode monthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName07[] = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode monthDefaultFullName07[] = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode monthDefaultNarrowName07[] = { 0x623, 0x0 };
    static const sal_Unicode monthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName08[] = { 0x633, 0x0 };
    static const sal_Unicode monthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName09[] = { 0x623, 0x0 };
    static const sal_Unicode monthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName010[] = { 0x646, 0x0 };
    static const sal_Unicode monthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName011[] = { 0x62f, 0x0 };
    static const sal_Unicode genitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName00[]
        = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName00[]
        = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName00[] = { 0x64a, 0x0 };
    static const sal_Unicode genitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName01[] = { 0x641, 0x0 };
    static const sal_Unicode genitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName02[]
        = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName02[] = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName02[] = { 0x645, 0x0 };
    static const sal_Unicode genitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName03[]
        = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName03[]
        = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName03[] = { 0x623, 0x0 };
    static const sal_Unicode genitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName04[]
        = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName04[] = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName04[] = { 0x645, 0x0 };
    static const sal_Unicode genitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName05[]
        = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName05[]
        = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName05[] = { 0x64a, 0x0 };
    static const sal_Unicode genitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName06[]
        = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName06[]
        = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName06[] = { 0x64a, 0x0 };
    static const sal_Unicode genitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName07[]
        = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName07[]
        = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName07[] = { 0x623, 0x0 };
    static const sal_Unicode genitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName08[] = { 0x633, 0x0 };
    static const sal_Unicode genitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName09[] = { 0x623, 0x0 };
    static const sal_Unicode genitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName010[] = { 0x646, 0x0 };
    static const sal_Unicode genitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName011[] = { 0x62f, 0x0 };
    static const sal_Unicode partitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName00[]
        = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName00[]
        = { 0x64a, 0x646, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName00[] = { 0x64a, 0x0 };
    static const sal_Unicode partitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName01[]
        = { 0x641, 0x628, 0x631, 0x627, 0x64a, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName01[] = { 0x641, 0x0 };
    static const sal_Unicode partitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName02[]
        = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName02[]
        = { 0x645, 0x627, 0x631, 0x633, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName02[] = { 0x645, 0x0 };
    static const sal_Unicode partitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName03[]
        = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName03[]
        = { 0x623, 0x628, 0x631, 0x64a, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName03[] = { 0x623, 0x0 };
    static const sal_Unicode partitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName04[]
        = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName04[]
        = { 0x645, 0x627, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName04[] = { 0x645, 0x0 };
    static const sal_Unicode partitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName05[]
        = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName05[]
        = { 0x64a, 0x648, 0x646, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName05[] = { 0x64a, 0x0 };
    static const sal_Unicode partitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName06[]
        = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName06[]
        = { 0x64a, 0x648, 0x644, 0x64a, 0x648, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName06[] = { 0x64a, 0x0 };
    static const sal_Unicode partitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName07[]
        = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName07[]
        = { 0x623, 0x63a, 0x633, 0x637, 0x633, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName07[] = { 0x623, 0x0 };
    static const sal_Unicode partitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName08[]
        = { 0x633, 0x628, 0x62a, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName08[] = { 0x633, 0x0 };
    static const sal_Unicode partitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName09[]
        = { 0x623, 0x643, 0x62a, 0x648, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName09[] = { 0x623, 0x0 };
    static const sal_Unicode partitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName010[]
        = { 0x646, 0x648, 0x641, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName010[] = { 0x646, 0x0 };
    static const sal_Unicode partitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName011[]
        = { 0x62f, 0x64a, 0x633, 0x645, 0x628, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName011[] = { 0x62f, 0x0 };
    static const sal_Unicode eraID00[] = { 0x62, 0x63, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName00[] = { 0x642, 0x2e, 0x645, 0x0 };
    static const sal_Unicode eraDefaultFullName00[] = { 0x642, 0x2e, 0x645, 0x0 };
    static const sal_Unicode eraID01[] = { 0x61, 0x64, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName01[] = { 0x645, 0x0 };
    static const sal_Unicode eraDefaultFullName01[] = { 0x645, 0x0 };
    static const sal_Unicode startDayOfWeek0[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode minimalDaysInFirstWeek0[] = { 1 };
    static const sal_Unicode calendarID1[] = { 0x68, 0x69, 0x6a, 0x72, 0x69, 0x0 };
    static const sal_Unicode defaultCalendar1[] = { 0 };
    static const sal_Unicode dayID10[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName10[] = { 0x627, 0x644, 0x623, 0x62d, 0x62f, 0x0 };
    static const sal_Unicode dayDefaultFullName10[] = { 0x627, 0x644, 0x623, 0x62d, 0x62f, 0x0 };
    static const sal_Unicode dayDefaultNarrowName10[] = { 0x627, 0x0 };
    static const sal_Unicode dayID11[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName11[]
        = { 0x627, 0x644, 0x627, 0x62b, 0x646, 0x64a, 0x646, 0x0 };
    static const sal_Unicode dayDefaultFullName11[]
        = { 0x627, 0x644, 0x627, 0x62b, 0x646, 0x64a, 0x646, 0x0 };
    static const sal_Unicode dayDefaultNarrowName11[] = { 0x627, 0x0 };
    static const sal_Unicode dayID12[] = { 0x74, 0x75, 0x65, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName12[]
        = { 0x627, 0x644, 0x62b, 0x644, 0x627, 0x62b, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultFullName12[]
        = { 0x627, 0x644, 0x62b, 0x644, 0x627, 0x62b, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultNarrowName12[] = { 0x627, 0x0 };
    static const sal_Unicode dayID13[] = { 0x77, 0x65, 0x64, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName13[]
        = { 0x627, 0x644, 0x623, 0x631, 0x628, 0x639, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultFullName13[]
        = { 0x627, 0x644, 0x623, 0x631, 0x628, 0x639, 0x627, 0x621, 0x0 };
    static const sal_Unicode dayDefaultNarrowName13[] = { 0x627, 0x0 };
    static const sal_Unicode dayID14[] = { 0x74, 0x68, 0x75, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName14[]
        = { 0x627, 0x644, 0x62e, 0x645, 0x64a, 0x633, 0x0 };
    static const sal_Unicode dayDefaultFullName14[]
        = { 0x627, 0x644, 0x62e, 0x645, 0x64a, 0x633, 0x0 };
    static const sal_Unicode dayDefaultNarrowName14[] = { 0x627, 0x0 };
    static const sal_Unicode dayID15[] = { 0x66, 0x72, 0x69, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName15[]
        = { 0x627, 0x644, 0x62c, 0x645, 0x639, 0x629, 0x0 };
    static const sal_Unicode dayDefaultFullName15[]
        = { 0x627, 0x644, 0x62c, 0x645, 0x639, 0x629, 0x0 };
    static const sal_Unicode dayDefaultNarrowName15[] = { 0x627, 0x0 };
    static const sal_Unicode dayID16[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName16[] = { 0x627, 0x644, 0x633, 0x628, 0x62a, 0x0 };
    static const sal_Unicode dayDefaultFullName16[] = { 0x627, 0x644, 0x633, 0x628, 0x62a, 0x0 };
    static const sal_Unicode dayDefaultNarrowName16[] = { 0x627, 0x0 };
    static const sal_Unicode monthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName10[] = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode monthDefaultFullName10[] = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode monthDefaultNarrowName10[] = { 0x645, 0x0 };
    static const sal_Unicode monthID11[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName11[] = { 0x635, 0x0 };
    static const sal_Unicode monthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode monthDefaultFullName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode monthDefaultNarrowName12[] = { 0x631, 0x0 };
    static const sal_Unicode monthID13[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode monthDefaultFullName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode monthDefaultNarrowName13[] = { 0x631, 0x0 };
    static const sal_Unicode monthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName14[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                           0x20,  0x627, 0x644, 0x623, 0x648,
                                                           0x644, 0x649, 0x0 };
    static const sal_Unicode monthDefaultFullName14[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                          0x20,  0x627, 0x644, 0x623, 0x648,
                                                          0x644, 0x649, 0x0 };
    static const sal_Unicode monthDefaultNarrowName14[] = { 0x62c, 0x0 };
    static const sal_Unicode monthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName15[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                           0x20,  0x627, 0x644, 0x622, 0x62e,
                                                           0x631, 0x629, 0x0 };
    static const sal_Unicode monthDefaultFullName15[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                          0x20,  0x627, 0x644, 0x622, 0x62e,
                                                          0x631, 0x629, 0x0 };
    static const sal_Unicode monthDefaultNarrowName15[] = { 0x62c, 0x0 };
    static const sal_Unicode monthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode monthDefaultFullName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode monthDefaultNarrowName16[] = { 0x631, 0x0 };
    static const sal_Unicode monthID17[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName17[] = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode monthDefaultFullName17[] = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode monthDefaultNarrowName17[] = { 0x634, 0x0 };
    static const sal_Unicode monthID18[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName18[] = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode monthDefaultFullName18[] = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode monthDefaultNarrowName18[] = { 0x631, 0x0 };
    static const sal_Unicode monthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName19[] = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode monthDefaultFullName19[] = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode monthDefaultNarrowName19[] = { 0x634, 0x0 };
    static const sal_Unicode monthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode monthDefaultFullName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode monthDefaultNarrowName110[] = { 0x630, 0x0 };
    static const sal_Unicode monthID111[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode monthDefaultFullName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode monthDefaultNarrowName111[] = { 0x630, 0x0 };
    static const sal_Unicode genitiveMonthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName10[]
        = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName10[] = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName10[] = { 0x645, 0x0 };
    static const sal_Unicode genitiveMonthID11[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName11[] = { 0x635, 0x0 };
    static const sal_Unicode genitiveMonthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName12[] = { 0x631, 0x0 };
    static const sal_Unicode genitiveMonthID13[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName13[] = { 0x631, 0x0 };
    static const sal_Unicode genitiveMonthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName14[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x649, 0x0
    };
    static const sal_Unicode genitiveMonthDefaultFullName14[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                                  0x20,  0x627, 0x644, 0x623, 0x648,
                                                                  0x644, 0x649, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName14[] = { 0x62c, 0x0 };
    static const sal_Unicode genitiveMonthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName15[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x629, 0x0
    };
    static const sal_Unicode genitiveMonthDefaultFullName15[] = { 0x62c, 0x645, 0x627, 0x62f, 0x649,
                                                                  0x20,  0x627, 0x644, 0x622, 0x62e,
                                                                  0x631, 0x629, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName15[] = { 0x62c, 0x0 };
    static const sal_Unicode genitiveMonthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName16[] = { 0x631, 0x0 };
    static const sal_Unicode genitiveMonthID17[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName17[]
        = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName17[]
        = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName17[] = { 0x634, 0x0 };
    static const sal_Unicode genitiveMonthID18[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName18[]
        = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName18[]
        = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName18[] = { 0x631, 0x0 };
    static const sal_Unicode genitiveMonthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName19[]
        = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName19[] = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName19[] = { 0x634, 0x0 };
    static const sal_Unicode genitiveMonthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName110[] = { 0x630, 0x0 };
    static const sal_Unicode genitiveMonthID111[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName111[] = { 0x630, 0x0 };
    static const sal_Unicode partitiveMonthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName10[]
        = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName10[]
        = { 0x645, 0x62d, 0x631, 0x645, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName10[] = { 0x645, 0x0 };
    static const sal_Unicode partitiveMonthID11[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName11[] = { 0x635, 0x641, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName11[] = { 0x635, 0x0 };
    static const sal_Unicode partitiveMonthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName12[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName12[] = { 0x631, 0x0 };
    static const sal_Unicode partitiveMonthID13[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName13[]
        = { 0x631, 0x628, 0x64a, 0x639, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName13[] = { 0x631, 0x0 };
    static const sal_Unicode partitiveMonthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName14[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x649, 0x0
    };
    static const sal_Unicode partitiveMonthDefaultFullName14[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x623, 0x648, 0x644, 0x649, 0x0
    };
    static const sal_Unicode partitiveMonthDefaultNarrowName14[] = { 0x62c, 0x0 };
    static const sal_Unicode partitiveMonthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName15[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x629, 0x0
    };
    static const sal_Unicode partitiveMonthDefaultFullName15[] = {
        0x62c, 0x645, 0x627, 0x62f, 0x649, 0x20, 0x627, 0x644, 0x622, 0x62e, 0x631, 0x629, 0x0
    };
    static const sal_Unicode partitiveMonthDefaultNarrowName15[] = { 0x62c, 0x0 };
    static const sal_Unicode partitiveMonthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName16[] = { 0x631, 0x62c, 0x628, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName16[] = { 0x631, 0x0 };
    static const sal_Unicode partitiveMonthID17[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName17[]
        = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName17[]
        = { 0x634, 0x639, 0x628, 0x627, 0x646, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName17[] = { 0x634, 0x0 };
    static const sal_Unicode partitiveMonthID18[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName18[]
        = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName18[]
        = { 0x631, 0x645, 0x636, 0x627, 0x646, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName18[] = { 0x631, 0x0 };
    static const sal_Unicode partitiveMonthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName19[]
        = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName19[]
        = { 0x634, 0x648, 0x627, 0x644, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName19[] = { 0x634, 0x0 };
    static const sal_Unicode partitiveMonthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName110[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x642, 0x639, 0x62f, 0x629, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName110[] = { 0x630, 0x0 };
    static const sal_Unicode partitiveMonthID111[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName111[]
        = { 0x630, 0x648, 0x20, 0x627, 0x644, 0x62d, 0x62c, 0x629, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName111[] = { 0x630, 0x0 };
    static const sal_Unicode eraID10[]
        = { 0x42, 0x65, 0x66, 0x6f, 0x72, 0x65, 0x48, 0x69, 0x6a, 0x72, 0x61, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName10[] = { 0x647, 0x200d, 0x0 };
    static const sal_Unicode eraDefaultFullName10[]
        = { 0x642, 0x628, 0x644, 0x20, 0x627, 0x644, 0x647, 0x62c, 0x631, 0x629, 0x0 };
    static const sal_Unicode eraID11[]
        = { 0x41, 0x66, 0x74, 0x65, 0x72, 0x48, 0x69, 0x6a, 0x72, 0x61, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName11[] = { 0x0 };
    static const sal_Unicode eraDefaultFullName11[] = { 0x647, 0x62c, 0x631, 0x64a, 0x0 };
    static const sal_Unicode startDayOfWeek1[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode minimalDaysInFirstWeek1[] = { 1 };
    static const sal_Int16 calendarsCount = 2;

    static const sal_Unicode nbOfDays[] = { 7, 7 };
    static const sal_Unicode nbOfMonths[] = { 12, 12 };
    static const sal_Unicode nbOfGenitiveMonths[] = { 12, 12 };
    static const sal_Unicode nbOfPartitiveMonths[] = { 12, 12 };
    static const sal_Unicode nbOfEras[] = { 2, 2 };
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
        monthID00,
        monthDefaultAbbrvName00,
        monthDefaultFullName00,
        monthDefaultNarrowName00,
        monthID01,
        monthDefaultAbbrvName01,
        monthDefaultFullName01,
        monthDefaultNarrowName01,
        monthID02,
        monthDefaultAbbrvName02,
        monthDefaultFullName02,
        monthDefaultNarrowName02,
        monthID03,
        monthDefaultAbbrvName03,
        monthDefaultFullName03,
        monthDefaultNarrowName03,
        monthID04,
        monthDefaultAbbrvName04,
        monthDefaultFullName04,
        monthDefaultNarrowName04,
        monthID05,
        monthDefaultAbbrvName05,
        monthDefaultFullName05,
        monthDefaultNarrowName05,
        monthID06,
        monthDefaultAbbrvName06,
        monthDefaultFullName06,
        monthDefaultNarrowName06,
        monthID07,
        monthDefaultAbbrvName07,
        monthDefaultFullName07,
        monthDefaultNarrowName07,
        monthID08,
        monthDefaultAbbrvName08,
        monthDefaultFullName08,
        monthDefaultNarrowName08,
        monthID09,
        monthDefaultAbbrvName09,
        monthDefaultFullName09,
        monthDefaultNarrowName09,
        monthID010,
        monthDefaultAbbrvName010,
        monthDefaultFullName010,
        monthDefaultNarrowName010,
        monthID011,
        monthDefaultAbbrvName011,
        monthDefaultFullName011,
        monthDefaultNarrowName011,
        genitiveMonthID00,
        genitiveMonthDefaultAbbrvName00,
        genitiveMonthDefaultFullName00,
        genitiveMonthDefaultNarrowName00,
        genitiveMonthID01,
        genitiveMonthDefaultAbbrvName01,
        genitiveMonthDefaultFullName01,
        genitiveMonthDefaultNarrowName01,
        genitiveMonthID02,
        genitiveMonthDefaultAbbrvName02,
        genitiveMonthDefaultFullName02,
        genitiveMonthDefaultNarrowName02,
        genitiveMonthID03,
        genitiveMonthDefaultAbbrvName03,
        genitiveMonthDefaultFullName03,
        genitiveMonthDefaultNarrowName03,
        genitiveMonthID04,
        genitiveMonthDefaultAbbrvName04,
        genitiveMonthDefaultFullName04,
        genitiveMonthDefaultNarrowName04,
        genitiveMonthID05,
        genitiveMonthDefaultAbbrvName05,
        genitiveMonthDefaultFullName05,
        genitiveMonthDefaultNarrowName05,
        genitiveMonthID06,
        genitiveMonthDefaultAbbrvName06,
        genitiveMonthDefaultFullName06,
        genitiveMonthDefaultNarrowName06,
        genitiveMonthID07,
        genitiveMonthDefaultAbbrvName07,
        genitiveMonthDefaultFullName07,
        genitiveMonthDefaultNarrowName07,
        genitiveMonthID08,
        genitiveMonthDefaultAbbrvName08,
        genitiveMonthDefaultFullName08,
        genitiveMonthDefaultNarrowName08,
        genitiveMonthID09,
        genitiveMonthDefaultAbbrvName09,
        genitiveMonthDefaultFullName09,
        genitiveMonthDefaultNarrowName09,
        genitiveMonthID010,
        genitiveMonthDefaultAbbrvName010,
        genitiveMonthDefaultFullName010,
        genitiveMonthDefaultNarrowName010,
        genitiveMonthID011,
        genitiveMonthDefaultAbbrvName011,
        genitiveMonthDefaultFullName011,
        genitiveMonthDefaultNarrowName011,
        partitiveMonthID00,
        partitiveMonthDefaultAbbrvName00,
        partitiveMonthDefaultFullName00,
        partitiveMonthDefaultNarrowName00,
        partitiveMonthID01,
        partitiveMonthDefaultAbbrvName01,
        partitiveMonthDefaultFullName01,
        partitiveMonthDefaultNarrowName01,
        partitiveMonthID02,
        partitiveMonthDefaultAbbrvName02,
        partitiveMonthDefaultFullName02,
        partitiveMonthDefaultNarrowName02,
        partitiveMonthID03,
        partitiveMonthDefaultAbbrvName03,
        partitiveMonthDefaultFullName03,
        partitiveMonthDefaultNarrowName03,
        partitiveMonthID04,
        partitiveMonthDefaultAbbrvName04,
        partitiveMonthDefaultFullName04,
        partitiveMonthDefaultNarrowName04,
        partitiveMonthID05,
        partitiveMonthDefaultAbbrvName05,
        partitiveMonthDefaultFullName05,
        partitiveMonthDefaultNarrowName05,
        partitiveMonthID06,
        partitiveMonthDefaultAbbrvName06,
        partitiveMonthDefaultFullName06,
        partitiveMonthDefaultNarrowName06,
        partitiveMonthID07,
        partitiveMonthDefaultAbbrvName07,
        partitiveMonthDefaultFullName07,
        partitiveMonthDefaultNarrowName07,
        partitiveMonthID08,
        partitiveMonthDefaultAbbrvName08,
        partitiveMonthDefaultFullName08,
        partitiveMonthDefaultNarrowName08,
        partitiveMonthID09,
        partitiveMonthDefaultAbbrvName09,
        partitiveMonthDefaultFullName09,
        partitiveMonthDefaultNarrowName09,
        partitiveMonthID010,
        partitiveMonthDefaultAbbrvName010,
        partitiveMonthDefaultFullName010,
        partitiveMonthDefaultNarrowName010,
        partitiveMonthID011,
        partitiveMonthDefaultAbbrvName011,
        partitiveMonthDefaultFullName011,
        partitiveMonthDefaultNarrowName011,
        eraID00,
        eraDefaultAbbrvName00,
        eraDefaultFullName00,
        eraID01,
        eraDefaultAbbrvName01,
        eraDefaultFullName01,
        startDayOfWeek0,
        minimalDaysInFirstWeek0,
        calendarID1,
        defaultCalendar1,
        dayID10,
        dayDefaultAbbrvName10,
        dayDefaultFullName10,
        dayDefaultNarrowName10,
        dayID11,
        dayDefaultAbbrvName11,
        dayDefaultFullName11,
        dayDefaultNarrowName11,
        dayID12,
        dayDefaultAbbrvName12,
        dayDefaultFullName12,
        dayDefaultNarrowName12,
        dayID13,
        dayDefaultAbbrvName13,
        dayDefaultFullName13,
        dayDefaultNarrowName13,
        dayID14,
        dayDefaultAbbrvName14,
        dayDefaultFullName14,
        dayDefaultNarrowName14,
        dayID15,
        dayDefaultAbbrvName15,
        dayDefaultFullName15,
        dayDefaultNarrowName15,
        dayID16,
        dayDefaultAbbrvName16,
        dayDefaultFullName16,
        dayDefaultNarrowName16,
        monthID10,
        monthDefaultAbbrvName10,
        monthDefaultFullName10,
        monthDefaultNarrowName10,
        monthID11,
        monthDefaultAbbrvName11,
        monthDefaultFullName11,
        monthDefaultNarrowName11,
        monthID12,
        monthDefaultAbbrvName12,
        monthDefaultFullName12,
        monthDefaultNarrowName12,
        monthID13,
        monthDefaultAbbrvName13,
        monthDefaultFullName13,
        monthDefaultNarrowName13,
        monthID14,
        monthDefaultAbbrvName14,
        monthDefaultFullName14,
        monthDefaultNarrowName14,
        monthID15,
        monthDefaultAbbrvName15,
        monthDefaultFullName15,
        monthDefaultNarrowName15,
        monthID16,
        monthDefaultAbbrvName16,
        monthDefaultFullName16,
        monthDefaultNarrowName16,
        monthID17,
        monthDefaultAbbrvName17,
        monthDefaultFullName17,
        monthDefaultNarrowName17,
        monthID18,
        monthDefaultAbbrvName18,
        monthDefaultFullName18,
        monthDefaultNarrowName18,
        monthID19,
        monthDefaultAbbrvName19,
        monthDefaultFullName19,
        monthDefaultNarrowName19,
        monthID110,
        monthDefaultAbbrvName110,
        monthDefaultFullName110,
        monthDefaultNarrowName110,
        monthID111,
        monthDefaultAbbrvName111,
        monthDefaultFullName111,
        monthDefaultNarrowName111,
        genitiveMonthID10,
        genitiveMonthDefaultAbbrvName10,
        genitiveMonthDefaultFullName10,
        genitiveMonthDefaultNarrowName10,
        genitiveMonthID11,
        genitiveMonthDefaultAbbrvName11,
        genitiveMonthDefaultFullName11,
        genitiveMonthDefaultNarrowName11,
        genitiveMonthID12,
        genitiveMonthDefaultAbbrvName12,
        genitiveMonthDefaultFullName12,
        genitiveMonthDefaultNarrowName12,
        genitiveMonthID13,
        genitiveMonthDefaultAbbrvName13,
        genitiveMonthDefaultFullName13,
        genitiveMonthDefaultNarrowName13,
        genitiveMonthID14,
        genitiveMonthDefaultAbbrvName14,
        genitiveMonthDefaultFullName14,
        genitiveMonthDefaultNarrowName14,
        genitiveMonthID15,
        genitiveMonthDefaultAbbrvName15,
        genitiveMonthDefaultFullName15,
        genitiveMonthDefaultNarrowName15,
        genitiveMonthID16,
        genitiveMonthDefaultAbbrvName16,
        genitiveMonthDefaultFullName16,
        genitiveMonthDefaultNarrowName16,
        genitiveMonthID17,
        genitiveMonthDefaultAbbrvName17,
        genitiveMonthDefaultFullName17,
        genitiveMonthDefaultNarrowName17,
        genitiveMonthID18,
        genitiveMonthDefaultAbbrvName18,
        genitiveMonthDefaultFullName18,
        genitiveMonthDefaultNarrowName18,
        genitiveMonthID19,
        genitiveMonthDefaultAbbrvName19,
        genitiveMonthDefaultFullName19,
        genitiveMonthDefaultNarrowName19,
        genitiveMonthID110,
        genitiveMonthDefaultAbbrvName110,
        genitiveMonthDefaultFullName110,
        genitiveMonthDefaultNarrowName110,
        genitiveMonthID111,
        genitiveMonthDefaultAbbrvName111,
        genitiveMonthDefaultFullName111,
        genitiveMonthDefaultNarrowName111,
        partitiveMonthID10,
        partitiveMonthDefaultAbbrvName10,
        partitiveMonthDefaultFullName10,
        partitiveMonthDefaultNarrowName10,
        partitiveMonthID11,
        partitiveMonthDefaultAbbrvName11,
        partitiveMonthDefaultFullName11,
        partitiveMonthDefaultNarrowName11,
        partitiveMonthID12,
        partitiveMonthDefaultAbbrvName12,
        partitiveMonthDefaultFullName12,
        partitiveMonthDefaultNarrowName12,
        partitiveMonthID13,
        partitiveMonthDefaultAbbrvName13,
        partitiveMonthDefaultFullName13,
        partitiveMonthDefaultNarrowName13,
        partitiveMonthID14,
        partitiveMonthDefaultAbbrvName14,
        partitiveMonthDefaultFullName14,
        partitiveMonthDefaultNarrowName14,
        partitiveMonthID15,
        partitiveMonthDefaultAbbrvName15,
        partitiveMonthDefaultFullName15,
        partitiveMonthDefaultNarrowName15,
        partitiveMonthID16,
        partitiveMonthDefaultAbbrvName16,
        partitiveMonthDefaultFullName16,
        partitiveMonthDefaultNarrowName16,
        partitiveMonthID17,
        partitiveMonthDefaultAbbrvName17,
        partitiveMonthDefaultFullName17,
        partitiveMonthDefaultNarrowName17,
        partitiveMonthID18,
        partitiveMonthDefaultAbbrvName18,
        partitiveMonthDefaultFullName18,
        partitiveMonthDefaultNarrowName18,
        partitiveMonthID19,
        partitiveMonthDefaultAbbrvName19,
        partitiveMonthDefaultFullName19,
        partitiveMonthDefaultNarrowName19,
        partitiveMonthID110,
        partitiveMonthDefaultAbbrvName110,
        partitiveMonthDefaultFullName110,
        partitiveMonthDefaultNarrowName110,
        partitiveMonthID111,
        partitiveMonthDefaultAbbrvName111,
        partitiveMonthDefaultFullName111,
        partitiveMonthDefaultNarrowName111,
        eraID10,
        eraDefaultAbbrvName10,
        eraDefaultFullName10,
        eraID11,
        eraDefaultAbbrvName11,
        eraDefaultFullName11,
        startDayOfWeek1,
        minimalDaysInFirstWeek1,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_ar_TN(sal_Int16& count)
    {
        count = calendarsCount;
        return (sal_Unicode**)calendars;
    }
    static const sal_Unicode defaultCurrency0[] = { 1 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
    static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
    static const sal_Unicode currencyID0[] = { 0x54, 0x4e, 0x44, 0x0 };
    static const sal_Unicode currencySymbol0[] = { 0x62f, 0x2e, 0x62a, 0x2e, 0x200f, 0x0 };
    static const sal_Unicode bankSymbol0[] = { 0x54, 0x4e, 0x44, 0x0 };
    static const sal_Unicode currencyName0[] = { 0x54, 0x4e, 0x44, 0x0 };
    static const sal_Unicode currencyDecimalPlaces0[] = { 3 };

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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_ar_TN(sal_Int16& count)
    {
        count = currencyCount;
        return (sal_Unicode**)currencies;
    }
    extern sal_Unicode** SAL_CALL getTransliterations_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_ar_TN(sal_Int16& count)
    {
        return getTransliterations_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_ar_TN(sal_Int16& count)
    {
        return getForbiddenCharacters_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_ar_TN(sal_Int16& count)
    {
        return getBreakIteratorRules_ar_EG(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_ar_EG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_ar_TN(sal_Int16& count)
    {
        return getReservedWords_ar_EG(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_ar_EG(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_ar_TN(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_ar_EG(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_ar_EG(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_ar_TN(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_ar_EG(nStyles, nLevels, nAttributes);
    }
} // extern "C"
