#!/usr/bin/env python3
"""How many corpus DOCX hold a REF field whose bookmark text differs from the cached result?

A REF field without \\p, \\r, \\n or \\w becomes a SwGetRefField with
ReferenceFieldSource::BOOKMARK, and Writer recomputes what it draws from the bookmark
rather than keeping w:fldSimple's or the fldChar run's cached result.  This counts the
documents where the two disagree, which are the documents a reader that keeps the cache
draws differently from 26.2.4.2.
"""
import re, sys, zipfile, pathlib

TOKEN = re.compile(
    r'<w:bookmarkStart [^>]*w:id="(?P<bs>\d+)"[^>]*w:name="(?P<name>[^"]*)"[^>]*/>'
    r'|<w:bookmarkEnd [^>]*w:id="(?P<be>\d+)"[^>]*/>'
    r'|<w:t(?: [^>]*)?>(?P<t>.*?)</w:t>'
    r'|<w:instrText(?: [^>]*)?>(?P<instr>.*?)</w:instrText>'
    r'|<w:fldChar [^>]*w:fldCharType="(?P<fld>begin|separate|end)"[^>]*/>'
    r'|<w:p(?: [^>]*)?/?>(?P<p>)'
    r'|<w:br(?: [^>]*)?/>(?P<br>)'
    r'|<w:tab(?: [^>]*)?/>(?P<tab>)',
    re.S)

UNESC = lambda s: (s.replace('&lt;', '<').replace('&gt;', '>').replace('&quot;', '"')
                    .replace('&apos;', "'").replace('&amp;', '&'))


def scan(xml):
    """Linear text, bookmark spans by name, and the REF fields with their cached results."""
    text = []          # list of str pieces
    pos = 0            # characters emitted so far
    open_marks = {}    # id -> (name, start)
    spans = {}         # name -> (start, end)
    stack = []         # field nesting: [instr, result, seen_separate]
    fields = []        # (instruction, cached result)

    for m in TOKEN.finditer(xml):
        if m.group('bs') is not None:
            open_marks[m.group('bs')] = (UNESC(m.group('name')), pos)
        elif m.group('be') is not None:
            got = open_marks.pop(m.group('be'), None)
            if got:
                spans.setdefault(got[0], (got[1], pos))
        elif m.group('t') is not None:
            s = UNESC(m.group('t'))
            text.append(s); pos += len(s)
            if stack and stack[-1][2]:
                stack[-1][1].append(s)
        elif m.group('instr') is not None:
            if stack:
                stack[-1][0].append(UNESC(m.group('instr')))
        elif m.group('fld') == 'begin':
            stack.append([[], [], False])
        elif m.group('fld') == 'separate':
            if stack:
                stack[-1][2] = True
        elif m.group('fld') == 'end':
            if stack:
                instr, result, _ = stack.pop()
                fields.append((''.join(instr), ''.join(result)))
        elif m.group('p') is not None:
            text.append('\n'); pos += 1
        elif m.group('br') is not None or m.group('tab') is not None:
            text.append(' '); pos += 1

    return ''.join(text), spans, fields


def reference_bookmark(instr):
    parts = instr.strip()
    if not parts:
        return None
    head = parts.split(None, 1)
    if head[0].upper() != 'REF' or len(head) < 2:
        return None
    rest = head[1].strip()
    for sw in ('\\p', '\\r', '\\n', '\\w'):
        if re.search(re.escape(sw) + r'(?![a-zA-Z])', parts):
            return None
    if rest.startswith('"'):
        close = rest.find('"', 1)
        return rest[1:close] if close > 0 else None
    name = rest.split(None, 1)[0]
    return None if name.startswith('\\') else name


def main(root):
    docs = [p for p in pathlib.Path(root).rglob('*') if p.suffix.lower() in ('.docx', '.docm')]
    total = with_ref = differing = 0
    rows = []
    for p in sorted(docs):
        total += 1
        try:
            with zipfile.ZipFile(p) as z:
                xml = z.read('word/document.xml').decode('utf-8', 'replace')
        except Exception as exc:
            print(f'SKIP\t{p.name}\t{exc}', file=sys.stderr); continue
        if ' REF ' not in xml:
            continue
        text, spans, fields = scan(xml)
        n = diff = 0
        for instr, cached in fields:
            name = reference_bookmark(instr)
            if name is None:
                continue
            n += 1
            span = spans.get(name)
            if span is None:
                continue
            want = text[span[0]:span[1]].replace('\n', ' ').strip()
            if want != cached.replace('\n', ' ').strip():
                diff += 1
        if n:
            with_ref += 1
            if diff:
                differing += 1
            rows.append((diff, n, p.name))
    rows.sort(reverse=True)
    for diff, n, name in rows:
        print(f'{diff}\t{n}\t{name}')
    print(f'# {total} documents, {with_ref} with a resolvable REF, {differing} where a '
          f'bookmark expansion differs from the cache')


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files')
