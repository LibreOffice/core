"""One-attribute variants of Demick_JetBlue's slide-5 value-axis number format.

The question is whether 26.2.4.2 right-aligns a value-axis label on its width INCLUDING the
trailing skip-width blank its accounting format states (`_)`), or on its visible ink. Deleting
that one token from the format and nothing else answers it: if the reference's plot-area left
edge then lands where this tree's does, the blank is the whole of the difference.
"""
import pathlib, re, shutil, zipfile

SRC = pathlib.Path('/home/user/sample-files/slides/ceiling-002/pptx/Demick_JetBlue.pptx')
OUT = pathlib.Path(__file__).parent
PART = 'ppt/charts/chart2.xml'

AUTHORED = ('_(&quot;$&quot;* #,##0.00_);_(&quot;$&quot;* \\(#,##0.00\\);'
            '_(&quot;$&quot;* &quot;-&quot;??_);_(@_)')

VARIANTS = {
    # the trailing `_)` gone from the positive and zero sections
    'no-trail': AUTHORED.replace('0.00_)', '0.00)').replace('??_)', '??)'),
    # the leading `_(` gone from every section instead, as the control on the other end
    'no-lead': AUTHORED.replace('_(&quot;', '&quot;').replace('_(@_)', '@_)'),
    # both blanks of the zero row's `??` gone, leaving the dash alone
    'no-qq': AUTHORED.replace('&quot;-&quot;??_)', '&quot;-&quot;_)'),
}

def write(name: str, code: str) -> pathlib.Path:
    target = OUT / f'v-{name}.pptx'
    with zipfile.ZipFile(SRC) as src, zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            data = src.read(item.filename)
            if item.filename == PART:
                text = data.decode('utf-8')
                assert AUTHORED in text, name
                data = text.replace(AUTHORED, code).encode('utf-8')
            dst.writestr(item, data)
    return target

for name, code in VARIANTS.items():
    print(name, write(name, code))

# The two variants the first cut got wrong and the two that settled it. Replacing `_)` with `)`
# leaves a literal closing parenthesis behind and answers a different question, which is why
# `drop-trail` deletes both characters.
CORRECTED = {
    'drop-trail': AUTHORED.replace('0.00_);', '0.00;').replace('&quot;??_);', '&quot;??;'),
    'drop-both': (AUTHORED.replace('_(&quot;', '&quot;')
                          .replace('0.00_);', '0.00;')
                          .replace('&quot;??_);', '&quot;??;')),
}

for name, code in CORRECTED.items():
    print(name, write(name, code))
