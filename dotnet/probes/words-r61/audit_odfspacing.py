#!/usr/bin/env python3
"""24.2.7-audit: do ODF's two paragraph-spacing settings still behave as the site records?

    python3 audit_odfspacing.py <outdir> [workers]

`OdtLayoutSource.AddsParagraphSpacing` and `OdtLayoutSource.KeepsParagraphSpacingAtPages` each carry
a measurement taken on **24.2.7.2** and the reference binary is now 26.2.4.2. Round 59 re-checked
the DOCX twin of the first (`WordCompatibility.AddsParagraphSpacing`) and verified it; this is the
ODF pair, which is a different code path in LibreOffice — `SwXDocumentSettings` reading
`office:settings` rather than `DomainMapper_Impl::ApplySettingsTable` reading `w:compat`.

The claims, quoted from the sites:

  * `AddParaTableSpacing` **false** puts every paragraph boundary at **24.00 pt** (the larger of a
    12 pt space-before and an 8 pt space-after wins) and **true** at **32.00 pt** (they sum).
    Absent means **true**, because the application default is `true`. The name says the opposite of
    what it does, which is the reason it is worth a probe at all.
  * `AddParaTableSpacingAtStart` **true** puts the first baseline at **93.60 pt** down an A4 page,
    **false** at **81.60**, and **absent** at 93.60.

Six arms, and the two `absent` arms are the discriminators — a claim that a *stated* value is
honoured says nothing about which way an unstated one falls, and it is the unstated case that every
real document takes.

Measured by the reference's own PDF geometry: the boundary is the difference between two consecutive
first-baselines, and the top is the first baseline against the page's top edge. Both are read off
text origins, so nothing here depends on reading a border or a fill.

**The arms are derived from round 53's own fixture rather than authored, and that was not the first
attempt.** A minimal flat ODF written here — correct namespaces, `ooo:configuration-settings`, the
item spelled exactly as the fixture spells it — was read correctly by *our* reader (24.00 pt with
the flag false) and **ignored outright by 26.2.4.2**, which gave 32.00 pt in all six arms including
the two that state `false`. A probe that reports "the reference ignores this setting" because its
own file was not read is the shape this project has paid for twice, so the arms are now one string
substitution each into `paragraph-spacing-collapsed.fodt`, a file 26.2.4.2 demonstrably reads: it
answers 24.00 with the flag as shipped. What the authored file lacked was not chased; the point of
the round-53 rule is that it does not have to be.

Each variant is rendered into the shared `ref` directory under its own numbered stem, per the test
corpus README: two `--convert-to` calls that would write the same output name silently produce one
file and exit 0 for both.
"""
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

PAGE_H = 841.89
PARAGRAPHS = 8

FIXTURE = ('/c/sandbox/workdir/wt-words-r50/dotnet/tests/corpus/features/'
           'paragraph-spacing-collapsed.fodt')

ITEM = ('<config:config-item config:name="%s" config:type="boolean">%s'
        '</config:config-item>')


def variant(source, changes):
    """`changes` maps a setting name to 'true', 'false' or None (remove the item)."""
    out = source
    for name, value in changes.items():
        pattern = re.compile(
            r'<config:config-item config:name="%s" config:type="boolean">'
            r'(?:true|false)</config:config-item>' % re.escape(name))
        if not pattern.search(out):
            raise SystemExit('the fixture does not state %s — refusing to guess' % name)
        out = pattern.sub('' if value is None else ITEM % (name, value), out, count=1)
    return out


ADD = 'AddParaTableSpacing'
AT_START = 'AddParaTableSpacingAtStart'

CASES = [
    ('as-shipped', {},
     'the fixture unchanged: add=false, atStart=true. Expect first 93.60, boundaries 24.00'),
    ('add-true', {ADD: 'true'}, 'expect boundaries 32.00 — the two spacings sum'),
    ('add-removed', {ADD: None},
     'the discriminator: absent must behave as TRUE, so 32.00'),
    ('atstart-false', {AT_START: 'false'},
     'expect the first baseline at 81.60, the space-before dropped'),
    ('atstart-removed', {AT_START: None},
     'the second discriminator: absent must behave as TRUE, so 93.60'),
    ('both-off', {ADD: 'false', AT_START: 'false'},
     'the corner: 81.60 and 24.00 together'),
]

