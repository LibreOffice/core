#include <sal/types.h>

#include <stdio.h>

extern "C" {

static const sal_Unicode langID[] = { 0x7a, 0x68, 0x0 };
static const sal_Unicode langDefaultName[]
    = { 0x54, 0x72, 0x61, 0x64, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x61,
        0x6c, 0x20, 0x43, 0x68, 0x69, 0x6e, 0x65, 0x73, 0x65, 0x0 };
static const sal_Unicode countryID[] = { 0x54, 0x57, 0x0 };
static const sal_Unicode countryDefaultName[] = { 0x54, 0x61, 0x69, 0x77, 0x61, 0x6e, 0x0 };
static const sal_Unicode Variant[] = { 0x0 };

static const sal_Unicode* LCInfoArray[]
    = { langID, langDefaultName, countryID, countryDefaultName, Variant };

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLCInfo_zh_TW(sal_Int16& count)
{
    count = SAL_N_ELEMENTS(LCInfoArray);
    return (sal_Unicode**)LCInfoArray;
}

static const sal_Unicode LC_CTYPE_Unoid[] = { 0x67, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x0 };
static const sal_Unicode dateSeparator[] = { 0x2f, 0x0 };
static const sal_Unicode thousandSeparator[] = { 0x2c, 0x0 };
static const sal_Unicode decimalSeparator[] = { 0x2e, 0x0 };
static const sal_Unicode decimalSeparatorAlternative[] = { 0x0 };
static const sal_Unicode timeSeparator[] = { 0x3a, 0x0 };
static const sal_Unicode time100SecSeparator[] = { 0x2e, 0x0 };
static const sal_Unicode listSeparator[] = { 0x3b, 0x0 };
static const sal_Unicode LongDateDayOfWeekSeparator[] = { 0x0 };
static const sal_Unicode LongDateDaySeparator[] = { 0x65e5, 0x0 };
static const sal_Unicode LongDateMonthSeparator[] = { 0x6708, 0x0 };
static const sal_Unicode LongDateYearSeparator[] = { 0x5e74, 0x0 };
static const sal_Unicode quotationStart[] = { 0x300c, 0x0 };
static const sal_Unicode quotationEnd[] = { 0x300d, 0x0 };
static const sal_Unicode doubleQuotationStart[] = { 0x300e, 0x0 };
static const sal_Unicode doubleQuotationEnd[] = { 0x300f, 0x0 };
static const sal_Unicode timeAM[] = { 0x4e0a, 0x5348, 0x0 };
static const sal_Unicode timePM[] = { 0x4e0b, 0x5348, 0x0 };
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

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getLocaleItem_zh_TW(sal_Int16& count)
{
    count = SAL_N_ELEMENTS(LCType);
    return (sal_Unicode**)LCType;
}
static const sal_Unicode replaceFrom0[] = { 0x0 };
static const sal_Unicode replaceTo0[] = { 0x0 };
static const sal_Unicode FormatKey0[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement0[] = { 1 };
static const sal_Unicode FormatType0[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage0[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex0[] = { 0 };
static const sal_Unicode FormatCode0[] = { 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c, 0x0 };
static const sal_Unicode FormatDefaultName0[] = { 0x0 };
static const sal_Unicode FormatKey1[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement1[] = { 0 };
static const sal_Unicode FormatType1[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage1[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex1[] = { 1 };
static const sal_Unicode FormatCode1[] = { 0x30, 0x0 };
static const sal_Unicode FormatDefaultName1[] = { 0x0 };
static const sal_Unicode FormatKey2[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement2[] = { 0 };
static const sal_Unicode FormatType2[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage2[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex2[] = { 2 };
static const sal_Unicode FormatCode2[] = { 0x30, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName2[] = { 0x0 };
static const sal_Unicode FormatKey3[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement3[] = { 1 };
static const sal_Unicode FormatType3[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage3[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex3[] = { 3 };
static const sal_Unicode FormatCode3[] = { 0x23, 0x2c, 0x23, 0x23, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName3[] = { 0x0 };
static const sal_Unicode FormatKey4[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement4[] = { 0 };
static const sal_Unicode FormatType4[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage4[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex4[] = { 4 };
static const sal_Unicode FormatCode4[] = { 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName4[] = { 0x0 };
static const sal_Unicode FormatKey5[] = { 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x46, 0x6f, 0x72,
                                          0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement5[] = { 0 };
static const sal_Unicode FormatType5[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage5[]
    = { 0x46, 0x49, 0x58, 0x45, 0x44, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex5[] = { 5 };
static const sal_Unicode FormatCode5[] = { 0x23, 0x2c, 0x23, 0x23, 0x23, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName5[] = { 0x0 };
static const sal_Unicode FormatKey6[]
    = { 0x53, 0x63, 0x69, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x63, 0x46,
        0x6f, 0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement6[] = { 0 };
static const sal_Unicode FormatType6[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage6[] = { 0x53, 0x43, 0x49, 0x45, 0x4e, 0x54, 0x49, 0x46, 0x49,
                                            0x43, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex6[] = { 6 };
static const sal_Unicode FormatCode6[]
    = { 0x30, 0x2e, 0x30, 0x30, 0x45, 0x2b, 0x30, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName6[] = { 0x0 };
static const sal_Unicode FormatKey7[]
    = { 0x53, 0x63, 0x69, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x63, 0x46,
        0x6f, 0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement7[] = { 1 };
static const sal_Unicode FormatType7[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage7[] = { 0x53, 0x43, 0x49, 0x45, 0x4e, 0x54, 0x49, 0x46, 0x49,
                                            0x43, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex7[] = { 7 };
static const sal_Unicode FormatCode7[] = { 0x30, 0x2e, 0x30, 0x30, 0x45, 0x2b, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName7[] = { 0x0 };
static const sal_Unicode FormatKey8[]
    = { 0x53, 0x63, 0x69, 0x65, 0x6e, 0x74, 0x69, 0x66, 0x69, 0x63, 0x46,
        0x6f, 0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement8[] = { 0 };
static const sal_Unicode FormatType8[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage8[] = { 0x53, 0x43, 0x49, 0x45, 0x4e, 0x54, 0x49, 0x46, 0x49,
                                            0x43, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex8[] = { 96 };
static const sal_Unicode FormatCode8[]
    = { 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x45, 0x2b, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName8[] = { 0x0 };
static const sal_Unicode FormatKey9[]
    = { 0x50, 0x65, 0x72, 0x63, 0x65, 0x6e, 0x74, 0x46, 0x6f, 0x72,
        0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement9[] = { 1 };
static const sal_Unicode FormatType9[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage9[]
    = { 0x50, 0x45, 0x52, 0x43, 0x45, 0x4e, 0x54, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex9[] = { 8 };
static const sal_Unicode FormatCode9[] = { 0x30, 0x25, 0x0 };
static const sal_Unicode FormatDefaultName9[] = { 0x0 };
static const sal_Unicode FormatKey10[]
    = { 0x50, 0x65, 0x72, 0x63, 0x65, 0x6e, 0x74, 0x46, 0x6f, 0x72,
        0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement10[] = { 1 };
static const sal_Unicode FormatType10[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage10[]
    = { 0x50, 0x45, 0x52, 0x43, 0x45, 0x4e, 0x54, 0x5f, 0x4e, 0x55, 0x4d, 0x42, 0x45, 0x52, 0x0 };
static const sal_Unicode Formatindex10[] = { 9 };
static const sal_Unicode FormatCode10[] = { 0x30, 0x2e, 0x30, 0x30, 0x25, 0x0 };
static const sal_Unicode FormatDefaultName10[] = { 0x0 };
static const sal_Unicode FormatKey11[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement11[] = { 1 };
static const sal_Unicode FormatType11[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage11[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex11[] = { 12 };
static const sal_Unicode FormatCode11[]
    = { 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23,
        0x2c, 0x23, 0x23, 0x30, 0x3b, 0x2d, 0x5b, 0x24, 0x4e, 0x54, 0x24,
        0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName11[] = { 0x0 };
static const sal_Unicode FormatKey12[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement12[] = { 0 };
static const sal_Unicode FormatType12[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage12[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex12[] = { 13 };
static const sal_Unicode FormatCode12[]
    = { 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23,
        0x23, 0x30, 0x2e, 0x30, 0x30, 0x3b, 0x2d, 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d,
        0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName12[] = { 0x0 };
static const sal_Unicode FormatKey13[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement13[] = { 0 };
static const sal_Unicode FormatType13[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage13[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex13[] = { 14 };
static const sal_Unicode FormatCode13[]
    = { 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23,
        0x23, 0x30, 0x3b, 0x5b, 0x52, 0x45, 0x44, 0x5d, 0x2d, 0x5b, 0x24, 0x4e, 0x54,
        0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName13[] = { 0x0 };
static const sal_Unicode FormatKey14[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement14[] = { 1 };
static const sal_Unicode FormatType14[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage14[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex14[] = { 15 };
static const sal_Unicode FormatCode14[]
    = { 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30,
        0x2e, 0x30, 0x30, 0x3b, 0x5b, 0x52, 0x45, 0x44, 0x5d, 0x2d, 0x5b, 0x24, 0x4e, 0x54, 0x24,
        0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName14[] = { 0x0 };
static const sal_Unicode FormatKey15[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement15[] = { 0 };
static const sal_Unicode FormatType15[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage15[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex15[] = { 16 };
static const sal_Unicode FormatCode15[]
    = { 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x20, 0x43, 0x43, 0x43, 0x0 };
static const sal_Unicode FormatDefaultName15[] = { 0x0 };
static const sal_Unicode FormatKey16[]
    = { 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x63, 0x79, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement16[] = { 0 };
static const sal_Unicode FormatType16[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage16[] = { 0x43, 0x55, 0x52, 0x52, 0x45, 0x4e, 0x43, 0x59, 0x0 };
static const sal_Unicode Formatindex16[] = { 17 };
static const sal_Unicode FormatCode16[]
    = { 0x5b, 0x24, 0x4e, 0x54, 0x24, 0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30,
        0x2e, 0x2d, 0x2d, 0x3b, 0x5b, 0x52, 0x45, 0x44, 0x5d, 0x2d, 0x5b, 0x24, 0x4e, 0x54, 0x24,
        0x2d, 0x34, 0x30, 0x34, 0x5d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x2d, 0x2d, 0x0 };
static const sal_Unicode FormatDefaultName16[] = { 0x0 };
static const sal_Unicode FormatKey17[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement17[] = { 1 };
static const sal_Unicode FormatType17[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage17[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex17[] = { 18 };
static const sal_Unicode FormatCode17[] = { 0x59, 0x59, 0x59, 0x59, 0x2f, 0x4d, 0x2f, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName17[] = { 0x0 };
static const sal_Unicode FormatKey18[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement18[] = { 1 };
static const sal_Unicode FormatType18[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage18[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex18[] = { 19 };
static const sal_Unicode FormatCode18[]
    = { 0x59, 0x59, 0x59,   0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,
        0x44, 0x22, 0x65e5, 0x22, 0x20, 0x4e,   0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName18[] = { 0x0 };
static const sal_Unicode FormatKey19[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement19[] = { 0 };
static const sal_Unicode FormatType19[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage19[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex19[] = { 20 };
static const sal_Unicode FormatCode19[] = { 0x45, 0x45, 0x2f, 0x4d, 0x4d, 0x2f, 0x44, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName19[] = { 0x0 };
static const sal_Unicode FormatKey20[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement20[] = { 0 };
static const sal_Unicode FormatType20[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage20[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex20[] = { 21 };
static const sal_Unicode FormatCode20[]
    = { 0x59, 0x59, 0x59, 0x59, 0x2f, 0x4d, 0x4d, 0x2f, 0x44, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName20[] = { 0x0 };
static const sal_Unicode FormatKey21[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement21[] = { 0 };
static const sal_Unicode FormatType21[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage21[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex21[] = { 22 };
static const sal_Unicode FormatCode21[]
    = { 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName21[] = { 0x0 };
static const sal_Unicode FormatKey22[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement22[] = { 0 };
static const sal_Unicode FormatType22[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage22[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex22[] = { 23 };
static const sal_Unicode FormatCode22[] = { 0x59, 0x59, 0x59, 0x59, 0x22,   0x5e74, 0x22, 0x4d,
                                            0x4d, 0x4d, 0x44, 0x22, 0x65e5, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName22[] = { 0x0 };
static const sal_Unicode FormatKey23[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement23[] = { 0 };
static const sal_Unicode FormatType23[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage23[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex23[] = { 24 };
static const sal_Unicode FormatCode23[] = { 0x59, 0x59, 0x59, 0x59, 0x22,   0x5e74, 0x22, 0x4d,
                                            0x4d, 0x4d, 0x44, 0x22, 0x865f, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName23[] = { 0x0 };
static const sal_Unicode FormatKey24[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement24[] = { 0 };
static const sal_Unicode FormatType24[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage24[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex24[] = { 25 };
static const sal_Unicode FormatCode24[]
    = { 0x5b, 0x4e,   0x61, 0x74, 0x4e, 0x75, 0x6d, 0x31, 0x5d, 0x59,   0x59, 0x59, 0x59,
        0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x4d, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName24[] = { 0x0 };
static const sal_Unicode FormatKey25[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x39, 0x0 };
static const sal_Unicode defaultFormatElement25[] = { 0 };
static const sal_Unicode FormatType25[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage25[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex25[] = { 26 };
static const sal_Unicode FormatCode25[]
    = { 0x5b, 0x4e,   0x61, 0x74, 0x4e, 0x75, 0x6d, 0x31, 0x5d, 0x59,   0x59, 0x59, 0x59,
        0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x4d, 0x44, 0x22, 0x865f, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName25[] = { 0x0 };
static const sal_Unicode FormatKey26[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x30, 0x0 };
static const sal_Unicode defaultFormatElement26[] = { 0 };
static const sal_Unicode FormatType26[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage26[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex26[] = { 27 };
static const sal_Unicode FormatCode26[]
    = { 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x44, 0x22, 0x65e5,
        0x20, 0x28, 0x22, 0x44,   0x44, 0x44, 0x22, 0x29, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName26[] = { 0x0 };
static const sal_Unicode FormatKey27[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement27[] = { 0 };
static const sal_Unicode FormatType27[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage27[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex27[] = { 28 };
static const sal_Unicode FormatCode27[]
    = { 0x45,   0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x44, 0x44, 0x22,
        0x65e5, 0x20, 0x28, 0x22,   0x44, 0x44, 0x44, 0x22, 0x29, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName27[] = { 0x0 };
static const sal_Unicode FormatKey28[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement28[] = { 0 };
static const sal_Unicode FormatType28[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage28[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex28[] = { 29 };
static const sal_Unicode FormatCode28[]
    = { 0x5b,   0x4e, 0x61, 0x74,   0x4e, 0x75, 0x6d, 0x31, 0x5d, 0x59, 0x59,
        0x59,   0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x4d, 0x44, 0x22,
        0x65e5, 0x20, 0x28, 0x22,   0x44, 0x44, 0x44, 0x22, 0x29, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName28[] = { 0x0 };
static const sal_Unicode FormatKey29[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement29[] = { 0 };
static const sal_Unicode FormatType29[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage29[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex29[] = { 30 };
static const sal_Unicode FormatCode29[]
    = { 0x5b, 0x4e, 0x61,   0x74, 0x4e,   0x75, 0x6d, 0x31, 0x5d, 0x59,
        0x59, 0x59, 0x59,   0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x4d, 0x4d,
        0x44, 0x22, 0x65e5, 0x22, 0x20,   0x4e, 0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName29[] = { 0x0 };
static const sal_Unicode FormatKey30[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement30[] = { 1 };
static const sal_Unicode FormatType30[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage30[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex30[] = { 31 };
static const sal_Unicode FormatCode30[] = { 0x4d, 0x4d, 0x2d, 0x44, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName30[] = { 0x0 };
static const sal_Unicode FormatKey31[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement31[] = { 0 };
static const sal_Unicode FormatType31[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage31[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex31[] = { 32 };
static const sal_Unicode FormatCode31[] = { 0x59, 0x59, 0x2d, 0x4d, 0x4d, 0x2d, 0x44, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName31[] = { 0x0 };
static const sal_Unicode FormatKey32[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement32[] = { 0 };
static const sal_Unicode FormatType32[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage32[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex32[] = { 33 };
static const sal_Unicode FormatCode32[]
    = { 0x59, 0x59, 0x59, 0x59, 0x2d, 0x4d, 0x4d, 0x2d, 0x44, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName32[]
    = { 0x49, 0x53, 0x4f, 0x20, 0x38, 0x36, 0x30, 0x31, 0x0 };
static const sal_Unicode FormatKey33[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement33[] = { 0 };
static const sal_Unicode FormatType33[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage33[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex33[] = { 34 };
static const sal_Unicode FormatCode33[]
    = { 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName33[] = { 0x0 };
static const sal_Unicode FormatKey34[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement34[] = { 0 };
static const sal_Unicode FormatType34[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage34[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex34[] = { 35 };
static const sal_Unicode FormatCode34[] = { 0x4d, 0x4d, 0x4d, 0x44, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName34[] = { 0x0 };
static const sal_Unicode FormatKey35[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x39, 0x0 };
static const sal_Unicode defaultFormatElement35[] = { 0 };
static const sal_Unicode FormatType35[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage35[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex35[] = { 36 };
static const sal_Unicode FormatCode35[] = { 0x4d, 0x4d, 0x4d, 0x4d, 0x0 };
static const sal_Unicode FormatDefaultName35[] = { 0x0 };
static const sal_Unicode FormatKey36[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x30, 0x0 };
static const sal_Unicode defaultFormatElement36[] = { 0 };
static const sal_Unicode FormatType36[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage36[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex36[] = { 37 };
static const sal_Unicode FormatCode36[]
    = { 0x59, 0x59, 0x59, 0x59, 0x22, 0x5e74, 0x22, 0x51, 0x51, 0x0 };
static const sal_Unicode FormatDefaultName36[] = { 0x0 };
static const sal_Unicode FormatKey37[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement37[] = { 0 };
static const sal_Unicode FormatType37[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage37[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex37[] = { 38 };
static const sal_Unicode FormatCode37[]
    = { 0x22, 0x7b2c, 0x22, 0x57, 0x57, 0x22, 0x9031, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName37[] = { 0x0 };
static const sal_Unicode FormatKey38[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement38[] = { 0 };
static const sal_Unicode FormatType38[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage38[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex38[] = { 66 };
static const sal_Unicode FormatCode38[] = { 0x45, 0x2f, 0x4d, 0x2f, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName38[] = { 0x0 };
static const sal_Unicode FormatKey39[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement39[] = { 0 };
static const sal_Unicode FormatType39[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage39[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex39[] = { 67 };
static const sal_Unicode FormatCode39[] = { 0x47, 0x47,   0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d,
                                            0x22, 0x6708, 0x22, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName39[] = { 0x0 };
static const sal_Unicode FormatKey40[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement40[] = { 0 };
static const sal_Unicode FormatType40[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage40[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex40[] = { 68 };
static const sal_Unicode FormatCode40[]
    = { 0x5b, 0x4e,   0x61, 0x74, 0x4e, 0x75,   0x6d, 0x31, 0x5d, 0x47,   0x47, 0x45, 0x45,
        0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName40[] = { 0x0 };
static const sal_Unicode FormatKey41[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement41[] = { 0 };
static const sal_Unicode FormatType41[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage41[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex41[] = { 69 };
static const sal_Unicode FormatCode41[]
    = { 0x47, 0x47,   0x47, 0x45, 0x45, 0x22,   0x5e74, 0x22, 0x4d,
        0x22, 0x6708, 0x22, 0x44, 0x22, 0x65e5, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName41[] = { 0x0 };
static const sal_Unicode FormatKey42[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement42[] = { 0 };
static const sal_Unicode FormatType42[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage42[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex42[] = { 70 };
static const sal_Unicode FormatCode42[]
    = { 0x5b, 0x4e, 0x61,   0x74, 0x4e, 0x75, 0x6d,   0x31, 0x5d, 0x47, 0x47,   0x47, 0x45,
        0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44, 0x22, 0x65e5, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName42[] = { 0x0 };
static const sal_Unicode FormatKey43[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement43[] = { 0 };
static const sal_Unicode FormatType43[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage43[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex43[] = { 71 };
static const sal_Unicode FormatCode43[]
    = { 0x47, 0x47,   0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44,
        0x22, 0x65e5, 0x20, 0x28, 0x22, 0x44,   0x44, 0x44, 0x22, 0x29,   0x22, 0x0 };
static const sal_Unicode FormatDefaultName43[] = { 0x0 };
static const sal_Unicode FormatKey44[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement44[] = { 0 };
static const sal_Unicode FormatType44[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage44[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex44[] = { 72 };
static const sal_Unicode FormatCode44[]
    = { 0x47, 0x47,   0x47, 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44,
        0x22, 0x65e5, 0x20, 0x28, 0x22, 0x44, 0x44,   0x44, 0x22, 0x29, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName44[] = { 0x0 };
static const sal_Unicode FormatKey45[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x39, 0x0 };
static const sal_Unicode defaultFormatElement45[] = { 0 };
static const sal_Unicode FormatType45[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage45[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex45[] = { 73 };
static const sal_Unicode FormatCode45[]
    = { 0x59, 0x59,   0x59, 0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44,
        0x22, 0x65e5, 0x20, 0x28, 0x22, 0x44,   0x44, 0x44, 0x22, 0x29,   0x22, 0x0 };
static const sal_Unicode FormatDefaultName45[] = { 0x0 };
static const sal_Unicode FormatKey46[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x30, 0x0 };
static const sal_Unicode defaultFormatElement46[] = { 0 };
static const sal_Unicode FormatType46[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage46[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex46[] = { 74 };
static const sal_Unicode FormatCode46[]
    = { 0x59, 0x59, 0x59,   0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,
        0x44, 0x22, 0x65e5, 0x20, 0x22, 0x4e,   0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName46[] = { 0x0 };
static const sal_Unicode FormatKey47[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement47[] = { 0 };
static const sal_Unicode FormatType47[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage47[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex47[] = { 75 };
static const sal_Unicode FormatCode47[]
    = { 0x47, 0x47, 0x45,   0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,
        0x44, 0x22, 0x65e5, 0x20, 0x22, 0x4e,   0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName47[] = { 0x0 };
static const sal_Unicode FormatKey48[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement48[] = { 0 };
static const sal_Unicode FormatType48[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage48[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex48[] = { 76 };
static const sal_Unicode FormatCode48[]
    = { 0x5b, 0x4e, 0x61,   0x74, 0x4e,   0x75, 0x6d, 0x31, 0x5d,   0x47,
        0x47, 0x45, 0x45,   0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,
        0x44, 0x22, 0x65e5, 0x20, 0x22,   0x4e, 0x4e, 0x4e, 0x4e,   0x0 };
static const sal_Unicode FormatDefaultName48[] = { 0x0 };
static const sal_Unicode FormatKey49[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement49[] = { 0 };
static const sal_Unicode FormatType49[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage49[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex49[] = { 77 };
static const sal_Unicode FormatCode49[]
    = { 0x47, 0x47, 0x47, 0x45,   0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708,
        0x22, 0x44, 0x22, 0x65e5, 0x20, 0x22, 0x4e,   0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName49[] = { 0x0 };
static const sal_Unicode FormatKey50[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement50[] = { 0 };
static const sal_Unicode FormatType50[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage50[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex50[] = { 78 };
static const sal_Unicode FormatCode50[]
    = { 0x5b, 0x4e,   0x61, 0x74, 0x4e,   0x75, 0x6d, 0x31, 0x5d,   0x47, 0x47,
        0x47, 0x45,   0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x44,
        0x22, 0x65e5, 0x20, 0x22, 0x4e,   0x4e, 0x4e, 0x4e, 0x0 };
static const sal_Unicode FormatDefaultName50[] = { 0x0 };
static const sal_Unicode FormatKey51[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement51[] = { 0 };
static const sal_Unicode FormatType51[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage51[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex51[] = { 79 };
static const sal_Unicode FormatCode51[]
    = { 0x5b, 0x4e, 0x61,   0x74, 0x4e, 0x75, 0x6d,   0x31, 0x5d, 0x45,
        0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName51[] = { 0x0 };
static const sal_Unicode FormatKey52[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement52[] = { 0 };
static const sal_Unicode FormatType52[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage52[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex52[] = { 80 };
static const sal_Unicode FormatCode52[]
    = { 0x5b, 0x4e, 0x61, 0x74,   0x4e, 0x75, 0x6d, 0x31,   0x5d, 0x47, 0x47,
        0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName52[] = { 0x0 };
static const sal_Unicode FormatKey53[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement53[] = { 0 };
static const sal_Unicode FormatType53[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage53[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex53[] = { 81 };
static const sal_Unicode FormatCode53[]
    = { 0x5b, 0x4e, 0x61, 0x74, 0x4e,   0x75, 0x6d, 0x31, 0x5d,   0x47, 0x47,
        0x47, 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName53[] = { 0x0 };
static const sal_Unicode FormatKey54[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement54[] = { 0 };
static const sal_Unicode FormatType54[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage54[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex54[] = { 82 };
static const sal_Unicode FormatCode54[]
    = { 0x59, 0x59, 0x59, 0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName54[] = { 0x0 };
static const sal_Unicode FormatKey55[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x39, 0x0 };
static const sal_Unicode defaultFormatElement55[] = { 0 };
static const sal_Unicode FormatType55[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage55[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex55[] = { 83 };
static const sal_Unicode FormatCode55[]
    = { 0x5b, 0x4e, 0x61, 0x74,   0x4e, 0x75, 0x6d, 0x31, 0x5d,   0x59, 0x59,
        0x59, 0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x4d, 0x22, 0x6708, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName55[] = { 0x0 };
static const sal_Unicode FormatKey56[] = { 0x44, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x30, 0x0 };
static const sal_Unicode defaultFormatElement56[] = { 0 };
static const sal_Unicode FormatType56[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage56[] = { 0x44, 0x41, 0x54, 0x45, 0x0 };
static const sal_Unicode Formatindex56[] = { 84 };
static const sal_Unicode FormatCode56[] = { 0x4d, 0x22, 0x2e, 0x22, 0x44, 0x0 };
static const sal_Unicode FormatDefaultName56[] = { 0x0 };
static const sal_Unicode FormatKey57[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement57[] = { 1 };
static const sal_Unicode FormatType57[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage57[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex57[] = { 39 };
static const sal_Unicode FormatCode57[] = { 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x0 };
static const sal_Unicode FormatDefaultName57[] = { 0x0 };
static const sal_Unicode FormatKey58[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement58[] = { 1 };
static const sal_Unicode FormatType58[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage58[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex58[] = { 40 };
static const sal_Unicode FormatCode58[] = { 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x0 };
static const sal_Unicode FormatDefaultName58[] = { 0x0 };
static const sal_Unicode FormatKey59[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement59[] = { 0 };
static const sal_Unicode FormatType59[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage59[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex59[] = { 41 };
static const sal_Unicode FormatCode59[]
    = { 0x41, 0x4d, 0x2f, 0x50, 0x4d, 0x20, 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x0 };
static const sal_Unicode FormatDefaultName59[] = { 0x0 };
static const sal_Unicode FormatKey60[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement60[] = { 0 };
static const sal_Unicode FormatType60[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage60[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex60[] = { 42 };
static const sal_Unicode FormatCode60[]
    = { 0x41, 0x4d, 0x2f, 0x50, 0x4d, 0x20, 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x0 };
static const sal_Unicode FormatDefaultName60[] = { 0x0 };
static const sal_Unicode FormatKey61[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement61[] = { 0 };
static const sal_Unicode FormatType61[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage61[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex61[] = { 43 };
static const sal_Unicode FormatCode61[]
    = { 0x5b, 0x48, 0x48, 0x5d, 0x3a, 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x0 };
static const sal_Unicode FormatDefaultName61[] = { 0x0 };
static const sal_Unicode FormatKey62[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement62[] = { 0 };
static const sal_Unicode FormatType62[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage62[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex62[] = { 44 };
static const sal_Unicode FormatCode62[] = { 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName62[] = { 0x0 };
static const sal_Unicode FormatKey63[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement63[] = { 1 };
static const sal_Unicode FormatType63[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage63[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex63[] = { 45 };
static const sal_Unicode FormatCode63[]
    = { 0x5b, 0x48, 0x48, 0x5d, 0x3a, 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x2e, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName63[] = { 0x0 };
static const sal_Unicode FormatKey64[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement64[] = { 0 };
static const sal_Unicode FormatType64[] = { 0x73, 0x68, 0x6f, 0x72, 0x74, 0x0 };
static const sal_Unicode FormatUsage64[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex64[] = { 85 };
static const sal_Unicode FormatCode64[]
    = { 0x48, 0x48, 0x22, 0x6642, 0x22, 0x4d, 0x4d, 0x22, 0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName64[] = { 0x0 };
static const sal_Unicode FormatKey65[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d,
                                           0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x39, 0x0 };
static const sal_Unicode defaultFormatElement65[] = { 0 };
static const sal_Unicode FormatType65[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage65[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex65[] = { 86 };
static const sal_Unicode FormatCode65[] = { 0x48,   0x48, 0x22, 0x6642, 0x22, 0x4d,   0x4d, 0x22,
                                            0x5206, 0x22, 0x53, 0x53,   0x22, 0x79d2, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName65[] = { 0x0 };
static const sal_Unicode FormatKey66[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x30, 0x0 };
static const sal_Unicode defaultFormatElement66[] = { 0 };
static const sal_Unicode FormatType66[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage66[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex66[] = { 87 };
static const sal_Unicode FormatCode66[]
    = { 0x41,   0x4d, 0x2f, 0x50, 0x4d, 0x20,   0x48, 0x48, 0x22,
        0x6642, 0x22, 0x4d, 0x4d, 0x22, 0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName66[] = { 0x0 };
static const sal_Unicode FormatKey67[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement67[] = { 0 };
static const sal_Unicode FormatType67[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage67[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex67[] = { 88 };
static const sal_Unicode FormatCode67[]
    = { 0x41, 0x4d, 0x2f, 0x50,   0x4d, 0x20, 0x48, 0x48, 0x22,   0x6642, 0x22,
        0x4d, 0x4d, 0x22, 0x5206, 0x22, 0x53, 0x53, 0x22, 0x79d2, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName67[] = { 0x0 };
static const sal_Unicode FormatKey68[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement68[] = { 0 };
static const sal_Unicode FormatType68[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage68[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex68[] = { 89 };
static const sal_Unicode FormatCode68[]
    = { 0x5b, 0x48,   0x48, 0x5d, 0x20, 0x22, 0x6642, 0x22, 0x4d, 0x4d,
        0x22, 0x5206, 0x22, 0x53, 0x53, 0x22, 0x79d2, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName68[] = { 0x0 };
static const sal_Unicode FormatKey69[] = { 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61,
                                           0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement69[] = { 0 };
static const sal_Unicode FormatType69[] = { 0x6d, 0x65, 0x64, 0x69, 0x75, 0x6d, 0x0 };
static const sal_Unicode FormatUsage69[] = { 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex69[] = { 90 };
static const sal_Unicode FormatCode69[]
    = { 0x4d, 0x4d, 0x22, 0x5206, 0x22, 0x53, 0x53, 0x22, 0x79d2, 0x22, 0x30, 0x30, 0x0 };
static const sal_Unicode FormatDefaultName69[] = { 0x0 };
static const sal_Unicode FormatKey70[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x0 };
static const sal_Unicode defaultFormatElement70[] = { 0 };
static const sal_Unicode FormatType70[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage70[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex70[] = { 46 };
static const sal_Unicode FormatCode70[]
    = { 0x59, 0x59, 0x59, 0x59, 0x2f, 0x4d, 0x2f, 0x44, 0x20, 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x0 };
static const sal_Unicode FormatDefaultName70[] = { 0x0 };
static const sal_Unicode FormatKey71[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x32, 0x0 };
static const sal_Unicode defaultFormatElement71[] = { 0 };
static const sal_Unicode FormatType71[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage71[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex71[] = { 47 };
static const sal_Unicode FormatCode71[]
    = { 0x59, 0x59, 0x59, 0x59, 0x2f, 0x4d, 0x4d, 0x2f, 0x44, 0x44,
        0x20, 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x3a, 0x53, 0x53, 0x0 };
static const sal_Unicode FormatDefaultName71[] = { 0x0 };
static const sal_Unicode FormatKey72[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x38, 0x0 };
static const sal_Unicode defaultFormatElement72[] = { 0 };
static const sal_Unicode FormatType72[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage72[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex72[] = { 50 };
static const sal_Unicode FormatCode72[]
    = { 0x59, 0x59, 0x59, 0x59, 0x2f, 0x4d, 0x2f, 0x44, 0x20, 0x48, 0x48, 0x3a, 0x4d, 0x4d, 0x0 };
static const sal_Unicode FormatDefaultName72[] = { 0x0 };
static const sal_Unicode FormatKey73[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x33, 0x0 };
static const sal_Unicode defaultFormatElement73[] = { 1 };
static const sal_Unicode FormatType73[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage73[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex73[] = { 91 };
static const sal_Unicode FormatCode73[]
    = { 0x59, 0x59, 0x59, 0x59, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,   0x44, 0x22, 0x65e5,
        0x22, 0x20, 0x48, 0x48, 0x22, 0x6642, 0x22, 0x4d, 0x4d, 0x22,   0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName73[] = { 0x0 };
static const sal_Unicode FormatKey74[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x34, 0x0 };
static const sal_Unicode defaultFormatElement74[] = { 0 };
static const sal_Unicode FormatType74[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage74[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex74[] = { 92 };
static const sal_Unicode FormatCode74[]
    = { 0x59, 0x59, 0x59,   0x59, 0x22, 0x5e74, 0x22, 0x4d,   0x22,   0x6708, 0x22,
        0x44, 0x22, 0x65e5, 0x22, 0x20, 0x48,   0x48, 0x22,   0x6642, 0x22,   0x4d,
        0x4d, 0x22, 0x5206, 0x22, 0x53, 0x53,   0x22, 0x79d2, 0x22,   0x0 };
static const sal_Unicode FormatDefaultName74[] = { 0x0 };
static const sal_Unicode FormatKey75[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x35, 0x0 };
static const sal_Unicode defaultFormatElement75[] = { 0 };
static const sal_Unicode FormatType75[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage75[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex75[] = { 93 };
static const sal_Unicode FormatCode75[]
    = { 0x45, 0x45, 0x22, 0x5e74, 0x22,   0x4d, 0x22, 0x6708, 0x22, 0x44,   0x22, 0x65e5, 0x22,
        0x20, 0x48, 0x48, 0x22,   0x6642, 0x22, 0x4d, 0x4d,   0x22, 0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName75[] = { 0x0 };
static const sal_Unicode FormatKey76[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x36, 0x0 };
static const sal_Unicode defaultFormatElement76[] = { 0 };
static const sal_Unicode FormatType76[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage76[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex76[] = { 94 };
static const sal_Unicode FormatCode76[]
    = { 0x47, 0x47, 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,   0x44, 0x22, 0x65e5,
        0x22, 0x20, 0x48, 0x48, 0x22, 0x6642, 0x22, 0x4d, 0x4d, 0x22,   0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName76[] = { 0x0 };
static const sal_Unicode FormatKey77[]
    = { 0x44, 0x61, 0x74, 0x65, 0x54, 0x69, 0x6d, 0x65, 0x46, 0x6f,
        0x72, 0x6d, 0x61, 0x74, 0x73, 0x6b, 0x65, 0x79, 0x37, 0x0 };
static const sal_Unicode defaultFormatElement77[] = { 0 };
static const sal_Unicode FormatType77[] = { 0x6c, 0x6f, 0x6e, 0x67, 0x0 };
static const sal_Unicode FormatUsage77[]
    = { 0x44, 0x41, 0x54, 0x45, 0x5f, 0x54, 0x49, 0x4d, 0x45, 0x0 };
static const sal_Unicode Formatindex77[] = { 95 };
static const sal_Unicode FormatCode77[]
    = { 0x47,   0x47, 0x47, 0x45, 0x45, 0x22, 0x5e74, 0x22, 0x4d, 0x22, 0x6708, 0x22,   0x44, 0x22,
        0x65e5, 0x22, 0x20, 0x48, 0x48, 0x22, 0x6642, 0x22, 0x4d, 0x4d, 0x22,   0x5206, 0x22, 0x0 };
static const sal_Unicode FormatDefaultName77[] = { 0x0 };

static const sal_Int16 FormatElementsCount0 = 78;
static const sal_Unicode* FormatElementsArray0[] = {
    FormatCode0,
    FormatDefaultName0,
    FormatKey0,
    FormatType0,
    FormatUsage0,
    Formatindex0,
    defaultFormatElement0,
    FormatCode1,
    FormatDefaultName1,
    FormatKey1,
    FormatType1,
    FormatUsage1,
    Formatindex1,
    defaultFormatElement1,
    FormatCode2,
    FormatDefaultName2,
    FormatKey2,
    FormatType2,
    FormatUsage2,
    Formatindex2,
    defaultFormatElement2,
    FormatCode3,
    FormatDefaultName3,
    FormatKey3,
    FormatType3,
    FormatUsage3,
    Formatindex3,
    defaultFormatElement3,
    FormatCode4,
    FormatDefaultName4,
    FormatKey4,
    FormatType4,
    FormatUsage4,
    Formatindex4,
    defaultFormatElement4,
    FormatCode5,
    FormatDefaultName5,
    FormatKey5,
    FormatType5,
    FormatUsage5,
    Formatindex5,
    defaultFormatElement5,
    FormatCode6,
    FormatDefaultName6,
    FormatKey6,
    FormatType6,
    FormatUsage6,
    Formatindex6,
    defaultFormatElement6,
    FormatCode7,
    FormatDefaultName7,
    FormatKey7,
    FormatType7,
    FormatUsage7,
    Formatindex7,
    defaultFormatElement7,
    FormatCode8,
    FormatDefaultName8,
    FormatKey8,
    FormatType8,
    FormatUsage8,
    Formatindex8,
    defaultFormatElement8,
    FormatCode9,
    FormatDefaultName9,
    FormatKey9,
    FormatType9,
    FormatUsage9,
    Formatindex9,
    defaultFormatElement9,
    FormatCode10,
    FormatDefaultName10,
    FormatKey10,
    FormatType10,
    FormatUsage10,
    Formatindex10,
    defaultFormatElement10,
    FormatCode11,
    FormatDefaultName11,
    FormatKey11,
    FormatType11,
    FormatUsage11,
    Formatindex11,
    defaultFormatElement11,
    FormatCode12,
    FormatDefaultName12,
    FormatKey12,
    FormatType12,
    FormatUsage12,
    Formatindex12,
    defaultFormatElement12,
    FormatCode13,
    FormatDefaultName13,
    FormatKey13,
    FormatType13,
    FormatUsage13,
    Formatindex13,
    defaultFormatElement13,
    FormatCode14,
    FormatDefaultName14,
    FormatKey14,
    FormatType14,
    FormatUsage14,
    Formatindex14,
    defaultFormatElement14,
    FormatCode15,
    FormatDefaultName15,
    FormatKey15,
    FormatType15,
    FormatUsage15,
    Formatindex15,
    defaultFormatElement15,
    FormatCode16,
    FormatDefaultName16,
    FormatKey16,
    FormatType16,
    FormatUsage16,
    Formatindex16,
    defaultFormatElement16,
    FormatCode17,
    FormatDefaultName17,
    FormatKey17,
    FormatType17,
    FormatUsage17,
    Formatindex17,
    defaultFormatElement17,
    FormatCode18,
    FormatDefaultName18,
    FormatKey18,
    FormatType18,
    FormatUsage18,
    Formatindex18,
    defaultFormatElement18,
    FormatCode19,
    FormatDefaultName19,
    FormatKey19,
    FormatType19,
    FormatUsage19,
    Formatindex19,
    defaultFormatElement19,
    FormatCode20,
    FormatDefaultName20,
    FormatKey20,
    FormatType20,
    FormatUsage20,
    Formatindex20,
    defaultFormatElement20,
    FormatCode21,
    FormatDefaultName21,
    FormatKey21,
    FormatType21,
    FormatUsage21,
    Formatindex21,
    defaultFormatElement21,
    FormatCode22,
    FormatDefaultName22,
    FormatKey22,
    FormatType22,
    FormatUsage22,
    Formatindex22,
    defaultFormatElement22,
    FormatCode23,
    FormatDefaultName23,
    FormatKey23,
    FormatType23,
    FormatUsage23,
    Formatindex23,
    defaultFormatElement23,
    FormatCode24,
    FormatDefaultName24,
    FormatKey24,
    FormatType24,
    FormatUsage24,
    Formatindex24,
    defaultFormatElement24,
    FormatCode25,
    FormatDefaultName25,
    FormatKey25,
    FormatType25,
    FormatUsage25,
    Formatindex25,
    defaultFormatElement25,
    FormatCode26,
    FormatDefaultName26,
    FormatKey26,
    FormatType26,
    FormatUsage26,
    Formatindex26,
    defaultFormatElement26,
    FormatCode27,
    FormatDefaultName27,
    FormatKey27,
    FormatType27,
    FormatUsage27,
    Formatindex27,
    defaultFormatElement27,
    FormatCode28,
    FormatDefaultName28,
    FormatKey28,
    FormatType28,
    FormatUsage28,
    Formatindex28,
    defaultFormatElement28,
    FormatCode29,
    FormatDefaultName29,
    FormatKey29,
    FormatType29,
    FormatUsage29,
    Formatindex29,
    defaultFormatElement29,
    FormatCode30,
    FormatDefaultName30,
    FormatKey30,
    FormatType30,
    FormatUsage30,
    Formatindex30,
    defaultFormatElement30,
    FormatCode31,
    FormatDefaultName31,
    FormatKey31,
    FormatType31,
    FormatUsage31,
    Formatindex31,
    defaultFormatElement31,
    FormatCode32,
    FormatDefaultName32,
    FormatKey32,
    FormatType32,
    FormatUsage32,
    Formatindex32,
    defaultFormatElement32,
    FormatCode33,
    FormatDefaultName33,
    FormatKey33,
    FormatType33,
    FormatUsage33,
    Formatindex33,
    defaultFormatElement33,
    FormatCode34,
    FormatDefaultName34,
    FormatKey34,
    FormatType34,
    FormatUsage34,
    Formatindex34,
    defaultFormatElement34,
    FormatCode35,
    FormatDefaultName35,
    FormatKey35,
    FormatType35,
    FormatUsage35,
    Formatindex35,
    defaultFormatElement35,
    FormatCode36,
    FormatDefaultName36,
    FormatKey36,
    FormatType36,
    FormatUsage36,
    Formatindex36,
    defaultFormatElement36,
    FormatCode37,
    FormatDefaultName37,
    FormatKey37,
    FormatType37,
    FormatUsage37,
    Formatindex37,
    defaultFormatElement37,
    FormatCode38,
    FormatDefaultName38,
    FormatKey38,
    FormatType38,
    FormatUsage38,
    Formatindex38,
    defaultFormatElement38,
    FormatCode39,
    FormatDefaultName39,
    FormatKey39,
    FormatType39,
    FormatUsage39,
    Formatindex39,
    defaultFormatElement39,
    FormatCode40,
    FormatDefaultName40,
    FormatKey40,
    FormatType40,
    FormatUsage40,
    Formatindex40,
    defaultFormatElement40,
    FormatCode41,
    FormatDefaultName41,
    FormatKey41,
    FormatType41,
    FormatUsage41,
    Formatindex41,
    defaultFormatElement41,
    FormatCode42,
    FormatDefaultName42,
    FormatKey42,
    FormatType42,
    FormatUsage42,
    Formatindex42,
    defaultFormatElement42,
    FormatCode43,
    FormatDefaultName43,
    FormatKey43,
    FormatType43,
    FormatUsage43,
    Formatindex43,
    defaultFormatElement43,
    FormatCode44,
    FormatDefaultName44,
    FormatKey44,
    FormatType44,
    FormatUsage44,
    Formatindex44,
    defaultFormatElement44,
    FormatCode45,
    FormatDefaultName45,
    FormatKey45,
    FormatType45,
    FormatUsage45,
    Formatindex45,
    defaultFormatElement45,
    FormatCode46,
    FormatDefaultName46,
    FormatKey46,
    FormatType46,
    FormatUsage46,
    Formatindex46,
    defaultFormatElement46,
    FormatCode47,
    FormatDefaultName47,
    FormatKey47,
    FormatType47,
    FormatUsage47,
    Formatindex47,
    defaultFormatElement47,
    FormatCode48,
    FormatDefaultName48,
    FormatKey48,
    FormatType48,
    FormatUsage48,
    Formatindex48,
    defaultFormatElement48,
    FormatCode49,
    FormatDefaultName49,
    FormatKey49,
    FormatType49,
    FormatUsage49,
    Formatindex49,
    defaultFormatElement49,
    FormatCode50,
    FormatDefaultName50,
    FormatKey50,
    FormatType50,
    FormatUsage50,
    Formatindex50,
    defaultFormatElement50,
    FormatCode51,
    FormatDefaultName51,
    FormatKey51,
    FormatType51,
    FormatUsage51,
    Formatindex51,
    defaultFormatElement51,
    FormatCode52,
    FormatDefaultName52,
    FormatKey52,
    FormatType52,
    FormatUsage52,
    Formatindex52,
    defaultFormatElement52,
    FormatCode53,
    FormatDefaultName53,
    FormatKey53,
    FormatType53,
    FormatUsage53,
    Formatindex53,
    defaultFormatElement53,
    FormatCode54,
    FormatDefaultName54,
    FormatKey54,
    FormatType54,
    FormatUsage54,
    Formatindex54,
    defaultFormatElement54,
    FormatCode55,
    FormatDefaultName55,
    FormatKey55,
    FormatType55,
    FormatUsage55,
    Formatindex55,
    defaultFormatElement55,
    FormatCode56,
    FormatDefaultName56,
    FormatKey56,
    FormatType56,
    FormatUsage56,
    Formatindex56,
    defaultFormatElement56,
    FormatCode57,
    FormatDefaultName57,
    FormatKey57,
    FormatType57,
    FormatUsage57,
    Formatindex57,
    defaultFormatElement57,
    FormatCode58,
    FormatDefaultName58,
    FormatKey58,
    FormatType58,
    FormatUsage58,
    Formatindex58,
    defaultFormatElement58,
    FormatCode59,
    FormatDefaultName59,
    FormatKey59,
    FormatType59,
    FormatUsage59,
    Formatindex59,
    defaultFormatElement59,
    FormatCode60,
    FormatDefaultName60,
    FormatKey60,
    FormatType60,
    FormatUsage60,
    Formatindex60,
    defaultFormatElement60,
    FormatCode61,
    FormatDefaultName61,
    FormatKey61,
    FormatType61,
    FormatUsage61,
    Formatindex61,
    defaultFormatElement61,
    FormatCode62,
    FormatDefaultName62,
    FormatKey62,
    FormatType62,
    FormatUsage62,
    Formatindex62,
    defaultFormatElement62,
    FormatCode63,
    FormatDefaultName63,
    FormatKey63,
    FormatType63,
    FormatUsage63,
    Formatindex63,
    defaultFormatElement63,
    FormatCode64,
    FormatDefaultName64,
    FormatKey64,
    FormatType64,
    FormatUsage64,
    Formatindex64,
    defaultFormatElement64,
    FormatCode65,
    FormatDefaultName65,
    FormatKey65,
    FormatType65,
    FormatUsage65,
    Formatindex65,
    defaultFormatElement65,
    FormatCode66,
    FormatDefaultName66,
    FormatKey66,
    FormatType66,
    FormatUsage66,
    Formatindex66,
    defaultFormatElement66,
    FormatCode67,
    FormatDefaultName67,
    FormatKey67,
    FormatType67,
    FormatUsage67,
    Formatindex67,
    defaultFormatElement67,
    FormatCode68,
    FormatDefaultName68,
    FormatKey68,
    FormatType68,
    FormatUsage68,
    Formatindex68,
    defaultFormatElement68,
    FormatCode69,
    FormatDefaultName69,
    FormatKey69,
    FormatType69,
    FormatUsage69,
    Formatindex69,
    defaultFormatElement69,
    FormatCode70,
    FormatDefaultName70,
    FormatKey70,
    FormatType70,
    FormatUsage70,
    Formatindex70,
    defaultFormatElement70,
    FormatCode71,
    FormatDefaultName71,
    FormatKey71,
    FormatType71,
    FormatUsage71,
    Formatindex71,
    defaultFormatElement71,
    FormatCode72,
    FormatDefaultName72,
    FormatKey72,
    FormatType72,
    FormatUsage72,
    Formatindex72,
    defaultFormatElement72,
    FormatCode73,
    FormatDefaultName73,
    FormatKey73,
    FormatType73,
    FormatUsage73,
    Formatindex73,
    defaultFormatElement73,
    FormatCode74,
    FormatDefaultName74,
    FormatKey74,
    FormatType74,
    FormatUsage74,
    Formatindex74,
    defaultFormatElement74,
    FormatCode75,
    FormatDefaultName75,
    FormatKey75,
    FormatType75,
    FormatUsage75,
    Formatindex75,
    defaultFormatElement75,
    FormatCode76,
    FormatDefaultName76,
    FormatKey76,
    FormatType76,
    FormatUsage76,
    Formatindex76,
    defaultFormatElement76,
    FormatCode77,
    FormatDefaultName77,
    FormatKey77,
    FormatType77,
    FormatUsage77,
    Formatindex77,
    defaultFormatElement77,
};

SAL_DLLPUBLIC_EXPORT sal_Unicode const* const* SAL_CALL
getAllFormats0_zh_TW(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
{
    count = FormatElementsCount0;
    from = replaceFrom0;
    to = replaceTo0;
    return (sal_Unicode**)FormatElementsArray0;
}
static const sal_Unicode DateAcceptancePattern0[] = { 0x59, 0x2f, 0x4d, 0x2f, 0x44, 0x0 };
static const sal_Unicode DateAcceptancePattern1[] = { 0x4d, 0x6708, 0x44, 0x65e5, 0x0 };
static const sal_Unicode DateAcceptancePattern2[] = { 0x4d, 0x2d, 0x44, 0x0 };
static const sal_Unicode DateAcceptancePattern3[] = { 0x4d, 0x2f, 0x44, 0x0 };
static const sal_Unicode DateAcceptancePattern4[]
    = { 0x59, 0x5e74, 0x4d, 0x6708, 0x44, 0x65e5, 0x0 };
static const sal_Unicode DateAcceptancePattern5[] = { 0x59, 0x2e, 0x4d, 0x2e, 0x44, 0x0 };
static const sal_Int16 DateAcceptancePatternsCount = 6;
static const sal_Unicode* DateAcceptancePatternsArray[] = {
    DateAcceptancePattern0, DateAcceptancePattern1, DateAcceptancePattern2,
    DateAcceptancePattern3, DateAcceptancePattern4, DateAcceptancePattern5,
};

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getDateAcceptancePatterns_zh_TW(sal_Int16& count)
{
    count = DateAcceptancePatternsCount;
    return (sal_Unicode**)DateAcceptancePatternsArray;
}
static const sal_Unicode CollatorID0[] = { 0x73, 0x74, 0x72, 0x6f, 0x6b, 0x65, 0x0 };
static const sal_Unicode CollatorRule0[] = { 0x0 };
static const sal_Unicode defaultCollator0[] = { 1 };

static const sal_Unicode CollatorID1[] = { 0x72, 0x61, 0x64, 0x69, 0x63, 0x61, 0x6c, 0x0 };
static const sal_Unicode CollatorRule1[] = { 0x0 };
static const sal_Unicode defaultCollator1[] = { 0 };

static const sal_Unicode CollatorID2[] = { 0x7a, 0x68, 0x75, 0x79, 0x69, 0x6e, 0x0 };
static const sal_Unicode CollatorRule2[] = { 0x0 };
static const sal_Unicode defaultCollator2[] = { 0 };

static const sal_Unicode CollatorID3[] = { 0x70, 0x69, 0x6e, 0x79, 0x69, 0x6e, 0x0 };
static const sal_Unicode CollatorRule3[] = { 0x0 };
static const sal_Unicode defaultCollator3[] = { 0 };

static const sal_Unicode collationOption0[]
    = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
static const sal_Int16 nbOfCollationOptions = 1;

static const sal_Int16 nbOfCollations = 4;

static const sal_Unicode* LCCollatorArray[] = {
    CollatorID0, defaultCollator0, CollatorRule0, CollatorID1, defaultCollator1, CollatorRule1,
    CollatorID2, defaultCollator2, CollatorRule2, CollatorID3, defaultCollator3, CollatorRule3,
};

static const sal_Unicode* collationOptions[] = { collationOption0, NULL };
SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollatorImplementation_zh_TW(sal_Int16& count)
{
    count = nbOfCollations;
    return (sal_Unicode**)LCCollatorArray;
}
SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getCollationOptions_zh_TW(sal_Int16& count)
{
    count = nbOfCollationOptions;
    return (sal_Unicode**)collationOptions;
}
extern sal_Unicode** SAL_CALL getSearchOptions_zh_CN(sal_Int16& count);
SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getSearchOptions_zh_TW(sal_Int16& count)
{
    return getSearchOptions_zh_CN(count);
}
static const sal_Unicode IndexID0[] = { 0x73, 0x74, 0x72, 0x6f, 0x6b, 0x65, 0x0 };
static const sal_Unicode IndexModule0[] = { 0x61, 0x73, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode IndexKey0[] = { 0x0 };
static const sal_Unicode defaultIndex0[] = { 1 };
static const sal_Unicode defaultPhonetic0[] = { 0 };

static const sal_Unicode IndexID1[] = { 0x72, 0x61, 0x64, 0x69, 0x63, 0x61, 0x6c, 0x0 };
static const sal_Unicode IndexModule1[] = { 0x61, 0x73, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode IndexKey1[] = { 0x0 };
static const sal_Unicode defaultIndex1[] = { 0 };
static const sal_Unicode defaultPhonetic1[] = { 0 };

static const sal_Unicode IndexID2[] = { 0x7a, 0x68, 0x75, 0x79, 0x69, 0x6e, 0x0 };
static const sal_Unicode IndexModule2[] = { 0x61, 0x73, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode IndexKey2[] = { 0x0 };
static const sal_Unicode defaultIndex2[] = { 0 };
static const sal_Unicode defaultPhonetic2[] = { 1 };

static const sal_Unicode IndexID3[] = { 0x70, 0x69, 0x6e, 0x79, 0x69, 0x6e, 0x0 };
static const sal_Unicode IndexModule3[] = { 0x61, 0x73, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode IndexKey3[] = { 0x0 };
static const sal_Unicode defaultIndex3[] = { 0 };
static const sal_Unicode defaultPhonetic3[] = { 1 };

static const sal_Unicode unicodeScript0[] = { 0x37, 0x30, 0x0 };
static const sal_Unicode followPageWord0[] = { 0x0 };
static const sal_Unicode followPageWord1[] = { 0x0 };
static const sal_Int16 nbOfIndexs = 4;

static const sal_Unicode* IndexArray[] = {
    IndexID0, IndexModule0, IndexKey0, defaultIndex0, defaultPhonetic0,
    IndexID1, IndexModule1, IndexKey1, defaultIndex1, defaultPhonetic1,
    IndexID2, IndexModule2, IndexKey2, defaultIndex2, defaultPhonetic2,
    IndexID3, IndexModule3, IndexKey3, defaultIndex3, defaultPhonetic3,
};

static const sal_Int16 nbOfUnicodeScripts = 1;

static const sal_Unicode* UnicodeScriptArray[] = { unicodeScript0, NULL };

static const sal_Int16 nbOfPageWords = 2;

static const sal_Unicode* FollowPageWordArray[] = { followPageWord0, followPageWord1, NULL };

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getIndexAlgorithm_zh_TW(sal_Int16& count)
{
    count = nbOfIndexs;
    return (sal_Unicode**)IndexArray;
}
SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getUnicodeScripts_zh_TW(sal_Int16& count)
{
    count = nbOfUnicodeScripts;
    return (sal_Unicode**)UnicodeScriptArray;
}
SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getFollowPageWords_zh_TW(sal_Int16& count)
{
    count = nbOfPageWords;
    return (sal_Unicode**)FollowPageWordArray;
}
static const sal_Unicode calendarID0[]
    = { 0x67, 0x72, 0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode defaultCalendar0[] = { 1 };
static const sal_Unicode dayRef0[] = { 0x72, 0x65, 0x66, 0x0 };
static const sal_Unicode dayRefName0[] = { 0x7a, 0x68, 0x5f, 0x43, 0x4e, 0x5f, 0x67, 0x72,
                                           0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode monthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode monthDefaultAbbrvName00[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName00[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName00[] = { 0x4e00, 0x0 };
static const sal_Unicode monthID01[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode monthDefaultAbbrvName01[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName01[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName01[] = { 0x4e8c, 0x0 };
static const sal_Unicode monthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode monthDefaultAbbrvName02[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName02[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName02[] = { 0x4e09, 0x0 };
static const sal_Unicode monthID03[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode monthDefaultAbbrvName03[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName03[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName03[] = { 0x56db, 0x0 };
static const sal_Unicode monthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode monthDefaultAbbrvName04[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName04[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName04[] = { 0x4e94, 0x0 };
static const sal_Unicode monthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode monthDefaultAbbrvName05[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName05[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName05[] = { 0x516d, 0x0 };
static const sal_Unicode monthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode monthDefaultAbbrvName06[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName06[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName06[] = { 0x4e03, 0x0 };
static const sal_Unicode monthID07[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode monthDefaultAbbrvName07[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName07[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName07[] = { 0x516b, 0x0 };
static const sal_Unicode monthID08[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode monthDefaultAbbrvName08[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName08[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName08[] = { 0x4e5d, 0x0 };
static const sal_Unicode monthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode monthDefaultAbbrvName09[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName09[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName09[] = { 0x5341, 0x0 };
static const sal_Unicode monthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode monthDefaultAbbrvName010[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName010[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName010[] = { 0x5341, 0x0 };
static const sal_Unicode monthID011[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode monthDefaultAbbrvName011[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName011[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName011[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName00[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName00[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName00[] = { 0x4e00, 0x0 };
static const sal_Unicode genitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName01[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName01[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName01[] = { 0x4e8c, 0x0 };
static const sal_Unicode genitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName02[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName02[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName02[] = { 0x4e09, 0x0 };
static const sal_Unicode genitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName03[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName03[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName03[] = { 0x56db, 0x0 };
static const sal_Unicode genitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName04[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName04[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName04[] = { 0x4e94, 0x0 };
static const sal_Unicode genitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName05[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName05[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName05[] = { 0x516d, 0x0 };
static const sal_Unicode genitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName06[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName06[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName06[] = { 0x4e03, 0x0 };
static const sal_Unicode genitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName07[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName07[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName07[] = { 0x516b, 0x0 };
static const sal_Unicode genitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName08[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName08[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName08[] = { 0x4e5d, 0x0 };
static const sal_Unicode genitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName09[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName09[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName09[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName010[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName010[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName010[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName011[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName011[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName011[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID00[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName00[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName00[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName00[] = { 0x4e00, 0x0 };
static const sal_Unicode partitiveMonthID01[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName01[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName01[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName01[] = { 0x4e8c, 0x0 };
static const sal_Unicode partitiveMonthID02[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName02[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName02[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName02[] = { 0x4e09, 0x0 };
static const sal_Unicode partitiveMonthID03[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName03[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName03[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName03[] = { 0x56db, 0x0 };
static const sal_Unicode partitiveMonthID04[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName04[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName04[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName04[] = { 0x4e94, 0x0 };
static const sal_Unicode partitiveMonthID05[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName05[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName05[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName05[] = { 0x516d, 0x0 };
static const sal_Unicode partitiveMonthID06[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName06[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName06[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName06[] = { 0x4e03, 0x0 };
static const sal_Unicode partitiveMonthID07[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName07[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName07[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName07[] = { 0x516b, 0x0 };
static const sal_Unicode partitiveMonthID08[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName08[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName08[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName08[] = { 0x4e5d, 0x0 };
static const sal_Unicode partitiveMonthID09[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName09[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName09[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName09[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID010[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName010[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName010[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName010[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID011[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName011[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName011[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName011[] = { 0x5341, 0x0 };
static const sal_Unicode eraID00[] = { 0x62, 0x63, 0x0 };

static const sal_Unicode eraDefaultAbbrvName00[] = { 0x897f, 0x5143, 0x524d, 0x0 };
static const sal_Unicode eraDefaultFullName00[] = { 0x897f, 0x5143, 0x524d, 0x0 };
static const sal_Unicode eraID01[] = { 0x61, 0x64, 0x0 };

static const sal_Unicode eraDefaultAbbrvName01[] = { 0x0 };
static const sal_Unicode eraDefaultFullName01[] = { 0x897f, 0x5143, 0x0 };
static const sal_Unicode startDayOfWeek0[] = { 0x73, 0x75, 0x6e, 0x0 };
static const sal_Unicode minimalDaysInFirstWeek0[] = { 7 };
static const sal_Unicode calendarID1[] = { 0x52, 0x4f, 0x43, 0x0 };
static const sal_Unicode defaultCalendar1[] = { 0 };
static const sal_Unicode dayRef1[] = { 0x72, 0x65, 0x66, 0x0 };
static const sal_Unicode dayRefName1[] = { 0x7a, 0x68, 0x5f, 0x43, 0x4e, 0x5f, 0x67, 0x72,
                                           0x65, 0x67, 0x6f, 0x72, 0x69, 0x61, 0x6e, 0x0 };
static const sal_Unicode monthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode monthDefaultAbbrvName10[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName10[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName10[] = { 0x4e00, 0x0 };
static const sal_Unicode monthID11[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode monthDefaultAbbrvName11[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName11[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName11[] = { 0x4e8c, 0x0 };
static const sal_Unicode monthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode monthDefaultAbbrvName12[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName12[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName12[] = { 0x4e09, 0x0 };
static const sal_Unicode monthID13[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode monthDefaultAbbrvName13[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName13[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName13[] = { 0x56db, 0x0 };
static const sal_Unicode monthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode monthDefaultAbbrvName14[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName14[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName14[] = { 0x4e94, 0x0 };
static const sal_Unicode monthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode monthDefaultAbbrvName15[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName15[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName15[] = { 0x516d, 0x0 };
static const sal_Unicode monthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode monthDefaultAbbrvName16[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName16[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName16[] = { 0x4e03, 0x0 };
static const sal_Unicode monthID17[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode monthDefaultAbbrvName17[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName17[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName17[] = { 0x516b, 0x0 };
static const sal_Unicode monthID18[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode monthDefaultAbbrvName18[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName18[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName18[] = { 0x4e5d, 0x0 };
static const sal_Unicode monthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode monthDefaultAbbrvName19[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName19[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName19[] = { 0x5341, 0x0 };
static const sal_Unicode monthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode monthDefaultAbbrvName110[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName110[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName110[] = { 0x5341, 0x0 };
static const sal_Unicode monthID111[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode monthDefaultAbbrvName111[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode monthDefaultFullName111[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode monthDefaultNarrowName111[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName10[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName10[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName10[] = { 0x4e00, 0x0 };
static const sal_Unicode genitiveMonthID11[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName11[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName11[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName11[] = { 0x4e8c, 0x0 };
static const sal_Unicode genitiveMonthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName12[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName12[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName12[] = { 0x4e09, 0x0 };
static const sal_Unicode genitiveMonthID13[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName13[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName13[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName13[] = { 0x56db, 0x0 };
static const sal_Unicode genitiveMonthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName14[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName14[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName14[] = { 0x4e94, 0x0 };
static const sal_Unicode genitiveMonthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName15[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName15[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName15[] = { 0x516d, 0x0 };
static const sal_Unicode genitiveMonthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName16[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName16[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName16[] = { 0x4e03, 0x0 };
static const sal_Unicode genitiveMonthID17[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName17[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName17[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName17[] = { 0x516b, 0x0 };
static const sal_Unicode genitiveMonthID18[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName18[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName18[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName18[] = { 0x4e5d, 0x0 };
static const sal_Unicode genitiveMonthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName19[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName19[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName19[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName110[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName110[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName110[] = { 0x5341, 0x0 };
static const sal_Unicode genitiveMonthID111[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode genitiveMonthDefaultAbbrvName111[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultFullName111[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode genitiveMonthDefaultNarrowName111[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID10[] = { 0x6a, 0x61, 0x6e, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName10[] = { 0x31, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName10[] = { 0x4e00, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName10[] = { 0x4e00, 0x0 };
static const sal_Unicode partitiveMonthID11[] = { 0x66, 0x65, 0x62, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName11[] = { 0x32, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName11[] = { 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName11[] = { 0x4e8c, 0x0 };
static const sal_Unicode partitiveMonthID12[] = { 0x6d, 0x61, 0x72, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName12[] = { 0x33, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName12[] = { 0x4e09, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName12[] = { 0x4e09, 0x0 };
static const sal_Unicode partitiveMonthID13[] = { 0x61, 0x70, 0x72, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName13[] = { 0x34, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName13[] = { 0x56db, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName13[] = { 0x56db, 0x0 };
static const sal_Unicode partitiveMonthID14[] = { 0x6d, 0x61, 0x79, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName14[] = { 0x35, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName14[] = { 0x4e94, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName14[] = { 0x4e94, 0x0 };
static const sal_Unicode partitiveMonthID15[] = { 0x6a, 0x75, 0x6e, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName15[] = { 0x36, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName15[] = { 0x516d, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName15[] = { 0x516d, 0x0 };
static const sal_Unicode partitiveMonthID16[] = { 0x6a, 0x75, 0x6c, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName16[] = { 0x37, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName16[] = { 0x4e03, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName16[] = { 0x4e03, 0x0 };
static const sal_Unicode partitiveMonthID17[] = { 0x61, 0x75, 0x67, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName17[] = { 0x38, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName17[] = { 0x516b, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName17[] = { 0x516b, 0x0 };
static const sal_Unicode partitiveMonthID18[] = { 0x73, 0x65, 0x70, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName18[] = { 0x39, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName18[] = { 0x4e5d, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName18[] = { 0x4e5d, 0x0 };
static const sal_Unicode partitiveMonthID19[] = { 0x6f, 0x63, 0x74, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName19[] = { 0x31, 0x30, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName19[] = { 0x5341, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName19[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID110[] = { 0x6e, 0x6f, 0x76, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName110[] = { 0x31, 0x31, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName110[] = { 0x5341, 0x4e00, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName110[] = { 0x5341, 0x0 };
static const sal_Unicode partitiveMonthID111[] = { 0x64, 0x65, 0x63, 0x0 };
static const sal_Unicode partitiveMonthDefaultAbbrvName111[] = { 0x31, 0x32, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultFullName111[] = { 0x5341, 0x4e8c, 0x6708, 0x0 };
static const sal_Unicode partitiveMonthDefaultNarrowName111[] = { 0x5341, 0x0 };
static const sal_Unicode eraID10[] = { 0x62, 0x65, 0x66, 0x6f, 0x72, 0x65, 0x0 };

static const sal_Unicode eraDefaultAbbrvName10[] = { 0x6c11, 0x570b, 0x524d, 0x0 };
static const sal_Unicode eraDefaultFullName10[] = { 0x4e2d, 0x83ef, 0x6c11, 0x570b, 0x524d, 0x0 };
static const sal_Unicode eraID11[] = { 0x4d, 0x49, 0x4e, 0x47, 0x55, 0x4f, 0x0 };

static const sal_Unicode eraDefaultAbbrvName11[] = { 0x6c11, 0x570b, 0x0 };
static const sal_Unicode eraDefaultFullName11[] = { 0x4e2d, 0x83ef, 0x6c11, 0x570b, 0x0 };
static const sal_Unicode startDayOfWeek1[] = { 0x73, 0x75, 0x6e, 0x0 };
static const sal_Unicode minimalDaysInFirstWeek1[] = { 7 };
static const sal_Int16 calendarsCount = 2;

static const sal_Unicode nbOfDays[] = { 0, 0 };
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
    dayRef0,
    dayRefName0,
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
    dayRef1,
    dayRefName1,
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

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCalendars_zh_TW(sal_Int16& count)
{
    count = calendarsCount;
    return (sal_Unicode**)calendars;
}
static const sal_Unicode defaultCurrency0[] = { 1 };
static const sal_Unicode defaultCurrencyUsedInCompatibleFormatCodes0[] = { 1 };
static const sal_Unicode defaultCurrencyLegacyOnly0[] = { 0 };
static const sal_Unicode currencyID0[] = { 0x54, 0x57, 0x44, 0x0 };
static const sal_Unicode currencySymbol0[] = { 0x4e, 0x54, 0x24, 0x0 };
static const sal_Unicode bankSymbol0[] = { 0x54, 0x57, 0x44, 0x0 };
static const sal_Unicode currencyName0[] = { 0x65b0, 0x81fa, 0x5e63, 0x0 };
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

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getAllCurrencies_zh_TW(sal_Int16& count)
{
    count = currencyCount;
    return (sal_Unicode**)currencies;
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
    = { 0x4e, 0x75, 0x6d, 0x54, 0x6f, 0x54, 0x65, 0x78, 0x74, 0x4c, 0x6f,
        0x77, 0x65, 0x72, 0x5f, 0x7a, 0x68, 0x5f, 0x54, 0x57, 0x0 };
static const sal_Unicode Transliteration6[]
    = { 0x4e, 0x75, 0x6d, 0x54, 0x6f, 0x54, 0x65, 0x78, 0x74, 0x55, 0x70,
        0x70, 0x65, 0x72, 0x5f, 0x7a, 0x68, 0x5f, 0x54, 0x57, 0x0 };
static const sal_Unicode Transliteration7[]
    = { 0x4e, 0x75, 0x6d, 0x54, 0x6f, 0x43, 0x68, 0x61, 0x72, 0x4c, 0x6f,
        0x77, 0x65, 0x72, 0x5f, 0x7a, 0x68, 0x5f, 0x54, 0x57, 0x0 };
static const sal_Unicode Transliteration8[]
    = { 0x4e, 0x75, 0x6d, 0x54, 0x6f, 0x43, 0x68, 0x61, 0x72, 0x55, 0x70,
        0x70, 0x65, 0x72, 0x5f, 0x7a, 0x68, 0x5f, 0x54, 0x57, 0x0 };
static const sal_Unicode Transliteration9[]
    = { 0x54, 0x65, 0x78, 0x74, 0x54, 0x6f, 0x43, 0x68, 0x75, 0x79,
        0x69, 0x6e, 0x5f, 0x7a, 0x68, 0x5f, 0x54, 0x57, 0x0 };
static const sal_Unicode Transliteration10[]
    = { 0x48, 0x41, 0x4c, 0x46, 0x57, 0x49, 0x44, 0x54, 0x48, 0x5f,
        0x46, 0x55, 0x4c, 0x4c, 0x57, 0x49, 0x44, 0x54, 0x48, 0x0 };
static const sal_Unicode Transliteration11[]
    = { 0x46, 0x55, 0x4c, 0x4c, 0x57, 0x49, 0x44, 0x54, 0x48, 0x5f,
        0x48, 0x41, 0x4c, 0x46, 0x57, 0x49, 0x44, 0x54, 0x48, 0x0 };
static const sal_Unicode Transliteration12[]
    = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x43, 0x41, 0x53, 0x45, 0x0 };
static const sal_Unicode Transliteration13[]
    = { 0x49, 0x47, 0x4e, 0x4f, 0x52, 0x45, 0x5f, 0x57, 0x49, 0x44, 0x54, 0x48, 0x0 };
static const sal_Int16 nbOfTransliterations = 14;

static const sal_Unicode* LCTransliterationsArray[] = {
    Transliteration0,  Transliteration1,  Transliteration2,  Transliteration3,  Transliteration4,
    Transliteration5,  Transliteration6,  Transliteration7,  Transliteration8,  Transliteration9,
    Transliteration10, Transliteration11, Transliteration12, Transliteration13,
};

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getTransliterations_zh_TW(sal_Int16& count)
{
    count = nbOfTransliterations;
    return (sal_Unicode**)LCTransliterationsArray;
}
static const sal_Unicode ReservedWord0[] = { 0x771f, 0x0 };
static const sal_Unicode ReservedWord1[] = { 0x5047, 0x0 };
static const sal_Unicode ReservedWord2[] = { 0x7b2c, 0x31, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord3[] = { 0x7b2c, 0x32, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord4[] = { 0x7b2c, 0x33, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord5[] = { 0x7b2c, 0x34, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord6[] = { 0x4e0a, 0x9762, 0x0 };
static const sal_Unicode ReservedWord7[] = { 0x4e0b, 0x9762, 0x0 };
static const sal_Unicode ReservedWord8[] = { 0x31, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord9[] = { 0x32, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord10[] = { 0x33, 0x5b63, 0x0 };
static const sal_Unicode ReservedWord11[] = { 0x34, 0x5b63, 0x0 };
static const sal_Int16 nbOfReservedWords = 12;

static const sal_Unicode* LCReservedWordsArray[] = {
    ReservedWord0, ReservedWord1, ReservedWord2, ReservedWord3, ReservedWord4,  ReservedWord5,
    ReservedWord6, ReservedWord7, ReservedWord8, ReservedWord9, ReservedWord10, ReservedWord11,
};

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getReservedWords_zh_TW(sal_Int16& count)
{
    count = nbOfReservedWords;
    return (sal_Unicode**)LCReservedWordsArray;
}
static const sal_Unicode forbiddenBegin[]
    = { 0x21,   0x29,   0x2c,   0x2e,   0x3a,   0x3b,   0x3f,   0x5d,   0x7d,   0xa2,   0xb7,
        0x2d,   0x2d,   0x27,   0x22,   0xa8,   0x2022, 0x2032, 0x3001, 0x3002, 0x3009, 0x300b,
        0x300d, 0x300f, 0x3011, 0x3015, 0x301e, 0xfe30, 0xfe31, 0xfe33, 0xfe34, 0xfe36, 0xfe38,
        0xfe3a, 0xfe3c, 0xfe3e, 0xfe40, 0xfe42, 0xfe44, 0xfe4f, 0xfe50, 0xff64, 0xfe52, 0xfe54,
        0xfe55, 0xfe56, 0xfe57, 0xfe5a, 0xfe5c, 0xfe5e, 0xff01, 0xff09, 0xff0c, 0xff0e, 0xff1a,
        0xff1b, 0xff1f, 0xff5c, 0xff5d, 0xff64, 0x0 };
static const sal_Unicode forbiddenEnd[]
    = { 0x28,   0x5b,   0x7b,   0xa3,   0xa5,   0x27,   0x22,   0x2035, 0x3008, 0x300a,
        0x300c, 0x300e, 0x3010, 0x3014, 0x301d, 0xfe35, 0xfe37, 0xfe39, 0xfe3b, 0xfe3d,
        0xfe3f, 0xfe41, 0xfe43, 0xfe59, 0xfe5b, 0xfe5d, 0xff08, 0xff5b, 0x0 };
static const sal_Unicode hangingChars[]
    = { 0x21,   0x2c,   0x2e,   0x3a,   0x3b,   0x3f,   0x3001, 0x3002,
        0xff01, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0x0 };

static const sal_Unicode* LCForbiddenCharactersArray[]
    = { forbiddenBegin, forbiddenEnd, hangingChars };

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getForbiddenCharacters_zh_TW(sal_Int16& count)
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

SAL_DLLPUBLIC_EXPORT sal_Unicode** SAL_CALL getBreakIteratorRules_zh_TW(sal_Int16& count)
{
    count = 5;
    return (sal_Unicode**)LCBreakIteratorRulesArray;
}
// ---> ContinuousNumbering
static const sal_Unicode continuousPrefix0[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType0[] = { 0x34, 0x0 };
static const sal_Unicode continuousSuffix0[] = { 0x2e, 0x0 };
static const sal_Unicode continuousTransliteration0[] = { 0x0 };
static const sal_Unicode continuousNatNum0[] = { 0x0 };
static const sal_Unicode continuousPrefix1[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType1[] = { 0x30, 0x0 };
static const sal_Unicode continuousSuffix1[] = { 0x2e, 0x0 };
static const sal_Unicode continuousTransliteration1[] = { 0x0 };
static const sal_Unicode continuousNatNum1[] = { 0x0 };
static const sal_Unicode continuousPrefix2[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType2[] = { 0x32, 0x0 };
static const sal_Unicode continuousSuffix2[] = { 0x2e, 0x0 };
static const sal_Unicode continuousTransliteration2[] = { 0x0 };
static const sal_Unicode continuousNatNum2[] = { 0x0 };
static const sal_Unicode continuousPrefix3[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType3[] = { 0x31, 0x33, 0x0 };
static const sal_Unicode continuousSuffix3[] = { 0x3001, 0x0 };
static const sal_Unicode continuousTransliteration3[] = { 0x0 };
static const sal_Unicode continuousNatNum3[] = { 0x0 };
static const sal_Unicode continuousPrefix4[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType4[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode continuousSuffix4[] = { 0x3001, 0x0 };
static const sal_Unicode continuousTransliteration4[] = { 0x0 };
static const sal_Unicode continuousNatNum4[] = { 0x0 };
static const sal_Unicode continuousPrefix5[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType5[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode continuousSuffix5[] = { 0x3001, 0x0 };
static const sal_Unicode continuousTransliteration5[] = { 0x0 };
static const sal_Unicode continuousNatNum5[] = { 0x0 };
static const sal_Unicode continuousPrefix6[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType6[] = { 0x31, 0x38, 0x0 };
static const sal_Unicode continuousSuffix6[] = { 0x3001, 0x0 };
static const sal_Unicode continuousTransliteration6[] = { 0x0 };
static const sal_Unicode continuousNatNum6[] = { 0x0 };
static const sal_Unicode continuousPrefix7[] = { 0x20, 0x0 };
static const sal_Unicode continuousNumType7[] = { 0x31, 0x39, 0x0 };
static const sal_Unicode continuousSuffix7[] = { 0x3001, 0x0 };
static const sal_Unicode continuousTransliteration7[] = { 0x0 };
static const sal_Unicode continuousNatNum7[] = { 0x0 };
static const sal_Int16 continuousNbOfStyles = 8;

static const sal_Int16 continuousNbOfAttributesPerStyle = 5;

static const sal_Unicode* continuousStyle0[]
    = { continuousPrefix0,          continuousNumType0, continuousSuffix0,
        continuousTransliteration0, continuousNatNum0,  0 };

static const sal_Unicode* continuousStyle1[]
    = { continuousPrefix1,          continuousNumType1, continuousSuffix1,
        continuousTransliteration1, continuousNatNum1,  0 };

static const sal_Unicode* continuousStyle2[]
    = { continuousPrefix2,          continuousNumType2, continuousSuffix2,
        continuousTransliteration2, continuousNatNum2,  0 };

static const sal_Unicode* continuousStyle3[]
    = { continuousPrefix3,          continuousNumType3, continuousSuffix3,
        continuousTransliteration3, continuousNatNum3,  0 };

static const sal_Unicode* continuousStyle4[]
    = { continuousPrefix4,          continuousNumType4, continuousSuffix4,
        continuousTransliteration4, continuousNatNum4,  0 };

static const sal_Unicode* continuousStyle5[]
    = { continuousPrefix5,          continuousNumType5, continuousSuffix5,
        continuousTransliteration5, continuousNatNum5,  0 };

static const sal_Unicode* continuousStyle6[]
    = { continuousPrefix6,          continuousNumType6, continuousSuffix6,
        continuousTransliteration6, continuousNatNum6,  0 };

static const sal_Unicode* continuousStyle7[]
    = { continuousPrefix7,          continuousNumType7, continuousSuffix7,
        continuousTransliteration7, continuousNatNum7,  0 };

static const sal_Unicode** LCContinuousNumberingLevelsArray[]
    = { continuousStyle0, continuousStyle1, continuousStyle2,
        continuousStyle3, continuousStyle4, continuousStyle5,
        continuousStyle6, continuousStyle7, 0 };

SAL_DLLPUBLIC_EXPORT const sal_Unicode*** SAL_CALL
getContinuousNumberingLevels_zh_TW(sal_Int16& nStyles, sal_Int16& nAttributes)
{
    nStyles = continuousNbOfStyles;
    nAttributes = continuousNbOfAttributesPerStyle;
    return LCContinuousNumberingLevelsArray;
}
// ---> OutlineNumbering
static const sal_Unicode outlinePrefix00[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType00[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix00[] = { 0x20, 0x0 };
static const sal_Unicode outlineBulletChar00[] = { 0x0 };
static const sal_Unicode outlineBulletFontName00[] = { 0x0 };
static const sal_Unicode outlineParentNumbering00[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin00[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance00[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset00[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration00[] = { 0x0 };
static const sal_Unicode outlineNatNum00[] = { 0x0 };
static const sal_Unicode outlinePrefix01[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType01[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix01[] = { 0x20, 0x0 };
static const sal_Unicode outlineBulletChar01[] = { 0x0 };
static const sal_Unicode outlineBulletFontName01[] = { 0x0 };
static const sal_Unicode outlineParentNumbering01[] = { 0x31, 0x0 };
static const sal_Unicode outlineLeftMargin01[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance01[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset01[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration01[] = { 0x0 };
static const sal_Unicode outlineNatNum01[] = { 0x0 };
static const sal_Unicode outlinePrefix02[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType02[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix02[] = { 0x20, 0x0 };
static const sal_Unicode outlineBulletChar02[] = { 0x0 };
static const sal_Unicode outlineBulletFontName02[] = { 0x0 };
static const sal_Unicode outlineParentNumbering02[] = { 0x32, 0x0 };
static const sal_Unicode outlineLeftMargin02[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance02[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset02[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration02[] = { 0x0 };
static const sal_Unicode outlineNatNum02[] = { 0x0 };
static const sal_Unicode outlinePrefix03[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType03[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix03[] = { 0x20, 0x0 };
static const sal_Unicode outlineBulletChar03[] = { 0x0 };
static const sal_Unicode outlineBulletFontName03[] = { 0x0 };
static const sal_Unicode outlineParentNumbering03[] = { 0x33, 0x0 };
static const sal_Unicode outlineLeftMargin03[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance03[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset03[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration03[] = { 0x0 };
static const sal_Unicode outlineNatNum03[] = { 0x0 };
static const sal_Unicode outlinePrefix04[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType04[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix04[] = { 0x20, 0x0 };
static const sal_Unicode outlineBulletChar04[] = { 0x0 };
static const sal_Unicode outlineBulletFontName04[] = { 0x0 };
static const sal_Unicode outlineParentNumbering04[] = { 0x34, 0x0 };
static const sal_Unicode outlineLeftMargin04[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance04[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset04[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration04[] = { 0x0 };
static const sal_Unicode outlineNatNum04[] = { 0x0 };
static const sal_Unicode outlinePrefix10[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType10[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix10[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar10[] = { 0x0 };
static const sal_Unicode outlineBulletFontName10[] = { 0x0 };
static const sal_Unicode outlineParentNumbering10[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin10[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance10[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset10[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration10[] = { 0x0 };
static const sal_Unicode outlineNatNum10[] = { 0x0 };
static const sal_Unicode outlinePrefix11[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType11[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix11[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar11[] = { 0x0 };
static const sal_Unicode outlineBulletFontName11[] = { 0x0 };
static const sal_Unicode outlineParentNumbering11[] = { 0x31, 0x0 };
static const sal_Unicode outlineLeftMargin11[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance11[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset11[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration11[] = { 0x0 };
static const sal_Unicode outlineNatNum11[] = { 0x0 };
static const sal_Unicode outlinePrefix12[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType12[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix12[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar12[] = { 0x0 };
static const sal_Unicode outlineBulletFontName12[] = { 0x0 };
static const sal_Unicode outlineParentNumbering12[] = { 0x32, 0x0 };
static const sal_Unicode outlineLeftMargin12[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance12[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset12[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration12[] = { 0x0 };
static const sal_Unicode outlineNatNum12[] = { 0x0 };
static const sal_Unicode outlinePrefix13[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType13[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix13[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar13[] = { 0x0 };
static const sal_Unicode outlineBulletFontName13[] = { 0x0 };
static const sal_Unicode outlineParentNumbering13[] = { 0x33, 0x0 };
static const sal_Unicode outlineLeftMargin13[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance13[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset13[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration13[] = { 0x0 };
static const sal_Unicode outlineNatNum13[] = { 0x0 };
static const sal_Unicode outlinePrefix14[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType14[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix14[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar14[] = { 0x0 };
static const sal_Unicode outlineBulletFontName14[] = { 0x0 };
static const sal_Unicode outlineParentNumbering14[] = { 0x34, 0x0 };
static const sal_Unicode outlineLeftMargin14[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance14[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset14[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration14[] = { 0x0 };
static const sal_Unicode outlineNatNum14[] = { 0x0 };
static const sal_Unicode outlinePrefix20[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType20[] = { 0x32, 0x0 };
static const sal_Unicode outlineSuffix20[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar20[] = { 0x0 };
static const sal_Unicode outlineBulletFontName20[] = { 0x0 };
static const sal_Unicode outlineParentNumbering20[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin20[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance20[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset20[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration20[] = { 0x0 };
static const sal_Unicode outlineNatNum20[] = { 0x0 };
static const sal_Unicode outlinePrefix21[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType21[] = { 0x30, 0x0 };
static const sal_Unicode outlineSuffix21[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar21[] = { 0x0 };
static const sal_Unicode outlineBulletFontName21[] = { 0x0 };
static const sal_Unicode outlineParentNumbering21[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin21[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance21[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset21[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration21[] = { 0x0 };
static const sal_Unicode outlineNatNum21[] = { 0x0 };
static const sal_Unicode outlinePrefix22[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType22[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix22[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar22[] = { 0x0 };
static const sal_Unicode outlineBulletFontName22[] = { 0x0 };
static const sal_Unicode outlineParentNumbering22[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin22[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance22[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset22[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration22[] = { 0x0 };
static const sal_Unicode outlineNatNum22[] = { 0x0 };
static const sal_Unicode outlinePrefix23[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType23[] = { 0x33, 0x0 };
static const sal_Unicode outlineSuffix23[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar23[] = { 0x0 };
static const sal_Unicode outlineBulletFontName23[] = { 0x0 };
static const sal_Unicode outlineParentNumbering23[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin23[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance23[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset23[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration23[] = { 0x0 };
static const sal_Unicode outlineNatNum23[] = { 0x0 };
static const sal_Unicode outlinePrefix24[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType24[] = { 0x31, 0x0 };
static const sal_Unicode outlineSuffix24[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar24[] = { 0x0 };
static const sal_Unicode outlineBulletFontName24[] = { 0x0 };
static const sal_Unicode outlineParentNumbering24[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin24[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance24[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset24[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration24[] = { 0x0 };
static const sal_Unicode outlineNatNum24[] = { 0x0 };
static const sal_Unicode outlinePrefix30[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType30[] = { 0x31, 0x38, 0x0 };
static const sal_Unicode outlineSuffix30[] = { 0x3001, 0x0 };
static const sal_Unicode outlineBulletChar30[] = { 0x0 };
static const sal_Unicode outlineBulletFontName30[] = { 0x0 };
static const sal_Unicode outlineParentNumbering30[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin30[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance30[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset30[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration30[] = { 0x0 };
static const sal_Unicode outlineNatNum30[] = { 0x0 };
static const sal_Unicode outlinePrefix31[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType31[] = { 0x31, 0x39, 0x0 };
static const sal_Unicode outlineSuffix31[] = { 0x3001, 0x0 };
static const sal_Unicode outlineBulletChar31[] = { 0x0 };
static const sal_Unicode outlineBulletFontName31[] = { 0x0 };
static const sal_Unicode outlineParentNumbering31[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin31[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance31[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset31[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration31[] = { 0x0 };
static const sal_Unicode outlineNatNum31[] = { 0x0 };
static const sal_Unicode outlinePrefix32[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType32[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode outlineSuffix32[] = { 0x3001, 0x0 };
static const sal_Unicode outlineBulletChar32[] = { 0x0 };
static const sal_Unicode outlineBulletFontName32[] = { 0x0 };
static const sal_Unicode outlineParentNumbering32[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin32[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance32[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset32[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration32[] = { 0x0 };
static const sal_Unicode outlineNatNum32[] = { 0x0 };
static const sal_Unicode outlinePrefix33[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType33[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode outlineSuffix33[] = { 0x3001, 0x0 };
static const sal_Unicode outlineBulletChar33[] = { 0x0 };
static const sal_Unicode outlineBulletFontName33[] = { 0x0 };
static const sal_Unicode outlineParentNumbering33[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin33[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance33[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset33[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration33[] = { 0x0 };
static const sal_Unicode outlineNatNum33[] = { 0x0 };
static const sal_Unicode outlinePrefix34[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType34[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix34[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar34[] = { 0x0 };
static const sal_Unicode outlineBulletFontName34[] = { 0x0 };
static const sal_Unicode outlineParentNumbering34[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin34[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance34[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset34[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration34[] = { 0x0 };
static const sal_Unicode outlineNatNum34[] = { 0x0 };
static const sal_Unicode outlinePrefix40[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType40[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix40[] = { 0x7ae0, 0x0 };
static const sal_Unicode outlineBulletChar40[] = { 0x0 };
static const sal_Unicode outlineBulletFontName40[] = { 0x0 };
static const sal_Unicode outlineParentNumbering40[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin40[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance40[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset40[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration40[] = { 0x0 };
static const sal_Unicode outlineNatNum40[] = { 0x0 };
static const sal_Unicode outlinePrefix41[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType41[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix41[] = { 0x7bc0, 0x0 };
static const sal_Unicode outlineBulletChar41[] = { 0x0 };
static const sal_Unicode outlineBulletFontName41[] = { 0x0 };
static const sal_Unicode outlineParentNumbering41[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin41[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance41[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset41[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration41[] = { 0x0 };
static const sal_Unicode outlineNatNum41[] = { 0x0 };
static const sal_Unicode outlinePrefix42[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType42[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix42[] = { 0x9805, 0x0 };
static const sal_Unicode outlineBulletChar42[] = { 0x0 };
static const sal_Unicode outlineBulletFontName42[] = { 0x0 };
static const sal_Unicode outlineParentNumbering42[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin42[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance42[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset42[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration42[] = { 0x0 };
static const sal_Unicode outlineNatNum42[] = { 0x0 };
static const sal_Unicode outlinePrefix43[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType43[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix43[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar43[] = { 0x0 };
static const sal_Unicode outlineBulletFontName43[] = { 0x0 };
static const sal_Unicode outlineParentNumbering43[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin43[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance43[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset43[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration43[] = { 0x0 };
static const sal_Unicode outlineNatNum43[] = { 0x0 };
static const sal_Unicode outlinePrefix44[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType44[] = { 0x31, 0x0 };
static const sal_Unicode outlineSuffix44[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar44[] = { 0x0 };
static const sal_Unicode outlineBulletFontName44[] = { 0x0 };
static const sal_Unicode outlineParentNumbering44[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin44[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance44[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset44[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration44[] = { 0x0 };
static const sal_Unicode outlineNatNum44[] = { 0x0 };
static const sal_Unicode outlinePrefix50[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType50[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode outlineSuffix50[] = { 0x7ae0, 0x0 };
static const sal_Unicode outlineBulletChar50[] = { 0x0 };
static const sal_Unicode outlineBulletFontName50[] = { 0x0 };
static const sal_Unicode outlineParentNumbering50[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin50[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance50[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset50[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration50[] = { 0x0 };
static const sal_Unicode outlineNatNum50[] = { 0x0 };
static const sal_Unicode outlinePrefix51[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType51[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode outlineSuffix51[] = { 0x7bc0, 0x0 };
static const sal_Unicode outlineBulletChar51[] = { 0x0 };
static const sal_Unicode outlineBulletFontName51[] = { 0x0 };
static const sal_Unicode outlineParentNumbering51[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin51[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance51[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset51[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration51[] = { 0x0 };
static const sal_Unicode outlineNatNum51[] = { 0x0 };
static const sal_Unicode outlinePrefix52[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType52[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode outlineSuffix52[] = { 0x9805, 0x0 };
static const sal_Unicode outlineBulletChar52[] = { 0x0 };
static const sal_Unicode outlineBulletFontName52[] = { 0x0 };
static const sal_Unicode outlineParentNumbering52[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin52[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance52[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset52[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration52[] = { 0x0 };
static const sal_Unicode outlineNatNum52[] = { 0x0 };
static const sal_Unicode outlinePrefix53[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType53[] = { 0x31, 0x35, 0x0 };
static const sal_Unicode outlineSuffix53[] = { 0x29, 0x0 };
static const sal_Unicode outlineBulletChar53[] = { 0x0 };
static const sal_Unicode outlineBulletFontName53[] = { 0x0 };
static const sal_Unicode outlineParentNumbering53[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin53[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance53[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset53[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration53[] = { 0x0 };
static const sal_Unicode outlineNatNum53[] = { 0x0 };
static const sal_Unicode outlinePrefix54[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType54[] = { 0x31, 0x0 };
static const sal_Unicode outlineSuffix54[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar54[] = { 0x0 };
static const sal_Unicode outlineBulletFontName54[] = { 0x0 };
static const sal_Unicode outlineParentNumbering54[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin54[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance54[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset54[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration54[] = { 0x0 };
static const sal_Unicode outlineNatNum54[] = { 0x0 };
static const sal_Unicode outlinePrefix60[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType60[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode outlineSuffix60[] = { 0x7ae0, 0x0 };
static const sal_Unicode outlineBulletChar60[] = { 0x0 };
static const sal_Unicode outlineBulletFontName60[] = { 0x0 };
static const sal_Unicode outlineParentNumbering60[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin60[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance60[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset60[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration60[] = { 0x0 };
static const sal_Unicode outlineNatNum60[] = { 0x0 };
static const sal_Unicode outlinePrefix61[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType61[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode outlineSuffix61[] = { 0x7bc0, 0x0 };
static const sal_Unicode outlineBulletChar61[] = { 0x0 };
static const sal_Unicode outlineBulletFontName61[] = { 0x0 };
static const sal_Unicode outlineParentNumbering61[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin61[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance61[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset61[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration61[] = { 0x0 };
static const sal_Unicode outlineNatNum61[] = { 0x0 };
static const sal_Unicode outlinePrefix62[] = { 0x7b2c, 0x0 };
static const sal_Unicode outlineNumType62[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode outlineSuffix62[] = { 0x9805, 0x0 };
static const sal_Unicode outlineBulletChar62[] = { 0x0 };
static const sal_Unicode outlineBulletFontName62[] = { 0x0 };
static const sal_Unicode outlineParentNumbering62[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin62[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance62[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset62[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration62[] = { 0x0 };
static const sal_Unicode outlineNatNum62[] = { 0x0 };
static const sal_Unicode outlinePrefix63[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType63[] = { 0x31, 0x37, 0x0 };
static const sal_Unicode outlineSuffix63[] = { 0x29, 0x0 };
static const sal_Unicode outlineBulletChar63[] = { 0x0 };
static const sal_Unicode outlineBulletFontName63[] = { 0x0 };
static const sal_Unicode outlineParentNumbering63[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin63[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance63[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset63[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration63[] = { 0x0 };
static const sal_Unicode outlineNatNum63[] = { 0x0 };
static const sal_Unicode outlinePrefix64[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType64[] = { 0x31, 0x0 };
static const sal_Unicode outlineSuffix64[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar64[] = { 0x0 };
static const sal_Unicode outlineBulletFontName64[] = { 0x0 };
static const sal_Unicode outlineParentNumbering64[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin64[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance64[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset64[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration64[] = { 0x0 };
static const sal_Unicode outlineNatNum64[] = { 0x0 };
static const sal_Unicode outlinePrefix70[]
    = { 0x43, 0x68, 0x61, 0x70, 0x74, 0x65, 0x72, 0x20, 0x0 };
static const sal_Unicode outlineNumType70[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix70[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar70[] = { 0x0 };
static const sal_Unicode outlineBulletFontName70[] = { 0x0 };
static const sal_Unicode outlineParentNumbering70[] = { 0x30, 0x0 };
static const sal_Unicode outlineLeftMargin70[] = { 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance70[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset70[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration70[] = { 0x0 };
static const sal_Unicode outlineNatNum70[] = { 0x0 };
static const sal_Unicode outlinePrefix71[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType71[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix71[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar71[] = { 0x0 };
static const sal_Unicode outlineBulletFontName71[] = { 0x0 };
static const sal_Unicode outlineParentNumbering71[] = { 0x31, 0x0 };
static const sal_Unicode outlineLeftMargin71[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance71[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset71[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration71[] = { 0x0 };
static const sal_Unicode outlineNatNum71[] = { 0x0 };
static const sal_Unicode outlinePrefix72[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType72[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix72[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar72[] = { 0x0 };
static const sal_Unicode outlineBulletFontName72[] = { 0x0 };
static const sal_Unicode outlineParentNumbering72[] = { 0x32, 0x0 };
static const sal_Unicode outlineLeftMargin72[] = { 0x31, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance72[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset72[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration72[] = { 0x0 };
static const sal_Unicode outlineNatNum72[] = { 0x0 };
static const sal_Unicode outlinePrefix73[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType73[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix73[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar73[] = { 0x0 };
static const sal_Unicode outlineBulletFontName73[] = { 0x0 };
static const sal_Unicode outlineParentNumbering73[] = { 0x33, 0x0 };
static const sal_Unicode outlineLeftMargin73[] = { 0x31, 0x35, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance73[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset73[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration73[] = { 0x0 };
static const sal_Unicode outlineNatNum73[] = { 0x0 };
static const sal_Unicode outlinePrefix74[] = { 0x20, 0x0 };
static const sal_Unicode outlineNumType74[] = { 0x34, 0x0 };
static const sal_Unicode outlineSuffix74[] = { 0x2e, 0x0 };
static const sal_Unicode outlineBulletChar74[] = { 0x0 };
static const sal_Unicode outlineBulletFontName74[] = { 0x0 };
static const sal_Unicode outlineParentNumbering74[] = { 0x34, 0x0 };
static const sal_Unicode outlineLeftMargin74[] = { 0x32, 0x30, 0x30, 0x0 };
static const sal_Unicode outlineSymbolTextDistance74[] = { 0x35, 0x30, 0x0 };
static const sal_Unicode outlineFirstLineOffset74[] = { 0x30, 0x0 };
static const sal_Unicode outlineTransliteration74[] = { 0x0 };
static const sal_Unicode outlineNatNum74[] = { 0x0 };
static const sal_Int16 outlineNbOfStyles = 8;

static const sal_Int16 outlineNbOfLevelsPerStyle = 5;

static const sal_Int16 outlineNbOfAttributesPerLevel = 11;

static const sal_Unicode* outlineStyle0Level0[]
    = { outlinePrefix00,          outlineNumType00,
        outlineSuffix00,          outlineBulletChar00,
        outlineBulletFontName00,  outlineParentNumbering00,
        outlineLeftMargin00,      outlineSymbolTextDistance00,
        outlineFirstLineOffset00, outlineTransliteration00,
        outlineNatNum00,          NULL };
static const sal_Unicode* outlineStyle0Level1[]
    = { outlinePrefix01,          outlineNumType01,
        outlineSuffix01,          outlineBulletChar01,
        outlineBulletFontName01,  outlineParentNumbering01,
        outlineLeftMargin01,      outlineSymbolTextDistance01,
        outlineFirstLineOffset01, outlineTransliteration01,
        outlineNatNum01,          NULL };
static const sal_Unicode* outlineStyle0Level2[]
    = { outlinePrefix02,          outlineNumType02,
        outlineSuffix02,          outlineBulletChar02,
        outlineBulletFontName02,  outlineParentNumbering02,
        outlineLeftMargin02,      outlineSymbolTextDistance02,
        outlineFirstLineOffset02, outlineTransliteration02,
        outlineNatNum02,          NULL };
static const sal_Unicode* outlineStyle0Level3[]
    = { outlinePrefix03,          outlineNumType03,
        outlineSuffix03,          outlineBulletChar03,
        outlineBulletFontName03,  outlineParentNumbering03,
        outlineLeftMargin03,      outlineSymbolTextDistance03,
        outlineFirstLineOffset03, outlineTransliteration03,
        outlineNatNum03,          NULL };
static const sal_Unicode* outlineStyle0Level4[]
    = { outlinePrefix04,          outlineNumType04,
        outlineSuffix04,          outlineBulletChar04,
        outlineBulletFontName04,  outlineParentNumbering04,
        outlineLeftMargin04,      outlineSymbolTextDistance04,
        outlineFirstLineOffset04, outlineTransliteration04,
        outlineNatNum04,          NULL };
static const sal_Unicode* outlineStyle1Level0[]
    = { outlinePrefix10,          outlineNumType10,
        outlineSuffix10,          outlineBulletChar10,
        outlineBulletFontName10,  outlineParentNumbering10,
        outlineLeftMargin10,      outlineSymbolTextDistance10,
        outlineFirstLineOffset10, outlineTransliteration10,
        outlineNatNum10,          NULL };
static const sal_Unicode* outlineStyle1Level1[]
    = { outlinePrefix11,          outlineNumType11,
        outlineSuffix11,          outlineBulletChar11,
        outlineBulletFontName11,  outlineParentNumbering11,
        outlineLeftMargin11,      outlineSymbolTextDistance11,
        outlineFirstLineOffset11, outlineTransliteration11,
        outlineNatNum11,          NULL };
static const sal_Unicode* outlineStyle1Level2[]
    = { outlinePrefix12,          outlineNumType12,
        outlineSuffix12,          outlineBulletChar12,
        outlineBulletFontName12,  outlineParentNumbering12,
        outlineLeftMargin12,      outlineSymbolTextDistance12,
        outlineFirstLineOffset12, outlineTransliteration12,
        outlineNatNum12,          NULL };
static const sal_Unicode* outlineStyle1Level3[]
    = { outlinePrefix13,          outlineNumType13,
        outlineSuffix13,          outlineBulletChar13,
        outlineBulletFontName13,  outlineParentNumbering13,
        outlineLeftMargin13,      outlineSymbolTextDistance13,
        outlineFirstLineOffset13, outlineTransliteration13,
        outlineNatNum13,          NULL };
static const sal_Unicode* outlineStyle1Level4[]
    = { outlinePrefix14,          outlineNumType14,
        outlineSuffix14,          outlineBulletChar14,
        outlineBulletFontName14,  outlineParentNumbering14,
        outlineLeftMargin14,      outlineSymbolTextDistance14,
        outlineFirstLineOffset14, outlineTransliteration14,
        outlineNatNum14,          NULL };
static const sal_Unicode* outlineStyle2Level0[]
    = { outlinePrefix20,          outlineNumType20,
        outlineSuffix20,          outlineBulletChar20,
        outlineBulletFontName20,  outlineParentNumbering20,
        outlineLeftMargin20,      outlineSymbolTextDistance20,
        outlineFirstLineOffset20, outlineTransliteration20,
        outlineNatNum20,          NULL };
static const sal_Unicode* outlineStyle2Level1[]
    = { outlinePrefix21,          outlineNumType21,
        outlineSuffix21,          outlineBulletChar21,
        outlineBulletFontName21,  outlineParentNumbering21,
        outlineLeftMargin21,      outlineSymbolTextDistance21,
        outlineFirstLineOffset21, outlineTransliteration21,
        outlineNatNum21,          NULL };
static const sal_Unicode* outlineStyle2Level2[]
    = { outlinePrefix22,          outlineNumType22,
        outlineSuffix22,          outlineBulletChar22,
        outlineBulletFontName22,  outlineParentNumbering22,
        outlineLeftMargin22,      outlineSymbolTextDistance22,
        outlineFirstLineOffset22, outlineTransliteration22,
        outlineNatNum22,          NULL };
static const sal_Unicode* outlineStyle2Level3[]
    = { outlinePrefix23,          outlineNumType23,
        outlineSuffix23,          outlineBulletChar23,
        outlineBulletFontName23,  outlineParentNumbering23,
        outlineLeftMargin23,      outlineSymbolTextDistance23,
        outlineFirstLineOffset23, outlineTransliteration23,
        outlineNatNum23,          NULL };
static const sal_Unicode* outlineStyle2Level4[]
    = { outlinePrefix24,          outlineNumType24,
        outlineSuffix24,          outlineBulletChar24,
        outlineBulletFontName24,  outlineParentNumbering24,
        outlineLeftMargin24,      outlineSymbolTextDistance24,
        outlineFirstLineOffset24, outlineTransliteration24,
        outlineNatNum24,          NULL };
static const sal_Unicode* outlineStyle3Level0[]
    = { outlinePrefix30,          outlineNumType30,
        outlineSuffix30,          outlineBulletChar30,
        outlineBulletFontName30,  outlineParentNumbering30,
        outlineLeftMargin30,      outlineSymbolTextDistance30,
        outlineFirstLineOffset30, outlineTransliteration30,
        outlineNatNum30,          NULL };
static const sal_Unicode* outlineStyle3Level1[]
    = { outlinePrefix31,          outlineNumType31,
        outlineSuffix31,          outlineBulletChar31,
        outlineBulletFontName31,  outlineParentNumbering31,
        outlineLeftMargin31,      outlineSymbolTextDistance31,
        outlineFirstLineOffset31, outlineTransliteration31,
        outlineNatNum31,          NULL };
static const sal_Unicode* outlineStyle3Level2[]
    = { outlinePrefix32,          outlineNumType32,
        outlineSuffix32,          outlineBulletChar32,
        outlineBulletFontName32,  outlineParentNumbering32,
        outlineLeftMargin32,      outlineSymbolTextDistance32,
        outlineFirstLineOffset32, outlineTransliteration32,
        outlineNatNum32,          NULL };
static const sal_Unicode* outlineStyle3Level3[]
    = { outlinePrefix33,          outlineNumType33,
        outlineSuffix33,          outlineBulletChar33,
        outlineBulletFontName33,  outlineParentNumbering33,
        outlineLeftMargin33,      outlineSymbolTextDistance33,
        outlineFirstLineOffset33, outlineTransliteration33,
        outlineNatNum33,          NULL };
static const sal_Unicode* outlineStyle3Level4[]
    = { outlinePrefix34,          outlineNumType34,
        outlineSuffix34,          outlineBulletChar34,
        outlineBulletFontName34,  outlineParentNumbering34,
        outlineLeftMargin34,      outlineSymbolTextDistance34,
        outlineFirstLineOffset34, outlineTransliteration34,
        outlineNatNum34,          NULL };
static const sal_Unicode* outlineStyle4Level0[]
    = { outlinePrefix40,          outlineNumType40,
        outlineSuffix40,          outlineBulletChar40,
        outlineBulletFontName40,  outlineParentNumbering40,
        outlineLeftMargin40,      outlineSymbolTextDistance40,
        outlineFirstLineOffset40, outlineTransliteration40,
        outlineNatNum40,          NULL };
static const sal_Unicode* outlineStyle4Level1[]
    = { outlinePrefix41,          outlineNumType41,
        outlineSuffix41,          outlineBulletChar41,
        outlineBulletFontName41,  outlineParentNumbering41,
        outlineLeftMargin41,      outlineSymbolTextDistance41,
        outlineFirstLineOffset41, outlineTransliteration41,
        outlineNatNum41,          NULL };
static const sal_Unicode* outlineStyle4Level2[]
    = { outlinePrefix42,          outlineNumType42,
        outlineSuffix42,          outlineBulletChar42,
        outlineBulletFontName42,  outlineParentNumbering42,
        outlineLeftMargin42,      outlineSymbolTextDistance42,
        outlineFirstLineOffset42, outlineTransliteration42,
        outlineNatNum42,          NULL };
static const sal_Unicode* outlineStyle4Level3[]
    = { outlinePrefix43,          outlineNumType43,
        outlineSuffix43,          outlineBulletChar43,
        outlineBulletFontName43,  outlineParentNumbering43,
        outlineLeftMargin43,      outlineSymbolTextDistance43,
        outlineFirstLineOffset43, outlineTransliteration43,
        outlineNatNum43,          NULL };
static const sal_Unicode* outlineStyle4Level4[]
    = { outlinePrefix44,          outlineNumType44,
        outlineSuffix44,          outlineBulletChar44,
        outlineBulletFontName44,  outlineParentNumbering44,
        outlineLeftMargin44,      outlineSymbolTextDistance44,
        outlineFirstLineOffset44, outlineTransliteration44,
        outlineNatNum44,          NULL };
static const sal_Unicode* outlineStyle5Level0[]
    = { outlinePrefix50,          outlineNumType50,
        outlineSuffix50,          outlineBulletChar50,
        outlineBulletFontName50,  outlineParentNumbering50,
        outlineLeftMargin50,      outlineSymbolTextDistance50,
        outlineFirstLineOffset50, outlineTransliteration50,
        outlineNatNum50,          NULL };
static const sal_Unicode* outlineStyle5Level1[]
    = { outlinePrefix51,          outlineNumType51,
        outlineSuffix51,          outlineBulletChar51,
        outlineBulletFontName51,  outlineParentNumbering51,
        outlineLeftMargin51,      outlineSymbolTextDistance51,
        outlineFirstLineOffset51, outlineTransliteration51,
        outlineNatNum51,          NULL };
static const sal_Unicode* outlineStyle5Level2[]
    = { outlinePrefix52,          outlineNumType52,
        outlineSuffix52,          outlineBulletChar52,
        outlineBulletFontName52,  outlineParentNumbering52,
        outlineLeftMargin52,      outlineSymbolTextDistance52,
        outlineFirstLineOffset52, outlineTransliteration52,
        outlineNatNum52,          NULL };
static const sal_Unicode* outlineStyle5Level3[]
    = { outlinePrefix53,          outlineNumType53,
        outlineSuffix53,          outlineBulletChar53,
        outlineBulletFontName53,  outlineParentNumbering53,
        outlineLeftMargin53,      outlineSymbolTextDistance53,
        outlineFirstLineOffset53, outlineTransliteration53,
        outlineNatNum53,          NULL };
static const sal_Unicode* outlineStyle5Level4[]
    = { outlinePrefix54,          outlineNumType54,
        outlineSuffix54,          outlineBulletChar54,
        outlineBulletFontName54,  outlineParentNumbering54,
        outlineLeftMargin54,      outlineSymbolTextDistance54,
        outlineFirstLineOffset54, outlineTransliteration54,
        outlineNatNum54,          NULL };
static const sal_Unicode* outlineStyle6Level0[]
    = { outlinePrefix60,          outlineNumType60,
        outlineSuffix60,          outlineBulletChar60,
        outlineBulletFontName60,  outlineParentNumbering60,
        outlineLeftMargin60,      outlineSymbolTextDistance60,
        outlineFirstLineOffset60, outlineTransliteration60,
        outlineNatNum60,          NULL };
static const sal_Unicode* outlineStyle6Level1[]
    = { outlinePrefix61,          outlineNumType61,
        outlineSuffix61,          outlineBulletChar61,
        outlineBulletFontName61,  outlineParentNumbering61,
        outlineLeftMargin61,      outlineSymbolTextDistance61,
        outlineFirstLineOffset61, outlineTransliteration61,
        outlineNatNum61,          NULL };
static const sal_Unicode* outlineStyle6Level2[]
    = { outlinePrefix62,          outlineNumType62,
        outlineSuffix62,          outlineBulletChar62,
        outlineBulletFontName62,  outlineParentNumbering62,
        outlineLeftMargin62,      outlineSymbolTextDistance62,
        outlineFirstLineOffset62, outlineTransliteration62,
        outlineNatNum62,          NULL };
static const sal_Unicode* outlineStyle6Level3[]
    = { outlinePrefix63,          outlineNumType63,
        outlineSuffix63,          outlineBulletChar63,
        outlineBulletFontName63,  outlineParentNumbering63,
        outlineLeftMargin63,      outlineSymbolTextDistance63,
        outlineFirstLineOffset63, outlineTransliteration63,
        outlineNatNum63,          NULL };
static const sal_Unicode* outlineStyle6Level4[]
    = { outlinePrefix64,          outlineNumType64,
        outlineSuffix64,          outlineBulletChar64,
        outlineBulletFontName64,  outlineParentNumbering64,
        outlineLeftMargin64,      outlineSymbolTextDistance64,
        outlineFirstLineOffset64, outlineTransliteration64,
        outlineNatNum64,          NULL };
static const sal_Unicode* outlineStyle7Level0[]
    = { outlinePrefix70,          outlineNumType70,
        outlineSuffix70,          outlineBulletChar70,
        outlineBulletFontName70,  outlineParentNumbering70,
        outlineLeftMargin70,      outlineSymbolTextDistance70,
        outlineFirstLineOffset70, outlineTransliteration70,
        outlineNatNum70,          NULL };
static const sal_Unicode* outlineStyle7Level1[]
    = { outlinePrefix71,          outlineNumType71,
        outlineSuffix71,          outlineBulletChar71,
        outlineBulletFontName71,  outlineParentNumbering71,
        outlineLeftMargin71,      outlineSymbolTextDistance71,
        outlineFirstLineOffset71, outlineTransliteration71,
        outlineNatNum71,          NULL };
static const sal_Unicode* outlineStyle7Level2[]
    = { outlinePrefix72,          outlineNumType72,
        outlineSuffix72,          outlineBulletChar72,
        outlineBulletFontName72,  outlineParentNumbering72,
        outlineLeftMargin72,      outlineSymbolTextDistance72,
        outlineFirstLineOffset72, outlineTransliteration72,
        outlineNatNum72,          NULL };
static const sal_Unicode* outlineStyle7Level3[]
    = { outlinePrefix73,          outlineNumType73,
        outlineSuffix73,          outlineBulletChar73,
        outlineBulletFontName73,  outlineParentNumbering73,
        outlineLeftMargin73,      outlineSymbolTextDistance73,
        outlineFirstLineOffset73, outlineTransliteration73,
        outlineNatNum73,          NULL };
static const sal_Unicode* outlineStyle7Level4[]
    = { outlinePrefix74,          outlineNumType74,
        outlineSuffix74,          outlineBulletChar74,
        outlineBulletFontName74,  outlineParentNumbering74,
        outlineLeftMargin74,      outlineSymbolTextDistance74,
        outlineFirstLineOffset74, outlineTransliteration74,
        outlineNatNum74,          NULL };

static const sal_Unicode** outlineStyle0[]
    = { outlineStyle0Level0, outlineStyle0Level1, outlineStyle0Level2,
        outlineStyle0Level3, outlineStyle0Level4, NULL };
static const sal_Unicode** outlineStyle1[]
    = { outlineStyle1Level0, outlineStyle1Level1, outlineStyle1Level2,
        outlineStyle1Level3, outlineStyle1Level4, NULL };
static const sal_Unicode** outlineStyle2[]
    = { outlineStyle2Level0, outlineStyle2Level1, outlineStyle2Level2,
        outlineStyle2Level3, outlineStyle2Level4, NULL };
static const sal_Unicode** outlineStyle3[]
    = { outlineStyle3Level0, outlineStyle3Level1, outlineStyle3Level2,
        outlineStyle3Level3, outlineStyle3Level4, NULL };
static const sal_Unicode** outlineStyle4[]
    = { outlineStyle4Level0, outlineStyle4Level1, outlineStyle4Level2,
        outlineStyle4Level3, outlineStyle4Level4, NULL };
static const sal_Unicode** outlineStyle5[]
    = { outlineStyle5Level0, outlineStyle5Level1, outlineStyle5Level2,
        outlineStyle5Level3, outlineStyle5Level4, NULL };
static const sal_Unicode** outlineStyle6[]
    = { outlineStyle6Level0, outlineStyle6Level1, outlineStyle6Level2,
        outlineStyle6Level3, outlineStyle6Level4, NULL };
static const sal_Unicode** outlineStyle7[]
    = { outlineStyle7Level0, outlineStyle7Level1, outlineStyle7Level2,
        outlineStyle7Level3, outlineStyle7Level4, NULL };

static const sal_Unicode*** LCOutlineNumberingLevelsArray[]
    = { outlineStyle0, outlineStyle1, outlineStyle2, outlineStyle3, outlineStyle4,
        outlineStyle5, outlineStyle6, outlineStyle7, NULL };

SAL_DLLPUBLIC_EXPORT const sal_Unicode**** SAL_CALL
getOutlineNumberingLevels_zh_TW(sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes)
{
    nStyles = outlineNbOfStyles;
    nLevels = outlineNbOfLevelsPerStyle;
    nAttributes = outlineNbOfAttributesPerLevel;
    return LCOutlineNumberingLevelsArray;
}
} // extern "C"
