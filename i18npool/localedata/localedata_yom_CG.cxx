#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x79, 0x6f, 0x6d, 0x0 };
    static const sal_Unicode langDefaultName[] = { 0x59, 0x6f, 0x6d, 0x62, 0x65, 0x0 };
    static const sal_Unicode countryID[] = { 0x43, 0x47, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x43, 0x6f, 0x6e, 0x67, 0x6f, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_yom_CG(sal_Int16& count)
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
    static const sal_Unicode LongDateDaySeparator[] = { 0x2c, 0x20, 0x0 };
    static const sal_Unicode LongDateMonthSeparator[] = { 0x20, 0x0 };
    static const sal_Unicode LongDateYearSeparator[] = { 0x20, 0x0 };
    static const sal_Unicode quotationStart[] = { 0x2018, 0x0 };
    static const sal_Unicode quotationEnd[] = { 0x2019, 0x0 };
    static const sal_Unicode doubleQuotationStart[] = { 0xab, 0x0 };
    static const sal_Unicode doubleQuotationEnd[] = { 0xbb, 0x0 };
    static const sal_Unicode timeAM[] = { 0x41, 0x4d, 0x0 };
    static const sal_Unicode timePM[] = { 0x50, 0x4d, 0x0 };
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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_yom_CG(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCType);
        return (sal_Unicode**)LCType;
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x43, 0x46, 0x41, 0x2d, 0x38, 0x32, 0x38, 0x34, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_kng_CG(sal_Int16& count,
                                                                    const sal_Unicode*& from,
                                                                    const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_yom_CG(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_kng_CG(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_kng_CG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_yom_CG(sal_Int16& count)
    {
        return getDateAcceptancePatterns_kng_CG(count);
    }
    static const sal_Unicode CollatorID0[]
        = { 0x61, 0x6c, 0x70, 0x68, 0x61, 0x6e, 0x75, 0x6d, 0x65, 0x72, 0x69, 0x63, 0x0 };
    static const sal_Unicode CollatorRule0[] = { 0x0 };
    static const sal_Unicode defaultCollator0[] = { 1 };

    static const sal_Unicode collationOption0[]
        = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Int16 nbOfCollationOptions = 1;

    static const sal_Int16 nbOfCollations = 1;

    static const sal_Unicode* LCCollatorArray[] = {
        CollatorID0,
        defaultCollator0,
        CollatorRule0,
    };

    static const sal_Unicode* collationOptions[] = { collationOption0, NULL };
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_yom_CG(sal_Int16& count)
    {
        count = nbOfCollations;
        return (sal_Unicode**)LCCollatorArray;
    }
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_yom_CG(sal_Int16& count)
    {
        count = nbOfCollationOptions;
        return (sal_Unicode**)collationOptions;
    }
    static const sal_Unicode searchOption0[]
        = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Int16 nbOfSearchOptions = 1;

    static const sal_Unicode* searchOptions[] = { searchOption0, NULL };
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_yom_CG(sal_Int16& count)
    {
        count = nbOfSearchOptions;
        return (sal_Unicode**)searchOptions;
    }
    static const sal_Unicode IndexID0[]
        = { 0x61, 0x6c, 0x70, 0x68, 0x61, 0x6e, 0x75, 0x6d, 0x65, 0x72, 0x69, 0x63, 0x0 };
    static const sal_Unicode IndexModule0[] = { 0x0 };
    static const sal_Unicode IndexKey0[] = { 0x41, 0x2d, 0x5a, 0x0 };
    static const sal_Unicode defaultIndex0[] = { 1 };
    static const sal_Unicode defaultPhonetic0[] = { 0 };

    static const sal_Unicode unicodeScript0[] = { 0x30, 0x0 };
    static const sal_Unicode unicodeScript1[] = { 0x31, 0x0 };
    static const sal_Unicode followPageWord0[] = { 0x2e, 0x2f, 0x2e, 0x0 };
    static const sal_Unicode followPageWord1[] = { 0x2e, 0x2e, 0x2f, 0x2e, 0x2e, 0x0 };
    static const sal_Int16 nbOfIndexs = 1;

    static const sal_Unicode* IndexArray[] = {
        IndexID0, IndexModule0, IndexKey0, defaultIndex0, defaultPhonetic0,
    };

    static const sal_Int16 nbOfUnicodeScripts = 2;

    static const sal_Unicode* UnicodeScriptArray[] = { unicodeScript0, unicodeScript1, NULL };

    static const sal_Int16 nbOfPageWords = 2;

    static const sal_Unicode* FollowPageWordArray[] = { followPageWord0, followPageWord1, NULL };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_yom_CG(sal_Int16& count)
    {
        count = nbOfIndexs;
        return (sal_Unicode**)IndexArray;
    }
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_yom_CG(sal_Int16& count)
    {
        count = nbOfUnicodeScripts;
        return (sal_Unicode**)UnicodeScriptArray;
    }
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_yom_CG(sal_Int16& count)
    {
        count = nbOfPageWords;
        return (sal_Unicode**)FollowPageWordArray;
    }
    static const sal_Unicode calendarID0[]
        = { 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
    static const sal_Unicode defaultCalendar0[] = { 1 };
    static const sal_Unicode dayID00[] = { 0x73, 0x75, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName00[] = { 0x64, 0x69, 0x6d, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName00[]
        = { 0x64, 0x69, 0x6d, 0x61, 0x6e, 0x63, 0x68, 0x65, 0x0 };
    static const sal_Unicode dayDefaultNarrowName00[] = { 0x64, 0x0 };
    static const sal_Unicode dayID01[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName01[] = { 0x6c, 0x75, 0x6e, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName01[] = { 0x6c, 0x75, 0x6e, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName01[] = { 0x6c, 0x0 };
    static const sal_Unicode dayID02[] = { 0x74, 0x75, 0x65, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName02[] = { 0x6d, 0x61, 0x72, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName02[] = { 0x6d, 0x61, 0x72, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName02[] = { 0x6d, 0x0 };
    static const sal_Unicode dayID03[] = { 0x77, 0x65, 0x64, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName03[] = { 0x6d, 0x65, 0x72, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName03[]
        = { 0x6d, 0x65, 0x72, 0x63, 0x72, 0x65, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName03[] = { 0x6d, 0x0 };
    static const sal_Unicode dayID04[] = { 0x74, 0x68, 0x75, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName04[] = { 0x6a, 0x65, 0x75, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName04[] = { 0x6a, 0x65, 0x75, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName04[] = { 0x6a, 0x0 };
    static const sal_Unicode dayID05[] = { 0x66, 0x72, 0x69, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName05[] = { 0x76, 0x65, 0x6e, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName05[]
        = { 0x76, 0x65, 0x6e, 0x64, 0x72, 0x65, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName05[] = { 0x76, 0x0 };
    static const sal_Unicode dayID06[] = { 0x73, 0x61, 0x74, 0x0 };
    static const sal_Unicode dayDefaultAbbrvName06[] = { 0x73, 0x61, 0x6d, 0x2e, 0x0 };
    static const sal_Unicode dayDefaultFullName06[] = { 0x73, 0x61, 0x6d, 0x65, 0x64, 0x69, 0x0 };
    static const sal_Unicode dayDefaultNarrowName06[] = { 0x73, 0x0 };
    static const sal_Unicode monthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName00[] = { 0x4e, 0x67, 0x30, 0x31, 0x0 };
    static const sal_Unicode monthDefaultFullName00[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6e, 0x74, 0x65, 0x74, 0x69, 0x0 };
    static const sal_Unicode monthDefaultNarrowName00[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName01[] = { 0x4e, 0x67, 0x30, 0x32, 0x0 };
    static const sal_Unicode monthDefaultFullName01[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6d, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode monthDefaultNarrowName01[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName02[] = { 0x4e, 0x67, 0x30, 0x33, 0x0 };
    static const sal_Unicode monthDefaultFullName02[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                          0x69, 0x20, 0x79, 0x69, 0x20,
                                                          0x74, 0x61, 0x74, 0x75, 0x0 };
    static const sal_Unicode monthDefaultNarrowName02[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName03[] = { 0x4e, 0x67, 0x30, 0x34, 0x0 };
    static const sal_Unicode monthDefaultFullName03[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6e, 0x61, 0x0 };
    static const sal_Unicode monthDefaultNarrowName03[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName04[] = { 0x4e, 0x67, 0x30, 0x35, 0x0 };
    static const sal_Unicode monthDefaultFullName04[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                          0x69, 0x20, 0x79, 0x69, 0x20,
                                                          0x74, 0x61, 0x6e, 0x75, 0x0 };
    static const sal_Unicode monthDefaultNarrowName04[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName05[] = { 0x4e, 0x67, 0x30, 0x36, 0x0 };
    static const sal_Unicode monthDefaultFullName05[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69,
            0x20, 0x73, 0x61, 0x6d, 0x75, 0x6e, 0x75, 0x0 };
    static const sal_Unicode monthDefaultNarrowName05[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName06[] = { 0x4e, 0x67, 0x30, 0x37, 0x0 };
    static const sal_Unicode monthDefaultFullName06[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20,
            0x73, 0x61, 0x62, 0x77, 0x61, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode monthDefaultNarrowName06[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName07[] = { 0x4e, 0x67, 0x30, 0x38, 0x0 };
    static const sal_Unicode monthDefaultFullName07[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                          0x69, 0x20, 0x79, 0x69, 0x20,
                                                          0x6e, 0x61, 0x6e, 0x61, 0x0 };
    static const sal_Unicode monthDefaultNarrowName07[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName08[] = { 0x4e, 0x67, 0x30, 0x39, 0x0 };
    static const sal_Unicode monthDefaultFullName08[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x76, 0x77, 0x61, 0x0 };
    static const sal_Unicode monthDefaultNarrowName08[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName09[] = { 0x4e, 0x67, 0x31, 0x30, 0x0 };
    static const sal_Unicode monthDefaultFullName09[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                          0x69, 0x20, 0x79, 0x69, 0x20,
                                                          0x6b, 0x75, 0x6d, 0x69, 0x0 };
    static const sal_Unicode monthDefaultNarrowName09[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName010[] = { 0x4e, 0x67, 0x31, 0x31, 0x0 };
    static const sal_Unicode monthDefaultFullName010[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x6d, 0x77, 0x65, 0x6b, 0x61, 0x0 };
    static const sal_Unicode monthDefaultNarrowName010[] = { 0x4e, 0x0 };
    static const sal_Unicode monthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode monthDefaultAbbrvName011[] = { 0x4e, 0x67, 0x31, 0x32, 0x0 };
    static const sal_Unicode monthDefaultFullName011[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode monthDefaultNarrowName011[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName00[] = { 0x4e, 0x67, 0x30, 0x31, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName00[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6e, 0x74, 0x65, 0x74, 0x69, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName00[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName01[] = { 0x4e, 0x67, 0x30, 0x32, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName01[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6d, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName01[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName02[] = { 0x4e, 0x67, 0x30, 0x33, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName02[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                  0x69, 0x20, 0x79, 0x69, 0x20,
                                                                  0x74, 0x61, 0x74, 0x75, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName02[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName03[] = { 0x4e, 0x67, 0x30, 0x34, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName03[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6e, 0x61, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName03[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName04[] = { 0x4e, 0x67, 0x30, 0x35, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName04[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                  0x69, 0x20, 0x79, 0x69, 0x20,
                                                                  0x74, 0x61, 0x6e, 0x75, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName04[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName05[] = { 0x4e, 0x67, 0x30, 0x36, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName05[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69,
            0x20, 0x73, 0x61, 0x6d, 0x75, 0x6e, 0x75, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName05[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName06[] = { 0x4e, 0x67, 0x30, 0x37, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName06[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20,
            0x73, 0x61, 0x62, 0x77, 0x61, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName06[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName07[] = { 0x4e, 0x67, 0x30, 0x38, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName07[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                  0x69, 0x20, 0x79, 0x69, 0x20,
                                                                  0x6e, 0x61, 0x6e, 0x61, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName07[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName08[] = { 0x4e, 0x67, 0x30, 0x39, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName08[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x76, 0x77, 0x61, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName08[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName09[] = { 0x4e, 0x67, 0x31, 0x30, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName09[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                  0x69, 0x20, 0x79, 0x69, 0x20,
                                                                  0x6b, 0x75, 0x6d, 0x69, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName09[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName010[] = { 0x4e, 0x67, 0x31, 0x31, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName010[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x6d, 0x77, 0x65, 0x6b, 0x61, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName010[] = { 0x4e, 0x0 };
    static const sal_Unicode genitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode genitiveMonthDefaultAbbrvName011[] = { 0x4e, 0x67, 0x31, 0x32, 0x0 };
    static const sal_Unicode genitiveMonthDefaultFullName011[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode genitiveMonthDefaultNarrowName011[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName00[] = { 0x4e, 0x67, 0x30, 0x31, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName00[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6e, 0x74, 0x65, 0x74, 0x69, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName00[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName01[] = { 0x4e, 0x67, 0x30, 0x32, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName01[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79,
            0x69, 0x20, 0x6d, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName01[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName02[] = { 0x4e, 0x67, 0x30, 0x33, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName02[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                   0x69, 0x20, 0x79, 0x69, 0x20,
                                                                   0x74, 0x61, 0x74, 0x75, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName02[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName03[] = { 0x4e, 0x67, 0x30, 0x34, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName03[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6e, 0x61, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName03[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName04[] = { 0x4e, 0x67, 0x30, 0x35, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName04[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                   0x69, 0x20, 0x79, 0x69, 0x20,
                                                                   0x74, 0x61, 0x6e, 0x75, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName04[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName05[] = { 0x4e, 0x67, 0x30, 0x36, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName05[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69,
            0x20, 0x73, 0x61, 0x6d, 0x75, 0x6e, 0x75, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName05[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName06[] = { 0x4e, 0x67, 0x30, 0x37, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName06[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20,
            0x73, 0x61, 0x62, 0x77, 0x61, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName06[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName07[] = { 0x4e, 0x67, 0x30, 0x38, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName07[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                   0x69, 0x20, 0x79, 0x69, 0x20,
                                                                   0x6e, 0x61, 0x6e, 0x61, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName07[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName08[] = { 0x4e, 0x67, 0x30, 0x39, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName08[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x76, 0x77, 0x61, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName08[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName09[] = { 0x4e, 0x67, 0x31, 0x30, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName09[] = { 0x4e, 0x67, 0x6f, 0x6e, 0x64,
                                                                   0x69, 0x20, 0x79, 0x69, 0x20,
                                                                   0x6b, 0x75, 0x6d, 0x69, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName09[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName010[] = { 0x4e, 0x67, 0x31, 0x31, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName010[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x6d, 0x77, 0x65, 0x6b, 0x61, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName010[] = { 0x4e, 0x0 };
    static const sal_Unicode partitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
    static const sal_Unicode partitiveMonthDefaultAbbrvName011[] = { 0x4e, 0x67, 0x31, 0x32, 0x0 };
    static const sal_Unicode partitiveMonthDefaultFullName011[]
        = { 0x4e, 0x67, 0x6f, 0x6e, 0x64, 0x69, 0x20, 0x79, 0x69, 0x20, 0x6b, 0x75,
            0x6d, 0x69, 0x20, 0x6e, 0x61, 0x20, 0x77, 0x61, 0x64, 0x69, 0x0 };
    static const sal_Unicode partitiveMonthDefaultNarrowName011[] = { 0x4e, 0x0 };
    static const sal_Unicode eraID00[] = { 0x62, 0x63, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName00[]
        = { 0x61, 0x76, 0x2e, 0x20, 0x4a, 0x2e, 0x2d, 0x43, 0x2e, 0x0 };
    static const sal_Unicode eraDefaultFullName00[]
        = { 0x61, 0x76, 0x2e, 0x20, 0x4a, 0x2e, 0x2d, 0x43, 0x2e, 0x0 };
    static const sal_Unicode eraID01[] = { 0x61, 0x64, 0x0 };

    static const sal_Unicode eraDefaultAbbrvName01[]
        = { 0x61, 0x70, 0x72, 0x2e, 0x20, 0x4a, 0x2e, 0x2d, 0x43, 0x2e, 0x0 };
    static const sal_Unicode eraDefaultFullName01[]
        = { 0x61, 0x70, 0x2e, 0x20, 0x4a, 0x2e, 0x2d, 0x43, 0x2e, 0x0 };
    static const sal_Unicode startDayOfWeek0[] = { 0x6d, 0x6f, 0x6e, 0x0 };
    static const sal_Unicode minimalDaysInFirstWeek0[] = { 7 };
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

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_yom_CG(sal_Int16& count)
    {
        count = calendarsCount;
        return (sal_Unicode**)calendars;
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_kng_CG(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_yom_CG(sal_Int16& count)
    {
        return getAllCurrencies_kng_CG(count);
    }
    static const sal_Unicode Transliteration0[]
        = { 0x53, 0x45, 0x4e, 0x54, 0x45, 0x4e, 0x43, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Unicode Transliteration1[]
        = { 0x4c, 0x4f, 0x57, 0x45, 0x52, 0x43, 0x41, 0x53, 0x45, 0x5f,
            0x55, 0x50, 0x50, 0x45, 0x52, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Unicode Transliteration2[]
        = { 0x55, 0x50, 0x50, 0x45, 0x52, 0x43, 0x41, 0x53, 0x45, 0x5f,
            0x4c, 0x4f, 0x57, 0x45, 0x52, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Unicode Transliteration3[]
        = { 0x54, 0x49, 0x54, 0x4c, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Unicode Transliteration4[]
        = { 0x54, 0x4f, 0x47, 0x47, 0x4c, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Unicode Transliteration5[]
        = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
    static const sal_Int16 nbOfTransliterations = 6;

    static const sal_Unicode* LCTransliterationsArray[] = {
        Transliteration0, Transliteration1, Transliteration2,
        Transliteration3, Transliteration4, Transliteration5,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_yom_CG(sal_Int16& count)
    {
        count = nbOfTransliterations;
        return (sal_Unicode**)LCTransliterationsArray;
    }
    static const sal_Unicode ReservedWord0[] = { 0x76, 0x72, 0x61, 0x69, 0x0 };
    static const sal_Unicode ReservedWord1[] = { 0x66, 0x61, 0x75, 0x78, 0x0 };
    static const sal_Unicode ReservedWord2[]
        = { 0x31, 0x65, 0x72, 0x20, 0x74, 0x72, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x72, 0x65, 0x0 };
    static const sal_Unicode ReservedWord3[]
        = { 0x32, 0x65, 0x20, 0x74, 0x72, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x72, 0x65, 0x0 };
    static const sal_Unicode ReservedWord4[]
        = { 0x33, 0x65, 0x20, 0x74, 0x72, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x72, 0x65, 0x0 };
    static const sal_Unicode ReservedWord5[]
        = { 0x34, 0x65, 0x20, 0x74, 0x72, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x72, 0x65, 0x0 };
    static const sal_Unicode ReservedWord6[] = { 0x73, 0x75, 0x70, 0x72, 0x61, 0x0 };
    static const sal_Unicode ReservedWord7[] = { 0x69, 0x6e, 0x66, 0x72, 0x61, 0x0 };
    static const sal_Unicode ReservedWord8[] = { 0x54, 0x31, 0x0 };
    static const sal_Unicode ReservedWord9[] = { 0x54, 0x32, 0x0 };
    static const sal_Unicode ReservedWord10[] = { 0x54, 0x33, 0x0 };
    static const sal_Unicode ReservedWord11[] = { 0x54, 0x34, 0x0 };
    static const sal_Int16 nbOfReservedWords = 12;

    static const sal_Unicode* LCReservedWordsArray[] = {
        ReservedWord0, ReservedWord1, ReservedWord2, ReservedWord3, ReservedWord4,  ReservedWord5,
        ReservedWord6, ReservedWord7, ReservedWord8, ReservedWord9, ReservedWord10, ReservedWord11,
    };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_yom_CG(sal_Int16& count)
    {
        count = nbOfReservedWords;
        return (sal_Unicode**)LCReservedWordsArray;
    }
    static const sal_Unicode forbiddenBegin[] = { 0x0 };
    static const sal_Unicode forbiddenEnd[] = { 0x0 };
    static const sal_Unicode hangingChars[] = { 0x0 };

    static const sal_Unicode* LCForbiddenCharactersArray[]
        = { forbiddenBegin, forbiddenEnd, hangingChars };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_yom_CG(sal_Int16& count)
    {
        count = 3;
        return (sal_Unicode**)LCForbiddenCharactersArray;
    }
    static const sal_Unicode EditMode[] = { 0x0 };
    static const sal_Unicode DictionaryMode[] = { 0x0 };
    static const sal_Unicode WordCountMode[] = { 0x0 };
    static const sal_Unicode CharacterMode[] = { 0x0 };
    static const sal_Unicode LineMode[] = { 0x0 };

    static const sal_Unicode* LCBreakIteratorRulesArray[]
        = { EditMode, DictionaryMode, WordCountMode, CharacterMode, LineMode };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_yom_CG(sal_Int16& count)
    {
        count = 5;
        return (sal_Unicode**)LCBreakIteratorRulesArray;
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_en_US(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_yom_CG(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_en_US(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_en_US(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_yom_CG(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_en_US(nStyles, nLevels, nAttributes);
    }
} // extern "C"
