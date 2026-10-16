#!/usr/bin/env python3
"""How many corpus documents the *inherited* family class would move, and which.

    python3 class-inheritance-census.py [corpus-root]

Round 54 shipped "a named family the font table does not file under `swiss` takes Writer's roman
default". `probes/words-r55/family-inheritance.py` measures the actual rule on 28 authored files:
the class is an **inherited property**, set only where `w:rFonts/@w:ascii` names a font the table
files under `roman` or `swiss`, and left alone by `auto`, `modern`, `script`, `decorative`, a
pitch-only entry, an absent entry, and by `w:asciiTheme` — which sets the *name* and never the
class.  Nothing anywhere naming one leaves it roman.

This walks every `.docx` in the words corpus and counts, per document, the runs whose resolved
family would take a **different** class under the inherited rule than under round 54's per-name
one, restricted to families that actually reach a fallback (an installed family or one of the
five strong metric aliases never gets there).

What it cannot see — stated here rather than discovered afterwards:

  * **table styles and numbering.** `w:tblStylePr` run properties and a `w:lvl`'s own `w:rPr` are
    property layers too, and this walks only docDefaults → paragraph style chain → character
    style → direct `w:rPr`. Both can only *add* movement, so this is a floor, not a ceiling.
  * **which of the two answers is right.** It counts disagreements between two rules; the probe
    says which rule the reference follows, and only the sweep says what that costs on a page.
  * **`.doc` and `.rtf`.** A different filter with a different font table; the WW8 `FFN` carries a
    family per font and `sw`'s reader sets it per font, so there is no inheritance to model there.
    Two `.doc` documents are in the corpus's current wrong-direction list and this cannot speak to
    them.
  * **line breaking.** A class change moves a face; whether that costs a page is the sweep's
    question, and DejaVu Serif and DejaVu Sans have different advances, so it usually does.
"""
import os
import re
import sys
import zipfile
from xml.etree import ElementTree

W = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
A = '{http://schemas.openxmlformats.org/drawingml/2006/main}'

# Families that never reach a fallback here: installed, or a strong metric alias onto one.
# `Helvetica`, `Albany` and `Thorndale` are deliberately *not* in this set — round 54 measured all
# three answering DejaVu Serif, so their chain entries are not installed and they do fall back.
RESOLVED = {
    'liberation serif', 'liberation sans', 'liberation mono', 'liberation sans narrow',
    'carlito', 'caladea', 'dejavu serif', 'dejavu sans', 'dejavu sans mono', 'opensymbol',
    'times new roman', 'arial', 'courier new', 'calibri', 'cambria', 'symbol',
    'wenquanyi zen hei', 'wenquanyi zen hei sharp',
}

SLOTS = [('ascii', 'asciiTheme'), ('hAnsi', 'hAnsiTheme'), ('cs', 'cstheme'),
         ('eastAsia', 'eastAsiaTheme')]


def theme_faces(zf):
    """{'majorHAnsi': name, …} from word/theme/theme1.xml, or {}."""
    for name in zf.namelist():
        if re.fullmatch(r'word/theme/theme\d*\.xml', name):
            root = ElementTree.fromstring(zf.read(name))
            out = {}
            for kind, prefix in (('majorFont', 'major'), ('minorFont', 'minor')):
                node = root.find(f'.//{A}fontScheme/{A}{kind}')
                if node is None:
                    continue
                for tag, suffix in (('latin', 'HAnsi'), ('latin', 'Ascii'),
                                    ('ea', 'EastAsia'), ('cs', 'Bidi')):
                    face = node.find(f'{A}{tag}')
                    if face is not None and face.get('typeface'):
                        out[prefix + suffix] = face.get('typeface')
            return out
    return {}


def slot_family(fonts, themes, direct, themed):
    """One slot's name, the theme attribute beating the direct one — and whether it was direct."""
    if fonts is None:
        return None, False
    key = fonts.get(f'{W}{themed}')
    if key and themes.get(key):
        return themes[key], False
    name = fonts.get(f'{W}{direct}')
    return (name, True) if name else (None, False)


def resolve(layers, themes):
    """The family the layers give, innermost first, ascii slot preferred."""
    for direct, themed in SLOTS:
        for fonts in layers:
            name, _ = slot_family(fonts, themes, direct, themed)
            if name:
                return name
    return None


def inherited_class(layers, themes, table):
    """The class the *inherited* rule gives: innermost direct `w:ascii` the table files."""
    for fonts in layers:
        name, direct = slot_family(fonts, themes, 'ascii', 'asciiTheme')
        if name and direct and table.get(name.lower()) in ('roman', 'swiss'):
            return table[name.lower()]
    return 'roman'


