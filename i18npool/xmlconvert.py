#!/usr/bin/env python3

import os, sys
import xml.etree.ElementTree as ET

header = '''#include <sal/types.h>


#include <stdio.h>

extern "C" {

'''

footer = '''} // extern "C"

'''

getlcinfo = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLCInfo_{}(sal_Int16& count)
{{
\tcount = SAL_N_ELEMENTS(LCInfoArray);
\treturn (sal_Unicode**)LCInfoArray;
}}
'''

getlitem = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getLocaleItem_{}(sal_Int16& count)
{{
\tcount = SAL_N_ELEMENTS(LCType);
\treturn (sal_Unicode**)LCType;
}}
'''

date_acc_patterns = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getDateAcceptancePatterns_{}(sal_Int16& count)
{{
\tcount = DateAcceptancePatternsCount;
\treturn (sal_Unicode**)DateAcceptancePatternsArray;
}}
'''
# Tabs are bad but the output must be identical with the old code.
indent = '\t'

collation_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollatorImplementation_{}(sal_Int16& count)
{{
\tcount = nbOfCollations;
\treturn (sal_Unicode**)LCCollatorArray;
}}
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getCollationOptions_{}(sal_Int16& count)
{{
\tcount = nbOfCollationOptions;
\treturn (sal_Unicode**)collationOptions;
}}
'''


search_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getSearchOptions_{}(sal_Int16& count)
{{
\tcount = nbOfSearchOptions;
\treturn (sal_Unicode**)searchOptions;
}}
'''

index_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getIndexAlgorithm_{}(sal_Int16& count)
{{
\tcount = nbOfIndexs;
\treturn (sal_Unicode**)IndexArray;
}}
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getUnicodeScripts_{}(sal_Int16& count)
{{
\tcount = nbOfUnicodeScripts;
\treturn (sal_Unicode**)UnicodeScriptArray;
}}
SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getFollowPageWords_{}(sal_Int16& count)
{{
\tcount = nbOfPageWords;
\treturn (sal_Unicode**)FollowPageWordArray;
}}
'''

calendar_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCalendars_{}(sal_Int16& count)
{{
\tcount = calendarsCount;
\treturn (sal_Unicode**)calendars;
}}
'''

currency_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getAllCurrencies_{}(sal_Int16& count)
{{
\tcount = currencyCount;
\treturn (sal_Unicode**)currencies;
}}
'''

translit_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getTransliterations_{}(sal_Int16& count)
{{
\tcount = nbOfTransliterations;
\treturn (sal_Unicode**)LCTransliterationsArray;
}}
'''

resword_templ = '''SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getReservedWords_{}(sal_Int16& count)
{{
\tcount = nbOfReservedWords;
\treturn (sal_Unicode**)LCReservedWordsArray;
}}
'''

fixme_hack = '''static const sal_Unicode forbiddenBegin[] = {{0x0}};
static const sal_Unicode forbiddenEnd[] = {{0x0}};
static const sal_Unicode hangingChars[] = {{0x0}};

static const sal_Unicode* LCForbiddenCharactersArray[] = {{
\tforbiddenBegin,
\tforbiddenEnd,
\thangingChars
}};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getForbiddenCharacters_{}(sal_Int16& count)
{{
\tcount = 3;
\treturn (sal_Unicode**)LCForbiddenCharactersArray;
}}
static const sal_Unicode EditMode[] = {{0x0}};
static const sal_Unicode DictionaryMode[] = {{0x0}};
static const sal_Unicode WordCountMode[] = {{0x0}};
static const sal_Unicode CharacterMode[] = {{0x0}};
static const sal_Unicode LineMode[] = {{0x0}};

static const sal_Unicode* LCBreakIteratorRulesArray[] = {{
\tEditMode,
\tDictionaryMode,
\tWordCountMode,
\tCharacterMode,
\tLineMode
}};

