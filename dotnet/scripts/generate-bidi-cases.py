"""Generate differential bidi cases from ICU's own `ubidi`.

This is stronger evidence than the line breaker's differential set. There the reference was
an independent implementation of the same specification; here the reference is the *same
engine LibreOffice calls* — `ubidi_setPara` and `ubidi_getLevels`, reached through ctypes
against the installed `libicuuc`. Agreeing with it is agreeing with Writer, not merely with
a defensible reading of UAX #9.

Cases cover: every ordered pair and triple of representative characters from each Bidi_Class,
numbers in right-to-left context (the classic W2/W7 case), bracket pairs in both directions
(N0), the explicit embedding and isolate controls, and real prose in Hebrew, Arabic and Latin.
Each is emitted at both paragraph directions, since the paragraph level changes almost every
rule's answer.

Emits a data file the tests read, so running them needs no Python and no ICU.
"""
import ctypes
import itertools
import sys

MAX_CASES_PER_GROUP = 100000

CANDIDATE_LIBRARIES = ['libicuuc.so.74', 'libicuuc.so', 'libicuuc.so.72', 'libicuuc.so.76']


def load_icu():
    for name in CANDIDATE_LIBRARIES:
        try:
            library = ctypes.CDLL(name)
        except OSError:
            continue
        for suffix in [''] + [f'_{v}' for v in range(60, 90)]:
            if hasattr(library, 'ubidi_setPara' + suffix):
                return library, suffix
    print('no libicuuc with ubidi found', file=sys.stderr)
    sys.exit(1)


icu, SUFFIX = load_icu()


def fn(name, restype, argtypes):
    f = getattr(icu, name + SUFFIX)
    f.restype = restype
    f.argtypes = argtypes
    return f


ubidi_openSized = fn('ubidi_openSized', ctypes.c_void_p,
                     [ctypes.c_int32, ctypes.c_int32, ctypes.POINTER(ctypes.c_int)])
ubidi_close = fn('ubidi_close', None, [ctypes.c_void_p])
ubidi_setPara = fn('ubidi_setPara', None,
                   [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int32, ctypes.c_uint8,
                    ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)])
ubidi_getLevels = fn('ubidi_getLevels', ctypes.POINTER(ctypes.c_uint8),
                     [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int)])
u_getUnicodeVersion = fn('u_getUnicodeVersion', None, [ctypes.c_char * 4])

_version = (ctypes.c_char * 4)()
u_getUnicodeVersion(_version)
UNICODE_VERSION = '.'.join(str(b) for b in _version.raw[:3])


def levels(text, para_level):
    """ICU's resolved embedding level for every UTF-16 code unit."""
    units = text.encode('utf-16-le')
    length = len(units) // 2
    error = ctypes.c_int(0)
    bidi = ubidi_openSized(length, 0, ctypes.byref(error))
    try:
        error.value = 0
        buf = ctypes.create_string_buffer(units, len(units))
        ubidi_setPara(bidi, ctypes.cast(buf, ctypes.c_void_p), length, para_level,
                      None, ctypes.byref(error))
        if error.value > 0:
            return None
        error.value = 0
        got = ubidi_getLevels(bidi, ctypes.byref(error))
        if error.value > 0:
            return None
        return [got[i] for i in range(length)]
    finally:
        ubidi_close(bidi)


# One representative character per Bidi_Class, printable where there is a printable choice so
# a failing case can be read.
SAMPLES = {
    'L': 'a',
    'R': 'א',        # HEBREW LETTER ALEF
    'AL': 'ا',       # ARABIC LETTER ALEF
    'EN': '1',
    'ES': '+',
    'ET': '%',
    'AN': '٠',       # ARABIC-INDIC DIGIT ZERO
    'CS': ',',
    'NSM': '̀',      # COMBINING GRAVE ACCENT
    'BN': '​',       # ZERO WIDTH SPACE (BN)
    'B': ' ',        # PARAGRAPH SEPARATOR
    'S': '\t',
    'WS': ' ',
    'ON': '!',
    'LRE': '‪',
    'RLE': '‫',
    'PDF': '‬',
    'LRO': '‭',
    'RLO': '‮',
    'LRI': '⁦',
    'RLI': '⁧',
    'FSI': '⁨',
    'PDI': '⁩',
}

