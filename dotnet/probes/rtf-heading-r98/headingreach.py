#!/usr/bin/env python3
r"""Which documents apply a heading-parented name, and which property each would newly take.

`rtf-style-r97/whichproperty.py` for the heading family. Same shape, same warning: this is a
reimplementation of `ContributionOf`'s own-beats-inherited rule and is therefore evidence about
the *model*, not about the reference. It exists to be written down **before** `reach-sweep.py`
runs, so that "12 documents apply an affected name" can be turned into a prediction of how many
renderings move -- and so that the prediction can be wrong in public.

A heading-parented pair can move on exactly the properties `COLL_HEADLINE_BASE` leaves unstated.
Read straight off the reference's own resolved `Heading` style (probe `p_h1_chars.fodt`), it
states a font, `fo:font-size`, `fo:margin-top`, `fo:margin-bottom` and `fo:keep-with-next` and
nothing else -- so size, space above, space below and keep are shadowed and cannot change, and
bold, italic, underline, strike, capitals, small capitals, colour, alignment and language pass
through from the document's own `Normal`. The face is in neither class: no style's face reaches
a run (`RtfStyles.ContributionOf`).

    heading name applied, no resolvable \sbasedon, Normal states P, the entry does not  ->  moves

  headingreach.py <corpus root>
"""
import pathlib
import re
import sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')
USE = re.compile(r'\\s(\d+)(?![0-9])')

# The members `ContributionOf` can newly answer for a heading-parented style: everything
# `RtfStyleFormatting` carries except the four `HeadingPool` states and the face it never emits.
PROPS = {
    'bold': re.compile(r'\\b\d?(?![a-zA-Z0-9])'),
    'italic': re.compile(r'\\i\d?(?![a-zA-Z0-9])'),
    'underline': re.compile(r'\\ul(?![a-zA-Z])|\\ulnone'),
    'strike': re.compile(r'\\strike\d?(?![a-zA-Z])'),
    'caps': re.compile(r'\\caps\d?(?![a-zA-Z])'),
    'smallcaps': re.compile(r'\\scaps\d?(?![a-zA-Z])'),
    'colour': re.compile(r'\\cf\d'),
    'align': re.compile(r'\\q[lcrj](?![a-zA-Z])'),
    'lang': re.compile(r'\\lang\d'),
}
# What `HeadingPool` shadows, printed beside the gain so a reader can see it was considered.
SHADOWED = {
    'size': re.compile(r'\\fs\d'),
    'spacebefore': re.compile(r'\\sb\d'),
    'keep': re.compile(r'\\keepn(?![a-zA-Z])'),
}
HEADINGS = ({'heading %d' % n for n in range(1, 10)}
            | {'Heading %d' % n for n in range(1, 10)}
            | {'Title', 'Subtitle'})


def entries(text):
    start = text.find('{\\stylesheet')
    if start < 0:
        return [], -1
    depth, opened, out = 0, None, []
    for index in range(start, len(text)):
        char = text[index]
        if char == '{':
            depth += 1
            if depth == 2:
                opened = index
        elif char == '}':
            if depth == 2 and opened is not None:
                out.append(text[opened:index + 1])
                opened = None
            depth -= 1
            if depth == 0:
                return out, index
    return out, len(text)


def name_of(entry):
    body = re.sub(r'\{[^{}]*\}', '', entry[1:-1])
    last = None
    for match in CW.finditer(body):
        last = match
    return (body[last.end():] if last else body).strip().rstrip(';').strip()


def stated(entry, table=PROPS):
    return {key for key, pattern in table.items() if pattern.search(entry)}


docs = movers = pairs = movingpairs = 0
for path in sorted(pathlib.Path(sys.argv[1]).rglob('*.rtf')):
    text = path.read_bytes().decode('cp1252', 'replace')
    sheet, end = entries(text)
    body = text if end < 0 else text[:text.find('{\\stylesheet')] + text[end + 1:]
    used = {int(m.group(1)) for m in USE.finditer(body)}
    seen, normal, rows = set(), set(), []
    for entry in sheet:
        match = re.match(r'\{\\(\*\\)?(s|cs|ts)(\d+)?\b', entry)
        kind = match.group(2) if match else 's'
        sid = int(match.group(3)) if match and match.group(3) else 0
        based = re.search(r'\\sbasedon(\d+)\b', entry)
        resolves = bool(based) and int(based.group(1)) != sid and int(based.group(1)) in seen
        seen.add(sid)
        if kind != 's':
            continue
        if sid == 0:
            normal = stated(entry)
        if resolves or sid not in used or name_of(entry) not in HEADINGS:
            continue
        rows.append((sid, name_of(entry), stated(entry), stated(entry, SHADOWED)))
    if not rows:
        continue
    docs += 1
    moved = False
    for sid, name, own, shadow in rows:
        pairs += 1
        gained = sorted(normal - own)
        if gained:
            movingpairs += 1
            moved = True
        print(f'{path.name}\ts{sid}\t{name}\t'
              + (','.join(gained) if gained else '(nothing)'))
    movers += 1 if moved else 0

print(f'\n# documents applying a heading-parented name with no resolvable \\sbasedon: {docs}')
print(f'# (document, style) pairs: {pairs}, of which predicted to change: {movingpairs}')
print(f'# documents predicted to change: {movers}')
