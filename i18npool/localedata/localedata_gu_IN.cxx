#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x67, 0x75, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x47, 0x75, 0x6a, 0x61, 0x72, 0x61, 0x74, 0x69, 0x0 };
    static const sal_Unicode countryID[] = { 0x49, 0x4e, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x49, 0x6e, 0x64, 0x69, 0x61, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_gu_IN(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_gu_IN(sal_Int16& count)
    {
        return getLocaleItem_hi_IN(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x20b9, 0x2d, 0x34, 0x34, 0x37, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_hi_IN(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_gu_IN(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_hi_IN(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_gu_IN(sal_Int16& count)
    {
        return getDateAcceptancePatterns_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_gu_IN(sal_Int16& count)
    {
        return getCollatorImplementation_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_gu_IN(sal_Int16& count)
    {
        return getCollationOptions_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_hi_IN(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_gu_IN(sal_Int16& count)
    {
        return getSearchOptions_hi_IN(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_gu_IN(sal_Int16& count)
    {
        return getIndexAlgorithm_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_gu_IN(sal_Int16& count)
    {
        return getUnicodeScripts_en_US(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_gu_IN(sal_Int16& count)
    {
        return getFollowPageWords_en_US(count);
    }
    static const sal_Unicode calendarID0[]
        = { 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode defaultCalendar0[] = { 1 };
    static const sal_Unicode dayID00[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName00[] = { 0xab0, 0xab5, 0xabf, 0x0 };
    static const sal_Unicode dayDefaultFullName00[]
        = { 0xab0, 0xab5, 0xabf, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName00[] = { 0xab0, 0x0 };
    static const sal_Unicode dayID01[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName01[]
        = { 0xab8, 0xacb, 0xaae, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultFullName01[]
        = { 0xab8, 0xacb, 0xaae, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName01[] = { 0xab8, 0x0 };
    static const sal_Unicode dayID02[] = { 0x74, 0x75, 0x65, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName02[] = { 0xaae, 0xa82, 0xa97, 0xab3, 0x0 };
    static const sal_Unicode dayDefaultFullName02[]
        = { 0xaae, 0xa82, 0xa97, 0xab3, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName02[] = { 0xaae, 0x0 };
    static const sal_Unicode dayID03[] = { 0x77, 0x65, 0x64, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName03[] = { 0xaac, 0xac1, 0xaa7, 0x0 };
    static const sal_Unicode dayDefaultFullName03[]
        = { 0xaac, 0xac1, 0xaa7, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName03[] = { 0xaac, 0x0 };
    static const sal_Unicode dayID04[] = { 0x74, 0x68, 0x75, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName04[] = { 0xa97, 0xac1, 0xab0, 0xac1, 0x0 };
    static const sal_Unicode dayDefaultFullName04[]
        = { 0xa97, 0xac1, 0xab0, 0xac1, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName04[] = { 0xa97, 0x0 };
    static const sal_Unicode dayID05[] = { 0x66, 0x72, 0x69, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName05[] = { 0xab6, 0xac1, 0xa95, 0xacd, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultFullName05[]
        = { 0xab6, 0xac1, 0xa95, 0xacd, 0xab0, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName05[] = { 0xab6, 0x0 };
    static const sal_Unicode dayID06[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName06[] = { 0xab6, 0xaa8, 0xabf, 0x0 };
    static const sal_Unicode dayDefaultFullName06[]
        = { 0xab6, 0xaa8, 0xabf, 0xab5, 0xabe, 0xab0, 0x0 };
    static const sal_Unicode dayDefaultNarrowName06[] = { 0xab6, 0x0 };
    static const sal_Unicode monthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName00[] = { 0xa9c, 0xabe, 0xaa8, 0x0 };
    static const sal_Unicode monthDefaultFullName00[]
        = { 0xa9c, 0xabe, 0xaa8, 0xacd, 0xaaf, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName00[] = { 0xa9c, 0x0 };
    static const sal_Unicode monthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName01[] = { 0xaab, 0xac7, 0xaac, 0x0 };
    static const sal_Unicode monthDefaultFullName01[]
        = { 0xaab, 0xac7, 0xaac, 0xacd, 0xab0, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName01[] = { 0xaab, 0x0 };
    static const sal_Unicode monthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName02[] = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode monthDefaultFullName02[] = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode monthDefaultNarrowName02[] = { 0xaae, 0x0 };
    static const sal_Unicode monthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode monthDefaultFullName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode monthDefaultNarrowName03[] = { 0xa8f, 0x0 };
    static const sal_Unicode monthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode monthDefaultFullName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode monthDefaultNarrowName04[] = { 0xaae, 0x0 };
    static const sal_Unicode monthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode monthDefaultFullName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode monthDefaultNarrowName05[] = { 0xa9c, 0x0 };
    static const sal_Unicode monthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName06[] = { 0xa9c, 0xac1, 0xab2, 0x0 };
    static const sal_Unicode monthDefaultFullName06[] = { 0xa9c, 0xac1, 0xab2, 0xabe, 0xa87, 0x0 };
    static const sal_Unicode monthDefaultNarrowName06[] = { 0xa9c, 0x0 };
    static const sal_Unicode monthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName07[] = { 0xa91, 0xa97, 0x0 };
    static const sal_Unicode monthDefaultFullName07[] = { 0xa91, 0xa97, 0xab8, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode monthDefaultNarrowName07[] = { 0xa91, 0x0 };
    static const sal_Unicode monthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName08[] = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode monthDefaultFullName08[]
        = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName08[] = { 0xab8, 0x0 };
    static const sal_Unicode monthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName09[] = { 0xa91, 0xa95, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode monthDefaultFullName09[]
        = { 0xa91, 0xa95, 0xacd, 0xa9f, 0xacb, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName09[] = { 0xa91, 0x0 };
    static const sal_Unicode monthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName010[] = { 0xaa8, 0xab5, 0x0 };
    static const sal_Unicode monthDefaultFullName010[]
        = { 0xaa8, 0xab5, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName010[] = { 0xaa8, 0x0 };
    static const sal_Unicode monthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName011[] = { 0xaa1, 0xabf, 0xab8, 0x0 };
    static const sal_Unicode monthDefaultFullName011[]
        = { 0xaa1, 0xabf, 0xab8, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode monthDefaultNarrowName011[] = { 0xaa1, 0x0 };
    static const sal_Unicode genitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName00[] = { 0xa9c, 0xabe, 0xaa8, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName00[]
        = { 0xa9c, 0xabe, 0xaa8, 0xacd, 0xaaf, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName00[] = { 0xa9c, 0x0 };
    static const sal_Unicode genitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName01[] = { 0xaab, 0xac7, 0xaac, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName01[]
        = { 0xaab, 0xac7, 0xaac, 0xacd, 0xab0, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName01[] = { 0xaab, 0x0 };
    static const sal_Unicode genitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName02[]
        = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName02[]
        = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName02[] = { 0xaae, 0x0 };
    static const sal_Unicode genitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName03[] = { 0xa8f, 0x0 };
    static const sal_Unicode genitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName04[] = { 0xaae, 0x0 };
    static const sal_Unicode genitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName05[] = { 0xa9c, 0x0 };
    static const sal_Unicode genitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName06[] = { 0xa9c, 0xac1, 0xab2, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName06[]
        = { 0xa9c, 0xac1, 0xab2, 0xabe, 0xa87, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName06[] = { 0xa9c, 0x0 };
    static const sal_Unicode genitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName07[] = { 0xa91, 0xa97, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName07[]
        = { 0xa91, 0xa97, 0xab8, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName07[] = { 0xa91, 0x0 };
    static const sal_Unicode genitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName08[]
        = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName08[]
        = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName08[] = { 0xab8, 0x0 };
    static const sal_Unicode genitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName09[]
        = { 0xa91, 0xa95, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName09[]
        = { 0xa91, 0xa95, 0xacd, 0xa9f, 0xacb, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName09[] = { 0xa91, 0x0 };
    static const sal_Unicode genitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName010[] = { 0xaa8, 0xab5, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName010[]
        = { 0xaa8, 0xab5, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName010[] = { 0xaa8, 0x0 };
    static const sal_Unicode genitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName011[] = { 0xaa1, 0xabf, 0xab8, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName011[]
        = { 0xaa1, 0xabf, 0xab8, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName011[] = { 0xaa1, 0x0 };
    static const sal_Unicode partitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName00[] = { 0xa9c, 0xabe, 0xaa8, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName00[]
        = { 0xa9c, 0xabe, 0xaa8, 0xacd, 0xaaf, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName00[] = { 0xa9c, 0x0 };
    static const sal_Unicode partitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName01[] = { 0xaab, 0xac7, 0xaac, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName01[]
        = { 0xaab, 0xac7, 0xaac, 0xacd, 0xab0, 0xac1, 0xa86, 0xab0, 0xac0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName01[] = { 0xaab, 0x0 };
    static const sal_Unicode partitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName02[]
        = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName02[]
        = { 0xaae, 0xabe, 0xab0, 0xacd, 0xa9a, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName02[] = { 0xaae, 0x0 };
    static const sal_Unicode partitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName03[]
        = { 0xa8f, 0xaaa, 0xacd, 0xab0, 0xabf, 0xab2, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName03[] = { 0xa8f, 0x0 };
    static const sal_Unicode partitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName04[] = { 0xaae, 0xac7, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName04[] = { 0xaae, 0x0 };
    static const sal_Unicode partitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName05[] = { 0xa9c, 0xac2, 0xaa8, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName05[] = { 0xa9c, 0x0 };
    static const sal_Unicode partitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName06[] = { 0xa9c, 0xac1, 0xab2, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName06[]
        = { 0xa9c, 0xac1, 0xab2, 0xabe, 0xa87, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName06[] = { 0xa9c, 0x0 };
    static const sal_Unicode partitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName07[] = { 0xa91, 0xa97, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName07[]
        = { 0xa91, 0xa97, 0xab8, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName07[] = { 0xa91, 0x0 };
    static const sal_Unicode partitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName08[]
        = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName08[]
        = { 0xab8, 0xaaa, 0xacd, 0xa9f, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName08[] = { 0xab8, 0x0 };
    static const sal_Unicode partitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName09[]
        = { 0xa91, 0xa95, 0xacd, 0xa9f, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName09[]
        = { 0xa91, 0xa95, 0xacd, 0xa9f, 0xacb, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName09[] = { 0xa91, 0x0 };
    static const sal_Unicode partitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName010[] = { 0xaa8, 0xab5, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName010[]
        = { 0xaa8, 0xab5, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName010[] = { 0xaa8, 0x0 };
    static const sal_Unicode partitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName011[] = { 0xaa1, 0xabf, 0xab8, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName011[]
        = { 0xaa1, 0xabf, 0xab8, 0xac7, 0xaae, 0xacd, 0xaac, 0xab0, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName011[] = { 0xaa1, 0x0 };
    static const sal_Unicode eraID00[] = { 0x62, 0x63, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName00[]
        = { 0xa88, 0x2e, 0xab8, 0x2e, 0xaaa, 0xac2, 0x2e, 0x0 };
    static const sal_Unicode eraDefaultFullName00[] = { 0xa88, 0xab8, 0xab5, 0xac0, 0xab8,
                                                        0xaa8, 0xaaa, 0xac2, 0xab0, 0xacd,
                                                        0xab5, 0xac7, 0x0 };
    static const sal_Unicode eraID01[] = { 0x61, 0x64, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName01[] = { 0xa88, 0x2e, 0xab8, 0x2e, 0x0 };
    static const sal_Unicode eraDefaultFullName01[]
        = { 0xa88, 0xab8, 0xab5, 0xac0, 0xab8, 0xaa8, 0x0 };
    static const sal_Unicode startDayOfWeek0[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode minimalDaysInFirstWeek0[] = { 1 };
    static const sal_Int16 calendarsCount = 1;

    static const sal_Unicode nbOfDays[] = { 7 };
    static const sal_Unicode nbOfMonths[] = { 12 };
    static const sal_Unicode nbOfGenitiveMonths[] = { 12 };
    static const sal_Unicode nbOfPartitiveMonths[] = { 12 };
    static const sal_Unicode nbOfEras[] = { 2 };
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
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_gu_IN(sal_Int16& count)
    {
        count = calendarsCount;
        return (sal_Unicode**)calendars;
    }
    static const sal_Unicode defaultCurrency0[] = { 1 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
    static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
    static const sal_Unicode currencyID0[] = { 0x49, 0x4e, 0x52, 0x0 };
    static const sal_Unicode currencySymbol0[] = { 0x20b9, 0x0 };
    static const sal_Unicode bankSymbol0[] = { 0x49, 0x4e, 0x52, 0x0 };
    static const sal_Unicode currencyName0[] = { 0x52, 0x75, 0x70, 0x65, 0x65, 0x0 };
    static const sal_Unicode currencyDecimalPlaces0[] = { 2 };

    static const sal_Unicode defaultCurrency1[] = { 0 };
    static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes1[] = { 0 };
    static const sal_Unicode defaultCurrencyLegacyOnly1[] = { 1 };
    static const sal_Unicode currencyID1[] = { 0x49, 0x4e, 0x52, 0x0 };
    static const sal_Unicode currencySymbol1[] = { 0xa8b, 0xac1, 0x2e, 0x0 };
    static const sal_Unicode bankSymbol1[] = { 0x49, 0x4e, 0x52, 0x0 };
    static const sal_Unicode currencyName1[] = { 0x52, 0x75, 0x70, 0x65, 0x65, 0x0 };
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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_gu_IN(sal_Int16& count)
    {
        count = currencyCount;
        return (sal_Unicode**)currencies;
    }
    extern sal_Unicode** SAL_CALL getTransliterations_en_US(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_gu_IN(sal_Int16& count)
    {
        return getTransliterations_en_US(count);
    }
    static const sal_Unicode ReservedWord0[] = { 0xa96, 0xab0, 0xac1, 0xa82, 0x0 };
    static const sal_Unicode ReservedWord1[] = { 0xa96, 0xacb, 0xa9f, 0xac1, 0xa82, 0x0 };
    static const sal_Unicode ReservedWord2[]
        = { 0xa9a, 0xaa4, 0xac1, 0xab0, 0xacd, 0xaa5, 0xabe, 0xa82, 0xab6, 0xae7, 0x0 };
    static const sal_Unicode ReservedWord3[]
        = { 0xa9a, 0xaa4, 0xac1, 0xab0, 0xacd, 0xaa5, 0xabe, 0xa82, 0xab6, 0xae8, 0x0 };
    static const sal_Unicode ReservedWord4[]
        = { 0xa9a, 0xaa4, 0xac1, 0xab0, 0xacd, 0xaa5, 0xabe, 0xa82, 0xab6, 0xae9, 0x0 };
    static const sal_Unicode ReservedWord5[]
        = { 0xa9a, 0xaa4, 0xac1, 0xab0, 0xacd, 0xaa5, 0xabe, 0xa82, 0xab6, 0xaea, 0x0 };
    static const sal_Unicode ReservedWord6[] = { 0xa8a, 0xaaa, 0xab0, 0x0 };
    static const sal_Unicode ReservedWord7[] = { 0xaa8, 0xac0, 0xa9a, 0xac7, 0x0 };
    static const sal_Unicode ReservedWord8[] = { 0x51, 0x31, 0x0 };
    static const sal_Unicode ReservedWord9[] = { 0x51, 0x32, 0x0 };
    static const sal_Unicode ReservedWord10[] = { 0x51, 0x33, 0x0 };
    static const sal_Unicode ReservedWord11[] = { 0x51, 0x34, 0x0 };
    static const sal_Int16 nbOfReservedWords = 12;

    static const sal_Unicode* LCReservedWordsArray[] = {
        ReservedWord0, ReservedWord1, ReservedWord2, ReservedWord3, ReservedWord4,  ReservedWord5,
        ReservedWord6, ReservedWord7, ReservedWord8, ReservedWord9, ReservedWord10, ReservedWord11,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_gu_IN(sal_Int16& count)
    {
        count = nbOfReservedWords;
        return (sal_Unicode**)LCReservedWordsArray;
    }
    static const sal_Unicode forbiddenBegin[] = { 0x0 };
    static const sal_Unicode forbiddenEnd[] = { 0x0 };
    static const sal_Unicode hangingChars[] = { 0x0 };

    static const sal_Unicode* LCForbiddenCharactersArray[]
        = { forbiddenBegin, forbiddenEnd, hangingChars };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_gu_IN(sal_Int16& count)
    {
        count = 3;
        return (sal_Unicode**)LCForbiddenCharactersArray;
    }
    static const sal_Unicode EditMode[] = { 0x0 };
    static const sal_Unicode DictionaryMode[] = { 0x0 };
    static const sal_Unicode WordCountMode[] = { 0x0 };
    static const sal_Unicode CharacterMode[] = { 0x63, 0x68, 0x61, 0x72, 0x5f, 0x69, 0x6e, 0x0 };
    static const sal_Unicode LineMode[] = { 0x0 };

    static const sal_Unicode* LCBreakIteratorRulesArray[]
        = { EditMode, DictionaryMode, WordCountMode, CharacterMode, LineMode };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_gu_IN(sal_Int16& count)
    {
        count = 5;
        return (sal_Unicode**)LCBreakIteratorRulesArray;
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_hi_IN(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_gu_IN(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_hi_IN(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_gu_IN(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