PARAGRAPHS = 8
PAGE_H = 841.89

def render_ref(src, outdir, slot):
    subprocess.run(
        ['soffice', '-env:UserInstallation=file://' + os.path.join(outdir, f'prof{slot}'),
         '--headless', '--norestore', '--convert-to', 'pdf',
         '--outdir', os.path.join(outdir, 'ref'), src], capture_output=True, timeout=300)


def render_ours(cli, src, outdir):
    subprocess.run([cli, 'render', src, '--outdir', os.path.join(outdir, 'ours')],
                   capture_output=True, timeout=300)


sys.path.insert(0, "/c/sandbox/workdir/wt-words-r50/dotnet/research/probes/slides-r15")


def baselines(pdf):
    """Every text origin's y on page 1, top-down, in drawing order."""
    from pdfops import objects, pages, content
    data = open(pdf, 'rb').read()
    objs = objects(data)
    out = []
    for pn in pages(data, objs):
        c = content(data, objs, pn).decode('latin1')
        cur = None
        for m in re.finditer(r'([-\d.]+) ([-\d.]+) Td|(?:TJ|Tj)', c):
            if m.group(2):
                cur = float(m.group(2))
            elif cur is not None:
                out.append(round(PAGE_H - cur, 2))
        break
    return out


if __name__ == '__main__':
    out = os.path.abspath(sys.argv[1])
    workers = int(sys.argv[2]) if len(sys.argv) > 2 else 4
    cli = os.environ.get('XRAY_OFFICE_CLI') or (
        '/c/sandbox/workdir/wt-words-r50/dotnet/tools/XRay.Office.Cli/bin/Debug/net10.0/'
        'linux-x64/XRay.Office.Cli')
    if os.path.isdir(out):
        shutil.rmtree(out)
    for d in ('in', 'ref', 'ours'):
        os.makedirs(os.path.join(out, d), exist_ok=True)

    source = open(FIXTURE, encoding='utf-8').read()

    built = []
    for i, (name, changes, why) in enumerate(CASES):
        stem = '%02d-%s' % (i, name)
        path = os.path.join(out, 'in', stem + '.fodt')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(variant(source, changes))
        built.append((name, stem, path, why))

    with ThreadPoolExecutor(workers) as pool:
        list(pool.map(lambda t: render_ref(t[2], out, built.index(t) % workers), built))
    for t in built:
        render_ours(cli, t[2], out)

    missing = ['%s: no %s' % (t[1], s) for t in built for s in ('ref', 'ours')
               if not os.path.exists(os.path.join(out, s, t[1] + '.pdf'))]
    if missing:
        print('REFUSING TO PRINT — %d halves missing: %s' % (len(missing), missing))
        sys.exit(2)

    read = {}
    for name, stem, path, why in built:
        for side in ('ref', 'ours'):
            ys = baselines(os.path.join(out, side, stem + '.pdf'))
            if len(ys) != PARAGRAPHS:
                print('REFUSING TO PRINT — %s/%s drew %d baselines, not %d'
                      % (side, stem, len(ys), PARAGRAPHS))
                sys.exit(2)
            read[(stem, side)] = ys

    print('%d flat-ODF variants of the round-53 fixture, %d halves, all %d baselines each\n'
          % (len(built), 2 * len(built), PARAGRAPHS))
    print('%-16s %-22s %-22s %s' % ('case', 'reference top / pitch', 'ours', 'what it tests'))
    for name, stem, path, why in built:
        r = read[(stem, 'ref')]
        o = read[(stem, 'ours')]
        rp = sorted({round(b - a, 2) for a, b in zip(r, r[1:])})
        op = sorted({round(b - a, 2) for a, b in zip(o, o[1:])})
        # Half a twip and a little: both sides quantise onto the twip grid and the reference
        # rounds its PDF coordinates to two places, so 93.60 against 93.59 is agreement.
        agree = ('AGREE'
                 if abs(r[0] - o[0]) <= 0.06 and len(rp) == len(op)
                 and all(abs(a - b) <= 0.06 for a, b in zip(rp, op))
                 else 'DIFFER')
        print('%-16s %7.2f / %-12s %7.2f / %-12s %-7s %s'
              % (name, r[0], rp, o[0], op, agree, why))
