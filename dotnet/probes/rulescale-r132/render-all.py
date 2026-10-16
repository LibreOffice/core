#!/usr/bin/env python3
r"""Render every corpus document with one binary and hash what comes out.

    render-all.py <cli> <corpus-root> <doclist> <out.tsv> [jobs]

The confinement control for O70.  `MetricGrid.ToLength` is not a sheets type -- charts, slides and
the four Spreadsheets files all reach it -- so "only Calc passes a zoom, therefore nothing else
moved" is an argument and not a measurement.  Running this against the base binary and against the
fixed one and diffing the two hash columns turns it into one: a document whose bytes are identical
did not move, whatever the reasoning says.

`SOURCE_DATE_EPOCH` is pinned so the PDF's own dates do not make every document differ.  One
temporary directory per DOCUMENT, never per worker slot -- see `CLAUDE.md`, where a per-slot
allocation cost a round 124 renders.
"""
import hashlib, os, shutil, subprocess, sys, tempfile
from concurrent.futures import ThreadPoolExecutor

CLI, ROOT, LIST, OUT = sys.argv[1:5]
JOBS = int(sys.argv[5]) if len(sys.argv) > 5 else 3
EPOCH = '1700000000'


def source_for(stem):
    """The corpus file a banked reference PDF's name came from."""
    want = stem.rsplit('__', 1)[0].replace(' ', '_')
    for dirpath, _d, files in os.walk(ROOT):
        for f in files:
            if os.path.splitext(f)[0].replace(' ', '_') == want:
                return os.path.join(dirpath, f)
    return None


def one(name):
    stem = name[:-4]
    src = source_for(stem)
    if src is None:
        return name, 'no-source', '', 0
    tmp = tempfile.mkdtemp(prefix='rall-')
    try:
        env = dict(os.environ, SOURCE_DATE_EPOCH=EPOCH)
        subprocess.run(['timeout', '-k', '30', '900', CLI, 'render', src,
                        '--format', 'pdf', '--outdir', tmp],
                       capture_output=True, timeout=1000, env=env)
        pdfs = sorted(f for f in os.listdir(tmp) if f.endswith('.pdf'))
        if not pdfs:
            return name, 'ours-failed', '', 0
        h = hashlib.sha256()
        total = 0
        for f in pdfs:
            with open(os.path.join(tmp, f), 'rb') as fh:
                data = fh.read()
            h.update(f.encode()), h.update(data)
            total += len(data)
        return name, 'ok', h.hexdigest(), total
    except Exception as exc:                                    # noqa: BLE001
        return name, 'error:' + type(exc).__name__, '', 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def main():
    names = [l.strip() for l in open(LIST) if l.strip()]
    done = 0
    with open(OUT, 'w') as fh:
        fh.write('doc\tstatus\tsha256\tbytes\n')
        with ThreadPoolExecutor(max_workers=JOBS) as pool:
            for name, status, digest, total in pool.map(one, names):
                fh.write('%s\t%s\t%s\t%d\n' % (name, status, digest, total))
                fh.flush()
                done += 1
    print('rows\t%d' % done)


if __name__ == '__main__':
    main()
