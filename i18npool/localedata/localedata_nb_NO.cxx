#include <sal/types.h>

#include <stdio.h>

extern "C"
{
    static const sal_Unicode langID[] = { 0x6e, 0x62, 0x0 };
    static const sal_Unicode langDefaultName[]
        = { 0x4e, 0x6f, 0x72, 0x77, 0x65, 0x67, 0x69, 0x61, 0x6e,
            0x20, 0x42, 0x6f, 0x6b, 0x6d, 0xe5, 0x6c, 0x0 };
    static const sal_Unicode countryID[] = { 0x4e, 0x4f, 0x0 };
    static const sal_Unicode countryDefaultName[] = { 0x4e, 0x6f, 0x72, 0x77, 0x61, 0x79, 0x0 };
    static const sal_Unicode Variant[] = { 0x0 };

    static const sal_Unicode* LCInfoArray[]
        = { langID, langDefaultName, countryID, countryDefaultName, Variant };

    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_nb_NO(sal_Int16& count)
    {
        count = SAL_N_ELEMENTS(LCInfoArray);
        return (sal_Unicode**)LCInfoArray;
    }
    extern sal_Unicode** SAL_CALL getLocaleItem_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_nb_NO(sal_Int16& count)
    {
        return getLocaleItem_no_NO(count);
    }
    static const sal_Unicode replaceTo0[]
        = { 0x5b, 0x24, 0x6b, 0x72, 0x2d, 0x34, 0x31, 0x34, 0x5d, 0x0 };
    extern sal_Unicode const* const* SAL_CALL getAllFormats0_no_NO(sal_Int16& count,
                                                                   const sal_Unicode*& from,
                                                                   const sal_Unicode*& to);
    SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
    getAllFormats0_nb_NO(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
    {
        to = replaceTo0;
        const sal_Unicode* tmp;
        return getAllFormats0_no_NO(count, from, tmp);
    }
    extern sal_Unicode** SAL_CALL getDateAcceptancePatterns_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_nb_NO(sal_Int16& count)
    {
        return getDateAcceptancePatterns_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getCollatorImplementation_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_nb_NO(sal_Int16& count)
    {
        return getCollatorImplementation_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getCollationOptions_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_nb_NO(sal_Int16& count)
    {
        return getCollationOptions_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getSearchOptions_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_nb_NO(sal_Int16& count)
    {
        return getSearchOptions_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getIndexAlgorithm_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_nb_NO(sal_Int16& count)
    {
        return getIndexAlgorithm_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getUnicodeScripts_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_nb_NO(sal_Int16& count)
    {
        return getUnicodeScripts_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getFollowPageWords_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_nb_NO(sal_Int16& count)
    {
        return getFollowPageWords_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getAllCalendars_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_nb_NO(sal_Int16& count)
    {
        return getAllCalendars_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getAllCurrencies_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_nb_NO(sal_Int16& count)
    {
        return getAllCurrencies_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getTransliterations_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_nb_NO(sal_Int16& count)
    {
        return getTransliterations_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getForbiddenCharacters_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_nb_NO(sal_Int16& count)
    {
        return getForbiddenCharacters_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getBreakIteratorRules_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_nb_NO(sal_Int16& count)
    {
        return getBreakIteratorRules_no_NO(count);
    }
    extern sal_Unicode** SAL_CALL getReservedWords_no_NO(sal_Int16& count);
    SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_nb_NO(sal_Int16& count)
    {
        return getReservedWords_no_NO(count);
    }
    // ---> ContinuousNumbering
    extern const sal_Unicode*** SAL_CALL getContinuousNumberingLevels_no_NO(sal_Int16& nStyles,
                                                                            sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
    getContinuousNumberingLevels_nb_NO(sal_Int16& nStyles, sal_Int16& nAttributes)
    {
        return getContinuousNumberingLevels_no_NO(nStyles, nAttributes);
    }
    // ---> OutlineNumbering
    extern const sal_Unicode**** SAL_CALL getOutlineNumberingLevels_no_NO(sal_Int16& nStyles,
                                                                          sal_Int16& nLevels,
                                                                          sal_Int16& nAttributes);
    SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
    getOutlineNumberingLevels_nb_NO(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
    {
        return getOutlineNumberingLevels_no_NO(nStyles, nLevels, nAttributes);
    }
} // extern "C"