def read_styles(zf):
    """(docDefaults rFonts, {styleId: (basedOn, rFonts)}, {type: defaultStyleId})."""
    try:
        root = ElementTree.fromstring(zf.read('word/styles.xml'))
    except KeyError:
        return None, {}, {}
    dd = root.find(f'{W}docDefaults/{W}rPrDefault/{W}rPr/{W}rFonts')
    styles, defaults = {}, {}
    for style in root.findall(f'{W}style'):
        sid = style.get(f'{W}styleId')
        based = style.find(f'{W}basedOn')
        styles[sid] = (based.get(f'{W}val') if based is not None else None,
                       style.find(f'{W}rPr/{W}rFonts'))
        if style.get(f'{W}default') in ('1', 'true'):
            defaults.setdefault(style.get(f'{W}type'), sid)
    return dd, styles, defaults


def chain(styles, sid, seen=None):
    """A style's rFonts layers, itself first, then its ancestors."""
    out, seen = [], seen or set()
    while sid and sid in styles and sid not in seen:
        seen.add(sid)
        based, fonts = styles[sid]
        if fonts is not None:
            out.append(fonts)
        sid = based
    return out


def census(path):
    """(runs-that-move, families-that-move) for one package."""
    moved, families = 0, {}
    with zipfile.ZipFile(path) as zf:
        try:
            table = {}
            for font in ElementTree.fromstring(zf.read('word/fontTable.xml')):
                name, fam = font.get(f'{W}name'), font.find(f'{W}family')
                if name:
                    table[name.lower()] = fam.get(f'{W}val') if fam is not None else None
        except KeyError:
            table = {}
        themes = theme_faces(zf)
        dd, styles, defaults = read_styles(zf)
        parts = [n for n in zf.namelist()
                 if re.fullmatch(r'word/(document|header\d*|footer\d*|footnotes|endnotes)\.xml', n)]
        for part in parts:
            try:
                root = ElementTree.fromstring(zf.read(part))
            except ElementTree.ParseError:
                continue
            for para in root.iter(f'{W}p'):
                pstyle = para.find(f'{W}pPr/{W}pStyle')
                pid = pstyle.get(f'{W}val') if pstyle is not None else defaults.get('paragraph')
                para_layers = chain(styles, pid)
                for run in para.iter(f'{W}r'):
                    rpr = run.find(f'{W}rPr')
                    own = rpr.find(f'{W}rFonts') if rpr is not None else None
                    rstyle = rpr.find(f'{W}rStyle') if rpr is not None else None
                    layers = ([own] if own is not None else []) \
                        + chain(styles, rstyle.get(f'{W}val') if rstyle is not None else None) \
                        + para_layers + ([dd] if dd is not None else [])
                    name = resolve(layers, themes)
                    if not name or name.lower() in RESOLVED:
                        continue
                    old = 'swiss' if table.get(name.lower()) == 'swiss' else 'roman'
                    new = inherited_class(layers, themes, table)
                    if old != new:
                        moved += 1
                        families[(name, old, new)] = families.get((name, old, new), 0) + 1
    return moved, families


def main():
    root = os.path.abspath(sys.argv[1] if len(sys.argv) > 1
                           else '/c/sandbox/workdir/sample-files')
    rows = []
    total_docx = 0
    for dirpath, _dirs, names in os.walk(os.path.join(root, 'words')):
        for name in sorted(names):
            if not name.lower().endswith(('.docx', '.docm', '.dotx', '.dotm')):
                continue
            path = os.path.join(dirpath, name)
            total_docx += 1
            try:
                moved, families = census(path)
            except (zipfile.BadZipFile, ElementTree.ParseError, KeyError):
                continue
            if moved:
                rows.append((moved, os.path.relpath(path, root), families))

    rows.sort(reverse=True)
    print(f'{total_docx} word-processing OOXML packages walked '
          f'(note: the mount aliases every directory, so this double-counts; '
          f'distinct paths below)')
    print(f'{len(rows)} of them have at least one run whose family class moves\n')
    seen = set()
    for moved, rel, families in rows:
        key = rel.lower()
        if key in seen:
            continue
        seen.add(key)
        shapes = ', '.join(f'{n} ({o}->{w}) x{c}' for (n, o, w), c in
                           sorted(families.items(), key=lambda kv: -kv[1])[:4])
        print(f'{moved:6d}  {rel}\n          {shapes}')
    print(f'\n{len(seen)} distinct documents')


if __name__ == '__main__':
    main()
