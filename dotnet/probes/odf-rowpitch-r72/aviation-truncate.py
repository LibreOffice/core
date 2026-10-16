#!/usr/bin/env python3
"""Bisect Aviation_Abbreviations.ods on the row that changes every other row's height.

Truncates Sheet1 to N rows, drops the other sheets, and asks 26.2.4.2 for its own row heights
through a flat-ODF export — which is the only instrument here that reads the recomputed grid
rather than a rendering of it.

    aviation-truncate.py 600 680 685 700

Measured 2026-09-07 against /opt/libreoffice26.2 (26.2.4.2) on the converted ODF corpus:

    600 rows -> the first row is 256 twips
    680 rows -> 256
    685 rows -> 276

Row 681 (index 680) is `<table:table-cell table:style-name="ce57"/>` — a cell that states a
format and holds nothing, in column C. It allocates the column, and an allocated column with no
pattern at a row contributes the sheet's default pattern's arithmetic height there: the document
default is Calibri 11 pt, trunc(220 x 1.18) + 40 - 23 = 276, where every cell in columns A and B
is Arial 9 pt and asks for less than the 256-twip sheet minimum.
"""
import os, re, subprocess, sys, tempfile, zipfile

SRC = "/home/user/corpus-odf/sheets/done-009/ods/Aviation_Abbreviations.ods"
SOFFICE = "/opt/libreoffice26.2/program/soffice"


def truncated(path, rows):
    z = zipfile.ZipFile(SRC)
    c = z.read("content.xml").decode("utf-8")
    i = c.find("<table:table ")
    j = c.find("</office:spreadsheet>")
    body = c[i:j]
    k = body.find("</table:table>")
    starts = [m.start() for m in re.finditer(r"<table:table-row\b", body[:k])]
    out = c[:i] + body[: starts[rows]] + "</table:table>" + c[j:]
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zo:
        for n in z.namelist():
            zo.writestr(n, out.encode("utf-8") if n == "content.xml" else z.read(n))


def first_row_height(path, work, profile):
    subprocess.run(
        [SOFFICE, f"-env:UserInstallation=file://{profile}", "--headless", "--norestore",
         "--convert-to", "fods", "--outdir", work, path],
        capture_output=True, timeout=600)
    name = os.path.splitext(os.path.basename(path))[0] + ".fods"
    c = open(os.path.join(work, name), encoding="utf-8").read()
    heights = {}
    for m in re.finditer(
            r'<style:style style:name="(ro\d+)"[^>]*style:family="table-row"[^>]*>\s*'
            r'<style:table-row-properties([^>]*)/>', c):
        v = re.search(r'style:row-height="([^"]+)"', m.group(2)).group(1)
        heights[m.group(1)] = round(
            float(v[:-2]) * 1440 if v.endswith("in") else float(v[:-2]) / 25.4 * 1440, 1)
    at = c.find("<table:table ")
    return heights[re.search(r'<table:table-row table:style-name="(ro\d+)"', c[at:]).group(1)]


def main():
    with tempfile.TemporaryDirectory() as work:
        profile = os.path.join(work, "prof")
        os.makedirs(profile, exist_ok=True)
        for rows in (int(a) for a in sys.argv[1:] or ["600", "680", "685", "700"]):
            path = os.path.join(work, f"av-{rows}.ods")
            truncated(path, rows)
            print(rows, first_row_height(path, work, profile))


main()
