#!/usr/bin/env python3
r"""Which style names reach a Writer style at all, by which of the two routes, and what each
one's pool parent chain is.

`probes/words-close-r95/standard-census.py` lists the 84 names the 338 converted `.rtf` apply
without a resolvable `\sbasedon`, and reads the `ConvertStyleName` map by hand for six of them.
That map is only *half* of what decides the question. `StyleSheetTable::ApplyStyleSheets`
(`sw/source/writerfilter/dmapper/StyleSheetTable.cxx`:1099-1101) converts the name and then asks
`xStyles->hasByName` -- and for a name the map does not hold, `ConvertStyleName` returns the name
**unchanged** (`:2083-2113`), so the second lookup is on the name as the file wrote it. That is a
set nobody had enumerated: it is Writer's own paragraph style names, programmatic *and* UI, and it
reaches names such as `Figure`, `Heading`, `Text` and `Comment` that are in the map for neither
spelling.

The rules modelled here, all from `ConvertStyleName` and `SwXStyleFamily::hasByName`
(`sw/source/core/unocore/unostyle.cxx`:1030-1039):

  1. a map entry with a non-empty value answers that Writer name, and it always exists;
  2. otherwise the name passes through -- but if it collides with any *value* of the map it gets
     a ` (WW)` suffix and then matches nothing (`:2093-2112`);
  3. `hasByName` resolves the name through `SwStyleNameMapper::FillUIName`
     (`sw/source/core/doc/SwStyleNameMapper.cxx`:306-335) and looks the answer up in the document's
     style pool. **That is a programmatic-name lookup and not "either spelling"**, which is what a
     first cut of this script assumed: a name that is a pool style's *UI* name and not its
     programmatic one falls into `FillUIName`'s `else` branch and is handed to `Find` with
     **`" (user)"` appended**, so it matches nothing. `Body Text` is the example -- it is
     `COLL_TEXT`'s UI name and `Text body` is its programmatic one -- and it reaches Writer's style
     only because `ConvertStyleName` maps it to the programmatic spelling first. The rule below
     therefore takes the programmatic column for the pass-through route and both columns for the
     map's answer, and prints any name where the two rules disagree.

The pool parent chain is `GetPoolParent` (`sw/source/core/doc/poolfmt.cxx`:169-296), transcribed
per group.

  hasbyname-census.py <writer-names.tsv> <StyleSheetTable.cxx> <corpus root>
"""
import collections
import pathlib
import re
import sys

CW = re.compile(r'\\[a-zA-Z]+-?\d*[ ]?')
USE = re.compile(r'\\s(\d+)(?![0-9])')

# GetPoolParent, poolfmt.cxx:169-296, by STR_POOLCOLL_* suffix. `None` is "no pool parent".
PARENT = {
    'STANDARD': None,
    'TEXT_IDENT': 'TEXT', 'TEXT_NEGIDENT': 'TEXT', 'TEXT_MOVE': 'TEXT',
    'CONFRONTATION': 'TEXT', 'MARGINAL': 'TEXT',
    'TEXT': 'STANDARD', 'GREETING': 'STANDARD', 'SIGNATURE': 'STANDARD',
    'TABLE_HDLN': 'TABLE',
    'FRAME': 'STANDARD', 'TABLE': 'STANDARD', 'FOOTNOTE': 'STANDARD',
    'ENDNOTE': 'STANDARD', 'ENVELOPE_ADDRESS': 'STANDARD', 'SEND_ADDRESS': 'STANDARD',
    'HEADERFOOTER': 'STANDARD', 'LABEL': 'STANDARD', 'COMMENT': 'STANDARD',
    'HEADER': 'HEADERFOOTER', 'HEADERL': 'HEADER', 'HEADERR': 'HEADER',
    'FOOTER': 'HEADERFOOTER', 'FOOTERL': 'FOOTER', 'FOOTERR': 'FOOTER',
    'LABEL_ABB': 'LABEL', 'LABEL_TABLE': 'LABEL', 'LABEL_FRAME': 'LABEL',
    'LABEL_DRAWING': 'LABEL', 'LABEL_FIGURE': 'LABEL',
    'REGISTER_BASE': 'STANDARD',
    'TOX_IDXH': 'HEADLINE_BASE',
    'TOX_USERH': 'TOX_IDXH', 'TOX_CNTNTH': 'TOX_IDXH', 'TOX_ILLUSH': 'TOX_IDXH',
    'TOX_OBJECTH': 'TOX_IDXH', 'TOX_TABLESH': 'TOX_IDXH', 'TOX_AUTHORITIESH': 'TOX_IDXH',
    'HEADLINE_BASE': 'STANDARD',
}
GROUP_DEFAULT = {'LISTS': 'NUMBER_BULLET_BASE', 'REGISTER': 'REGISTER_BASE',
                 'DOC': 'HEADLINE_BASE', 'HTML': 'STANDARD'}
PARENT['NUMBER_BULLET_BASE'] = 'TEXT'


