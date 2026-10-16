"""The reference states the mixed colour itself, which is a stronger leg than reading pixels.

26.2.4.2's own ODF export of the witness writes each conditional style's resolved background
as a literal `fo:background-color`. Those six values are `Fill::finalizeImport`'s answer for
the workbook's six `dxf` — three solids and three `lightUp` hatches over three different
backgrounds — so they can be compared against this tree's without a rasteriser, a tolerance
or an anchor anywhere in the comparison."""
import re
import zipfile

TWIN = ('/home/user/corpus-odf/ods/'
        '7f570683a4cd-072_Gantt_project_planner_dde00e33.ods')

# The style names LibreOffice gives the six, and what the .xlsx states for each.
EXPECTED = {
    '_25__20_complete': ('#735773', 'solid, bgColor theme 7'),
    '_25__20_complete_20__28_beyond_20_plan_29__20_legend':
        ('#e9ab51', 'solid, bgColor theme 9'),
    'Actual_20_legend': ('#b5a1b5', 'lightUp, fgColor theme 7 over bgColor theme 7 tint 0.6'),
    'Actual_20__28_beyond_20_plan_29__20_legend':
        ('#d6bca8', 'lightUp, fgColor theme 7 over bgColor theme 9 tint 0.6'),
}

STYLE = re.compile(
    r'<style:style[^>]*style:name="([^"]*)"[^>]*style:family="table-cell"[^>]*>'
    r'(?:(?!</style:style>).)*?fo:background-color="(#[0-9a-fA-F]{6})"', re.S)


def main():
    archive = zipfile.ZipFile(TWIN)
    found = {}
    for part in ('styles.xml', 'content.xml'):
        for name, colour in STYLE.findall(archive.read(part).decode('utf8')):
            found.setdefault(name, colour.lower())

    for name, (colour, what) in EXPECTED.items():
        got = found.get(name)
        print('%-8s %-9s %s   (%s)'
              % ('OK' if got == colour else 'DIFFERS', got, colour, what))


if __name__ == '__main__':
    main()
