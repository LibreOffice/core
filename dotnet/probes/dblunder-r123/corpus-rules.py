#!/usr/bin/env python3
r"""Score the rule model against the CORPUS's own faces and sizes, out of the banked reference.

    corpus-rules.py <ref-dir> > rows.tsv

The authored probe covers six faces at seven sizes; this covers whatever 26.2.4.2 actually drew
over the 947-document corpus, at whatever sizes the documents ask for, with no new render at all
-- `/home/user/gate-r122/ref` holds one reference PDF per corpus document.

**What counts as a text line here, and why the filter is what it is.**  A page's thin horizontal
strokes are underlines, strikethroughs, table borders, paragraph borders and drawing-layer lines,
and only the first two are this model's.  The discriminator is that a text line is drawn across
the *advance of its own run*: it is required to sit in an em-scaled window of a span's baseline
AND to match that span's x-extent at both ends.  A border spans its cell, not its text, and a
drawing line has no span over it at all.  The filter can still admit a border that happens to
underrun a full-width run, so a MISS is a candidate to look at rather than a proven failure --
which is the direction that keeps the number honest, since it can only make the score worse.

Only faces that resolve to a file on this machine are scored.  A document that embeds its own
font is subset into the PDF under a name no system face carries, and its metrics are the file's
rather than the machine's; those are counted and reported separately rather than guessed at.
"""
import os, re, subprocess, sys, collections
import pymupdf
from predict_pos import device_metrics, drawn          # noqa: E402
from fontmetrics import metrics                        # noqa: E402

# A word-processing page is written in twips and a Calc or Impress one in hundredths of a
# millimetre, and the extension says which application painted it -- see quantise-r120.
WRITER = ('doc', 'docx', 'docm', 'dot', 'rtf', 'odt', 'ott', 'fodt')

WINDOW = {'under': (0.005, 0.60), 'strike': (-0.70, -0.02)}


def font_files():
    """PostScript name -> (file, family), from fontconfig's own index."""
    out = subprocess.run(['fc-list', ':', 'file', 'family', 'postscriptname'],
                         capture_output=True, text=True).stdout
    table = {}
    for line in out.splitlines():
        path, rest = line.split(': ', 1)
        family, _, ps = rest.partition(':postscriptname=')
        if not ps:
            continue
        table.setdefault(ps.strip(), (path, family.split(',')[0].strip()))
    return table


def rules_on(page):
    out = []
    for d in page.get_drawings():
        r = d['rect']
        if r.width < 3 or d['type'] != 's' or r.height > 1.5:
            continue
        width = d.get('width') or 0.0
        if width > 0:
            out.append((r.x0, r.x1, (r.y0 + r.y1) / 2.0, width))
    return out


def main(root):
    table = font_files()
    cache = {}
    scaled_docs = set()
    print('doc\tpage\tface\tsize\tkind\tbranch\tdrawn_off\tdrawn_w\twant_off\twant_w\tverdict')
    tally = collections.Counter()

    for name in sorted(os.listdir(root)):
        if not name.endswith('.pdf'):
            continue
        ext = name.rsplit('__', 1)[-1][:-4]
        upi = 1440 if ext in WRITER else 2540
        try:
            doc = pymupdf.open(os.path.join(root, name))
        except Exception:
            tally['unreadable'] += 1
            continue

        for pi in range(doc.page_count):
            page = doc[pi]
            rules = rules_on(page)
            if not rules:
                continue
            for block in page.get_text('dict')['blocks']:
                for line in block.get('lines', []):
                    for span in line['spans']:
                        if not span['text'].strip():
                            continue
                        x0, _, x1, _ = span['bbox']
                        base = span['origin'][1]
                        em = span['size']
                        ps = re.sub(r'^[A-Z]{6}\+', '', span['font'])
                        for (rx0, rx1, ry, w) in rules:
                            if abs(rx0 - x0) > 1.0 or abs(rx1 - x1) > 1.0:
                                continue
                            depth = ry - base
                            kind = ('under' if WINDOW['under'][0] * em <= depth <= WINDOW['under'][1] * em
                                    else 'strike' if WINDOW['strike'][0] * em <= depth <= WINDOW['strike'][1] * em
                                    else None)
                            if kind is None:
                                continue
                            if ps not in table:
                                tally['embedded'] += 1
                                continue
                            path, family = table[ps]
                            if path not in cache:
                                try:
                                    cache[path] = metrics(path)
                                except Exception:
                                    cache[path] = None
                            m = cache[path]
                            if m is None:
                                tally['unreadable-face'] += 1
                                continue
                            dm = device_metrics(m, family, em, upi)
                            key = 'single' if kind == 'under' else 'strike'
                            size_px, offsets = dm[key]
                            want_h, want_pos = drawn(key, size_px, offsets, upi)
                            ok = abs(w - want_h) <= 0.0015 and abs(depth - want_pos[0]) <= 0.0015
                            # A slide, a shrunk cell or a sheet printed to a scale is drawn through
                            # a transform, so what the page states is the reference's own number
                            # MULTIPLIED by something this instrument cannot see -- the em pymupdf
                            # reports is the size after scaling while the metric was computed
                            # before it. Those rows cannot be scored from the page alone, and O64
                            # is what separates them: an unscaled rule's thickness is a whole
                            # logical unit of the map mode the page was painted in, and a scaled
                            # one is not. A half-point test on the SIZE does not do it -- a scale
                            # of 0.975 turns a 5.13 pt em into a clean 5.00.
                            unit = 72.0 / upi
                            scaled = abs(w / unit - round(w / unit)) > 0.02
                            bucket = 'scaled' if scaled else ('%s-%s' % (key, 'hit' if ok else 'miss'))
                            tally[bucket] += 1
                            tally['%s|%s' % (ext, bucket)] += 1
                            if scaled:
                                scaled_docs.add(name)
                                continue
                            if not ok:
                                print('%s\t%d\t%s\t%.4f\t%s\t%s\t%.4f\t%.4f\t%.4f\t%.4f\tMISS'
                                      % (name, pi, ps, em, key, dm['branch'],
                                         depth, w, want_pos[0], want_h))
        doc.close()

    for k, v in sorted(tally.items()):
        print('# %s\t%d' % (k, v), file=sys.stderr)
    with open('scaled-docs.txt', 'w') as fh:
        for name in sorted(scaled_docs):
            fh.write(name + '\n')


if __name__ == '__main__':
    main(sys.argv[1])
