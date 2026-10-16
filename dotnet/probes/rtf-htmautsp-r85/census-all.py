#!/usr/bin/env python3
r"""Every `checkFirstRun` caller that stands before a document's `\htmautsp`.

    python3 census-all.py /home/user/corpus-odf/words

The thirteen call sites of `RTFDocumentImpl::checkFirstRun`, verified by
reading each one in `/home/user/libreoffice-core`:

    rtfdispatchdestination.cxx:264   \footnote
    rtfdispatchdestination.cxx:405   \shptxt, \dptxbxtext  (not a picture frame)
    rtfdispatchflag.cxx:893          \super                (not a style-sheet entry)
    rtfdispatchflag.cxx:1103         \dptxbx
    rtfdispatchsymbol.cxx:133        \par                  (not \chftnsep's group)
    rtfdispatchsymbol.cxx:250        \cell, \nestcell
    rtfdispatchsymbol.cxx:518        \column
    rtfdispatchsymbol.cxx:574        \page
    rtfdocumentimpl.cxx:724          tableBreak()          \row, \nestrow
    rtfdocumentimpl.cxx:732          parBreak()
    rtfdocumentimpl.cxx:1297         resolvePict()         a {\pict} group closing
    rtfdocumentimpl.cxx:1706         text()                text in most destinations
    rtfdocumentimpl.cxx:4137         RTFFrame::setSprm     a frame property, first run only

None of them is dispatched at all inside a `Destination::SKIP` group, because
`RTFTokenizer::dispatchKeyword` returns first (`rtftokenizer.cxx`:2156-2164).
"""
import pathlib
import sys
from census import SKIPPED, GUARDED, opening_word

FLAGS = {"super"}
SYMBOLS = {"par", "cell", "nestcell", "column", "page", "row", "nestrow"}
DESTS = {"footnote", "shptxt", "dptxbxtext", "dptxbx"}


def scan(d: bytes) -> tuple[int, list[tuple[int, str]]]:
    stack: list[str] = []
    word = -1
    hits: list[tuple[int, str]] = []
    i, n = 0, len(d)
    while i < n:
        c = d[i]
        if c == 0x5C:
            if i + 1 < n and d[i + 1] in b"\\{}'":
                i += 4 if d[i + 1] == 0x27 else 2
                continue
            j = i + 1
            while j < n and (65 <= d[j] <= 90 or 97 <= d[j] <= 122):
                j += 1
            kw = d[i + 1:j].decode("latin-1")
            live = not any(s in SKIPPED for s in stack)
            if kw == "htmautsp" and word < 0 and live:
                word = i
            if live and (kw in SYMBOLS or kw in DESTS
                         or (kw in FLAGS and not any(s in GUARDED for s in stack))):
                hits.append((i, kw))
            i = j
            continue
        if c == 0x7B:
            stack.append(opening_word(d, i))
            i += 1
            continue
        if c == 0x7D:
            if stack:
                stack.pop()
            i += 1
            continue
        i += 1
    return word, hits


def main(root: pathlib.Path) -> None:
    files = sorted(root.rglob("*.rtf"))
    states = affected = 0
    tally: dict[str, int] = {}
    for f in files:
        word, hits = scan(f.read_bytes())
        if word < 0:
            continue
        states += 1
        early = [(o, k) for o, k in hits if o < word]
        if early:
            affected += 1
            kinds = sorted({k for _, k in early})
            print(f"{f.name[:58]:58s} word {word:8d}  first {min(early)[1]:10s} "
                  f"at {min(early)[0]:8d}  all {kinds}")
            for k in kinds:
                tally[k] = tally.get(k, 0) + 1
    print(f"\n{len(files)} .rtf, {states} state the word, {affected} have a caller before it")
    for k, v in sorted(tally.items(), key=lambda kv: -kv[1]):
        print(f"  {k:12s} {v}")


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "/home/user/corpus-odf/words"))
