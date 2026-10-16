#!/usr/bin/env python3
"""Audit the predecessor census's `run_gradfill` rule against a matched-tag one.

slides-b-01/census.py:74 counts a run-level gradient with

    <a:(?:rPr|defRPr|endParaRPr)\\b.*?</a:(?:rPr|defRPr|endParaRPr)>

which does not require the closing tag to match the opening one. A self-closing
`<a:rPr lang="en-GB"/>` -- overwhelmingly the commonest form of the element in a
real deck -- therefore opens a span that runs forward under re.S until the next
closing tag of *any* of the three, swallowing whatever lies between: sibling
paragraphs, and `<a:spPr>` blocks whose `a:gradFill` is a shape fill and has
nothing to do with any run.

This reports both counts per deck and prints, for one instance, what the loose
rule actually matched.
"""
import collections
import os
import re
import sys
import zipfile

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/c/sandbox/workdir/sample-files/slides"

PART = re.compile(r'^ppt/(slides|slideLayouts|slideMasters)/[^/]+\.xml$')
LOOSE = re.compile(r'<a:(?:rPr|defRPr|endParaRPr)\b.*?</a:(?:rPr|defRPr|endParaRPr)>', re.S)
TIGHT = re.compile(r'<a:(defRPr|rPr|endParaRPr)\b[^>]*?(?<!/)>(.*?)</a:\1>', re.S)


def main() -> int:
    loose_decks, tight_decks = set(), set()
    loose_n = tight_n = 0
    sample = None

    for dirpath, _, names in os.walk(ROOT):
        for name in sorted(names):
            if not name.lower().endswith((".pptx", ".pptm", ".ppsx", ".potx")):
                continue
            path = os.path.join(dirpath, name)
            try:
                z = zipfile.ZipFile(path)
            except Exception:
                continue
            for part in sorted(n for n in z.namelist() if PART.match(n)):
                try:
                    text = z.read(part).decode("utf-8", "replace")
                except Exception:
                    continue
                for m in LOOSE.finditer(text):
                    if "<a:gradFill" in m.group(0):
                        loose_n += 1
                        loose_decks.add(name)
                        if sample is None and len(m.group(0)) > 400:
                            sample = (name, part, m.group(0))
                for m in TIGHT.finditer(text):
                    if "<a:gradFill" in m.group(2):
                        tight_n += 1
                        tight_decks.add(name)

    print(f"loose (census.py:74)   {len(loose_decks):3d} decks {loose_n:4d} instances")
    print(f"tight (matched tags)   {len(tight_decks):3d} decks {tight_n:4d} instances")
    print(f"\ndecks the loose rule adds: "
          f"{sorted(loose_decks - tight_decks)}")

    if sample:
        name, part, text = sample
        print(f"\n--- what one loose match swallowed, {name} / {part} "
              f"({len(text)} chars) ---")
        print(text[:300].replace("><", ">\n<"))
        print("   ... ")
        print(text[-300:].replace("><", ">\n<"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