def load_writer_names(path):
    """{prog -> suffix}, {either spelling -> suffix}, {suffix -> group}."""
    by_prog, by_either, group = {}, {}, {}
    for line in pathlib.Path(path).read_text(encoding='utf-8').splitlines():
        g, macro, prog, ui = line.split('\t')
        suffix = macro[len('STR_POOLCOLL_'):]
        group[suffix] = g
        by_prog.setdefault(prog, suffix)
        by_either.setdefault(prog, suffix)
        by_either.setdefault(ui, suffix)
    return by_prog, by_either, group


def load_map(path):
    """`ConvertStyleName`'s map, read out of the source rather than retyped."""
    src = pathlib.Path(path).read_text(encoding='utf-8')
    start = src.index('static const std::map< OUString, OUString> StyleNameMap {')
    end = src.index('// find style-name using map', start)
    pairs = re.findall(r'\{\s*"((?:[^"\\]|\\.)*)",\s*"((?:[^"\\]|\\.)*)"\s*\}', src[start:end])
    return dict(pairs)


def chain_of(suffix, group):
    out, seen = [], set()
    while suffix and suffix not in seen:
        seen.add(suffix)
        out.append(suffix)
        suffix = PARENT.get(suffix, GROUP_DEFAULT.get(group.get(suffix), None))
    return out


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


def main():
    by_prog, by_either, group = load_writer_names(sys.argv[1])
    name_map = load_map(sys.argv[2])
    reserved = set(name_map.values())

    def resolve(raw, lenient=False):
        """(converted name, route, pool suffix or None) -- ConvertStyleName then hasByName.

        `lenient` is the rejected rule: it lets a pool style's UI name hit as well, which is what
        this script asserted before `FillUIName` was read. Kept so the two can be differenced.
        """
        table = by_either if lenient else by_prog
        name = raw.strip()
        mapped = name_map.get(name)
        if mapped:
            # A map value is a programmatic name by construction, so the two tables agree here.
            return mapped, 'map', by_either.get(mapped)
        if name in reserved or name.endswith(' (WW)'):
            return name + ' (WW)', 'suffixed', None
        return name, 'hasByName', table.get(name)

    documents = collections.Counter()
    where = collections.defaultdict(set)
    files = 0
    for path in sorted(pathlib.Path(sys.argv[3]).rglob('*.rtf')):
        files += 1
        text = path.read_bytes().decode('cp1252', 'replace')
        sheet, end = entries(text)
        body = text if end < 0 else text[:text.find('{\\stylesheet')] + text[end + 1:]
        used = {int(m.group(1)) for m in USE.finditer(body)}
        seen, here = set(), set()
        for entry in sheet:
            match = re.match(r'\{\\(\*\\)?(s|cs|ts)(\d+)?\b', entry)
            kind = match.group(2) if match else 's'
            sid = int(match.group(3)) if match and match.group(3) else 0
            based = re.search(r'\\sbasedon(\d+)\b', entry)
            resolves = bool(based) and int(based.group(1)) != sid and int(based.group(1)) in seen
            seen.add(sid)
            if kind != 's' or resolves or sid not in used:
                continue
            here.add(name_of(entry))
            where[name_of(entry)].add(path.name)
        for name in here:
            documents[name] += 1

    print(f'{files} .rtf scanned; {len(documents)} distinct names applied without a '
          f'resolvable \\sbasedon')
    rows = []
    for name, count in sorted(documents.items(), key=lambda kv: (-kv[1], kv[0])):
        converted, route, suffix = resolve(name)
        chain = chain_of(suffix, group) if suffix else []
        rows.append((name, count, converted, route, suffix, chain))

    hit = [r for r in rows if r[4]]
    standard = [r for r in hit if 'STANDARD' in r[5]]
    print(f'{len(hit)} of them reach an existing Writer paragraph style; '
          f'{len(standard)} have COLL_STANDARD somewhere above')
    print()
    print(f'{"docs":>5}  {"name":<26} {"route":<10} {"Writer style":<22} pool chain')
    for name, count, converted, route, suffix, chain in rows:
        if not suffix:
            continue
        print(f'{count:>5}  {name!r:<26} {route:<10} {converted:<22} '
              + ' > '.join('COLL_' + c for c in chain))
    print()
    docs = set()
    for name, count, converted, route, suffix, chain in rows:
        if suffix and 'STANDARD' in chain:
            docs |= where[name]
    print(f'{len(docs)} of the {files} documents apply at least one name that reaches '
          f'COLL_STANDARD through Writer\'s own style')
    print()
    for name, count, converted, route, suffix, chain in rows:
        if suffix and count <= 6:
            print(f'  {name!r}: {", ".join(sorted(where[name]))}')
    print()
    print('names that reach no Writer paragraph style at all:')
    for name, count, converted, route, suffix, chain in rows:
        if not suffix:
            print(f'{count:>5}  {name!r:<26} {route:<10} -> {converted!r}')
    print()
    lenient = [(n, c) for n, c, *_ in rows if not resolve(n)[2] and resolve(n, True)[2]]
    print(f'{len(lenient)} names the UI-spelling rule would have added and FillUIName rejects:'
          + (' none' if not lenient else ''))
    for name, count in lenient:
        print(f'{count:>5}  {name!r}')


main()
