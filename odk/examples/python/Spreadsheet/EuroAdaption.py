# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import traceback

import officehelper
from com.sun.star.lang import Locale
from com.sun.star.util import NumberFormat
from com.sun.star.sheet import CellFlags


def get_desktop():
    desktop = None
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        if srv_mgr is None:
            print("Can't create a desktop. No connection, no remote office servicemanager available!")
        else:
            desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to get desktop: {e}")
        traceback.print_exc()
        sys.exit(1)
    return desktop


def get_number_format_key(number_formats, format: str, language) -> int:
    """Get number format key, create one if not exist yet.

    :param number_formats: object managing number formats. It implements XNumberFormats interface.
    :param str format: the number format.
    :param language: language locale.
    :type language: com.sun.star.lang.Locale
    :return: key of the number format specific to the given language and format.
    """
    key = 0
    try:
        # Try to query the existing number format
        if (key := number_formats.queryKey(format, language, True)) == -1:
            # If not exist, create a new one
            if (key := number_formats.addNew(format, language)) == -1:
                key == 0
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to get key: {e}")
        traceback.print_exc()
    return key


def create_example_data(sheet, number_formats):
    """Create example data with specific number format.

    :param sheet: spreadsheet object. It implements XSpreadsheet interface.
    :param number_formats: object managing number formats. It implements XNumberFormats interface.
    """
    try:
        language = Locale()
        language.Country = "de"  # Germany -> DM
        language.Language = "de"  # German

        # Numberformat string from DM
        simple = "0 [$DM]"
        number_format_key = get_number_format_key(number_formats, simple, language)

        for counter in range(1, 10):
            cell = sheet[1 + counter, 2]
            cell.Value = float(counter * 2)
            cell.NumberFormat = number_format_key
            cell_range = sheet[counter + 1:counter + 2, 2:3]
            cell_range.NumberFormat = number_format_key
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to create example data: {e}")
        traceback.print_exc()


def convert(sheet, number_formats, old_symbol: str, new_symbol: str, factor: float):
    """Convert the currency

    :param sheet: spreadsheet object. It implements XSpreadsheet interface.
    :param number_formats: object managing number formats. It implements XNumberFormats interface.
    :param str old_symbol: the old number format symbol.
    :param str new_symbol: convert to this the new one.
    :param float factor: conversion factor.
    """
    try:
        language = Locale()
        language.Country = "de"  # Germany -> DM
        language.Language = "de"  # German

        simple = f"0 [${new_symbol}]"
        simple_key = get_number_format_key(number_formats, simple, language)

        for cell_range in sheet.CellFormatRanges:
            format = number_formats.getByKey(cell_range.NumberFormat)
            format_type = format.Type
            currency_symbol = format.CurrencySymbol
            if format_type & NumberFormat.CURRENCY and currency_symbol == old_symbol:
                thousands_sep = format.ThousandsSeparator
                negative_red = format.NegativeRed
                decimals = format.Decimals
                leading_zeros = format.LeadingZeros
                locale = format.Locale
                # create a new numberformat string
                new_number_format = number_formats.generateFormat(
                    simple_key, locale, thousands_sep, negative_red, decimals, leading_zeros
                )
                # get the NumberKey from the numberformat
                new_number_format_key = get_number_format_key(number_formats, new_number_format, locale)
                # set the new numberformat to the cellrange DM->EUR
                cell_range.NumberFormat = new_number_format_key

                # iterate over all cells from the cellrange with a
                # content and use the DM/EUR factor
                sheet_cell_ranges = cell_range.queryContentCells(CellFlags.VALUE)
                if sheet_cell_ranges.getCount() > 0:
                    for cell in sheet_cell_ranges.getCells():
                        cell.Value = cell.Value / factor
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to convert currency: {e}")
        traceback.print_exc()


def main():
    desktop = get_desktop()
    if desktop is None:
        return

    try:
        doc = desktop.loadComponentFromURL("private:factory/scalc", "_blank", 0, tuple())
        print("Create a new Spreadsheet")
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to load component from URL: {e}")
        traceback.print_exc()
        return

    # the Action Interface provides methods to hide actions,
    # like inserting data, on a sheet, that increase the performance
    doc.addActionLock()

    try:
        sheet = doc.Sheets[0]
    # removing bare except: and handling it similar to the try/except already
    # committed in './SCalc.py'
    except Exception as e:
        print(f"Failed to get sheet: {e}")
        traceback.print_exc()
        return

    # insert some example data in a sheet
    create_example_data(sheet, doc.NumberFormats)
    print("Insert example data and use the number format with the currency 'DM'")

    # Change the currency from the cells from DM to Euro
    convert(sheet, doc.NumberFormats, "DM", "EUR", 1.95583)
    print("Change the number format to EUR and divide the values with the factor 1.95583")

    # remove all locks, the user see all changes
    doc.removeActionLock()
    print("done")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
