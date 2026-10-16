#!/usr/bin/env python3
"""Which corpus DOCX hold a TOC `\\t` switch, and how much direct paragraph formatting it voids.

26.2.4.2 discards a paragraph's direct paragraph properties when its style is named by a TOC
field's `\\t` template switch.  This counts, per document, the styles so named and the body
paragraphs in them that state a `w:pPr` beyond their `w:pStyle`.
"""
import re, sys, zipfile, pathlib

INSTR = re.compile(r'<w:instrText[^>]*>(.*?)</w:instrText>', re.S)
SIMPLE = re.compile(r'<w:fldSimple[^>]*w:instr="([^"]*)"')
PARA = re.compile(r'<w:p(?: [^>]*)?>(.*?)</w:p>', re.S)
PPR = re.compile(r'<w:pPr>(.*?)</w:pPr>', re.S)
PSTYLE = re.compile(r'<w:pStyle w:val="([^"]*)"/>')
# The properties the reference keeps: a break is applied by another path.
KEPT = ('pageBreakBefore', 'rPr', 'sectPr', 'pStyle')


def templates(xml):
    """The style names a TOC field's `\\t` switch registers, in the whole part."""
    names = set()
    text = ''.join(INSTR.findall(xml)) + ' ' + ' '.join(SIMPLE.findall(xml))
    for m in re.finditer(r'\\t\s+"([^"]*)"', text):
        parts = [p.strip() for p in re.split('[,;]', m.group(1))]
        for i in range(0, len(parts) - 1, 2):
            if parts[i]:
                names.add(parts[i])
    return names


def main(root):
    total = withtoc = withreach = 0
    rows = []
    for p in sorted(pathlib.Path(root).rglob('*')):
        if p.suffix.lower() not in ('.docx', '.docm'):
            continue
        total += 1
        try:
            with zipfile.ZipFile(p) as z:
                names = set(z.namelist())
                xml = z.read('word/document.xml').decode('utf-8', 'replace')
                styles = (z.read('word/styles.xml').decode('utf-8', 'replace')
                          if 'word/styles.xml' in names else '')
        except Exception as exc:
            print(f'SKIP\t{p.name}\t{exc}', file=sys.stderr)
            continue

        wanted = templates(xml)
        if not wanted:
            continue
        withtoc += 1

        # A `\t` names a style by its w:name; a paragraph names it by w:styleId.
        # A `\t` names a style by its w:name and a paragraph by its w:styleId, and the match is
        # case-insensitive: Word writes `Heading 3` in the switch where the style sheet says
        # `heading 3`, and 26.2.4.2 pairs them.
        # Measured: only a BUILT-IN `heading 1`..`heading 9` is affected. A custom style carrying
        # the same name, a custom style with an outline level, and the built-in `Title` are all
        # named in a `\t` without losing anything. See results.md.
        builtin = {'heading %d' % n for n in range(1, 10)}
        folded = {name.casefold() for name in wanted}
        ids = {}
        for m in re.finditer(r'<w:style [^>]*w:styleId="([^"]*)"[^>]*>(.*?)</w:style>', styles,
                             re.S):
            nm = re.search(r'<w:name w:val="([^"]*)"/>', m.group(2))
            if nm:
                ids[m.group(1)] = nm.group(1)
        custom = {m.group(1) for m in
                  re.finditer(r'<w:style [^>]*w:customStyle="1"[^>]*w:styleId="([^"]*)"', styles)}
        targets = {sid for sid, name in ids.items()
                   if name.casefold() in folded and name.casefold() in builtin
                   and sid not in custom}

        voided = 0
        for m in PARA.finditer(xml):
            body = m.group(1)
            ppr = PPR.search(body)
            if not ppr:
                continue
            style = PSTYLE.search(ppr.group(1))
            if not style or style.group(1) not in targets:
                continue
            rest = re.sub(r'<w:pStyle[^>]*/>', '', ppr.group(1))
            rest = re.sub(r'<w:rPr>.*?</w:rPr>', '', rest, flags=re.S)
            rest = re.sub(r'<w:sectPr.*?</w:sectPr>', '', rest, flags=re.S)
            rest = re.sub(r'<w:pageBreakBefore[^>]*/?>', '', rest)
            if re.search(r'<w:[a-zA-Z]', rest):
                voided += 1

        if voided:
            withreach += 1
        rows.append((voided, len(targets), sorted(wanted)[:3], p.name))

    rows.sort(reverse=True)
    for voided, ntargets, wanted, name in rows:
        print(f'{voided}\t{ntargets}\t{",".join(wanted)}\t{name}')
    print(f'# {total} documents, {withtoc} with a TOC \\t switch, {withreach} where it voids a '
          f'paragraph\'s direct formatting')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