SAL_DLLPUBLIC_EXPORT sal_Unicode **  SAL_CALL getBreakIteratorRules_{}(sal_Int16& count)
{{
\tcount = 5;
\treturn (sal_Unicode**)LCBreakIteratorRulesArray;
}}
'''

cont_templ = '''SAL_DLLPUBLIC_EXPORT const sal_Unicode ***  SAL_CALL getContinuousNumberingLevels_{}( sal_Int16& nStyles, sal_Int16& nAttributes )
{{
\tnStyles     = continuousNbOfStyles;
\tnAttributes = continuousNbOfAttributesPerStyle;
\treturn LCContinuousNumberingLevelsArray;
}}
'''

outline_templ = '''SAL_DLLPUBLIC_EXPORT const sal_Unicode ****  SAL_CALL getOutlineNumberingLevels_{}( sal_Int16& nStyles, sal_Int16& nLevels, sal_Int16& nAttributes )
{{
\tnStyles     = outlineNbOfStyles;
\tnLevels     = outlineNbOfLevelsPerStyle;
\tnAttributes = outlineNbOfAttributesPerLevel;
\treturn LCOutlineNumberingLevelsArray;
}}
'''

class XmlConverter:
    def __init__(self, locale_name, ifilename, ofilename):
        self.data_dir = os.path.split(ifilename)[0]
        self.locale_name = locale_name
        self.root = ET.parse(ifilename)
        self.ofile = open(ofilename, 'w')

    def load_locale(self, locale):
        fname = os.path.join(self.data_dir, locale + '.xml')
        return ET.parse(fname)


    def write_unicode_string(self, name, contents):
        content_arr = ['0x%x' % ord(i) for i in contents] + ['0x0']
        s = '{' + ', '.join(content_arr) + '}'
        self.ofile.write('static const sal_Unicode {}[] = {};\n'.format(name, s))

    def write_unicode_bool(self, c_name, value):
        v = 1 if value else 0
        self.ofile.write('static const sal_Unicode {}[] = {{{}}};\n'.format(c_name, v))

    def write_unicode_int(self, c_name, value):
        self.ofile.write('static const sal_Unicode {}[] = {{{}}};\n'.format(c_name, value))

    def write_array(self, name, entries, ctype='sal_Unicode*'):
        self.ofile.write('static const {} {}[] = {{\n'.format(ctype, name))
        t = ',\n'.join([indent + i for i in entries])
        self.ofile.write(t)
        self.ofile.write('\n};\n\n')

    def convert_xpaths(self, items):
        for c_name, xpath in items:
            n = self.root.find(xpath)
            if n is None:
                value = ''
            else:
                value = n.text
            self.write_unicode_string(c_name, value)
        self.ofile.write('\n')

    def convert_lcinfo(self):
        items = [('langID', './LC_INFO/Language/LangID'),
                 ('langDefaultName', './LC_INFO/Language/DefaultName'),
                 ('countryID', './LC_INFO/Country/CountryID'),
                 ('countryDefaultName', './LC_INFO/Country/DefaultName'),
                 ('Variant', './LC_INFO/Country/Variant')]
        self.convert_xpaths(items)
        self.write_array('LCInfoArray', [x[0] for x in items])
        self.ofile.write(getlcinfo.format(self.locale_name))
        self.ofile.write('\n\n')

    def convert_lctype(self):
        items = [('dateSeparator', './LC_CTYPE/Separators/DateSeparator'),
                 ('thousandSeparator', './LC_CTYPE/Separators/ThousandSeparator'),
                 ('decimalSeparator', './LC_CTYPE/Separators/DecimalSeparator'),
                 ('timeSeparator', './LC_CTYPE/Separators/TimeSeparator'),
                 ('time100SecSeparator', './LC_CTYPE/Separators/Time100SecSeparator'),
                 ('listSeparator', './LC_CTYPE/Separators/ListSeparator'),
                 ('quotationStart', './LC_CTYPE/Markers/QuotationStart'),
                 ('quotationEnd', './LC_CTYPE/Markers/QuotationEnd'),
                 ('doubleQuotationStart', './LC_CTYPE/Markers/DoubleQuotationStart'),
                 ('doubleQuotationEnd', './LC_CTYPE/Markers/DoubleQuotationEnd'),
                 ('timeAM', './LC_CTYPE/TimeAM'),
                 ('timePM', './LC_CTYPE/TimePM'),
                 ('measurementSystem', './LC_CTYPE/MeasurementSystem'),
                 ('LongDateDayOfWeekSeparator', './LC_CTYPE/Separators/LongDateDayOfWeekSeparator'),
                 ('LongDateDaySeparator', './LC_CTYPE/Separators/LongDateDaySeparator'),
                 ('LongDateMonthSeparator', './LC_CTYPE/Separators/LongDateMonthSeparator'),
                 ('LongDateYearSeparator', './LC_CTYPE/Separators/LongDateYearSeparator'),
                 ('decimalSeparatorAlternative', './LC_CTYPE/Separators/DecimalSeparator'),
                 ]
        unoid = self.root.find('LC_CTYPE').attrib.get('unoid', '')
        self.write_unicode_string('LC_CTYPE_Unoid', unoid)
        self.convert_xpaths(items)
        self.write_array('LCType', ['LC_CTYPE_Unoid'] + [x[0] for x in items])
        self.ofile.write(getlitem.format(self.locale_name))
        self.ofile.write('\n\n')

    def convert_xattr(self, xpath, aname, oname=None):
        if oname is None:
            oname = aname
        e = self.root.find(xpath)
        if e is None:
            value = ''
        else:
            value = e.attrib.get(aname, '')
        self.write_unicode_string(oname, value)

    def convert_lcf(self, name, lc_index):
        self.convert_xattr('./{}'.format(name), 'replaceFrom', 'replaceFrom{}'.format(lc_index))
        self.convert_xattr('./{}'.format(name), 'replaceTo', 'replaceTo{}'.format(lc_index))

    def convert_lcf_content(self, node, idnum, convert_offset):
        num_converted = 0
        for e in self.root.findall('./{}/FormatElement'.format(node)):
            items = [('FormatKey', 'msgid'),
                     ('defaultFormatElement', 'default'),
                     ('FormatType', 'type'),
                     ('FormatUsage', 'usage'),
                     ('Formatindex', 'formatindex'),]
            for basename, attrname in items:
                c_name = '{}{}'.format(basename, num_converted + convert_offset)
                value = e.attrib.get(attrname, '')
                if c_name.startswith('default'):
                    assert(value == 'true' or value == 'false')
                    self.write_unicode_bool(c_name, value == 'true')
                elif c_name.startswith('Formatindex'):
                    self.write_unicode_int(c_name, int(value))
                else:
                    self.write_unicode_string(c_name, value)
            code = e.find('FormatCode').text
            self.write_unicode_string('FormatCode{}'.format(num_converted + convert_offset),
                                      code)
            fname = e.find('DefaultName')
            if fname is None:
                defname = ''
            else:
                defname = fname.text
            self.write_unicode_string('FormatDefaultName{}'.format(num_converted + convert_offset),
                                      defname)
            num_converted += 1

        self.ofile.write('\nstatic const sal_Int16 FormatElementsCount{} = {};\n'.format(idnum, num_converted))
        self.ofile.write('static const sal_Unicode* FormatElementsArray{}[] = {{\n'.format(idnum))
        for i in range(num_converted):
            for j in ('FormatCode',
                      'FormatDefaultName',
                      'FormatKey',
                      'FormatType',
                      'FormatUsage',
                      'Formatindex',
                      'defaultFormatElement'):
                self.ofile.write('\t{}{},\n'.format(j, i + convert_offset))
        self.ofile.write('};\n\n')

        self.ofile.write('''SAL_DLLPUBLIC_EXPORT sal_Unicode const * const * SAL_CALL getAllFormats{}_{}(sal_Int16& count, const sal_Unicode*& from, const sal_Unicode*& to)
{{
\tcount = FormatElementsCount{};
\tfrom = replaceFrom{};
\tto = replaceTo{};
\treturn (sal_Unicode**)FormatElementsArray{};
}}
'''.format(idnum, self.locale_name, idnum, idnum, idnum, idnum))
        # FIXME: unclear where this comes from
        if node == 'LC_FORMAT':
            self.write_unicode_string('DateAcceptancePattern0', 'M/D/Y')
            date_entries = ['DateAcceptancePattern0']
            date_patterns = 1
            for dap in self.root.findall('./{}/DateAcceptancePattern'.format(node)):
                pattern = dap.text
                name = 'DateAcceptancePattern{}'.format(date_patterns)
                self.write_unicode_string(name, pattern)
                date_patterns += 1
                date_entries.append(name)
            self.ofile.write('static const sal_Int16 DateAcceptancePatternsCount = {};\n'.format(date_patterns))
            self.write_array('DateAcceptancePatternsArray', date_entries)
            self.ofile.write(date_acc_patterns.format(self.locale_name))
        return num_converted

    def convert_lcformat(self):
        self.convert_lcf('LC_FORMAT', 0)
        num_converted = self.convert_lcf_content('LC_FORMAT', 0, 0)

        if self.root.find('LC_FORMAT_1'):
            self.convert_lcf('LC_FORMAT_1', 1)
            self.convert_lcf_content('LC_FORMAT_1', 1, num_converted)

    def convert_lccollation(self):
        num_collators = 0
        num_options = 0
        for coll in self.root.findall('./LC_COLLATION/Collator'):
            cid = coll.attrib['unoid']
            default = coll.attrib['default'] == 'true'
            rule = coll.text
            if rule is None:
                rule = ''
            self.write_unicode_string('CollatorID{}'.format(num_collators), cid)
            self.write_unicode_string('CollatorRule{}'.format(num_collators), rule)
            self.write_unicode_bool('defaultCollator{}'.format(num_collators), default)
            num_collators += 1

        self.ofile.write('\n')
        collopt_names = []
        for collopt in self.root.findall('./LC_COLLATION/CollationOptions'):
            optname = collopt.find('TransliterationModules').text
            collopt_names.append('collationOption{}'.format(num_options))
            self.write_unicode_string(collopt_names[-1], optname)
            num_options += 1

        self.ofile.write('static const sal_Int16 nbOfCollationOptions = {};\n\n'.format(num_options))
        self.ofile.write('static const sal_Int16 nbOfCollations = {};\n\n\n'.format(num_collators))
        names = []
        for i in range(num_collators):
            names.append('CollatorID{}'.format(i))
            names.append('defaultCollator{}'.format(i))
            names.append('CollatorRule{}'.format(i))
        self.write_array('LCCollatorArray', names)
        self.ofile.write('\n')
        tmp = collopt_names + ['NULL']
        self.ofile.write('static const sal_Unicode* collationOptions[] = {{{} }};\n'.format(', '.join(tmp)))
        self.ofile.write(collation_templ.format(self.locale_name, self.locale_name))

    def convert_lcsearch(self):
        num_options = 0
        searchopt_names = []
        for collopt in self.root.findall('./LC_SEARCH/SearchOptions'):
            optname = collopt.find('TransliterationModules').text
            searchopt_names.append('searchOption{}'.format(num_options))
            self.write_unicode_string(searchopt_names[-1], optname)
            num_options += 1

        self.ofile.write('static const sal_Int16 nbOfSearchOptions = {};\n\n'.format(num_options))
        tmp = searchopt_names + ['NULL']
        self.ofile.write('static const sal_Unicode* searchOptions[] = {{{} }};\n'.format(', '.join(tmp)))
        self.ofile.write(search_templ.format(self.locale_name))

    def convert_lcindex(self):
        num_indexes = 0
        num_unicode_scripts = 0
        num_follows = 0
        for i in self.root.findall('./LC_INDEX/IndexKey'):
            iid = i.attrib['unoid']
            mod = i.attrib.get('module', '')
            ikey = i.text
            default = i.attrib['default'] == 'true'
            phonetic = i.attrib['phonetic'] == 'true'
            self.write_unicode_string('IndexID{}'.format(num_indexes), iid)
            self.write_unicode_string('IndexModule{}'.format(num_indexes), mod)
            self.write_unicode_string('IndexKey{}'.format(num_indexes), ikey)
            self.write_unicode_bool('defaultIndex{}'.format(num_indexes), default)
            self.write_unicode_bool('defaultPhonetic{}'.format(num_indexes), phonetic)
            num_indexes += 1

        for i in self.root.findall('./LC_INDEX/UnicodeScript'):
            s = i.text
            self.write_unicode_string('unicodeScript{}'.format(num_unicode_scripts), s)
            num_unicode_scripts += 1

        for i in self.root.findall('./LC_INDEX/FollowPageWord'):
            s = i.text
            self.write_unicode_string('followPageWord{}'.format(num_follows), s)
            num_follows += 1

        self.ofile.write('static const sal_Int16 nbOfIndexs = {};\n\n\n'.format(num_indexes))
        arr = []
        for i in range(num_indexes):
            for j in ('IndexID{}',
                      'IndexModule{}',
                      'IndexKey{}',
                      'defaultIndex{}',
                      'defaultPhonetic{}'):
                arr.append(j.format(i))
        self.write_array('IndexArray', arr)

        self.ofile.write('\nstatic const sal_Int16 nbOfUnicodeScripts = {};\n'.format(num_unicode_scripts))
        arr = []
        for i in range(num_unicode_scripts):
            arr.append('unicodeScript{}'.format(i))
        arr.append('NULL')
        self.ofile.write('static const sal_Unicode* UnicodeScriptArray[] = {{{} }};\n'.format(', '.join(arr)))

        self.ofile.write('\nstatic const sal_Int16 nbOfPageWords = {};\n\n'.format(num_follows))
        arr = []
        for i in range(num_follows):
            arr.append('followPageWord{}'.format(i))
        arr.append('NULL')
        self.write_array('FollowPageWordArray', arr)
        self.ofile.write(index_templ.format(self.locale_name,
                                            self.locale_name,
                                            self.locale_name))

    def convert_months(self, nodes, cal_num, typestr):
        num_months = 0
        if typestr == '':
            em = 'm'
        else:
            em = 'M'
        for month in nodes:
            monthid = month.find('MonthID').text
            abbrname = month.find('DefaultAbbrvName').text
            fullname = month.find('DefaultFullName').text
            narrowname = month.attrib.get('DefaultNarrowName', fullname[0])
            self.write_unicode_string('{}{}onthID{}{}'.format(typestr, em, cal_num, num_months), monthid)
            self.write_unicode_string('{}{}onthDefaultAbbrvName{}{}'.format(typestr, em, cal_num, num_months), abbrname)
            self.write_unicode_string('{}{}onthDefaultFullName{}{}'.format(typestr, em, cal_num, num_months), fullname)
            self.write_unicode_string('{}{}onthDefaultNarrowName{}{}'.format(typestr, em, cal_num, num_months), narrowname)
            num_months += 1
        return num_months

    def convert_calendar(self):
        day_counts = []
        month_counts = []
        era_counts = []
        num_calendars = 0
        for c in self.root.findall('./LC_CALENDAR/Calendar'):
            num_days = 0
            calid = c.attrib['unoid']
            caldef = c.attrib['default'] == 'true'
            self.write_unicode_string('calendarID{}'.format(num_calendars), calid)
            self.write_unicode_bool('defaultCalendar{}'.format(num_calendars), caldef)
            for day in c.findall('./DaysOfWeek/Day'):
                dayid = day.find('DayID').text
                abbrname = day.find('DefaultAbbrvName').text
                fullname = day.find('DefaultFullName').text
                narrowname = day.attrib.get('DefaultNarrowName', fullname[0])
                self.write_unicode_string('dayID{}{}'.format(num_calendars, num_days), dayid)
                self.write_unicode_string('dayDefaultAbbrvName{}{}'.format(num_calendars, num_days), abbrname)
                self.write_unicode_string('dayDefaultFullName{}{}'.format(num_calendars, num_days), fullname)
                self.write_unicode_string('dayDefaultNarrowName{}{}'.format(num_calendars, num_days), narrowname)
                num_days += 1
            num_months = self.convert_months(c.findall('./MonthsOfYear/Month'), num_calendars, '')
            if c.find('./GenitiveMonths') is not None:
                num_months2 = self.convert_months(c.findall('./GenitiveMonths/Month'), num_calendars, 'genitive')
            else:
                num_months2 = self.convert_months(c.findall('./MonthsOfYear/Month'), num_calendars, 'genitive')
            assert(num_months == num_months2)
            if c.find('./PartitiveMonths') is not None:
                num_months2 = self.convert_months(c.findall('./PartitiveMonths/Month'), num_calendars, 'partitive')
            else:
                num_months2 = self.convert_months(c.findall('./MonthsOfYear/Month'), num_calendars, 'partitive')
            assert(num_months == num_months2)
            num_eras = 0
            for e in c.findall('./Eras/Era'):
                eraid = e.find('EraID').text
                abbrname = e.find('DefaultAbbrvName').text
                fullname = e.find('DefaultFullName').text
                self.write_unicode_string('eraID{}{}'.format(num_calendars, num_eras), eraid)
                self.ofile.write('\n')
                self.write_unicode_string('eraDefaultAbbrvName{}{}'.format(num_calendars, num_eras), abbrname)
                self.write_unicode_string('eraDefaultFullName{}{}'.format(num_calendars, num_eras), fullname)
                num_eras += 1
            firstday = c.find('./StartDayOfWeek/DayID').text
            mindays = c.find('./MinimalDaysInFirstWeek').text
            self.write_unicode_string('startDayOfWeek{}'.format(num_calendars), firstday)
            self.write_unicode_int('minimalDaysInFirstWeek{}'.format(num_calendars), mindays)
            day_counts.append(num_days)
            month_counts.append(num_months)
            era_counts.append(num_eras)
            num_calendars += 1

        self.ofile.write('static const sal_Int16 calendarsCount = {};\n\n'.format(num_calendars))
        self.ofile.write('static const sal_Unicode nbOfDays[] = {{{}}};\n'.format(', '.join([str(x) for x in day_counts])))
        self.ofile.write('static const sal_Unicode nbOfMonths[] = {{{}}};\n'.format(', '.join([str(x) for x in month_counts])))
        self.ofile.write('static const sal_Unicode nbOfGenitiveMonths[] = {{{}}};\n'.format(', '.join([str(x) for x in month_counts])))
        self.ofile.write('static const sal_Unicode nbOfPartitiveMonths[] = {{{}}};\n'.format(', '.join([str(x) for x in month_counts])))
        self.ofile.write('static const sal_Unicode nbOfEras[] = {{{}}};\n'.format(', '.join([str(x) for x in era_counts])))

        arr = ['nbOfDays',
               'nbOfMonths',
               'nbOfGenitiveMonths',
               'nbOfPartitiveMonths',
               'nbOfEras'
               ]
        for i in range(num_calendars):
            arr.append('calendarID{}'.format(i))
            arr.append('defaultCalendar{}'.format(i))
            day = day_counts[i]
            month = month_counts[i]
            era = era_counts[i]
            for d in range(day):
                arr.append('dayID{}{}'.format(i, d))
                arr.append('dayDefaultAbbrvName{}{}'.format(i, d))
                arr.append('dayDefaultFullName{}{}'.format(i, d))
                arr.append('dayDefaultNarrowName{}{}'.format(i, d))
            for d in range(month):
                arr.append('monthID{}{}'.format(i, d))
                arr.append('monthDefaultAbbrvName{}{}'.format(i, d))
                arr.append('monthDefaultFullName{}{}'.format(i, d))
                arr.append('monthDefaultNarrowName{}{}'.format(i, d))
            for d in range(month):
                arr.append('genitiveMonthID{}{}'.format(i, d))
                arr.append('genitiveMonthDefaultAbbrvName{}{}'.format(i, d))
                arr.append('genitiveMonthDefaultFullName{}{}'.format(i, d))
                arr.append('genitiveMonthDefaultNarrowName{}{}'.format(i, d))
            for d in range(month):
                arr.append('partitiveMonthID{}{}'.format(i, d))
                arr.append('partitiveMonthDefaultAbbrvName{}{}'.format(i, d))
                arr.append('partitiveMonthDefaultFullName{}{}'.format(i, d))
                arr.append('partitiveMonthDefaultNarrowName{}{}'.format(i, d))
            for d in range(era):
                arr.append('eraID{}{}'.format(i, d))
                arr.append('eraDefaultAbbrvName{}{}'.format(i, d))
                arr.append('eraDefaultFullName{}{}'.format(i, d))
            arr.append('startDayOfWeek{}'.format(i))
            arr.append('minimalDaysInFirstWeek{}'.format(i))
        self.write_array('calendars', arr)
        self.ofile.write('\n')
        self.ofile.write(calendar_templ.format(self.locale_name))

    def convert_currency(self):
        num_currencies = 0
        for c in self.root.findall('./LC_CURRENCY/Currency'):
            default = c.attrib['default'] == 'true'
            compatible = c.attrib['usedInCompatibleFormatCodes'] == 'true'
            legacy = c.attrib.get('legacyOnly', 'false') == 'true'
            cid = c.find('CurrencyID').text
            symbol = c.find('CurrencySymbol').text
            banksymbol = c.find('BankSymbol').text
            cname = c.find('CurrencyName').text
            decimal = int(c.find('DecimalPlaces').text)
            self.write_unicode_bool('defaultCurrency{}'.format(num_currencies), default)
            self.write_unicode_bool('defaultCurrencyUsedInCompatibleFormatCodes{}'.format(num_currencies),
                                    compatible)
            self.write_unicode_bool('defaultCurrencyLegacyOnly{}'.format(num_currencies), legacy)
            self.write_unicode_string('currencyID{}'.format(num_currencies), cid)
            self.write_unicode_string('currencySymbol{}'.format(num_currencies), symbol)
            self.write_unicode_string('bankSymbol{}'.format(num_currencies), banksymbol)
            self.write_unicode_string('currencyName{}'.format(num_currencies), cname)
            self.write_unicode_int('currencyDecimalPlaces{}'.format(num_currencies), decimal)
            num_currencies += 1

        self.ofile.write('\nstatic const sal_Int16 currencyCount = {};\n\n'.format(num_currencies))
        arr = []
        for i in range(num_currencies):
            arr.append('currencyID{}'.format(i))
            arr.append('currencySymbol{}'.format(i))
            arr.append('bankSymbol{}'.format(i))
            arr.append('currencyName{}'.format(i))
            arr.append('defaultCurrency{}'.format(i))
            arr.append('defaultCurrencyUsedInCompatibleFormatCodes{}'.format(i))
            arr.append('currencyDecimalPlaces{}'.format(i))
            arr.append('defaultCurrencyLegacyOnly{}'.format(i))
        self.write_array('currencies', arr)
        self.ofile.write(currency_templ.format(self.locale_name))

    def convert_transliteration(self):
        num_translits = 0
        for c in self.root.findall('./LC_TRANSLITERATION/Transliteration'):
            unoid = c.attrib['unoid']
            self.write_unicode_string('Transliteration{}'.format(num_translits), unoid)
            num_translits += 1


        self.ofile.write('\nstatic const sal_Int16 nbOfTransliterations = {};\n\n'.format(num_translits))
        arr = []
        for i in range(num_translits):
            arr.append('Transliteration{}'.format(i))
        self.write_array('LCTransliterationsArray', arr)
        self.ofile.write(translit_templ.format(self.locale_name))

    def convert_misc(self):
        num_reserved = 0
        for c in self.root.find('./LC_MISC/ReservedWords'):
            word = c.text
            self.write_unicode_string('ReservedWord{}'.format(num_reserved), word)
            num_reserved += 1
        self.ofile.write('\nstatic const sal_Int16 nbOfReservedWords = {};\n\n'.format(num_reserved))
        arr = []
        for i in range(num_reserved):
            arr.append('ReservedWord{}'.format(i))
        self.write_array('LCReservedWordsArray', arr)
        self.ofile.write(resword_templ.format(self.locale_name))
        # FIXME, not used by en locales so I was too lazy to write this.
        # Needs to be fixed for asian languages (at least).
        self.ofile.write(fixme_hack.format(self.locale_name, self.locale_name))

    def convert_lcnumbering(self):
        num_levels = 0
        for c in self.root.find('./LC_NumberingLevel'):
            numtype = c.attrib['NumType']
            prefix = c.attrib['Prefix']
            suffix = c.attrib['Suffix']
            translit = '' # ??? FIXME
            natnum = ''
            self.write_unicode_string('continuousPrefix{}'.format(num_levels), prefix)
            self.write_unicode_string('continuousNumType{}'.format(num_levels), numtype)
            self.write_unicode_string('continuousSuffix{}'.format(num_levels), suffix)
            self.write_unicode_string('continuousTransliteration{}'.format(num_levels), translit)
            self.write_unicode_string('continuousNatNum{}'.format(num_levels), natnum)
            num_levels += 1
        self.ofile.write('\nstatic const sal_Int16 continuousNbOfStyles = {};\n\n'.format(num_levels))
        self.ofile.write('static const sal_Int16 continuousNbOfAttributesPerStyle = {};\n\n'.format(5))
        for i in range(num_levels):
            arr = []
            arr.append('continuousPrefix{}'.format(i))
            arr.append('continuousNumType{}'.format(i))
            arr.append('continuousSuffix{}'.format(i))
            arr.append('continuousTransliteration{}'.format(i))
            arr.append('continuousNatNum{}'.format(i))
            arr.append('0')
            self.write_array('continuousStyle{}'.format(i), arr)
        arr = []
        for i in range(num_levels):
            arr.append('continuousStyle{}'.format(i))
        self.write_array('LCContinuousNumberingLevelsArray', arr, ctype='sal_Unicode**')
        self.ofile.write(cont_templ.format(self.locale_name))

    def convert_outlinenumbering(self):
        num_outline_styles = 0
        attrmap = (('Prefix', 'outlinePrefix{}{}'),
                   ('NumType', 'outlineNumType{}{}'),
                   ('Suffix', 'outlineSuffix{}{}'),
                   ('BulletChar', 'outlineBulletChar{}{}'),
                   ('BulletFontName', 'outlineBulletFontName{}{}'),
                   ('ParentNumbering', 'outlineParentNumbering{}{}'),
                   ('LeftMargin', 'outlineLeftMargin{}{}'),
                   ('SymbolTextDistance', 'outlineSymbolTextDistance{}{}'),
                   ('FirstLineOffset', 'outlineFirstLineOffset{}{}'),
                   ('Transliteration', 'outlineTransliteration{}{}'),
                   ('NatNum', 'outlineNatNum{}{}')
                   )
        for ostyle in self.root.find('./LC_OutLineNumberingLevel'):
            num_levels = 0
            for level in ostyle:
                for xmlname, headername in attrmap:
                    value = level.attrib.get(xmlname, '')
                    self.write_unicode_string(headername.format(num_outline_styles, num_levels), value)
                num_levels += 1
            num_outline_styles += 1

        self.ofile.write('static const sal_Int16 outlineNbOfStyles = {};\n'.format(num_outline_styles))
        self.ofile.write('\nstatic const sal_Int16 outlineNbOfLevelsPerStyle = {};\n'.format(5))
        self.ofile.write('\nstatic const sal_Int16 outlineNbOfAttributesPerLevel = {};\n\n'.format(11))

        for i in range(num_outline_styles):
            for j in range(num_levels):
                arr = []
                for _, cname in attrmap:
                    arr.append(cname.format(i, j))
                arr.append('NULL')
                t = 'static const sal_Unicode* outlineStyle{}Level{}[] = {{ {} }};\n'
                self.ofile.write(t.format(i, j, ', '.join(arr)))

        self.ofile.write('\n')
        for j in range(num_outline_styles):
            arr = []
            for i in range(5):
                arr.append('outlineStyle{}Level{}'.format(j, i))
            arr.append('NULL')
            t = 'static const sal_Unicode** outlineStyle{}[] = {{ {} }};\n'
            self.ofile.write(t.format(j, ', '.join(arr)))
        self.ofile.write('\nstatic const sal_Unicode*** LCOutlineNumberingLevelsArray[] = {\n')
        for i in range(num_outline_styles):
            self.ofile.write('\toutlineStyle{},\n'.format(i))
        self.ofile.write('\tNULL\n};\n\n')
        self.ofile.write(outline_templ.format(self.locale_name))

    def resolve_refs(self):
        num_children = len(list(self.root.getroot()))
        for i in range(num_children):
            c = self.root.getroot()[i]
            while 'ref' in c.attrib:
                refl = c.attrib['ref']
                ref_root = self.load_locale(refl)
                newn = ref_root.getroot().find(c.tag)
                #print(c.attrib['ref'])
                #print(c.tag)
                self.root.getroot()[i] = newn
                c = newn
        #self.root.write('abba.xml')
        #sys.exit('replacement: ' + c.tag)

    def convert(self):
        self.resolve_refs()
        self.ofile.write(header)
        self.convert_lcinfo()
        self.convert_lctype()
        self.convert_lcformat()
        self.convert_lccollation()
        self.convert_lcsearch()
        self.convert_lcindex()
        self.convert_calendar()
        self.convert_currency()
        self.convert_transliteration()
        self.convert_misc()
        self.convert_lcnumbering()
        self.convert_outlinenumbering()
        self.ofile.write(footer)

    def close(self):
        self.ofile.close()

if __name__ == '__main__':
    ifile = sys.argv[1]
    ofile = sys.argv[2]
    locale = os.path.splitext(os.path.split(ifile)[1])[0]
    x = XmlConverter(locale, ifile, ofile)
    x.convert()
    x.close()
