"""Generate differential line-break cases from an independent UAX #14 implementation.

`uniseg` implements the same specification from the same property data by different code, so
agreeing with it over text covering every line-break class is real evidence rather than a
restatement of our own logic. Where LibreOffice deliberately differs from plain UAX #14 the
two must disagree, and those cases are excluded here and asserted separately.

Emits a data file the tests read, so running them needs no Python.
"""
import sys
import unicodedata

from uniseg.linebreak import line_break_breakables, line_break

# One representative code point per line-break class, chosen to be printable where possible so
# a failing case is legible.
SAMPLES = {
    'AL': 'a', 'HL': 'א', 'NU': '5', 'ID': '一', 'CJ': 'ぁ',
    'OP': '(', 'CL': ')', 'CP': ']', 'QU': '"', 'EX': '!', 'IS': ',',
    'SY': '/', 'HY': '-', 'BA': '‐', 'BB': '´', 'B2': '—',
    'GL': ' ', 'SP': ' ', 'ZW': '​', 'WJ': '⁠', 'NS': '‼',
    'IN': '‥', 'PR': '$', 'PO': '%', 'JL': 'ᄀ', 'JV': 'ᅡ',
    'JT': 'ᆨ', 'H2': '가', 'H3': '각', 'RI': '\U0001f1e6',
    'EB': '\U0001f466', 'EM': '\U0001f3fb', 'CM': '́', 'ZWJ': '‍',
    'CB': '￼', 'CR': '\r', 'LF': '\n', 'NL': '', 'BK': ' ',
    'AI': '¡', 'SA': 'ก', 'AK': 'ক', 'AP': 'เ',
    'AS': 'া', 'VI': '্', 'VF': 'ি',
}

for name, ch in SAMPLES.items():
    actual = line_break(ch)
    if actual != name:
        print(f'  note: {name} sample U+{ord(ch[0]):04X} is actually {actual}', file=sys.stderr)


def uniseg_breaks(text):
    """The UTF-16 offsets uniseg permits a break at, including the end."""
    offsets = []
    utf16 = 0
    for ch, breakable in zip(text, line_break_breakables(text)):
        if breakable and utf16 > 0:
            offsets.append(utf16)
        utf16 += 2 if ord(ch) > 0xFFFF else 1
    offsets.append(utf16)
    return offsets


# Where LibreOffice's rule file deliberately differs from plain UAX #14, the two implementations
# must disagree — so those shapes are excluded here and asserted directly in the tests instead.
def is_customised(text):
    classes = [line_break(ch) for ch in text]

    for i in range(len(classes) - 1):
        # LB15 disabled: QU SP* OP.
        if classes[i] == 'QU':
            j = i + 1
            while j < len(classes) and classes[j] == 'SP':
                j += 1
            if j < len(classes) and classes[j] == 'OP':
                return True

        # LB21a: LibreOffice's ICU has the Unicode 15.0 form, without 15.1's [^HL].
        if (i + 2 < len(classes) and classes[i] == 'HL'
                and classes[i + 1] in ('HY', 'BA') and classes[i + 2] == 'HL'):
            return True

        # Number range: NU HY NU.
        if (i + 2 < len(classes)
                and classes[i] == 'NU' and classes[i + 1] == 'HY' and classes[i + 2] == 'NU'):
            return True

    # CJ: LibreOffice folds it into NS for strict breaking; uniseg's default is normal.
    if 'CJ' in classes:
        return True

    return False


cases = []
names = sorted(SAMPLES)

# Every ordered pair of classes, with a letter on each side so the pair is not at a boundary.
for a in names:
    for b in names:
        text = 'x' + SAMPLES[a] + SAMPLES[b] + 'y'
        if not is_customised(text):
            cases.append(text)

# Every pair separated by a space, which is what the "X SP* Y" rules are about.
for a in names:
    for b in names:
        text = 'x' + SAMPLES[a] + ' ' + SAMPLES[b] + 'y'
        if not is_customised(text):
            cases.append(text)

# Real prose and structured text, where the interesting rules interact.
PROSE = [
    'The quick brown fox jumps over the lazy dog.',
    'Hello, world! How are you today?',
    'A number: 1,234.56 and a range 5%.',
    'Parenthesised (like this) and bracketed [like that].',
    'A hyphen-ated word and an em—dash.',
    'Non breaking spaces hold on.',
    '日本語のテキストです。',
    '한국어 반갓습니다.',
    'Mixed 中文 and English text.',
    'עברית-אנגלית',
    'Emoji \U0001f466\U0001f3fb and flags \U0001f1e6\U0001f1e7\U0001f1e8\U0001f1e9.',
    'A URL: https://example.invalid/path?query=1',
    'Ellipsis… and leaders‥ too.',
    '$100 costs £50 or ¥900.',
    'Line one\nline two\r\nline three',
    'Zero​width​spaces',
    'Word⁠joiner⁠holds',
    'á combining märk',
    'Thai: สวัสดีครับ',
    'ক্ষ Bengali cluster',
]
cases.extend(t for t in PROSE if not is_customised(t))

print(f'{len(cases)} cases', file=sys.stderr)

lines = []
lines.append('# Differential line-break cases.')
lines.append('#')
lines.append('# Generated from the uniseg package, an independent implementation of UAX #14 over the')
lines.append('# same property data. One case per line: the text as \\uXXXX escapes, a tab, then the')
lines.append('# permitted break offsets as UTF-16 indices separated by spaces.')
lines.append('#')
lines.append('# Cases where LibreOffice deliberately differs from plain UAX #14 are excluded, because')
lines.append('# the two implementations must disagree on them; those are asserted directly in the tests.')
lines.append('#')
lines.append('# DO NOT EDIT. Regenerate with scripts/generate-line-break-cases.py.')

seen = set()
for text in cases:
    if text in seen:
        continue
    seen.add(text)
    escaped = ''.join(f'\\u{unit:04X}' for unit in text.encode('utf-16-le')[0::1][0:0] or
                      [int.from_bytes(text.encode('utf-16-le')[i:i + 2], 'little')
                       for i in range(0, len(text.encode('utf-16-le')), 2)])
    offsets = ' '.join(str(o) for o in uniseg_breaks(text))
    lines.append(f'{escaped}\t{offsets}')

path = ('/home/user/libreoffice-core/dotnet/tests/Paperless.Text.Tests/'
        'line-break-cases.txt')
open(path, 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
print('written ' + path, file=sys.stderr)
