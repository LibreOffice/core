#!/usr/bin/env python3
"""Why 16 decks state a run-level a:gradFill and only one rendering moved.

The predecessor's census counted `a:gradFill` inside an `a:rPr` or an `a:defRPr`
in a slide, layout or master part: 16 decks, 40 instances, reproduced to the digit.
The measured reach of reading it is one deck. This says where the other 39 go.

For each instance it reports the three things that decide whether it can change a
pixel, all of which a declaration census is blind to:

  * `alpha`    -- does the gradient carry an a:alpha? Without one the run resolves
                  to an opaque colour, and if that colour equals the one it was
                  already inheriting, nothing moves.
  * `where`    -- the part kind. A defRPr in a master lstStyle reaches only runs
                  that inherit from it, which may be none.
  * `stop0`    -- the colour the new rule actually picks, so it can be compared
                  against black, which is what the run resolved to before.
"""
import collections
import os
import re
import sys
import zipfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/c/sandbox/workdir/sample-files/slides"

PART = re.compile(r'^ppt/(slides|slideLayouts|slideMasters)/[^/]+\.xml$')
RPR = re.compile(r'<a:(defRPr|rPr)\b[^>]*?(?<!/)>(.*?)</a:\1>', re.S)
GRAD = re.compile(r'<a:gradFill\b.*?</a:gradFill>', re.S)
ALPHA = re.compile(r'<a:alpha\s+val="(\d+)"')
FIRSTCLR = re.compile(r'<a:(srgbClr|schemeClr|sysClr)\s+val="([^"]+)"')


def decks(root):
    for dirpath, _, names in os.walk(root):
        for name in sorted(names):
            if name.lower().endswith((".pptx", ".pptm", ".ppsx", ".potx")):
                yield os.path.join(dirpath, name)


def main() -> int:
    rows = []
    per_deck = collections.Counter()

    for path in sorted(decks(ROOT)):
        try:
            z = zipfile.ZipFile(path)
        except Exception:
            continue
        for part in sorted(n for n in z.namelist() if PART.match(n)):
            try:
                text = z.read(part).decode("utf-8", "replace")
            except Exception:
                continue
            for props in RPR.finditer(text):
                body = props.group(2)
                for grad in GRAD.finditer(body):
                    g = grad.group(0)
                    alphas = sorted({int(a) for a in ALPHA.findall(g)})
                    clr = FIRSTCLR.search(g)
                    rows.append((
                        os.path.basename(path),
                        part.split("/")[1],
                        ",".join(str(a) for a in alphas) or "-",
                        f"{clr.group(1)}:{clr.group(2)}" if clr else "-",
                    ))
                    per_deck[os.path.basename(path)] += 1

    print(f"{len(rows)} run/defRPr gradient fills in {len(per_deck)} decks\n")
    print(f"{'deck':<58} {'part kind':<14} {'alpha':<12} {'stop 0'}")
    for row in rows:
        print(f"{row[0][:57]:<58} {row[1]:<14} {row[2]:<12} {row[3]}")

    translucent = [r for r in rows if r[2] != "-"]
    print(f"\ncarrying an a:alpha: {len(translucent)} of {len(rows)} instances, "
          f"in {len({r[0] for r in translucent})} decks")
    return 0


if __name__ == "__main__":
    sys.exit(main())