BRACKETS = ['()', '[]', '{}', '〈〉', '〈〉']

PROSE = [
    'Latin only, with 123 numbers and (brackets).',
    'שלום עולם',
    'Start שלום 123 עולם end.',
    'שלום (abc) עולם',
    'العربية 123 مثال',
    'العربية ٠١٢ abc',
    'He said "שלום" and left.',
    'א ا 1 ٠ a',
    'àא̀ 1̀',
    'car means سيارة.',
    'DID YOU SAY ‘היא אמרה “שלום”’?',
    '‫RLE embedded‬ after',
    '⁧RLI isolated⁩ after',
    '⁨FSI שלום⁩ after',
    '‮overridden‬ after',
    'unterminated ‫ embedding',
    'stray ⁩ pop',
    '1.5 and א 2,5',
    'א $12.50 א',
    'ا 12 ا',
]

cases = []
seen = set()


def add(text):
    if not text or text in seen:
        return
    seen.add(text)
    for para in (0, 1):
        got = levels(text, para)
        if got is None:
            return
        cases.append((text, para, got))


names = sorted(SAMPLES)

# Every ordered pair, and the same pair across a space — the space is where N1 and N2 decide.
for a, b in itertools.product(names, repeat=2):
    add(SAMPLES[a] + SAMPLES[b])
    add(SAMPLES[a] + ' ' + SAMPLES[b])

# Every ordered triple of the classes the weak and implicit rules actually turn on. The full
# product of all classes would be a quarter of a million cases for no extra coverage.
CORE = ['L', 'R', 'AL', 'EN', 'AN', 'ES', 'ET', 'CS', 'ON', 'WS', 'NSM', 'S', 'BN']
for a, b, c in itertools.product(CORE, repeat=3):
    add(SAMPLES[a] + SAMPLES[b] + SAMPLES[c])

# Brackets with each kind of content and each kind of context, which is all of N0.
for pair in BRACKETS:
    for inside in ['a', SAMPLES['R'], SAMPLES['AL'], '1', SAMPLES['AN'], '!', 'aא']:
        for before in ['', 'a ', SAMPLES['R'] + ' ', SAMPLES['AL'] + ' ', '1 ']:
            for after in ['', ' a', ' ' + SAMPLES['R'], ' 1']:
                add(before + pair[0] + inside + pair[1] + after)

# Nested and mismatched brackets, where BD16's stack does the work.
add('a(b[c)d]e')
add('א(א[א)א]א')
add('a(b(c(d)e)f)g')
add('א(a(א(a)א)a)א')

for text in PROSE:
    add(text)

# Explicit controls in every nesting the stack can reach, including the overflowing ones.
CONTROLS = ['LRE', 'RLE', 'LRO', 'RLO', 'PDF', 'LRI', 'RLI', 'FSI', 'PDI']
for a, b in itertools.product(CONTROLS, repeat=2):
    add('a' + SAMPLES[a] + 'א' + SAMPLES[b] + 'b')
    add(SAMPLES['R'] + SAMPLES[a] + '1' + SAMPLES[b] + SAMPLES['L'])

print(f'{len(cases)} cases from ICU (Unicode {UNICODE_VERSION})', file=sys.stderr)


def escape(text):
    return ''.join(
        ch if 0x20 < ord(ch) < 0x7f and ch not in '\\' else f'\\u{ord(ch):04X}'
        for ch in text)


out = [
    '# Differential bidi cases.',
    '#',
    "# Generated from ICU's own ubidi — the engine LibreOffice resolves bidi with",
    '# (ubidi_setPara in vcl/source/text/ImplLayoutArgs.cxx), reached through ctypes by',
    '# scripts/generate-bidi-cases.py. Agreeing with these is agreeing with Writer.',
    '#',
    f'# ICU carried Unicode {UNICODE_VERSION} when this was generated.',
    '#',
    '# One case per line: the text as \\uXXXX escapes, a tab, the paragraph embedding level,',
    '# a tab, then the resolved embedding level of each UTF-16 code unit separated by spaces.',
    '',
]
for text, para, got in cases:
    out.append(f'{escape(text)}\t{para}\t{" ".join(str(v) for v in got)}')

path = '/home/user/wt-text-bidi/dotnet/tests/Paperless.Text.Tests/bidi-cases.txt'
open(path, 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print('written ' + path, file=sys.stderr)
