#define MAX_LANGUAGES           99

#define COMMENT                 0
#define COMMENT_ISO             "x-comment"
#define COMMENT_INDEX               0

#define ENGLISH_US              1
#define ENGLISH_US_ISO          "en-US"
#define ENGLISH_US_PROPERTY     "en_US"
#define ENGLISH_US_INDEX            1


#define PORTUGUESE              3
#define PORTUGUESE_ISO          "pt"
#define PORTUGUESE_INDEX            2
#define PORTUGUESE_ISO2                 "pt-PT"

#define GERMAN_DE               4
#define GERMAN_DE_ISO           "x-german"
#define GERMAN_DE_INDEX             3
#define GERMAN_ISO2                     "de-DE"

#define RUSSIAN                 7
#define RUSSIAN_ISO             "ru"
#define RUSSIAN_INDEX               4
#define RUSSIAN_ISO2                    "ru-RU"

#define GREEK                   30
#define GREEK_ISO               "el"
#define GREEK_INDEX                 5
#define GREEK_ISO2                      "el-GR"

#define DUTCH                   31
#define DUTCH_ISO               "nl"
#define DUTCH_INDEX                 6
#define DUTCH_ISO2                      "nl-NL"

#define FRENCH                  33
#define FRENCH_ISO              "fr"
#define FRENCH_INDEX                7
#define FRENCH_ISO2                     "fr-FR"

#define SPANISH                 34
#define SPANISH_ISO             "es"
#define SPANISH_INDEX               8
#define SPANISH_ISO2                    "es-ES"

#define FINNISH                 35
#define FINNISH_ISO             "fi"
#define FINNISH_INDEX               9
#define FINNISH_ISO2                    "fi-FI"

#define HUNGARIAN               36
#define HUNGARIAN_ISO           "hu"
#define HUNGARIAN_INDEX             10
#define HUNGARIAN_ISO2                  "hu-HU"

#define ITALIAN                 39
#define ITALIAN_ISO             "it"
#define ITALIAN_INDEX               11
#define ITALIAN_ISO2                    "it-IT"

#define CZECH                   42
#define CZECH_ISO               "cs"
#define CZECH_INDEX                 12
#define CZECH_ISO2                      "cs-CZ"

#define SLOVAK                  43
#define SLOVAK_ISO              "sk"
#define SLOVAK_INDEX                13
#define SLOVAK_ISO2                     "sk-SK"

#define ENGLISH                 44
#define ENGLISH_ISO             "x-translate"
#define ENGLISH_INDEX               14


#define DANISH                  45
#define DANISH_ISO              "da"
#define DANISH_INDEX                15
#define DANISH_ISO2                     "da-DK"

#define SWEDISH                 46
#define SWEDISH_ISO             "sv"
#define SWEDISH_INDEX               16
#define SWEDISH_ISO2                    "sv-SE"

#define NORWEGIAN               47
#define NORWEGIAN_ISO           "no"
#define NORWEGIAN_INDEX             17
#define NORWEGIAN_ISO2                  "no-NO"

#define POLISH                  48
#define POLISH_ISO              "pl"
#define POLISH_INDEX                18
#define POLISH_ISO2                     "pl-PL"

#define GERMAN                  49
#define GERMAN_ISO              "de"
#define GERMAN_INDEX                19
#define GERMAN_ISO2                     "de-DE"

#define PORTUGUESE_BRAZILIAN    55
#define PORTUGUESE_BRAZILIAN_ISO "pt-BR"
#define PORTUGUESE_BRAZILIAN_PROPERTY "pt_BR"
#define PORTUGUESE_BRAZILIAN_INDEX  20
#define PORTUGUESE_BRAZILIAN_ISO2       "pt-BR"

#define JAPANESE                81
#define JAPANESE_ISO            "ja"
#define JAPANESE_INDEX              21
#define JAPANESE_ISO2                   "ja-JP"

#define KOREAN                  82
#define KOREAN_ISO              "ko"
#define KOREAN_INDEX                22
#define KOREAN_ISO2                     "ko-KO"

#define CHINESE_SIMPLIFIED      86
#define CHINESE_SIMPLIFIED_ISO  "zh-CN"
#define CHINESE_SIMPLIFIED_PROPERTY "zh_CN"
#define CHINESE_SIMPLIFIED_INDEX    23
#define CHINESE_SIMPLIFIED_ISO2         "zh-CN"

#define CHINESE_TRADITIONAL     88
#define CHINESE_TRADITIONAL_ISO "zh-TW"
#define CHINESE_TRADITIONAL_PROPERTY    "zh_TW"
#define CHINESE_TRADITIONAL_INDEX   24
#define CHINESE_TRADITIONAL_ISO2        "zh-TW"

#define TURKISH                 90
#define TURKISH_ISO             "tr"
#define TURKISH_INDEX               25
#define TURKISH_ISO2                    "tr-TR"

#define ARABIC                  96
#define ARABIC_ISO              "ar"
#define ARABIC_INDEX                26


#define HEBREW                  97
#define HEBREW_ISO              "he"
#define HEBREW_INDEX                27
#define HEBREW_ISO2                     "he-IL"

#define CATALAN                 37
#define CATALAN_ISO             "ca"
#define CATALAN_INDEX               28
#define CATALAN_ISO2                    "ca-ES"

#define THAI                    66
#define THAI_ISO                "th"
#define THAI_INDEX                  29
#define THAI_ISO2                       "th-TH"

// special language for l10n framework
#define EXTERN                  99
#define EXTERN_INDEX                30

#define LANGUAGES                   31

#define LANGUAGE_ALLOWED( n )  (( n != 0xFFFF ) && ( Export::LanguageAllowed( Export::LangId[ n ] )) &&                                                     \
                               (( Export::LangId[ n ] == 01 ) || ( Export::LangId[ n ] == 03 ) || ( Export::LangId[ n ] == 07 ) ||  \
                                ( Export::LangId[ n ] == 30 ) || ( Export::LangId[ n ] == 31 ) || ( Export::LangId[ n ] == 33 ) ||      \
                                ( Export::LangId[ n ] == 34 ) || ( Export::LangId[ n ] == 35 ) || ( Export::LangId[ n ] == 37 ) ||  \
                                ( Export::LangId[ n ] == 39 ) || ( Export::LangId[ n ] == 45 ) || ( Export::LangId[ n ] == 46 ) ||  \
                                ( Export::LangId[ n ] == 48 ) || ( Export::LangId[ n ] == 49 ) || ( Export::LangId[ n ] == 55 ) ||  \
                                ( Export::LangId[ n ] == 81 ) || ( Export::LangId[ n ] == 82 ) || ( Export::LangId[ n ] == 86 ) ||  \
                                ( Export::LangId[ n ] == 88 ) || ( Export::LangId[ n ] == 90 ) || ( Export::LangId[ n ] == 96 ) ||  \
                                ( Export::LangId[ n ] == 66 ) || ( Export::LangId[ n ] == 00 ) || ( Export::LangId[ n ] == 99 )))

#define GERMAN_LIST_LINE_INDEX  LANGUAGES
#define LIST_REFID              (GERMAN_LIST_LINE_INDEX+1)

