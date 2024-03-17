# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import math
import random
import sys
import traceback

from com.sun.star.awt import Rectangle
from com.sun.star.container import NoSuchElementException


class CalcHelper:

    _data_sheet_name = "Data"
    _chart_sheet_name = "Chart"

    def __init__(self, doc):
        self.spread_sheet_doc = doc
        self._init_spread_sheet()

    def get_chart_sheet(self):
        sheets = self.spread_sheet_doc.Sheets
        try:
            sheet = sheets.getByName(self._chart_sheet_name)
        except NoSuchElementException as err:
            print(f"Couldn't find sheet with name {self._chart_sheet_name}: {err}")
            traceback.print_exc()
        except Exception as err:
            print(err)
            traceback.print_exc()
        return sheet

    def get_data_sheet(self):
        sheets = self.spread_sheet_doc.Sheets
        try:
            sheet = sheets.getByName(self._data_sheet_name)
        except NoSuchElementException as err:
            print(f"Couldn't find sheet with name {self._data_sheet_name}: {err}")
            traceback.print_exc()
        except Exception as err:
            print(err)
            traceback.print_exc()
        return sheet

    def insert_chart(self, chart_name, range_, upper_left, extent, chart_service_name):
        """Insert a chart using the given name as name of the OLE object and the range as corresponding
           range of data to be used for rendering.  The chart is placed in the sheet for charts at
           position upper_left extending as large as given in extent.

           The service name must be the name of a diagram service that can be instantiated via the
           factory of the chart document

        Args:
            chart_name (str): _description_
            range_ (com.sun.star.table.CellRangeAddress): _description_
            upper_left (Point): _description_
            extent (Size): _description_
            chart_service_name (str): _description_
        """
        result = None
        try:
            sheet = self.get_chart_sheet()
        except Exception as err:
            print(f"Sheet not found {err}")
            traceback.print_exc()
            return

        chart_collection = sheet.getCharts()

        if not chart_collection.hasByName(chart_name):
            rect = Rectangle(upper_left.X, upper_left.Y, extent.Width, extent.Height)
            addresses = []
            addresses.append(range_)

            # first bool: ColumnHeaders
            # second bool: RowHeaders
            chart_collection.addNewByName(chart_name, rect, addresses, True, False)

            try:
                table_chart = chart_collection.getByName(chart_name)

                # the table chart is an embedded object which contains the chart document
                result = table_chart.getEmbeddedObject()

                # create a diagram via the factory and set this as new diagram
                result.setDiagram(result.createInstance(chart_service_name))

            except NoSuchElementException as err:
                print(f"Couldn't find chart with name {chart_name}: {err}")
                traceback.print_exc()
                return

            except Exception as err:
                print(err)
                traceback.print_exc()
                return

        return result

    def insert_random_range(self, column_count, row_count):
        """Fill a rectangular range with random numbers.
           The first column has increasing values

        Args:
            column_count (int): _description_
            row_count (int): _description_

        Return:
            (com.sun.star.table.XCellRange)
        """
        cell_range = None
        try:
            # get the sheet to insert the chart
            sheet = self.get_data_sheet()
            cell_range = sheet[0:row_count, 0:column_count]

            base = 0.0
            float_range = 10.0

            for col in range(column_count):
                if col == 0:
                    sheet[0, col].Formula = "X"
                else:
                    sheet[0, col].Formula = f"Random {col}"

                for row in range(1, row_count):
                    if col == 0:
                        value = row + random.random()
                    else:
                        value = base + random.gauss(0.0, 1.0) * float_range

                    # put value into cell
                    sheet[row, col].Value = value

        except Exception as err:
            print(f"Sheet not found {err}")
            traceback.print_exc()

        return cell_range

    def insert_formula_range(self, column_count, row_count):
        try:
            # get the sheet to insert the chart
            sheet = self.get_data_sheet()
            cell_range = sheet[0 : row_count, 0 : column_count]
            factor = 2.0 * math.pi / (row_count - 1)

            factor_col = column_count + 2
            sheet[0, factor_col - 1].Value = 0.2
            sheet[1, factor_col - 1].String = "Change the factor above and\nwatch the changes in the chart"

            for col in range(column_count):
                for row in range(row_count):
                    if col == 0:
                        # x values: ascending numbers
                        value = row * factor
                        sheet[row, col].Value = value
                    else:
                        formula = "="
                        if col % 2 == 0:
                            formula += "SIN"
                        else:
                            formula += "COS"

                        formula += f"(INDIRECT(ADDRESS({row + 1};1)))+RAND()*INDIRECT(ADDRESS(1;{factor_col}))"
                        sheet[row, col].Formula = formula

        except Exception as err:
            print(f"Sheet not found {err}")
            traceback.print_exc()

        return cell_range

    def raise_chart_sheet(self):
        """Bring the sheet containing charts visually to the foreground"""
        self.spread_sheet_doc.getCurrentController().setActiveSheet(self.get_chart_sheet())

    def _init_spread_sheet(self):
        """create two sheets, one for data and one for charts in the document"""
        if self.spread_sheet_doc is not None:
            sheets = self.spread_sheet_doc.Sheets
            if sheets:
                for i in range(len(sheets) - 1, 0, -1):
                    sheets.removeByName(sheets.getByIndex(i).getName())
                try:
                    first_sheet = sheets[0]
                    first_sheet.setName(self._data_sheet_name)
                    sheets.insertNewByName(self._chart_sheet_name, 1)
                except Exception as e:
                    print(f"Couldn't initialize Spreadsheet Document: {e}", file=sys.stderr)
                    traceback.print_exc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
