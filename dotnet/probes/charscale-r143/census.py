#!/usr/bin/env python3
"""Census the paragraphs a character width can reach through the uniform shortcut.

`w:w` was resolved correctly through the style chain all along. What lost it is
that a paragraph whose runs all carry their style's formatting is UNIFORM: the
run splitter folds the runs away and three fallbacks rebuild one run from the
paragraph's own face, size, shaping and tracking, none of which carried a
width. So the population is not "documents whose styles.xml states a `w:w`" --
it is every paragraph whose effective scale is not 100 and whose runs do not
differ from each other, which includes a paragraph whose runs each state the
SAME scale.

Two columns per document therefore:

  * `style`  -- paragraphs taking a non-100 scale from the style chain with no
                run of their own stating one;
  * `runs`   -- paragraphs where every run states the same non-100 scale AND
                the paragraph mark resolves to that same value, which is the
                only way a run-stated scale is also uniform.

*The second column's obvious reading is wrong and the sweep is what says so.*
A first cut counted a paragraph as uniform when its runs merely agreed WITH
EACH OTHER, and reported 4 documents and 1138 paragraphs; the sweep moved
ONE document. `RunsOf` compares each run against the PARAGRAPH MARK, not
against its neighbours, so a run stating 99 inside a paragraph whose mark
resolves to 100 makes the paragraph vary and keeps the runs -- and that path
was always right. Census the predicate the code tests, not the one the prose
describes.
"""
import sys, zipfile, pathlib, collections
from xml.etree import ElementTree as ET

W = 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'


def scale_of(rpr):
    if rpr is None:
        return None
    el = rpr.find(f'{{{W}}}w')
    if el is None:
        return None
    try:
        return int(el.get(f'{{{W}}}val'))
    except (TypeError, ValueError):
        return None


def style_scales(zf):
    """The scale each paragraph style resolves to, following w:basedOn."""
    try:
        root = ET.fromstring(zf.read('word/styles.xml'))
    except Exception:
        return {}, None

    own, parent = {}, {}
    for style in root.iter(f'{{{W}}}style'):
        if style.get(f'{{{W}}}type') != 'paragraph':
            continue
        sid = style.get(f'{{{W}}}styleId')
        if not sid:
            continue
        own[sid] = scale_of(style.find(f'{{{W}}}rPr'))
        based = style.find(f'{{{W}}}basedOn')
        parent[sid] = based.get(f'{{{W}}}val') if based is not None else None

    default = None
    defaults = root.find(f'{{{W}}}docDefaults')
    if defaults is not None:
        rpr_default = defaults.find(f'{{{W}}}rPrDefault')
        if rpr_default is not None:
            default = scale_of(rpr_default.find(f'{{{W}}}rPr'))

    resolved = {}
    for sid in own:
        seen, name = set(), sid
        value = None
        while name and name not in seen and name in own:
            seen.add(name)
            if own[name] is not None:
                value = own[name]
                break
            name = parent.get(name)
        resolved[sid] = value if value is not None else default
    return resolved, default


def main():
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '/home/user/sample-files/words')
    print('document\tparagraphs_scaled_by_style\tparagraphs_scaled_by_uniform_runs')
    totals = collections.Counter()
    documents = collections.Counter()

    for path in sorted(root.rglob('*')):
        if path.suffix.lower() not in ('.docx', '.docm') or not path.is_file():
            continue
        try:
            zf = zipfile.ZipFile(path)
            body = ET.fromstring(zf.read('word/document.xml'))
        except Exception:
            continue

        resolved, default = style_scales(zf)
        by_style = by_runs = 0

        for p in body.iter(f'{{{W}}}p'):
            runs = [r for r in p.iter(f'{{{W}}}r')]
            if not runs:
                continue

            stated = [scale_of(r.find(f'{{{W}}}rPr')) for r in runs]
            ppr = p.find(f'{{{W}}}pPr')
            style = ppr.find(f'{{{W}}}pStyle') if ppr is not None else None
            sid = style.get(f'{{{W}}}val') if style is not None else None
            inherited = resolved.get(sid, default)

            if all(s is None for s in stated):
                if inherited not in (None, 100):
                    by_style += 1
            elif (len(set(stated)) == 1 and stated[0] not in (None, 100)
                  and stated[0] == inherited):
                by_runs += 1

        if by_style or by_runs:
            print(f'{path.name}\t{by_style}\t{by_runs}')
            totals['style'] += by_style
            totals['runs'] += by_runs
            if by_style:
                documents['style'] += 1
            if by_runs:
                documents['runs'] += 1

    print(f"# {documents['style']} documents, {totals['style']} paragraphs scaled by their style")
    print(f"# {documents['runs']} documents, {totals['runs']} paragraphs uniform on their runs' own scale")


if __name__ == '__main__':
    main()
