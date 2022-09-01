import pathlib
import re
import unittest

from os import getenv, path

from org.libreoffice.unotest import pyuno, mkPropertyValue, makeCopyFromTDOC


class InsertRemoveCells(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.xContext = pyuno.getComponentContext()
        pyuno.private_initTestEnvironment()

    def test_fdo74824_load(self):
        ctxt = self.xContext
        assert(ctxt)

        smgr = ctxt.ServiceManager
        desktop = smgr.createInstanceWithContext('com.sun.star.frame.Desktop', ctxt)
        load_props = tuple(mkPropertyValue(k, v) for (k, v) in (
          ('Hidden', True),
          ('ReadOnly', False)
        ))
        tdoc_path = makeCopyFromTDOC('fdo74824.ods')
        url = pathlib.Path(tdoc_path).as_uri()
        doc = desktop.loadComponentFromURL(url, "_blank", 0, load_props)

        sheet = doc.Sheets.Sheet1
        area = sheet.getCellRangeByName('A2:B4')
        addr = area.getRangeAddress()

        # 2 = intended to shift cells right, but I don't know where to find
        # the ENUM to put in its place.  Corrections welcome.
        sheet.insertCells(addr, 2)

        # basically, the insertCells call is the test: it should not crash
        # LibreOffice.  However, for completeness, we should test the cell
        # contents as well.

        empty_cells = (
          (0, 0), (0, 1), (0, 2), (0, 3), (1, 0), (1, 1), (1, 2), (1, 3),
          (3, 1), (4, 0), (4, 2), (5, 0), (5, 2), (5, 3),
        )
        formula_cells = (
          (2, 0, '=(1+GDR)^-D1', '1.000', 1.0),
          (4, 1, '=(1+GDR)^-F2', '0.125', 0.125),
          (4, 3, '=SUM(C1:C2)', '1.000', 1.0),
        )
        value_cells = (
          (2, 2, '2010', 2010.0),
          (2, 3, '7', 7.0),
          (3, 0, '0', 0),
          (3, 2, '2012', 2012.0),
          (3, 3, '6', 6.0),
          (5, 1, '1', 1.0),
        )
        for col, row in empty_cells:
            cell = sheet[row,col]
            self.assertEqual('EMPTY', cell.Type.value)

        for col, row, f, s, val in formula_cells:
            cell = sheet[row,col]
            self.assertEqual('FORMULA', cell.Type.value)
            self.assertEqual(f, cell.getFormula())
            self.assertEqual(s, cell.String)
            self.assertEqual(val, cell.Value)

        for col, row, s, val in value_cells:
            cell = sheet[row,col]
            self.assertEqual(s, cell.String)
            self.assertEqual(val, cell.Value)

        doc.close(True)


if __name__ == '__main__':
    unittest.main()
