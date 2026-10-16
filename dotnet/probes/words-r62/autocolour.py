#!/usr/bin/env python3
"""What background does 26.2.4.2 resolve a `COL_AUTO` run against, inside a Word text box?

Round 59 removed an arm that passed *a floating frame's own fill* down as the background: it turned
383 glyphs white that the reference draws black, and the removal was right on that evidence. Round
61 then found one witness pointing the other way — `012_Project_Timeline_Template_Black_and_Brown_
Theme`'s title, a run stating no `w:color` in a `wps` text box with `<a:noFill/>`, which the
reference draws **white** and we draw black. One witness each way is a lead, not a rule.

This is the probe that separates them, and it is a **discriminating quadruple** rather than a pair:
both variables are moved independently, and each is moved in both directions.

    autocolour.py /abs/scratch/dir

The fixture is the corpus document itself with one substitution per arm, so nothing about it is
authored and nothing about the rest of the page changes. Its title box is anchored **inside a table
cell**, and that cell — like the eleven beside it — carries
`<w:shd w:val="clear" w:color="auto" w:fill="000000" w:themeFill="text1"/>`.

    arm  anchor cell   the box's own fill   title drawn
    ---  -----------   -----------------   -----------
    a    black         noFill              WHITE   (the document as found)
    p    WHITE         noFill              BLACK
    s    black         WHITE               BLACK
    t    WHITE         black               WHITE

So the shape's own fill wins when it has one, and when it has none the walk continues to the
**anchor's** background — which is `SwFrame::GetBackgroundBrush`, `sw/source/core/layout/
paintfrm.cxx`:8059, reached from `SwFntObj::SetDevFont`'s `bChgFntColor` branch
(`sw/source/core/txtnode/fntcache.cxx`:2369-2437) with `bConsiderTextBox=true`.

**What this does NOT settle, and it is the reason nothing was implemented on it.** Round 59's
counter-witness is a shape filled `#0070C0`, which is dark by `Color::IsDark`'s WCAG rule
(luminance 15.2 against a threshold of 87), and the rule measured here predicts **white** where the
reference draws **black**. Either those shapes are not Writer text boxes — a DrawingML shape's text
is drawn by editeng, where `SdrObject::getBackgroundFillSet` walks shape → page → master page and
never sees an anchor at all — or there is a further term. Re-measure that document before
implementing either direction.

Colour is read with `textcolour.py` beside this file, which reports the fill colour in force at
every text-showing operator.
"""
from __future__ import annotations

import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

SRC = ('/c/sandbox/workdir/sample-files/words/chartset-008/'
       '012_Project_Timeline_Template_Black_and_Brown_Theme_35c76550.docx')
SRC_ALT = ('/c/sandbox/workdir/sample-files/words/chartset-008/docx/'
           '012_Project_Timeline_Template_Black_and_Brown_Theme_35c76550.docx')

BLACK_CELL = '<w:shd w:val="clear" w:color="auto" w:fill="000000" w:themeFill="text1"/>'
WHITE_CELL = '<w:shd w:val="clear" w:color="auto" w:fill="FFFFFF"/>'

# The title's own shape. `name="Text Box 13"` holds "Project Timeline Template", an 88-half-point
# bold run that states no `w:color` at all.
TITLE = 'name="Text Box 13"'


def white_cells(xml: str) -> str:
    assert xml.count(BLACK_CELL) == 12, xml.count(BLACK_CELL)
    return xml.replace(BLACK_CELL, WHITE_CELL)


def fill_title(xml: str, colour: str) -> str:
    i = xml.index(TITLE)
    j = xml.index('<a:noFill/>', i)
    return (xml[:j]
            + f'<a:solidFill><a:srgbClr val="{colour}"/></a:solidFill>'
            + xml[j + len('<a:noFill/>'):])


ARMS = {
    'a-asfound': lambda s: s,
    'p-whitecells': white_cells,
    's-boxwhite': lambda s: fill_title(s, 'FFFFFF'),
    't-whitecells-blackbox': lambda s: fill_title(white_cells(s), '000000'),
}


def build(src: Path, out: Path, fn) -> None:
    with zipfile.ZipFile(src) as zin, zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as zo:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename == 'word/document.xml':
                data = fn(data.decode('utf-8')).encode('utf-8')
            zo.writestr(item, data)


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    work = Path(sys.argv[1]).resolve()
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    src = Path(SRC_ALT if Path(SRC_ALT).exists() else SRC)
    if not src.exists():
        print(f'corpus document not found: {src}')
        return 2

    here = Path(__file__).resolve().parent
    print(f'{"arm":<24} {"title y=561.70":>16}   shows white / black')
    for name, fn in ARMS.items():
        docx = work / f'{name}.docx'
        build(src, docx, fn)
        subprocess.run(
            ['soffice', '--headless', f'-env:UserInstallation=file://{work / "prof"}',
             '--convert-to', 'pdf', '--outdir', str(work), str(docx)],
            capture_output=True, timeout=300)
        pdf = work / f'{name}.pdf'
        if not pdf.exists():
            print(f'{name:<24} {"CONVERT FAILED":>16}')
            continue
        out = subprocess.run(
            [sys.executable, str(here / 'textcolour.py'), str(pdf), '1'],
            capture_output=True, text=True).stdout
        title = '?'
        white = black = 0
        for line in out.splitlines():
            parts = line.split()
            if len(parts) >= 2 and parts[0] == '#FFFFFF':
                white = int(parts[1])
            elif len(parts) >= 2 and parts[0] == '#000000':
                black = int(parts[1])
            if len(parts) >= 3 and parts[0] == 'y' and parts[1] == '561.70' and title == '?':
                title = parts[2]
        print(f'{name:<24} {title:>16}   {white} / {black}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
