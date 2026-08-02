"""Generate the font substitution table from LibreOffice's own configuration.

`officecfg/registry/data/org/openoffice/VCL.xcu` carries, per locale, a map from a requested
font name to the ordered list of substitutes LibreOffice will actually try. The research notes
call this "the single most valuable, portable artifact for a C# port", because it encodes what
LibreOffice renders when a document asks for a font that is not installed — independently of
any platform font API. So it is read rather than reimplemented.

Only the neutral (`en`) locale is emitted. The per-locale tables differ mainly in the
CJK-preference ordering, and taking one locale's answers for another would change which font a
document renders in; a locale-aware table is a later job and wants the locale plumbed through
the resolver first.
"""
import os
import sys
import xml.etree.ElementTree as ET

# Derived from this script's own location rather than hardcoded. An absolute path pinned to one
# checkout silently writes into *that* tree when the script is run from a git worktree, so the
# regenerated table lands outside the branch that is meant to carry it — and the worktree builds
# on, unchanged, against a table nobody edited.
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOURCE = os.path.join(ROOT, 'officecfg/registry/data/org/openoffice/VCL.xcu')
OUT = os.path.join(ROOT, 'dotnet/src/Paperless.Text/Fonts/FontSubstitutions.Tables.cs')

OOR = '{http://openoffice.org/2001/registry}'
LOCALE = 'en'

tree = ET.parse(SOURCE)
root = tree.getroot()


def named(node, name):
    for child in node.findall(f'{{{OOR[1:-1]}}}node') or []:
        pass
    for child in node:
        if child.tag.endswith('node') and child.get(OOR + 'name') == name:
            return child
    return None


def prop_value(node, name):
    """The text of a <prop oor:name="..."> child, or '' when it has none."""
    for prop in node:
        if prop.tag.endswith('prop') and prop.get(OOR + 'name') == name:
            return ''.join(v.text or '' for v in prop if v.tag.endswith('value')).strip()
    return ''


# `DefaultFonts` answers a different question from `FontSubstitutions`, and conflating the two
# reflows every document that names no font at all. A document *requesting* a face nobody has is
# a substitution; a document requesting nothing is asking for the application default, and this
# is where LibreOffice's default template gets it. LATIN_TEXT is the body-text default — a serif
# list headed by Liberation Serif — where every other LATIN_* category is a sans list.
defaults = named(root, 'DefaultFonts')
latin_text = []
if defaults is not None:
    locale_defaults = named(defaults, LOCALE)
    if locale_defaults is not None:
        for name in prop_value(locale_defaults, 'LATIN_TEXT').split(';'):
            normalised = ''.join(c for c in name.strip().lower() if c.isalnum())
            if normalised and normalised not in latin_text:
                latin_text.append(normalised)

if not latin_text:
    print('DefaultFonts/LATIN_TEXT not found', file=sys.stderr)
    sys.exit(1)
print(f'default latin text chain: {latin_text[:4]}', file=sys.stderr)

substitutions = named(root, 'FontSubstitutions')
if substitutions is None:
    print('FontSubstitutions not found', file=sys.stderr)
    sys.exit(1)

locale = named(substitutions, LOCALE)
if locale is None:
    print(f'locale {LOCALE} not found', file=sys.stderr)
    sys.exit(1)

# `FontType` is a comma-separated token list mixing weight/width words with a shape word, and
# the shape word is what drives the generic fallback once nothing in `SubstFonts` turned out to
# be installed. Tokens are matched exactly rather than by substring, since "SansSerif" contains
# "Serif" and a substring test would classify every grotesque as a roman.
SHAPE_TOKENS = (
    ('Symbol', 'Symbol'),
    ('Fixed', 'Fixed'),
    ('SansSerif', 'SansSerif'),
    ('Serif', 'Serif'),
)


def classify(font_type):
    """The shape token of a FontType value, or '' when it names none.

    Ordered by how strongly the token constrains the substitute: a symbol face shares no
    metrics with a text face at all, and a monospaced request is relying on its columns
    lining up, so both outrank the roman/grotesque distinction.
    """
    if not font_type:
        return ''
    tokens = {t.strip() for t in font_type.split(',') if t.strip()}
    for token, kind in SHAPE_TOKENS:
        if token in tokens:
            return kind
    return ''


