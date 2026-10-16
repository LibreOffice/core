#!/usr/bin/env python3
"""Authored flat-ODF probes for a table cell's writing mode.

One table, one row, five cells of a stated width, each carrying the same word.  The cells
differ in exactly one attribute — the spelling and the value of ``writing-mode`` on their
``style:table-cell-properties`` — so what the reference does with each is read off one
rendering.  A second family varies the text length in a turned cell, which is how the row's
height is shown to follow the text's *length* rather than its line count.
"""
import pathlib

OUT = pathlib.Path(__file__).parent / 'probes'
OUT.mkdir(exist_ok=True)

HEAD = '''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
 xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 xmlns:loext="urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
 <office:automatic-styles>
  <style:page-layout style:name="pm1">
   <style:page-layout-properties fo:page-width="8.5in" fo:page-height="11in"
    fo:margin-top="1in" fo:margin-bottom="1in" fo:margin-left="1in" fo:margin-right="1in"/>
  </style:page-layout>
  <style:style style:name="Tbl" style:family="table">
   <style:table-properties style:width="6in" table:align="left"/>
  </style:style>
  <style:style style:name="Col" style:family="table-column">
   <style:table-column-properties style:column-width="1in"/>
  </style:style>
  <style:style style:name="P1" style:family="paragraph" style:parent-style-name="Standard">
   <style:text-properties style:font-name="Liberation Serif" fo:font-size="12pt"/>
  </style:style>
%(cellstyles)s </office:automatic-styles>
 <office:master-styles>
  <style:master-page style:name="Standard" style:page-layout-name="pm1"/>
 </office:master-styles>
 <office:body><office:text>
'''

TAIL = ''' </office:text></office:body>
</office:document>
'''


def cell_style(name, attrs):
    return (f'  <style:style style:name="{name}" style:family="table-cell">\n'
            f'   <style:table-cell-properties fo:padding="0.02in" fo:border="0.5pt solid #000000"'
            f'{attrs}/>\n  </style:style>\n')


def table(name, cells, text='Strategy'):
    """cells: list of (style name, attribute string)."""
    styles = ''.join(cell_style(s, a) for s, a in cells)
    body = [f'  <table:table table:name="{name}" table:style-name="Tbl">',
            f'   <table:table-column table:style-name="Col" '
            f'table:number-columns-repeated="{len(cells)}"/>',
            '   <table:table-row>']
    for s, _ in cells:
        body.append(f'    <table:table-cell table:style-name="{s}" office:value-type="string">'
                    f'<text:p text:style-name="P1">{text}</text:p></table:table-cell>')
    body += ['   </table:table-row>', '  </table:table>', '  <text:p text:style-name="P1">after</text:p>']
    return (HEAD % {'cellstyles': styles}) + '\n'.join(body) + '\n' + TAIL


# a — one cell per spelling/value, all in one table, so one rendering answers all of them.
SPELLINGS = [
    ('c0', ''),                                          # control: nothing stated
    ('c1', ' loext:writing-mode="bt-lr"'),               # what LibreOffice's exporter writes
    ('c2', ' style:writing-mode="bt-lr"'),               # the same value, ODF-namespace spelling
    ('c3', ' style:writing-mode="tb-rl"'),               # standard vertical
    ('c4', ' loext:writing-mode="tb-rl90"'),             # OOXML vert="vert"
    ('c5', ' style:writing-mode="tb-lr"'),               # standard vertical, left to right
]
(OUT / 'a-spellings.fodt').write_text(table('a', SPELLINGS), encoding='utf-8')

# b — one turned cell holding a longer and longer word, to read the row height off the text.
for n, word in enumerate(['Aa', 'AaAa', 'AaAaAa', 'AaAaAaAaAa']):
    (OUT / f'b{n}-len{len(word)}.fodt').write_text(
        table('b', [('c0', ''), ('c1', ' loext:writing-mode="bt-lr"')], text=word),
        encoding='utf-8')

# c — the control for b: the same words in an untuned cell.
print('\n'.join(sorted(p.name for p in OUT.glob('*.fodt'))))

# d — a row whose ONLY cell is turned: what height does a turned cell ask for?
(OUT / 'd-alone.fodt').write_text(
    table('d', [('c1', ' loext:writing-mode="bt-lr"')], text='Strategy'),
    encoding='utf-8')

# e — a turned cell beside one whose text forces the row tall, so the turned text has room.
TALL = ('  <style:style style:name="cT" style:family="table-cell">\n'
        '   <style:table-cell-properties fo:padding="0.02in" fo:border="0.5pt solid #000000"/>\n'
        '  </style:style>\n')
tall_cells = [('cT', ''), ('c1', ' loext:writing-mode="bt-lr"')]
doc = table('e', tall_cells, text='Strategy')
# make the first cell four lines tall by giving it a long paragraph
doc = doc.replace(
    '<table:table-cell table:style-name="cT" office:value-type="string">'
    '<text:p text:style-name="P1">Strategy</text:p></table:table-cell>',
    '<table:table-cell table:style-name="cT" office:value-type="string">'
    '<text:p text:style-name="P1">one two three four five six seven eight nine ten</text:p>'
    '</table:table-cell>')
(OUT / 'e-tallrow.fodt').write_text(doc, encoding='utf-8')

# f — a stated row height, which is the other way a turned cell gets room.
for h in ('0.5in', '1in', '2in'):
    doc = table('f', [('c1', ' loext:writing-mode="bt-lr"')], text='Strategy')
    doc = doc.replace(
        '  <style:style style:name="Col" style:family="table-column">',
        f'  <style:style style:name="Row" style:family="table-row">\n'
        f'   <style:table-row-properties style:row-height="{h}"/>\n'
        f'  </style:style>\n'
        f'  <style:style style:name="Col" style:family="table-column">')
    doc = doc.replace('<table:table-row>', f'<table:table-row table:style-name="Row">')
    (OUT / f'f-rowheight-{h}.fodt').write_text(doc, encoding='utf-8')
