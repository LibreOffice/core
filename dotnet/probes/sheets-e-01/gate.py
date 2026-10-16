#!/usr/bin/env python3
"""Reach, direction and the gate's verdict for all three banks.

    gate.py <before-dir> <after-dir> <reference-dir> > rows.tsv 2> summary.txt

`xray-office analyze` takes many files in one invocation and emits one TSV row each, so the whole
gate is **three** process launches rather than 342. The first version of this script called it
once per file per bank and had reached the letter C after twenty minutes; the batched form is
the same instrument and finishes in one.

`###` counts need the extracted text, which is per-file — so they are taken **only for the
documents whose rendering actually changed**, plus their references. Every other document's
`###` count is unchanged by construction (its PDF is byte-identical), which is asserted by the
byte comparison rather than assumed.
"""
import os
import subprocess
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "..", "words-rebase-02"))
from verdict import verdict                                       # noqa: E402

CLI = os.environ["XRAY_OFFICE_CLI"]


def rows(paths):
    """{path: (pages, alnumWords, unembedded)} for many PDFs in one launch."""
    out = {}
    for chunk in (paths[i:i + 40] for i in range(0, len(paths), 40)):
        text = subprocess.run([CLI, "analyze", "--no-header", *chunk],
                              capture_output=True, text=True).stdout
        for line in text.splitlines():
            f = line.split("\t")
            if len(f) > 10:
                # 1 pages, 4 wordsAlnum, 9 unembedded. **Not 10** — that is `subset`, which is
                # nonzero on nearly every PDF here, and reading it as the unembedded count made
                # the first run of this script score all 171 documents as failing.
                out[f[0]] = (f[1], f[4], f[9])
    return out


def hashes(path):
    text = subprocess.run([CLI, "analyze", "--text", path],
                          capture_output=True, text=True).stdout
    return sum(1 for t in text.split() if t and set(t) == {"#"})


def main(before, after, ref):
    names = sorted(f for f in os.listdir(after) if f.endswith(".pdf"))
    pairs = []
    for n in names:
        stem = n[:-4]
        cands = [f for f in os.listdir(ref)
                 if f.startswith(stem + "__") and f.endswith(".pdf")]
        if not cands:
            print("NO REFERENCE FOR %s" % stem, file=sys.stderr)
            continue
        pairs.append((stem, os.path.join(before, n), os.path.join(after, n),
                      os.path.join(ref, cands[0])))

    B = rows([p[1] for p in pairs])
    A = rows([p[2] for p in pairs])
    R = rows([p[3] for p in pairs])

    changed = [p for p in pairs
               if open(p[1], "rb").read() != open(p[2], "rb").read()]
    hb = {p[0]: hashes(p[1]) for p in changed}
    ha = {p[0]: hashes(p[2]) for p in changed}
    hr = {p[0]: hashes(p[3]) for p in changed}

    print("stem\tpagesB\tpagesA\tpagesR\twordsB\twordsA\twordsR"
          "\thashB\thashA\thashR\tverdictB\tverdictA\tbytesChanged")

    closer = further = level = 0
    vb = va = 0
    flips = []
    tot = [0, 0, 0]

    for stem, bp_, ap_, rp_ in pairs:
        bpg, bw, bu = B[bp_]
        apg, aw, au = A[ap_]
        rpg, rw, _ = R[rp_]
        ch = stem in ha

        if ch:
            tot[0] += hb[stem]
            tot[1] += ha[stem]
            tot[2] += hr[stem]
            if abs(ha[stem] - hr[stem]) < abs(hb[stem] - hr[stem]):
                closer += 1
            elif abs(ha[stem] - hr[stem]) > abs(hb[stem] - hr[stem]):
                further += 1
            else:
                level += 1

        b = verdict(bpg, rpg, bw, rw, bu)
        a = verdict(apg, rpg, aw, rw, au)
        vb += b == "match"
        va += a == "match"
        if a != b:
            flips.append((stem, b, a, bw, aw, rw))

        print("\t".join(str(x) for x in (
            stem, bpg, apg, rpg, bw, aw, rw,
            hb.get(stem, "-"), ha.get(stem, "-"), hr.get(stem, "-"),
            b, a, int(ch))))

    print("documents: %d" % len(pairs), file=sys.stderr)
    print("renderings byte-changed: %d" % len(changed), file=sys.stderr)
    print("of those, ### count closer / equal / further: %d / %d / %d"
          % (closer, level, further), file=sys.stderr)
    print("### on the changed documents: before %d, after %d, reference %d"
          % tuple(tot), file=sys.stderr)
    print("gate matches over 171: before %d, after %d" % (vb, va), file=sys.stderr)
    for f in flips:
        print("VERDICT %s: %s -> %s (words %s -> %s vs ref %s)" % f, file=sys.stderr)


if __name__ == "__main__":
    main(*sys.argv[1:4])