entries = {}
classes = {}
for font in locale:
    if not font.tag.endswith('node'):
        continue
    requested = font.get(OOR + 'name')
    if not requested:
        continue

    substitutes = None
    microsoft = None
    font_type = None
    for prop in font:
        if not prop.tag.endswith('prop'):
            continue
        key = prop.get(OOR + 'name')
        value = ''.join(v.text or '' for v in prop if v.tag.endswith('value')).strip()
        if key == 'SubstFonts':
            substitutes = value
        elif key == 'SubstFontsMS':
            microsoft = value
        elif key == 'FontType':
            font_type = value

    # The names are already normalised — lower case, no spaces — which is the form a lookup has
    # to normalise a document's request into as well.
    chain = []
    for name in (substitutes or '').split(';'):
        name = name.strip()
        if name and name not in chain:
            chain.append(name)

    kind = classify(font_type)

    # An entry earns its place with either half: a chain says what to try, and a FontType says
    # what shape to fall back to when none of the chain is installed. Requiring both would drop
    # the classification for families whose chain the table leaves empty.
    if chain or kind:
        entries[requested] = (chain, microsoft or '', kind)
        if kind:
            classes[requested] = kind

print(f'{len(entries)} substitution entries, {len(classes)} of them classified', file=sys.stderr)
for kind in ('SansSerif', 'Serif', 'Fixed', 'Symbol'):
    print(f'  {kind}: {sum(1 for k in classes.values() if k == kind)}', file=sys.stderr)

# The metric-compatible pairs matter more than anything else in the table, so they are checked
# rather than assumed: a substitution that is not metric-compatible reflows the document.
for probe in ('calibri', 'cambria', 'arial', 'timesnewroman', 'couriernew'):
    if probe in entries:
        print(f'  {probe} -> {entries[probe][0][:4]}', file=sys.stderr)
    else:
        print(f'  {probe} is absent from the table', file=sys.stderr)

out = []
w = out.append
w('// <auto-generated>')
w("// Generated from LibreOffice's own font substitution configuration:")
w('//   officecfg/registry/data/org/openoffice/VCL.xcu, the neutral "en" locale')
w('//')
w('// This table encodes what LibreOffice actually renders when a document asks for a font that is')
w('// not installed, which is why it is read rather than reimplemented. Regenerate rather than edit:')
w('// scripts/generate-font-substitutions.py.')
w('// </auto-generated>')
w('')
w('namespace Paperless.Text.Fonts;')
w('')
w('public static partial class FontSubstitutions')
w('{')
w('    /// <summary>Requested name to the ordered substitutes LibreOffice tries, both normalised.</summary>')
w('    private static readonly Dictionary<string, string[]> Chains =')
w('        new(StringComparer.Ordinal)')
w('        {')
for requested in sorted(entries):
    chain, _, _ = entries[requested]
    if not chain:
        continue
    joined = ', '.join(f'"{name}"' for name in chain)
    w(f'            ["{requested}"] = [{joined}],')
w('        };')
w('')
w('    /// <summary>Requested name to the Microsoft font it stands in for, where the table names one.</summary>')
w('    /// <remarks>')
w('    /// Useful in the other direction: it is how a free font declares which commercial one it is')
w('    /// metric-compatible with, and therefore which requests it can satisfy without reflowing.')
w('    /// </remarks>')
w('    private static readonly Dictionary<string, string> MicrosoftEquivalents =')
w('        new(StringComparer.Ordinal)')
w('        {')
for requested in sorted(entries):
    _, microsoft, _ = entries[requested]
    if microsoft:
        w(f'            ["{requested}"] = "{microsoft}",')
w('        };')
w('')
w('    /// <summary>Requested name to the shape LibreOffice files it under, from its <c>FontType</c>.</summary>')
w('    /// <remarks>')
w('    /// The half of the table that decides what happens when <em>nothing</em> in a chain is')
w("    /// installed, which on a typical Linux box is the common case: the chains name Microsoft and")
w('    /// Agfa faces that are not there. Without it the shape has to be guessed from the family name,')
w('    /// and the guess is wrong for exactly the families that matter — Tahoma, Verdana and Segoe UI')
w('    /// carry no hint that they are grotesques.')
w('    /// </remarks>')
w('    private static readonly Dictionary<string, FontFamilyClass> Classes =')
w('        new(StringComparer.Ordinal)')
w('        {')
for requested in sorted(classes):
    w(f'            ["{requested}"] = FontFamilyClass.{classes[requested]},')
w('        };')
w('')
w('    /// <summary>The body-text faces LibreOffice defaults to when a document names none.</summary>')
w('    /// <remarks>')
w("    /// <c>DefaultFonts</c>/<c>LATIN_TEXT</c> for the neutral locale, which is a different question")
w('    /// from a substitution: it is what the default template asks for, not what stands in for a')
w('    /// missing face. Every other LATIN_* category in that node is a sans list; this one is serif,')
w('    /// which is why a document specifying no font renders in Liberation Serif.')
w('    /// </remarks>')
joined = ', '.join(f'"{name}"' for name in latin_text)
w(f'    private static readonly string[] LatinTextDefaultChain = [{joined}];')
w('}')

open(OUT, 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print('written ' + OUT, file=sys.stderr)
