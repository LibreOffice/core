#!/usr/bin/env python3
"""Separate `055_Project_timeline`'s date axis from its volatile formulas.

Three variants of one workbook, each differing from it by one edit:

  static2023   the `<f ca="1">DATE(YEAR(TODAY()),m,d)</f>` of C20:C32 removed, the cached
               2023 serials left standing
  static2026   the same, with each cached serial shifted by 46117-45021 so the frozen dates
               are the ones the reference itself recalculates to
  cache40000   the formulas LEFT IN PLACE and only C20's cached `<v>` changed, 45021 -> 40000

The first two answer "is the axis difference the scaling or the recalculation"; the third
answers "where does the reference's axis minimum come from", because a value that is only in
the cache cannot reach a recalculated cell.

    freeze-055.py <outdir>
    soffice --headless --convert-to pdf --outdir <outdir> <outdir>/055_*.xlsx
"""
import re
import sys
import zipfile
from pathlib import Path

SRC = ("/home/user/sample-files/sheets/chartset-008/xlsx/"
       "055_Project_timeline_with_milestones_Use_this_template_546cecc0.xlsx")
SHEET = "xl/worksheets/sheet11.xml"

out = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
out.mkdir(parents=True, exist_ok=True)


def strip_formulas(xml, shift=0):
    """Drop the <f> of every C20:C32 cell, optionally shifting its cached <v>."""
    def one(match):
        cell = re.sub(r"<f[^>]*>.*?</f>", "", match.group(0), flags=re.S)
        if shift:
            cell = re.sub(r"<v>(\d+)</v>",
                          lambda m: "<v>%d</v>" % (int(m.group(1)) + shift), cell)
        return cell

    return re.sub(r'<c r="C(2[0-9]|3[0-2])"[^>]*>.*?</c>', one, xml, flags=re.S)


def cache_only(xml):
    """Leave the formula alone and change C20's cached value alone."""
    return re.sub(r'(<c r="C20"[^>]*>.*?)<v>45021</v>', r"\g<1><v>40000</v>", xml, flags=re.S)


def write(name, transform):
    with zipfile.ZipFile(SRC) as zin:
        sheet = zin.read(SHEET).decode("utf-8")
        path = out / ("055_%s.xlsx" % name)
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zout:
            for item in zin.infolist():
                data = zin.read(item.filename)
                if item.filename == SHEET:
                    data = transform(sheet).encode("utf-8")
                zout.writestr(item.filename, data)
    print(path)


write("static2023", lambda x: strip_formulas(x, 0))
write("static2026", lambda x: strip_formulas(x, 46117 - 45021))
write("cache40000", cache_only)
