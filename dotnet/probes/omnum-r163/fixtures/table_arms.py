import make, zipfile

W = make.W

def tbl(rows, vmerge_col=3, span_rows=None, cols=(2972,1985,2557,1848)):
    """rows: list of list of list-of-paragraph-texts, one per column."""
    grid = ''.join(f'<w:gridCol w:w="{c}"/>' for c in cols)
    out = [f'<w:tbl><w:tblPr><w:tblW w:w="0" w:type="auto"/>'
           f'<w:tblBorders>'
           f'<w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'<w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'<w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'<w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'<w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
           f'</w:tblBorders></w:tblPr><w:tblGrid>{grid}</w:tblGrid>']
    for ri, row in enumerate(rows):
        out.append('<w:tr>')
        for ci, paras in enumerate(row):
            vm = ''
            if ci == vmerge_col and span_rows:
                if ri == span_rows[0]: vm = '<w:vMerge w:val="restart"/>'
                elif ri in span_rows: vm = '<w:vMerge/>'
            body = ''.join(
                f'<w:p><w:r><w:t xml:space="preserve">{t}</w:t></w:r></w:p>' if t
                else '<w:p/>' for t in paras) or '<w:p/>'
            out.append(f'<w:tc><w:tcPr><w:tcW w:w="{cols[ci]}" w:type="dxa"/>{vm}</w:tcPr>{body}</w:tc>')
        out.append('</w:tr>')
    out.append('</w:tbl>')
    return ''.join(out)

def doc(filler, table):
    fill = ''.join(f'<w:p><w:r><w:t>Filler line {i+1}</w:t></w:r></w:p>' for i in range(filler))
    sect = ('<w:sectPr><w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="720" w:right="567" w:bottom="720" w:left="1134" '
            'w:header="709" w:footer="709" w:gutter="0"/></w:sectPr>')
    return (f'<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:document {W}><w:body>'
            + fill + table + '<w:p/>' + sect + '</w:body></w:document>')

STYLES = make.styles([])
NUM = make.numbering([(360,360,'Heading1','%1.')])

# Row 1 is tall and splittable; column 4 is vertically merged across all three rows.
ROWS = [
    [['Row one col one'], [''], ['3 Years'], ['[responsible person]']],
    [['Training and qualifications for the specific operations:', 'EUR RVSM', 'NAT-MNPS',
      'LVTO', 'STEEP APPR', 'RNP', ''], [''], ['5 Years'], ['']],
    [['Row three col one'], [''], ['3 Years (ICAO 9284)'], ['']],
]
for n in range(44, 56):
    make.build(f'vm-filler{n}.docx', NUM, STYLES, doc(n, tbl(ROWS, 3, (0,1,2))))

# Control: identical table with NO vertical merge at all.
make.build('vm-none-filler50.docx', NUM, STYLES, doc(50, tbl(ROWS, 3, None)))
