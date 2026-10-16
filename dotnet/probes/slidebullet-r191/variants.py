#!/usr/bin/env python3
"""What decides where the first line of a bulleted slide paragraph starts.

`Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx` page 13 heads that deck's
`diff%` ranking at 15.93 and holds no chart at all. Its master's body list states
lvl2 `marL="742950" indent="-285750"` — a 58.5 pt margin with the bullet 22.5 pt left of it — and
26.2.4.2 starts every second-level paragraph's FIRST line 8.63 pt to the right of its own
continuation lines, where this tree starts both at `marL`.

Each variant rewrites ONE attribute of the master (or of the slide's autofit) and nothing else.
`firstline.py` reads the level's bullet x and its two text x off the rendered page.
"""
import pathlib
import zipfile

SRC = pathlib.Path(
    '/home/user/sample-files/slides/done-010/pptx/'
    'Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx')
OUT = pathlib.Path(__file__).parent / 'var'
MASTER = 'ppt/slideMasters/slideMaster1.xml'
SLIDE = 'ppt/slides/slide13.xml'

LEVEL = 'marL="742950" indent="-285750"'
BULLET = '<a:buChar char="–"/>'
SIZE = '<a:defRPr sz="2800"'
AUTOFIT = '<a:normAutofit fontScale="25000" lnSpcReduction="20000"/>'

# (part, the text to replace, the replacement) — the needle is found AFTER the level's own
# attributes so that only the second-level entry of the body list is touched.
VARIANTS = {
    # the hanging indent, which moves the bullet and nothing else
    'ind0':          (MASTER, LEVEL, 'marL="742950" indent="0"'),
    'ind-9pt':       (MASTER, LEVEL, 'marL="742950" indent="-114300"'),
    'ind-36pt':      (MASTER, LEVEL, 'marL="742950" indent="-457200"'),
    # the margin, which moves the bullet and the continuation lines together
    'marL-78.7pt':   (MASTER, LEVEL, 'marL="1000000" indent="-285750"'),
    # the bullet character, which is what the offset turns out to be measured from
    'buW':           (MASTER, BULLET, '<a:buChar char="W"/>'),
    'buI':           (MASTER, BULLET, '<a:buChar char="i"/>'),
    # the bullet's own stated size, which turns out to decide nothing
    'buSz14pt':      (MASTER, SIZE, '<a:defRPr sz="1400"'),
    'buSz56pt':      (MASTER, SIZE, '<a:defRPr sz="5600"'),
    # the autofit, which LibreOffice recomputes rather than honours
    'fontScale50':   (SLIDE, AUTOFIT, '<a:normAutofit fontScale="50000" lnSpcReduction="20000"/>'),
    'noFontScale':   (SLIDE, AUTOFIT, '<a:normAutofit/>'),
}


def write(name: str, part: str, needle: str, replacement: str) -> pathlib.Path:
    OUT.mkdir(exist_ok=True)
    target = OUT / f'v-{name}.pptx'

    with zipfile.ZipFile(SRC) as src, zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)

            if item.filename == part:
                text = data.decode('utf-8')
                # anchor at the level's own attributes so a shared spelling elsewhere is left alone
                at = 0 if needle == LEVEL else text.index(LEVEL)
                found = text.index(needle, at)
                data = (text[:found] + replacement
                        + text[found + len(needle):]).encode('utf-8')

            dst.writestr(item, data)

    return target


for name, (part, needle, replacement) in VARIANTS.items():
    print(name, write(name, part, needle, replacement))
